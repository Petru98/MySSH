#include <Client.hpp>
#include <Lock.hpp>
#include <cstring>
#include <stdexcept>
#include <limits>



Client::Client(PgpSocket&& sock, IpAddress ip, uint16_t port) : mutex(), key(), pipe(), sock(std::move(sock)), ip(ip), port(port)
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
        uint8_t code = 0;
        this->pipe.write(&code, sizeof(code));
    }
}

bool Client::isConnected() const
{
    return this->sock.isValid();
}
