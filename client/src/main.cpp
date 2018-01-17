#include <Client.hpp>
#include <Logging.hpp>
#include <memory>
#include <iostream>



int main(int argc, char** argv)
{
    std::ios_base::sync_with_stdio(false);

    try
    {
        std::unique_ptr<Client> client(new Client());
        client->run(argc, argv);
    }
    catch(std::exception& e)
    {
        error(e.what());
    }
    catch(...)
    {
        error("unknown exception caught in 'main'");
    }

    return 0;
}
