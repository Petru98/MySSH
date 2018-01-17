#include <IpAddress.hpp>
#include <error.hpp>
#include <cerrno>



const IpAddress IpAddress::Any(ntohl(INADDR_ANY));



IpAddress::IpAddress()
{
    this->address.s_addr = 0;
}
IpAddress::IpAddress(const IpAddress& that)
{
    (*this) = that;
}
IpAddress::IpAddress(const std::string& address)
{
    (*this) = address;
}
IpAddress::IpAddress(uint32_t address)
{
    (*this) = address;
}



IpAddress& IpAddress::operator= (const IpAddress& that)
{
    this->address.s_addr = that.address.s_addr;
    return (*this);
}
IpAddress& IpAddress::operator= (const std::string& address)
{
    // https://github.com/SFML/SFML/blob/247b03172c34f25a808bcfdc49f390d619e7d5e0/src/SFML/Network/IpAddress.cpp#L164

    if(inet_aton(address.c_str(), &this->address) == 0) // Try converting it and if it's not a valid IP address then interpret it as a host name
    {
        addrinfo hints = {};
        hints.ai_family = AF_INET;
        addrinfo* result = nullptr;

        const int error = getaddrinfo(address.c_str(), nullptr, &hints, &result);
        if(error != 0)
        {
            if(error != EAI_SYSTEM)
                throw GetHostAddressError(error, gai_strerr(error));
            else
                throw GetHostAddressError(errno, "could not get host's IP address");
        }

        if(result == nullptr)
            throw InvalidAddressError("invalid IP address and host name");

        this->address = reinterpret_cast<sockaddr_in*>(result->ai_addr)->sin_addr;
        freeaddrinfo(result);
    }

    return (*this);
}
IpAddress& IpAddress::operator= (uint32_t address)
{
    this->address.s_addr = address;
    return (*this);
}



std::string IpAddress::toString() const
{
    return inet_ntoa(this->address);
}
uint32_t IpAddress::toInt() const
{
    return ntohl(this->address.s_addr);
}



bool operator== (const IpAddress& lval, const IpAddress& rval)
{
    return lval.address.s_addr == rval.address.s_addr;
}
bool operator!= (const IpAddress& lval, const IpAddress& rval)
{
    return !(lval == rval);
}
