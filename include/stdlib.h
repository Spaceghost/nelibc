#ifndef NELIBC_STDLIB_WRAPPER_H
#define NELIBC_STDLIB_WRAPPER_H

/* Declare Nelibc extensions before glibc's function-like bsearch macro. */
#include "nelibc_stdlib.h"
#ifdef NELIBC_STANDALONE
#include <stddef.h>

typedef struct { int quot; int rem; } div_t;
typedef struct { long quot; long rem; } ldiv_t;
typedef struct { long long quot; long long rem; } lldiv_t;

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1
#define RAND_MAX 2147483647

void *malloc(size_t size);
void *calloc(size_t count, size_t size);
void *realloc(void *ptr, size_t size);
void free(void *ptr);
#else
#include_next <stdlib.h>
#endif

div_t div(int numerator, int denominator);
ldiv_t ldiv(long numerator, long denominator);
lldiv_t lldiv(long long numerator, long long denominator);

/* C23 integer conversion entry points used by GNU/C23-mode headers. */
#if defined(_GNU_SOURCE) || defined(NELIBC_STANDALONE) || \
    (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 202311L)
long __isoc23_strtol(const char *restrict, char **restrict, int);
long long __isoc23_strtoll(const char *restrict, char **restrict, int);
unsigned long __isoc23_strtoul(const char *restrict, char **restrict, int);
unsigned long long __isoc23_strtoull(const char *restrict, char **restrict, int);
#endif

#endif
