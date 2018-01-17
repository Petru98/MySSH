#include <Server.hpp>
#include <Lock.hpp>
#include <Logging.hpp>
#include <CommandTree.hpp>
#include <pgpdef.hpp>
#include <stdexcept>
#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <cryptopp/osrng.h>
#include <cryptopp/hex.h>



Server::Server()
{
    this->initializeOptions();

    CryptoPP::AutoSeededRandomPool rng;
    this->privatekey.GenerateRandomWithKeySize(rng, pgp::RSA_KEY_SIZE);
    this->publickey = this->privatekey;

    this->home_dir = "home";
}
Server::~Server()
{}



void Server::setClientCWD(std::size_t index, const std::string& dir)
{
    Client& client = (*this->clients[index]);
    std::string abs_path;

    if(dir[0] == '/')
        abs_path = dir;
    else
    {
        abs_path = client.cwd;
        auto token_begin = dir.begin();

        while(token_begin < dir.end())
        {
            auto token_end = token_begin;
            while(token_end != dir.end() && (*token_end) != '/')
                ++token_end;

            if(token_end != token_begin)
            {
                const std::string token(token_begin, token_end);

                if(token == ".")
                    {}
                else if(token == "..")
                {
                    if(abs_path.size() > 1)
                    {
                        const auto pos = abs_path.find_last_of('/');

                        if(pos == 0 || pos == std::string::npos)
                            abs_path = "/";
                        else
                            abs_path.resize(pos);
                    }
                }
                else
                {
                    if(abs_path.size() > 1)
                        abs_path += '/';
                    abs_path += token;
                }
            }

            token_begin = token_end + 1;
        }
    }

    struct stat info;
    if(stat((client.home + abs_path).c_str(), &info) == -1)
        throw std::runtime_error(strerror_r(errno, client.errmsg, client.ERRMSG_MAX_SIZE));

    if(!S_ISDIR(info.st_mode))
        throw std::runtime_error("not a directory");

    client.cwd = abs_path;
}



void Server::initializeOptions()
{
    this->options.addOption('d', "database", "db.xml");
    this->options.addOption('p', "port", "1100");
}



void Server::init(int argc, char** argv)
{
    // Options
    this->options.parse(argc, argv);

    // Databse
    tinyxml2::XMLError xmlerr = this->database.LoadFile(this->options.findOption("database").c_str());

    if(xmlerr == tinyxml2::XML_ERROR_FILE_NOT_FOUND)
    {
        tinyxml2::XMLElement* xml = this->database.NewElement("xml");
        if(xml == nullptr)
            throw std::runtime_error(this->database.ErrorStr());
        tinyxml2::XMLElement* users = this->database.NewElement("users");
        if(users == nullptr)
            throw std::runtime_error(this->database.ErrorStr());

        if(this->database.InsertFirstChild(xml) == nullptr || xml->InsertEndChild(users) == nullptr)
        {
            this->database.DeleteNode(users);
            this->database.DeleteNode(xml);
            throw std::runtime_error(this->database.ErrorStr());
        }
    }
    else if(xmlerr != tinyxml2::XML_SUCCESS)
        throw std::runtime_error(this->database.ErrorStr());

    // Create home directory for users
    if(mkdir(this->home_dir.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == -1 && errno != EEXIST)
        throw std::runtime_error("could not create 'home' directory");

    // Listener
    int port = std::stoi(this->options.findOption("port"));
    if(port < 0 || port > 65535)
        throw std::runtime_error("invalid port");

    this->listener.create(Socket::Tcp, Socket::INet, SOCK_CLOEXEC);
    const int reuse_addr = 1;
    setsockopt(this->listener.getFD(), SOL_SOCKET, SO_REUSEADDR, &reuse_addr, sizeof(reuse_addr));
    this->listener.bind(port);
    this->listener.listen();
}

void Server::run(int argc, char** argv)
{
    this->init(argc, argv);

    Thread runner(&Server::loopAcceptConn, this);
    runner.detach();
    this->loopInterface();

    this->free();
}

void Server::free()
{
    Lock lock(this->mutex);

    this->listener.close();

    for(std::size_t i = 0; i < this->clients.size(); ++i)
    {
        this->clients[i]->disconnect();
        this->handlers[i]->join();
        delete this->clients[i];
        delete this->handlers[i];
    }

    this->clients.clear();
    this->handlers.clear();
}



void Server::loopAcceptConn()
{
    PgpSocket sock;
    IpAddress ip;
    uint16_t port;

    try
    {
        while(this->listener.accept(ip, port, sock))
        {
            this->mutex.lock();

            if(this->listener.isValid() == false)
                break;

            if(fcntl(sock.getFD(), F_SETFD, FD_CLOEXEC) == -1)
                sock.close();
            else
            {
                this->clients.push_back(new Client(std::move(sock), ip, port));
                this->handlers.push_back(new Thread(&Server::handleClient, this, this->clients.size() - 1));

                for(std::size_t i = 0; i < this->handlers.size(); ++i)
                    if(this->handlers[i]->isAlive() == false)
                    {
                        delete this->clients[i];
                        delete this->handlers[i];
                        this->handlers.erase(this->handlers.begin() + i);
                        this->clients.erase(this->clients.begin() + i);
                    }
            }

            this->mutex.unlock();
        }
    }
    catch(...)
    {}
}

void Server::handleClient(std::size_t index)
{
    Client& client = (*this->clients[index]);
    if(this->handleClientInit(client) == false)
        return;

    // Loop
    std::string prompt;
    std::string buffer;
    bool must_exit = false;

    while(must_exit == false)
    {
        try
        {
            prompt.clear();
            prompt += client.name;
            prompt += ':';
            prompt += client.cwd;
            prompt += "$ ";
            client.sock.send(prompt);

            fd_set fdset;
            FD_ZERO(&fdset);
            FD_SET(client.sock.getFD(), &fdset);
            FD_SET(client.server_pipe.getReadFD(), &fdset);

            if(select(std::max(client.sock.getFD(), client.server_pipe.getReadFD()) + 1, &fdset, nullptr, nullptr, nullptr) > 0)
            {
                Lock(client.mutex);

                // Command from server thread
                if(FD_ISSET(client.server_pipe.getReadFD(), &fdset))
                    must_exit = this->executeServerCommand(client) == false;

                // Communication with the client
                if(must_exit == false && FD_ISSET(client.sock.getFD(), &fdset))
                {
                    client.sock.recvString(buffer);
                    CommandTree(buffer).execute(&Server::executeClientCommand, this, index);

                    client.sock.send8(0);
                }
            }
        }
        catch(ExitEvent& e)
        {
            must_exit = true;
        }
        catch(Socket::Error& e)
        {
            error(e.what());
            break;
        }
        catch(std::exception& e)
        {
            error(e.what());
        }
        catch(...)
        {
            error("unknown exception caught in 'Server::handleClient'");
            break;
        }

        if(must_exit == true)
            client.sock.send8(255);
    }

    client.sock.close();
}

bool Server::handleClientInit(Client& client)
{
    Lock(client.mutex);

    std::string buffer;

    // Public key exchange
    client.sock.recvUnprocessed(buffer);
    client.publickey.Load(CryptoPP::StringSource(buffer, true).Ref());

    this->publickey.Save(CryptoPP::StringSink(buffer).Ref());
    client.sock.sendUnprocessed(buffer);

    client.sock.setKeys(&client.publickey, &this->privatekey);

    // Username
    client.sock.recvString(buffer);

    tinyxml2::XMLElement* user_info = this->findUser(buffer.c_str());
    if(user_info == nullptr)
    {
        client.sock.send8(0);
        return false;
    }

    client.name = std::move(buffer);
    client.home = this->home_dir + '/' + client.name;
    client.sock.send8(1);

    // Password
    std::string dbpasshash_bin;
    CryptoPP::StringSource(user_info->FirstChildElement("password")->GetText(), true,
        new CryptoPP::HexDecoder(
            new CryptoPP::StringSink(dbpasshash_bin)
        )
    );

    std::string recvpasshash_bin;
    client.sock.recvString(buffer);
    CryptoPP::StringSource(buffer, true,
        new CryptoPP::HashFilter(
            CryptoPP::SHA512().Ref(), new CryptoPP::StringSink(recvpasshash_bin)
        )
    );

    if(recvpasshash_bin != dbpasshash_bin)
    {
        client.sock.send8(0);
        return false;
    }

    client.sock.send8(1);
    return true;
}

int Server::executeClientCommand(std::size_t index, const std::vector<std::string>& cmd, int stdinfd, int stdoutfd, int stderrfd, bool async)
{
    Client& client = (*this->clients[index]);
    Lock(this->mutex);
    Lock(client.mutex);

    if(cmd[0] == "exit")
        throw ExitEvent();

    if(cmd[0] == "cd")
    {
        if(cmd.size() == 1)
            client.cwd = '/';
        else if(cmd.size() == 2)
        {
            try
            {
                this->setClientCWD(index, cmd[1]);
            }
            catch(std::exception& e)
            {
                client.sock.send8(1);
                client.sock.sendString("server: ");
                client.sock.send8(1);
                client.sock.sendString(e.what());
                client.sock.send8(1);
                client.sock.sendString("\n");
                return 1;
            }
        }
        else
        {
            client.sock.send8(1);
            client.sock.sendString("server: invalid number of arguments\n");
            return 2;
        }

        return 0;
    }

    int exit_code = 0;
    Pipe pipe;
    pid_t pid = fork();

    if(pid == -1)
    {
        constexpr char msg[] = "server: internal error\n";
        client.sock.send8(1);
        client.sock.sendString(msg, sizeof(msg) - 1);
        exit_code = -1;
    }
    else if(pid == 0)
    {
        ::close(pipe.getReadFD());

        int argc = static_cast<int>(cmd.size());
        char** argv = nullptr;

        try
        {
            argv = new char*[argc + 1];
            argv[argc] = nullptr;

            for(int i = 0; i < argc; ++i)
            {
                argv[i] = new char[cmd[i].length() + 1];
                strcpy(argv[i], cmd[i].c_str());
            }
        }
        catch(...)
        {
            exit(255);
        }



        auto replace_fd = [](int to_replace, int main_replacement, int alternative_replacement)
        {
            int replacement = to_replace;

            if(main_replacement != to_replace)
                replacement = main_replacement;
            else if(alternative_replacement != to_replace)
                replacement = alternative_replacement;

            if(replacement != to_replace)
            {
                if(dup2(replacement, to_replace) == -1)
                    exit(255);
            }
        };

        replace_fd(STDIN_FILENO , stdinfd , stdinfd);
        replace_fd(STDOUT_FILENO, stdoutfd, pipe.getWriteFD());
        replace_fd(STDERR_FILENO, stderrfd, pipe.getWriteFD());

        if(chdir((client.home + client.cwd).c_str()) == -1)
            exit(255);

        execvp(argv[0], argv);
        exit(255);
    }
    else
    {
        if(async == false)
        {
            ::close(pipe.getWriteFD());

            std::string buffer;
            std::size_t size;
            constexpr std::size_t max_buffer_size = 8192;
            buffer.resize(max_buffer_size);

            while((size = pipe.read(&buffer[0], max_buffer_size)) > 0)
            {
                buffer.resize(size);
                client.sock.send8(1);
                client.sock.send(buffer);
                buffer.resize(max_buffer_size);
            }

            int status;
            waitpid(pid, &status, 0);

            if(WIFEXITED(status))
            {
                exit_code = WEXITSTATUS(status);
                if(exit_code == 255)
                {
                    constexpr char msg[] = "server: executable not found, permission denied or internal error\n";
                    client.sock.send8(1);
                    client.sock.sendString(msg, sizeof(msg) - 1);
                }
            }
            else
                exit_code = -1;
        }
    }

    return exit_code;
}



void Server::loopInterface()
{
    std::string input;
    bool must_exit = false;

    while(must_exit == false)
    {
        print("> ");

        if(!std::getline(std::cin, input))
            must_exit = true;
        else
        {
            std::vector<std::string> cmd = this->parseCommand(input.c_str());

            if(cmd.size() > 0)
            {
                try
                {
                    if(cmd[0] == "exit")
                        must_exit = true;
                    else if(cmd[0] == "adduser")
                    {
                        if(cmd.size() - 1 != 2)
                            error("invalid number of arguments");
                        else
                            this->addUser(cmd[1], cmd[2]);
                    }
                    else if(cmd[0] == "rmuser")
                    {
                        if(cmd.size() - 1 != 1)
                            error("invalid number of arguments");
                        else
                            this->removeUser(cmd[1]);
                    }
                    else if(cmd[0] == "status")
                    {
                        print("port: %u\n", this->listener.getPort());
                    }
                    else
                        error("unknown command");
                }
                catch(std::exception& e)
                {
                    error(e.what());
                }
            }
        }
    }
}

bool Server::executeServerCommand(Client& client)
{

    uint8_t code;
    client.server_pipe.read(&code, sizeof(code));

    switch(code)
    {
    case 0: return false;
    }

    return true;
}

std::vector<std::string> Server::parseCommand(const char* buffer)
{
    constexpr char delims[] = " \t";
    bool done = ((*buffer) == '\0');
    std::vector<std::string> argv;

    while(done == false)
    {
        while((*buffer) != '\0' && strchr(delims, *buffer) != nullptr)
            ++buffer;

        if((*buffer) == '\0')
        {
            done = true;
        }
        else
        {
            std::string str;
            bool quotes_open = false;

            while((*buffer) != '\0' && (quotes_open == true || strchr(delims, *buffer) == nullptr))
            {
                if((*buffer) == '\"')
                    quotes_open = !quotes_open;

                str.push_back(*buffer);
                ++buffer;
            }

            done = ((*buffer) == '\0');
            ++buffer;
            argv.push_back(std::move(str));
        }
    }

    return std::move(argv);
}



tinyxml2::XMLElement* Server::findUser(const char* name)
{
    tinyxml2::XMLElement* xml = this->database.RootElement();
    assert(xml != nullptr);
    tinyxml2::XMLElement* users = xml->FirstChildElement("users");
    assert(users != nullptr);

    tinyxml2::XMLElement* user = users->FirstChildElement("user");
    while(user != nullptr && user->FirstChildElement("name") != nullptr && strcmp(user->FirstChildElement("name")->GetText(), name) != 0)
        user = user->NextSiblingElement("user");

    return user;
}
tinyxml2::XMLElement* Server::findUser(const std::string& name)
{
    return this->findUser(name.c_str());
}

void Server::addUser(const std::string& name, const std::string& password)
{
    tinyxml2::XMLElement* xml = this->database.RootElement();
    assert(xml != nullptr);
    tinyxml2::XMLElement* users = xml->FirstChildElement("users");
    assert(users != nullptr);


    tinyxml2::XMLElement* user = this->findUser(name);
    if(user != nullptr)
        throw std::runtime_error("an user with the same name already exists");

    user = this->database.NewElement("user");
    if(user == nullptr)
        throw std::runtime_error(this->database.ErrorStr());

    tinyxml2::XMLElement* user_name = this->database.NewElement("name");
    if(user_name == nullptr)
    {
        this->database.DeleteNode(user);
        throw std::runtime_error(this->database.ErrorStr());
    }

    tinyxml2::XMLElement* user_password = this->database.NewElement("password");
    if(user_password == nullptr)
    {
        this->database.DeleteNode(user);
        this->database.DeleteNode(user_name);
        throw std::runtime_error(this->database.ErrorStr());
    }



    std::string passhash_hex;
    CryptoPP::StringSource(password, true,
        new CryptoPP::HashFilter(CryptoPP::SHA512().Ref(),
            new CryptoPP::HexEncoder(
                new CryptoPP::StringSink(passhash_hex)
            )
        )
    );

    user_name->SetText(name.c_str());
    user_password->SetText(passhash_hex.c_str());
    user->InsertEndChild(user_name);
    user->InsertEndChild(user_password);
    users->InsertEndChild(user);

    if(this->database.SaveFile(this->options.findOption("database").c_str()) != tinyxml2::XML_SUCCESS)
        throw std::runtime_error(this->database.ErrorStr());

    if(mkdir((this->home_dir + '/' + name).c_str(), S_IRWXU | S_IRGRP) == -1)
        throw std::runtime_error("could not create user directory");
}

void Server::removeUser(const std::string& name)
{
    tinyxml2::XMLElement* user = this->findUser(name);
    if(user == nullptr)
        throw std::runtime_error("user not found");

    user->DeleteChildren();
    this->database.DeleteNode(user);

    if(this->database.SaveFile(this->options.findOption("database").c_str()) != tinyxml2::XML_SUCCESS)
        throw std::runtime_error(this->database.ErrorStr());
}
