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



void Client::init(int argc, char** argv)
{
    this->options.parse(argc, argv);

    const std::vector<std::string>& args = this->options.getArguments();
    if(args.size() == 0 || args.size() > 2)
        throw std::runtime_error("invalid number of arguments");

    this->server.ip = args[0];

    if(args.size() >= 2)
    {
        this->server.port = std::stoi(args[1]);
        if(port < 0 || port > 65535)
            throw std::runtime_error("invalid port");
    }
    else
    {
        int port = std::stoi(this->options.findOption("port"));
        if(port < 0 || port > 65535)
            throw std::runtime_error("invalid port");
    }



    this->server.sock.connect(this->server.ip, this->server.port);
}

void Client::run(int argc, char** argv)
{
    this->init(argc, argv);
    this->loop();
    this->free();
}

void Client::free()
{
    this->server.sock.close();
}



void Client::loop()
{

}
