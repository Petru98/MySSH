#include <Client.hpp>
#include <Lock.hpp>
#include <cstring>
#include <stdexcept>
#include <limits>



Client::Client(PgpSocket&& sock, IpAddress ip, uint16_t port) : mutex(), pipe(), sock(std::move(sock)), ip(ip), port(port)
{}
Client::~Client()
{
    this->disconnect();
}



void Client::disconnect()
{
    Lock lock(this->mutex);

    if(this->sock.isValid())
    {
        this->sock.close();
    }
}

bool Client::isConnected() const
{
    return this->sock.isValid();
}