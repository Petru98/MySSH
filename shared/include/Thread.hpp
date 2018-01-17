#ifndef INCLUDED_THREAD_HPP
#define INCLUDED_THREAD_HPP

#include <pthread.h>
#include <system_error>
#include <functional>



class Thread
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

    class LaunchError          : public Error {public: using Error::Error;};
    class DetachError          : public Error {public: using Error::Error;};
    class JoinError            : public Error {public: using Error::Error;};
    class NotLaunchedError     : public Error {public: using Error::Error;};
    class AlreadyLaunchedError : public Error {public: using Error::Error;};



    Thread();
    Thread(Thread&& that);

    template <typename F, typename... Args>
    Thread(F func, Args&&... args);

    ~Thread();



    template <typename F, typename... Args>
    Thread& launch(F&& func, Args&&... args);

    void detach();
    void join();

    bool isAlive() const;
    bool isJoinable() const;

    pthread_t getId() const;

    static pthread_t selfId();

private:
    template <typename Callable>
    static void* entryPoint(void* userdata);



private:
    pthread_t thread;
    bool joinable;
};





template <typename F, typename... Args>
Thread::Thread(F func, Args&&... args) : Thread()
{
    this->launch(std::forward<F>(func), std::forward<Args>(args)...);
}

// https://linux.die.net/man/3/pthread_create
template <typename F, typename... Args>
Thread& Thread::launch(F&& func, Args&&... args)
{
    if(this->isJoinable())
        throw AlreadyLaunchedError("thread object was already launched");

    auto thread_proc = new std::function<void()>(std::bind(std::forward<F&&>(func), std::forward<Args&&>(args)...));
    const int error = pthread_create(&this->thread, nullptr, Thread::entryPoint<std::remove_pointer<decltype(thread_proc)>::type>, thread_proc);
    if(error != 0)
    {
        delete thread_proc;
        throw LaunchError(error, "could not launch thread");
    }

    this->joinable = true;
    return (*this);
}

// https://stackoverflow.com/questions/9306014/pthread-create-template-function-static-casting-a-template-class
template <typename Callable>
void* Thread::entryPoint(void* userdata)
{
    Callable* procedure_ptr = reinterpret_cast<Callable*>(userdata);
    Callable& procedure = (*procedure_ptr);
    procedure();
    delete procedure_ptr;
    return nullptr;
}

#endif





////////////////////////////////////////////////////////////////////////////////
/// \class Thread
/// \ingroup shared
////////////////////////////////////////////////////////////////////////////////
