#include <Server.hpp>
#include <Lock.hpp>
#include <Sha512.hpp>
#include <stdexcept>
#include <iostream>



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
    this->parse(argc, argv);

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

    tinyxml2::XMLElement* user_info = users->FirstChildElement(buffer);
    if(user_info == nullptr)
        client.sock.send8(0);

    client.send8(1);

    // Password
    size = client.sock.recvString(buffer);

    const char* dbpasshash = user_info->FirstChildElement(buffer)->GetText();
    const char* recvpasshash = Sha512(buffer, size).finish(buffer);
    if(memcmp(dbpasshash, recvpasshash, 512 / 8) != 0)
        client.sock.send8(0);

    client.sock.send8(1);

    // Loop
    try
    {
        while(client.isConnected() && (size = client.sock.recvString(buffer)) > 0)
        {
            char** cmd = this->parseClientCommand(buffer);

            if(cmd == nullptr)
            {
                constexpr char msg[] = "server: error: could not parse the command\n";
                client.sock.send8(1);
                client.sock.sendString(msg, sizeof(msg) - 1);
            }
            else
            {

            }

            client.sock.send8(0);
        }
    }
    catch(Socket::ReceiveError& e)
    {}
    catch(...)
    {}
}
