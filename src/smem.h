#ifndef __SMEM_H
#define __SMEM_H

#include <stdio.h>
#include <stdlib.h>

void release(void);
void *salloc(size_t size);
void *srealloc(void *ptr, size_t size);
void *saquire(void *ptr);
void sfree(void *ptr);
#ifndef NDEBUG
void smem_diagnostic(void);
#endif

#define die(fmt, ...) { \
    fprintf(stderr, fmt, ##__VA_ARGS__); \
    release(); \
    exit(1); \
}

#endif
