#include <errno.h>
#include <pthread.h>
#include <stdatomic.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

static volatile int detached_release;
static volatile int detached_done;
static volatile int mutex_counter;
static pthread_mutex_t static_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t condition_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t static_condition = PTHREAD_COND_INITIALIZER;
static _Atomic int condition_waiting;
static int condition_ready;
static int condition_woken;
static pthread_once_t once_control = PTHREAD_ONCE_INIT;
static _Atomic int once_callers;
static _Atomic int once_go;
static _Atomic int once_started;
static _Atomic int once_release;
static _Atomic int once_runs;
static pthread_key_t value_key;
static int value_destructor_calls;

typedef struct {
  pthread_mutex_t mutex;
  pthread_cond_t condition;
  _Atomic int entered;
  int ready;
} process_shared_condition_state;

static void *return_self(void *argument) {
  (void)argument;
  return (void *)pthread_self();
}

static void *exit_with_value(void *argument) {
  pthread_exit(argument);
  return 0;
}

static void *return_detached(void *argument) {
  (void)argument;
  while (!detached_release) {
  }
  detached_done = 1;
  return 0;
}

static void *increment_under_mutex(void *argument) {
  (void)argument;
  for (int i = 0; i < 10000; ++i) {
    if (pthread_mutex_lock(&static_mutex) != 0) return (void *)1;
    ++mutex_counter;
    if (pthread_mutex_unlock(&static_mutex) != 0) return (void *)2;
  }
  return 0;
}

static void *wait_for_condition(void *argument) {
  (void)argument;
  if (pthread_mutex_lock(&condition_mutex) != 0) return (void *)1;
  atomic_fetch_add_explicit(&condition_waiting, 1, memory_order_release);
  while (!condition_ready) {
    if (pthread_cond_wait(&static_condition, &condition_mutex) != 0) {
      pthread_mutex_unlock(&condition_mutex);
      return (void *)2;
    }
  }
  ++condition_woken;
  if (pthread_mutex_unlock(&condition_mutex) != 0) return (void *)3;
  return 0;
}

static void *wait_for_condition_timed(void *argument) {
  (void)argument;
  if (pthread_mutex_lock(&condition_mutex) != 0) return (void *)1;
  atomic_fetch_add_explicit(&condition_waiting, 1, memory_order_release);
  const struct timespec deadline = {4102444800L, 0};
  while (!condition_ready) {
    if (pthread_cond_timedwait(&static_condition, &condition_mutex,
                               &deadline) != 0) {
      pthread_mutex_unlock(&condition_mutex);
      return (void *)2;
    }
  }
  ++condition_woken;
  if (pthread_mutex_unlock(&condition_mutex) != 0) return (void *)3;
  return 0;
}

static void process_shared_condition_child(process_shared_condition_state *state) {
  if (pthread_mutex_lock(&state->mutex) != 0) _exit(71);
  atomic_store_explicit(&state->entered, 1, memory_order_release);
  const struct timespec deadline = {4102444800L, 0};
  while (!state->ready) {
    if (pthread_cond_timedwait(&state->condition, &state->mutex, &deadline) != 0)
      _exit(72);
  }
  if (pthread_mutex_unlock(&state->mutex) != 0) _exit(73);
  _exit(0);
}

static void once_initializer(void) {
  atomic_fetch_add_explicit(&once_runs, 1, memory_order_relaxed);
  atomic_store_explicit(&once_started, 1, memory_order_release);
  while (!atomic_load_explicit(&once_release, memory_order_acquire)) {
  }
}

static void *once_worker(void *argument) {
  (void)argument;
  atomic_fetch_add_explicit(&once_callers, 1, memory_order_release);
  while (!atomic_load_explicit(&once_go, memory_order_acquire)) {
  }
  return (void *)(uintptr_t)pthread_once(&once_control, once_initializer);
}

static void value_destructor(void *value) {
  if (value != (void *)(uintptr_t)22) return;
  ++value_destructor_calls;
  if (value_destructor_calls == 1) pthread_setspecific(value_key, value);
}

static void *value_worker(void *argument) {
  if (pthread_setspecific(value_key, argument) != 0 ||
      pthread_getspecific(value_key) != argument)
    return (void *)1;
  return 0;
}

static void *value_exit_worker(void *argument) {
  if (pthread_setspecific(value_key, (void *)(uintptr_t)22) != 0)
    return (void *)1;
  pthread_exit(argument);
  return 0;
}

int main(void) {
  pthread_attr_t attributes;
  if (pthread_attr_init(&attributes) != 0) return 1;
  if (pthread_attr_setstacksize(&attributes, 65536) != 0) return 2;
  if (pthread_attr_setdetachstate(&attributes, PTHREAD_CREATE_DETACHED) != 0)
    return 3;
  int detachstate = PTHREAD_CREATE_JOINABLE;
  if (pthread_attr_getdetachstate(&attributes, &detachstate) != 0 ||
      detachstate != PTHREAD_CREATE_DETACHED)
    return 4;

  pthread_t thread;
  if (pthread_create(&thread, &attributes, return_detached, 0) != 0) return 5;
  if (pthread_join(thread, 0) != EINVAL) return 6;
  detached_release = 1;
  for (unsigned int i = 0; i < 10000000U && !detached_done; ++i) {
  }
  if (!detached_done) return 7;

  if (pthread_attr_setdetachstate(&attributes, PTHREAD_CREATE_JOINABLE) != 0)
    return 8;
  detached_release = 0;
  detached_done = 0;
  if (pthread_create(&thread, &attributes, return_detached, 0) != 0) return 9;
  if (pthread_detach(thread) != 0) return 10;
  if (pthread_detach(thread) != EINVAL || pthread_join(thread, 0) != EINVAL)
    return 11;
  detached_release = 1;
  for (unsigned int i = 0; i < 10000000U && !detached_done; ++i) {
  }
  if (!detached_done) return 12;

  if (pthread_create(&thread, &attributes, return_self, 0) != 0) return 13;
  void *result = 0;
  if (pthread_join(thread, &result) != 0) return 14;
  if (!pthread_equal(thread, (pthread_t)result) ||
      pthread_equal(thread, pthread_self()))
    return 15;
  void *exit_value = (void *)0x1234;
  if (pthread_create(&thread, &attributes, exit_with_value, exit_value) != 0)
    return 16;
  result = 0;
  if (pthread_join(thread, &result) != 0 || result != exit_value) return 17;
  if (pthread_mutex_lock(&static_mutex) != 0) return 18;
  if (pthread_mutex_trylock(&static_mutex) != EBUSY) return 19;
  if (pthread_mutex_destroy(&static_mutex) != EBUSY) return 20;
  if (pthread_mutex_unlock(&static_mutex) != 0) return 21;
  pthread_t first_mutex_thread;
  pthread_t second_mutex_thread;
  if (pthread_create(&first_mutex_thread, &attributes, increment_under_mutex, 0) != 0 ||
      pthread_create(&second_mutex_thread, &attributes, increment_under_mutex, 0) != 0)
    return 22;
  if (pthread_join(first_mutex_thread, &result) != 0 || result != 0 ||
      pthread_join(second_mutex_thread, &result) != 0 || result != 0 ||
      mutex_counter != 20000)
    return 23;

  pthread_mutexattr_t mutex_attributes;
  if (pthread_mutexattr_init(&mutex_attributes) != 0 ||
      pthread_mutexattr_settype(&mutex_attributes, PTHREAD_MUTEX_RECURSIVE) != 0)
    return 24;
  int mutex_type = -1;
  if (pthread_mutexattr_gettype(&mutex_attributes, &mutex_type) != 0 ||
      mutex_type != PTHREAD_MUTEX_RECURSIVE)
    return 25;
  pthread_mutex_t recursive_mutex;
  if (pthread_mutex_init(&recursive_mutex, &mutex_attributes) != 0 ||
      pthread_mutex_lock(&recursive_mutex) != 0 ||
      pthread_mutex_lock(&recursive_mutex) != 0 ||
      pthread_mutex_unlock(&recursive_mutex) != 0 ||
      pthread_mutex_unlock(&recursive_mutex) != 0 ||
      pthread_mutex_destroy(&recursive_mutex) != 0 ||
      pthread_mutexattr_destroy(&mutex_attributes) != 0)
    return 26;

  if (pthread_mutexattr_init(&mutex_attributes) != 0 ||
      pthread_mutexattr_settype(&mutex_attributes, PTHREAD_MUTEX_ERRORCHECK) != 0)
    return 27;
  pthread_mutex_t errorcheck_mutex;
  if (pthread_mutex_init(&errorcheck_mutex, &mutex_attributes) != 0 ||
      pthread_mutex_lock(&errorcheck_mutex) != 0 ||
      pthread_mutex_lock(&errorcheck_mutex) != EDEADLK ||
      pthread_mutex_unlock(&errorcheck_mutex) != 0 ||
      pthread_mutex_destroy(&errorcheck_mutex) != 0 ||
      pthread_mutexattr_destroy(&mutex_attributes) != 0)
    return 28;
  if (pthread_mutex_destroy(&static_mutex) != 0) return 29;

  pthread_t first_waiter;
  pthread_t second_waiter;
  if (pthread_create(&first_waiter, &attributes, wait_for_condition, 0) != 0 ||
      pthread_create(&second_waiter, &attributes, wait_for_condition, 0) != 0)
    return 30;
  for (unsigned int i = 0; i < 10000000U &&
       atomic_load_explicit(&condition_waiting, memory_order_acquire) != 2; ++i) {
  }
  if (atomic_load_explicit(&condition_waiting, memory_order_acquire) != 2)
    return 31;
  if (pthread_mutex_lock(&condition_mutex) != 0) return 32;
  if (pthread_cond_destroy(&static_condition) != EBUSY) return 33;
  condition_ready = 1;
  if (pthread_cond_broadcast(&static_condition) != 0 ||
      pthread_mutex_unlock(&condition_mutex) != 0)
    return 34;
  if (pthread_join(first_waiter, &result) != 0 || result != 0 ||
      pthread_join(second_waiter, &result) != 0 || result != 0 ||
      condition_woken != 2)
    return 35;

  if (pthread_mutex_lock(&condition_mutex) != 0) return 36;
  condition_ready = 0;
  condition_woken = 0;
  atomic_store_explicit(&condition_waiting, 0, memory_order_release);
  if (pthread_mutex_unlock(&condition_mutex) != 0) return 37;
  if (pthread_create(&first_waiter, &attributes, wait_for_condition, 0) != 0)
    return 38;
  for (unsigned int i = 0; i < 10000000U &&
       atomic_load_explicit(&condition_waiting, memory_order_acquire) != 1; ++i) {
  }
  if (atomic_load_explicit(&condition_waiting, memory_order_acquire) != 1)
    return 39;
  if (pthread_mutex_lock(&condition_mutex) != 0) return 40;
  condition_ready = 1;
  if (pthread_cond_signal(&static_condition) != 0 ||
      pthread_mutex_unlock(&condition_mutex) != 0)
    return 41;
  if (pthread_join(first_waiter, &result) != 0 || result != 0 ||
      condition_woken != 1)
    return 42;
  if (pthread_mutex_lock(&condition_mutex) != 0) return 43;
  condition_ready = 0;
  condition_woken = 0;
  atomic_store_explicit(&condition_waiting, 0, memory_order_release);
  if (pthread_mutex_unlock(&condition_mutex) != 0) return 44;
  if (pthread_create(&first_waiter, &attributes, wait_for_condition_timed, 0) != 0)
    return 45;
  for (unsigned int i = 0; i < 10000000U &&
       atomic_load_explicit(&condition_waiting, memory_order_acquire) != 1; ++i) {
  }
  if (atomic_load_explicit(&condition_waiting, memory_order_acquire) != 1)
    return 46;
  if (pthread_mutex_lock(&condition_mutex) != 0) return 47;
  condition_ready = 1;
  if (pthread_cond_signal(&static_condition) != 0 ||
      pthread_mutex_unlock(&condition_mutex) != 0)
    return 48;
  if (pthread_join(first_waiter, &result) != 0 || result != 0 ||
      condition_woken != 1)
    return 49;
  struct timespec invalid_deadline = {0, 1000000000L};
  if (pthread_mutex_lock(&condition_mutex) != 0) return 50;
  if (pthread_cond_timedwait(&static_condition, &condition_mutex,
                             &invalid_deadline) != EINVAL)
    return 51;
  struct timespec expired_deadline = {0, 0};
  if (pthread_cond_timedwait(&static_condition, &condition_mutex,
                             &expired_deadline) != ETIMEDOUT)
    return 52;
  if (pthread_mutex_unlock(&condition_mutex) != 0) return 53;
  if (pthread_cond_destroy(&static_condition) != 0 ||
      pthread_mutex_destroy(&condition_mutex) != 0)
    return 54;

  pthread_condattr_t condition_attributes;
  if (pthread_condattr_init(&condition_attributes) != 0 ||
      pthread_condattr_setpshared(&condition_attributes, PTHREAD_PROCESS_SHARED) != 0)
    return 55;
  int pshared = PTHREAD_PROCESS_PRIVATE;
  if (pthread_condattr_getpshared(&condition_attributes, &pshared) != 0 ||
      pshared != PTHREAD_PROCESS_SHARED)
    return 45;
  pthread_cond_t shared_condition;
  if (pthread_cond_init(&shared_condition, &condition_attributes) != 0)
    return 56;
  pthread_mutexattr_t shared_mutex_attributes;
  if (pthread_mutexattr_init(&shared_mutex_attributes) != 0 ||
      pthread_mutexattr_setpshared(&shared_mutex_attributes, PTHREAD_PROCESS_SHARED) != 0)
    return 57;
  int mutex_pshared = PTHREAD_PROCESS_PRIVATE;
  if (pthread_mutexattr_getpshared(&shared_mutex_attributes, &mutex_pshared) != 0 ||
      mutex_pshared != PTHREAD_PROCESS_SHARED)
    return 58;
  process_shared_condition_state *shared_state = mmap(
      0, sizeof(*shared_state), PROT_READ | PROT_WRITE,
      MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  if (shared_state == MAP_FAILED) return 59;
  atomic_store_explicit(&shared_state->entered, 0, memory_order_relaxed);
  shared_state->ready = 0;
  if (pthread_mutex_init(&shared_state->mutex, &shared_mutex_attributes) != 0 ||
      pthread_cond_init(&shared_state->condition, &condition_attributes) != 0)
    return 60;
  pid_t child = fork();
  if (child < 0) return 61;
  if (child == 0) process_shared_condition_child(shared_state);
  for (unsigned int i = 0; i < 10000000U &&
       !atomic_load_explicit(&shared_state->entered, memory_order_acquire); ++i) {
  }
  if (!atomic_load_explicit(&shared_state->entered, memory_order_acquire)) return 62;
  if (pthread_mutex_lock(&shared_state->mutex) != 0) return 63;
  shared_state->ready = 1;
  if (pthread_cond_signal(&shared_state->condition) != 0 ||
      pthread_mutex_unlock(&shared_state->mutex) != 0)
    return 64;
  int child_status = -1;
  if (waitpid(child, &child_status, 0) != child || child_status != 0)
    return 65;
  if (pthread_cond_destroy(&shared_state->condition) != 0 ||
      pthread_mutex_destroy(&shared_state->mutex) != 0 ||
      munmap(shared_state, sizeof(*shared_state)) != 0)
    return 66;
  if (pthread_mutexattr_destroy(&shared_mutex_attributes) != 0 ||
      pthread_cond_destroy(&shared_condition) != 0 ||
      pthread_condattr_destroy(&condition_attributes) != 0)
    return 67;
  pthread_t once_threads[4];
  for (int i = 0; i < 4; ++i) {
    if (pthread_create(&once_threads[i], &attributes, once_worker, 0) != 0)
      return 68;
  }
  for (unsigned int i = 0; i < 10000000U &&
       atomic_load_explicit(&once_callers, memory_order_acquire) != 4; ++i) {
  }
  if (atomic_load_explicit(&once_callers, memory_order_acquire) != 4) return 69;
  atomic_store_explicit(&once_go, 1, memory_order_release);
  for (unsigned int i = 0; i < 10000000U &&
       !atomic_load_explicit(&once_started, memory_order_acquire); ++i) {
  }
  if (!atomic_load_explicit(&once_started, memory_order_acquire)) return 70;
  for (volatile unsigned int i = 0; i < 1000000U; ++i) {
  }
  atomic_store_explicit(&once_release, 1, memory_order_release);
  for (int i = 0; i < 4; ++i) {
    if (pthread_join(once_threads[i], &result) != 0 || result != 0) return 71;
  }
  if (atomic_load_explicit(&once_runs, memory_order_acquire) != 1 ||
      pthread_once(&once_control, once_initializer) != 0 ||
      atomic_load_explicit(&once_runs, memory_order_acquire) != 1)
    return 72;
  if (pthread_once(0, once_initializer) != EINVAL ||
      pthread_once(&once_control, 0) != EINVAL)
    return 73;
  if (pthread_key_create(&value_key, value_destructor) != 0 ||
      pthread_setspecific(value_key, 0) != 0 ||
      pthread_getspecific(value_key) != 0 ||
      pthread_setspecific(value_key, (void *)(uintptr_t)11) != 0 ||
      pthread_getspecific(value_key) != (void *)(uintptr_t)11)
    return 75;
  if (pthread_create(&thread, &attributes, value_worker,
                     (void *)(uintptr_t)22) != 0 ||
      pthread_join(thread, &result) != 0 || result != 0 ||
      value_destructor_calls != 2 ||
      pthread_getspecific(value_key) != (void *)(uintptr_t)11)
    return 76;
  if (pthread_create(&thread, &attributes, value_exit_worker,
                     (void *)(uintptr_t)44) != 0 ||
      pthread_join(thread, &result) != 0 || result != (void *)(uintptr_t)44 ||
      value_destructor_calls != 3 ||
      pthread_getspecific(value_key) != (void *)(uintptr_t)11)
    return 79;
  if (pthread_key_delete(value_key) != 0 ||
      pthread_getspecific(value_key) != 0 ||
      pthread_setspecific(value_key, (void *)(uintptr_t)33) != EINVAL)
    return 77;
  pthread_key_t replacement_key;
  if (pthread_key_create(&replacement_key, 0) != 0 ||
      replacement_key == value_key ||
      pthread_getspecific(replacement_key) != 0 ||
      pthread_key_delete(replacement_key) != 0)
    return 78;
  if (pthread_attr_destroy(&attributes) != 0) return 74;
  return 0;
}
