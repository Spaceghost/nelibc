#ifndef NELIBC_SYS_PIDFD_WRAPPER_H
#define NELIBC_SYS_PIDFD_WRAPPER_H

#include <fcntl.h>
#ifdef NELIBC_STANDALONE
#include <sys/types.h>
#include <signal.h>
struct siginfo;
typedef struct siginfo siginfo_t;
#elif defined(__GLIBC__)
#include_next <sys/pidfd.h>
#else
#include <sys/types.h>
#include <signal.h>
#endif

#ifndef PIDFD_NONBLOCK
#define PIDFD_NONBLOCK O_NONBLOCK
#endif

/* Linux pidfd interfaces are exposed by glibc under _GNU_SOURCE. */
#if defined(_GNU_SOURCE) && (!defined(__GLIBC__) || defined(NELIBC_STANDALONE))
int pidfd_open(pid_t pid, unsigned int flags);
int pidfd_send_signal(int pidfd, int signal, siginfo_t *info,
                      unsigned int flags);
#endif

#endif
