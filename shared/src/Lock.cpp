#include <Lock.hpp>



inline Lock::Lock(Mutex& mutex) : mutex(mutex)
{
    this->mutex.lock();
}

inline Lock::~Lock()
{
    this->mutex.unlock();
}
