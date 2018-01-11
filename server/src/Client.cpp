#include <Client.hpp>
#include <Lock.hpp>

Client::Client(PgpSocket&& sock, IpAddress ip, uint16_t port) : mutex(), sock(std::move(sock)), ip(ip), port(port)
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



void Client::send(const void* data, std::size_t size)
{
    if(this->isConnected() == false)
        throw DisconnectedError();
    Lock lock(this->mutex);

    while(size > 0)
    {
        std::size_t sent = this->sock.send(data, size);
        data = reinterpret_cast<const uint8_t*>(data) + sent;
        size -= sent;
    }
}
void Client::recv(void* data, std::size_t size)
{
    Lock lock(this->mutex);

    while(size > 0)
    {
        std::size_t received = this->sock.recv(data, size);
        data = reinterpret_cast<uint8_t*>(data) + received;
        size -= received;
    }
}
