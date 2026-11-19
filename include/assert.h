#if !defined(NELIBC_STANDALONE)
#include_next <assert.h>
#else

/* The definition depends on NDEBUG at each inclusion. */
#undef assert
#ifdef NDEBUG
#define assert(...) ((void)0)
#else
#include <stdio.h>
#include <stdlib.h>

#ifndef NELIBC_ASSERT_HELPER_H
#define NELIBC_ASSERT_HELPER_H
static inline void nelibc_assert_failure(const char *expression, const char *file,
                                         int line, const char *function)
{
    fprintf(stderr, "%s:%d: %s: assertion failed: %s\n",
            file, line, function, expression);
    abort();
}
#endif

#define assert(...) ((void)((__VA_ARGS__) || \
    (nelibc_assert_failure(#__VA_ARGS__, __FILE__, __LINE__, __func__), 0)))
#endif

#if !defined(__cplusplus) && defined(__STDC_VERSION__) && \
    __STDC_VERSION__ >= 201112L && __STDC_VERSION__ < 202311L
#define static_assert _Static_assert
#endif

#endif
