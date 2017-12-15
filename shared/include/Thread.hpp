#ifndef INCLUDED_THREAD_HPP
#define INCLUDED_THREAD_HPP

#include <pthread.h>
#include <exception>
#include <functional>

////////////////////////////////////////////////////////////////////////////////
/// \brief A class used to manipulate threads.
////////////////////////////////////////////////////////////////////////////////
class Thread
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

    /// \brief Exception thrown when thread creation fails
    struct CreateError      : public Error {using Error::Error;};

    /// \brief Exception thrown when detaching fails
    struct DetachError      : public Error {using Error::Error;};

    /// \brief Exception thrown when joining fails
    struct JoinError        : public Error {using Error::Error;};

    /// \brief Exception thrown when no thread is owned or a thread is already owned by the object (depending on the context)
    struct InvalThreadError : public Error {using Error::Error;};

public:
    ////////////////////////////////////////////////////////////////////////////////
    /// \brief Default constructor.
    /// \details No thread is created. You have to call launch.
    ////////////////////////////////////////////////////////////////////////////////
    Thread();

    ////////////////////////////////////////////////////////////////////////////////
    /// \brief Move constructor.
    /// \details The thread that will become invalid
    /// \param that [in] The thread to move from
    ////////////////////////////////////////////////////////////////////////////////
    Thread(Thread&& that);

    ////////////////////////////////////////////////////////////////////////////////
    /// \brief Create and start thread from callable and arguments.
    /// \details The callable can be a function pointer, functor or lambda. If the
    ///          object already own a joinable thread then InvalThreadError is
    ///          thrown. If an error occurs then CreateError is thrown.
    /// \param func [in] The callable object
    /// \param args [in, out] The arguments for the thread
    ////////////////////////////////////////////////////////////////////////////////
    template <typename F, typename... Args>
    Thread(F func, Args&&... args);

    ////////////////////////////////////////////////////////////////////////////////
    /// \brief Destructor
    /// \details If the thread is joinable then join will be called.
    ////////////////////////////////////////////////////////////////////////////////
    ~Thread();



    ////////////////////////////////////////////////////////////////////////////////
    /// \brief Create and start thread from callable and arguments.
    /// \details The callable can be a function pointer, functor or lambda. If the
    ///          object already own a joinable thread then InvalThreadError is
    ///          thrown. If an error occurs then CreateError is thrown.
    /// \param func [in] The callable object
    /// \param args [in, out] The arguments for the thread
    /// \return Reference to the object the method was called on
    ////////////////////////////////////////////////////////////////////////////////
    template <typename F, typename... Args>
    Thread& launch(F func, Args&&... args);



    ////////////////////////////////////////////////////////////////////////////////
    /// \brief Detach the thread if it is joinable.
    /// \details A detached thread will have its resources automatically released,
    ///          but cannot be joined. This function will remove the object's
    ///          ownership over the thread. If an error occurs then DetachError is
    ///          thrown.
    /// \sa isJoinable join
    ////////////////////////////////////////////////////////////////////////////////
    void detach();

    ////////////////////////////////////////////////////////////////////////////////
    /// \brief Wait until the thread returns or exits.
    /// \details If the thread is not joinable then this method does nothing. If an
    ///          error occurs then JoinError is thrown.
    /// \sa isJoinable
    ////////////////////////////////////////////////////////////////////////////////
    void join();



    ////////////////////////////////////////////////////////////////////////////////
    /// \brief Check if thread is joinable.
    /// \details A thread is joinable if it exists, is not detached and does not
    ///          represent the current thread.
    /// \return True if thread is joinable
    /// \sa detach selfId join
    ////////////////////////////////////////////////////////////////////////////////
    bool isJoinable() const;

    ////////////////////////////////////////////////////////////////////////////////
    /// \brief Get thread's id.
    /// \details If the thread is not joinable then InvalThreadError is thrown.
    /// \return Thread's id
    /// \sa selfId
    ////////////////////////////////////////////////////////////////////////////////
    pthread_t getId() const;



    ////////////////////////////////////////////////////////////////////////////////
    /// \brief Get the calling thread's id.
    /// \return Calling thread's id
    ////////////////////////////////////////////////////////////////////////////////
    static pthread_t selfId();

private:
    ////////////////////////////////////////////////////////////////////////////////
    /// \brief Entry point for the threads.
    ////////////////////////////////////////////////////////////////////////////////
    template <typename Callable>
    static void* entryPoint(void* userdata);



private:
    pthread_t thread; ///< Thread id
    bool joinable;    ///< True if the thread is joinable
};





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

#endif
