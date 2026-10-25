#ifndef NELIBC_SYS_SELECT_WRAPPER_H
#define NELIBC_SYS_SELECT_WRAPPER_H

#ifdef NELIBC_STANDALONE

#ifndef FD_SETSIZE
#define FD_SETSIZE 1024
#endif

typedef struct {
  unsigned long __bits[FD_SETSIZE / (8 * sizeof(unsigned long))];
} fd_set;

#include <stddef.h>
#include <sys/time.h>

#define FD_ZERO(set) do { \
  unsigned long *__fd_bits = (unsigned long *)(set); \
  for (size_t __fd_index = 0; \
       __fd_index < FD_SETSIZE / (8 * sizeof(unsigned long)); ++__fd_index) \
    __fd_bits[__fd_index] = 0; \
} while (0)
#define FD_SET(fd, set) \
  (((unsigned long *)(set))[(fd) / (8 * sizeof(unsigned long))] |= \
   1UL << ((fd) % (8 * sizeof(unsigned long))))
#define FD_CLR(fd, set) \
  (((unsigned long *)(set))[(fd) / (8 * sizeof(unsigned long))] &= \
   ~(1UL << ((fd) % (8 * sizeof(unsigned long)))))
#define FD_ISSET(fd, set) \
  ((((const unsigned long *)(set))[(fd) / (8 * sizeof(unsigned long))] & \
    (1UL << ((fd) % (8 * sizeof(unsigned long))))) != 0)

#else

#include_next <sys/select.h>

#endif

/* Add Nelibc's Linux syscall declarations to the standard select API. */
#include "nelibc_syscall.h"

#ifdef NELIBC_STANDALONE
int pselect(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds,
            const struct timespec *timeout, const sigset_t *sigmask);
#endif

#endif
