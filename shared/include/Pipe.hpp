#ifndef INCLUDED_PIPE_HPP
#define INCLUDED_PIPE_HPP

#include "FileDescriptor.hpp"



class Pipe
{
public:
    class Error : std::exception
    {
    public:
        Error() : error(0) {}
        Error(int error) : error(error) {}

    protected:
        int error;
    };

    class CreateError : public Error
    {
    public:
        CreateError() : Error(0), flags(0) {}
        CreateError(int error, int flags) Error(error), flags(flags) {}

        const char* what() {return "could not create pipe";}

    protected:
        int flags;
    };

    class SetSizeError : public Error
    {
    public:
        SetSizeError() : Error(0), size(0) {}
        SetSizeError(int error, std::size_t size) Error(error), size(size) {}

        const char* what() {return "could not set pipe size";}

    protected:
        std::size_t size;
    };



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
