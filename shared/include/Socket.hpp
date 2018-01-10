#ifndef INCLUDED_SOCKET_HPP
#define INCLUDED_SOCKET_HPP

#include <sys/socket.h>
#include <sys/types.h>
#include "IpAddress.hpp"

class Socket
{
public:
    ////////////////////////////////////////////////////////////////////////////////
    /// \brief Protocol families enumeration.
    ////////////////////////////////////////////////////////////////////////////////
    enum Families
    {
        Local = AF_LOCAL,
        Unix  = AF_UNIX,
        INet  = AF_INET
    };

    ////////////////////////////////////////////////////////////////////////////////
    /// \brief Protocols enumeration.
    ////////////////////////////////////////////////////////////////////////////////
    enum Protocols
    {
        Tcp = SOCK_STREAM,
        Udp = SOCK_DGRAM
    };

    ////////////////////////////////////////////////////////////////////////////////
    /// \brief Socket flags enumeration.
    ////////////////////////////////////////////////////////////////////////////////
    enum Flags
    {
        NonBlock = SOCK_NONBLOCK
    };



    ////////////////////////////////////////////////////////////////////////////////
    /// \brief Base class for the other error classes that might be thrown.
    ////////////////////////////////////////////////////////////////////////////////
    class Error : public std::exception
    {};

    ////////////////////////////////////////////////////////////////////////////////
    /// \brief Error thrown when trying to create a socket before closing it.
    ////////////////////////////////////////////////////////////////////////////////
    class AlreadyCreatedError : public Error
        {public: virtual const char* what() {return "Socket was already created";}};

    ////////////////////////////////////////////////////////////////////////////////
    /// \brief Error thrown when creation fails.
    ////////////////////////////////////////////////////////////////////////////////
    class CreateError : public Error
        {public: virtual const char* what() {return "Could not create socket";}};

    ////////////////////////////////////////////////////////////////////////////////
    /// \brief Error thrown when binding fails.
    ////////////////////////////////////////////////////////////////////////////////
    class BindError : public Error
        {public: virtual const char* what() {return "Could not bind socket";}};

    ////////////////////////////////////////////////////////////////////////////////
    /// \brief Error thrown when listening fails.
    ////////////////////////////////////////////////////////////////////////////////
    class ListenError : public Error
        {public: virtual const char* what() {return "Could not listen socket";}};

    ////////////////////////////////////////////////////////////////////////////////
    /// \brief Error thrown when accepting connection fails.
    ////////////////////////////////////////////////////////////////////////////////
    class AcceptError : public Error
        {public: virtual const char* what() {return "Could not accept socket";}};

    ////////////////////////////////////////////////////////////////////////////////
    /// \brief Error thrown when connecting fails.
    ////////////////////////////////////////////////////////////////////////////////
    class ConnectError : public Error
        {public: virtual const char* what() {return "Could not connect";}};

    ////////////////////////////////////////////////////////////////////////////////
    /// \brief Error thrown when connecting fails.
    ////////////////////////////////////////////////////////////////////////////////
    class SendError : public Error
        {public: virtual const char* what() {return "Could not send data";}};

    ////////////////////////////////////////////////////////////////////////////////
    /// \brief Error thrown when connecting fails.
    ////////////////////////////////////////////////////////////////////////////////
    class ReceiveError : public Error
        {public: virtual const char* what() {return "Could not receive data";}};



    ////////////////////////////////////////////////////////////////////////////////
    /// \brief Default constructor.
    /// \details Creates an invalid socket.
    ////////////////////////////////////////////////////////////////////////////////
    Socket();

    ////////////////////////////////////////////////////////////////////////////////
    /// \brief Move constructor.
    /// \param that Where to move from
    ////////////////////////////////////////////////////////////////////////////////
    Socket(Socket&& that);

    ////////////////////////////////////////////////////////////////////////////////
    /// \brief Create from file descriptor.
    /// \param sock File descriptor of the socket
    /// \param family Protocol family of the socket
    ////////////////////////////////////////////////////////////////////////////////
    Socket(int sock, int family);

    ////////////////////////////////////////////////////////////////////////////////
    /// \brief Create from protocol and protocol family.
    /// \details This constructor calls create().
    /// \param protocol A value from \a Protocols enumeration
    /// \param family A value from \a Families enumeration
    /// \sa create()
    ////////////////////////////////////////////////////////////////////////////////
    Socket(Protocols protocol, int family = INet, int flags = 0);

    ////////////////////////////////////////////////////////////////////////////////
    /// \brief Destructor.
    /// \details The destructor calls close.
    /// \sa close()
    ////////////////////////////////////////////////////////////////////////////////
    virtual ~Socket();



    ////////////////////////////////////////////////////////////////////////////////
    /// \brief Create from protocol and protocol family.
    /// \details If the socket is already valid then AlreadyCreatedError is thrown.
    ///          If another error occurs then CreateError is thrown.
    /// \param protocol A value from \a Protocols enumeration
    /// \param family A value from \a Families enumeration
    ////////////////////////////////////////////////////////////////////////////////
    void create(Protocols protocol, int family = INet, int flags = 0);

    ////////////////////////////////////////////////////////////////////////////////
    /// \brief Close the socket.
    ////////////////////////////////////////////////////////////////////////////////
    void close();

    ////////////////////////////////////////////////////////////////////////////////
    /// \brief Check if the socket is valid or not.
    /// \details An invalid socket cannot be used.
    /// \return True if the socket is valid, false otherwise.
    ////////////////////////////////////////////////////////////////////////////////
    bool isValid() const;



    ////////////////////////////////////////////////////////////////////////////////
    /// \brief Bind socket to a port and address.
    /// \details Throws BindError on failure.
    /// \param port Port
    /// \param address Address
    ////////////////////////////////////////////////////////////////////////////////
    void bind(uint16_t port, IpAddress address = IpAddress::Any);

    ////////////////////////////////////////////////////////////////////////////////
    /// \brief Listen to the port and IP address that the socket is binded to.
    /// \details Throws ListenError on failure.
    /// \param max_pending_connections Maximum number of pending connections.
    ////////////////////////////////////////////////////////////////////////////////
    void listen(int max_pending_connections = 0);

    ////////////////////////////////////////////////////////////////////////////////
    /// \brief Accept a pending connection.
    /// \details Throws AcceptError on failure. The parameters are filled with the
    ///          new socket's information. If \a sock is a valid socket then it is
    ///          closed first. If the listener socket is set to non-block and there
    ///          are no pending connections then the return
    ///          value is false.
    /// \param address Reference to IpAddress instance where to put the IP address
    /// \param port Reference to a 16-bit unsigned integer where to put the port
    /// \param sock Reference to socket corresponding to the new connection
    /// \return True on success, false if socket is non-blocking and there are no
    ///         pending connections
    ////////////////////////////////////////////////////////////////////////////////
    bool accept(IpAddress& address, uint16_t& port, Socket& sock);

    ////////////////////////////////////////////////////////////////////////////////
    /// \brief Connect to remote IP address and port.
    /// \details Throws ConnectError on failure.
    /// \param address Remote IP address
    /// \param port Remote port
    /// \return True on success, false if socket is non-blocking and the connection
    ///         was not completed immediately
    ////////////////////////////////////////////////////////////////////////////////
    bool connect(IpAddress address, uint16_t port);



    ////////////////////////////////////////////////////////////////////////////////
    /// \brief Send binary data.
    /// \details Throws SendError on failure. If the socket is non-blocking and the
    ///          data cannot fit in the buffer then 0 is returned. If \a data is
    ///          a null pointer then nothing is done and 0 is returned.
    /// \param data The data to send
    /// \param size The size of data
    /// \param flags Optional flags
    /// \return Number of bytes sent
    ////////////////////////////////////////////////////////////////////////////////
    std::size_t send(const void* data, std::size_t size, int flags = 0);

    ////////////////////////////////////////////////////////////////////////////////
    /// \brief Receive binary data.
    /// \details Throws ReceiveError on failure. If the socket is non-blocking and
    ///          there is no message available then 0 is returned. If the buffer is
    ///          too small then the excess bytes may be discarded on UDP. If \a data
    ///          is a null pointer then nothing is done and 0 is returned.
    /// \param buffer The buffer
    /// \param size The size of the buffer
    /// \param flags Optional flags
    /// \return Number of bytes received
    ////////////////////////////////////////////////////////////////////////////////
    std::size_t recv(void* buffer, std::size_t size, int flags = 0);



protected:
    ////////////////////////////////////////////////////////////////////////////////
    /// \brief Method called before sending data.
    /// \details This method can be overwritten by child classes. You can change the
    ///          \a size parameter if the size of data changes.
    ///          \warning If you return a pointer to a dinamically allocated chunk
    ///          of memory then you are responsible of freeing it.
    /// \param data Pointer to data to be processed
    /// \param size Size of data
    /// \return Data after processing, which is the actual data sent
    ////////////////////////////////////////////////////////////////////////////////
    virtual const void* onSend(const uint8_t* data, std::size_t& size) {return data;}

    ////////////////////////////////////////////////////////////////////////////////
    /// \brief Method called after receiving data.
    /// \details This method can be overwritten by child classes. You must put the
    ///          result in the buffer pointed by \a data. You can change the \a size
    ///          parameter if, for example, the data is uncompressed.
    ///          \warning You must make sure that the buffer pointed by data is big
    ///          enough to hold the result of this method.
    /// \param data Pointer to data received
    /// \param size Size of data received
    ////////////////////////////////////////////////////////////////////////////////
    virtual void onReceive(uint8_t* data, std::size_t& size) {}



private:
    int sock; ///< Socket file descriptor
    int family; ///< Protocol family
};

#endif
