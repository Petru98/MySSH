#ifndef INCLUDED_SERVERINFO_HPP
#define INCLUDED_SERVERINFO_HPP

#include <PgpSocket.hpp>



class ServerInfo
{
public:
    ServerInfo();
    ~ServerInfo();



public:
    CryptoPP::RSA::PublicKey publickey;
    PgpSocket sock;
    IpAddress ip;
    uint16_t port;
};

#endif





////////////////////////////////////////////////////////////////////////////////
/// \class ServerInfo
/// \ingroup client
////////////////////////////////////////////////////////////////////////////////
