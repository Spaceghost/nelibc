#ifndef NELIBC_STDIO_H
#define NELIBC_STDIO_H

#ifdef NELIBC_STANDALONE
#include "nelibc_stdio_standalone.h"
#else
#include <stdio.h>
#endif

#define NELIBC_EOF (-1)

int putchar(int character);
int getchar(void);
int puts(const char *string);
int fputc(int character, FILE *stream);
int fputs(const char *restrict string, FILE *restrict stream);
void perror(const char *string);

#endif
