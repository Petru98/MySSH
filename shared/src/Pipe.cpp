#include <Pipe.hpp>
#include <unistd.h>
#include <fcntl.h>



Pipe::Pipe(int flags)
{
    int pipefd[2];

    int error = pipe2(pipefd, flags);
    if(error == -1)
        throw CreateError(error, flags);

    this->fd[0].setFD(pipefd[0]);
    this->fd[1].setFD(pipefd[1]);
}
Pipe::~Pipe()
{
    ::close(this->fd[0].getFD());
    ::close(this->fd[1].getFD());
}



int Pipe::getReadFD() const
{
    return this->fd[0].getFD();
}
int Pipe::getWriteFD() const
{
    return this->fd[1].getFD();
}



std::size_t Pipe::read(void* buffer, std::size_t size)
{
    return this->fd[0].read(buffer, size);
}
std::size_t Pipe::write(const void* buffer, std::size_t size)
{
    return this->fd[1].write(buffer, size);
}

void Pipe::setSize(std::size_t size)
{
    int error = fcntl(this->fd, F_SETPIPE_SZ, static_cast<int>(size));
    if(error == -1)
        throw SetSizeError(errno, size);
}
