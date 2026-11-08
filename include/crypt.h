#ifndef NELIBC_CRYPT_WRAPPER_H
#define NELIBC_CRYPT_WRAPPER_H

/* Hybrid builds use the host's <crypt.h> and crypt. The standalone profile
 * provides src/crypt_standalone.nelua; struct crypt_data is musl's (crypt_r
 * uses it only as the buffer for the result). */
#ifdef NELIBC_STANDALONE

#ifdef __cplusplus
extern "C" {
#endif

struct crypt_data {
  int initialized;
  char __buf[256];
};

char *crypt(const char *, const char *);
char *crypt_r(const char *, const char *, struct crypt_data *);

#ifdef __cplusplus
}
#endif

#else

#include_next <crypt.h>

#endif

#endif
