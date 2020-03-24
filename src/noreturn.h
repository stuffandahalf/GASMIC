#ifndef GASMIC_NORETURN_H
#define GASMIC_NORETURN_H

#if defined(NORETURN_START) || defined(NORETURN_END)
#error NORETURN macros are already defined
#endif

#if (defined(__cplusplus) && __cplusplus >= 201103L) || (defined(_MSVC_LANG) && _MSVC_LANG >= 201103L)
#define NORETURN_START	[[ noreturn ]]
#define NORETURN_END
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
#define NORETURN_START	_Noreturn
#define NORETURN_END
#else
#if defined(_MSC_VER)
#define NORETURN_START	__declspec(noreturn)
#define NORETURN_END
#elif defined(__GNUC__)
#define NORETURN_START
#define NORETURN_END	__attribute__((noreturn))
#else
#define NORETURN_START	/**/
#define NORETURN_END	/**/
#endif /* Compiler specific extensions */
#endif /* Standard C or C++ noreturn attribute */

#endif