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

long fsize(FILE *fptr)
{
    long current_pos;
    long size;
	
    current_pos = ftell(fptr);

    fseek(fptr, 0, SEEK_END);
    size = ftell(fptr);
    fseek(fptr, current_pos, SEEK_SET);

    return size;
}
