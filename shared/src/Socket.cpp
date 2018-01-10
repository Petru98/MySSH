#include <Socket.hpp>
#include <unistd.h>

Socket::Socket() : sock(-1), family(INet)
{}
Socket::Socket(Socket&& that) : sock(that.sock), family(that.family)
{}
Socket::Socket(int sock, int family) : sock(sock), family(family)
{}
Socket::Socket(Protocols protocol, int family, int flags) : Socket()
{
    this->create(protocol, family, flags);
}
Socket::~Socket()
{
    this->close();
}



void Socket::create(Protocols protocol, int family, int flags)
{
    if(this->isValid())
        throw AlreadyCreatedError();

    this->sock = socket(family, protocol | flags, 0);
    if(this->sock == -1)
        throw CreateError();
}
void Socket::close()
{
    if(this->isValid())
    {
        ::close(this->sock);
        this->sock = -1;
    }
}
bool Socket::isValid() const
{
    return this->sock != -1;
}



void Socket::bind(uint16_t port, IpAddress address)
{
    sockaddr_in addr = {};
    addr.sin_family = this->family;
    addr.sin_addr.s_addr = htonl(address.toInt());
    addr.sin_port = htons(port);

    if(::bind(this->sock, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == -1)
        throw BindError();
}
void Socket::listen(int max_pending_connections)
{
    if(::listen(this->sock, max_pending_connections) == -1)
        throw ListenError();
}
bool Socket::accept(IpAddress& address, uint16_t& port, Socket& sock)
{
    sockaddr_in addr;
    socklen_t len = sizeof(addr);
    int new_sock = ::accept(this->sock, reinterpret_cast<sockaddr*>(&addr), &len);

    if(new_sock == -1)
    {
        if(errno == EWOULDBLOCK || errno == EAGAIN)
            return false;
        throw AcceptError();
    }

    address = ntohl(addr.sin_addr.s_addr);
    port = ntohs(addr.sin_port);
    sock.close();
    sock.sock = new_sock;
    sock.family = addr.sin_family;
    return true;
}
bool Socket::connect(IpAddress address, uint16_t port)
{
    sockaddr_in addr = {};
    addr.sin_family = this->family;
    addr.sin_addr.s_addr = htonl(address.toInt());
    addr.sin_port = htons(port);

    if(::connect(this->sock, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == -1)
    {
        if(errno == EINPROGRESS)
            return false;
        throw ConnectError();
    }

    return true;
}



std::size_t Socket::send(const void* data, std::size_t size, int flags)
{
    if(data == nullptr)
        return 0;

    data = this->onSend(reinterpret_cast<const uint8_t*>(data), size);
    ssize_t sent = ::send(this->sock, data, size, flags);

    if(sent == -1)
    {
        if(errno == EWOULDBLOCK || errno == EAGAIN)
            return 0;
        throw SendError();
    }

    return sent;
}
std::size_t Socket::recv(void* buffer, std::size_t size, int flags)
{
    if(buffer == nullptr)
        return 0;

    ssize_t received = ::recv(this->sock, buffer, size, flags);
    if(received == -1)
    {
        if(errno == EWOULDBLOCK || errno == EAGAIN)
            return 0;
        throw ReceiveError();
    }

    this->onReceive(reinterpret_cast<uint8_t*>(buffer), size);
    return size;
}
