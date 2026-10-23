#define _GNU_SOURCE
#include <dlfcn.h>
#include <errno.h>
#include <semaphore.h>
#include <time.h>

/* glibc checks tv_nsec before its fast acquisition path. POSIX permits an
 * immediate lock to succeed without validating the deadline, so preserve that
 * behavior here and hand every blocking wait back to glibc. */
typedef int (*glibc_sem_clockwait_fn)(sem_t *, clockid_t,
                                      const struct timespec *);

static glibc_sem_clockwait_fn resolve_glibc_sem_clockwait(void) {
  return (glibc_sem_clockwait_fn)
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
  glibc_sem_clockwait_fn host = resolve_glibc_sem_clockwait();
  if (host == 0) {
    errno = ENOSYS;
    return -1;
  }
  return host(sem, clock_id, abstime);
}
