#include <PgpSocket.hpp>

const void* PgpSocket::onSend(const uint8_t* data, std::size_t& size)
{
    return data;
    ((void)size);
}

void PgpSocket::onReceive(uint8_t* data, std::size_t& size)
{
    ((void)data);
    ((void)size);
}
