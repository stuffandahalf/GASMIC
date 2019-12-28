#ifndef GASMIC_SMEM_H
#define GASMIC_SMEM_H

#include <stdio.h>
#include <stdlib.h>

void release(void);
void *salloc(size_t size);
void *srealloc(void *ptr, size_t size);
void *saquire(void *ptr);
void sfree(void *ptr);
void die(const char *msg, ...);
#ifndef NDEBUG
void smem_diagnostic(void);
#endif

#if defined(_WIN32) && !defined(NDEBUG)
#define AWAIT_WINDOWS getc(stdin)
#else
#define AWAIT_WINDOWS
#endif

#endif /* GASMIC_SMEM_H */
