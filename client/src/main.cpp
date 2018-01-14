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
        std::cout << "error: " << e.what() << '\n';
    }
    catch(...)
    {
        std::cout << "error: unknown error caught in 'main'\n";
    }

    return 0;
}
