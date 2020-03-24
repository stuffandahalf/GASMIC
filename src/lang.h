#ifndef GASMIC_LANG_H
#define GASMIC_LANG_H

#include <stdio.h>
#include <string.h>
#include "noreturn.h"

#ifdef __cplusplus
#define INLINE inline
/* bool is a standard type in c++ */
#else
#if __STDC_VERSION__ >= 199901L
#define INLINE inline
#include <stdbool.h>
#else
#define INLINE
typedef enum { false, true } bool;
#endif /* __STDC_VERSION >= 199901L */
#endif /* __cplusplus */

#ifndef NDEBUG
#define printdf(args) { \
	printef("[%s:%d] >> ", __FILE__, __LINE__); \
	printef args; \
}
#else
#define printdf(args)
#endif

#if SIZEOF_SIZE_T == SIZEOF_UL
#define SZuFMT "%lu"
#define SZxFMT "%lx"
#define SZXFMT "%lX"
#elif defined(HAVE_ULL) && SIZEOF_SIZE_T == SIZEOF_ULL
#define SZuFMT "%llu"
#define SZxFMT "%llx"
#define SZXFMT "%llX"
#else
#define SZuFMT "%u"
#define SZxFMT "%x"
#define SZXFMT "%X"
#endif

/* test the equality of 2 strings */
/*#define streq(__s1, __s2) !strcmp((const char *)(__s1), (const char *)(__s2))*/
#define streq(s1, s2) !strcmp((s1), (s2))

/* print to stderr */
int printef(const char *fmt, ...);
long fsize(FILE *fptr);
char *str_clone(const char *src);
char *str_to_upper(char *str);
char *str_trim(char *str);

#endif /* GASMIC_LANG_H */
