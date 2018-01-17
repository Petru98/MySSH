#ifndef INCLUDED_CLIENT_HPP
#define INCLUDED_CLIENT_HPP

#include <ServerInfo.hpp>
#include <CommandLine.hpp>
#include <cryptopp/rsa.h>



class Client
{
public:
    Client();
    ~Client();

    void run(int argc, char** argv);

private:
    void initializeOptions();

    void init(int argc, char** argv);
    void loop();
    void free();



private:
    CommandLine options;
    CryptoPP::RSA::PrivateKey privatekey;
    CryptoPP::RSA::PublicKey publickey;

    ServerInfo server;
};

#endif





////////////////////////////////////////////////////////////////////////////////
/// \class Client
/// \ingroup client
////////////////////////////////////////////////////////////////////////////////
