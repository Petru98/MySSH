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
    CryptoPP::RSA::PublicKey publickey;
    std::string name;
    std::string home;
    std::string cwd;

    Pipe server_pipe;
    PgpSocket sock;
    IpAddress ip;
    uint16_t port;

    static constexpr std::size_t ERRMSG_MAX_SIZE = 256;
    char errmsg[ERRMSG_MAX_SIZE];
};

#endif
