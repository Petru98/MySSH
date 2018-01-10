#include <Logging.hpp>
#include <cstdio>
#include <cstdarg>

void error(const char* fmt, ...)
{
    va_list va;
    va_start(va, fmt);

    puts("error: ");
    vprintf(fmt, va);
    putchar('\n');
    fflush(stdout);

    va_end(va);
}
void warning(const char* fmt, ...)
{
    va_list va;
    va_start(va, fmt);

    puts("warning: ");
    vprintf(fmt, va);
    putchar('\n');
    fflush(stdout);

    va_end(va);
}
void info(const char* fmt, ...)
{
    va_list va;
    va_start(va, fmt);

    puts("info: ");
    vprintf(fmt, va);
    putchar('\n');
    fflush(stdout);

    va_end(va);
}
