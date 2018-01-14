#include <Client.hpp>
#include <pgpdef.hpp>
#include <cryptopp/osrng.h>



Client::Client()
{
    this->initializeOptions();

    CryptoPP::AutoSeededRandomPool rng;
    this->private_key.GenerateRandomWithKeySize(rng, pgp::RSA_KEY_SIZE);
    this->public_key = this->private_key;
}
Client::~Client()
{}



void Client::initializeOptions()
{
    this->options.addOption('p', "port", "1100");
}



void Client::run(int argc, char** argv)
{

}
