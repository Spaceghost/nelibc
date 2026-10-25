#ifndef NELIBC_SYS_TIME_WRAPPER_H
#define NELIBC_SYS_TIME_WRAPPER_H

#ifdef NELIBC_STANDALONE

#ifndef __timeval_defined
#define __timeval_defined 1
struct timeval {
  long tv_sec;
  long tv_usec;
};
#endif

#ifndef __itimerval_defined
#define __itimerval_defined 1
struct itimerval {
  struct timeval it_interval;
  struct timeval it_value;
};
#endif

#ifndef ITIMER_REAL
#define ITIMER_REAL 0
#define ITIMER_VIRTUAL 1
#define ITIMER_PROF 2
#endif

#ifndef __timezone_defined
#define __timezone_defined 1
struct timezone {
  int tz_minuteswest;
  int tz_dsttime;
};
#endif

#include <sys/types.h>
#include <time.h>

int gettimeofday(struct timeval *restrict tv, void *restrict timezone);
int getitimer(int which, struct itimerval *current_value);
int setitimer(int which, const struct itimerval *new_value,
              struct itimerval *old_value);

#define timerclear(tv) ((tv)->tv_sec = (tv)->tv_usec = 0)
#define timerisset(tv) ((tv)->tv_sec != 0 || (tv)->tv_usec != 0)
#define timercmp(a, b, CMP) \
  ((a)->tv_sec == (b)->tv_sec ? (a)->tv_usec CMP (b)->tv_usec : \
                               (a)->tv_sec CMP (b)->tv_sec)
#define timeradd(a, b, result) do { \
  (result)->tv_sec = (a)->tv_sec + (b)->tv_sec; \
  (result)->tv_usec = (a)->tv_usec + (b)->tv_usec; \
  if ((result)->tv_usec >= 1000000) { \
    ++(result)->tv_sec; \
    (result)->tv_usec -= 1000000; \
  } \
} while (0)
#define timersub(a, b, result) do { \
  (result)->tv_sec = (a)->tv_sec - (b)->tv_sec; \
  (result)->tv_usec = (a)->tv_usec - (b)->tv_usec; \
  if ((result)->tv_usec < 0) { \
    --(result)->tv_sec; \
    (result)->tv_usec += 1000000; \
  } \
} while (0)

#else

#include_next <sys/time.h>

#endif

/* Add Nelibc's Linux syscall declarations to the standard time API header. */
#include "nelibc_syscall.h"

#endif
