#include <lang.h>
#include <stdio.h>
#include <stdarg.h>

int printef(const char *msg, ...)
{
    va_list args;
    va_start(args, msg);

    vfprintf(stderr, msg, args);

    va_end(args);
}
