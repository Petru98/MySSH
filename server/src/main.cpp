#include <Server.hpp>
#include <Logging.hpp>
#include <exception>

int main(int argc, char** argv)
{
    Server server;

    try
    {
        server.init(argc, argv);
        server.run();
        server.free();
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
