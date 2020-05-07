#ifndef GASMIC_NORETURN_H
#define GASMIC_NORETURN_H

#if defined(NORETURN)
#error NORETURN macro is already defined
#endif

#if (defined(__cplusplus) && __cplusplus >= 201103L) || (defined(_MSVC_LANG) && _MSVC_LANG >= 201103L)
#define NORETURN	[[ noreturn ]]
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
#define NORETURN	_Noreturn
#else
#if defined(_MSC_VER)
#define NORETURN	__declspec(noreturn)
#elif defined(__GNUC__)
#define NORETURN	__attribute__((__noreturn__))
#else
#define NORETURN	/**/
#endif /* Compiler specific extensions */
#endif /* Standard C or C++ noreturn attribute */

#endif /* defined(GASMIC_NORETURN_H) */
