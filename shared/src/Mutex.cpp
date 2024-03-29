#include <Mutex.hpp>



// https://linux.die.net/man/3/pthread_mutex_init
Mutex::Mutex()
{
    const int error = pthread_mutex_init(&this->mutex, nullptr);
    if(error != 0)
        throw CreateError(error, "could not create mutex");
}

Mutex::Mutex(const Mutex& that) : Mutex()
{
    ((void)that);
}

// https://linux.die.net/man/3/pthread_mutex_destroy
Mutex::~Mutex()
{
    pthread_mutex_destroy(&this->mutex);
}



// https://linux.die.net/man/3/pthread_mutex_lock
void Mutex::lock()
{
    const int error = pthread_mutex_lock(&this->mutex);
    if(error != 0)
        throw LockError(error, "could not lock mutex");
}

// https://linux.die.net/man/3/pthread_mutex_unlock
void Mutex::unlock()
{
    const int error = pthread_mutex_unlock(&this->mutex);
    if(error != 0)
        throw LockError(error, "could not unlock mutex");
}

// https://linux.die.net/man/3/pthread_mutex_trylock
bool Mutex::trylock()
{
    const int error = pthread_mutex_trylock(&this->mutex);

    if(error == 0)
        return true;
    if(error == EBUSY)
        return false;

    throw LockError(error, "could not lock mutex");
}
