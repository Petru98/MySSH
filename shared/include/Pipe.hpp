#ifndef INCLUDED_PIPE_HPP
#define INCLUDED_PIPE_HPP

#include "FileDescriptor.hpp"
#include <system_error>



class Pipe
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

    class CreateError  : public Error {public: using Error::Error;};
    class SetSizeError : public Error {public: using Error::Error;};



    Pipe(int flags = 0);
    ~Pipe();

    int getReadFD() const;
    int getWriteFD() const;

    std::size_t read(void* buffer, std::size_t size);
    std::size_t write(const void* buffer, std::size_t size);

    void setSize(std::size_t size);



protected:
    FileDescriptor fd[2];
};

#endif





////////////////////////////////////////////////////////////////////////////////
/// \class Pipe
/// \ingroup shared
////////////////////////////////////////////////////////////////////////////////
