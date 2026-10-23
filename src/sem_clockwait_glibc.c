#define _GNU_SOURCE
#include <dlfcn.h>
#include <errno.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>

/* glibc checks tv_nsec before its fast acquisition path. POSIX permits an
 * immediate lock to succeed without validating the deadline, so preserve that
 * behavior here and hand every blocking wait back to glibc. */
typedef int (*glibc_sem_clockwait_fn)(sem_t *, clockid_t,
                                      const struct timespec *);

static pthread_once_t sem_clockwait_host_once = PTHREAD_ONCE_INIT;
static glibc_sem_clockwait_fn sem_clockwait_host;

static void resolve_glibc_sem_clockwait(void) {
  sem_clockwait_host = (glibc_sem_clockwait_fn)
      dlvsym(RTLD_NEXT, "sem_clockwait", "GLIBC_2.30");
}

int sem_clockwait(sem_t *sem, clockid_t clock_id,
                  const struct timespec *abstime) {
  if (clock_id != CLOCK_REALTIME && clock_id != CLOCK_MONOTONIC) {
    errno = EINVAL;
    return -1;
  }
  if (sem_trywait(sem) == 0) return 0;
  if (errno != EAGAIN) return -1;
  int once_error = pthread_once(&sem_clockwait_host_once,
                                resolve_glibc_sem_clockwait);
  if (once_error != 0) {
    errno = once_error;
    return -1;
  }
  if (sem_clockwait_host == 0) {
    errno = ENOSYS;
    return -1;
  }
  return sem_clockwait_host(sem, clock_id, abstime);
}
