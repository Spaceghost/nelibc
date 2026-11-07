#ifndef NELIBC_ICONV_WRAPPER_H
#define NELIBC_ICONV_WRAPPER_H

/* Hybrid builds use the host's <iconv.h>. The standalone profile provides
 * src/iconv_standalone.nelua (after musl 1.2.5; charsets listed there). */
#ifdef NELIBC_STANDALONE

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void *iconv_t;

iconv_t iconv_open(const char *, const char *);
size_t iconv(iconv_t, char **__restrict, size_t *__restrict, char **__restrict, size_t *__restrict);
int iconv_close(iconv_t);

#ifdef __cplusplus
}
#endif

#else

#include_next <iconv.h>

#endif

#endif
