#include <Server.hpp>
#include <Lock.hpp>
#include <Sha512.hpp>
#include <stdexcept>
#include <iostream>
#include <unistd.h>



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
    if(xmlerr != tinyxml2::XML_SUCCESS && xmlerr != tinyxml2::XML_ERROR_FILE_NOT_FOUND)
        throw std::runtime_error(this->database.ErrorStr());

    int port = std::stoi(this->options.findOption("port"));
    if(port < 0 || port > 65535)
        throw std::runtime_error("invalid port");
    this->listener.create(Socket::Tcp);
    this->listener.bind(port);
    this->listener.listen();
}



void Server::loopAcceptConn()
{
    PgpSocket sock;
    IpAddress ip;
    uint16_t port;

    while(this->listener.accept(ip, port, sock))
    {
        this->mutex.lock();

        if(this->listener.isValid() == false)
            break;

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

        this->mutex.unlock();
    }
}



void Server::loopInterface()
{
    std::string cmd;
    bool must_exit = false;

    while(must_exit == false && std::getline(std::cin, cmd))
    {
        if(cmd == "exit")
            must_exit = true;
        else
        {

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



void Server::handleClient(std::size_t index)
{
    Client& client = (*this->clients[index]);

    std::string buffer;
    std::size_t size;

    // TODO: Public key exchange

    // Username
    size = client.sock.recvString(buffer);

    tinyxml2::XMLElement* users = this->database.FirstChildElement("users");
    if(users == nullptr)
        client.sock.send8(0);

    tinyxml2::XMLElement* user_info = users->FirstChildElement(buffer.c_str());
    if(user_info == nullptr)
        client.sock.send8(0);

    client.sock.send8(1);

    // Password
    size = client.sock.recvString(buffer);

    const char* dbpasshash = user_info->FirstChildElement(buffer.c_str())->GetText();
    char* recvpasshash[Sha512::DIGEST_SIZE];
    Sha512(buffer.c_str(), size).finish(recvpasshash);

    if(memcmp(dbpasshash, recvpasshash, Sha512::DIGEST_SIZE) != 0)
        client.sock.send8(0);

    client.sock.send8(1);

    // Loop
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
                client.mutex.lock();

                // Command from main thread
                if(FD_ISSET(client.pipe.getReadFD(), &fdset))
                {
                    uint8_t code;
                    client.pipe.read(&code, sizeof(code));

                    switch(code)
                    {
                    case 0: must_exit = true; break;
                    }
                }

                // Communication with the client
                if(must_exit == true)
                {
                    client.sock.send8(-1);
                }
                else if(FD_ISSET(client.sock.getFD(), &fdset))
                {
                    size = client.sock.recvString(buffer);
                    std::vector<std::string> cmd = this->parseClientCommand(buffer.c_str());

                    if(cmd.size() != 0)
                    {
                        must_exit = this->executeClientCommand(cmd);
                    }

                    client.sock.send8(0);
                }

                client.mutex.unlock();
            }
        }
        catch(Socket::ReceiveError& e)
        {}
        catch(...)
        {}
    }
}

std::vector<std::string> Server::parseClientCommand(const char* buffer)
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
bool Server::executeClientCommand(std::vector<std::string>& cmd)
{
    if(cmd[0] == "exit")
        return false;

    return true;
}
