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

size_t __ctype_get_mb_cur_max(void);
#define MB_CUR_MAX (__ctype_get_mb_cur_max())

void *malloc(size_t size);
void *calloc(size_t count, size_t size);
void *realloc(void *ptr, size_t size);
void free(void *ptr);

/* The 48-bit generators and random(), src/extras_standalone.nelua. */
double drand48(void);
double erand48(unsigned short [3]);
long lrand48(void);
long nrand48(unsigned short [3]);
long mrand48(void);
long jrand48(unsigned short [3]);
void srand48(long);
unsigned short *seed48(unsigned short [3]);
void lcong48(unsigned short [7]);
long random(void);
void srandom(unsigned int);
char *initstate(unsigned int, char *, size_t);
char *setstate(char *);
char *mktemp(char *);

/* Pseudo-terminals, src/termios_standalone.nelua. */
int posix_openpt(int flags);
int grantpt(int fd);
int unlockpt(int fd);
char *ptsname(int fd);
int ptsname_r(int fd, char *buf, size_t size);

/* XSI DES key schedule for encrypt(), src/crypt_standalone.nelua. */
void setkey(const char *);
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
