#include <ClientInfo.hpp>
#include <Lock.hpp>
#include <cstring>
#include <stdexcept>
#include <limits>



constexpr std::size_t ClientInfo::ERRMSG_MAX_SIZE;



ClientInfo::ClientInfo(PgpSocket&& sock, IpAddress ip, uint16_t port)
    : mutex(), publickey(), name(), home(), cwd("/"), server_pipe(), sock(std::move(sock)), ip(ip), port(port)
{}
ClientInfo::~ClientInfo()
{
    this->disconnect();
}



void ClientInfo::disconnect()
{
    Lock lock(this->mutex);

    if(this->sock.isValid())
    {
        uint8_t code = 0;
        this->server_pipe.write(&code, sizeof(code));
    }
}

bool ClientInfo::isConnected() const
{
    return this->sock.isValid();
}
