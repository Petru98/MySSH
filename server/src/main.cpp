#include <Server.hpp>
#include <Logging.hpp>
#include <iostream>

#include <unistd.h>

int main(int argc, char** argv)
{
    std::ios_base::sync_with_stdio(false);

    try
    {
        Server* server = new Server();
        server->run(argc, argv);
    }
    catch(std::exception& e)
    {
        error(e.what());
    }
    catch(...)
    {
        error("unknown error caught in 'main'");
    }

    return 0;
}
