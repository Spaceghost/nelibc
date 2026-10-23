#ifndef NELIBC_SYS_UIO_WRAPPER_H
#define NELIBC_SYS_UIO_WRAPPER_H

#ifdef NELIBC_STANDALONE
#include <stddef.h>

#ifndef __iovec_defined
#define __iovec_defined 1
struct iovec {
  void *iov_base;
  size_t iov_len;
};
#endif
#else
#include_next <sys/uio.h>
#endif

/* Add Nelibc's Linux vectored-I/O declarations to the standard uio API. */
#include "nelibc_syscall.h"

#endif
