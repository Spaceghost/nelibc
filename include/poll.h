#ifndef NELIBC_POLL_WRAPPER_H
#define NELIBC_POLL_WRAPPER_H

#ifdef NELIBC_STANDALONE
/* Standalone Linux x86-64 poll ABI. */
#ifndef _SYS_POLL_H
#define _SYS_POLL_H 1
#endif
#ifndef _POLL_H
#define _POLL_H 1
#endif

#ifndef __NELIBC_NFDS_T_DEFINED
typedef unsigned long nfds_t;
#define __NELIBC_NFDS_T_DEFINED 1
#endif

struct pollfd {
  int fd;
  short events;
  short revents;
};

#define POLLIN     0x001
#define POLLPRI    0x002
#define POLLOUT    0x004
#define POLLERR    0x008
#define POLLHUP    0x010
#define POLLNVAL   0x020
#define POLLRDNORM 0x040
#define POLLRDBAND 0x080
#define POLLWRNORM 0x100
#define POLLWRBAND 0x200
#define POLLMSG    0x400
#define POLLRDHUP  0x2000
#else
#include_next <poll.h>
#endif

/* Add Nelibc's Linux syscall declarations to the standard poll API header. */
#include "nelibc_syscall.h"

#endif
