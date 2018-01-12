#include <Socket.hpp>
#include <unistd.h>
#include <cstring>
#include <limits>



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
        throw AlreadyCreatedError();

    this->fd = socket(family, protocol | flags, 0);
    if(this->fd == -1)
        throw CreateError();
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



void Socket::bind(uint16_t port, IpAddress address)
{
    sockaddr_in addr = {};
    addr.sin_family = this->family;
    addr.sin_addr.s_addr = htonl(address.toInt());
    addr.sin_port = htons(port);

    if(::bind(this->fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == -1)
        throw BindError();
}
void Socket::listen(int max_pending_connections)
{
    if(::listen(this->fd, max_pending_connections) == -1)
        throw ListenError();
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
        throw AcceptError();
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
        throw ConnectError();
    }

    return true;
}



void Socket::sendSize(std::size_t data, int flags)
{
    if(data > std::numeric_limits<uint32_t>::max())
        throw std::runtime_error("could not send size because it is too big");

    const uint32_t fixed_data = hton(static_cast<uint32_t>(data));
    const uint8_t* data_ptr = reinterpret_cast<const uint8_t*>(&fixed_data);
    std::size_t data_remaining = sizeof(fixed_data);

    while(data_remaining > 0)
    {
        ssize_t sent = ::send(this->fd, &data_ptr, data_remaining, flags);
        if(sent == -1)
        {
            if(errno != EWOULDBLOCK && errno != EAGAIN)
                throw SendError();
        }
        else
        {
            data_ptr = data_ptr + sent;
            data_remaining -= sent;
        }
    }
}
std::size_t Socket::recvSize(int flags)
{
    uint32_t fixed_buffer;
    uint8_t* buffer_ptr = reinterpret_cast<uint8_t*>(&fixed_buffer);
    std::size_t buffer_remaining = sizeof(fixed_buffer);

    while(buffer_remaining > 0)
    {
        ssize_t received = ::recv(this->fd, &buffer_ptr, buffer_remaining, flags);
        if(received == -1)
        {
            if(errno != EWOULDBLOCK && errno != EAGAIN)
                throw ReceiveError();
        }
        else
        {
            buffer_ptr = buffer_ptr + received;
            buffer_remaining -= received;
        }
    }

    return static_cast<std::size_t>(ntoh(fixed_buffer));
}



void Socket::send(const void* data, std::size_t size, int flags)
{
    if(data == nullptr || size == 0)
        return;

    const uint32_t size_content = static_cast<uint32_t>(size);
    this->onSend(reinterpret_cast<const uint8_t*>(data), size);
    const uint32_t size_packet = static_cast<uint32_t>(this->buffer.size());

    // Send the sizes of the content and packet
    this->sendSize(size_content, flags);
    this->sendSize(size_packet, flags);

    // Send the packet
    const uint8_t* it = this->buffer.data();

    while(size > 0)
    {
        ssize_t sent = ::send(this->fd, it, size, flags);
        if(sent == -1)
        {
            if(errno != EWOULDBLOCK && errno != EAGAIN)
                throw SendError();
        }
        else
        {
            it += sent;
            size -= sent;
        }
    }
}
std::size_t Socket::recv(void* buffer, std::size_t size, int flags)
{
    if(buffer == nullptr || size == 0)
        return 0;

    // Receive the size of the content and packet
    const std::size_t size_content = this->recvSize();
    const std::size_t size_packet = this->recvSize();

    if(size_content > size)
        throw ReceiveError();

    // Receive the packet
    this->buffer.resize(size_packet);
    uint8_t* it = this->buffer.data();
    size = size_packet;

    while(size > 0)
    {
        ssize_t received = ::recv(this->fd, it, size, flags);
        if(received == -1)
        {
            if(errno != EWOULDBLOCK && errno != EAGAIN)
                throw ReceiveError();
        }
        else
        {
            it += received;
            size -= received;
        }
    }

    this->onReceive(reinterpret_cast<uint8_t*>(buffer), size_packet);
    return size_content;
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
    if(data == nullptr || (*data) == '\0')
        return;
    if(length == 0)
        length = strlen(data);
    if(length > std::numeric_limits<uint32_t>::max())
        throw std::runtime_error("could not send string because it is too big");

    this->send32(static_cast<uint32_t>(length), flags);
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
std::size_t Socket::recvString(char* buffer, std::size_t size, int flags)
{
    const uint32_t length = this->recv32(flags);

    uint32_t receive_length = length;
    if(size > 0 && receive_length > size - 1)
        receive_length = static_cast<uint32_t>(size - 1);

    this->recv(buffer, receive_length, flags);
    buffer[receive_length] = '\0';
    return length - receive_length;
}
std::size_t Socket::recvString(std::string& buffer, std::size_t size, int flags)
{
    const uint32_t length = this->recv32(flags);

    uint32_t receive_length = length;
    if(size > 0 && receive_length > size - 1)
        receive_length = static_cast<uint32_t>(size - 1);

    buffer.resize(receive_length);
    this->recv(const_cast<char*>(buffer.c_str()), receive_length, flags);
    return length - receive_length;
}
