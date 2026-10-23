#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <semaphore.h>
#include <stdatomic.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

static volatile int detached_release;
static volatile int pthread_kill_signal;
static int atfork_order[16];
static int atfork_order_count;
static volatile int detached_done;
static volatile int mutex_counter;
static pthread_mutex_t static_mutex = PTHREAD_MUTEX_INITIALIZER;
static int spin_counter;
static pthread_spinlock_t test_spinlock;
static pthread_mutex_t condition_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t static_condition = PTHREAD_COND_INITIALIZER;
static _Atomic int condition_waiting;
static _Atomic int pi_cancel_acquired;
static int condition_ready;
static int condition_woken;
static pthread_once_t once_control = PTHREAD_ONCE_INIT;
static _Atomic int once_callers;
static _Atomic int once_go;
static _Atomic int once_started;
static _Atomic int once_release;
static _Atomic int once_runs;
static pthread_barrier_t round_barrier;
static _Atomic int barrier_serial_returns;
static _Atomic int cpu_clock_worker_release;
static _Atomic unsigned long cpu_clock_worker_ticks;
static pthread_rwlock_t static_rwlock = PTHREAD_RWLOCK_INITIALIZER;
static _Atomic int rwlock_counter;
static pthread_key_t value_key;
static int value_destructor_calls;
static pthread_mutex_t cancel_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cancel_condition = PTHREAD_COND_INITIALIZER;
static int cancel_read_pipe[2];
static volatile int cancel_waiting;
static volatile int cancel_read_started;
static volatile int cancel_sleep_started;
static volatile int cancel_state_ready;
static volatile int cancel_state_release;
static volatile int cancel_async_ready;
static volatile int cancel_type_ready;
static volatile int cancel_type_release;
static int cancel_cleanup_calls;
static sem_t cancel_semaphore;
static volatile int cancel_sem_started;
enum { NAMED_SEMAPHORE_RACE_WORKERS = 8 };
static char named_semaphore_path[64];
static _Atomic int named_semaphore_race_ready;
static _Atomic int named_semaphore_race_go;
static struct {
  sem_t *semaphore;
  int value;
  int error;
} named_semaphore_race_results[NAMED_SEMAPHORE_RACE_WORKERS];

static void cancel_cleanup(void *argument) {
  ++cancel_cleanup_calls;
  if (argument != 0) pthread_mutex_unlock((pthread_mutex_t *)argument);
}

static void pthread_kill_handler(int signal) {
  pthread_kill_signal = signal;
}

static void atfork_order_add(int value) {
  if (atfork_order_count < (int)(sizeof(atfork_order) / sizeof(atfork_order[0])))
    atfork_order[atfork_order_count++] = value;
}
static void atfork_prepare_a(void) { atfork_order_add(1); }
static void atfork_prepare_b(void) { atfork_order_add(2); }
static void atfork_parent_a(void) { atfork_order_add(3); }
static void atfork_parent_b(void) { atfork_order_add(4); }
static void atfork_child_a(void) { atfork_order_add(5); }
static void atfork_child_b(void) { atfork_order_add(6); }

static pthread_key_t initial_exit_key;

static void initial_exit_cleanup(void *argument) {
  ++*(volatile int *)argument;
}

static void initial_exit_key_destructor(void *argument) {
  *(volatile int *)argument += 10;
}

typedef struct {
  pthread_t target;
  sem_t *release;
  _Atomic int ready;
} initial_cancel_request;

static void *cancel_initial_worker(void *argument) {
  initial_cancel_request *request = argument;
  while (!atomic_load_explicit(&request->ready, memory_order_acquire)) {
  }
  if (pthread_cancel(request->target) != 0) sem_post(request->release);
  const struct timespec fallback = {0, 100000000L};
  nanosleep(&fallback, 0);
  sem_post(request->release);
  return 0;
}

static void *cancel_cond_worker(void *argument) {
  (void)argument;
  if (pthread_mutex_lock(&cancel_mutex) != 0) return (void *)1;
  pthread_cleanup_push(cancel_cleanup, &cancel_mutex);
  cancel_waiting = 1;
  (void)pthread_cond_wait(&cancel_condition, &cancel_mutex);
  pthread_cleanup_pop(0);
  pthread_mutex_unlock(&cancel_mutex);
  return 0;
}

static void *cancel_disabled_worker(void *argument) {
  (void)argument;
  pthread_cleanup_push(cancel_cleanup, 0);
  if (pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, 0) != 0) return (void *)1;
  cancel_state_ready = 1;
  while (!cancel_state_release) {
  }
  pthread_testcancel();
  if (pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, 0) != 0) return (void *)2;
  pthread_cleanup_pop(0);
  return 0;
}

static void *cancel_async_worker(void *argument) {
  (void)argument;
  int old_type = -1;
  pthread_cleanup_push(cancel_cleanup, 0);
  if (pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, &old_type) != 0 ||
      old_type != PTHREAD_CANCEL_DEFERRED)
    return (void *)10;
  if (pthread_setcanceltype(99, 0) != EINVAL) return (void *)11;
  if (pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, &old_type) != 0 ||
      old_type != PTHREAD_CANCEL_DEFERRED)
    return (void *)12;
  cancel_async_ready = 1;
  for (;;) {
  }
  pthread_cleanup_pop(0);
  return 0;
}

static void *cancel_pending_type_worker(void *argument) {
  (void)argument;
  pthread_cleanup_push(cancel_cleanup, 0);
  cancel_type_ready = 1;
  while (!cancel_type_release) {
  }
  if (pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, 0) != 0)
    return (void *)13;
  pthread_cleanup_pop(0);
  return 0;
}

static void *cancel_read_worker(void *argument) {
  (void)argument;
  char byte;
  pthread_cleanup_push(cancel_cleanup, 0);
  cancel_read_started = 1;
  (void)read(cancel_read_pipe[0], &byte, 1);
  pthread_cleanup_pop(0);
  return (void *)3;
}

static void *cancel_sleep_worker(void *argument) {
  (void)argument;
  const struct timespec delay = {60, 0};
  pthread_cleanup_push(cancel_cleanup, 0);
  cancel_sleep_started = 1;
  (void)nanosleep(&delay, 0);
  pthread_cleanup_pop(0);
  return (void *)4;
}

static void *cancel_sem_worker(void *argument) {
  (void)argument;
  pthread_cleanup_push(cancel_cleanup, 0);
  cancel_sem_started = 1;
  (void)sem_wait(&cancel_semaphore);
  pthread_cleanup_pop(0);
  return (void *)5;
}

static void *cancel_disabled_sem_worker(void *argument) {
  (void)argument;
  pthread_cleanup_push(cancel_cleanup, 0);
  if (pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, 0) != 0) return (void *)6;
  cancel_sem_started = 1;
  if (sem_wait(&cancel_semaphore) != 0) return (void *)7;
  if (pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, 0) != 0) return (void *)8;
  pthread_cleanup_pop(0);
  return (void *)9;
}

static void *named_semaphore_race_worker(void *argument) {
  unsigned int index = (unsigned int)(uintptr_t)argument;
  atomic_fetch_add_explicit(&named_semaphore_race_ready, 1, memory_order_release);
  while (!atomic_load_explicit(&named_semaphore_race_go, memory_order_acquire)) {
  }
  sem_t *semaphore = sem_open(named_semaphore_path, O_CREAT, 0600, index + 1U);
  named_semaphore_race_results[index].semaphore = semaphore;
  if (semaphore == SEM_FAILED) {
    named_semaphore_race_results[index].error = errno;
  } else if (sem_getvalue(semaphore, &named_semaphore_race_results[index].value) != 0) {
    named_semaphore_race_results[index].error = errno;
  }
  return 0;
}

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

static void process_shared_semaphore_child(sem_t *semaphore) {
  if (sem_wait(semaphore) != 0) _exit(74);
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

static void *barrier_worker(void *argument) {
  (void)argument;
  for (int round = 0; round < 100; ++round) {
    int result = pthread_barrier_wait(&round_barrier);
    if (result == PTHREAD_BARRIER_SERIAL_THREAD)
      atomic_fetch_add_explicit(&barrier_serial_returns, 1, memory_order_relaxed);
    else if (result != 0)
      return (void *)(uintptr_t)1;
  }
  return 0;
}

struct rwlock_timeout_request {
  pthread_rwlock_t *rwlock;
  struct timespec deadline;
  int write_lock;
  int result;
};

static void *rwlock_timeout_worker(void *argument) {
  struct rwlock_timeout_request *request = argument;
  request->result = request->write_lock
      ? pthread_rwlock_timedwrlock(request->rwlock, &request->deadline)
      : pthread_rwlock_timedrdlock(request->rwlock, &request->deadline);
  if (request->result == 0) pthread_rwlock_unlock(request->rwlock);
  return 0;
}

static void *rwlock_stress_worker(void *argument) {
  int write_lock = argument != 0;
  for (int i = 0; i < 500; ++i) {
    int result = write_lock ? pthread_rwlock_wrlock(&static_rwlock)
                            : pthread_rwlock_rdlock(&static_rwlock);
    if (result != 0) return (void *)(uintptr_t)result;
    if (write_lock) {
      int value = atomic_load_explicit(&rwlock_counter, memory_order_relaxed);
      atomic_store_explicit(&rwlock_counter, value + 1, memory_order_relaxed);
    } else {
      (void)atomic_load_explicit(&rwlock_counter, memory_order_relaxed);
    }
    if (pthread_rwlock_unlock(&static_rwlock) != 0) return (void *)(uintptr_t)1;
  }
  return 0;
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

static void *timed_mutex_worker(void *argument) {
  struct timespec expired = {0, 0};
  int result = pthread_mutex_timedlock((pthread_mutex_t *)argument, &expired);
  return (void *)(intptr_t)result;
}

static _Alignas(16) unsigned char user_thread_stack[65536];
static uintptr_t user_thread_stack_low;
static uintptr_t user_thread_stack_high;

static void *user_stack_worker(void *argument) {
  (void)argument;
  volatile unsigned char marker = 0;
  uintptr_t address = (uintptr_t)&marker;
  return address >= user_thread_stack_low && address < user_thread_stack_high
             ? 0
             : (void *)3;
}

static void *pi_mutex_worker(void *argument) {
  pthread_mutex_t *mutex = argument;
  int result = pthread_mutex_lock(mutex);
  if (result != 0) return (void *)(intptr_t)result;
  result = pthread_mutex_unlock(mutex);
  return (void *)(intptr_t)result;
}

typedef struct {
  pthread_mutex_t *mutex;
  struct timespec deadline;
} pi_timed_mutex_request;

static void *pi_timed_mutex_worker(void *argument) {
  pi_timed_mutex_request *request = argument;
  int result = pthread_mutex_timedlock(request->mutex, &request->deadline);
  return (void *)(intptr_t)result;
}

static void *pi_cancel_mutex_worker(void *argument) {
  pthread_mutex_t *mutex = argument;
  int result = pthread_mutex_lock(mutex);
  if (result != 0) return (void *)(intptr_t)result;
  atomic_store_explicit(&pi_cancel_acquired, 1, memory_order_release);
  result = pthread_mutex_unlock(mutex);
  if (result != 0) return (void *)(intptr_t)result;
  pthread_testcancel();
  return 0;
}

static void *robust_owner_worker(void *argument) {
  pthread_mutex_t *mutex = argument;
  return pthread_mutex_lock(mutex) == 0 ? 0 : (void *)4;
}
static sem_t robust_owner_release;
static _Atomic int robust_owner_ready;

static void *robust_gated_owner_worker(void *argument) {
  pthread_mutex_t *mutex = argument;
  if (pthread_mutex_lock(mutex) != 0) return (void *)5;
  atomic_store_explicit(&robust_owner_ready, 1, memory_order_release);
  if (sem_wait(&robust_owner_release) != 0) return (void *)6;
  return 0;
}

static void *robust_recovery_waiter(void *argument) {
  pthread_mutex_t *mutex = argument;
  int result = pthread_mutex_lock(mutex);
  if (result != EOWNERDEAD || pthread_mutex_consistent(mutex) != 0 ||
      pthread_mutex_unlock(mutex) != 0)
    return (void *)7;
  return 0;
}

static void *explicit_schedule_worker(void *argument) {
  (void)argument;
  int policy = -1;
  struct sched_param param = {-1};
  if (pthread_getschedparam(pthread_self(), &policy, &param) != 0)
    return (void *)1;
  return policy == SCHED_OTHER && param.sched_priority == 0 ? 0 : (void *)2;
}

static int priority_protect_schedule_smoke(void) {
  int original_policy = -1;
  struct sched_param original_param = {-1};
  if (pthread_getschedparam(pthread_self(), &original_policy,
                            &original_param) != 0)
    return 1;
  struct sched_param realtime_param = {1};
  int result = pthread_setschedparam(pthread_self(), SCHED_FIFO,
                                    &realtime_param);
  if (result == EPERM || result == EACCES || result == ENOTSUP) return 0;
  if (result != 0) return 2;

  pthread_mutexattr_t attributes;
  pthread_mutex_t mutex;
  int status = 0;
  int attr_ready = pthread_mutexattr_init(&attributes) == 0;
  if (!attr_ready) {
    status = 3;
    goto restore_policy;
  }
  if (pthread_mutexattr_setprotocol(&attributes, PTHREAD_PRIO_PROTECT) != 0 ||
      pthread_mutexattr_setprioceiling(&attributes, 3) != 0) {
    status = 3;
    pthread_mutexattr_destroy(&attributes);
    goto restore_policy;
  }
  if (pthread_mutex_init(&mutex, &attributes) != 0) {
    status = 3;
    pthread_mutexattr_destroy(&attributes);
    goto restore_policy;
  }

  result = pthread_mutex_lock(&mutex);
  if (result == EPERM || result == EACCES) {
    pthread_mutex_destroy(&mutex);
    pthread_mutexattr_destroy(&attributes);
    goto restore_policy;
  }
  if (result != 0) {
    status = 4;
    pthread_mutex_destroy(&mutex);
    pthread_mutexattr_destroy(&attributes);
    goto restore_policy;
  }

  int policy = -1;
  struct sched_param param = {-1};
  if (pthread_getschedparam(pthread_self(), &policy, &param) != 0 ||
      policy != SCHED_FIFO || param.sched_priority != 1)
    status = 5;
  param.sched_priority = 2;
  if (status == 0 &&
      (pthread_setschedparam(pthread_self(), SCHED_FIFO, &param) != 0 ||
       pthread_getschedparam(pthread_self(), &policy, &param) != 0 ||
       policy != SCHED_FIFO || param.sched_priority != 2))
    status = 6;
  param.sched_priority = 4;
  if (status == 0 &&
      (pthread_setschedparam(pthread_self(), SCHED_FIFO, &param) != EINVAL ||
       pthread_getschedparam(pthread_self(), &policy, &param) != 0 ||
       policy != SCHED_FIFO || param.sched_priority != 2))
    status = 7;
  if (status == 0 &&
      (pthread_setschedprio(pthread_self(), 1) != 0 ||
       pthread_getschedparam(pthread_self(), &policy, &param) != 0 ||
       policy != SCHED_FIFO || param.sched_priority != 1))
    status = 8;
  if (pthread_mutex_unlock(&mutex) != 0 && status == 0) status = 9;
  if (pthread_getschedparam(pthread_self(), &policy, &param) != 0 ||
      policy != SCHED_FIFO || param.sched_priority != 1) {
    if (status == 0) status = 10;
  }
  pthread_mutex_destroy(&mutex);
  pthread_mutexattr_destroy(&attributes);

restore_policy:
  if (pthread_setschedparam(pthread_self(), original_policy,
                            &original_param) != 0 && status == 0)
    status = 11;
  return status;
}

static void *spin_increment_worker(void *argument) {
  (void)argument;
  for (int i = 0; i < 10000; ++i) {
    if (pthread_spin_lock(&test_spinlock) != 0) return (void *)1;
    ++spin_counter;
    if (pthread_spin_unlock(&test_spinlock) != 0) return (void *)2;
  }
  return 0;
}

static void *cpu_clock_worker(void *argument) {
  (void)argument;
  while (!atomic_load_explicit(&cpu_clock_worker_release, memory_order_acquire))
    atomic_fetch_add_explicit(&cpu_clock_worker_ticks, 1, memory_order_relaxed);
  return 0;
}

typedef struct {
  pthread_spinlock_t lock;
  int value;
} shared_spin_state;

int main(void) {
  sigset_t cancellation_mask;
  struct sigaction cancellation_action = {0};
  cancellation_action.sa_handler = SIG_IGN;
  if (sigfillset(&cancellation_mask) != 0 ||
      sigismember(&cancellation_mask, 32) != 0)
    return 143;
  errno = 0;
  if (sigaddset(&cancellation_mask, 32) != -1 || errno != EINVAL) return 144;
  errno = 0;
  if (sigaction(32, &cancellation_action, 0) != -1 || errno != EINVAL)
    return 145;

  sigset_t original_signal_mask;
  sigset_t observed_signal_mask;
  cancellation_mask.__bits[0] |= 1UL << 31;
  if (pthread_sigmask(SIG_SETMASK, &cancellation_mask,
                      &original_signal_mask) != 0 ||
      pthread_sigmask(99, NULL, &observed_signal_mask) != 0 ||
      sigismember(&observed_signal_mask, SIGUSR1) != 1 ||
      (observed_signal_mask.__bits[0] & (1UL << 31)) != 0)
    return 275;
  errno = EDOM;
  if (pthread_sigmask(99, &cancellation_mask, NULL) != EINVAL || errno != EDOM ||
      pthread_sigmask(SIG_SETMASK, &original_signal_mask, NULL) != 0)
    return 276;
  sigset_t usr1_signal_set;
  if (sigemptyset(&usr1_signal_set) != 0 ||
      sigaddset(&usr1_signal_set, SIGUSR1) != 0 ||
      pthread_sigmask(SIG_BLOCK, &usr1_signal_set, NULL) != 0 ||
      pthread_sigmask(SIG_SETMASK, NULL, &observed_signal_mask) != 0 ||
      sigismember(&observed_signal_mask, SIGUSR1) != 1 ||
      pthread_sigmask(SIG_UNBLOCK, &usr1_signal_set, NULL) != 0 ||
      pthread_sigmask(SIG_SETMASK, NULL, &observed_signal_mask) != 0 ||
      sigismember(&observed_signal_mask, SIGUSR1) != 0 ||
      pthread_sigmask(SIG_SETMASK, &original_signal_mask, NULL) != 0)
    return 277;

  struct sigaction pthread_kill_action = {0};
  struct sigaction previous_pthread_kill_action;
  pthread_kill_action.sa_handler = pthread_kill_handler;
  if (sigaction(SIGUSR2, &pthread_kill_action, &previous_pthread_kill_action) != 0 ||
      pthread_kill(pthread_self(), 0) != 0 ||
      pthread_kill(0, 0) != ESRCH ||
      pthread_kill(pthread_self(), 65) != EINVAL ||
      pthread_kill(pthread_self(), SIGUSR2) != 0 ||
      pthread_kill_signal != SIGUSR2 ||
      sigaction(SIGUSR2, &previous_pthread_kill_action, 0) != 0)
    return 278;

  clockid_t self_cpu_clock = 0;
  struct timespec self_cpu_time;
  if (pthread_getcpuclockid(pthread_self(), &self_cpu_clock) != 0 ||
      pthread_getcpuclockid(pthread_self(), 0) != EINVAL ||
      pthread_getcpuclockid(0, &self_cpu_clock) != ESRCH ||
      clock_gettime(self_cpu_clock, &self_cpu_time) != 0 ||
      self_cpu_time.tv_sec < 0 || self_cpu_time.tv_nsec < 0 ||
      self_cpu_time.tv_nsec >= 1000000000L)
    return 281;

  pthread_t cpu_clock_thread;
  clockid_t worker_cpu_clock = 0;
  struct timespec worker_cpu_start;
  struct timespec worker_cpu_end;
  atomic_store_explicit(&cpu_clock_worker_release, 0, memory_order_relaxed);
  atomic_store_explicit(&cpu_clock_worker_ticks, 0, memory_order_relaxed);
  if (pthread_create(&cpu_clock_thread, 0, cpu_clock_worker, 0) != 0 ||
      pthread_getcpuclockid(cpu_clock_thread, &worker_cpu_clock) != 0 ||
      clock_gettime(worker_cpu_clock, &worker_cpu_start) != 0)
    return 282;
  while (atomic_load_explicit(&cpu_clock_worker_ticks, memory_order_relaxed) < 100000) {
  }
  if (clock_gettime(worker_cpu_clock, &worker_cpu_end) != 0 ||
      worker_cpu_end.tv_sec < worker_cpu_start.tv_sec ||
      (worker_cpu_end.tv_sec == worker_cpu_start.tv_sec &&
       worker_cpu_end.tv_nsec <= worker_cpu_start.tv_nsec))
    return 283;
  atomic_store_explicit(&cpu_clock_worker_release, 1, memory_order_release);
  if (pthread_join(cpu_clock_thread, 0) != 0) return 284;

  int initial_cancel_type = -1;
  if (pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, &initial_cancel_type) != 0 ||
      initial_cancel_type != PTHREAD_CANCEL_DEFERRED ||
      pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, &initial_cancel_type) != 0 ||
      initial_cancel_type != PTHREAD_CANCEL_ASYNCHRONOUS)
    return 197;
  int initial_cancel_state = -1;
  if (pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &initial_cancel_state) != 0 ||
      initial_cancel_state != PTHREAD_CANCEL_ENABLE ||
      pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &initial_cancel_state) != 0 ||
      initial_cancel_state != PTHREAD_CANCEL_DISABLE)
    return 214;
  cancel_cleanup_calls = 0;
  pthread_cleanup_push(cancel_cleanup, 0);
  pthread_cleanup_pop(1);
  if (cancel_cleanup_calls != 1 ||
      pthread_key_create(&initial_exit_key, initial_exit_key_destructor) != 0)
    return 215;
  if (pthread_atfork(atfork_prepare_a, atfork_parent_a, atfork_child_a) != 0 ||
      pthread_atfork(atfork_prepare_b, atfork_parent_b, atfork_child_b) != 0)
    return 279;

  volatile int *initial_exit_marker = mmap(0, sizeof(*initial_exit_marker),
      PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  if (initial_exit_marker == MAP_FAILED) return 216;
  *initial_exit_marker = 0;
  pid_t initial_exit_child = fork();
  if (initial_exit_child == 0) {
    if (atfork_order_count != 4 || atfork_order[0] != 2 ||
        atfork_order[1] != 1 || atfork_order[2] != 5 ||
        atfork_order[3] != 6)
      _exit(280);
    if (pthread_setspecific(initial_exit_key, (void *)initial_exit_marker) != 0)
      _exit(217);
    pthread_cleanup_push(initial_exit_cleanup, (void *)initial_exit_marker);
    pthread_exit(0);
    pthread_cleanup_pop(0);
  }
  int initial_exit_status = -1;
  if (initial_exit_child < 0 ||
      waitpid(initial_exit_child, &initial_exit_status, 0) != initial_exit_child ||
      initial_exit_status != 0 || *initial_exit_marker != 11 ||
      atfork_order_count != 4 || atfork_order[0] != 2 ||
      atfork_order[1] != 1 || atfork_order[2] != 3 ||
      atfork_order[3] != 4 ||
      pthread_key_delete(initial_exit_key) != 0 ||
      munmap((void *)initial_exit_marker, sizeof(*initial_exit_marker)) != 0)
    return 218;
  atfork_order_count = 0;

  volatile int *initial_cancel_marker = mmap(0, sizeof(*initial_cancel_marker),
      PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  if (initial_cancel_marker == MAP_FAILED) return 219;
  *initial_cancel_marker = 0;
  pid_t initial_cancel_child = fork();
  if (initial_cancel_child == 0) {
    sem_t release;
    if (sem_init(&release, 0, 0) != 0) _exit(220);
    initial_cancel_request request = {0};
    request.target = pthread_self();
    request.release = &release;
    pthread_cleanup_push(initial_exit_cleanup, (void *)initial_cancel_marker);
    pthread_t canceler;
    if (pthread_create(&canceler, 0, cancel_initial_worker, &request) != 0)
      _exit(221);
    atomic_store_explicit(&request.ready, 1, memory_order_release);
    sem_wait(&release);
    pthread_cleanup_pop(0);
    _exit(222);
  }
  int initial_cancel_status = -1;
  if (initial_cancel_child < 0 ||
      waitpid(initial_cancel_child, &initial_cancel_status, 0) != initial_cancel_child ||
      initial_cancel_status != 0 || *initial_cancel_marker != 1 ||
      munmap((void *)initial_cancel_marker, sizeof(*initial_cancel_marker)) != 0)
    return 223;

  pthread_attr_t attributes;
  if (pthread_attr_init(&attributes) != 0) return 1;
  pthread_attr_t user_stack_attributes;
  if (pthread_attr_init(&user_stack_attributes) != 0) return 232;
  void *reported_stack = (void *)1;
  size_t reported_stack_size = 0;
  if (pthread_attr_getstack(&user_stack_attributes, &reported_stack,
                            &reported_stack_size) != 0)
    return 236;
  if (reported_stack != 0 || reported_stack_size != 1048576) return 237;
  if (pthread_attr_getstack(&user_stack_attributes, 0, &reported_stack_size) != EINVAL ||
      pthread_attr_getstack(&user_stack_attributes, &reported_stack, 0) != EINVAL)
    return 238;
  if (pthread_attr_setstack(&user_stack_attributes, user_thread_stack,
                            PTHREAD_STACK_MIN - 1) != EINVAL ||
      pthread_attr_setstack(&user_stack_attributes, 0, sizeof(user_thread_stack)) != EINVAL ||
      pthread_attr_setstack(&user_stack_attributes,
                            (void *)(UINTPTR_MAX - 8191), 16384) != EINVAL)
    return 239;
  if (pthread_attr_setstack(&user_stack_attributes, user_thread_stack,
                            sizeof(user_thread_stack)) != 0) return 241;
  if (pthread_attr_getstack(&user_stack_attributes, &reported_stack,
                            &reported_stack_size) != 0 ||
      reported_stack != user_thread_stack ||
      reported_stack_size != sizeof(user_thread_stack)) return 242;
  user_thread_stack[0] = 0x5a;
  user_thread_stack_low = (uintptr_t)user_thread_stack;
  user_thread_stack_high = user_thread_stack_low + sizeof(user_thread_stack);
  pthread_t user_stack_thread;
  if (pthread_create(&user_stack_thread, &user_stack_attributes,
                     user_stack_worker, 0) != 0)
    return 233;
  void *user_stack_result = 0;
  if (pthread_join(user_stack_thread, &user_stack_result) != 0 ||
      user_stack_result != 0 || user_thread_stack[0] != 0x5a)
    return 234;
  if (pthread_attr_destroy(&user_stack_attributes) != 0) return 235;
  int scope = -1;
  int inheritsched = -1;
  int schedpolicy = -1;
  struct sched_param schedparam = {-1};
  if (pthread_attr_getscope(&attributes, &scope) != 0 ||
      scope != PTHREAD_SCOPE_SYSTEM ||
      pthread_attr_setscope(&attributes, PTHREAD_SCOPE_PROCESS) != ENOTSUP ||
      pthread_attr_setscope(&attributes, 99) != EINVAL ||
      pthread_attr_getscope(&attributes, 0) != EINVAL ||
      pthread_attr_getinheritsched(&attributes, &inheritsched) != 0 ||
      inheritsched != PTHREAD_INHERIT_SCHED ||
      pthread_attr_getschedpolicy(&attributes, &schedpolicy) != 0 ||
      schedpolicy != SCHED_OTHER ||
      pthread_attr_getschedparam(&attributes, &schedparam) != 0 ||
      schedparam.sched_priority != 0 ||
      pthread_attr_setinheritsched(&attributes, 99) != EINVAL ||
      pthread_attr_setschedpolicy(&attributes, 99) != EINVAL)
    return 225;
  schedparam.sched_priority = 1;
  if (pthread_attr_setschedpolicy(&attributes, SCHED_FIFO) != 0 ||
      pthread_attr_setschedparam(&attributes, &schedparam) != 0 ||
      pthread_attr_getschedpolicy(&attributes, &schedpolicy) != 0 ||
      schedpolicy != SCHED_FIFO ||
      pthread_attr_getschedparam(&attributes, &schedparam) != 0 ||
      schedparam.sched_priority != 1)
    return 226;
  schedparam.sched_priority = 0;
  if (pthread_attr_setschedpolicy(&attributes, SCHED_OTHER) != 0 ||
      pthread_attr_setschedparam(&attributes, &schedparam) != 0 ||
      pthread_attr_setinheritsched(&attributes, PTHREAD_EXPLICIT_SCHED) != 0)
    return 227;
  if (pthread_setschedparam(pthread_self(), SCHED_OTHER, &schedparam) != 0 ||
      pthread_setschedparam(pthread_self(), SCHED_OTHER, 0) != EINVAL ||
      pthread_setschedprio(pthread_self(), 0) != 0 ||
      pthread_setschedprio(pthread_self(), 1) != EINVAL)
    return 228;
  if (priority_protect_schedule_smoke() != 0) return 229;
  size_t guardsize = 0;
  if (pthread_attr_getguardsize(&attributes, &guardsize) != 0 || guardsize != 4096 ||
      pthread_attr_setguardsize(&attributes, 8192) != 0 ||
      pthread_attr_getguardsize(&attributes, &guardsize) != 0 || guardsize != 8192 ||
      pthread_attr_setguardsize(&attributes, SIZE_MAX) != EINVAL)
    return 224;
  if (pthread_attr_setstacksize(&attributes, 65536) != 0) return 2;
  if (pthread_attr_setdetachstate(&attributes, PTHREAD_CREATE_DETACHED) != 0)
    return 3;
  int detachstate = PTHREAD_CREATE_JOINABLE;
  if (pthread_attr_getdetachstate(&attributes, &detachstate) != 0 ||
      detachstate != PTHREAD_CREATE_DETACHED)
    return 4;

  pthread_t thread;
  if (pthread_attr_setdetachstate(&attributes, PTHREAD_CREATE_JOINABLE) != 0 ||
      pthread_create(&thread, &attributes, explicit_schedule_worker, 0) != 0)
    return 229;
  void *schedule_result = 0;
  if (pthread_join(thread, &schedule_result) != 0 || schedule_result != 0)
    return 230;
  if (pthread_attr_setdetachstate(&attributes, PTHREAD_CREATE_DETACHED) != 0)
    return 231;

  if (pthread_spin_init(&test_spinlock, PTHREAD_PROCESS_PRIVATE) != 0 ||
      pthread_spin_trylock(&test_spinlock) != 0 ||
      pthread_spin_trylock(&test_spinlock) != EBUSY ||
      pthread_spin_destroy(&test_spinlock) != EBUSY ||
      pthread_spin_unlock(&test_spinlock) != 0 ||
      pthread_spin_unlock(&test_spinlock) != EPERM)
    return 206;
  pthread_t spin_threads[4];
  spin_counter = 0;
  for (int i = 0; i < 4; ++i)
    if (pthread_create(&spin_threads[i], 0, spin_increment_worker, 0) != 0)
      return 207;
  for (int i = 0; i < 4; ++i)
    if (pthread_join(spin_threads[i], 0) != 0) return 208;
  if (spin_counter != 40000 || pthread_spin_destroy(&test_spinlock) != 0 ||
      pthread_spin_lock(&test_spinlock) != EINVAL ||
      pthread_spin_init(&test_spinlock, 2) != EINVAL)
    return 209;

  shared_spin_state *shared_spin = mmap(0, sizeof(*shared_spin),
      PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  if (shared_spin == MAP_FAILED ||
      pthread_spin_init(&shared_spin->lock, PTHREAD_PROCESS_SHARED) != 0)
    return 210;
  pid_t spin_child = fork();
  if (spin_child == 0) {
    if (pthread_spin_lock(&shared_spin->lock) != 0) _exit(211);
    ++shared_spin->value;
    if (pthread_spin_unlock(&shared_spin->lock) != 0) _exit(212);
    _exit(0);
  }
  int spin_child_status = -1;
  if (spin_child < 0 || waitpid(spin_child, &spin_child_status, 0) != spin_child ||
      spin_child_status != 0 || shared_spin->value != 1 ||
      pthread_spin_destroy(&shared_spin->lock) != 0 ||
      munmap(shared_spin, sizeof(*shared_spin)) != 0)
    return 213;

  pthread_mutex_t timed_mutex;
  if (pthread_mutex_init(&timed_mutex, 0) != 0) return 201;
  struct timespec invalid_mutex_deadline = {0, 1000000000L};
  if (pthread_mutex_timedlock(&timed_mutex, &invalid_mutex_deadline) != EINVAL)
    return 202;
  struct timespec expired_mutex_deadline = {0, 0};
  if (pthread_mutex_timedlock(&timed_mutex, &expired_mutex_deadline) != 0 ||
      pthread_mutex_unlock(&timed_mutex) != 0 ||
      pthread_mutex_lock(&timed_mutex) != 0)
    return 203;
  if (pthread_create(&thread, 0, timed_mutex_worker, &timed_mutex) != 0)
    return 204;
  void *timed_mutex_result = 0;
  if (pthread_join(thread, &timed_mutex_result) != 0 ||
      timed_mutex_result != (void *)(intptr_t)ETIMEDOUT ||
      pthread_mutex_unlock(&timed_mutex) != 0 ||
      pthread_mutex_destroy(&timed_mutex) != 0)
    return 205;

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

  cancel_waiting = 0;
  cancel_cleanup_calls = 0;
  if (pthread_create(&thread, 0, cancel_cond_worker, 0) != 0) return 130;
  for (unsigned int i = 0; i < 10000000U && !cancel_waiting; ++i) {
  }
  if (!cancel_waiting || pthread_cancel(thread) != 0) return 131;
  result = 0;
  if (pthread_join(thread, &result) != 0 || result != PTHREAD_CANCELED ||
      cancel_cleanup_calls != 1)
    return 132;
  if (pthread_mutex_lock(&cancel_mutex) != 0 ||
      pthread_mutex_unlock(&cancel_mutex) != 0)
    return 133;

  cancel_cleanup_calls = 0;
  cancel_read_started = 0;
  if (pipe(cancel_read_pipe) != 0) return 137;
  if (pthread_create(&thread, 0, cancel_read_worker, 0) != 0) return 138;
  for (unsigned int i = 0; i < 10000000U && !cancel_read_started; ++i) {
  }
  if (!cancel_read_started) return 139;
  for (volatile unsigned int i = 0; i < 10000000U; ++i) {
  }
  if (pthread_cancel(thread) != 0) return 140;
  result = 0;
  if (pthread_join(thread, &result) != 0 || result != PTHREAD_CANCELED ||
      cancel_cleanup_calls != 1)
    return 141;
  if (close(cancel_read_pipe[0]) != 0 || close(cancel_read_pipe[1]) != 0)
    return 142;

  cancel_cleanup_calls = 0;
  cancel_sleep_started = 0;
  if (pthread_create(&thread, 0, cancel_sleep_worker, 0) != 0) return 143;
  for (unsigned int i = 0; i < 10000000U && !cancel_sleep_started; ++i) {
  }
  if (!cancel_sleep_started) return 144;
  for (volatile unsigned int i = 0; i < 10000000U; ++i) {
  }
  if (pthread_cancel(thread) != 0) return 145;
  result = 0;
  if (pthread_join(thread, &result) != 0 || result != PTHREAD_CANCELED ||
      cancel_cleanup_calls != 1)
    return 146;

  if (sem_init(&cancel_semaphore, 0, 0) != 0) return 147;
  int semaphore_value = -1;
  if (sem_getvalue(&cancel_semaphore, &semaphore_value) != 0 || semaphore_value != 0)
    return 148;
  if (sem_trywait(&cancel_semaphore) != -1 || errno != EAGAIN) return 149;
  struct timespec expired = {0, 0};
  if (sem_timedwait(&cancel_semaphore, &expired) != -1 || errno != ETIMEDOUT)
    return 150;
  cancel_sem_started = 0;
  if (pthread_create(&thread, 0, cancel_sem_worker, 0) != 0) return 151;
  for (unsigned int i = 0; i < 10000000U && !cancel_sem_started; ++i) {
  }
  for (volatile unsigned int i = 0; i < 10000000U; ++i) {
  }
  if (!cancel_sem_started || sem_post(&cancel_semaphore) != 0) return 152;
  result = 0;
  if (pthread_join(thread, &result) != 0 || result != (void *)5) return 153;
  cancel_cleanup_calls = 0;
  cancel_sem_started = 0;
  if (pthread_create(&thread, 0, cancel_sem_worker, 0) != 0) return 154;
  for (unsigned int i = 0; i < 10000000U && !cancel_sem_started; ++i) {
  }
  for (volatile unsigned int i = 0; i < 10000000U; ++i) {
  }
  if (!cancel_sem_started || pthread_cancel(thread) != 0) return 155;
  result = 0;
  if (pthread_join(thread, &result) != 0 || result != PTHREAD_CANCELED ||
      cancel_cleanup_calls != 1)
    return 156;
  cancel_cleanup_calls = 0;
  cancel_sem_started = 0;
  if (pthread_create(&thread, 0, cancel_disabled_sem_worker, 0) != 0) return 166;
  for (unsigned int i = 0; i < 10000000U && !cancel_sem_started; ++i) {
  }
  for (volatile unsigned int i = 0; i < 10000000U; ++i) {
  }
  if (!cancel_sem_started || pthread_cancel(thread) != 0) return 167;
  if (sem_post(&cancel_semaphore) != 0) return 168;
  result = 0;
  if (pthread_join(thread, &result) != 0 || result != PTHREAD_CANCELED ||
      cancel_cleanup_calls != 1)
    return 169;
  if (sem_destroy(&cancel_semaphore) != 0) return 157;
  if (sem_init(&cancel_semaphore, 0, SEM_VALUE_MAX) != 0) return 158;
  if (sem_post(&cancel_semaphore) != -1 || errno != EOVERFLOW) return 159;
  if (sem_destroy(&cancel_semaphore) != 0) return 160;
  sem_t semaphore_edge_cases;
  if (sem_init(&semaphore_edge_cases, 2, 1) != 0) return 186;
  struct timespec semaphore_invalid_deadline = {0, 1000000000L};
  if (sem_timedwait(&semaphore_edge_cases, &semaphore_invalid_deadline) != 0) return 187;
  if (sem_post(&semaphore_edge_cases) != 0 || sem_timedwait(&semaphore_edge_cases, 0) != 0)
    return 188;
  if (sem_post(&semaphore_edge_cases) != 0 ||
      sem_clockwait(&semaphore_edge_cases, CLOCK_MONOTONIC,
                    &semaphore_invalid_deadline) != 0)
    return 193;
  if (sem_post(&semaphore_edge_cases) != 0 ||
      sem_clockwait(&semaphore_edge_cases, 12345, &expired) != -1 || errno != EINVAL ||
      sem_trywait(&semaphore_edge_cases) != 0)
    return 194;
  if (sem_timedwait(&semaphore_edge_cases, &semaphore_invalid_deadline) != -1 || errno != EINVAL)
    return 189;
  if (sem_clockwait(&semaphore_edge_cases, CLOCK_REALTIME, &expired) != -1 ||
      errno != ETIMEDOUT)
    return 195;
  if (sem_clockwait(&semaphore_edge_cases, CLOCK_MONOTONIC, &expired) != -1 ||
      errno != ETIMEDOUT)
    return 196;
  if (sem_destroy(&semaphore_edge_cases) != 0) return 190;
  if (snprintf(named_semaphore_path, sizeof named_semaphore_path,
               "/nelibc-test-sem-%ld", (long)getpid()) < 0)
    return 170;
  (void)sem_unlink(named_semaphore_path);
  pthread_t named_semaphore_race_threads[NAMED_SEMAPHORE_RACE_WORKERS];
  for (unsigned int i = 0; i < NAMED_SEMAPHORE_RACE_WORKERS; ++i) {
    named_semaphore_race_results[i].semaphore = SEM_FAILED;
    named_semaphore_race_results[i].value = -1;
    named_semaphore_race_results[i].error = 0;
    if (pthread_create(&named_semaphore_race_threads[i], 0,
                       named_semaphore_race_worker, (void *)(uintptr_t)i) != 0)
      return 180;
  }
  for (unsigned int i = 0; i < 10000000U &&
       atomic_load_explicit(&named_semaphore_race_ready, memory_order_acquire) !=
           NAMED_SEMAPHORE_RACE_WORKERS; ++i) {
  }
  if (atomic_load_explicit(&named_semaphore_race_ready, memory_order_acquire) !=
      NAMED_SEMAPHORE_RACE_WORKERS)
    return 181;
  atomic_store_explicit(&named_semaphore_race_go, 1, memory_order_release);
  for (unsigned int i = 0; i < NAMED_SEMAPHORE_RACE_WORKERS; ++i) {
    void *race_result = 0;
    if (pthread_join(named_semaphore_race_threads[i], &race_result) != 0 || race_result != 0)
      return 182;
  }
  int race_initial_value = named_semaphore_race_results[0].value;
  if (race_initial_value < 1 || race_initial_value > NAMED_SEMAPHORE_RACE_WORKERS)
    return 183;
  for (unsigned int i = 0; i < NAMED_SEMAPHORE_RACE_WORKERS; ++i) {
    if (named_semaphore_race_results[i].error != 0 ||
        named_semaphore_race_results[i].value != race_initial_value ||
        sem_close(named_semaphore_race_results[i].semaphore) != 0)
      return 184;
  }
  if (sem_unlink(named_semaphore_path) != 0) return 185;
  sem_t *named_semaphore = sem_open(named_semaphore_path, O_CREAT | O_EXCL, 0600, 1U);
  if (named_semaphore == SEM_FAILED) return 171;
  if (sem_wait(named_semaphore) != 0) return 172;
  sem_t *named_semaphore_second = sem_open(named_semaphore_path, O_CREAT, 0600, ~0U);
  if (named_semaphore_second == SEM_FAILED) return 173;
  if (sem_getvalue(named_semaphore_second, &semaphore_value) != 0 || semaphore_value != 0)
    return 174;
  if (sem_post(named_semaphore_second) != 0 || sem_wait(named_semaphore) != 0)
    return 175;
  if (sem_open(named_semaphore_path, O_CREAT | O_EXCL, 0600, ~0U) != SEM_FAILED ||
      errno != EEXIST)
    return 176;
  pid_t named_sem_child = fork();
  if (named_sem_child < 0) return 178;
  if (named_sem_child == 0) {
    sem_t *child_semaphore = sem_open(named_semaphore_path, 0);
    if (child_semaphore == SEM_FAILED || sem_post(child_semaphore) != 0 ||
        sem_close(child_semaphore) != 0)
      _exit(79);
    _exit(0);
  }
  int named_sem_child_status = -1;
  if (waitpid(named_sem_child, &named_sem_child_status, 0) != named_sem_child ||
      named_sem_child_status != 0 || sem_wait(named_semaphore) != 0)
    return 179;
  if (sem_unlink(named_semaphore_path) != 0 ||
      sem_post(named_semaphore_second) != 0 || sem_wait(named_semaphore) != 0 ||
      sem_close(named_semaphore_second) != 0 || sem_close(named_semaphore) != 0)
    return 177;
  if (sem_open(named_semaphore_path, O_CREAT, 0600, ~0U) != SEM_FAILED || errno != EINVAL)
    return 191;
  if (sem_open(named_semaphore_path, 0) != SEM_FAILED || errno != ENOENT) return 192;

  cancel_state_ready = 0;
  cancel_state_release = 0;
  cancel_cleanup_calls = 0;
  if (pthread_create(&thread, 0, cancel_disabled_worker, 0) != 0) return 134;
  for (unsigned int i = 0; i < 10000000U && !cancel_state_ready; ++i) {
  }
  if (!cancel_state_ready || pthread_cancel(thread) != 0) return 135;
  cancel_state_release = 1;
  result = 0;
  if (pthread_join(thread, &result) != 0 || result != PTHREAD_CANCELED ||
      cancel_cleanup_calls != 1)
    return 136;

  cancel_async_ready = 0;
  cancel_cleanup_calls = 0;
  if (pthread_create(&thread, 0, cancel_async_worker, 0) != 0) return 194;
  for (unsigned int i = 0; i < 10000000U && !cancel_async_ready; ++i) {
  }
  if (!cancel_async_ready || pthread_cancel(thread) != 0) return 195;
  result = 0;
  if (pthread_join(thread, &result) != 0 || result != PTHREAD_CANCELED ||
      cancel_cleanup_calls != 1)
    return 196;

  cancel_type_ready = 0;
  cancel_type_release = 0;
  cancel_cleanup_calls = 0;
  if (pthread_create(&thread, 0, cancel_pending_type_worker, 0) != 0) return 198;
  for (unsigned int i = 0; i < 10000000U && !cancel_type_ready; ++i) {
  }
  if (!cancel_type_ready || pthread_cancel(thread) != 0) return 199;
  cancel_type_release = 1;
  result = 0;
  if (pthread_join(thread, &result) != 0 || result != PTHREAD_CANCELED ||
      cancel_cleanup_calls != 1)
    return 200;

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

  if (pthread_mutexattr_init(&mutex_attributes) != 0) return 236;
  int robustness = -1;
  if (pthread_mutexattr_getrobust(&mutex_attributes, &robustness) != 0 ||
      robustness != PTHREAD_MUTEX_STALLED ||
      pthread_mutexattr_setrobust(&mutex_attributes, 99) != EINVAL ||
      pthread_mutexattr_setrobust(&mutex_attributes, PTHREAD_MUTEX_ROBUST) != 0 ||
      pthread_mutexattr_getrobust(&mutex_attributes, &robustness) != 0 ||
      robustness != PTHREAD_MUTEX_ROBUST)
    return 237;
  pthread_mutex_t robust_mutex;
  pthread_mutexattr_t pi_attributes;
  int protocol = -1;
  int prioceiling = -1;
  pthread_mutexattr_t ceiling_attributes;
  if (pthread_mutexattr_init(&ceiling_attributes) != 0 ||
      pthread_mutexattr_getprioceiling(&ceiling_attributes, &prioceiling) != 0 ||
      prioceiling != 1 ||
      pthread_mutexattr_setprioceiling(&ceiling_attributes, 99) != 0 ||
      pthread_mutexattr_getprioceiling(&ceiling_attributes, &prioceiling) != 0 ||
      prioceiling != 99 ||
      pthread_mutexattr_setprioceiling(&ceiling_attributes, 0) != EINVAL ||
      pthread_mutexattr_setprioceiling(&ceiling_attributes, 100) != EINVAL ||
      pthread_mutexattr_getprioceiling(0, &prioceiling) != EINVAL ||
      pthread_mutexattr_destroy(&ceiling_attributes) != 0)
    return 250;
  if (pthread_mutexattr_init(&pi_attributes) != 0 ||
      pthread_mutexattr_getprotocol(&pi_attributes, &protocol) != 0 ||
      protocol != PTHREAD_PRIO_NONE ||
      pthread_mutexattr_setprotocol(&pi_attributes, 99) != EINVAL ||
      pthread_mutexattr_setprotocol(&pi_attributes, PTHREAD_PRIO_PROTECT) != 0 ||
      pthread_mutexattr_getprotocol(&pi_attributes, &protocol) != 0 ||
      protocol != PTHREAD_PRIO_PROTECT)
    return 251;
  pthread_mutex_t protect_mutex;
  int protect_ceiling = -1;
  if (pthread_mutex_init(&protect_mutex, &pi_attributes) != 0 ||
      pthread_mutex_getprioceiling(&protect_mutex, &protect_ceiling) != 0 ||
      protect_ceiling != 1 ||
      pthread_mutex_setprioceiling(&protect_mutex, 7, &protect_ceiling) != 0 ||
      protect_ceiling != 1 ||
      pthread_mutex_getprioceiling(&protect_mutex, &protect_ceiling) != 0 ||
      protect_ceiling != 7 ||
      pthread_mutex_setprioceiling(&protect_mutex, 100, &protect_ceiling) != EINVAL ||
      pthread_mutex_lock(&protect_mutex) != EINVAL ||
      pthread_mutex_destroy(&protect_mutex) != 0 ||
      pthread_mutex_getprioceiling(0, &protect_ceiling) != EINVAL ||
      pthread_mutex_setprioceiling(0, 7, &protect_ceiling) != EINVAL ||
      pthread_mutexattr_setprotocol(&pi_attributes, PTHREAD_PRIO_INHERIT) != 0 ||
      pthread_mutexattr_getprotocol(&pi_attributes, &protocol) != 0 ||
      protocol != PTHREAD_PRIO_INHERIT)
    return 251;
  pthread_mutex_t pi_mutex;
  if (pthread_mutex_init(&pi_mutex, &pi_attributes) != 0 ||
      pthread_mutexattr_destroy(&pi_attributes) != 0 ||
      pthread_mutex_lock(&pi_mutex) != 0 ||
      pthread_mutex_trylock(&pi_mutex) != EBUSY)
    return 252;
  pthread_t pi_waiter;
  if (pthread_create(&pi_waiter, 0, pi_mutex_worker, &pi_mutex) != 0)
    return 253;
  int pi_waiter_queued = 0;
  for (int i = 0; i < 1000000; ++i) {
    if (pi_mutex.__state < 0) {
      pi_waiter_queued = 1;
      break;
    }
    sched_yield();
  }
  if (!pi_waiter_queued || pthread_mutex_unlock(&pi_mutex) != 0 ||
      pthread_join(pi_waiter, &result) != 0 || result != 0)
    return 254;
  if (pthread_mutex_lock(&pi_mutex) != 0) return 255;
  atomic_store_explicit(&pi_cancel_acquired, 0, memory_order_relaxed);
  if (pthread_create(&pi_waiter, 0, pi_cancel_mutex_worker, &pi_mutex) != 0)
    return 269;
  int pi_cancel_waiter_queued = 0;
  for (int i = 0; i < 1000000; ++i) {
    if (pi_mutex.__state < 0) {
      pi_cancel_waiter_queued = 1;
      break;
    }
    sched_yield();
  }
  if (!pi_cancel_waiter_queued || pthread_cancel(pi_waiter) != 0 ||
      pthread_mutex_unlock(&pi_mutex) != 0 ||
      pthread_join(pi_waiter, &result) != 0 || result != PTHREAD_CANCELED ||
      atomic_load_explicit(&pi_cancel_acquired, memory_order_acquire) != 1)
    return 270;
  if (pthread_mutex_lock(&pi_mutex) != 0) return 255;
  struct timespec invalid_pi_deadline = {0, -1};
  if (pthread_mutex_timedlock(&pi_mutex, &invalid_pi_deadline) != EINVAL)
    return 265;
  invalid_pi_deadline.tv_nsec = 1000000000L;
  if (pthread_mutex_timedlock(&pi_mutex, &invalid_pi_deadline) != EINVAL)
    return 266;
  pi_timed_mutex_request timed_request = {.mutex = &pi_mutex, .deadline = {0, 0}};
  if (pthread_create(&pi_waiter, 0, pi_timed_mutex_worker, &timed_request) != 0)
    return 256;
  if (pthread_join(pi_waiter, &result) != 0 || result != (void *)(intptr_t)ETIMEDOUT)
    return 257;
  if (clock_gettime(CLOCK_REALTIME, &timed_request.deadline) != 0)
    return 267;
  timed_request.deadline.tv_nsec += 100000000L;
  if (timed_request.deadline.tv_nsec >= 1000000000L) {
    ++timed_request.deadline.tv_sec;
    timed_request.deadline.tv_nsec -= 1000000000L;
  }
  if (pthread_create(&pi_waiter, 0, pi_timed_mutex_worker, &timed_request) != 0 ||
      pthread_join(pi_waiter, &result) != 0 ||
      result != (void *)(intptr_t)ETIMEDOUT ||
      pthread_mutex_unlock(&pi_mutex) != 0 ||
      pthread_mutex_destroy(&pi_mutex) != 0)
    return 268;

  if (pthread_mutex_init(&robust_mutex, &mutex_attributes) != 0 ||
      pthread_mutexattr_destroy(&mutex_attributes) != 0)
    return 238;
  pthread_t robust_owner;
  pthread_t recovery_waiter;
  if (sem_init(&robust_owner_release, 0, 0) != 0) return 239;
  atomic_store_explicit(&robust_owner_ready, 0, memory_order_relaxed);
  if (pthread_create(&robust_owner, 0, robust_gated_owner_worker,
                     &robust_mutex) != 0)
    return 244;
  while (atomic_load_explicit(&robust_owner_ready, memory_order_acquire) == 0)
    sched_yield();
  if (pthread_create(&recovery_waiter, 0, robust_recovery_waiter,
                     &robust_mutex) != 0)
    return 245;
  int robust_waiter_queued = 0;
  for (int i = 0; i < 1000000; ++i) {
    if (robust_mutex.__state < 0) {
      robust_waiter_queued = 1;
      break;
    }
    sched_yield();
  }
  if (!robust_waiter_queued || sem_post(&robust_owner_release) != 0 ||
      pthread_join(robust_owner, &result) != 0 || result != 0 ||
      pthread_join(recovery_waiter, &result) != 0 || result != 0 ||
      sem_destroy(&robust_owner_release) != 0)
    return 246;
  if (pthread_mutex_lock(&robust_mutex) != 0 ||
      pthread_mutex_unlock(&robust_mutex) != 0 ||
      pthread_mutex_destroy(&robust_mutex) != 0)
    return 240;

  if (pthread_mutexattr_init(&mutex_attributes) != 0 ||
      pthread_mutexattr_setrobust(&mutex_attributes, PTHREAD_MUTEX_ROBUST) != 0)
    return 241;
  if (pthread_mutex_init(&robust_mutex, &mutex_attributes) != 0 ||
      pthread_mutexattr_destroy(&mutex_attributes) != 0)
    return 242;
  if (pthread_create(&robust_owner, 0, robust_owner_worker, &robust_mutex) != 0 ||
      pthread_join(robust_owner, &result) != 0 || result != 0 ||
      pthread_mutex_lock(&robust_mutex) != EOWNERDEAD ||
      pthread_mutex_unlock(&robust_mutex) != 0 ||
      pthread_mutex_lock(&robust_mutex) != ENOTRECOVERABLE ||
      pthread_mutex_destroy(&robust_mutex) != 0)
    return 243;

  if (pthread_mutexattr_init(&mutex_attributes) != 0 ||
      pthread_mutexattr_setrobust(&mutex_attributes, PTHREAD_MUTEX_ROBUST) != 0 ||
      pthread_mutexattr_setprotocol(&mutex_attributes, PTHREAD_PRIO_INHERIT) != 0 ||
      pthread_mutex_init(&robust_mutex, &mutex_attributes) != 0 ||
      pthread_mutexattr_destroy(&mutex_attributes) != 0)
    return 258;
  if (pthread_create(&robust_owner, 0, robust_owner_worker, &robust_mutex) != 0 ||
      pthread_join(robust_owner, &result) != 0 || result != 0 ||
      pthread_mutex_lock(&robust_mutex) != EOWNERDEAD ||
      pthread_mutex_consistent(&robust_mutex) != 0 ||
      pthread_mutex_unlock(&robust_mutex) != 0 ||
      pthread_mutex_destroy(&robust_mutex) != 0)
    return 259;

  if (pthread_mutexattr_init(&mutex_attributes) != 0 ||
      pthread_mutexattr_setrobust(&mutex_attributes, PTHREAD_MUTEX_ROBUST) != 0 ||
      pthread_mutexattr_setprotocol(&mutex_attributes, PTHREAD_PRIO_INHERIT) != 0 ||
      pthread_mutex_init(&robust_mutex, &mutex_attributes) != 0 ||
      pthread_mutexattr_destroy(&mutex_attributes) != 0)
    return 260;
  if (sem_init(&robust_owner_release, 0, 0) != 0) return 261;
  atomic_store_explicit(&robust_owner_ready, 0, memory_order_relaxed);
  if (pthread_create(&robust_owner, 0, robust_gated_owner_worker,
                     &robust_mutex) != 0)
    return 262;
  while (atomic_load_explicit(&robust_owner_ready, memory_order_acquire) == 0)
    sched_yield();
  if (pthread_create(&recovery_waiter, 0, robust_recovery_waiter,
                     &robust_mutex) != 0)
    return 263;
  int robust_pi_waiter_queued = 0;
  for (int i = 0; i < 1000000; ++i) {
    if (robust_mutex.__state < 0) {
      robust_pi_waiter_queued = 1;
      break;
    }
    sched_yield();
  }
  if (!robust_pi_waiter_queued || sem_post(&robust_owner_release) != 0 ||
      pthread_join(robust_owner, &result) != 0 || result != 0 ||
      pthread_join(recovery_waiter, &result) != 0 || result != 0 ||
      sem_destroy(&robust_owner_release) != 0 ||
      pthread_mutex_lock(&robust_mutex) != 0 ||
      pthread_mutex_unlock(&robust_mutex) != 0 ||
      pthread_mutex_destroy(&robust_mutex) != 0)
    return 264;

  struct process_shared_robust_state {
    pthread_mutex_t mutex;
  };
  struct process_shared_robust_state *shared_robust = mmap(
      0, sizeof(*shared_robust), PROT_READ | PROT_WRITE,
      MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  if (shared_robust == MAP_FAILED) return 247;
  if (pthread_mutexattr_init(&mutex_attributes) != 0 ||
      pthread_mutexattr_setpshared(&mutex_attributes, PTHREAD_PROCESS_SHARED) != 0 ||
      pthread_mutexattr_setrobust(&mutex_attributes, PTHREAD_MUTEX_ROBUST) != 0 ||
      pthread_mutex_init(&shared_robust->mutex, &mutex_attributes) != 0 ||
      pthread_mutexattr_destroy(&mutex_attributes) != 0)
    return 248;
  pid_t robust_child = fork();
  if (robust_child == 0) {
    if (pthread_mutex_lock(&shared_robust->mutex) != 0) _exit(249);
    _exit(0);
  }
  int robust_child_status = -1;
  if (robust_child < 0 ||
      waitpid(robust_child, &robust_child_status, 0) != robust_child ||
      robust_child_status != 0 ||
      pthread_mutex_lock(&shared_robust->mutex) != EOWNERDEAD ||
      pthread_mutex_consistent(&shared_robust->mutex) != 0 ||
      pthread_mutex_unlock(&shared_robust->mutex) != 0 ||
      pthread_mutex_destroy(&shared_robust->mutex) != 0 ||
      munmap(shared_robust, sizeof(*shared_robust)) != 0)
    return 250;

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

  pthread_condattr_t monotonic_attributes;
  clockid_t selected_clock = -1;
  if (pthread_condattr_init(&monotonic_attributes) != 0 ||
      pthread_condattr_getclock(&monotonic_attributes, &selected_clock) != 0 ||
      selected_clock != CLOCK_REALTIME ||
      pthread_condattr_setclock(&monotonic_attributes, CLOCK_MONOTONIC_RAW) != EINVAL ||
      pthread_condattr_getclock(&monotonic_attributes, &selected_clock) != 0 ||
      selected_clock != CLOCK_REALTIME ||
      pthread_condattr_setclock(&monotonic_attributes, CLOCK_MONOTONIC) != 0 ||
      pthread_condattr_getclock(&monotonic_attributes, &selected_clock) != 0 ||
      selected_clock != CLOCK_MONOTONIC ||
      pthread_condattr_getclock(&monotonic_attributes, NULL) != EINVAL)
    return 271;
  pthread_cond_t monotonic_condition;
  pthread_mutex_t monotonic_mutex;
  if (pthread_cond_init(&monotonic_condition, &monotonic_attributes) != 0 ||
      pthread_condattr_destroy(&monotonic_attributes) != 0 ||
      pthread_mutex_init(&monotonic_mutex, NULL) != 0 ||
      pthread_mutex_lock(&monotonic_mutex) != 0)
    return 272;
  struct timespec monotonic_start;
  struct timespec monotonic_deadline;
  struct timespec monotonic_end;
  if (clock_gettime(CLOCK_MONOTONIC, &monotonic_start) != 0)
    return 273;
  monotonic_deadline = monotonic_start;
  monotonic_deadline.tv_nsec += 100000000L;
  if (monotonic_deadline.tv_nsec >= 1000000000L) {
    ++monotonic_deadline.tv_sec;
    monotonic_deadline.tv_nsec -= 1000000000L;
  }
  if (pthread_cond_timedwait(&monotonic_condition, &monotonic_mutex,
                             &monotonic_deadline) != ETIMEDOUT ||
      clock_gettime(CLOCK_MONOTONIC, &monotonic_end) != 0 ||
      monotonic_end.tv_sec < monotonic_deadline.tv_sec ||
      (monotonic_end.tv_sec == monotonic_deadline.tv_sec &&
       monotonic_end.tv_nsec < monotonic_deadline.tv_nsec) ||
      pthread_mutex_unlock(&monotonic_mutex) != 0 ||
      pthread_cond_destroy(&monotonic_condition) != 0 ||
      pthread_mutex_destroy(&monotonic_mutex) != 0)
    return 274;

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
  sem_t *shared_semaphore = mmap(0, sizeof(*shared_semaphore), PROT_READ | PROT_WRITE,
      MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  if (shared_semaphore == MAP_FAILED || sem_init(shared_semaphore, 1, 0) != 0)
    return 161;
  child = fork();
  if (child < 0) return 162;
  if (child == 0) process_shared_semaphore_child(shared_semaphore);
  for (unsigned int i = 0; i < 10000000U &&
       __atomic_load_n(&shared_semaphore->__waiters, __ATOMIC_ACQUIRE) == 0; ++i) {
  }
  if (__atomic_load_n(&shared_semaphore->__waiters, __ATOMIC_ACQUIRE) == 0 ||
      sem_post(shared_semaphore) != 0)
    return 163;
  child_status = -1;
  if (waitpid(child, &child_status, 0) != child || child_status != 0) return 164;
  if (sem_destroy(shared_semaphore) != 0 || munmap(shared_semaphore, sizeof(*shared_semaphore)) != 0)
    return 165;
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
  pthread_barrierattr_t barrier_attributes;
  int barrier_pshared = -1;
  if (pthread_barrierattr_init(&barrier_attributes) != 0 ||
      pthread_barrierattr_setpshared(&barrier_attributes, PTHREAD_PROCESS_SHARED) != 0 ||
      pthread_barrierattr_getpshared(&barrier_attributes, &barrier_pshared) != 0 ||
      barrier_pshared != PTHREAD_PROCESS_SHARED ||
      pthread_barrier_init(&round_barrier, &barrier_attributes, 4) != 0 ||
      pthread_barrier_init(&round_barrier, &barrier_attributes, 0) != EINVAL)
    return 197;
  pthread_t barrier_threads[4];
  for (int i = 0; i < 4; ++i) {
    if (pthread_create(&barrier_threads[i], &attributes, barrier_worker, 0) != 0)
      return 198;
  }
  for (int i = 0; i < 4; ++i) {
    if (pthread_join(barrier_threads[i], &result) != 0 || result != 0)
      return 199;
  }
  if (atomic_load_explicit(&barrier_serial_returns, memory_order_relaxed) != 100 ||
      pthread_barrier_destroy(&round_barrier) != 0)
    return 200;
  pthread_barrier_t *process_barrier = mmap(0, sizeof(*process_barrier),
      PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  if (process_barrier == MAP_FAILED ||
      pthread_barrier_init(process_barrier, &barrier_attributes, 2) != 0)
    return 201;
  child = fork();
  if (child < 0) return 202;
  if (child == 0) {
    int wait_result = pthread_barrier_wait(process_barrier);
    _exit(wait_result == 0 || wait_result == PTHREAD_BARRIER_SERIAL_THREAD ? 0 : 1);
  }
  int process_wait_result = pthread_barrier_wait(process_barrier);
  child_status = -1;
  if ((process_wait_result != 0 &&
       process_wait_result != PTHREAD_BARRIER_SERIAL_THREAD) ||
      waitpid(child, &child_status, 0) != child || child_status != 0 ||
      pthread_barrier_destroy(process_barrier) != 0 ||
      munmap(process_barrier, sizeof(*process_barrier)) != 0 ||
      pthread_barrierattr_destroy(&barrier_attributes) != 0)
    return 203;
  struct timespec invalid_rwlock_deadline = {0, 1000000000L};
  struct timespec expired_rwlock_deadline = {0, 0};
  if (pthread_rwlock_timedrdlock(&static_rwlock, &invalid_rwlock_deadline) != 0 ||
      pthread_rwlock_destroy(&static_rwlock) != EBUSY ||
      pthread_rwlock_unlock(&static_rwlock) != 0 ||
      pthread_rwlock_timedwrlock(&static_rwlock, &invalid_rwlock_deadline) != 0 ||
      pthread_rwlock_tryrdlock(&static_rwlock) != EBUSY ||
      pthread_rwlock_trywrlock(&static_rwlock) != EBUSY ||
      pthread_rwlock_timedrdlock(&static_rwlock, &expired_rwlock_deadline) != EDEADLK ||
      pthread_rwlock_unlock(&static_rwlock) != 0)
    return 204;
  if (pthread_rwlock_rdlock(&static_rwlock) != 0 ||
      pthread_create(&thread, &attributes, rwlock_stress_worker,
                     (void *)(uintptr_t)1) != 0)
    return 205;
  for (unsigned int i = 0; i < 10000000U &&
       __atomic_load_n(&static_rwlock.__waiting_writers, __ATOMIC_ACQUIRE) == 0; ++i) {
  }
  if (__atomic_load_n(&static_rwlock.__waiting_writers, __ATOMIC_ACQUIRE) == 0 ||
      pthread_rwlock_rdlock(&static_rwlock) != 0 ||
      pthread_rwlock_unlock(&static_rwlock) != 0 ||
      pthread_rwlock_unlock(&static_rwlock) != 0 ||
      pthread_join(thread, &result) != 0 || result != 0)
    return 206;
  if (pthread_rwlock_rdlock(&static_rwlock) != 0) return 219;
  struct rwlock_timeout_request timeout_request = {
    .rwlock = &static_rwlock,
    .deadline = {0, 0},
    .write_lock = 1,
  };
  if (pthread_create(&thread, &attributes, rwlock_timeout_worker,
                     &timeout_request) != 0 ||
      pthread_join(thread, &result) != 0 || result != 0 ||
      timeout_request.result != ETIMEDOUT ||
      pthread_rwlock_unlock(&static_rwlock) != 0)
    return 207;
  if (pthread_rwlock_wrlock(&static_rwlock) != 0) return 208;
  timeout_request.write_lock = 0;
  timeout_request.result = 0;
  if (pthread_create(&thread, &attributes, rwlock_timeout_worker,
                     &timeout_request) != 0 ||
      pthread_join(thread, &result) != 0 || result != 0 ||
      timeout_request.result != ETIMEDOUT ||
      pthread_rwlock_unlock(&static_rwlock) != 0)
    return 209;
  if (pthread_rwlock_rdlock(&static_rwlock) != 0 ||
      pthread_rwlock_timedwrlock(&static_rwlock, &invalid_rwlock_deadline) != EINVAL ||
      pthread_rwlock_unlock(&static_rwlock) != 0 ||
      pthread_rwlock_wrlock(&static_rwlock) != 0 ||
      pthread_rwlock_timedrdlock(&static_rwlock, &invalid_rwlock_deadline) != EDEADLK ||
      pthread_rwlock_unlock(&static_rwlock) != 0)
    return 210;
  pthread_t rwlock_threads[6];
  atomic_store_explicit(&rwlock_counter, 0, memory_order_relaxed);
  for (int i = 0; i < 6; ++i) {
    if (pthread_create(&rwlock_threads[i], &attributes, rwlock_stress_worker,
                       i < 4 ? (void *)(uintptr_t)1 : 0) != 0)
      return 211;
  }
  for (int i = 0; i < 6; ++i) {
    if (pthread_join(rwlock_threads[i], &result) != 0 || result != 0)
      return 212;
  }
  if (atomic_load_explicit(&rwlock_counter, memory_order_relaxed) != 2000 ||
      pthread_rwlock_destroy(&static_rwlock) != 0)
    return 213;
  pthread_rwlockattr_t rwlock_attributes;
  int rwlock_pshared = -1;
  if (pthread_rwlockattr_init(&rwlock_attributes) != 0 ||
      pthread_rwlockattr_setpshared(&rwlock_attributes, PTHREAD_PROCESS_SHARED) != 0 ||
      pthread_rwlockattr_getpshared(&rwlock_attributes, &rwlock_pshared) != 0 ||
      rwlock_pshared != PTHREAD_PROCESS_SHARED)
    return 214;
  struct process_shared_rwlock_state {
    pthread_rwlock_t rwlock;
    _Atomic int started;
    _Atomic int acquired;
  };
  struct process_shared_rwlock_state *shared_rwlock = mmap(0, sizeof(*shared_rwlock),
      PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  if (shared_rwlock == MAP_FAILED ||
      pthread_rwlock_init(&shared_rwlock->rwlock, &rwlock_attributes) != 0 ||
      pthread_rwlock_wrlock(&shared_rwlock->rwlock) != 0)
    return 215;
  atomic_store_explicit(&shared_rwlock->started, 0, memory_order_relaxed);
  atomic_store_explicit(&shared_rwlock->acquired, 0, memory_order_relaxed);
  child = fork();
  if (child < 0) return 216;
  if (child == 0) {
    atomic_store_explicit(&shared_rwlock->started, 1, memory_order_release);
    int lock_result = pthread_rwlock_wrlock(&shared_rwlock->rwlock);
    if (lock_result != 0) _exit(1);
    atomic_store_explicit(&shared_rwlock->acquired, 1, memory_order_release);
    _exit(pthread_rwlock_unlock(&shared_rwlock->rwlock) == 0 ? 0 : 2);
  }
  for (unsigned int i = 0; i < 10000000U &&
       (!atomic_load_explicit(&shared_rwlock->started, memory_order_acquire) ||
        __atomic_load_n(&shared_rwlock->rwlock.__waiting_writers, __ATOMIC_ACQUIRE) == 0); ++i) {
  }
  if (!atomic_load_explicit(&shared_rwlock->started, memory_order_acquire) ||
      __atomic_load_n(&shared_rwlock->rwlock.__waiting_writers, __ATOMIC_ACQUIRE) == 0 ||
      pthread_rwlock_unlock(&shared_rwlock->rwlock) != 0)
    return 217;
  child_status = -1;
  if (waitpid(child, &child_status, 0) != child || child_status != 0 ||
      !atomic_load_explicit(&shared_rwlock->acquired, memory_order_acquire) ||
      pthread_rwlock_destroy(&shared_rwlock->rwlock) != 0 ||
      munmap(shared_rwlock, sizeof(*shared_rwlock)) != 0 ||
      pthread_rwlockattr_destroy(&rwlock_attributes) != 0)
    return 218;
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
