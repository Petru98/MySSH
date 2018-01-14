#ifndef INCLUDED_PGPSOCKET_HPP
#define INCLUDED_PGPSOCKET_HPP

#include "Socket.hpp"



class PgpSocket : public Socket
{
public:
    using Socket::Socket;

private:
    virtual void onSend(const uint8_t* data, std::size_t size) override;
    virtual void onReceive(uint8_t* data, std::size_t size) override;
};

#endif
