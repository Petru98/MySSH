#ifndef INCLUDED_MUTEX_HPP
#define INCLUDED_MUTEX_HPP

#include <pthread.h>
#include <errno.h>
#include <exception>

////////////////////////////////////////////////////////////////////////////////
/// \brief A class used for synchronizing threads.
////////////////////////////////////////////////////////////////////////////////
class Mutex
{
public:
    ////////////////////////////////////////////////////////////////////////////////
    /// \brief Base class for all exceptions
    /// \details This class is inherited from std::exception
    ////////////////////////////////////////////////////////////////////////////////
    struct Error : public std::exception
    {
        /// \brief Construct from text message and (optionally) form error code
        Error(const char* msg, int code = 0) : msg(msg), code(code) {}

        /// \brief Defined to have consistent interface with std exceptions
        virtual const char* what() {return this->msg;}



        const char* msg; ///< Text message
        int code;        ///< Error code
    };

    /// \brief Exception thrown when creation fails
    struct CreateError  : public Error {using Error::Error;};

    /// \brief Exception thrown when locking fails
    struct LockError    : public Error {using Error::Error;};

    /// \brief Exception thrown when unlocking fails
    struct UnlockError  : public Error {using Error::Error;};

    /// \brief Exception thrown when try-locking fails
    struct TryLockError : public Error {using Error::Error;};

public:
    ////////////////////////////////////////////////////////////////////////////////
    /// \brief Default constructor.
    /// \details If creation of the mutex fails then CreateError is thrown.
    ////////////////////////////////////////////////////////////////////////////////
    Mutex();

    ////////////////////////////////////////////////////////////////////////////////
    /// \brief Destructor.
    ////////////////////////////////////////////////////////////////////////////////
    ~Mutex();



    ////////////////////////////////////////////////////////////////////////////////
    /// \brief Lock the mutex.
    /// \details If the mutex is already locked then the thread blocks until it
    ///          becomes unlocked. Attempting to lock a mutex twice in the same
    ///          thread causes undefined behaviour. If an error occurs then
    ///          LockError is thrown. For a non-blocking alternative see trylock.
    /// \sa trylock unlock
    ////////////////////////////////////////////////////////////////////////////////
    void lock();

    ////////////////////////////////////////////////////////////////////////////////
    /// \brief Unlock the mutex.
    /// \details Attempting to unlock an already unlocked mutex causes undefined
    ///          behaviour. Attempting to unlock a mutex locked by another thread
    ///          causes undefined behaviour. If an error occurs then UnlockError is
    ///          thrown.
    /// \sa lock trylock
    ////////////////////////////////////////////////////////////////////////////////
    void unlock();

    ////////////////////////////////////////////////////////////////////////////////
    /// \brief Try locking the mutex.
    /// \details Like lock, but return immediately without blocking. If the mutex is
    ///          unlocked then this method locks it and returns true. Otherwise, it
    ///          returns false. If an error occurs then TryLockError is thrown. For
    ///          a blocking alternative see lock.
    /// \return True if locking was successfully. False otherwise
    /// \sa lock unlock
    ////////////////////////////////////////////////////////////////////////////////
    bool trylock();



private:
    pthread_mutex_t mutex; ///< Mutex
};





// https://linux.die.net/man/3/pthread_mutex_init
inline Mutex::Mutex()
{
    const int error = pthread_mutex_init(&this->mutex, nullptr);
    if(error != 0)
        throw CreateError("Could not create mutex", error);
}

// https://linux.die.net/man/3/pthread_mutex_destroy
inline Mutex::~Mutex()
{
    pthread_mutex_destroy(&this->mutex);
}



// https://linux.die.net/man/3/pthread_mutex_lock
inline void Mutex::lock()
{
    const int error = pthread_mutex_lock(&this->mutex);
    if(error != 0)
        throw LockError("Could not lock mutex", error);
}

// https://linux.die.net/man/3/pthread_mutex_unlock
inline void Mutex::unlock()
{
    const int error = pthread_mutex_unlock(&this->mutex);
    if(error != 0)
        throw LockError("Could not unlock mutex", error);
}

// https://linux.die.net/man/3/pthread_mutex_trylock
inline bool Mutex::trylock()
{
    const int error = pthread_mutex_trylock(&this->mutex);

    if(error == 0)
        return true;
    if(error == EBUSY)
        return false;

    throw LockError("Could not lock mutex", error);
}
#endif
