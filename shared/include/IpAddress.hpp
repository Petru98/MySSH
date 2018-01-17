#ifndef INCLUDED_IPADDRESS_HPP
#define INCLUDED_IPADDRESS_HPP

#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <cstdint>
#include <string>
#include <system_error>



class IpAddress
{
public:
    class Error : public std::system_error
    {
    public:
        using std::system_error::system_error;
        Error(const char* what_arg) : std::system_error(0, std::generic_category(), what_arg) {};
        Error(const std::string& what_arg) : std::system_error(0, std::generic_category(), what_arg) {};
        Error(int code, const char* what_arg) : std::system_error(code, std::system_category(), what_arg) {};
        Error(int code, const std::string& what_arg) : std::system_error(code, std::system_category(), what_arg) {};
    };

    class GetHostAddressError : public Error {public: using Error::Error;};
    class InvalidAddressError : public Error {public: using Error::Error;};



    IpAddress();
    IpAddress(const IpAddress& that);
    IpAddress(const std::string& address);
    IpAddress(uint32_t address);

    IpAddress& operator= (const IpAddress& that);
    IpAddress& operator= (const std::string& address);
    IpAddress& operator= (uint32_t address);

    std::string toString() const;
    uint32_t toInt() const;



    friend bool operator== (const IpAddress& lval, const IpAddress& rval);
    friend bool operator!= (const IpAddress& lval, const IpAddress& rval);



    static const IpAddress Any;

private:
    in_addr address;
};
#endif





////////////////////////////////////////////////////////////////////////////////
/// \class IpAddress
/// \ingroup shared
////////////////////////////////////////////////////////////////////////////////
