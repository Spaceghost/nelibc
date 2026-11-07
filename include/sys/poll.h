#ifndef NELIBC_SYS_POLL_WRAPPER_H
#define NELIBC_SYS_POLL_WRAPPER_H

/* __GLIBC__ is only known once <features.h> has been read, and glibc's own
   <poll.h> reaches this wrapper before any other header has done that. */
#ifndef NELIBC_STANDALONE
#include <features.h>
#endif
#if defined(__GLIBC__) && !defined(NELIBC_STANDALONE)
#include_next <sys/poll.h>
#else
#include <poll.h>
#endif

#endif
