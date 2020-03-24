#ifndef GASMIC_SMEM_H
#define GASMIC_SMEM_H

#include <stdio.h>
#include <stdlib.h>
#include "noreturn.h"

/*#define SMEM_CPP*/

void release(void);
#ifdef SMEM_CPP
void *salloc_real(size_t size);
void *srealloc_real(void *ptr, size_t size);
void *saquire_real(void *ptr);
#else
void *salloc(size_t size);
void *srealloc(void *ptr, size_t size);
void *saquire(void *ptr);
#endif
void sfree(void *ptr);
NORETURN void die(const char *msg, ...);

#ifdef SMEM_CPP
#ifdef __cplusplus
#define salloc(T, size)			(T *)(salloc_real(size))
#define srealloc(T, ptr, size)	(T *)(srealloc_real(ptr, size))
#define saquire(T, ptr)			(T *)(saquire_real(ptr))
#else
#define salloc(T, size)			salloc_real(size)
#define srealloc(T, ptr, size)	srealloc_real(ptr, size)
#define saquire(T, ptr)			saquire_real(ptr)
#endif
#endif

#ifndef NDEBUG
void smem_diagnostic(void);
#endif

#if defined(_WIN32) && !defined(NDEBUG)
#define AWAIT_WINDOWS getc(stdin)
#else
#define AWAIT_WINDOWS
#endif

#endif /* GASMIC_SMEM_H */
