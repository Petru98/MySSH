#ifndef INCLUDED_LOCK_HPP
#define INCLUDED_LOCK_HPP

#include "Mutex.hpp"

////////////////////////////////////////////////////////////////////////////////
/// \brief Wrapper for locking and unlocking mutexes automatically.
////////////////////////////////////////////////////////////////////////////////
class Lock
{
public:
    ////////////////////////////////////////////////////////////////////////////////
    /// \brief Lock mutex and take onwership over it.
    /// \details This constructor calls Mutex::lock(), which may throw an exception.
    /// \param mutex Mutex to take ownership over
    ////////////////////////////////////////////////////////////////////////////////
    explicit Lock(Mutex& mutex);

    ////////////////////////////////////////////////////////////////////////////////
    /// \brief Unlock mutex when lock goes out of scope.
    /// \details This destructor calls Mutex::unlock(), which may throw an exception.
    ////////////////////////////////////////////////////////////////////////////////
    ~Lock();



private:
    Mutex& mutex; ///< Mutex
};





inline Lock::Lock(Mutex& mutex) : mutex(mutex)
{
    this->mutex.lock();
}
inline Lock::~Lock()
{
    this->mutex.unlock();
}
#endif





////////////////////////////////////////////////////////////////////////////////
/// \class Lock
/// \ingroup shared
///
/// This class is used to automatically unlock a mutex when the lock goes out of scope.
/// This is especially useful when the function might throw an exception, or it has a lot of return statements.
///
/// Usage example:
/// \code
///     Mutex mutex;
///
///     void f(int x)
///     {
///         Lock lock(mutex); // mutex is locked here
///
///         if(x == 1)
///             throw "mutex is unlocked when this exception is thrown";
///
///         if(x == 0)
///             return; // mutex is unlocked
///
///         if(x % 2 == 1)
///             return; // mutex is unlocked
///
///         // mutex is unlocked at the end of function
///     }
/// \endcode
///
/// \sa Mutex
////////////////////////////////////////////////////////////////////////////////
