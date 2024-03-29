#include <Socket.hpp>
#include <unistd.h>
#include <cstring>
#include <limits>
#include <cassert>
#include <algorithm>



Socket::Socket() : FileDescriptor(-1), family(INet)
{}
Socket::Socket(Socket&& that) : FileDescriptor(that.fd), family(that.family)
{
    that.fd = -1;
}
Socket::Socket(int fd, int family) : FileDescriptor(fd), family(family)
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
        throw CreateError("the socket object was already created");

    this->fd = socket(family, protocol | flags, 0);
    if(this->fd == -1)
        throw CreateError(errno, "could not create socket");
}
void Socket::close()
{
    if(this->isValid())
    {
        ::close(this->fd);
        this->fd = -1;
    }
}
bool Socket::isValid() const
{
    return this->fd != -1;
}



uint16_t Socket::getPort() const
{
    sockaddr_in sin;
    socklen_t len = sizeof(sin);

    if(getsockname(this->fd, reinterpret_cast<sockaddr*>(&sin), &len) == -1)
        throw GetPortError(errno, "could not get the socket's port");
    return ntohs(sin.sin_port);
}



void Socket::bind(uint16_t port, IpAddress address)
{
    sockaddr_in addr = {};
    addr.sin_family = this->family;
    addr.sin_addr.s_addr = htonl(address.toInt());
    addr.sin_port = htons(port);

    if(::bind(this->fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == -1)
        throw BindError(errno, "could not bind the socket");
}
void Socket::listen(int max_pending_connections)
{
    if(::listen(this->fd, max_pending_connections) == -1)
        throw ListenError(errno, "could not listen to port with socket");
}
bool Socket::accept(IpAddress& address, uint16_t& port, Socket& sock)
{
    sockaddr_in addr;
    socklen_t len = sizeof(addr);
    int new_sock = ::accept(this->fd, reinterpret_cast<sockaddr*>(&addr), &len);

    if(new_sock == -1)
    {
        if(errno == EWOULDBLOCK || errno == EAGAIN)
            return false;
        throw AcceptError(errno, "could not accept a connection with socket");
    }

    address = ntohl(addr.sin_addr.s_addr);
    port = ntohs(addr.sin_port);
    sock.close();
    sock.fd = new_sock;
    sock.family = addr.sin_family;
    return true;
}
bool Socket::connect(IpAddress address, uint16_t port)
{
    sockaddr_in addr = {};
    addr.sin_family = this->family;
    addr.sin_addr.s_addr = htonl(address.toInt());
    addr.sin_port = htons(port);

    if(::connect(this->fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == -1)
    {
        if(errno == EINPROGRESS)
            return false;
        throw ConnectError(errno, "could not connect with socket");
    }

    return true;
}



void Socket::sendRaw(const void* data, std::size_t size, int flags)
{
    assert(data != nullptr || size == 0);

    while(size > 0)
    {
        ssize_t sent = ::send(this->fd, data, size, flags);
        if(sent == -1)
        {
            if(errno != EWOULDBLOCK && errno != EAGAIN)
                throw SendError(errno, "could not send data with socket");
        }
        else
        {
            data = reinterpret_cast<const uint8_t*>(data) + sent;
            size -= sent;
        }
    }
}
void Socket::recvRaw(void* data, std::size_t size, int flags)
{
    assert(data != nullptr || size == 0);

    while(size > 0)
    {
        ssize_t received = ::recv(this->fd, data, size, flags);
        if(received == -1)
        {
            if(errno != EWOULDBLOCK && errno != EAGAIN)
                throw ReceiveError(errno, "could not receive data with socket");
        }
        else
        {
            data = reinterpret_cast<uint8_t*>(data) + received;
            size -= received;
        }
    }
}



void Socket::sendSize(std::size_t data, int flags)
{
    typedef uint32_t fixed_size_t;
    if(sizeof(std::size_t) > sizeof(fixed_size_t) && data > std::numeric_limits<fixed_size_t>::max())
        throw SendError("could not send size because its value is too big");

    const fixed_size_t size = hton(static_cast<fixed_size_t>(data));
    return this->sendRaw(&size, sizeof(size), flags);
}
std::size_t Socket::recvSize(int flags)
{
    typedef uint32_t fixed_size_t;
    fixed_size_t size;
    this->recvRaw(&size, sizeof(size), flags);

    if(sizeof(std::size_t) < sizeof(fixed_size_t) && std::numeric_limits<std::size_t>::max() < size)
        throw ReceiveError("could not receive size because its value is too big");
    return static_cast<std::size_t>(ntoh(size));
}



void Socket::sendUnprocessed(const void* data, std::size_t size, int flags)
{
    assert(data != nullptr || size == 0);
    if(size == 0)
        return;

    this->sendSize(size, flags);
    return this->sendRaw(data, size, flags);
}
std::size_t Socket::recvUnprocessed(void* data, std::size_t size, int flags)
{
    assert(data != nullptr || size == 0);
    if(size == 0)
        return 0;

    const std::size_t packet_size = this->recvSize(flags);
    if(packet_size > size)
        throw ReceiveError("packet size is bigger than the buffer size");

    this->recvRaw(data, packet_size, flags);
    return packet_size;
}



void Socket::send(const void* data, std::size_t size, int flags)
{
    assert(data != nullptr || size == 0);
    if(size == 0)
        return;

    this->onSend(reinterpret_cast<const uint8_t*>(data), size);
    this->sendSize(size, flags);
    return this->sendUnprocessed(this->buffer.data(), this->buffer.size(), flags);
}
std::size_t Socket::recv(void* data, std::size_t size, int flags)
{
    assert(data != nullptr || size == 0);
    if(size == 0)
        return 0;

    const std::size_t content_size = this->recvSize(flags);
    if(content_size > size)
        throw ReceiveError("packet content size is bigger than the buffer size");

    this->recvUnprocessed(this->buffer, flags);
    this->onReceive(reinterpret_cast<uint8_t*>(data), this->buffer.size());
    return content_size;
}



void Socket::send8(uint8_t data, int flags)
{
    data = hton(data);
    this->send(&data, sizeof(data), flags);
}
void Socket::send16(uint16_t data, int flags)
{
    data = hton(data);
    this->send(&data, sizeof(data), flags);
}
void Socket::send32(uint32_t data, int flags)
{
    data = hton(data);
    this->send(&data, sizeof(data), flags);
}
void Socket::sendString(const char* data, std::size_t length, int flags)
{
    assert(data != nullptr);
    if(length == 0)
        length = strlen(data);

    this->send(data, length, flags);
}
void Socket::sendString(const std::string& data, int flags)
{
    return this->sendString(data.c_str(), data.length(), flags);
}

uint8_t Socket::recv8(int flags)
{
    uint8_t buffer;
    this->recv(&buffer, sizeof(buffer), flags);
    return ntoh(buffer);
}
uint16_t Socket::recv16(int flags)
{
    uint16_t buffer;
    this->recv(&buffer, sizeof(buffer), flags);
    return ntoh(buffer);
}
uint32_t Socket::recv32(int flags)
{
    uint32_t buffer;
    this->recv(&buffer, sizeof(buffer), flags);
    return ntoh(buffer);
}
std::size_t Socket::recvString(char* data, std::size_t size, int flags)
{
    assert(data != nullptr || size == 0);
    if(size == 0)
        return 0;

    this->recv(data, size - 1, flags);
    data[size] = '\0';
    return size;
}
std::size_t Socket::recvString(std::string& data, int flags)
{
    return this->recv(data, flags);
}



void Socket::onSend(const uint8_t* data, std::size_t size)
{
    this->buffer.assign(data, data + size);
}

void Socket::onReceive(uint8_t* data, std::size_t size)
{
    size = this->buffer.size();
    std::copy_n(this->buffer.data(), size, data);
}
