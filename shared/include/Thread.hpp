#ifndef INCLUDED_THREAD_HPP
#define INCLUDED_THREAD_HPP

#include <thread>

////////////////////////////////////////////////////////////////
/// \brief A class used to manipulate threads.
////////////////////////////////////////////////////////////////
class Thread
{
public:
    ////////////////////////////////////////////////////////////////
    /// \brief Default constructor.
    /// \details Creates an empty thread that cannot be launched.
    ////////////////////////////////////////////////////////////////
    Thread();

    ////////////////////////////////////////////////////////////////
    /// \brief Move constructor.
    ////////////////////////////////////////////////////////////////
    Thread(Thread&& that);

    ////////////////////////////////////////////////////////////////
    /// \brief Create and start thread from callable and arguments.
    /// \details The callable can be a function pointer, functor or
    ///          lambda.
    /// \param func [in] The callable object
    /// \param args [in, out] The arguments for the thread
    ////////////////////////////////////////////////////////////////
    template <typename F, typename... Args>
    Thread(F func, Args&&... args);

    ////////////////////////////////////////////////////////////////
    /// \brief Destructor
    /// \details If the thread is joinable the join will be called.
    ////////////////////////////////////////////////////////////////
    ~Thread();



    ////////////////////////////////////////////////////////////////
    /// \brief Create and start thread from callable and arguments.
    /// \details The callable can be a function pointer, functor or
    ///          lambda.
    /// \param func [in] The callable object
    /// \param args [in, out] The arguments for the thread
    /// \return Reference to the instance it was called on
    ////////////////////////////////////////////////////////////////
    template <typename F, typename... Args>
    Thread& launch(F func, Args&&... args);



    ////////////////////////////////////////////////////////////////
    /// \brief Detach the thread if it is joinable.
    /// \details A detached thread will have its resources
    ///          automatically released and cannot be joined.
    /// \sa isJoinable join
    ////////////////////////////////////////////////////////////////
    void detach();

    ////////////////////////////////////////////////////////////////
    /// \brief Wait until the thread returns or exits.
    /// \details If the thread is not joinable then this method
    ///          does nothing.
    /// \sa isJoinable
    ////////////////////////////////////////////////////////////////
    void join();



    ////////////////////////////////////////////////////////////////
    /// \brief Check if join can be used.
    /// \details A thread is joinable if it exists, is not detached
    ///          and does not represent the current thread.
    /// \return True if join can be used
    /// \sa detach selfId join
    ////////////////////////////////////////////////////////////////
    bool isJoinable() const;



private:
    std::thread thread;
};



inline Thread::Thread() : thread()
{}

inline Thread::Thread(Thread&& that) : thread(std::move(that.thread))
{}

template <typename F, typename... Args>
inline Thread::Thread(F func, Args&&... args) : thread(std::forward<F>(func), std::forward<Args>(args)...)
{}

inline Thread::~Thread()
{
    this->join();
}



template <typename F, typename... Args>
inline Thread& Thread::launch(F func, Args&&... args)
{
    if(this->isJoinable())
        throw;

    this->thread = thread(std::forward<F>(func), std::forward<Args>(args)...);
    return (*this);
}



inline void Thread::detach()
{
    if(this->isJoinable())
        this->thread.detach();
}

inline void Thread::join()
{
    if(this->isJoinable())
        this->thread.join();
}



inline bool Thread::isJoinable() const
{
    return this->thread.joinable();
}

#endif
