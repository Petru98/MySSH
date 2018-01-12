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
    void loopAcceptConn();
    void loopInterface();
    void free();

    void initializeOptions();
    void parseArgs(int argc, char** argv);

    void handleClient(std::size_t index);
    bool handleClientInit(Client& client);

    bool executeServerCommand(Client& client);

    std::vector<std::string> parseClientCommand(const char* buffer);
    bool executeClientCommand(std::vector<std::string>& cmd);



private:
    CommandLine options;
    tinyxml2::XMLDocument database;

    Socket listener;

    std::vector<Thread*> handlers;
    std::vector<Client*> clients;
    Mutex mutex;
};

#endif
