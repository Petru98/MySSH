#include <Server.hpp>
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

void Server::run()
{

}

void Server::free()
{

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
