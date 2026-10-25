#include <fcntl.h>
#include <errno.h>
#include <mqueue.h>
#include <pthread.h>
#include <signal.h>
#include <stdint.h>
#include <stdarg.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

struct mq_attr;
extern int __nelibc_mq_notify_syscall(int mqdes,
                                     const struct sigevent *notification);
extern int __nelibc_mq_open_impl(const char *name, int flags, unsigned int mode,
                                 const struct mq_attr *attributes);

struct mq_notify_thread_args {
  pthread_barrier_t ready;
  int socket_fd;
  void (*callback)(union sigval);
  union sigval value;
};

static void *mq_notify_thread_start(void *opaque) {
  struct mq_notify_thread_args *args = opaque;
  int socket_fd = args->socket_fd;
  void (*callback)(union sigval) = args->callback;
  union sigval value = args->value;
  char message[32];

  pthread_barrier_wait(&args->ready);
  ssize_t size = recv(socket_fd, message, sizeof(message), MSG_NOSIGNAL | MSG_WAITALL);
  close(socket_fd);
  if (size == (ssize_t)sizeof(message) && message[sizeof(message) - 1] == 1)
    callback(value);
  return NULL;
}

/* POSIX mq_open is variadic only when O_CREAT supplies mode and attributes. */
int mq_open(const char *name, int flags, ...) {
  unsigned int mode = 0;
  const struct mq_attr *attributes = 0;
  if ((flags & O_CREAT) != 0) {
    va_list arguments;
    va_start(arguments, flags);
    mode = va_arg(arguments, mode_t);
    attributes = va_arg(arguments, const struct mq_attr *);
    va_end(arguments);
  }
  return __nelibc_mq_open_impl(name, flags, mode, attributes);
}

int mq_notify(mqd_t mqdes, const struct sigevent *notification) {
  if (notification == NULL || notification->sigev_notify != SIGEV_THREAD)
    return __nelibc_mq_notify_syscall(mqdes, notification);
  if (notification->sigev_notify_function == NULL) {
    errno = EINVAL;
    return -1;
  }

  int socket_fd = socket(AF_NETLINK, SOCK_RAW | SOCK_CLOEXEC, 0);
  if (socket_fd < 0) return -1;

  struct mq_notify_thread_args args = {
    .socket_fd = socket_fd,
    .callback = notification->sigev_notify_function,
    .value = notification->sigev_value,
  };
  pthread_attr_t attributes;
  int attributes_initialized = 0;
  int error;
  error = pthread_attr_init(&attributes);
  if (error != 0) {
    close(socket_fd);
    errno = error;
    return -1;
  }
  attributes_initialized = 1;
  const pthread_attr_t *requested = notification->sigev_notify_attributes;
  if (requested != NULL) {
    void *stack_address;
    size_t stack_size, guard_size;
    int scope, inherit_scheduler, scheduler_policy;
    struct sched_param scheduler_parameters;
    error = pthread_attr_getstacksize(requested, &stack_size);
    if (error != 0) goto attr_error;
    size_t reported_stack_size;
    error = pthread_attr_getstack(requested, &stack_address,
                                  &reported_stack_size);
    if (error != 0 && error != EINVAL) goto attr_error;
    /* glibc represents an unset stack address as (void *)-stack_size. */
    if (error == 0 && stack_address != NULL &&
        (uintptr_t)stack_address + stack_size != 0)
      error = pthread_attr_setstack(&attributes, stack_address, stack_size);
    else
      error = pthread_attr_setstacksize(&attributes, stack_size);
    if (error != 0) goto attr_error;
#define GET_SET(getter, setter, output) do { \
      error = getter(requested, &(output)); \
      if (error == 0) error = setter(&attributes, (output)); \
      if (error != 0) goto attr_error; \
    } while (0)
    GET_SET(pthread_attr_getguardsize, pthread_attr_setguardsize, guard_size);
    GET_SET(pthread_attr_getscope, pthread_attr_setscope, scope);
    GET_SET(pthread_attr_getinheritsched, pthread_attr_setinheritsched,
            inherit_scheduler);
    GET_SET(pthread_attr_getschedpolicy, pthread_attr_setschedpolicy,
            scheduler_policy);
#undef GET_SET
    error = pthread_attr_getschedparam(requested, &scheduler_parameters);
    if (error == 0)
      error = pthread_attr_setschedparam(&attributes, &scheduler_parameters);
    if (error != 0) goto attr_error;
  }
  error = pthread_attr_setdetachstate(&attributes, PTHREAD_CREATE_DETACHED);
  if (error != 0) {
    if (attributes_initialized) pthread_attr_destroy(&attributes);
    close(socket_fd);
    errno = error;
    return -1;
  }
  error = pthread_barrier_init(&args.ready, NULL, 2);
  if (error != 0) {
    if (attributes_initialized) pthread_attr_destroy(&attributes);
    close(socket_fd);
    errno = error;
    return -1;
  }

  pthread_t thread;
  error = pthread_create(&thread, &attributes, mq_notify_thread_start, &args);
  if (attributes_initialized) pthread_attr_destroy(&attributes);
  if (error != 0) {
    pthread_barrier_destroy(&args.ready);
    close(socket_fd);
    errno = error;
    return -1;
  }

  static const char empty_payload[32];
  struct sigevent kernel_notification = {0};
  kernel_notification.sigev_notify = SIGEV_THREAD;
  kernel_notification.sigev_signo = socket_fd;
  kernel_notification.sigev_value.sival_ptr = (void *)empty_payload;
  int result = __nelibc_mq_notify_syscall(mqdes, &kernel_notification);
  if (result < 0) pthread_cancel(thread);

  pthread_barrier_wait(&args.ready);
  pthread_barrier_destroy(&args.ready);
  if (result < 0) close(socket_fd);
  return result;

attr_error:
  if (attributes_initialized) pthread_attr_destroy(&attributes);
  close(socket_fd);
  errno = error;
  return -1;
}
