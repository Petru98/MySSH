#ifndef INCLUDED_SERVER_HPP
#define INCLUDED_SERVER_HPP

#include <PgpSocket.hpp>



class Server
{
public:
    Server();
    ~Server();



private:
    CryptoPP::RSA::PublicKey publickey;
    PgpSocket sock;
    IpAddress ip;
    uint16_t port;
};

#endif
