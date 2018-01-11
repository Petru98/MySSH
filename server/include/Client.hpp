#ifndef INCLUDED_CLIENT_HPP
#define INCLUDED_CLIENT_HPP

#include <PgpSocket.hpp>
#include <Mutex.hpp>

class Client
{
public:
    class DisconnectedError : std::exception
        {public: virtual const char* what() {return "client disconnected";}};



    Client(PgpSocket&& sock, IpAddress ip, uint16_t port);
    ~Client();

    void disconnect();
    bool isConnected() const;

    void send(const void* data, std::size_t size);
    void recv(void* data, std::size_t size);



private:
    Mutex mutex;
    PgpSocket sock;
    IpAddress ip;
    uint16_t port;
};

#endif
