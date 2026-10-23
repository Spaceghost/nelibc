#ifndef NELIBC_SYS_POLL_WRAPPER_H
#define NELIBC_SYS_POLL_WRAPPER_H

#if defined(__GLIBC__) && !defined(NELIBC_STANDALONE)
#include_next <sys/poll.h>
#else
#include <poll.h>
#endif

#endif
