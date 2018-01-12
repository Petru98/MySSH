#ifndef INCLUDED_SHA512_HPP
#define INCLUDED_SHA512_HPP

#include <cryptopp/sha.h>
#include <cstdint>



class Sha512 : private CryptoPP::SHA512
{
public:
    static constexpr std::size_t DIGEST_SIZE = DIGESTSIZE;



    Sha512();
    Sha512(const void* data, std::size_t size);

    Sha512& update(const void* data, std::size_t size);
    Sha512& finish(void* buffer);
    Sha512& restart();
};

#endif
