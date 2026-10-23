#define _GNU_SOURCE
#include <sys/types.h>
#include <unistd.h>

#if defined(NELIBC_STATIC_MUSL)
#include <errno.h>

/*
 * The static musl profile reuses musl's hidden synchronization primitives.
 * This keeps its NPTL-equivalent process-wide credential behavior without
 * resolving public setters back to this executable's interposed symbols.
 */
extern int __setxid(int number, int first, int second, int third);
extern void __synccall(void (*callback)(void *), void *context);

static long linux_syscall2(long number, long first, long second) {
  long result;
  __asm__ volatile("syscall"
                   : "=a"(result)
                   : "a"(number), "D"(first), "S"(second)
                   : "rcx", "r11", "memory");
  return result;
}

struct setgroups_context {
  size_t count;
  const gid_t *groups;
  long result;
};

static void setgroups_all_threads(void *opaque) {
  struct setgroups_context *context = opaque;
  if (context->result < 0) return;
  long result = linux_syscall2(116, (long)context->count, (long)context->groups);
  if (result < 0 && result >= -4095 && context->result == 0) {
    long pid = linux_syscall2(39, 0, 0);
    linux_syscall2(62, pid, 9); /* kill(getpid(), SIGKILL) on partial success */
  }
  context->result = result;
}

#define DEFINE_MUSL_SETXID(name, signature, number, first, second, third) \
  int name signature { \
    return __setxid(number, (int)(first), (int)(second), (int)(third)); \
  }

DEFINE_MUSL_SETXID(setuid, (uid_t uid), 105, uid, 0, 0)
DEFINE_MUSL_SETXID(seteuid, (uid_t uid), 117, -1, uid, -1)
DEFINE_MUSL_SETXID(setreuid, (uid_t ruid, uid_t euid), 113, ruid, euid, 0)
DEFINE_MUSL_SETXID(setresuid, (uid_t ruid, uid_t euid, uid_t suid), 117, ruid, euid, suid)
DEFINE_MUSL_SETXID(setgid, (gid_t gid), 106, gid, 0, 0)
DEFINE_MUSL_SETXID(setegid, (gid_t gid), 119, -1, gid, -1)
DEFINE_MUSL_SETXID(setregid, (gid_t rgid, gid_t egid), 114, rgid, egid, 0)
DEFINE_MUSL_SETXID(setresgid, (gid_t rgid, gid_t egid, gid_t sgid), 119, rgid, egid, sgid)

int setgroups(size_t count, const gid_t groups[]) {
  struct setgroups_context context = {
      .count = count, .groups = groups, .result = 1};
  __synccall(setgroups_all_threads, &context);
  if (context.result > 0) {
    errno = EAGAIN;
    return -1;
  }
  if (context.result < 0 && context.result >= -4095) {
    errno = (int)-context.result;
    return -1;
  }
  return 0;
}

#undef DEFINE_MUSL_SETXID

#elif defined(NELIBC_STANDALONE)
#include <errno.h>
#include <limits.h>

/*
 * Standalone child credential changes are applied locally first, then
 * serialized through the parent thread so Linux credentials stay process-wide.
 */
static int credential_sync_active;
static int credential_sync_users;
static int credential_sync_registration_lock;
static int credential_sync_parent_tid;
static int credential_sync_request_lock;
static int credential_sync_state;
static long credential_sync_number;
static long credential_sync_arguments[3];
static long credential_sync_result;
extern int nelibc_thread_current_context_detached(void);

static int credential_sync_is_detached_thread(void) {
  void *thread_pointer = __builtin_thread_pointer();
  return ((void **)thread_pointer)[3] != 0 ||
         nelibc_thread_current_context_detached();
}

static long standalone_syscall3(long number, long first, long second, long third) {
  long result;
  __asm__ volatile("syscall"
                   : "=a"(result)
                   : "a"(number), "D"(first), "S"(second), "d"(third)
                   : "rcx", "r11", "memory");
  return result;
}

static void credential_futex_wake(int *word, int count) {
  standalone_syscall3(202, (long)word, 1, count); /* FUTEX_WAKE */
}

static void credential_spin_lock(int *lock) {
  for (;;) {
    int expected = 0;
    if (__atomic_compare_exchange_n(lock, &expected, 1, 0, __ATOMIC_ACQUIRE,
                                    __ATOMIC_RELAXED))
      return;
    standalone_syscall3(202, (long)lock, 0, 1); /* FUTEX_WAIT */
  }
}

static void credential_spin_unlock(int *lock) {
  __atomic_store_n(lock, 0, __ATOMIC_RELEASE);
  credential_futex_wake(lock, INT_MAX);
}

int nelibc_credential_sync_begin(void) {
  credential_spin_lock(&credential_sync_registration_lock);
  if (credential_sync_users == 0) {
    credential_sync_parent_tid = (int)standalone_syscall3(186, 0, 0, 0);
    __atomic_store_n(&credential_sync_state, 0, __ATOMIC_RELEASE);
    __atomic_store_n(&credential_sync_active, 1, __ATOMIC_RELEASE);
  }
  ++credential_sync_users;
  credential_spin_unlock(&credential_sync_registration_lock);
  return 0;
}

void nelibc_credential_sync_serve(void) {
  if (__atomic_load_n(&credential_sync_active, __ATOMIC_ACQUIRE) != 1 ||
      __atomic_load_n(&credential_sync_state, __ATOMIC_ACQUIRE) != 1)
    return;
  long result = standalone_syscall3(
      credential_sync_number,
      credential_sync_arguments[0],
      credential_sync_arguments[1],
      credential_sync_arguments[2]);
  if (result < 0 && result >= -4095) {
    long pid = standalone_syscall3(39, 0, 0, 0);
    standalone_syscall3(62, pid, 9, 0); /* kill(getpid(), SIGKILL) after partial success */
    return;
  }
  __atomic_store_n(&credential_sync_result, result, __ATOMIC_RELAXED);
  __atomic_store_n(&credential_sync_state, 2, __ATOMIC_RELEASE);
  credential_futex_wake(&credential_sync_state, INT_MAX);
}

void nelibc_credential_sync_end(void) {
  credential_spin_lock(&credential_sync_registration_lock);
  if (credential_sync_users > 0 && --credential_sync_users == 0) {
    __atomic_store_n(&credential_sync_active, 0, __ATOMIC_RELEASE);
    __atomic_store_n(&credential_sync_state, 0, __ATOMIC_RELEASE);
  }
  credential_spin_unlock(&credential_sync_registration_lock);
}

static long credential_sync_request(long number, long first, long second, long third) {
  credential_spin_lock(&credential_sync_request_lock);
  credential_sync_number = number;
  credential_sync_arguments[0] = first;
  credential_sync_arguments[1] = second;
  credential_sync_arguments[2] = third;
  __atomic_store_n(&credential_sync_state, 1, __ATOMIC_RELEASE);

  while (__atomic_load_n(&credential_sync_state, __ATOMIC_ACQUIRE) != 2) {
    int expected = 1;
    standalone_syscall3(202, (long)&credential_sync_state, 0, expected); /* FUTEX_WAIT */
  }
  long result = __atomic_load_n(&credential_sync_result, __ATOMIC_ACQUIRE);
  __atomic_store_n(&credential_sync_state, 0, __ATOMIC_RELEASE);
  credential_spin_unlock(&credential_sync_request_lock);
  return result;
}

static int standalone_credential_set(long number, long first, long second, long third) {
  if (credential_sync_is_detached_thread()) {
    errno = ENOTSUP;
    return -1;
  }
  long result = standalone_syscall3(number, first, second, third);
  if (result < 0 && result >= -4095) {
    errno = (int)-result;
    return -1;
  }
  if (__atomic_load_n(&credential_sync_active, __ATOMIC_ACQUIRE) == 1 &&
      (int)standalone_syscall3(186, 0, 0, 0) != credential_sync_parent_tid) {
    result = credential_sync_request(number, first, second, third);
    if (result < 0 && result >= -4095) {
      errno = (int)-result;
      return -1;
    }
  }
  return 0;
}

#define DEFINE_STANDALONE_SETTER(name, signature, number, first, second, third) \
  int name signature { \
    return standalone_credential_set(number, (long)(first), (long)(second), (long)(third)); \
  }

DEFINE_STANDALONE_SETTER(setuid, (uid_t uid), 105, uid, 0, 0)
DEFINE_STANDALONE_SETTER(seteuid, (uid_t uid), 117, (uid_t)-1, uid, (uid_t)-1)
DEFINE_STANDALONE_SETTER(setreuid, (uid_t ruid, uid_t euid), 113, ruid, euid, 0)
DEFINE_STANDALONE_SETTER(setresuid, (uid_t ruid, uid_t euid, uid_t suid), 117, ruid, euid, suid)
DEFINE_STANDALONE_SETTER(setgid, (gid_t gid), 106, gid, 0, 0)
DEFINE_STANDALONE_SETTER(setegid, (gid_t gid), 119, (gid_t)-1, gid, (gid_t)-1)
DEFINE_STANDALONE_SETTER(setregid, (gid_t rgid, gid_t egid), 114, rgid, egid, 0)
DEFINE_STANDALONE_SETTER(setresgid, (gid_t rgid, gid_t egid, gid_t sgid), 119, rgid, egid, sgid)
DEFINE_STANDALONE_SETTER(setgroups, (size_t count, const gid_t groups[]), 116, count, groups, 0)

#undef DEFINE_STANDALONE_SETTER

#else

#ifndef NELIBC_STANDALONE
#include <dlfcn.h>

/* Dynamic host setters coordinate per-thread kernel credentials process-wide. */
extern void *dlsym(void *handle, const char *name) __attribute__((weak));

static void *next_libc_symbol(const char *name) {
  if (!dlsym) return 0;
  return dlsym(RTLD_NEXT, name);
}
#endif

#define CREDENTIAL_LOOKUP(name, type) ((type)0)
#ifndef NELIBC_STANDALONE
#undef CREDENTIAL_LOOKUP
#define CREDENTIAL_LOOKUP(name, type) ((type)next_libc_symbol(name))
#endif

#define DEFINE_CREDENTIAL_WRAPPER(name, type, signature, arguments) \
  int name signature { \
    type next = CREDENTIAL_LOOKUP(#name, type); \
    if (next) return next arguments; \
    return nelibc_raw_##name arguments; \
  }

typedef int (*setuid_fn)(uid_t);
extern int nelibc_raw_setuid(uid_t);
DEFINE_CREDENTIAL_WRAPPER(setuid, setuid_fn, (uid_t uid), (uid))

typedef int (*seteuid_fn)(uid_t);
extern int nelibc_raw_seteuid(uid_t);
DEFINE_CREDENTIAL_WRAPPER(seteuid, seteuid_fn, (uid_t uid), (uid))

typedef int (*setreuid_fn)(uid_t, uid_t);
extern int nelibc_raw_setreuid(uid_t, uid_t);
DEFINE_CREDENTIAL_WRAPPER(setreuid, setreuid_fn, (uid_t ruid, uid_t euid), (ruid, euid))

typedef int (*setresuid_fn)(uid_t, uid_t, uid_t);
extern int nelibc_raw_setresuid(uid_t, uid_t, uid_t);
DEFINE_CREDENTIAL_WRAPPER(setresuid, setresuid_fn,
                          (uid_t ruid, uid_t euid, uid_t suid), (ruid, euid, suid))

typedef int (*setgid_fn)(gid_t);
extern int nelibc_raw_setgid(gid_t);
DEFINE_CREDENTIAL_WRAPPER(setgid, setgid_fn, (gid_t gid), (gid))

typedef int (*setegid_fn)(gid_t);
extern int nelibc_raw_setegid(gid_t);
DEFINE_CREDENTIAL_WRAPPER(setegid, setegid_fn, (gid_t gid), (gid))

typedef int (*setregid_fn)(gid_t, gid_t);
extern int nelibc_raw_setregid(gid_t, gid_t);
DEFINE_CREDENTIAL_WRAPPER(setregid, setregid_fn, (gid_t rgid, gid_t egid), (rgid, egid))

typedef int (*setresgid_fn)(gid_t, gid_t, gid_t);
extern int nelibc_raw_setresgid(gid_t, gid_t, gid_t);
DEFINE_CREDENTIAL_WRAPPER(setresgid, setresgid_fn,
                          (gid_t rgid, gid_t egid, gid_t sgid), (rgid, egid, sgid))

typedef int (*setgroups_fn)(size_t, const gid_t *);
extern int nelibc_raw_setgroups(size_t, const gid_t *);
DEFINE_CREDENTIAL_WRAPPER(setgroups, setgroups_fn,
                          (size_t size, const gid_t *groups), (size, groups))

#undef DEFINE_CREDENTIAL_WRAPPER
#undef CREDENTIAL_LOOKUP
#endif
