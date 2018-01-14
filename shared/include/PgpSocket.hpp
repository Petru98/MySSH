#ifndef INCLUDED_PGPSOCKET_HPP
#define INCLUDED_PGPSOCKET_HPP

#include "Socket.hpp"
#include <cryptopp/rsa.h>



class PgpSocket : public Socket
{
public:
    using Socket::Socket;

    PgpSocket();
    PgpSocket(PgpSocket&& that);
    virtual ~PgpSocket();

    void setKeys(CryptoPP::RSA::PublicKey* encode_key, CryptoPP::RSA::PrivateKey* decode_key);

private:
    virtual void onSend(const uint8_t* data, std::size_t size) override;
    virtual void onReceive(uint8_t* data, std::size_t size) override;



protected:
    CryptoPP::RSA::PublicKey* encode_key;
    CryptoPP::RSA::PrivateKey* decode_key;
};

#endif
