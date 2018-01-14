#ifndef INCLUDED_CLIENT_HPP
#define INCLUDED_CLIENT_HPP

#include <Server.hpp>
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
    CryptoPP::RSA::PrivateKey private_key;
    CryptoPP::RSA::PublicKey public_key;

    Server server;
};

#endif
