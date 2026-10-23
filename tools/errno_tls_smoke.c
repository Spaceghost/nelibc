#include <errno.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdint.h>

static atomic_int ready = 0;
static atomic_int failures = 0;

static void *check_errno_is_thread_local(void *value) {
  int expected = (int)(intptr_t)value;
  errno = expected;
  atomic_fetch_add_explicit(&ready, 1, memory_order_release);
  while (atomic_load_explicit(&ready, memory_order_acquire) != 2) {
  }
  if (errno != expected) {
    atomic_fetch_add_explicit(&failures, 1, memory_order_relaxed);
  }
  return 0;
}

int main(void) {
  pthread_t first;
  pthread_t second;
  if (pthread_create(&first, 0, check_errno_is_thread_local, (void *)(intptr_t)31) != 0) return 1;
  if (pthread_create(&second, 0, check_errno_is_thread_local, (void *)(intptr_t)47) != 0) return 2;
  if (pthread_join(first, 0) != 0 || pthread_join(second, 0) != 0) return 3;
  return atomic_load_explicit(&failures, memory_order_relaxed) == 0 ? 0 : 4;
}
