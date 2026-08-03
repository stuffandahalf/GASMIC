#ifndef GASMIC_SMEM_H
#define GASMIC_SMEM_H

#include <stdio.h>
#include <stdlib.h>
#include "noreturn.h"

void release(void);
void *salloc(size_t size);
void *srealloc(void *ptr, size_t size);
void *saquire(void *ptr);
void sfree(void *ptr);
NORETURN void die(const char *msg, ...);

#ifndef NDEBUG
void smem_diagnostic(void);
#endif

#endif /* GASMIC_SMEM_H */
