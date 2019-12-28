#include <lang.h>
#include <stdio.h>
#include <stdarg.h>

int printef(const char *msg, ...)
{
    int c;
    va_list args;
    va_start(args, msg);

    c = vfprintf(stderr, msg, args);

    va_end(args);

    return c;
}
