#include <IpAddress.hpp>



const IpAddress::IpAddress Any("0.0.0.0");



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
}
IpAddress& IpAddress::operator= (const std::string& address)
{
    // https://github.com/SFML/SFML/blob/247b03172c34f25a808bcfdc49f390d619e7d5e0/src/SFML/Network/IpAddress.cpp#L164

    if(inet_aton(address.c_str(), &this->address) == 0) // Try converting it and if it is not an IP address (zero returned) then interpret it as a host name
    {
        addrinfo hints = {0};
        hints.ai_family = AF_INET;
        addrinfo* result = nullptr;

        if(getaddrinfo(address.c_str(), nullptr, &hints, &result) != 0)
            throw GetHostAddressError();

        if(result == nullptr)
            throw InvalidAddressError();

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
