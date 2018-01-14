#include <PgpSocket.hpp>
#include <algorithm>



PgpSocket::PgpSocket() : Socket(Tcp, INet), encode_key(nullptr), decode_key(nullptr)
{}
PgpSocket::PgpSocket(PgpSocket&& that) : Socket(std::move(that)), encode_key(that.encode_key), decode_key(that.decode_key)
{}

PgpSocket::~PgpSocket()
{
    this->encode_key = nullptr;
    this->decode_key = nullptr;
}



void PgpSocket::setKeys(CryptoPP::RSA::PublicKey* encode_key, CryptoPP::RSA::PrivateKey* decode_key)
{
    this->encode_key = encode_key;
    this->decode_key = decode_key;
}



void PgpSocket::onSend(const uint8_t* data, std::size_t size)
{
    this->buffer.assign(data, data + size);
}

void PgpSocket::onReceive(uint8_t* data, std::size_t size)
{
    size = this->buffer.size();
    std::copy_n(this->buffer.data(), size, data);
}
