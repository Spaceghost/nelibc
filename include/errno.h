#ifndef NELIBC_ERRNO_H
#define NELIBC_ERRNO_H

/* Hybrid builds preserve the host's error-number constants. */
#ifdef NELIBC_STANDALONE
#include "nelibc_errno_linux.h"
#else
#include_next <errno.h>
#endif

#ifdef NELIBC_STANDALONE
#undef errno

#ifdef __cplusplus
extern "C" {
#endif
int *nelibc_errno_location(void);
#ifdef __cplusplus
}
#endif

#define errno (*nelibc_errno_location())
#endif

#endif
