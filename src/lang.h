#ifndef GASMIC_LANG_H
#define GASMIC_LANG_H

#if __STDC_VERSION >= 199901L
#include <stdbool.h>
#define INLINE inline
#else
typedef enum { false, true } bool;
#define INLINE
#endif

/* print to stderr */
int printef(const char *msg, ...);

#endif /* GASMIC_LANG_H */
