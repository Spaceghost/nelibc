#ifndef NELIBC_RESOLV_WRAPPER_H
#define NELIBC_RESOLV_WRAPPER_H

/* Hybrid builds use the host's <resolv.h>. The standalone profile has no
 * resolver: only the DNS name helpers dn_expand and dn_skipname
 * (src/stdio_extras_adapter.c, after musl 1.2.5). */
#ifdef NELIBC_STANDALONE

#ifdef __cplusplus
extern "C" {
#endif

int dn_expand(const unsigned char *, const unsigned char *, const unsigned char *, char *, int);
int dn_skipname(const unsigned char *, const unsigned char *);

#ifdef __cplusplus
}
#endif

#else

#include_next <resolv.h>

#endif

#endif
