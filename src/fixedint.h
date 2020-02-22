#ifndef GASMIC_FIXEDINT_H
#define GASMIC_FIXEDINT_H

//#if (__STDC_VERSION__ >= 199901L || __cplusplus >= 201103L || (defined(_MSVC_LANG) && _MSVC_LANG))
//#include <stdint.h>
#ifdef HAVE_INTTYPES_H
#ifdef __cplusplus
#define __STDC_FORMAT_MACROS
#endif /* __cplusplus */
#include <inttypes.h>
#else
#include <limits.h>

#define BIT_TYPE(base, bits)    base##bits##_t
#define UINT_TYPE(bits)         BIT_TYPE(uint, bits)
#define INT_TYPE(bits)          BIT_TYPE(int, bits)

#define PRId8                   "d"
#define PRIu8                   "u"
#define PRIx8                   "x"
#define PRIX8                   "X"
#define INT8_MIN                SCHAR_MIN
#define INT8_MAX                SCHAR_MAX
#define UINT8_MAX               UCHAR_MAX
typedef unsigned char           UINT_TYPE(CHAR_BIT);
typedef signed char             INT_TYPE(CHAR_BIT);

#if USHRT_MAX == 0xffffu
#define PRId16                  "d"
#define PRIu16                  "u"
#define PRIx16                  "x"
#define PRIX16                  "X"
#define INT16_MIN               SHRT_MIN
#define INT16_MAX               SHRT_MAX
#define UINT16_MAX              USHRT_MAX
typedef unsigned short int      UINT_TYPE(16);
typedef signed short int        INT_TYPE(16);
#elif UINT_MAX == 0xffffu
#define PRId16                  "d"
#define PRIu16                  "u"
#define PRIx16                  "x"
#define PRIX16                  "X"
#define INT16_MIN               INT_MIN
#define INT16_MAX               INT_MAX
#define UINT16_MAX              UINT_MAX
typedef unsigned int			UINT_TYPE(16);
typedef signed int		        INT_TYPE(16);
#else
#error No 16-bit int type exists
#endif

#if UINT_MAX == 0xffffffffUL
#define PRId32                  "d"
#define PRIu32                  "u"
#define PRIx32                  "x"
#define PRIX32                  "X"
#define INT32_MIN               INT_MIN
#define INT32_MAX               INT_MAX
#define UINT32_MAX              UINT_MAX
typedef unsigned int            UINT_TYPE(32);
typedef signed int              INT_TYPE(32);
#elif ULONG_MAX == 0xffffffffUL
#define PRId32                  "ld"
#define PRIu32                  "lu"
#define PRIx32                  "lx"
#define PRIX32                  "lX"
#define INT32_MIN               LONG_MIN
#define INT32_MAX               LONG_MAX
#define UINT32_MAX              ULONG_MAX
typedef unsigned long int       UINT_TYPE(32);
typedef signed long int         INT_TYPE(32);
#else
#error No 32-bit int type exists
#endif

#if ULONG_MAX == 0xffffffffffffffffUL
#define PRId64                  "ld"
#define PRIu64                  "lu"
#define PRIx64                  "lx"
#define PRIX64                  "lX"
#define INT64_MIN               LONG_MIN
#define INT64_MAX               LONG_MAX
#define UINT64_MAX              ULONG_MAX
typedef unsigned long int       UINT_TYPE(64);
typedef signed long int         INT_TYPE(64);
#elif ULLONG_MAX == 0xffffffffffffffffUL
#define PRId64                  "lld"
#define PRIu64                  "llu"
#define PRIx64                  "llx"
#define PRIX64                  "llX"
#define INT64_MIN               LONG_LONG_MIN
#define INT64_MAX               LONG_LONG_MAX
#define UINT64_MAX              ULONG_LONG_MAX
typedef unsigned long long int  UINT_TYPE(64);
typedef signed long long int    INT_TYPE(64);
#else
#error No 64-bit int type exists
#endif

#undef INT_TYPE
#undef UINT_TYPE
#undef BIT_TYPE

#endif /* HAVE_INTTYPES_H */

#endif /* GASMIC_FIXEDINT_H */
