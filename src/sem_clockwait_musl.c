#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <semaphore.h>
#include <time.h>

/*
 * musl releases without sem_clockwait expose the rest of the POSIX semaphore
 * API. Use that opaque API rather than depending on musl's private sem_t
 * layout. Monotonic waits poll at 1 ms intervals; nanosleep remains a
 * cancellation point and bounds the delay after a post.
 */
int sem_clockwait(sem_t *sem, clockid_t clock_id,
                  const struct timespec *abstime) {
  if (clock_id != CLOCK_REALTIME && clock_id != CLOCK_MONOTONIC) {
    errno = EINVAL;
    return -1;
  }
  if (clock_id == CLOCK_REALTIME)
    return sem_timedwait(sem, abstime);

  if (sem_trywait(sem) == 0) return 0;
  if (errno != EAGAIN) return -1;
  if (abstime == 0 || abstime->tv_nsec < 0 ||
      abstime->tv_nsec >= 1000000000L) {
    errno = EINVAL;
    return -1;
  }

  for (;;) {
    struct timespec now;
    if (clock_gettime(CLOCK_MONOTONIC, &now) != 0) return -1;
    if (now.tv_sec > abstime->tv_sec ||
        (now.tv_sec == abstime->tv_sec && now.tv_nsec >= abstime->tv_nsec)) {
      if (sem_trywait(sem) == 0) return 0;
      if (errno != EAGAIN) return -1;
      errno = ETIMEDOUT;
      return -1;
    }

    struct timespec pause = { .tv_sec = 0, .tv_nsec = 1000000L };
    time_t seconds = abstime->tv_sec - now.tv_sec;
    long nanoseconds = abstime->tv_nsec - now.tv_nsec;
    if (nanoseconds < 0) {
      --seconds;
      nanoseconds += 1000000000L;
    }
    if (seconds == 0 && nanoseconds < pause.tv_nsec) {
      pause.tv_nsec = nanoseconds;
    }
    if (nanosleep(&pause, 0) != 0) return -1;
    if (sem_trywait(sem) == 0) return 0;
    if (errno != EAGAIN) return -1;
  }
}
