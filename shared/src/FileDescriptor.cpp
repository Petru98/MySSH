#include <FileDescriptor.hpp>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>



FileDescriptor::FileDescriptor() : fd(-1)
{}
FileDescriptor::FileDescriptor(int fd) : fd(fd)
{}



int FileDescriptor::getFD() const
{
    return this->fd;
}
void FileDescriptor::setFD(int fd)
{
    this->fd = fd;
}



std::size_t FileDescriptor::read(void* buffer, std::size_t size)
{
    const auto read = ::read(this->fd, buffer, size);
    if(read == -1)
    {
        if(errno == EAGAIN || errno == EWOULDBLOCK)
            return 0;
        throw ReadError(errno, buffer, size);
    }

    return static_cast<std::size_t>(read);
}
std::size_t FileDescriptor::write(const void* buffer, std::size_t size)
{
    const auto written = ::write(this->fd, buffer, size);
    if(written == -1)
    {
        if(errno == EAGAIN || errno == EWOULDBLOCK)
            return 0;
        throw WriteError(errno, buffer, size);
    }

    return static_cast<std::size_t>(written);
}

void FileDescriptor::readAll(void* buffer, std::size_t size)
{
    while(size > 0)
    {
        std::size_t read = this->read(buffer, size);
        buffer = reinterpret_cast<uint8_t*>(buffer) + read;
        size -= read;
    }
}
void FileDescriptor::writeAll(const void* buffer, std::size_t size)
{
    while(size > 0)
    {
        std::size_t written = this->write(buffer, size);
        buffer = reinterpret_cast<const uint8_t*>(buffer) + written;
        size -= written;
    }
}
