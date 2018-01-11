#include <Server.hpp>
#include <Lock.hpp>
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
    Thread runner(&Server::loopClients, this);
    this->free();
}



void Server::init(int argc, char** argv)
{
    this->parse(argc, argv);
    if(this->users_db.LoadFile(this->options.findOption("usersdb").c_str()) != tinyxml2::XML_SUCCESS)
        throw std::runtime_error("invalid users database XML file");

    int port = std::stoi(this->options.findOption("port"));
    if(port < 0 || port > 65535)
        throw std::runtime_error("invalid port");
    this->listener.create(Socket::Tcp);
    this->listener.bind(port);
    this->listener.listen();
}



void Server::loopClients()
{
    PgpSocket sock;
    IpAddress ip;
    uint16_t port;

    while(this->listener.accept(ip, port, sock))
    {
        this->mutex.lock();

        this->clients.push_back(new Client(std::move(sock), ip, port));
        this->handlers.push_back(new Thread(Server::handleClient, this->clients.size() - 1));

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
    this->clients.clear();
}



void Server::initializeOptions()
{
    this->options.addOption('u', "usersdb", "users.xml");
    this->options.addOption('p', "port", "1100");
}
void Server::parse(int argc, char** argv)
{
    this->options.parse(argc, argv);
}



void Server::handleClient(std::size_t index)
{
    ((void)index);
}
