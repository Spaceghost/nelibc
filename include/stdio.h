#ifndef NELIBC_STDIO_WRAPPER_H
#define NELIBC_STDIO_WRAPPER_H

#ifdef NELIBC_STANDALONE
#include "nelibc_stdio_standalone.h"
#else
#include_next <stdio.h>

/* Large-file positioning is implemented by Nelibc on 64-bit Linux. */
#if defined(_LARGEFILE_SOURCE) || defined(_GNU_SOURCE)
int fseeko(FILE *stream, off_t offset, int whence);
off_t ftello(FILE *stream);
#endif

/* GNU/BSD stdio extensions provided by the hybrid Nelibc archives. */
#if defined(_DEFAULT_SOURCE) || defined(_GNU_SOURCE)
char *fgets_unlocked(char *restrict buffer, int count, FILE *restrict stream);
int fputs_unlocked(const char *restrict string, FILE *restrict stream);
int getchar_unlocked(void);
int putchar_unlocked(int character);
void setbuffer(FILE *restrict stream, char *restrict buffer, size_t size);
void setlinebuf(FILE *stream);
#endif
#endif

#endif
