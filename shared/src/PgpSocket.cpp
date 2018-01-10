#include <PgpSocket.hpp>

const void* onSend(const uint8_t* data, std::size_t& size)
{
    return data;
    ((void)size);
}

void onReceive(uint8_t* data, std::size_t& size)
{
    ((void)data);
    ((void)size);
}
