#define _GNU_SOURCE
#include <dlfcn.h>
#include <errno.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>

#define IMMEDIATE_ITERATIONS 2000000
#define EMPTY_ITERATIONS 200000
#define BLOCKED_ITERATIONS 200

typedef int (*sem_clockwait_fn)(sem_t *, clockid_t, const struct timespec *);

static uint64_t monotonic_ns(void) {
  struct timespec now;
  if (clock_gettime(CLOCK_MONOTONIC, &now) != 0) return 0;
  return (uint64_t)now.tv_sec * 1000000000ULL + (uint64_t)now.tv_nsec;
}

static int run_immediate(const char *name, sem_clockwait_fn wait_fn) {
  sem_t semaphore;
  if (sem_init(&semaphore, 0, IMMEDIATE_ITERATIONS) != 0) return 1;
  struct timespec deadline;
  if (clock_gettime(CLOCK_MONOTONIC, &deadline) != 0) return 2;
  deadline.tv_sec += 60;

  uint64_t start = monotonic_ns();
  for (int i = 0; i < IMMEDIATE_ITERATIONS; ++i)
    if (wait_fn(&semaphore, CLOCK_MONOTONIC, &deadline) != 0) return 3;
  uint64_t elapsed = monotonic_ns() - start;
  if (sem_destroy(&semaphore) != 0) return 4;

  printf("immediate,%s,%d,%llu,%.2f\n", name, IMMEDIATE_ITERATIONS,
         (unsigned long long)elapsed,
         (double)elapsed / IMMEDIATE_ITERATIONS);
  return 0;
}

static int run_empty_timeout(const char *name, sem_clockwait_fn wait_fn) {
  sem_t semaphore;
  if (sem_init(&semaphore, 0, 0) != 0) return 1;
  const struct timespec expired = { .tv_sec = 0, .tv_nsec = 0 };

  uint64_t start = monotonic_ns();
  for (int i = 0; i < EMPTY_ITERATIONS; ++i) {
    errno = 0;
    if (wait_fn(&semaphore, CLOCK_MONOTONIC, &expired) != -1 ||
        errno != ETIMEDOUT) return 2;
  }
  uint64_t elapsed = monotonic_ns() - start;
  if (sem_destroy(&semaphore) != 0) return 3;

  printf("empty-expired,%s,%d,%llu,%.2f\n", name, EMPTY_ITERATIONS,
         (unsigned long long)elapsed,
         (double)elapsed / EMPTY_ITERATIONS);
  return 0;
}

struct blocked_state {
  sem_t request;
  sem_t ready;
  sem_t response;
  sem_clockwait_fn wait_fn;
  struct timespec deadline;
  uint64_t elapsed_ns;
  int failed;
};

static void *blocked_waiter(void *opaque) {
  struct blocked_state *state = opaque;
  for (int i = 0; i < BLOCKED_ITERATIONS; ++i) {
    if (sem_post(&state->ready) != 0) state->failed = 1;
    uint64_t start = monotonic_ns();
    if (state->wait_fn(&state->request, CLOCK_MONOTONIC,
                       &state->deadline) != 0)
      state->failed = 1;
    state->elapsed_ns += monotonic_ns() - start;
    if (sem_post(&state->response) != 0) state->failed = 1;
  }
  return 0;
}

static int run_blocked(const char *name, sem_clockwait_fn wait_fn) {
  struct blocked_state state = { .wait_fn = wait_fn };
  if (sem_init(&state.request, 0, 0) != 0 ||
      sem_init(&state.ready, 0, 0) != 0 ||
      sem_init(&state.response, 0, 0) != 0) return 1;
  if (clock_gettime(CLOCK_MONOTONIC, &state.deadline) != 0) return 2;
  state.deadline.tv_sec += 60;

  pthread_t thread;
  if (pthread_create(&thread, 0, blocked_waiter, &state) != 0) return 3;
  const struct timespec park_delay = { .tv_sec = 0, .tv_nsec = 50000L };
  for (int i = 0; i < BLOCKED_ITERATIONS; ++i) {
    if (sem_wait(&state.ready) != 0 || nanosleep(&park_delay, 0) != 0 ||
        sem_post(&state.request) != 0 || sem_wait(&state.response) != 0)
      return 4;
  }
  if (pthread_join(thread, 0) != 0 || state.failed) return 5;
  if (sem_destroy(&state.request) != 0 || sem_destroy(&state.ready) != 0 ||
      sem_destroy(&state.response) != 0) return 6;

  printf("blocked-roundtrip,%s,%d,%llu,%.2f\n", name, BLOCKED_ITERATIONS,
         (unsigned long long)state.elapsed_ns,
         (double)state.elapsed_ns / BLOCKED_ITERATIONS);
  return 0;
}

int main(void) {
  sem_clockwait_fn host =
      (sem_clockwait_fn)dlvsym(RTLD_NEXT, "sem_clockwait", "GLIBC_2.30");
  if (host == NULL) return 1;
  puts("path,implementation,iterations,total_ns,ns_per_call");
  if (run_immediate("glibc", host) != 0 ||
      run_immediate("nelibc", sem_clockwait) != 0 ||
      run_empty_timeout("glibc", host) != 0 ||
      run_empty_timeout("nelibc", sem_clockwait) != 0 ||
      run_blocked("glibc", host) != 0 ||
      run_blocked("nelibc", sem_clockwait) != 0)
    return 2;
  return 0;
}
