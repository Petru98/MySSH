#include <Client.hpp>
#include <pgpdef.hpp>
#include <iostream>
#include <cryptopp/osrng.h>



Client::Client()
{
    this->initializeOptions();

    CryptoPP::AutoSeededRandomPool rng;
    this->privatekey.GenerateRandomWithKeySize(rng, pgp::RSA_KEY_SIZE);
    this->publickey = this->privatekey;
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
        const int port = std::stoi(args[1]);
        if(port < 0 || port > 65535)
            throw std::runtime_error("invalid port");
        this->server.port = port;
    }
    else
    {
        const int port = std::stoi(this->options.findOption("port"));
        if(port < 0 || port > 65535)
            throw std::runtime_error("invalid port");
        this->server.port = port;
    }



    this->server.sock.connect(this->server.ip, this->server.port);
    std::string buffer;

    // Public key exchange
    this->publickey.Save(CryptoPP::StringSink(buffer).Ref());
    this->server.sock.sendUnprocessed(buffer);

    this->server.sock.recvUnprocessed(buffer);
    this->server.publickey.Load(CryptoPP::StringSource(buffer, true).Ref());

    this->server.sock.setKeys(&this->server.publickey, &this->privatekey);

    // Username
    std::cout << "username: ";
    std::getline(std::cin, buffer);

    this->server.sock.send(buffer);
    if(this->server.sock.recv8() != 1)
        throw std::runtime_error("no user with this username exists");

    // Password
    std::cout << "password: ";
    std::getline(std::cin, buffer);

    this->server.sock.send(buffer);
    if(this->server.sock.recv8() != 1)
        throw std::runtime_error("incorrect password");
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
    std::string prompt;
    std::string buffer;
    bool must_exit = false;

    while(must_exit == false)
    {
        try
        {
            this->server.sock.recv(prompt);
            std::cout << prompt;

            if(!std::getline(std::cin, buffer))
                must_exit = true;
            else
            {
                this->server.sock.send(buffer);

                int response = this->server.sock.recv8();
                while(response == 1)
                {
                    this->server.sock.recv(buffer);
                    std::cout << buffer;
                }

                if(response == 255)
                {
                    must_exit = true;
                    std::cout << "\n";
                }
            }
        }
        catch(Socket::ReceiveError& e)
        {
            std::cout << "error: " << e.what() << '\n';
            must_exit = true;
        }
        catch(Socket::SendError& e)
        {
            std::cout << "error: " << e.what() << '\n';
            must_exit = true;
        }
        catch(std::exception& e)
        {
            std::cout << "error: " << e.what() << '\n';
        }
        catch(...)
        {
            std::cout << "error: unknown exception caught in 'Client::loop'\n";
            must_exit = true;
        }
    }
}
