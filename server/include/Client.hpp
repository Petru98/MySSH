#ifndef INCLUDED_CLIENT_HPP
#define INCLUDED_CLIENT_HPP

#include <PgpSocket.hpp>
#include <Mutex.hpp>
#include <Pipe.hpp>
#include <cryptopp/rsa.h>



class Client
{
public:
    class DisconnectedError : std::exception
        {public: virtual const char* what() {return "client disconnected";}};



    Client(PgpSocket&& sock, IpAddress ip, uint16_t port);
    ~Client();

    void disconnect();
    bool isConnected() const;



public:
    Mutex mutex;
    CryptoPP::RSA::PublicKey key;
    Pipe pipe;
    PgpSocket sock;
    IpAddress ip;
    uint16_t port;
};

#endif
