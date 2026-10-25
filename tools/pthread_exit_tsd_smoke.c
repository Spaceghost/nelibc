#include <errno.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdatomic.h>
#include <stdint.h>

static pthread_key_t exit_key;
static sem_t destructor_gate;
static _Atomic int destructor_started;
static _Atomic int destructor_calls;
static _Atomic int destructor_error;

static void blocking_destructor(void *value) {
  if (value != (void *)(uintptr_t)0x88) return;
  atomic_store_explicit(&destructor_started, 1, memory_order_release);
  if (sem_wait(&destructor_gate) != 0)
    atomic_store_explicit(&destructor_error, errno, memory_order_relaxed);
  atomic_fetch_add_explicit(&destructor_calls, 1, memory_order_relaxed);
}

static void *worker(void *argument) {
  (void)argument;
  if (pthread_setspecific(exit_key, (void *)(uintptr_t)0x88) != 0)
    return (void *)(uintptr_t)1;
  return (void *)(uintptr_t)0x99;
}

int main(void) {
  if (pthread_key_create(&exit_key, blocking_destructor) != 0 ||
      sem_init(&destructor_gate, 0, 0) != 0)
    return 1;

  pthread_t thread;
  if (pthread_create(&thread, 0, worker, 0) != 0) return 2;
  for (unsigned int i = 0; i < 10000000U &&
       (!atomic_load_explicit(&destructor_started, memory_order_acquire) ||
        __atomic_load_n(&destructor_gate.__waiters, __ATOMIC_ACQUIRE) == 0); ++i) {
  }
  if (!atomic_load_explicit(&destructor_started, memory_order_acquire) ||
      __atomic_load_n(&destructor_gate.__waiters, __ATOMIC_ACQUIRE) == 0)
    return 3;

  if (pthread_cancel(thread) != 0 || sem_post(&destructor_gate) != 0) return 4;
  void *result = 0;
  if (pthread_join(thread, &result) != 0 ||
      result != (void *)(uintptr_t)0x99 ||
      atomic_load_explicit(&destructor_calls, memory_order_relaxed) != 1 ||
      atomic_load_explicit(&destructor_error, memory_order_relaxed) != 0 ||
      sem_destroy(&destructor_gate) != 0 || pthread_key_delete(exit_key) != 0)
    return 5;
  return 0;
}
