#ifndef INCLUDED_MUTEX_HPP
#define INCLUDED_MUTEX_HPP

#include <pthread.h>
#include <cerrno>
#include <system_error>



class Mutex
{
public:
    class Error : public std::system_error
    {
    public:
        using std::system_error::system_error;
        Error(const char* what_arg) : std::system_error(0, std::generic_category(), what_arg) {};
        Error(const std::string& what_arg) : std::system_error(0, std::generic_category(), what_arg) {};
        Error(int code, const char* what_arg) : std::system_error(code, std::system_category(), what_arg) {};
        Error(int code, const std::string& what_arg) : std::system_error(code, std::system_category(), what_arg) {};
    };

    struct CreateError  : public Error {using Error::Error;};
    struct LockError    : public Error {using Error::Error;};
    struct UnlockError  : public Error {using Error::Error;};
    struct TryLockError : public Error {using Error::Error;};



    Mutex();
    Mutex(const Mutex& that);
    ~Mutex();



    void lock();
    void unlock();
    bool trylock();



private:
    pthread_mutex_t mutex;
};

#endif





////////////////////////////////////////////////////////////////////////////////
/// \class Mutex
/// \ingroup shared
////////////////////////////////////////////////////////////////////////////////
