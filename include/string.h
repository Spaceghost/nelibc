#ifndef NELIBC_STRING_H
#define NELIBC_STRING_H

#ifndef NELIBC_STANDALONE
#include <features.h>
#endif
#include <stddef.h>

size_t strlen(const char *s);
size_t strnlen(const char *s, size_t maxlen);
char *strdup(const char *s);
char *strndup(const char *s, size_t n);
int strcmp(const char *a, const char *b);
int strverscmp(const char *left, const char *right);
int strcasecmp(const char *a, const char *b);
int strcoll(const char *a, const char *b);
int strncmp(const char *a, const char *b, size_t n);
int strncasecmp(const char *a, const char *b, size_t n);
size_t strxfrm(char *restrict dest, const char *restrict src, size_t n);
char *strcpy(char *restrict dest, const char *restrict src);
size_t strlcpy(char *restrict dest, const char *restrict src, size_t dstsize);
size_t strlcat(char *restrict dest, const char *restrict src, size_t dstsize);
char *stpcpy(char *restrict dest, const char *restrict src);
char *strcat(char *restrict dest, const char *restrict src);
char *strncat(char *restrict dest, const char *restrict src, size_t n);
char *strncpy(char *restrict dest, const char *restrict src, size_t n);
char *stpncpy(char *restrict dest, const char *restrict src, size_t n);
char *strchr(const char *s, int c);
char *strchrnul(const char *s, int c);
char *strrchr(const char *s, int c);
char *strstr(const char *haystack, const char *needle);
char *strcasestr(const char *haystack, const char *needle);
size_t strspn(const char *s, const char *accept);
size_t strcspn(const char *s, const char *reject);
char *strpbrk(const char *s, const char *accept);
char *strtok(char *restrict s, const char *restrict delim);
char *strtok_r(char *restrict s, const char *restrict delim, char **restrict saveptr);
char *strsep(char **restrict stringp, const char *restrict delim);
char *strerror(int errnum);
int nelibc_strerror_r(int errnum, char *buf, size_t buflen);
char *nelibc_gnu_strerror_r(int errnum, char *buf, size_t buflen);
/* Select GNU or XSI declarations by feature mode; hybrid GNU stays on host. */
#if defined(_GNU_SOURCE) && defined(NELIBC_STANDALONE)
#define strerror_r nelibc_gnu_strerror_r
#elif defined(_GNU_SOURCE) && defined(__GLIBC__)
char *strerror_r(int errnum, char *buf, size_t buflen);
#else
#define strerror_r nelibc_strerror_r
#endif
void *memcpy(void *restrict dest, const void *restrict src, size_t n);
void *mempcpy(void *restrict dest, const void *restrict src, size_t n);
void *memmove(void *dest, const void *src, size_t n);
void *memccpy(void *restrict dest, const void *restrict src, int c, size_t n);
void *memset(void *dest, int value, size_t n);
void explicit_bzero(void *dest, size_t n);
int memcmp(const void *a, const void *b, size_t n);
void *memchr(const void *s, int c, size_t n);
void *memrchr(const void *s, int c, size_t n);
void *memmem(const void *haystack, size_t haystack_len, const void *needle, size_t needle_len);

#endif
