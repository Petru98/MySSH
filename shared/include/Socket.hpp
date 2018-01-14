#ifndef INCLUDED_SOCKET_HPP
#define INCLUDED_SOCKET_HPP

#include <sys/socket.h>
#include <string>
#include <vector>
#include "IpAddress.hpp"
#include "FileDescriptor.hpp"



class Socket : public FileDescriptor
{
public:
    enum Families
    {
        Local = AF_LOCAL,
        Unix  = AF_UNIX,
        INet  = AF_INET
    };

    enum Protocols
    {
        Tcp = SOCK_STREAM,
        Udp = SOCK_DGRAM
    };

    enum Flags
    {
        NonBlock = SOCK_NONBLOCK,
        CloseOnExec = SOCK_CLOEXEC
    };



    class Error : public std::exception
    {};

    class NotImplemented : public Error
        {public: virtual const char* what() {return "Socket method is not implemented";}};

    class AlreadyCreatedError : public Error
        {public: virtual const char* what() {return "Socket was already created";}};

    class CreateError : public Error
        {public: virtual const char* what() {return "Could not create socket";}};

    class GetPortError : public Error
        {public: virtual const char* what() {return "Could not find the socket's port";}};

    class BindError : public Error
        {public: virtual const char* what() {return "Could not bind socket";}};

    class ListenError : public Error
        {public: virtual const char* what() {return "Could not listen socket";}};

    class AcceptError : public Error
        {public: virtual const char* what() {return "Could not accept socket";}};

    class ConnectError : public Error
        {public: virtual const char* what() {return "Could not connect";}};

    class SendError : public Error
        {public: virtual const char* what() {return "Could not send data";}};

    class ReceiveError : public Error
        {public: virtual const char* what() {return "Could not receive data";}};



    Socket();
    Socket(Socket&& that);
    Socket(int sock, int family);
    Socket(Protocols protocol, int family = INet, int flags = 0);
    virtual ~Socket();

    void create(Protocols protocol, int family = INet, int flags = 0);
    void close();
    bool isValid() const;

    IpAddress getIP() const;
    uint16_t getPort() const;

    void bind(uint16_t port, IpAddress address = IpAddress::Any);
    void listen(int max_pending_connections = 0);
    bool accept(IpAddress& address, uint16_t& port, Socket& sock);
    bool connect(IpAddress address, uint16_t port);



    void sendRaw(const void* data, std::size_t size, int flags = 0);
    void sendSize(std::size_t data, int flags = 0);
    void sendUnprocessed(const void* data, std::size_t size, int flags = 0);
    void send(const void* data, std::size_t size, int flags = 0);

    void send8(uint8_t data, int flags = 0);
    void send16(uint16_t data, int flags = 0);
    void send32(uint32_t data, int flags = 0);
    void sendString(const char* data, std::size_t length = 0, int flags = 0);
    void sendString(const std::string& data, int flags = 0);



    void        recvRaw(void* data, std::size_t size, int flags = 0);
    std::size_t recvSize(int flags = 0);
    std::size_t recvUnprocessed(void* data, std::size_t size, int flags = 0);
    std::size_t recv(void* data, std::size_t size, int flags = 0);

    uint8_t     recv8(int flags = 0);
    uint16_t    recv16(int flags = 0);
    uint32_t    recv32(int flags = 0);
    std::size_t recvString(char* data, std::size_t size = 0, int flags = 0);
    std::size_t recvString(std::string& data, int flags = 0);



    template <typename Container>
    void sendUnprocessed(const Container& container, int flags = 0);
    template <typename Container>
    void send(const Container& container, int flags = 0);

    template <typename Container>
    std::size_t recvUnprocessed(Container& container, int flags = 0);
    template <typename Container>
    std::size_t recv(Container& container, int flags = 0);

protected:
    virtual void onSend(const uint8_t* data, std::size_t size);
    virtual void onReceive(uint8_t* data, std::size_t size);



protected:
    int family; ///< Protocol family
    std::vector<uint8_t> buffer; ///< Buffer used internally
};





template <typename Container>
void Socket::sendUnprocessed(const Container& container, int flags)
{
    return this->sendUnprocessed(container.data(), container.size(), flags);
}

template <typename Container>
void Socket::send(const Container& container, int flags)
{
    return this->send(container.data(), container.size(), flags);
}



template <typename Container>
std::size_t Socket::recvUnprocessed(Container& container, int flags)
{
    const std::size_t packet_size = this->recvSize(flags);
    container.resize(packet_size);
    this->recvRaw((void*)(container.data()), packet_size, flags);
    return packet_size;
}

template <typename Container>
std::size_t Socket::recv(Container& container, int flags)
{
    const std::size_t content_size = this->recvSize(flags);
    container.resize(content_size);

    this->recvUnprocessed(this->buffer, flags);
    this->onReceive((uint8_t*)(container.data()), container.size());
    return content_size;
}





// Non-member
template <typename T>
T ntoh(T x)
{
    T y = 0;
    for(unsigned i = 0; i < sizeof(x) / 2; ++i)
        y = ((x >> (8 * i)) & 0xff) | ((x >> (8 * (sizeof(x) - i - 1))) & 0xff);

    return y;
}
template <typename T>
T hton(T x)
{
    return ntoh(x);
}

#endif
