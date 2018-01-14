#include <Server.hpp>
#include <Lock.hpp>
#include <Sha512.hpp>
#include <Logging.hpp>
#include <memory.hpp>
#include <CommandTree.hpp>
#include <stdexcept>
#include <iostream>
#include <unistd.h>
#include <sys/wait.h>



Server::Server()
{
    this->initializeOptions();
}
Server::~Server()
{}



void Server::run(int argc, char** argv)
{
    this->init(argc, argv);

    Thread runner(&Server::loopAcceptConn, this);
    runner.detach();
    this->loopInterface();

    this->free();
}



void Server::init(int argc, char** argv)
{
    this->parseArgs(argc, argv);
    tinyxml2::XMLError xmlerr = this->database.LoadFile(this->options.findOption("database").c_str());

    if(xmlerr == tinyxml2::XML_ERROR_FILE_NOT_FOUND)
    {
        tinyxml2::XMLElement* xml = this->database.NewElement("xml");
        if(xml == nullptr)
            throw std::runtime_error(this->database.ErrorStr());
        tinyxml2::XMLElement* users = this->database.NewElement("users");
        if(users == nullptr)
            throw std::runtime_error(this->database.ErrorStr());

        if(xml->InsertEndChild(users) == nullptr)
        {
            this->database.DeleteNode(users);
            this->database.DeleteNode(xml);
            throw std::runtime_error(this->database.ErrorStr());
        }
    }
    else if(xmlerr != tinyxml2::XML_SUCCESS)
        throw std::runtime_error(this->database.ErrorStr());

    int port = std::stoi(this->options.findOption("port"));
    if(port < 0 || port > 65535)
        throw std::runtime_error("invalid port");
    this->listener.create(Socket::Tcp, Socket::INet, SOCK_CLOEXEC);
    this->listener.bind(port);
    this->listener.listen();
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

void Server::loopInterface()
{
    std::string line_str;
    bool must_exit = false;

    while(must_exit == false && std::cout << "> " && std::getline(std::cin, line_str))
    {
        std::vector<std::string> cmd = this->parseCommand(line_str.c_str());

        if(cmd.size() > 0)
        {
            if(cmd[0] == "exit")
                must_exit = true;
            else if(cmd[0] == "adduser")
            {
                if(cmd.size() - 1 != 2)
                    error("invalid number of arguments");
                else
                {
                    try
                        {this->addUser(cmd[1], cmd[2]);}
                    catch(std::exception& e)
                        {error(e.what());}
                }
            }
            else
                error("unknown command");
        }
    }
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



void Server::initializeOptions()
{
    this->options.addOption('d', "database", "db.xml");
    this->options.addOption('p', "port", "1100");
}

void Server::parseArgs(int argc, char** argv)
{
    this->options.parse(argc, argv);
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



void Server::handleClient(std::size_t index)
{
    Client& client = (*this->clients[index]);
    if(this->handleClientInit(client) == false)
        return;

    // Loop
    std::string buffer;
    bool must_exit = false;

    while(must_exit == false)
    {
        fd_set fdset;
        FD_ZERO(&fdset);
        FD_SET(client.sock.getFD(), &fdset);
        FD_SET(client.pipe.getReadFD(), &fdset);

        try
        {
            if(select(std::max(client.sock.getFD(), client.pipe.getReadFD()) + 1, &fdset, nullptr, nullptr, nullptr) > 0)
            {
                Lock(client.mutex);

                // Command from server thread
                if(FD_ISSET(client.pipe.getReadFD(), &fdset))
                    must_exit = this->executeServerCommand(client) == false;

                // Communication with the client
                if(must_exit == true)
                {
                    client.sock.send8(-1);
                }
                else if(FD_ISSET(client.sock.getFD(), &fdset))
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
        catch(Socket::ReceiveError& e)
        {
            must_exit = true;
        }
        catch(Socket::SendError& e)
        {
            must_exit = true;
        }
        catch(...)
        {
            must_exit = true;
        }
    }

    client.sock.close();
}

bool Server::handleClientInit(Client& client)
{
    Lock(client.mutex);

    std::string buffer;
    std::size_t size;

    // TODO: Public key exchange

    // Username
    size = client.sock.recvString(buffer);

    tinyxml2::XMLElement* user_info = this->findUser(buffer.c_str());
    if(user_info == nullptr)
    {
        client.sock.send8(0);
        return false;
    }

    client.sock.send8(1);

    // Password
    size = client.sock.recvString(buffer);

    char recvpasshash[Sha512::DIGEST_SIZE];
    char dbpasshash[sizeof(recvpasshash)];
    fromHex(user_info->FirstChildElement(buffer.c_str())->GetText(), dbpasshash);
    Sha512(buffer.c_str(), size).finish(recvpasshash);

    if(memcmp(dbpasshash, recvpasshash, Sha512::DIGEST_SIZE) != 0)
    {
        client.sock.send8(0);
        return false;
    }

    client.sock.send8(1);
    return true;
}

bool Server::executeServerCommand(Client& client)
{

    uint8_t code;
    client.pipe.read(&code, sizeof(code));

    switch(code)
    {
    case 0: return false;
    }

    return true;
}

int Server::executeClientCommand(std::size_t index, const std::vector<std::string>& cmd, int stdinfd, int stdoutfd, int stderrfd, bool async)
{
    if(cmd[0] == "exit")
        throw ExitEvent();

    Client& client = (*this->clients[index]);
    int exit_code = 0;
    pid_t pid = fork();

    if(pid == -1)
    {
        constexpr char msg[] = "server: internal error";
        client.sock.send8(1);
        client.sock.sendString(msg, sizeof(msg) - 1);
        exit_code = -1;
    }
    else if(pid == 0)
    {
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
            exit(-1);
        }

        if(stdinfd != STDIN_FILENO)
        {
            ::close(STDIN_FILENO);
            if(dup2(stdinfd, STDIN_FILENO) == -1)
                exit(errno);
        }
        if(stdoutfd != STDIN_FILENO)
        {
            ::close(STDOUT_FILENO);
            if(dup2(stdoutfd, STDOUT_FILENO) == -1)
                exit(errno);
        }
        if(stderrfd != STDERR_FILENO)
        {
            ::close(STDIN_FILENO);
            if(dup2(stderrfd, STDERR_FILENO) == -1)
                exit(errno);
        }

        execvp(argv[0], argv);
        exit(errno);
    }
    else
    {
        if(async == false)
        {
            int status;
            waitpid(pid, &status, 0);

            if(WIFEXITED(status))
            {
                exit_code = WEXITSTATUS(status);
                if(exit_code != 0)
                {
                    constexpr char msg[] = "server: executable not found, permission denied or internal error";
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





void Server::addUser(const std::string& name, const std::string& password)
{
    tinyxml2::XMLElement* xml = this->database.RootElement();
    assert(xml != nullptr);
    tinyxml2::XMLElement* users = xml->FirstChildElement("users");
    assert(users != nullptr);


    tinyxml2::XMLElement* user = this->findUser(name.c_str());
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



    char password_hash[Sha512::DIGEST_SIZE];
    char password_hash_hex[sizeof(password_hash) * 2 + 1];
    Sha512(password.c_str(), password.length()).finish(password_hash);
    toHex(password_hash, password_hash_hex);

    user_name->SetText(name.c_str());
    user_password->SetText(password_hash_hex);
    user->InsertEndChild(user_name);
    user->InsertEndChild(user_password);
    users->InsertEndChild(user);

    this->database.SaveFile(this->options.findOption("database").c_str());
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
