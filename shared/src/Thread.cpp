#include <Thread.hpp>
#include <Mutex.hpp>
#include <Lock.hpp>
#include <csignal>
#include <cstring>



// Thread
Thread::Thread() : thread(), joinable(false)
{}

Thread::Thread(Thread&& that) : thread(that.thread), joinable(that.joinable)
{
    that.joinable = false;
}

Thread::~Thread()
{
    this->join();
}



// https://linux.die.net/man/3/pthread_detach
void Thread::detach()
{
    if(this->isJoinable())
    {
        const int error = pthread_detach(this->thread);
        if(error != 0)
            throw DetachError(error, "could not detach thread");

        this->joinable = false;
    }
}

// https://linux.die.net/man/3/pthread_join
void Thread::join()
{
    if(this->isJoinable())
    {
        const int error = pthread_join(this->thread, nullptr);
        if(error != 0)
            throw JoinError(error, "could not join with thread");

        this->joinable = false;
    }
}



// https://linux.die.net/man/3/pthread_kill
bool Thread::isAlive() const
{
    if(this->isJoinable())
    {
        if(pthread_kill(this->thread, 0) == -1)
            return errno == ESRCH;
        return true;
    }

    return false;
}

bool Thread::isJoinable() const
{
    return (this->joinable == true) && (this->thread != Thread::selfId());
}

pthread_t Thread::getId() const
{
    if(this->isJoinable() == false)
        throw NotLaunchedError("could not get the thread's id");
    return this->thread;
}



// https://linux.die.net/man/3/pthread_self
pthread_t Thread::selfId()
{
    return pthread_self();
}
