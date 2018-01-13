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
    class ExitEvent {};



    void init(int argc, char** argv);
    void loopAcceptConn();
    void loopInterface();
    void free();

    void initializeOptions();
    void parseArgs(int argc, char** argv);
    std::vector<std::string> parseCommand(const char* buffer);

    void handleClient(std::size_t index);
    bool handleClientInit(Client& client);

    bool executeServerCommand(Client& client);
    int executeClientCommand(const std::vector<std::string>& cmd, int stdinfd, int stdoutfd, int stderrfd, bool async);

    void addUser(const std::string& name, const std::string& password);
    tinyxml2::XMLElement* findUser(const char* name);



private:
    CommandLine options;
    tinyxml2::XMLDocument database;

    Socket listener;

    std::vector<Thread*> handlers;
    std::vector<Client*> clients;
    Mutex mutex;
};

#endif
