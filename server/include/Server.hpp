#ifndef INCLUDED_SERVER_HPP
#define INCLUDED_SERVER_HPP

#include <ClientInfo.hpp>
#include <CommandLine.hpp>
#include <Thread.hpp>
#include <tinyxml2.h>
#include <vector>
#include <cryptopp/rsa.h>



class Server
{
public:
    Server();
    ~Server();

    void run(int argc, char** argv);

private:
    class ExitEvent {};



    void initializeOptions();

    void init(int argc, char** argv);
    void free();

    void loopAcceptConn();
    void handleClient(std::size_t index);
    bool handleClientInit(ClientInfo& client);
    int  executeClientCommand(std::size_t index, const std::vector<std::string>& cmd, int stdinfd, int stdoutfd, int stderrfd, bool async);
    void setClientCWD(std::size_t index, const std::string& dir);

    void loopInterface();
    bool executeServerCommand(ClientInfo& client);
    std::vector<std::string> parseCommand(const char* buffer);

    tinyxml2::XMLElement* findUser(const char* name);
    tinyxml2::XMLElement* findUser(const std::string& name);
    void addUser(const std::string& name, const std::string& password);
    void removeUser(const std::string& name);



private:
    CommandLine options;
    tinyxml2::XMLDocument database;
    CryptoPP::RSA::PublicKey publickey;
    CryptoPP::RSA::PrivateKey privatekey;

    Socket listener;

    std::vector<Thread*> handlers;
    std::vector<ClientInfo*> clients;
    std::string home_dir;
    std::string cwd;
    Mutex mutex;
};

#endif





////////////////////////////////////////////////////////////////////////////////
/// \class Server
/// \ingroup server
////////////////////////////////////////////////////////////////////////////////
