#include <error.hpp>
#include <cstring>
#include <netdb.h>

#include <Mutex.hpp>
#include <Lock.hpp>



std::string strerr(int error_code)
{
    static Mutex mutex;
    Lock lock(mutex);

    std::string str = strerror(error_code);
    return std::move(str);
}

std::string gai_strerr(int error_code)
{
    static Mutex mutex;
    Lock lock(mutex);

    std::string str = gai_strerror(error_code);
    return std::move(str);
}
