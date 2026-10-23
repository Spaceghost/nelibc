#ifndef NELIBC_SYS_TIMES_WRAPPER_H
#define NELIBC_SYS_TIMES_WRAPPER_H

#ifdef NELIBC_STANDALONE
#include <time.h>

struct tms {
  clock_t tms_utime;
  clock_t tms_stime;
  clock_t tms_cutime;
  clock_t tms_cstime;
};
clock_t times(struct tms *buffer);
#else
#include_next <sys/times.h>
#endif

/* Add Nelibc's Linux syscall declarations to the standard times API. */
#include "nelibc_syscall.h"

#endif
