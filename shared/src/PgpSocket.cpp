#include <PgpSocket.hpp>
#include <algorithm>



void PgpSocket::onSend(const uint8_t* data, std::size_t size)
{
    this->buffer.assign(data, data + size);
}

void PgpSocket::onReceive(uint8_t* data, std::size_t size)
{
    size = this->buffer.size();
    std::copy_n(this->buffer.data(), size, data);
}
