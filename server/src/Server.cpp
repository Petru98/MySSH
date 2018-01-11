#include <Server.hpp>
#include <Lock.hpp>
#include <stdexcept>

Server::Server()
{
    this->initializeOptions();
}
Server::~Server()
{}



void Server::init(int argc, char** argv)
{
    this->parse(argc, argv);
    if(this->users_db.LoadFile(this->options.findOption("usersdb").c_str()) != tinyxml2::XML_SUCCESS)
        throw std::runtime_error("Invalid users database XML file");
}



void Server::run(int argc, char** argv)
{
    this->init(argc, argv);
    this->free();
}



void Server::free()
{
    Lock lock(this->clients_mutex);
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
