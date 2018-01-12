#ifndef INCLUDED_FILEDESCRIPTOR_HPP
#define INCLUDED_FILEDESCRIPTOR_HPP

#include <cstdint>
#include <stdexcept>



class FileDescriptor
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

    class ReadError : public Error
    {
    public:
        ReadError() : Error(), buffer(nullptr), size(0) {}
        ReadError(int error, void* buffer, std::size_t size) : Error(error), buffer(buffer), size(size) {}

        const char* what() {return "could not read from file descriptor";}

    protected:
        void* buffer;
        std::size_t size;
    };

    class WriteError : public Error
    {
    public:
        WriteError() : Error(), buffer(nullptr), size(0) {}
        WriteError(int error, const void* buffer, std::size_t size) : Error(error), buffer(buffer), size(size) {}

        const char* what() {return "could not write to file descriptor";}

    protected:
        const void* buffer;
        std::size_t size;
    };



    FileDescriptor();
    FileDescriptor(int fd);

    int getFD() const;
    void setFD(int fd);

    std::size_t read(void* buffer, std::size_t size);
    std::size_t write(const void* buffer, std::size_t size);



protected:
    int fd;
};

#endif
