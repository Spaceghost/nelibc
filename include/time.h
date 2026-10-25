#ifndef NELIBC_TIME_WRAPPER_H
#define NELIBC_TIME_WRAPPER_H

#include <stddef.h>

#ifdef NELIBC_STANDALONE

#include <sys/types.h>
struct sigevent;

#ifndef __timer_t_defined
#define __timer_t_defined 1
typedef int timer_t;
#endif

#ifndef _STRUCT_TIMESPEC
#define _STRUCT_TIMESPEC 1
struct timespec {
  time_t tv_sec;
  long tv_nsec;
};
#endif

#ifndef __itimerspec_defined
#define __itimerspec_defined 1
struct itimerspec {
  struct timespec it_interval;
  struct timespec it_value;
};
#endif

struct tm {
  int tm_sec;
  int tm_min;
  int tm_hour;
  int tm_mday;
  int tm_mon;
  int tm_year;
  int tm_wday;
  int tm_yday;
  int tm_isdst;
  long tm_gmtoff;
  const char *tm_zone;
};

#ifndef CLOCKS_PER_SEC
#define CLOCKS_PER_SEC 1000000
#endif
#ifndef CLOCK_REALTIME
#define CLOCK_REALTIME 0
#define CLOCK_MONOTONIC 1
#define CLOCK_PROCESS_CPUTIME_ID 2
#define CLOCK_THREAD_CPUTIME_ID 3
#define CLOCK_MONOTONIC_RAW 4
#define CLOCK_REALTIME_COARSE 5
#define CLOCK_MONOTONIC_COARSE 6
#define CLOCK_BOOTTIME 7
#define CLOCK_REALTIME_ALARM 8
#define CLOCK_BOOTTIME_ALARM 9
#define CLOCK_SGI_CYCLE 10
#define CLOCK_TAI 11
#endif
#ifndef TIMER_ABSTIME
#define TIMER_ABSTIME 1
#endif
#ifndef TIME_UTC
#define TIME_UTC 1
#endif

int clock_gettime(clockid_t clock_id, struct timespec *time_value);
int clock_getres(clockid_t clock_id, struct timespec *resolution);
int nanosleep(const struct timespec *requested, struct timespec *remaining);
int clock_nanosleep(clockid_t clock_id, int flags,
                    const struct timespec *requested,
                    struct timespec *remaining);
time_t time(time_t *timer);
int timer_create(clockid_t clock_id, struct sigevent *event, timer_t *timer_id);
int timer_delete(timer_t timer_id);
int timer_getoverrun(timer_t timer_id);
int timer_gettime(timer_t timer_id, struct itimerspec *current_value);
int timer_settime(timer_t timer_id, int flags,
                  const struct itimerspec *new_value,
                  struct itimerspec *old_value);

#else

#include_next <time.h>
#include "nelibc_syscall.h"

#endif

void tzset(void);
extern char *tzname[2];
extern int daylight;
extern long timezone;

int timespec_get(struct timespec *time_value, int base);
int timespec_getres(struct timespec *resolution, int base);
clock_t clock(void);
double difftime(time_t time1, time_t time0);
struct tm *gmtime_r(const time_t *restrict timer, struct tm *restrict result);
struct tm *gmtime(const time_t *timer);
struct tm *localtime_r(const time_t *restrict timer, struct tm *restrict result);
struct tm *localtime(const time_t *timer);
time_t mktime(struct tm *time_value);
time_t timegm(struct tm *time_value);
char *strptime(const char *restrict input, const char *restrict format,
               struct tm *restrict time_value);
char *asctime_r(const struct tm *restrict time_value, char *restrict buffer);
char *asctime(const struct tm *time_value);
char *ctime_r(const time_t *restrict timer, char *restrict buffer);
char *ctime(const time_t *timer);
size_t strftime(char *restrict buffer, size_t max_size,
                const char *restrict format, const struct tm *restrict time_value);

#endif
