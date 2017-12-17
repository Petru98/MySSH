#include <Thread.hpp>

inline Thread::Thread() : thread(), joinable(false)
{}

inline Thread::Thread(Thread&& that) : thread(that.thread), joinable(that.joinable)
{
    that.joinable = false;
}

template <typename F, typename... Args>
inline Thread::Thread(F func, Args&&... args) : Thread()
{
    this->launch(std::forward<F>(func), std::forward<Args>(args)...);
}

inline Thread::~Thread()
{
    this->join();
}



// https://linux.die.net/man/3/pthread_create
template <typename F, typename... Args>
inline Thread& Thread::launch(F func, Args&&... args)
{
    if(this->isJoinable())
        throw InvalThreadError("Thread object already owns a thread");

    auto thread_proc = std::bind(std::forward<F>(func), std::forward<Args>(args)...);
    const int error = pthread_create(&this->thread, nullptr, Thread::entryPoint<decltype(thread_proc)>, &thread_proc);
    if(error != 0)
        throw CreateError("Could not create thread", error);

    this->joinable = true;
    return (*this);
}


// https://linux.die.net/man/3/pthread_detach
inline void Thread::detach()
{
    if(this->isJoinable())
    {
        const int error = pthread_detach(this->thread);
        if(error != 0)
            throw DetachError("Could not detach thread", error);

        this->joinable = false;
    }
}

// https://linux.die.net/man/3/pthread_join
inline void Thread::join()
{
    if(this->isJoinable())
    {
        const int error = pthread_join(this->thread, nullptr);
        if(error != 0)
            throw JoinError("Could not join thread", error);

        this->joinable = false;
    }
}



inline bool Thread::isJoinable() const
{
    return (this->joinable == true) && (this->thread != Thread::selfId());
}

inline pthread_t Thread::getId() const
{
    if(this->isJoinable() == false)
        throw InvalThreadError("No thread whose id to get");
    return this->thread;
}



// https://linux.die.net/man/3/pthread_self
inline pthread_t Thread::selfId()
{
    return pthread_self();
}



// https://stackoverflow.com/questions/9306014/pthread-create-template-function-static-casting-a-template-class
template <typename Callable>
inline void* Thread::entryPoint(void* userdata)
{
    Callable procedure = std::move(*reinterpret_cast<Callable*>(userdata));
    procedure();
    return nullptr;
}
