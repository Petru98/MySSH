#include <Client.hpp>
#include <iostream>



int main(int argc, char** argv)
{
    std::ios_base::sync_with_stdio(false);

    try
    {
        Client* client = new Client();
        client->run(argc, argv);
    }
    catch(std::exception& e)
    {
        std::cout << e.what();
    }
    catch(...)
    {
        std::cout << "unknown error caught in 'main'";
    }

    return 0;
}
