#include <errno.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* The fixture uses the real thread runtime without linking stdio. */
int fflush(FILE *stream) {
  (void)stream;
  return 0;
}

static _Thread_local int tls = 17;
static _Thread_local _Alignas(64) int zeroed;
static pthread_key_t key;
static atomic_int destroyed;
static atomic_int detached_done;
static atomic_int worker_ready;
static int token;

static void destroy(void *value) {
  if (value == &token) atomic_fetch_add(&destroyed, 1);
}

static int check_direct_allocations(void) {
  for (unsigned char i = 1; i <= 16; ++i) {
    volatile unsigned char *allocation = malloc(65536);
    if (!allocation) return 0;
    allocation[0] = i;
    allocation[65535] = i + 1;
    int valid = allocation[0] == i && allocation[65535] == i + 1;
    free((void *)allocation);
    if (!valid) return 0;
  }
  return 1;
}

static void *worker(void *argument) {
  if (tls != 17 || zeroed || errno || pthread_getspecific(key) ||
      pthread_setspecific(key, &token) || !check_direct_allocations()) return 0;
  tls = 42;
  zeroed = 1;
  errno = EINVAL;
  if (argument == &detached_done) atomic_store(&detached_done, 1);
  return argument;
}

static void *cancelled(void *argument) {
  (void)argument;
  struct timespec wait = {.tv_sec = 10};
  atomic_store(&worker_ready, 1);
  nanosleep(&wait, 0);
  return 0;
}

int main(void) {
  pthread_t thread;
  void *result;
  if (pthread_key_create(&key, destroy)) return 1;
  tls = 29;
  errno = EBUSY;
  if (pthread_create(&thread, 0, worker, &token) ||
      !check_direct_allocations() ||
      pthread_join(thread, &result) || result != &token ||
      tls != 29 || zeroed || errno != EBUSY ||
      atomic_load(&destroyed) != 1) return 2;

  pthread_attr_t attr;
  if (pthread_attr_init(&attr) ||
      pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED) ||
      pthread_create(&thread, &attr, worker, &detached_done) ||
      pthread_attr_destroy(&attr)) return 3;
  const struct timespec pause = {.tv_nsec = 1000000};
  for (int i = 0; i < 2000 && atomic_load(&destroyed) != 2; ++i)
    nanosleep(&pause, 0);
  if (!atomic_load(&detached_done) || atomic_load(&destroyed) != 2) return 4;

  if (pthread_create(&thread, 0, cancelled, 0)) return 5;
  for (int i = 0; i < 2000 && !atomic_load(&worker_ready); ++i)
    nanosleep(&pause, 0);
  if (!atomic_load(&worker_ready) || pthread_cancel(thread) ||
      pthread_join(thread, &result) || result != PTHREAD_CANCELED) return 6;
  return pthread_key_delete(key);
}
