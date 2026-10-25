#ifndef NELIBC_SCHED_WRAPPER_H
#define NELIBC_SCHED_WRAPPER_H

#ifdef NELIBC_STANDALONE

#include <stddef.h>
#include <sys/types.h>
#include <time.h>

#define SCHED_OTHER 0
#define SCHED_FIFO 1
#define SCHED_RR 2
#define SCHED_BATCH 3
#define SCHED_IDLE 5
#define SCHED_RESET_ON_FORK 0x40000000

struct sched_param {
  int sched_priority;
};

#define CPU_SETSIZE 1024
#define __NELIBC_CPU_BITS (8 * sizeof(unsigned long))

typedef struct { unsigned long __bits[CPU_SETSIZE / __NELIBC_CPU_BITS]; }
    cpu_set_t;

#define CPU_ZERO(set) do { \
  for (size_t __cpu_i = 0; __cpu_i < sizeof(*(set)) / sizeof(unsigned long); \
       ++__cpu_i) ((unsigned long *)(set)->__bits)[__cpu_i] = 0; \
} while (0)
#define CPU_SET(cpu, set) do { \
  size_t __cpu_n = (size_t)(cpu); \
  if (__cpu_n < CPU_SETSIZE) \
    (set)->__bits[__cpu_n / __NELIBC_CPU_BITS] |= \
        1UL << (__cpu_n % __NELIBC_CPU_BITS); \
} while (0)
#define CPU_CLR(cpu, set) do { \
  size_t __cpu_n = (size_t)(cpu); \
  if (__cpu_n < CPU_SETSIZE) \
    (set)->__bits[__cpu_n / __NELIBC_CPU_BITS] &= \
        ~(1UL << (__cpu_n % __NELIBC_CPU_BITS)); \
} while (0)
#define CPU_ISSET(cpu, set) \
  ((size_t)(cpu) < CPU_SETSIZE && \
   (((set)->__bits[(size_t)(cpu) / __NELIBC_CPU_BITS] >> \
     ((size_t)(cpu) % __NELIBC_CPU_BITS)) & 1UL) != 0)

int sched_yield(void);
int sched_getaffinity(pid_t process, size_t size, cpu_set_t *mask);
int sched_setaffinity(pid_t process, size_t size, const cpu_set_t *mask);

#else

#include_next <sched.h>
#include "nelibc_syscall.h"

#endif

int sched_get_priority_max(int policy);
int sched_get_priority_min(int policy);
int sched_getparam(pid_t pid, struct sched_param *param);
int sched_getscheduler(pid_t pid);
int sched_setparam(pid_t pid, const struct sched_param *param);
int sched_setscheduler(pid_t pid, int policy,
                       const struct sched_param *param);
int sched_rr_get_interval(pid_t pid, struct timespec *interval);

#endif
