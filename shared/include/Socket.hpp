#ifndef INCLUDED_SOCKET_HPP
#define INCLUDED_SOCKET_HPP

#include <sys/socket.h>
#include <arpa/inet.h>
#include <string>
#include <vector>
#include <cassert>
#include <system_error>
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



    class Error : public std::system_error
    {
    public:
        using std::system_error::system_error;
        Error(const char* what_arg) : std::system_error(0, std::generic_category(), what_arg) {};
        Error(const std::string& what_arg) : std::system_error(0, std::generic_category(), what_arg) {};
        Error(int code, const char* what_arg) : std::system_error(code, std::system_category(), what_arg) {};
        Error(int code, const std::string& what_arg) : std::system_error(code, std::system_category(), what_arg) {};
    };

    class CreateError  : public Error {public: using Error::Error;};
    class GetPortError : public Error {public: using Error::Error;};
    class BindError    : public Error {public: using Error::Error;};
    class ListenError  : public Error {public: using Error::Error;};
    class AcceptError  : public Error {public: using Error::Error;};
    class ConnectError : public Error {public: using Error::Error;};
    class SendError    : public Error {public: using Error::Error;};
    class ReceiveError : public Error {public: using Error::Error;};



    Socket();
    Socket(Socket&& that);
    Socket(int sock, int family);
    Socket(Protocols protocol, int family = INet, int flags = 0);
    virtual ~Socket();

    void create(Protocols protocol, int family = INet, int flags = 0);
    void close();
    bool isValid() const;

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
    return this->sendUnprocessed(&container[0], container.size() * sizeof(container[0]), flags);
}

template <typename Container>
void Socket::send(const Container& container, int flags)
{
    return this->send(&container[0], container.size() * sizeof(container[0]), flags);
}



template <typename Container>
std::size_t Socket::recvUnprocessed(Container& container, int flags)
{
    const std::size_t packet_size = this->recvSize(flags);
    assert(packet_size % sizeof(container[0]) == 0);
    container.resize(packet_size / sizeof(container[0]));

    this->recvRaw(&container[0], packet_size, flags);
    return packet_size;
}

template <typename Container>
std::size_t Socket::recv(Container& container, int flags)
{
    const std::size_t content_size = this->recvSize(flags);
    assert(content_size % sizeof(container[0]) == 0);
    container.resize(content_size / sizeof(container[0]));

    this->recvUnprocessed(this->buffer, flags);
    this->onReceive((uint8_t*)(&container[0]), content_size);
    return content_size;
}





////////////////////////////////////////////////////////////////////////////////
/// \ingroup shared
///@{
inline uint8_t ntoh(uint8_t x)
{
    return x;
}
inline uint16_t ntoh(uint16_t x)
{
    return ntohs(x);
}
inline uint32_t ntoh(uint32_t x)
{
    return ntohl(x);
}
inline uint64_t ntoh(uint64_t x)
{
    return (static_cast<uint64_t>(ntoh(static_cast<uint32_t>(x))) << 32) | ntoh(static_cast<uint32_t>(x >> 32));
}

inline uint8_t hton(uint8_t x)
{
    return x;
}
inline uint16_t hton(uint16_t x)
{
    return htons(x);
}
inline uint32_t hton(uint32_t x)
{
    return htonl(x);
}
inline uint64_t hton(uint64_t x)
{
    return (static_cast<uint64_t>(hton(static_cast<uint32_t>(x))) << 32) | hton(static_cast<uint32_t>(x >> 32));
}
///@}

#endif





////////////////////////////////////////////////////////////////////////////////
/// \class Socket
/// \ingroup shared
////////////////////////////////////////////////////////////////////////////////
