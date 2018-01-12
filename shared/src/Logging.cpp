#include <Logging.hpp>
#include <Mutex.hpp>
#include <Lock.hpp>
#include <cstdio>
#include <cstdarg>


namespace
{
    Mutex stdout_logging_mutex;
}



void print(const char* fmt, ...)
{
    Lock lock(stdout_logging_mutex);
    va_list va;
    va_start(va, fmt);

    vprintf(fmt, va);
    fflush(stdout);

    va_end(va);
}
void error(const char* fmt, ...)
{
    Lock lock(stdout_logging_mutex);
    va_list va;
    va_start(va, fmt);

    printf("error: ");
    vprintf(fmt, va);
    putchar('\n');
    fflush(stdout);

    va_end(va);
}
void warning(const char* fmt, ...)
{
    Lock lock(stdout_logging_mutex);
    va_list va;
    va_start(va, fmt);

    printf("warning: ");
    vprintf(fmt, va);
    putchar('\n');
    fflush(stdout);

    va_end(va);
}
void info(const char* fmt, ...)
{
    Lock lock(stdout_logging_mutex);
    va_list va;
    va_start(va, fmt);

    printf("info: ");
    vprintf(fmt, va);
    putchar('\n');
    fflush(stdout);

    va_end(va);
}
