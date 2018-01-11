#include <Lock.hpp>



Lock::Lock(Mutex& mutex) : mutex(mutex)
{
    this->mutex.lock();
}

Lock::~Lock()
{
    this->mutex.unlock();
}
