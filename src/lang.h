#ifndef GASMIC_LANG_H
#define GASMIC_LANG_H

#include <stdio.h>

#if __STDC_VERSION >= 199901L
#include <stdbool.h>
#define INLINE inline
#else
typedef enum { false, true } bool;
#define INLINE
#endif

/* print to stderr */
int printef(const char *msg, ...);

#ifndef NDEBUG
#define printdf(args) { \
    printf("[%s:%d] >> ", __FILE__, __LINE__); \
    printf args; \
}
#else
#define printdf(args)
#endif

#endif /* GASMIC_LANG_H */
