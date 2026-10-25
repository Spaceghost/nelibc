#define _GNU_SOURCE
#include <errno.h>
#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <wchar.h>
#include <string.h>

int getdate_err;

int *nelibc_getdate_err_location(void) {
  return &getdate_err;
}
#include <locale.h>
#include <time.h>

#ifdef NELIBC_STANDALONE
extern void nelibc_pthread_cancel_defer_begin(void) __attribute__((weak));
extern void nelibc_pthread_cancel_defer_end(void) __attribute__((weak));
#endif

void nelibc_allocator_cancel_defer_begin(void) {
#ifdef NELIBC_STANDALONE
  if (nelibc_pthread_cancel_defer_begin != NULL)
    nelibc_pthread_cancel_defer_begin();
#endif
}

void nelibc_allocator_cancel_defer_end(void) {
#ifdef NELIBC_STANDALONE
  if (nelibc_pthread_cancel_defer_end != NULL)
    nelibc_pthread_cancel_defer_end();
#endif
}

/* timer_t is opaque and has different public representations across libcs. */
void nelibc_timerid_store(void *output, int timer_id) {
  timer_t value = (timer_t)(uintptr_t)timer_id;
  memcpy(output, &value, sizeof(value));
}

int nelibc_atfork_lock_try(int *lock) {
  int expected = 0;
  return __atomic_compare_exchange_n(lock, &expected, 1, 0,
                                     __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE);
}

void nelibc_atfork_lock_reset(int *lock) {
  __atomic_store_n(lock, 0, __ATOMIC_RELEASE);
}

extern void nelibc_allocator_lock(void) __attribute__((weak));
extern void nelibc_allocator_unlock(void) __attribute__((weak));
extern void nelibc_stdio_fork_prepare(void) __attribute__((weak));
extern void nelibc_stdio_fork_parent(void) __attribute__((weak));
extern void nelibc_stdio_fork_child(void) __attribute__((weak));

void nelibc_fork_prepare(void) {
  if (nelibc_stdio_fork_prepare != NULL) nelibc_stdio_fork_prepare();
  if (nelibc_allocator_lock != NULL) nelibc_allocator_lock();
}

void nelibc_fork_parent(void) {
  if (nelibc_allocator_unlock != NULL) nelibc_allocator_unlock();
  if (nelibc_stdio_fork_parent != NULL) nelibc_stdio_fork_parent();
}

void nelibc_fork_child(void) {
  if (nelibc_allocator_unlock != NULL) nelibc_allocator_unlock();
  if (nelibc_stdio_fork_child != NULL) nelibc_stdio_fork_child();
}

#include <stdlib.h>
#include <stdio.h>
#ifndef NELIBC_STANDALONE
#include <langinfo.h>
#include <dlfcn.h>
#include <pthread.h>
#ifndef NELIBC_STATIC_MUSL
static pthread_once_t nelibc_locale_host_once = PTHREAD_ONCE_INIT;
static struct lconv *(*nelibc_localeconv_host)(void);
static char *(*nelibc_setlocale_host)(int, const char *);
static int (*nelibc_fseeko_host)(FILE *, off_t, int);
static off_t (*nelibc_ftello_host)(FILE *);
static void (*nelibc_exit_host)(int);
static void nelibc_resolve_locale_host(void) {
  nelibc_localeconv_host = (struct lconv *(*)(void))dlsym(RTLD_NEXT, "localeconv");
  nelibc_setlocale_host = (char *(*)(int, const char *))dlsym(RTLD_NEXT, "setlocale");
  nelibc_fseeko_host = (int (*)(FILE *, off_t, int))dlsym(RTLD_NEXT, "fseeko");
  nelibc_ftello_host = (off_t (*)(FILE *))dlsym(RTLD_NEXT, "ftello");
  nelibc_exit_host = (void (*)(int))dlsym(RTLD_NEXT, "exit");
}
struct lconv *nelibc_host_localeconv(void) {
  (void)pthread_once(&nelibc_locale_host_once, nelibc_resolve_locale_host);
  return nelibc_localeconv_host != NULL ? nelibc_localeconv_host() : NULL;
}
char *nelibc_host_setlocale(int category, const char *locale) {
  (void)pthread_once(&nelibc_locale_host_once, nelibc_resolve_locale_host);
  return nelibc_setlocale_host != NULL
      ? nelibc_setlocale_host(category, locale) : NULL;
}
int nelibc_host_fseeko(FILE *stream, long offset, int whence) {
  (void)pthread_once(&nelibc_locale_host_once, nelibc_resolve_locale_host);
  return nelibc_fseeko_host != NULL
      ? nelibc_fseeko_host(stream, (off_t)offset, whence) : -1;
}
long nelibc_host_ftello(FILE *stream) {
  (void)pthread_once(&nelibc_locale_host_once, nelibc_resolve_locale_host);
  return nelibc_ftello_host != NULL
      ? (long)nelibc_ftello_host(stream) : -1L;
}
void nelibc_host_exit(int status) {
  (void)pthread_once(&nelibc_locale_host_once, nelibc_resolve_locale_host);
  if (nelibc_exit_host != NULL) nelibc_exit_host(status);
  _Exit(status);
}
#endif
#endif

#if defined(__TINYC__) && !defined(NELIBC_STANDALONE)
/* Use C TLS: TinyCC emits it even though Nelua's backend does not. */
static pthread_key_t nelibc_state_key;
static pthread_once_t nelibc_state_key_once = PTHREAD_ONCE_INIT;
static int nelibc_state_key_error;
static __thread int nelibc_state_cleanup_registered;
static __thread int nelibc_state_cleanup_initializing;
static __thread union {
  long double alignment;
  unsigned char bytes[18][256];
} nelibc_states;
extern void nelibc_allocator_thread_cleanup(void *) __attribute__((weak));
static void nelibc_destroy_state(void *state) {
  if (nelibc_allocator_thread_cleanup != NULL)
    nelibc_allocator_thread_cleanup(state);
}
static void nelibc_initialize_state_key(void) {
  nelibc_state_key_error = pthread_key_create(&nelibc_state_key,
                                               nelibc_destroy_state);
}
#elif !defined(NELIBC_STANDALONE)
extern void nelibc_allocator_thread_cleanup(void *) __attribute__((weak));
static pthread_key_t nelibc_state_cleanup_key;
static pthread_once_t nelibc_state_cleanup_once = PTHREAD_ONCE_INIT;
static int nelibc_state_cleanup_error;
static _Thread_local int nelibc_state_cleanup_registered;
static _Thread_local int nelibc_state_cleanup_initializing;
static void nelibc_destroy_state(void *state) {
  if (nelibc_allocator_thread_cleanup != NULL)
    nelibc_allocator_thread_cleanup(state);
}
static void nelibc_initialize_state_cleanup_key(void) {
  nelibc_state_cleanup_error = pthread_key_create(&nelibc_state_cleanup_key,
                                                   nelibc_destroy_state);
}
#endif

extern char **environ;
extern int nelibc_waitpid(pid_t pid, int *status, int options);
int nelibc_cancellation_enter(void);
void nelibc_cancellation_leave(int previous_type);
extern long nelibc_raw_syscall3(unsigned long number, unsigned long arg1,
                                unsigned long arg2, unsigned long arg3)
    __attribute__((weak));
extern long nelibc_raw_syscall6(unsigned long number, unsigned long arg1,
                                unsigned long arg2, unsigned long arg3,
                                unsigned long arg4, unsigned long arg5,
                                unsigned long arg6) __attribute__((weak));
extern void nelibc_rt_sigreturn(void) __attribute__((weak));
#ifdef NELIBC_USE_GLIBC_CANCEL_WRAPPERS
#include <poll.h>
#include <sys/resource.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
typedef ssize_t (*nelibc_read_fn)(int, void *, size_t);
typedef ssize_t (*nelibc_write_fn)(int, const void *, size_t);
typedef ssize_t (*nelibc_readv_fn)(int, const struct iovec *, int);
typedef ssize_t (*nelibc_writev_fn)(int, const struct iovec *, int);
typedef int (*nelibc_close_fn)(int);
typedef int (*nelibc_poll_fn)(struct pollfd *, nfds_t, int);
typedef int (*nelibc_accept_fn)(int, struct sockaddr *, socklen_t *);
typedef int (*nelibc_accept4_fn)(int, struct sockaddr *, socklen_t *, int);
typedef int (*nelibc_openat_fn)(int, const char *, int, mode_t);
typedef int (*nelibc_sync_fn)(int);
typedef ssize_t (*nelibc_pread_fn)(int, void *, size_t, off_t);
typedef ssize_t (*nelibc_pwrite_fn)(int, const void *, size_t, off_t);
typedef int (*nelibc_select_fn)(int, fd_set *, fd_set *, fd_set *, struct timeval *);
typedef int (*nelibc_nanosleep_fn)(const struct timespec *, struct timespec *);
typedef int (*nelibc_clock_nanosleep_fn)(clockid_t, int, const struct timespec *, struct timespec *);
typedef int (*nelibc_pause_fn)(void);
typedef int (*nelibc_connect_fn)(int, const struct sockaddr *, socklen_t);
typedef ssize_t (*nelibc_sendto_fn)(int, const void *, size_t, int, const struct sockaddr *, socklen_t);
typedef ssize_t (*nelibc_recvfrom_fn)(int, void *, size_t, int, struct sockaddr *, socklen_t *);
typedef ssize_t (*nelibc_sendmsg_fn)(int, const struct msghdr *, int);
typedef ssize_t (*nelibc_recvmsg_fn)(int, struct msghdr *, int);
typedef pid_t (*nelibc_wait4_fn)(pid_t, int *, int, struct rusage *);
typedef int (*nelibc_waitid_fn)(idtype_t, id_t, siginfo_t *, int);
typedef ssize_t (*nelibc_vectored_offset_fn)(int, const struct iovec *, int, off_t);
typedef ssize_t (*nelibc_vectored_offset_flags_fn)(int, const struct iovec *, int, off_t, int);
static struct {
  nelibc_read_fn read;
  nelibc_write_fn write;
  nelibc_readv_fn readv;
  nelibc_writev_fn writev;
  nelibc_close_fn close;
  nelibc_poll_fn poll;
  nelibc_accept_fn accept;
  nelibc_accept4_fn accept4;
  nelibc_openat_fn openat;
  nelibc_sync_fn fsync;
  nelibc_sync_fn fdatasync;
  nelibc_pread_fn pread;
  nelibc_pwrite_fn pwrite;
  nelibc_vectored_offset_fn preadv;
  nelibc_vectored_offset_fn pwritev;
  nelibc_vectored_offset_flags_fn preadv2;
  nelibc_vectored_offset_flags_fn pwritev2;
  nelibc_select_fn select;
  nelibc_nanosleep_fn nanosleep;
  nelibc_clock_nanosleep_fn clock_nanosleep;
  nelibc_pause_fn pause;
  nelibc_connect_fn connect;
  nelibc_sendto_fn sendto;
  nelibc_recvfrom_fn recvfrom;
  nelibc_sendmsg_fn sendmsg;
  nelibc_recvmsg_fn recvmsg;
  nelibc_wait4_fn wait4;
  nelibc_waitid_fn waitid;
} nelibc_glibc;
static pthread_once_t nelibc_glibc_once = PTHREAD_ONCE_INIT;
static void nelibc_glibc_resolve(void) {
  nelibc_glibc.read = (nelibc_read_fn)dlvsym(RTLD_NEXT, "__read", "GLIBC_2.2.5");
  nelibc_glibc.write = (nelibc_write_fn)dlvsym(RTLD_NEXT, "__write", "GLIBC_2.2.5");
  nelibc_glibc.readv = (nelibc_readv_fn)dlvsym(RTLD_NEXT, "readv", "GLIBC_2.2.5");
  nelibc_glibc.writev = (nelibc_writev_fn)dlvsym(RTLD_NEXT, "writev", "GLIBC_2.2.5");
  nelibc_glibc.close = (nelibc_close_fn)dlvsym(RTLD_NEXT, "__close", "GLIBC_2.2.5");
  nelibc_glibc.poll = (nelibc_poll_fn)dlvsym(RTLD_NEXT, "poll", "GLIBC_2.2.5");
  nelibc_glibc.accept = (nelibc_accept_fn)dlvsym(RTLD_NEXT, "accept", "GLIBC_2.2.5");
  nelibc_glibc.accept4 = (nelibc_accept4_fn)dlvsym(RTLD_NEXT, "accept4", "GLIBC_2.10");
  nelibc_glibc.openat = (nelibc_openat_fn)dlvsym(RTLD_NEXT, "openat", "GLIBC_2.4");
  nelibc_glibc.fsync = (nelibc_sync_fn)dlvsym(RTLD_NEXT, "fsync", "GLIBC_2.2.5");
  nelibc_glibc.fdatasync = (nelibc_sync_fn)dlvsym(RTLD_NEXT, "fdatasync", "GLIBC_2.2.5");
  nelibc_glibc.pread = (nelibc_pread_fn)dlvsym(RTLD_NEXT, "pread", "GLIBC_2.2.5");
  nelibc_glibc.pwrite = (nelibc_pwrite_fn)dlvsym(RTLD_NEXT, "pwrite", "GLIBC_2.2.5");
  nelibc_glibc.preadv = (nelibc_vectored_offset_fn)dlvsym(RTLD_NEXT, "preadv", "GLIBC_2.10");
  nelibc_glibc.pwritev = (nelibc_vectored_offset_fn)dlvsym(RTLD_NEXT, "pwritev", "GLIBC_2.10");
  nelibc_glibc.preadv2 = (nelibc_vectored_offset_flags_fn)dlvsym(RTLD_NEXT, "preadv2", "GLIBC_2.26");
  nelibc_glibc.pwritev2 = (nelibc_vectored_offset_flags_fn)dlvsym(RTLD_NEXT, "pwritev2", "GLIBC_2.26");
  nelibc_glibc.select = (nelibc_select_fn)dlvsym(RTLD_NEXT, "select", "GLIBC_2.2.5");
  nelibc_glibc.nanosleep = (nelibc_nanosleep_fn)dlvsym(RTLD_NEXT, "nanosleep", "GLIBC_2.2.5");
  nelibc_glibc.clock_nanosleep = (nelibc_clock_nanosleep_fn)dlvsym(RTLD_NEXT, "clock_nanosleep", "GLIBC_2.17");
  nelibc_glibc.pause = (nelibc_pause_fn)dlvsym(RTLD_NEXT, "pause", "GLIBC_2.2.5");
  nelibc_glibc.connect = (nelibc_connect_fn)dlvsym(RTLD_NEXT, "connect", "GLIBC_2.2.5");
  nelibc_glibc.sendto = (nelibc_sendto_fn)dlvsym(RTLD_NEXT, "sendto", "GLIBC_2.2.5");
  nelibc_glibc.recvfrom = (nelibc_recvfrom_fn)dlvsym(RTLD_NEXT, "recvfrom", "GLIBC_2.2.5");
  nelibc_glibc.sendmsg = (nelibc_sendmsg_fn)dlvsym(RTLD_NEXT, "sendmsg", "GLIBC_2.2.5");
  nelibc_glibc.recvmsg = (nelibc_recvmsg_fn)dlvsym(RTLD_NEXT, "recvmsg", "GLIBC_2.2.5");
  nelibc_glibc.wait4 = (nelibc_wait4_fn)dlvsym(RTLD_NEXT, "wait4", "GLIBC_2.2.5");
  nelibc_glibc.waitid = (nelibc_waitid_fn)dlvsym(RTLD_NEXT, "waitid", "GLIBC_2.2.5");
}
static long nelibc_glibc_result(long r) { return r == -1 ? -(long)errno : r; }
#define NELIBC_GLIBC_RESULT_OR_FALLBACK(function, call) \
  ((function) != NULL ? nelibc_glibc_result(call) : LONG_MIN)
static long nelibc_glibc_cp3(unsigned long nr, unsigned long a, unsigned long b,
                             unsigned long c) {
  (void)pthread_once(&nelibc_glibc_once, nelibc_glibc_resolve);
  switch (nr) {
    case 0: return NELIBC_GLIBC_RESULT_OR_FALLBACK(nelibc_glibc.read, nelibc_glibc.read((int)a, (void *)b, c));
    case 1: return NELIBC_GLIBC_RESULT_OR_FALLBACK(nelibc_glibc.write, nelibc_glibc.write((int)a, (void *)b, c));
    case 3: return NELIBC_GLIBC_RESULT_OR_FALLBACK(nelibc_glibc.close, nelibc_glibc.close((int)a));
    case 7: return NELIBC_GLIBC_RESULT_OR_FALLBACK(nelibc_glibc.poll, nelibc_glibc.poll((void *)a, b, (int)c));
    case 19: return NELIBC_GLIBC_RESULT_OR_FALLBACK(nelibc_glibc.readv, nelibc_glibc.readv((int)a, (void *)b, (int)c));
    case 20: return NELIBC_GLIBC_RESULT_OR_FALLBACK(nelibc_glibc.writev, nelibc_glibc.writev((int)a, (void *)b, (int)c));
    case 43: return NELIBC_GLIBC_RESULT_OR_FALLBACK(nelibc_glibc.accept, nelibc_glibc.accept((int)a, (void *)b, (void *)c));
    case 34: return NELIBC_GLIBC_RESULT_OR_FALLBACK(nelibc_glibc.pause, nelibc_glibc.pause());
    case 35: return NELIBC_GLIBC_RESULT_OR_FALLBACK(nelibc_glibc.nanosleep, nelibc_glibc.nanosleep((void *)a, (void *)b));
    case 42: return NELIBC_GLIBC_RESULT_OR_FALLBACK(nelibc_glibc.connect, nelibc_glibc.connect((int)a, (void *)b, (socklen_t)c));
    case 74: return NELIBC_GLIBC_RESULT_OR_FALLBACK(nelibc_glibc.fsync, nelibc_glibc.fsync((int)a));
    case 75: return NELIBC_GLIBC_RESULT_OR_FALLBACK(nelibc_glibc.fdatasync, nelibc_glibc.fdatasync((int)a));
    default: return LONG_MIN;
  }
}
static long nelibc_glibc_cp6(unsigned long nr, unsigned long a, unsigned long b,
                             unsigned long c, unsigned long d, unsigned long e,
                             unsigned long f) {
  (void)pthread_once(&nelibc_glibc_once, nelibc_glibc_resolve);
  switch (nr) {
    case 257: return NELIBC_GLIBC_RESULT_OR_FALLBACK(nelibc_glibc.openat, nelibc_glibc.openat((int)a, (void *)b, (int)c, (mode_t)d));
    case 44: return NELIBC_GLIBC_RESULT_OR_FALLBACK(nelibc_glibc.sendto, nelibc_glibc.sendto((int)a, (void *)b, c, (int)d, (void *)e, (socklen_t)f));
    case 45: return NELIBC_GLIBC_RESULT_OR_FALLBACK(nelibc_glibc.recvfrom, nelibc_glibc.recvfrom((int)a, (void *)b, c, (int)d, (void *)e, (void *)f));
    case 46: return NELIBC_GLIBC_RESULT_OR_FALLBACK(nelibc_glibc.sendmsg, nelibc_glibc.sendmsg((int)a, (void *)b, (int)c));
    case 47: return NELIBC_GLIBC_RESULT_OR_FALLBACK(nelibc_glibc.recvmsg, nelibc_glibc.recvmsg((int)a, (void *)b, (int)c));
    case 61: return NELIBC_GLIBC_RESULT_OR_FALLBACK(nelibc_glibc.wait4, nelibc_glibc.wait4((pid_t)a, (void *)b, (int)c, (void *)d));
    case 17: return NELIBC_GLIBC_RESULT_OR_FALLBACK(nelibc_glibc.pread, nelibc_glibc.pread((int)a, (void *)b, c, (off_t)d));
    case 18: return NELIBC_GLIBC_RESULT_OR_FALLBACK(nelibc_glibc.pwrite, nelibc_glibc.pwrite((int)a, (void *)b, c, (off_t)d));
    case 23: return NELIBC_GLIBC_RESULT_OR_FALLBACK(nelibc_glibc.select, nelibc_glibc.select((int)a, (void *)b, (void *)c, (void *)d, (void *)e));
    case 230: { if (nelibc_glibc.clock_nanosleep == NULL) return LONG_MIN; int r = nelibc_glibc.clock_nanosleep((clockid_t)a, (int)b, (void *)c, (void *)d); return r ? -(long)r : 0; }
    case 288: return NELIBC_GLIBC_RESULT_OR_FALLBACK(nelibc_glibc.accept4, nelibc_glibc.accept4((int)a, (void *)b, (void *)c, (int)d));
    case 247: return NELIBC_GLIBC_RESULT_OR_FALLBACK(nelibc_glibc.waitid, nelibc_glibc.waitid((idtype_t)a, (id_t)b, (siginfo_t *)c, (int)d));
    case 295: return NELIBC_GLIBC_RESULT_OR_FALLBACK(nelibc_glibc.preadv, nelibc_glibc.preadv((int)a, (void *)b, (int)c, (off_t)d));
    case 296: return NELIBC_GLIBC_RESULT_OR_FALLBACK(nelibc_glibc.pwritev, nelibc_glibc.pwritev((int)a, (void *)b, (int)c, (off_t)d));
    case 327: return NELIBC_GLIBC_RESULT_OR_FALLBACK(nelibc_glibc.preadv2, nelibc_glibc.preadv2((int)a, (void *)b, (int)c, (off_t)d, (int)f));
    case 328: return NELIBC_GLIBC_RESULT_OR_FALLBACK(nelibc_glibc.pwritev2, nelibc_glibc.pwritev2((int)a, (void *)b, (int)c, (off_t)d, (int)f));
    default: (void)e; (void)f; return LONG_MIN;
  }
}
#undef NELIBC_GLIBC_RESULT_OR_FALLBACK
#endif
#ifdef NELIBC_USE_MUSL_SYSCALL_CP
extern long __syscall_cp(long number, long arg1, long arg2, long arg3,
                         long arg4, long arg5, long arg6);
long nelibc_cancellable_syscall3(unsigned long number, unsigned long arg1,
                                 unsigned long arg2, unsigned long arg3) {
  return __syscall_cp((long)number, (long)arg1, (long)arg2, (long)arg3,
                      0, 0, 0);
}
long nelibc_cancellable_syscall6(unsigned long number, unsigned long arg1,
                                 unsigned long arg2, unsigned long arg3,
                                 unsigned long arg4, unsigned long arg5,
                                 unsigned long arg6) {
  return __syscall_cp((long)number, (long)arg1, (long)arg2, (long)arg3,
                      (long)arg4, (long)arg5, (long)arg6);
}
#else
long nelibc_cancellable_syscall3(unsigned long number, unsigned long arg1,
                                 unsigned long arg2, unsigned long arg3) {
#ifdef NELIBC_USE_GLIBC_CANCEL_WRAPPERS
  long host_result = nelibc_glibc_cp3(number, arg1, arg2, arg3);
  if (host_result != LONG_MIN) return host_result;
#endif
  int previous_type = nelibc_cancellation_enter();
  long result = nelibc_raw_syscall3(number, arg1, arg2, arg3);
  nelibc_cancellation_leave(previous_type);
  return result;
}
long nelibc_cancellable_syscall6(unsigned long number, unsigned long arg1,
                                 unsigned long arg2, unsigned long arg3,
                                 unsigned long arg4, unsigned long arg5,
                                 unsigned long arg6) {
#ifdef NELIBC_USE_GLIBC_CANCEL_WRAPPERS
  long host_result = nelibc_glibc_cp6(number, arg1, arg2, arg3, arg4, arg5, arg6);
  if (host_result != LONG_MIN) return host_result;
#endif
  int previous_type = nelibc_cancellation_enter();
  long result = nelibc_raw_syscall6(number, arg1, arg2, arg3, arg4, arg5,
                                    arg6);
  nelibc_cancellation_leave(previous_type);
  return result;
}
#endif
#ifdef NELIBC_STANDALONE
extern void pthread_testcancel(void) __attribute__((weak));
extern void nelibc_pthread_cancel_signal(void);
static void nelibc_cancel_signal_handler(int signal_number) {
  (void)signal_number;
  nelibc_pthread_cancel_signal();
}

int nelibc_thread_cancel_signal_install(void) {
  if (nelibc_raw_syscall6 == NULL || nelibc_rt_sigreturn == NULL) return ENOSYS;
  struct nelibc_kernel_sigaction {
    void (*handler)(int);
    unsigned long flags;
    void (*restorer)(void);
    unsigned long mask;
  } action = {
    .handler = nelibc_cancel_signal_handler,
    .flags = 0x04000000UL, /* SA_RESTORER, without SA_RESTART */
    .restorer = nelibc_rt_sigreturn,
    .mask = 0,
  };
  long result = nelibc_raw_syscall6(13, 32, (unsigned long)&action,
                                   0, sizeof(unsigned long), 0, 0);
  return result < 0 && result >= -4095 ? (int)-result : 0;
}

int nelibc_thread_cancel_signal_unblock(void) {
  if (nelibc_raw_syscall6 == NULL) return ENOSYS;
  unsigned long set = 1UL << 31; /* signal 32 is reserved for cancellation */
  long result = nelibc_raw_syscall6(14, 1, (unsigned long)&set, 0,
                                   sizeof(unsigned long), 0, 0);
  return result < 0 && result >= -4095 ? (int)-result : 0;
}

int nelibc_cancellation_enter(void) {
  if (pthread_testcancel != NULL) pthread_testcancel();
  return -1;
}
void nelibc_cancellation_leave(int previous_type) {
  (void)previous_type;
  if (pthread_testcancel != NULL) pthread_testcancel();
}
#elif defined(NELIBC_WITH_UNISTD)
int nelibc_cancellation_enter(void) {
  int previous_type;
  if (pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, &previous_type) != 0)
    return -1;
  return previous_type;
}
void nelibc_cancellation_leave(int previous_type) {
  if (previous_type < 0) return;
  (void)pthread_setcanceltype(previous_type, NULL);
  if (previous_type == PTHREAD_CANCEL_DEFERRED) pthread_testcancel();
}
#else
extern void pthread_testcancel(void) __attribute__((weak));
extern int pthread_setcanceltype(int type, int *oldtype) __attribute__((weak));
int nelibc_cancellation_enter(void) {
  if (pthread_setcanceltype == NULL) return -1;
  int previous_type;
  if (pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, &previous_type) != 0)
    return -1;
  return previous_type;
}
void nelibc_cancellation_leave(int previous_type) {
  if (previous_type < 0 || pthread_setcanceltype == NULL) return;
  (void)pthread_setcanceltype(previous_type, NULL);
  if (previous_type == PTHREAD_CANCEL_DEFERRED && pthread_testcancel != NULL)
    pthread_testcancel();
}
#endif
#ifdef NELIBC_STANDALONE
int waitpid(pid_t pid, int *status, int options) {
  return nelibc_waitpid(pid, status, options);
}
#elif defined(NELIBC_WITH_UNISTD)
int waitpid(pid_t pid, int *status, int options) {
  return nelibc_waitpid(pid, status, options);
}
#endif
int nelibc_allocator_fastpath_supported =
#ifdef NELIBC_STANDALONE
    0;
#elif defined(NELIBC_ALLOCATOR_FASTPATH_SAFE)
    1;
#else
    0;
#endif

char *optarg;
int optind = 1;
int opterr = 1;
int optopt;

static int nelibc_locale_name_equal(const char *left, const char *right) {
  size_t index = 0;
  while (left[index] != 0 && right[index] != 0) {
    unsigned char a = (unsigned char)left[index];
    unsigned char b = (unsigned char)right[index];
    if (a >= 'A' && a <= 'Z') a = (unsigned char)(a + ('a' - 'A'));
    if (b >= 'A' && b <= 'Z') b = (unsigned char)(b + ('a' - 'A'));
    if (a != b) return 0;
    ++index;
  }
  return left[index] == right[index];
}

#ifdef NELIBC_STANDALONE
typedef struct nelibc_thread_tid_slot {
  int tid;
  int allocated;
  int released;
} nelibc_thread_tid_slot;

static nelibc_thread_tid_slot nelibc_thread_tid_slots[1024];
static int nelibc_thread_tid_slots_lock;

static void nelibc_thread_tid_slots_acquire(void) {
  while (__atomic_exchange_n(&nelibc_thread_tid_slots_lock, 1, __ATOMIC_ACQUIRE)) {
    while (__atomic_load_n(&nelibc_thread_tid_slots_lock, __ATOMIC_RELAXED)) {
    }
  }
}

static void nelibc_thread_tid_slots_release(void) {
  __atomic_store_n(&nelibc_thread_tid_slots_lock, 0, __ATOMIC_RELEASE);
}

int *nelibc_thread_tid_slot_alloc(void) {
  nelibc_thread_tid_slots_acquire();
  for (size_t i = 0; i < sizeof(nelibc_thread_tid_slots) / sizeof(nelibc_thread_tid_slots[0]); ++i) {
    nelibc_thread_tid_slot *slot = &nelibc_thread_tid_slots[i];
    if (slot->allocated && slot->released &&
        __atomic_load_n(&slot->tid, __ATOMIC_ACQUIRE) == 0) {
      slot->allocated = 0;
      slot->released = 0;
    }
    if (!slot->allocated) {
      slot->tid = 0;
      slot->allocated = 1;
      slot->released = 0;
      nelibc_thread_tid_slots_release();
      return &slot->tid;
    }
  }
  nelibc_thread_tid_slots_release();
  return NULL;
}

void nelibc_thread_tid_slot_release(int *tid) {
  if (tid == NULL) return;
  nelibc_thread_tid_slots_acquire();
  for (size_t i = 0; i < sizeof(nelibc_thread_tid_slots) / sizeof(nelibc_thread_tid_slots[0]); ++i) {
    nelibc_thread_tid_slot *slot = &nelibc_thread_tid_slots[i];
    if (&slot->tid == tid) {
      slot->released = 1;
      if (__atomic_load_n(&slot->tid, __ATOMIC_ACQUIRE) == 0)
        slot->allocated = 0;
      break;
    }
  }
  nelibc_thread_tid_slots_release();
}

int nelibc_atomic_load_int(const int *value) {
  return __atomic_load_n(value, __ATOMIC_ACQUIRE);
}

void nelibc_atomic_store_int(int *value, int next) {
  __atomic_store_n(value, next, __ATOMIC_RELEASE);
}

int nelibc_atomic_cas_int(int *value, int expected, int next) {
  return __atomic_compare_exchange_n(value, &expected, next, 0,
                                     __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE) ? 0 : 1;
}

int nelibc_atomic_exchange_int(int *value, int next) {
  return __atomic_exchange_n(value, next, __ATOMIC_ACQ_REL);
}

int nelibc_atomic_fetch_add_int(int *value, int delta) {
  return __atomic_fetch_add(value, delta, __ATOMIC_ACQ_REL);
}

void *nelibc_atomic_load_pointer(void *const *value) {
  return __atomic_load_n(value, __ATOMIC_ACQUIRE);
}

void nelibc_atomic_store_pointer(void **value, void *next) {
  __atomic_store_n(value, next, __ATOMIC_RELEASE);
}

int nelibc_thread_tid_load(const int *tid) {
  return __atomic_load_n(tid, __ATOMIC_ACQUIRE);
}

void *nelibc_thread_current_identity(void) {
  void *thread_pointer = __builtin_thread_pointer();
  void *identity = ((void **)thread_pointer)[1];
  return identity != NULL ? identity : thread_pointer;
}

void *nelibc_thread_current_pthread_context(void) {
  void *thread_pointer = __builtin_thread_pointer();
  return ((void **)thread_pointer)[2];
}

void nelibc_thread_set_pthread_context(void *context) {
  void *thread_pointer = __builtin_thread_pointer();
  ((void **)thread_pointer)[2] = context;
}

void *nelibc_thread_current_key_values(void) {
  void *thread_pointer = __builtin_thread_pointer();
  return ((void **)thread_pointer)[4];
}

void nelibc_thread_set_key_values(void *values) {
  void *thread_pointer = __builtin_thread_pointer();
  ((void **)thread_pointer)[4] = values;
}

int nelibc_thread_current_context_detached(void) {
  typedef struct {
    void *start;
    void *arg;
    void *result;
    void *handle;
    void *object;
    int lifecycle;
  } pthread_context_prefix;
  void **thread_pointer = __builtin_thread_pointer();
  pthread_context_prefix *context = thread_pointer[2];
  if (context == NULL) return 0;
  int lifecycle = __atomic_load_n(&context->lifecycle, __ATOMIC_ACQUIRE);
  return lifecycle == 1 || lifecycle == 3 || lifecycle == 5 || lifecycle == 6;
}

__attribute__((naked, noreturn))
void nelibc_thread_detached_exit(void) {
  __asm__ volatile(
      "mov %r8, %rsp\n"
      "add %r9, %rsp\n"
      "mov %rdi, %r10\n"
      "mov %rsi, %r11\n"
      "mov %rdx, %r12\n"
      "mov %rcx, %r13\n"
      "mov %r8, %r14\n"
      "mov %r9, %r15\n"
      "mov $11, %rax\n"
      "mov %r10, %rdi\n"
      "mov %r11, %rsi\n"
      "syscall\n"
      "mov $11, %rax\n"
      "mov %r12, %rdi\n"
      "mov %r13, %rsi\n"
      "syscall\n"
      "mov $11, %rax\n"
      "mov %r14, %rdi\n"
      "mov %r15, %rsi\n"
      "syscall\n"
      "xor %edi, %edi\n"
      "mov $60, %rax\n"
      "syscall\n"
      "ud2\n");
}

static int nelibc_standalone_ctype_utf8;
static int nelibc_standalone_numeric_de;
static int nelibc_standalone_monetary_de;
static int nelibc_standalone_time_de;
void *nelibc_thread_state(unsigned int slot, size_t size);
static char nelibc_standalone_locale_c[] = "C";
static char nelibc_standalone_locale_utf8[] = "C.UTF-8";
static char nelibc_standalone_locale_de_numeric[] = "de_DE.UTF-8";
static char nelibc_standalone_locale_de_monetary[] = "de_DE.UTF-8";
static char nelibc_standalone_locale_de_time[] = "de_DE.UTF-8";
static const char nelibc_standalone_locale_all_utf8[] =
    "LC_CTYPE=C.UTF-8;LC_NUMERIC=C;LC_TIME=C;LC_COLLATE=C;"
    "LC_MONETARY=C;LC_MESSAGES=C";
static const char nelibc_standalone_locale_all_de[] =
    "LC_CTYPE=C;LC_NUMERIC=de_DE.UTF-8;LC_TIME=C;LC_COLLATE=C;"
    "LC_MONETARY=C;LC_MESSAGES=C";
static const char nelibc_standalone_locale_all_utf8_de[] =
    "LC_CTYPE=C.UTF-8;LC_NUMERIC=de_DE.UTF-8;LC_TIME=C;LC_COLLATE=C;"
    "LC_MONETARY=C;LC_MESSAGES=C";
static const char nelibc_standalone_locale_all_mon[] =
    "LC_CTYPE=C;LC_NUMERIC=C;LC_TIME=C;LC_COLLATE=C;"
    "LC_MONETARY=de_DE.UTF-8;LC_MESSAGES=C";
static const char nelibc_standalone_locale_all_utf8_mon[] =
    "LC_CTYPE=C.UTF-8;LC_NUMERIC=C;LC_TIME=C;LC_COLLATE=C;"
    "LC_MONETARY=de_DE.UTF-8;LC_MESSAGES=C";
static const char nelibc_standalone_locale_all_de_mon[] =
    "LC_CTYPE=C;LC_NUMERIC=de_DE.UTF-8;LC_TIME=C;LC_COLLATE=C;"
    "LC_MONETARY=de_DE.UTF-8;LC_MESSAGES=C";
static const char nelibc_standalone_locale_all_utf8_de_mon[] =
    "LC_CTYPE=C.UTF-8;LC_NUMERIC=de_DE.UTF-8;LC_TIME=C;LC_COLLATE=C;"
    "LC_MONETARY=de_DE.UTF-8;LC_MESSAGES=C";
static const char nelibc_standalone_locale_all_time[] =
    "LC_CTYPE=C;LC_NUMERIC=C;LC_TIME=de_DE.UTF-8;LC_COLLATE=C;"
    "LC_MONETARY=C;LC_MESSAGES=C";
static const char nelibc_standalone_locale_all_utf8_time[] =
    "LC_CTYPE=C.UTF-8;LC_NUMERIC=C;LC_TIME=de_DE.UTF-8;LC_COLLATE=C;"
    "LC_MONETARY=C;LC_MESSAGES=C";
static const char nelibc_standalone_locale_all_de_time[] =
    "LC_CTYPE=C;LC_NUMERIC=de_DE.UTF-8;LC_TIME=de_DE.UTF-8;LC_COLLATE=C;"
    "LC_MONETARY=C;LC_MESSAGES=C";
static const char nelibc_standalone_locale_all_utf8_de_time[] =
    "LC_CTYPE=C.UTF-8;LC_NUMERIC=de_DE.UTF-8;LC_TIME=de_DE.UTF-8;LC_COLLATE=C;"
    "LC_MONETARY=C;LC_MESSAGES=C";
static const char nelibc_standalone_locale_all_mon_time[] =
    "LC_CTYPE=C;LC_NUMERIC=C;LC_TIME=de_DE.UTF-8;LC_COLLATE=C;"
    "LC_MONETARY=de_DE.UTF-8;LC_MESSAGES=C";
static const char nelibc_standalone_locale_all_utf8_mon_time[] =
    "LC_CTYPE=C.UTF-8;LC_NUMERIC=C;LC_TIME=de_DE.UTF-8;LC_COLLATE=C;"
    "LC_MONETARY=de_DE.UTF-8;LC_MESSAGES=C";
static const char nelibc_standalone_locale_all_de_mon_time[] =
    "LC_CTYPE=C;LC_NUMERIC=de_DE.UTF-8;LC_TIME=de_DE.UTF-8;LC_COLLATE=C;"
    "LC_MONETARY=de_DE.UTF-8;LC_MESSAGES=C";
static const char nelibc_standalone_locale_all_utf8_de_mon_time[] =
    "LC_CTYPE=C.UTF-8;LC_NUMERIC=de_DE.UTF-8;LC_TIME=de_DE.UTF-8;LC_COLLATE=C;"
    "LC_MONETARY=de_DE.UTF-8;LC_MESSAGES=C";
static const char *const nelibc_standalone_locale_all_composites[16] = {
  nelibc_standalone_locale_c,
  nelibc_standalone_locale_all_utf8,
  nelibc_standalone_locale_all_de,
  nelibc_standalone_locale_all_utf8_de,
  nelibc_standalone_locale_all_mon,
  nelibc_standalone_locale_all_utf8_mon,
  nelibc_standalone_locale_all_de_mon,
  nelibc_standalone_locale_all_utf8_de_mon,
  nelibc_standalone_locale_all_time,
  nelibc_standalone_locale_all_utf8_time,
  nelibc_standalone_locale_all_de_time,
  nelibc_standalone_locale_all_utf8_de_time,
  nelibc_standalone_locale_all_mon_time,
  nelibc_standalone_locale_all_utf8_mon_time,
  nelibc_standalone_locale_all_de_mon_time,
  nelibc_standalone_locale_all_utf8_de_mon_time,
};

int nelibc_standalone_global_ctype_mode(void) {
  return nelibc_standalone_ctype_utf8;
}

int nelibc_standalone_global_numeric_is_de(void) {
  return nelibc_standalone_numeric_de;
}

int nelibc_standalone_global_monetary_is_de(void) {
  return nelibc_standalone_monetary_de;
}

int nelibc_standalone_global_time_is_de(void) {
  return nelibc_standalone_time_de;
}

int nelibc_standalone_global_collate_mode(void) {
  return 0;
}

int nelibc_standalone_collate_mode(void) {
  int *thread_locale = (int *)nelibc_thread_state(16, 5 * sizeof(int));
  if (thread_locale != NULL && thread_locale[3] != 0)
    return thread_locale[3] - 1;
  return nelibc_standalone_global_collate_mode();
}

int nelibc_standalone_numeric_is_de(void) {
  int *thread_locale = (int *)nelibc_thread_state(16, 2 * sizeof(int));
  if (thread_locale != NULL && thread_locale[1] != 0)
    return thread_locale[1] == 2;
  return nelibc_standalone_global_numeric_is_de();
}

int nelibc_standalone_monetary_is_de(void) {
  int *thread_locale = (int *)nelibc_thread_state(16, 3 * sizeof(int));
  if (thread_locale != NULL && thread_locale[2] != 0)
    return thread_locale[2] == 2;
  return nelibc_standalone_monetary_de;
}

static int nelibc_standalone_locale_category_valid(int category) {
  if (category == LC_ALL || category == LC_COLLATE || category == LC_CTYPE ||
      category == LC_MONETARY || category == LC_NUMERIC || category == LC_TIME)
    return 1;
#ifdef LC_MESSAGES
  if (category == LC_MESSAGES) return 1;
#endif
  return 0;
}

char *nelibc_standalone_setlocale(int category, const char *name) {
  if (!nelibc_standalone_locale_category_valid(category)) return NULL;
  if (name == NULL) {
    if (category == LC_ALL) {
      unsigned int state = (nelibc_standalone_ctype_utf8 ? 1u : 0u) |
          (nelibc_standalone_numeric_de ? 2u : 0u) |
          (nelibc_standalone_monetary_de ? 4u : 0u) |
          (nelibc_standalone_time_de ? 8u : 0u);
      return (char *)nelibc_standalone_locale_all_composites[state];
    }
    if (category == LC_CTYPE && nelibc_standalone_ctype_utf8)
      return nelibc_standalone_locale_utf8;
    if (category == LC_NUMERIC && nelibc_standalone_numeric_de)
      return nelibc_standalone_locale_de_numeric;
    if (category == LC_MONETARY && nelibc_standalone_monetary_de)
      return nelibc_standalone_locale_de_monetary;
    if (category == LC_TIME && nelibc_standalone_time_de)
      return nelibc_standalone_locale_de_time;
    return nelibc_standalone_locale_c;
  }
  if (nelibc_locale_name_equal(name, "C") || nelibc_locale_name_equal(name, "POSIX")) {
    if (category == LC_CTYPE || category == LC_ALL)
      nelibc_standalone_ctype_utf8 = 0;
    if (category == LC_NUMERIC || category == LC_ALL)
      nelibc_standalone_numeric_de = 0;
    if (category == LC_MONETARY || category == LC_ALL)
      nelibc_standalone_monetary_de = 0;
    if (category == LC_TIME || category == LC_ALL)
      nelibc_standalone_time_de = 0;
    return nelibc_standalone_locale_c;
  }
  if (category == LC_NUMERIC &&
      (nelibc_locale_name_equal(name, "de_DE.UTF-8") ||
       nelibc_locale_name_equal(name, "de_DE.utf8") ||
       nelibc_locale_name_equal(name, "de_DE"))) {
    nelibc_standalone_numeric_de = 1;
    return nelibc_standalone_locale_de_numeric;
  }
  if (category == LC_MONETARY &&
      (nelibc_locale_name_equal(name, "de_DE.UTF-8") ||
       nelibc_locale_name_equal(name, "de_DE.utf8") ||
       nelibc_locale_name_equal(name, "de_DE"))) {
    nelibc_standalone_monetary_de = 1;
    return nelibc_standalone_locale_de_monetary;
  }
  if (category == LC_TIME &&
      (nelibc_locale_name_equal(name, "de_DE.UTF-8") ||
       nelibc_locale_name_equal(name, "de_DE.utf8") ||
       nelibc_locale_name_equal(name, "de_DE"))) {
    nelibc_standalone_time_de = 1;
    return nelibc_standalone_locale_de_time;
  }
  if (nelibc_locale_name_equal(name, "C.UTF-8") ||
      nelibc_locale_name_equal(name, "C.utf8") ||
      nelibc_locale_name_equal(name, "C.UTF8")) {
    if (category == LC_CTYPE) {
      nelibc_standalone_ctype_utf8 = 1;
      return nelibc_standalone_locale_utf8;
    }
    if (category == LC_ALL) {
      nelibc_standalone_ctype_utf8 = 1;
      nelibc_standalone_numeric_de = 0;
      nelibc_standalone_monetary_de = 0;
      nelibc_standalone_time_de = 0;
      return (char *)nelibc_standalone_locale_all_utf8;
    }
  }
  if (category == LC_ALL &&
      strcmp(name, nelibc_standalone_locale_all_utf8) == 0) {
    nelibc_standalone_ctype_utf8 = 1;
    nelibc_standalone_numeric_de = 0;
    nelibc_standalone_monetary_de = 0;
    nelibc_standalone_time_de = 0;
    return (char *)nelibc_standalone_locale_all_utf8;
  }
  if (category == LC_ALL) {
    for (unsigned int state = 0; state < 16; ++state) {
      const char *composite = nelibc_standalone_locale_all_composites[state];
      if (strcmp(name, composite) == 0) {
        nelibc_standalone_ctype_utf8 = (state & 1u) != 0;
        nelibc_standalone_numeric_de = (state & 2u) != 0;
        nelibc_standalone_monetary_de = (state & 4u) != 0;
        nelibc_standalone_time_de = (state & 8u) != 0;
        return (char *)composite;
      }
    }
  }
  return NULL;
}
#endif

int nelibc_time_locale_is_german(void) {
#ifdef NELIBC_STANDALONE
  int *thread_locale = (int *)nelibc_thread_state(16, 5 * sizeof(int));
  if (thread_locale != NULL && thread_locale[4] != 0)
    return thread_locale[4] == 2;
  return nelibc_standalone_time_de;
#else
  const char *january = nl_langinfo(MON_1);
  return january != NULL && strcmp(january, "Januar") == 0;
#endif
}

int nelibc_radix_character(void) {
#ifdef NELIBC_STANDALONE
  return nelibc_standalone_numeric_is_de() ? ',' : '.';
#else
  struct lconv *data = localeconv();
  return data != NULL && data->decimal_point != NULL && data->decimal_point[0]
      ? (unsigned char)data->decimal_point[0] : '.';
#endif
}

int nelibc_grouping_size(int position) {
  if (position < 0) return -1;
  struct lconv *data = localeconv();
  if (data == NULL || data->grouping == NULL) return -1;
  int previous = 0;
  for (int index = 0; index < 32; ++index) {
    unsigned char value = (unsigned char)data->grouping[index];
    if (value == (unsigned char)CHAR_MAX) return -1;
    if (value == 0) return previous > 0 ? previous : -1;
    previous = value;
    if (index == position) return value;
  }
  return -1;
}

int nelibc_grouping_separator_length(void) {
  struct lconv *data = localeconv();
  return data != NULL && data->thousands_sep != NULL
      ? (int)strlen(data->thousands_sep) : 0;
}

int nelibc_grouping_separator_byte(int index) {
  struct lconv *data = localeconv();
  return data != NULL && data->thousands_sep != NULL && index >= 0 &&
      data->thousands_sep[index] != '\0'
      ? (unsigned char)data->thousands_sep[index] : 0;
}

int nelibc_multibyte_mode(void) {
#ifdef NELIBC_STANDALONE
  int *thread_locale = (int *)nelibc_thread_state(16, 2 * sizeof(int));
  if (thread_locale != NULL && *thread_locale != 0) return *thread_locale - 1;
  return nelibc_standalone_ctype_utf8 ? 1 : 0;
#else
  const char *codeset = nl_langinfo(CODESET);
  if (codeset == NULL) return 2;
  if (nelibc_locale_name_equal(codeset, "UTF-8") ||
      nelibc_locale_name_equal(codeset, "UTF8")) return 1;
  if (nelibc_locale_name_equal(codeset, "C") ||
      nelibc_locale_name_equal(codeset, "POSIX") ||
      nelibc_locale_name_equal(codeset, "ASCII") ||
      nelibc_locale_name_equal(codeset, "US-ASCII") ||
      nelibc_locale_name_equal(codeset, "ANSI_X3.4-1968")) return 0;
  return 2;
#endif
}

void *nelibc_thread_state(unsigned int slot, size_t size) {
  if (slot >= 18 || size > 256) return NULL;
#if defined(__TINYC__) && !defined(NELIBC_STANDALONE)
  if (slot >= 7 && !nelibc_state_cleanup_registered) {
    /* pthread key setup can allocate. If that reenters public malloc,
       force the nested allocation onto the global pools until setup ends. */
    if (nelibc_state_cleanup_initializing) return NULL;
    nelibc_state_cleanup_initializing = 1;
    (void)pthread_once(&nelibc_state_key_once, nelibc_initialize_state_key);
    if (nelibc_state_key_error != 0 ||
        pthread_setspecific(nelibc_state_key, nelibc_states.bytes) != 0) {
      nelibc_state_cleanup_initializing = 0;
      return NULL;
    }
    nelibc_state_cleanup_registered = 1;
    nelibc_state_cleanup_initializing = 0;
  }
  return nelibc_states.bytes[slot];
#else
#ifdef NELIBC_STANDALONE
  #define NELIBC_TLS_MODEL __attribute__((tls_model("local-exec")))
#else
  #define NELIBC_TLS_MODEL
  #endif
  static _Thread_local union {
    max_align_t alignment;
    unsigned char bytes[18][256];
  } states NELIBC_TLS_MODEL;
#ifndef NELIBC_STANDALONE
  if (slot >= 7 && !nelibc_state_cleanup_registered) {
    /* See the TinyCC path above: key setup may recursively allocate. */
    if (nelibc_state_cleanup_initializing) return NULL;
    nelibc_state_cleanup_initializing = 1;
    (void)pthread_once(&nelibc_state_cleanup_once,
                       nelibc_initialize_state_cleanup_key);
    if (nelibc_state_cleanup_error != 0 ||
        pthread_setspecific(nelibc_state_cleanup_key, states.bytes) != 0) {
      nelibc_state_cleanup_initializing = 0;
      return NULL;
    }
    nelibc_state_cleanup_registered = 1;
    nelibc_state_cleanup_initializing = 0;
  }
#endif
  return states.bytes[slot];
#endif
}

/* Standalone uses a dedicated local-exec TLS pointer. Hybrid builds get this
 * provider from string_locale_lock.c so string-only consumers can link it. */
#ifdef NELIBC_STANDALONE
char **nelibc_strtok_state(void) {
  #define NELIBC_STRTOK_TLS_MODEL __attribute__((tls_model("local-exec")))
  static _Thread_local char *state NELIBC_STRTOK_TLS_MODEL;
  return &state;
}
#endif

#ifndef NELIBC_STANDALONE
static pthread_mutex_t nelibc_system_mutex = PTHREAD_MUTEX_INITIALIZER;

void nelibc_system_lock(void) {
  (void)pthread_mutex_lock(&nelibc_system_mutex);
}

void nelibc_system_unlock(void) {
  (void)pthread_mutex_unlock(&nelibc_system_mutex);
}

int nelibc_system_waitpid(pid_t pid, int *status, int options,
                          void (*cleanup)(void *), void *context) {
  int result;
  pthread_cleanup_push(cleanup, context);
  result = waitpid(pid, status, options);
  pthread_cleanup_pop(0);
  return result;
}

typedef size_t (*nelibc_host_mbrtowc_fn)(wchar_t *, const char *, size_t, mbstate_t *);
typedef size_t (*nelibc_host_wcrtomb_fn)(char *, wchar_t, mbstate_t *);
typedef int (*nelibc_host_mbsinit_fn)(const mbstate_t *);
static nelibc_host_mbrtowc_fn nelibc_host_mbrtowc;
static nelibc_host_wcrtomb_fn nelibc_host_wcrtomb;
static nelibc_host_mbsinit_fn nelibc_host_mbsinit;
static pthread_once_t nelibc_host_multibyte_once = PTHREAD_ONCE_INIT;

static void nelibc_resolve_host_multibyte_once(void) {
  nelibc_host_mbrtowc = (nelibc_host_mbrtowc_fn)dlsym(RTLD_NEXT, "mbrtowc");
  nelibc_host_wcrtomb = (nelibc_host_wcrtomb_fn)dlsym(RTLD_NEXT, "wcrtomb");
  nelibc_host_mbsinit = (nelibc_host_mbsinit_fn)dlsym(RTLD_NEXT, "mbsinit");
}

static void nelibc_resolve_host_multibyte(void) {
  (void)pthread_once(&nelibc_host_multibyte_once,
                     nelibc_resolve_host_multibyte_once);
}
#else
static int nelibc_system_lock_word;
extern long nelibc_raw_syscall6(unsigned long number, unsigned long arg1,
                                unsigned long arg2, unsigned long arg3,
                                unsigned long arg4, unsigned long arg5,
                                unsigned long arg6);

static void nelibc_system_futex_wait(void) {
  (void)nelibc_raw_syscall6(202, (unsigned long)&nelibc_system_lock_word,
                            128, 1, 0, 0, 0);
}

void nelibc_system_lock(void) {
  while (__atomic_exchange_n(&nelibc_system_lock_word, 1, __ATOMIC_ACQUIRE)) {
    while (__atomic_load_n(&nelibc_system_lock_word, __ATOMIC_RELAXED)) {
      nelibc_system_futex_wait();
    }
  }
}

void nelibc_system_unlock(void) {
  __atomic_store_n(&nelibc_system_lock_word, 0, __ATOMIC_RELEASE);
  (void)nelibc_raw_syscall6(202, (unsigned long)&nelibc_system_lock_word,
                            129, 1, 0, 0, 0);
}

extern int waitpid(pid_t pid, int *status, int options);
extern void __nelibc_pthread_cleanup_push(void (*routine)(void *), void *arg);
extern void __nelibc_pthread_cleanup_pop(int execute);
int nelibc_system_waitpid(pid_t pid, int *status, int options,
                          void (*cleanup)(void *), void *context) {
  int result;
  __nelibc_pthread_cleanup_push(cleanup, context);
  result = waitpid(pid, status, options);
  __nelibc_pthread_cleanup_pop(0);
  return result;
}
#endif

char **nelibc_environment_vector(void) {
  return environ;
}

void nelibc_environment_set_vector(char **vector) {
  environ = vector;
}

size_t nelibc_environment_pointer_size(void) {
  return sizeof(char *);
}

/* Keep mbstate_t layout in the C adapter; Nelua treats it as opaque storage. */
_Static_assert(sizeof(mbstate_t) <= 128, "mbstate_t exceeds Nelibc storage");
_Static_assert(sizeof(mbstate_t) >= sizeof(uint32_t), "mbstate_t is too small for Nelibc UTF-8 state");

void nelibc_multibyte_state_reset(void *state) {
#ifdef NELIBC_STANDALONE
  (void)state;
#else
  memset(state, 0, sizeof(mbstate_t));
#endif
}

void nelibc_multibyte_state_copy(void *destination, const void *source) {
#ifdef NELIBC_STANDALONE
  memcpy(destination, source, sizeof(uint32_t));
#else
  memcpy(destination, source, sizeof(mbstate_t));
#endif
}

int nelibc_multibyte_state_initial(const void *state) {
  if (state == NULL) return 1;
#ifndef NELIBC_STANDALONE
  if (nelibc_multibyte_mode() == 2) {
    nelibc_resolve_host_multibyte();
    if (nelibc_host_mbsinit != NULL)
      return nelibc_host_mbsinit((const mbstate_t *)state);
  } else {
    uint32_t packed;
    memcpy(&packed, state, sizeof(packed));
    return packed == 0;
  }
#else
  uint32_t packed;
  memcpy(&packed, state, sizeof(packed));
  return packed == 0;
#endif
  const unsigned char *bytes = (const unsigned char *)state;
  for (size_t index = 0; index < sizeof(mbstate_t); ++index)
    if (bytes[index] != 0) return 0;
  return 1;
}

size_t nelibc_multibyte_decode(void *state, int *wide, const char *text, size_t size) {
#ifdef NELIBC_STANDALONE
  (void)state;
  if (text == NULL) return 0;
  if (size == 0) return (size_t)-2;
  unsigned char byte = (unsigned char)text[0];
  if (!nelibc_standalone_ctype_utf8 || byte < 0x80) {
    if (byte > 0x7f) return (size_t)-1;
    if (wide != NULL) *wide = (int)byte;
    return byte == 0 ? 0 : 1;
  }

  size_t count;
  unsigned int value;
  if (byte >= 0xc2 && byte <= 0xdf) {
    count = 2;
    value = byte & 0x1f;
  } else if (byte >= 0xe0 && byte <= 0xef) {
    count = 3;
    value = byte & 0x0f;
  } else if (byte >= 0xf0 && byte <= 0xf4) {
    count = 4;
    value = byte & 0x07;
  } else {
    return (size_t)-1;
  }
  if (size < count) return (size_t)-2;
  for (size_t index = 1; index < count; index++) {
    unsigned char continuation = (unsigned char)text[index];
    if (continuation < 0x80 || continuation > 0xbf) return (size_t)-1;
    if (index == 1 && ((byte == 0xe0 && continuation < 0xa0) ||
                       (byte == 0xed && continuation >= 0xa0) ||
                       (byte == 0xf0 && continuation < 0x90) ||
                       (byte == 0xf4 && continuation > 0x8f))) return (size_t)-1;
    value = (value << 6) | (continuation & 0x3f);
  }
  if (wide != NULL) *wide = (int)value;
  return count;
#else
  nelibc_resolve_host_multibyte();
  if (nelibc_host_mbrtowc == NULL) return (size_t)-1;
  return nelibc_host_mbrtowc((wchar_t *)wide, text, size, (mbstate_t *)state);
#endif
}

size_t nelibc_multibyte_encode(void *state, char *output, int wide) {
#ifdef NELIBC_STANDALONE
  (void)state;
  if (wide < 0 || wide > 0x10ffff || (wide >= 0xd800 && wide <= 0xdfff)) return (size_t)-1;
  if (!nelibc_standalone_ctype_utf8 || wide < 0x80) {
    if (wide > 0x7f) return (size_t)-1;
    if (output != NULL) output[0] = (char)wide;
    return 1;
  }
  if (wide < 0x800) {
    if (output != NULL) {
      output[0] = (char)(0xc0 | (wide >> 6));
      output[1] = (char)(0x80 | (wide & 0x3f));
    }
    return 2;
  }
  if (wide < 0x10000) {
    if (output != NULL) {
      output[0] = (char)(0xe0 | (wide >> 12));
      output[1] = (char)(0x80 | ((wide >> 6) & 0x3f));
      output[2] = (char)(0x80 | (wide & 0x3f));
    }
    return 3;
  }
  if (output != NULL) {
    output[0] = (char)(0xf0 | (wide >> 18));
    output[1] = (char)(0x80 | ((wide >> 12) & 0x3f));
    output[2] = (char)(0x80 | ((wide >> 6) & 0x3f));
    output[3] = (char)(0x80 | (wide & 0x3f));
  }
  return 4;
#else
  nelibc_resolve_host_multibyte();
  if (nelibc_host_wcrtomb == NULL) return (size_t)-1;
  return nelibc_host_wcrtomb(output, (wchar_t)wide, (mbstate_t *)state);
#endif
}
