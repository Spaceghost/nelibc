#ifndef NELIBC_SETJMP_WRAPPER_H
#define NELIBC_SETJMP_WRAPPER_H

/* Hybrid builds use the host's <setjmp.h>. The standalone and host profiles
 * provide setjmp, _setjmp, longjmp, _longjmp, sigsetjmp and siglongjmp from
 * src/setjmp_x86_64.S, with musl's jmp_buf layout. */
#ifdef NELIBC_STANDALONE

#ifdef __cplusplus
extern "C" {
#endif

#if defined(__GNUC__) || defined(__clang__)
#define NELIBC_RETURNS_TWICE __attribute__((__returns_twice__))
#define NELIBC_SETJMP_NORETURN __attribute__((__noreturn__))
#else
#define NELIBC_RETURNS_TWICE
#define NELIBC_SETJMP_NORETURN
#endif

typedef unsigned long __jmp_buf[8];

typedef struct __jmp_buf_tag {
  __jmp_buf __jb;
  unsigned long __fl;
  unsigned long __ss[128 / sizeof(long)];
} jmp_buf[1];

int setjmp(jmp_buf) NELIBC_RETURNS_TWICE;
NELIBC_SETJMP_NORETURN void longjmp(jmp_buf, int);

#if !defined(__STRICT_ANSI__) || defined(_POSIX_SOURCE) || defined(_POSIX_C_SOURCE) || \
    defined(_XOPEN_SOURCE) || defined(_GNU_SOURCE) || defined(_BSD_SOURCE) || defined(_DEFAULT_SOURCE)
typedef jmp_buf sigjmp_buf;
int sigsetjmp(sigjmp_buf, int) NELIBC_RETURNS_TWICE;
NELIBC_SETJMP_NORETURN void siglongjmp(sigjmp_buf, int);
#endif

#if !defined(__STRICT_ANSI__) || defined(_XOPEN_SOURCE) || defined(_GNU_SOURCE) || \
    defined(_BSD_SOURCE) || defined(_DEFAULT_SOURCE)
int _setjmp(jmp_buf) NELIBC_RETURNS_TWICE;
NELIBC_SETJMP_NORETURN void _longjmp(jmp_buf, int);
#endif

#define setjmp setjmp

#undef NELIBC_RETURNS_TWICE
#undef NELIBC_SETJMP_NORETURN

#ifdef __cplusplus
}
#endif

#else

#include_next <setjmp.h>

#endif

#endif
