#ifndef INCLUDED_SERVER_HPP
#define INCLUDED_SERVER_HPP

#include <Client.hpp>
#include <CommandLine.hpp>
#include <Thread.hpp>
#include <tinyxml2.h>
#include <vector>

class Server
{
public:
    Server();
    ~Server();

    void run(int argc, char** argv);

private:
    void init(int argc, char** argv);
    void free();

    void initializeOptions();
    void parse(int argc, char** argv);



private:
    CommandLine options;
    tinyxml2::XMLDocument users_db;

    Socket listener;

    std::vector<Thread> handlers;
    std::vector<Client> clients;
    Mutex clients_mutex;
};

#endif
