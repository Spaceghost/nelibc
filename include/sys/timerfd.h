#ifndef NELIBC_SYS_TIMERFD_WRAPPER_H
#define NELIBC_SYS_TIMERFD_WRAPPER_H

#ifdef NELIBC_STANDALONE
#include <fcntl.h>
#include <time.h>

#ifndef TFD_CLOEXEC
#define TFD_CLOEXEC O_CLOEXEC
#endif
#ifndef TFD_NONBLOCK
#define TFD_NONBLOCK O_NONBLOCK
#endif
#ifndef TFD_TIMER_ABSTIME
#define TFD_TIMER_ABSTIME (1 << 0)
#endif
#ifndef TFD_TIMER_CANCEL_ON_SET
#define TFD_TIMER_CANCEL_ON_SET (1 << 1)
#endif
#else
#include_next <sys/timerfd.h>
#endif

int timerfd_create(clockid_t clock_id, int flags);
int timerfd_settime(int fd, int flags, const struct itimerspec *new_value,
                    struct itimerspec *old_value);
int timerfd_gettime(int fd, struct itimerspec *current_value);

#endif
