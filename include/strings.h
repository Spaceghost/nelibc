#ifndef NELIBC_STRINGS_H
#define NELIBC_STRINGS_H

#ifndef NELIBC_STANDALONE
#include <features.h>
#endif
#include <stddef.h>

int ffs(int value);
int ffsl(long value);
int ffsll(long long value);
int bcmp(const void *left, const void *right, size_t count);
void bcopy(const void *source, void *dest, size_t count);
void bzero(void *dest, size_t count);
void explicit_bzero(void *dest, size_t count);
char *index(const char *text, int character);
char *rindex(const char *text, int character);
int strcasecmp(const char *left, const char *right);
int strncasecmp(const char *left, const char *right, size_t count);

/* POSIX.1-2008 locale-object comparisons. */
#if defined(_GNU_SOURCE) || defined(NELIBC_STANDALONE) || \
    (defined(_POSIX_C_SOURCE) && _POSIX_C_SOURCE >= 200809L) || \
    (defined(_XOPEN_SOURCE) && _XOPEN_SOURCE >= 700)
#include <locale.h>
int strcasecmp_l(const char *left, const char *right, locale_t locale);
int strncasecmp_l(const char *left, const char *right, size_t count, locale_t locale);
#endif

#endif
