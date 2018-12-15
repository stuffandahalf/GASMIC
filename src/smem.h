#ifndef __SMEM_H
#define __SMEM_H

#include <stdio.h>
#include <stdlib.h>

#define die(fmt, ...) { \
    release(); \
    fprintf(stderr, fmt, ##__VA_ARGS__); \
    exit(1); \
}

void release();
void *salloc(size_t size);
void *srealloc(void *ptr, size_t size);
void sfree(void *ptr);

#endif
