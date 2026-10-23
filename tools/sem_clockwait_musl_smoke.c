#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>

static sem_t blocking_semaphore;
static int waiter_started;

static void *waiter(void *unused) {
  (void)unused;
  struct timespec deadline;
  if (clock_gettime(CLOCK_MONOTONIC, &deadline) != 0) return (void *)1;
  deadline.tv_sec += 2;
  __atomic_store_n(&waiter_started, 1, __ATOMIC_RELEASE);
  return (void *)(long)(sem_clockwait(&blocking_semaphore, CLOCK_MONOTONIC,
                                      &deadline) != 0);
}

int main(void) {
  sem_t semaphore;
  if (sem_init(&semaphore, 0, 1) != 0) return 1;

  struct timespec invalid = { .tv_sec = 0, .tv_nsec = 1000000000L };
  if (sem_clockwait(&semaphore, CLOCK_MONOTONIC, &invalid) != 0) return 2;

  struct timespec expired = { .tv_sec = 0, .tv_nsec = 0 };
  if (sem_clockwait(&semaphore, 99, &expired) != -1 || errno != EINVAL) return 3;
  if (sem_clockwait(&semaphore, CLOCK_MONOTONIC, &expired) != -1 ||
      errno != ETIMEDOUT) return 4;
  if (sem_clockwait(&semaphore, CLOCK_REALTIME, &expired) != -1 ||
      errno != ETIMEDOUT) return 5;

  if (sem_destroy(&semaphore) != 0) return 6;

  if (sem_init(&blocking_semaphore, 0, 0) != 0) return 7;
  pthread_t thread;
  if (pthread_create(&thread, 0, waiter, 0) != 0) return 8;
  while (!__atomic_load_n(&waiter_started, __ATOMIC_ACQUIRE)) { }
  struct timespec pause = { .tv_sec = 0, .tv_nsec = 10000000L };
  if (nanosleep(&pause, 0) != 0) return 9;
  if (sem_post(&blocking_semaphore) != 0) return 10;
  void *result;
  if (pthread_join(thread, &result) != 0 || result != 0) return 11;

  __atomic_store_n(&waiter_started, 0, __ATOMIC_RELAXED);
  if (pthread_create(&thread, 0, waiter, 0) != 0) return 12;
  while (!__atomic_load_n(&waiter_started, __ATOMIC_ACQUIRE)) { }
  if (nanosleep(&pause, 0) != 0) return 13;
  if (pthread_cancel(thread) != 0) return 14;
  if (pthread_join(thread, &result) != 0 || result != PTHREAD_CANCELED) return 15;

  if (sem_destroy(&blocking_semaphore) != 0) return 16;
  return 0;
}
