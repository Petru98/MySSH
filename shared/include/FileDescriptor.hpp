#ifndef INCLUDED_FILEDESCRIPTOR_HPP
#define INCLUDED_FILEDESCRIPTOR_HPP

#include <cstdint>
#include <system_error>



class FileDescriptor
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

    class ReadError  : public Error {public: using Error::Error;};
    class WriteError : public Error {public: using Error::Error;};



    FileDescriptor();
    FileDescriptor(int fd);

    int getFD() const;
    void setFD(int fd);

    std::size_t read(void* buffer, std::size_t size);
    std::size_t write(const void* buffer, std::size_t size);

    void readAll(void* buffer, std::size_t size);
    void writeAll(const void* buffer, std::size_t size);



protected:
    int fd;
};

#endif





////////////////////////////////////////////////////////////////////////////////
/// \class FileDescriptor
/// \ingroup shared
////////////////////////////////////////////////////////////////////////////////
