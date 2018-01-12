#include <Sha512.hpp>



constexpr std::size_t Sha512::DIGEST_SIZE;



Sha512::Sha512() : SHA512()
{}
Sha512::Sha512(const void* data, std::size_t size) : SHA512()
{
    this->update(data, size);
}



Sha512& Sha512::update(const void* data, std::size_t size)
{
    this->Update(reinterpret_cast<const byte*>(data), size);
    return (*this);
}
Sha512& Sha512::finish(void* buffer)
{
    this->Final(reinterpret_cast<byte*>(buffer));
    return (*this);
}
Sha512& Sha512::restart()
{
    this->Restart();
    return (*this);
}
