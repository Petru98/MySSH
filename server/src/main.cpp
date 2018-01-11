#include <Server.hpp>
#include <Logging.hpp>
#include <exception>

int main(int argc, char** argv)
{
    Server server;

    try
    {
        server.run(argc, argv);
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
