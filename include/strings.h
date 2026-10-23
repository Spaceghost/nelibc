#ifndef NELIBC_STRINGS_H
#define NELIBC_STRINGS_H

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

#endif
