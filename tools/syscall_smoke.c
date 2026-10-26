#define _GNU_SOURCE
#include "nelibc_alloc.h"
#include "nelibc_thread.h"
#include <pthread.h>
#include "nelibc_stdlib.h"
#include "syscall_interpose.h"
#include "string.h"
#include "nelibc_stdio.h"
#include <errno.h>
#include <ctype.h>
#include <fnmatch.h>
#include <stddef.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <utime.h>
#include <sys/mman.h>
#include <sys/random.h>
#include <spawn.h>
#include <sys/wait.h>
#include <sys/utsname.h>
#include <sys/un.h>
#include <stdint.h>
#ifdef NELIBC_STANDALONE
_Static_assert(offsetof(struct stat, st_size) == 48 &&
               offsetof(struct stat, st_atim) == 72 &&
               sizeof(struct stat) == 144,
               "standalone Linux x86-64 stat ABI");
_Static_assert(offsetof(struct statx, stx_mnt_id) == 144 &&
               sizeof(struct statx) == 256,
               "standalone Linux statx UAPI");
_Static_assert(offsetof(nelibc_thread, stack) == 24 &&
               offsetof(nelibc_thread, tid) == 48 &&
               offsetof(nelibc_thread, stack_size) == 56 &&
               offsetof(nelibc_thread, cleanup_stack) == 64 &&
               offsetof(nelibc_thread, cleanup_stack_size) == 72 &&
               offsetof(nelibc_thread, tid_word) == 80 &&
               sizeof(nelibc_thread) == 88,
               "standalone thread handle ABI");
_Static_assert(sizeof(pthread_attr_t) == 56,
               "standalone pthread attribute ABI");
_Static_assert(sizeof(pthread_once_t) == 4,
               "standalone pthread once ABI");
_Static_assert(sizeof(pthread_mutexattr_t) == 32 &&
               sizeof(pthread_mutex_t) == 64 &&
               offsetof(pthread_mutex_t, __robust_next) == 40 &&
               offsetof(pthread_mutex_t, __state) == 8 &&
               offsetof(pthread_mutex_t, __robust_next) == 40,
               "standalone pthread robust mutex ABI");
_Static_assert(sizeof(pthread_condattr_t) == 16 &&
               offsetof(pthread_condattr_t, __clock) == 12 &&
               sizeof(pthread_cond_t) == 24 &&
               offsetof(pthread_cond_t, __clock) == 20,
               "standalone pthread condition ABI");
_Static_assert(offsetof(struct utsname, machine) == 260 &&
               sizeof(struct utsname) == 390,
               "standalone Linux utsname ABI");
_Static_assert(offsetof(struct sockaddr_un, sun_path) == 2 &&
               sizeof(struct sockaddr_un) == 110,
               "standalone Linux sockaddr_un ABI");
_Static_assert(GRND_NONBLOCK == 1 && GRND_RANDOM == 2 && GRND_INSECURE == 4,
               "standalone Linux getrandom flags");
#endif
#include <sys/ioctl.h>
#include <sys/eventfd.h>
#include <sys/epoll.h>
_Static_assert(sizeof(struct epoll_event) == 12, "Linux x86_64 epoll_event size");
_Static_assert(offsetof(struct epoll_event, data) == 4, "Linux x86_64 epoll_event data offset");
#include <sys/pidfd.h>
#include <dirent.h>
#include <signal.h>
#ifdef NELIBC_TESTING
extern int __nelibc_tz_file_signature(const char *path, char signature[56]);
extern int __nelibc_set_localtime_path(const char *path);
#endif
#include <stdio.h>
#include <wchar.h>
#include <locale.h>
#include <stdarg.h>
#include <stdlib.h>
#include <math.h>
#ifndef NELIBC_STANDALONE
#include <sys/syscall.h>
static void *allocator_thread_worker(void *opaque) {
  size_t seed = (size_t)(uintptr_t)opaque;
  for (size_t i = 0; i < 512; ++i) {
    size_t size = 1 + ((i * 37 + seed * 101) % 5000);
    unsigned char *block = nelibc_malloc(size);
    if (block == NULL) return (void *)(uintptr_t)1;
    block[0] = (unsigned char)(i ^ seed);
    block[size - 1] = (unsigned char)(size ^ seed);
    if ((i & 1) != 0) {
      size_t resized_size = size + 17;
      unsigned char *resized = nelibc_realloc(block, resized_size);
      if (resized == NULL || resized[0] != (unsigned char)(i ^ seed) ||
          resized[size - 1] != (unsigned char)(size ^ seed))
        return (void *)(uintptr_t)2;
      block = resized;
    }
    nelibc_free(block);
  }
  return NULL;
}

static int allocator_thread_smoke(void) {
  pthread_t threads[4];
  size_t created = 0;
  while (created < sizeof threads / sizeof threads[0]) {
    if (pthread_create(&threads[created], NULL, allocator_thread_worker,
                       (void *)(uintptr_t)(created + 1)) != 0) {
      for (size_t i = 0; i < created; ++i) pthread_join(threads[i], NULL);
      return 1;
    }
    ++created;
  }
  for (size_t i = 0; i < created; ++i) {
    void *result = NULL;
    if (pthread_join(threads[i], &result) != 0 || result != NULL) return 2;
  }
  return 0;
}
#endif
#include <unistd.h>
static int compare_smoke_ints(const void *left, const void *right) {
  int a = *(const int *)left;
  int b = *(const int *)right;
  return (a > b) - (a < b);
}

static int select_non_dot_entries(const struct dirent *entry) {
  return entry->d_name[0] != '.';
}

#ifdef NELIBC_STANDALONE
static int call_vswprintf(wchar_t *output, size_t capacity,
                          const wchar_t *format, ...) {
  va_list arguments;
  va_start(arguments, format);
  int result = vswprintf(output, capacity, format, arguments);
  va_end(arguments);
  return result;
}

static int call_vfwprintf(FILE *stream, const wchar_t *format, ...) {
  va_list arguments;
  va_start(arguments, format);
  int result = vfwprintf(stream, format, arguments);
  va_end(arguments);
  return result;
}
#endif

#ifdef NELIBC_STANDALONE
_Static_assert(sizeof(FILE) == 4480, "standalone FILE ABI size");
_Static_assert(offsetof(FILE, pushback_value) == 8, "standalone FILE pushback offset");
_Static_assert(offsetof(FILE, read_position) == 16, "standalone FILE read offset");
_Static_assert(offsetof(FILE, buffer_size) == 56, "standalone FILE capacity offset");
_Static_assert(offsetof(FILE, user_buffer) == 64, "standalone FILE user buffer offset");
_Static_assert(offsetof(FILE, next) == 72, "standalone FILE link offset");
_Static_assert(offsetof(FILE, buffer) == 80, "standalone FILE buffer offset");
_Static_assert(offsetof(FILE, memstream_buffer) == 4176, "standalone memstream buffer offset");
_Static_assert(offsetof(FILE, memstream_size) == 4184, "standalone memstream size offset");
_Static_assert(offsetof(FILE, orientation) == 4192, "standalone FILE orientation offset");
_Static_assert(offsetof(FILE, wide_read_state) == 4200, "standalone FILE read state offset");
_Static_assert(offsetof(FILE, wide_write_state) == 4328, "standalone FILE write state offset");
_Static_assert(offsetof(FILE, has_wide_pushback) == 4456, "standalone FILE wide pushback flag offset");
_Static_assert(offsetof(FILE, wide_pushback_value) == 4460, "standalone FILE wide pushback value offset");

static int smoke_vfprintf(FILE *stream, const char *format, ...) {
  va_list arguments;
  va_start(arguments, format);
  int result = vfprintf(stream, format, arguments);
  va_end(arguments);
  return result;
}

static int smoke_vsnprintf(char *buffer, size_t size, const char *format, ...) {
  va_list arguments;
  va_start(arguments, format);
  int result = vsnprintf(buffer, size, format, arguments);
  va_end(arguments);
  return result;
}

static int smoke_vdprintf(int fd, const char *format, ...) {
  va_list arguments;
  va_start(arguments, format);
  int result = vdprintf(fd, format, arguments);
  va_end(arguments);
  return result;
}

static int smoke_vasprintf(char **buffer, const char *format, ...) {
  va_list arguments;
  va_start(arguments, format);
  int result = vasprintf(buffer, format, arguments);
  va_end(arguments);
  return result;
}
#endif
#ifdef getc
#undef getc
#endif
#ifdef getchar
#undef getchar
#endif

#ifndef AT_EMPTY_PATH
#define AT_EMPTY_PATH 0x1000
#endif

extern long nelibc_raw_syscall3(unsigned long number, unsigned long arg1,
                                unsigned long arg2, unsigned long arg3);
extern long nelibc_raw_syscall6(unsigned long number, unsigned long arg1,
                                unsigned long arg2, unsigned long arg3,
                                unsigned long arg4, unsigned long arg5,
                                unsigned long arg6);
/* glibc may redirect preadv/pwritev declarations to their 64-bit aliases. */
extern ssize_t nelibc_smoke_preadv(int fd, const struct iovec *iov, int iovcnt,
                                   off_t offset) __asm__("preadv");
extern ssize_t nelibc_smoke_pwritev(int fd, const struct iovec *iov, int iovcnt,
                                    off_t offset) __asm__("pwritev");
extern ssize_t nelibc_smoke_preadv2(int fd, const struct iovec *iov, int iovcnt,
                                    off_t offset, int flags) __asm__("preadv2");
extern ssize_t nelibc_smoke_pwritev2(int fd, const struct iovec *iov, int iovcnt,
                                     off_t offset, int flags) __asm__("pwritev2");
extern int nelibc_smoke_pipe2(int pipefd[2], int flags) __asm__("pipe2");

static volatile sig_atomic_t handled_signal;
static void smoke_signal_handler(int signal) { handled_signal = signal; }
static void (*volatile sync_symbol)(void) = sync;

#ifndef NELIBC_STANDALONE
static void *stdio_writer(void *argument) {
  const char *line = (const char *)argument;
  for (int i = 0; i < 100; ++i) {
    if (puts(line) != 0) return (void *)1;
  }
  return 0;
}
#endif

#ifdef NELIBC_STANDALONE
#include <stdatomic.h>
extern int nelibc_tls_fixture_check(void);
extern int nelibc_tls_fixture_recheck(void);
extern int nelibc_tls_alignment_check(void);
extern int nelibc_tls_alignment_recheck(void);
static int tls_thread_entry(void *arg) {
  atomic_int *result = (atomic_int *)arg;
  if (errno != 0) {
    atomic_store_explicit(result, 0, memory_order_release);
    return 1;
  }
  if (setreuid((uid_t)-1, (uid_t)-1) != 0 ||
      setregid((gid_t)-1, (gid_t)-1) != 0 ||
      setresuid((uid_t)-1, (uid_t)-1, (uid_t)-1) != 0 ||
      setresgid((gid_t)-1, (gid_t)-1, (gid_t)-1) != 0) {
    atomic_store_explicit(result, 0, memory_order_release);
    return 1;
  }
  errno = 123;
  int passed = nelibc_tls_fixture_check() && nelibc_tls_fixture_recheck();
  atomic_store_explicit(result, passed, memory_order_release);
  return passed ? 0 : 1;
}

static int credential_drop_thread_entry(void *arg) {
  int *result = arg;
  if (setreuid((uid_t)-1, (uid_t)65534) != 0) {
    *result = errno == EPERM ? 77 : 1;
    return 1;
  }
  *result = geteuid() == (uid_t)65534 ? 0 : 1;
  return *result;
}

static int thread_return_value(void *arg) {
  (void)arg;
  return 73;
}

static int thread_wait_for_gate(void *arg) {
  atomic_int *gate = arg;
  while (!atomic_load_explicit(gate, memory_order_acquire)) {}
  return 73;
}

static void *pthread_return_self(void *arg) {
  (void)arg;
  return (void *)pthread_self();
}
#endif

static void make_seek_path(char path[64]) {
  static const char prefix[] = "/tmp/nelibc-seek-";
  static unsigned int path_sequence;
  char digits[16];
  size_t length = 0;
  size_t digit_count = 0;
  while (prefix[length] != 0) {
    path[length] = prefix[length];
    ++length;
  }
  unsigned int pid = (unsigned int)getpid();
  do {
    digits[digit_count++] = (char)('0' + pid % 10);
    pid /= 10;
  } while (pid != 0);
  while (digit_count != 0) path[length++] = digits[--digit_count];
  path[length++] = '-';
  unsigned int sequence = ++path_sequence;
  digit_count = 0;
  do {
    digits[digit_count++] = (char)('0' + sequence % 10);
    sequence /= 10;
  } while (sequence != 0);
  while (digit_count != 0) path[length++] = digits[--digit_count];
  path[length] = 0;
}

static int wide_scanset_smoke(void) {
  char ascii_range_bytes[] = "a\303\251Z";
  FILE *stream = fmemopen(ascii_range_bytes, sizeof ascii_range_bytes - 1, "r");
  wchar_t output[8] = {0};
  if (stream == NULL || fscanf(stream, "%l[a-z]", output) != 1 ||
      output[0] != L'a' || output[1] != 0 || fgetc(stream) != 0xC3 ||
      fgetc(stream) != 0xA9 || fgetc(stream) != 'Z' || fclose(stream) != 0)
    return 1;

  char unicode_literal_bytes[] = "\303\251Z";
  stream = fmemopen(unicode_literal_bytes, sizeof unicode_literal_bytes - 1, "r");
  memset(output, 0, sizeof output);
  if (stream == NULL || fscanf(stream, "%l[\303\251]", output) != 1 ||
      output[0] != 0xE9 || output[1] != 0 || fgetc(stream) != 'Z' ||
      fclose(stream) != 0)
    return 2;

  char unicode_range_bytes[] = "\303\251\303\252Z";
  stream = fmemopen(unicode_range_bytes, sizeof unicode_range_bytes - 1, "r");
  memset(output, 0, sizeof output);
  if (stream == NULL ||
      fscanf(stream, "%l[\303\251-\303\252]", output) != 1 ||
      output[0] != 0xE9 || output[1] != 0xEA || output[2] != 0 ||
      fgetc(stream) != 'Z' || fclose(stream) != 0)
    return 3;

  char whitespace_bytes[] = "\303\251\343\200\200x";
  stream = fmemopen(whitespace_bytes, sizeof whitespace_bytes - 1, "r");
  memset(output, 0, sizeof output);
  if (stream == NULL) return 4;
  if (fscanf(stream, "%ls", output) != 1 || output[0] != 0xE9 || output[1] != 0) return 5;
  if (ftell(stream) != 2) return 6;
  if (fseek(stream, 0, SEEK_CUR) != 0) return 7;
  if (fgetc(stream) != 0xE3) return 8;
  if (fgetc(stream) != 0x80) return 9;
  if (fgetc(stream) != 0x80) return 10;
  int next = fgetc(stream);
  if (next != 'x') return 11;
  if (fclose(stream) != 0) return 12;
  return 0;
}

#ifdef NELIBC_TESTING
static int copy_path_to_fd(int destination_fd, const char *source_path) {
  int source_fd = openat(AT_FDCWD, source_path, O_RDONLY);
  if (source_fd < 0 || ftruncate(destination_fd, 0) != 0 ||
      lseek(destination_fd, 0, SEEK_SET) != 0) {
    if (source_fd >= 0) close(source_fd);
    return -1;
  }
  char buffer[8192];
  int result = 0;
  for (;;) {
    ssize_t amount = read(source_fd, buffer, sizeof buffer);
    if (amount < 0) { result = -1; break; }
    if (amount == 0) break;
    ssize_t written = 0;
    while (written < amount) {
      ssize_t chunk = write(destination_fd, buffer + written, (size_t)(amount - written));
      if (chunk <= 0) { result = -1; break; }
      written += chunk;
    }
    if (result != 0) break;
  }
  if (close(source_fd) != 0) result = -1;
  return result;
}
#endif

static int fcntl_argument_smoke(void) {
  char path[64];
  make_seek_path(path);
  int fd = openat(AT_FDCWD, path, O_CREAT | O_EXCL | O_RDWR, 0600);
  if (fd < 0) return 400;
  int result = 0;
#ifdef F_GETOWN_EX
  struct f_owner_ex owner = {.type = F_OWNER_PID, .pid = getpid()};
  if (fcntl(fd, F_SETOWN_EX, &owner) != 0) result = 401;
  owner.type = 0;
  owner.pid = 0;
  if (result == 0 &&
      (fcntl(fd, F_GETOWN_EX, &owner) != 0 || owner.type != F_OWNER_PID ||
       owner.pid != getpid())) result = 402;
#endif
#ifdef F_OFD_SETLK
  static struct flock ofd_lock = {
      .l_type = F_WRLCK, .l_whence = SEEK_SET, .l_start = 0, .l_len = 1};
  if (result == 0 && fcntl(fd, F_OFD_SETLK, &ofd_lock) != 0) result = 403;
  int peer = result == 0 ? openat(AT_FDCWD, path, O_RDWR) : -1;
  if (result == 0 && peer < 0) result = 404;
  if (result == 0) {
    ofd_lock.l_type = F_WRLCK;
    if (fcntl(peer, F_OFD_GETLK, &ofd_lock) != 0 ||
        ofd_lock.l_type != F_WRLCK || ofd_lock.l_pid != -1) result = 405;
  }
  if (peer >= 0) {
    ofd_lock.l_type = F_UNLCK;
    ofd_lock.l_whence = SEEK_SET;
    ofd_lock.l_start = 0;
    ofd_lock.l_len = 1;
    ofd_lock.l_pid = 0;
    if (fcntl(fd, F_OFD_SETLK, &ofd_lock) != 0 && result == 0) result = 406;
    if (close(peer) != 0 && result == 0) result = 407;
  }
#endif
#ifdef F_GET_RW_HINT
  uint64_t hint = 2;
  uint64_t raw_hint = hint;
  long raw_result = nelibc_raw_syscall3(
      72, (unsigned long)fd, F_SET_RW_HINT, (unsigned long)&raw_hint);
  errno = 0;
  int wrapper_result = fcntl(fd, F_SET_RW_HINT, &hint);
  int wrapper_errno = errno;
  if (result == 0 && raw_result < 0 &&
      (wrapper_result != -1 || wrapper_errno != -raw_result)) result = 408;
  if (result == 0 && raw_result >= 0 && wrapper_result != raw_result) result = 409;
  hint = 0;
  raw_hint = 0;
  raw_result = nelibc_raw_syscall3(
      72, (unsigned long)fd, F_GET_RW_HINT, (unsigned long)&raw_hint);
  errno = 0;
  wrapper_result = fcntl(fd, F_GET_RW_HINT, &hint);
  wrapper_errno = errno;
  if (result == 0 && raw_result < 0 &&
      (wrapper_result != -1 || wrapper_errno != -raw_result)) result = 410;
  if (result == 0 && raw_result >= 0 &&
      (wrapper_result != raw_result || hint != raw_hint)) result = 411;
#endif
  if (close(fd) != 0 && result == 0) result = 412;
  if (unlinkat(AT_FDCWD, path, 0) != 0 && result == 0) result = 413;
  if (result != 0) return result;

  fd = creat(path, 0600);
  if (fd < 0) return 414;
  if (write(fd, "x", 1) != 1 || close(fd) != 0) return 415;
  fd = creat(path, 0600);
  if (fd < 0) return 416;
  struct stat created_status;
  if (fstat(fd, &created_status) != 0 || created_status.st_size != 0) result = 417;
  if (close(fd) != 0 && result == 0) result = 418;
  if (unlinkat(AT_FDCWD, path, 0) != 0 && result == 0) result = 419;
  return result;
}

static int positioned_io_smoke(void) {
  char path[64];
  char link_path[64];
  char bad_link_path[64];
  char hard_path[64];
  char hard_alias_path[64];
  char moved_path[64];
  char denied_path[64];
  char received[sizeof "positioned I/O"] = {0};
  char link_target[64];
  struct stat status;
  struct stat fd_status;
  struct stat path_status;
  struct stat link_status;
  static const char payload[] = "positioned I/O";
  make_seek_path(path);
  size_t path_length = 0;
  while (path[path_length] != 0) ++path_length;
  for (size_t i = 0; i < path_length; ++i) link_path[i] = bad_link_path[i] = path[i];
  static const char suffix[] = ".link";
  for (size_t i = 0; i < sizeof suffix; ++i) link_path[path_length + i] = suffix[i];
  static const char bad_link_suffix[] = ".badlink";
  for (size_t i = 0; i < sizeof bad_link_suffix; ++i) bad_link_path[path_length + i] = bad_link_suffix[i];
  static const char hard_suffix[] = ".hard";
  static const char moved_suffix[] = ".moved";
  for (size_t i = 0; i < path_length; ++i) hard_path[i] = hard_alias_path[i] = moved_path[i] = path[i];
  for (size_t i = 0; i < sizeof hard_suffix; ++i) hard_path[path_length + i] = hard_suffix[i];
  static const char hard_alias_suffix[] = ".hard-alias";
  for (size_t i = 0; i < sizeof hard_alias_suffix; ++i) hard_alias_path[path_length + i] = hard_alias_suffix[i];
  for (size_t i = 0; i < sizeof moved_suffix; ++i) moved_path[path_length + i] = moved_suffix[i];
  static const char denied_suffix[] = ".denied";
  for (size_t i = 0; i < path_length; ++i) denied_path[i] = path[i];
  for (size_t i = 0; i < sizeof denied_suffix; ++i) denied_path[path_length + i] = denied_suffix[i];
  int fd = openat(AT_FDCWD, path, O_CREAT | O_EXCL | O_RDWR, 0600);
  if (fd < 0) return 57;

  int result = 0;
  if (faccessat(AT_FDCWD, path, R_OK, 0) != 0) result = 144;
  if (result == 0 && faccessat2(AT_FDCWD, path, R_OK, 0) != 0) result = 420;
  if (result == 0 && access(path, R_OK) != 0) result = 270;
  errno = 0;
  if (result == 0 && (access(path, 8) != -1 || errno != EINVAL)) result = 271;
  if (result == 0 && faccessat(AT_FDCWD, path, R_OK, AT_EACCESS) != 0) result = 145;
  if (result == 0 && faccessat(fd, "", R_OK, AT_EMPTY_PATH) != 0) result = 153;
  errno = 0;
  if (result == 0 && (faccessat(AT_FDCWD, path, 8, 0) != -1 || errno != EINVAL)) result = 146;
  errno = 0;
  if (result == 0 && (faccessat(AT_FDCWD, path, F_OK, 0x40000000) != -1 || errno != EINVAL)) result = 148;
  errno = 0;
  if (result == 0 && (faccessat(-1, "missing-nelibc-relative-path", F_OK, 0) != -1 || errno != EBADF)) result = 149;
  int denied_fd = openat(AT_FDCWD, denied_path, O_CREAT | O_EXCL | O_WRONLY, 0000);
  if (denied_fd < 0 && result == 0) result = 150;
  if (denied_fd >= 0) {
    close(denied_fd);
    if (getuid() != 0) {
      errno = 0;
      if (faccessat(AT_FDCWD, denied_path, R_OK, 0) != -1 || errno != EACCES) result = 151;
    }
    if (unlinkat(AT_FDCWD, denied_path, 0) != 0 && result == 0) result = 152;
  }
  if (pwrite(fd, payload, sizeof payload, 37) != (ssize_t)sizeof payload) result = 58;
  if (result == 0 && fdatasync(fd) != 0) result = 285;
  if (result == 0 && fsync(fd) != 0) result = 286;
  errno = 0;
  if (result == 0 && (pread(fd, received, 1, -1) != -1 || errno != EINVAL)) result = 68;
  errno = 0;
  if (result == 0 && (pwrite(fd, payload, 1, -1) != -1 || errno != EINVAL)) result = 69;
  if (result == 0 && lseek(fd, 0, SEEK_CUR) != 0) result = 59;
  if (result == 0 && pread(fd, received, sizeof received, 37) != (ssize_t)sizeof received) result = 60;
  if (result == 0 && memcmp(received, payload, sizeof payload) != 0) result = 61;
  if (result == 0 && lseek(fd, 0, SEEK_CUR) != 0) result = 62;
  if (result == 0 && lseek(fd, 37, SEEK_SET) != 37) result = 63;
  if (result == 0 && read(fd, received, sizeof received) != (ssize_t)sizeof received) result = 64;
  if (result == 0 && memcmp(received, payload, sizeof payload) != 0) result = 65;
  if (result == 0 && fstatat(AT_FDCWD, path, &status, 0) != 0) result = 68;
  if (result == 0 && (!S_ISREG(status.st_mode) || status.st_size != 37 + (off_t)sizeof payload)) result = 70;
  if (result == 0 && stat(path, &path_status) != 0) result = 77;
  if (result == 0 && (path_status.st_dev != status.st_dev || path_status.st_ino != status.st_ino || path_status.st_mode != status.st_mode || path_status.st_size != status.st_size)) result = 78;
  if (result == 0 && linkat(AT_FDCWD, path, AT_FDCWD, hard_path, 0) != 0) result = 137;
  if (result == 0 && link(path, hard_alias_path) != 0) result = 317;
  if (result == 0 && (stat(hard_alias_path, &path_status) != 0 || path_status.st_ino != status.st_ino)) result = 318;
  if (result == 0 && renameat(AT_FDCWD, hard_path, AT_FDCWD, moved_path) != 0) result = 138;
  if (result == 0 && stat(moved_path, &path_status) != 0) result = 139;
  if (result == 0 && path_status.st_ino != status.st_ino) result = 140;
  int link_created = 0;
  if (result == 0 && symlinkat(path, AT_FDCWD, link_path) != 0) result = 80;
  else if (result == 0) link_created = 1;
  errno = 0;
  if (result == 0 && (symlink(path, link_path) != -1 || errno != EEXIST)) result = 322;
  errno = 0;
  if (result == 0 && (symlink((const char *)(uintptr_t)1, bad_link_path) != -1 || errno != EFAULT)) result = 323;
  if (result == 0 && chown(path, getuid(), getgid()) != 0) result = 339;
  if (result == 0 && lchown(link_path, getuid(), getgid()) != 0) result = 340;
  if (result == 0 && (lstat(link_path, &link_status) != 0 || link_status.st_uid != getuid() ||
                      link_status.st_gid != getgid())) result = 341;
  for (size_t i = 0; i < sizeof link_target; ++i) link_target[i] = '#';
  if (result == 0 && readlinkat(AT_FDCWD, link_path, link_target, sizeof link_target) != (ssize_t)path_length) result = 87;
  if (result == 0 && (memcmp(link_target, path, path_length) != 0 || link_target[path_length] != '#')) result = 88;
  for (size_t i = 0; i < sizeof link_target; ++i) link_target[i] = '#';
  if (result == 0 && readlink(link_path, link_target, sizeof link_target) != (ssize_t)path_length) result = 313;
  if (result == 0 && (memcmp(link_target, path, path_length) != 0 || link_target[path_length] != '#')) result = 314;
  char short_target[3];
  if (result == 0 && readlinkat(AT_FDCWD, link_path, short_target, sizeof short_target) != (ssize_t)sizeof short_target) result = 89;
  if (result == 0 && memcmp(short_target, path, sizeof short_target) != 0) result = 90;
  if (result == 0 && stat(link_path, &link_status) != 0) result = 81;
  if (result == 0 && (!S_ISREG(link_status.st_mode) || link_status.st_ino != status.st_ino)) result = 82;
  char canonical_link[4096];
  char canonical_target[4096];
  if (result == 0 && (realpath(link_path, canonical_link) != canonical_link ||
                      realpath(path, canonical_target) != canonical_target ||
                      strcmp(canonical_link, canonical_target) != 0)) result = 494;
  char relative_link_path[96];
  char relative_target[96];
  if (result == 0) {
    make_seek_path(relative_link_path);
    static const char relative_suffix[] = ".realpath";
    size_t relative_path_length = strlen(relative_link_path);
    for (size_t i = 0; i < sizeof relative_suffix; ++i)
      relative_link_path[relative_path_length + i] = relative_suffix[i];
    relative_target[0] = '.';
    relative_target[1] = '.';
    relative_target[2] = '/';
    memcpy(relative_target + 3, path + 1, path_length);
    relative_target[path_length + 2] = 0;
    if (symlink(relative_target, relative_link_path) != 0 ||
        realpath(relative_link_path, canonical_link) != canonical_link ||
        strcmp(canonical_link, canonical_target) != 0) result = 498;
    if (unlink(relative_link_path) != 0 && result == 0) result = 499;
  }
  if (result == 0) {
    char *allocated_canonical = realpath(link_path, NULL);
    if (allocated_canonical == NULL || strcmp(allocated_canonical, canonical_target) != 0) result = 495;
    free(allocated_canonical);
  }
  if (result == 0 && lstat(link_path, &link_status) != 0) result = 83;
  if (result == 0 && (!S_ISLNK(link_status.st_mode) || link_status.st_size != (off_t)path_length)) result = 84;
  if (result == 0 && fstat(fd, &fd_status) != 0) result = 73;
  if (result == 0 && (fd_status.st_dev != status.st_dev || fd_status.st_ino != status.st_ino || fd_status.st_mode != status.st_mode || fd_status.st_size != status.st_size)) result = 74;
  if (result == 0 && chmod(path, 0400) != 0) result = 324;
  if (result == 0 && (stat(path, &path_status) != 0 || (path_status.st_mode & 0777) != 0400)) result = 325;
  if (result == 0 && fchmod(fd, 0600) != 0) result = 326;
  if (result == 0 && (fstat(fd, &fd_status) != 0 || (fd_status.st_mode & 0777) != 0600)) result = 327;
  if (result == 0 && fchownat(AT_FDCWD, path, getuid(), getgid(), 0) != 0) result = 335;
  if (result == 0 && (fstat(fd, &fd_status) != 0 || fd_status.st_uid != getuid() || fd_status.st_gid != getgid())) result = 336;
  if (result == 0 && fchmodat(AT_FDCWD, path, 0640, 0) != 0) result = 330;
  if (result == 0 && (stat(path, &path_status) != 0 || (path_status.st_mode & 0777) != 0640)) result = 331;
  if (result == 0 && fchmodat(fd, "", 0600, AT_EMPTY_PATH) != 0) result = 332;
  if (result == 0 && (fstat(fd, &fd_status) != 0 || (fd_status.st_mode & 0777) != 0600)) result = 333;
  errno = 0;
  if (result == 0 && (fchmodat(AT_FDCWD, path, 0600, 0x80000000) != -1 || errno != EINVAL)) result = 334;
  errno = 0;
  if (result == 0 && (fchownat(AT_FDCWD, path, getuid(), getgid(), 0x80000000) != -1 || errno != EINVAL)) result = 337;
  errno = 0;
  if (result == 0 && (fchownat(AT_FDCWD, "/nelibc-no-such-owner-path", getuid(), getgid(), 0) != -1 || errno != ENOENT)) result = 338;
  errno = 0;
  if (result == 0 && (chmod("/nelibc-no-such-mode-path", 0600) != -1 || errno != ENOENT)) result = 328;
  errno = 0;
  if (result == 0 && (fchmod(-1, 0600) != -1 || errno != EBADF)) result = 329;
  off_t original_size = status.st_size;
  unsigned char extended_byte = 0xFF;
  if (result == 0 && ftruncate(fd, original_size + 3) != 0) result = 273;
  if (result == 0 && (fstat(fd, &fd_status) != 0 || fd_status.st_size != original_size + 3)) result = 274;
  if (result == 0 && (pread(fd, &extended_byte, 1, original_size) != 1 || extended_byte != 0)) result = 275;
  if (result == 0 && ftruncate(fd, original_size) != 0) result = 276;
  if (result == 0 && (fstat(fd, &fd_status) != 0 || fd_status.st_size != original_size)) result = 277;
  errno = 0;
  if (result == 0 && (ftruncate(fd, -1) != -1 || errno != EINVAL)) result = 278;
  if (result == 0 && truncate(link_path, original_size + 3) != 0) result = 279;
  if (result == 0 && (fstat(fd, &fd_status) != 0 || fd_status.st_size != original_size + 3)) result = 280;
  if (result == 0 && truncate(path, original_size) != 0) result = 281;
  if (result == 0 && (fstat(fd, &fd_status) != 0 || fd_status.st_size != original_size)) result = 282;
  errno = 0;
  if (result == 0 && (truncate(path, -1) != -1 || errno != EINVAL)) result = 283;
  static const char vector_first[] = "vector ";
  static const char vector_second[] = "I/O";
  char vector_read_first[sizeof vector_first] = {0};
  char vector_read_second[sizeof vector_second] = {0};
  struct iovec vector_write[2] = {
    {(void *)vector_first, sizeof vector_first - 1},
    {(void *)vector_second, sizeof vector_second - 1}
  };
  struct iovec vector_read[2] = {
    {vector_read_first, sizeof vector_first - 1},
    {vector_read_second, sizeof vector_second - 1}
  };
  off_t vector_original_offset = lseek(fd, 0, SEEK_CUR);
  off_t vector_high_offset = ((off_t)1 << 32) + 256;
  size_t vector_length = sizeof vector_first + sizeof vector_second - 2;
  if (result == 0 && nelibc_smoke_pwritev(fd, vector_write, 2, 256) != (ssize_t)vector_length) result = 291;
  if (result == 0 && nelibc_smoke_preadv(fd, vector_read, 2, 256) != (ssize_t)vector_length) result = 292;
  if (result == 0 && (memcmp(vector_read_first, vector_first, sizeof vector_first - 1) != 0 ||
                      memcmp(vector_read_second, vector_second, sizeof vector_second - 1) != 0)) result = 293;
  if (result == 0 && lseek(fd, 0, SEEK_CUR) != vector_original_offset) result = 294;
  if (result == 0 && nelibc_smoke_preadv(fd, vector_read, 2, vector_high_offset) != 0) result = 297;
  errno = 0;
  if (result == 0 && (nelibc_smoke_preadv(fd, vector_read, 2, -1) != -1 || errno != EINVAL)) result = 295;
  errno = 0;
  if (result == 0 && (nelibc_smoke_pwritev(fd, vector_write, 2, -1) != -1 || errno != EINVAL)) result = 296;
  if (result == 0 && nelibc_smoke_pwritev2(fd, vector_write, 2, 320, 0) != (ssize_t)vector_length) result = 298;
  if (result == 0 && nelibc_smoke_preadv2(fd, vector_read, 2, 320, 0) != (ssize_t)vector_length) result = 299;
  if (result == 0 && (memcmp(vector_read_first, vector_first, sizeof vector_first - 1) != 0 ||
                      memcmp(vector_read_second, vector_second, sizeof vector_second - 1) != 0)) result = 300;
  if (result == 0 && nelibc_smoke_preadv2(fd, vector_read, 2, vector_high_offset, 0) != 0) result = 301;
  errno = 0;
  if (result == 0 && (nelibc_smoke_preadv2(fd, vector_read, 2, -2, 0) != -1 || errno != EINVAL)) result = 302;
  errno = 0;
  if (result == 0 && (nelibc_smoke_pwritev2(fd, vector_write, 2, -2, 0) != -1 || errno != EINVAL)) result = 303;
  if (result == 0 && nelibc_smoke_pwritev2(fd, vector_write, 2, 0, RWF_APPEND) != (ssize_t)vector_length) result = 304;
  if (result == 0 && nelibc_smoke_preadv2(fd, vector_read, 2, 330, 0) != (ssize_t)vector_length) result = 305;
  if (result == 0 && (memcmp(vector_read_first, vector_first, sizeof vector_first - 1) != 0 ||
                      memcmp(vector_read_second, vector_second, sizeof vector_second - 1) != 0)) result = 306;
  errno = 0;
  if (result == 0 && (fstatat(AT_FDCWD, path, (struct stat *)(uintptr_t)1, 0) != -1 || errno != EFAULT)) result = 71;
  errno = 0;
  if (result == 0 && (fstat(-1, &fd_status) != -1 || errno != EBADF)) result = 75;
  errno = 0;
  if (result == 0 && (fdatasync(-1) != -1 || errno != EBADF)) result = 287;
  errno = 0;
  if (result == 0 && (fsync(-1) != -1 || errno != EBADF)) result = 288;
  errno = 0;
  if (result == 0 && (syncfs(-1) != -1 || errno != EBADF)) result = 289;
  errno = 0;
  if (result == 0 && (fstat(fd, (struct stat *)(uintptr_t)1) != -1 || errno != EFAULT)) result = 76;
  errno = 0;
  if (result == 0 && (fstatat(-1, "missing-nelibc-smoke-path", &status, 0) != -1 || errno != EBADF)) result = 72;
  if (close(fd) != 0 && result == 0) result = 66;
  if (unlinkat(AT_FDCWD, moved_path, 0) != 0 && result == 0) result = 141;
  if (unlink(hard_alias_path) != 0 && result == 0) result = 319;
  if (unlink(path) != 0 && result == 0) result = 67;
  errno = 0;
  if (result == 0 && (faccessat(AT_FDCWD, path, F_OK, 0) != -1 || errno != ENOENT)) result = 147;
  errno = 0;
  if (result == 0 && (access(path, F_OK) != -1 || errno != ENOENT)) result = 272;
  errno = 0;
  if (result == 0 && (truncate(path, 0) != -1 || errno != ENOENT)) result = 284;
  errno = 0;
  if (result == 0 && (faccessat(AT_FDCWD, link_path, F_OK, 0) != -1 || errno != ENOENT)) result = 154;
  if (result == 0 && faccessat(AT_FDCWD, link_path, F_OK, AT_SYMLINK_NOFOLLOW) != 0) result = 155;
  if (link_created && unlinkat(AT_FDCWD, link_path, 0) != 0 && result == 0) result = 85;
  errno = 0;
  if (result == 0 && (stat(path, &path_status) != -1 || errno != ENOENT)) result = 79;
  errno = 0;
  if (result == 0 && (lstat(link_path, &link_status) != -1 || errno != ENOENT)) result = 86;
  errno = 0;
  if (result == 0 && (readlinkat(AT_FDCWD, link_path, link_target, sizeof link_target) != -1 || errno != ENOENT)) result = 91;
  errno = 0;
  if (result == 0 && (readlink(link_path, link_target, sizeof link_target) != -1 || errno != ENOENT)) result = 315;
  return result;
}

int main(int argc, char **argv) {
  if (fnmatch("*.c", "main.c", 0) != 0 ||
      fnmatch("a/*.c", "a/b/main.c", FNM_PATHNAME) != FNM_NOMATCH ||
      fnmatch("a/*.c", "a/b/main.c", 0) != 0 ||
      fnmatch("*", ".hidden", FNM_PERIOD) != FNM_NOMATCH ||
      fnmatch(".*", ".hidden", FNM_PERIOD) != 0 ||
      fnmatch("[a-c]at", "bat", 0) != 0 ||
      fnmatch("[!a-c]at", "zat", 0) != 0 ||
      fnmatch("foo\\*", "foo*", 0) != 0 ||
      fnmatch("foo\\*", "foo\\bar", FNM_NOESCAPE) != 0 ||
      fnmatch("*.C", "main.c", FNM_CASEFOLD) != 0 ||
      fnmatch("*.c", "main.c/child", FNM_LEADING_DIR) != 0 ||
      fnmatch("[[:alnum:]]", "7", 0) != 0 ||
      fnmatch("[[:alpha:]]", "Q", 0) != 0 ||
      fnmatch("[[:blank:]]", "\t", 0) != 0 ||
      fnmatch("[[:cntrl:]]", "\n", 0) != 0 ||
      fnmatch("[[:digit:]]", "8", 0) != 0 ||
      fnmatch("[[:graph:]]", "!", 0) != 0 ||
      fnmatch("[[:lower:]]", "q", 0) != 0 ||
      fnmatch("[[:print:]]", " ", 0) != 0 ||
      fnmatch("[[:punct:]]", "!", 0) != 0 ||
      fnmatch("[[:space:]]", "\n", 0) != 0 ||
      fnmatch("[[:upper:]]", "Q", 0) != 0 ||
      fnmatch("[[:xdigit:]]", "f", 0) != 0 ||
      fnmatch("[[:digit:]]", "q", 0) == 0 ||
      fnmatch("[![:digit:]]", "q", 0) != 0)
    return 339;
  if (argc == 2 && strcmp(argv[1], "--execveat-child") == 0) return 42;
  char executable_path[128];
  size_t executable_path_size = confstr(_CS_PATH, executable_path, sizeof executable_path);
  if (getpagesize() != 4096 || sysconf(_SC_PAGESIZE) != getpagesize() ||
      executable_path_size == 0 || executable_path_size > sizeof executable_path ||
      executable_path[executable_path_size - 1] != '\0') return 552;
  const char *environment_probe = getenv("NELIBC_GETENV_PROBE");
  if (environment_probe == NULL || strcmp(environment_probe, "visible") != 0 ||
      getenv("") != NULL || getenv("NELIBC_GETENV_PROBE=visible") != NULL) return 487;
  char realpath_cwd[4096];
  char realpath_result[4096];
  if (getcwd(realpath_cwd, sizeof realpath_cwd) == NULL ||
      realpath(".", realpath_result) != realpath_result ||
      strcmp(realpath_cwd, realpath_result) != 0) return 496;
  struct statx statx_probe = {0};
  if (statx(AT_FDCWD, ".", AT_SYMLINK_NOFOLLOW, STATX_TYPE, &statx_probe) != 0 ||
      (statx_probe.stx_mask & STATX_TYPE) == 0 || (statx_probe.stx_mode & S_IFMT) != S_IFDIR)
    return 500;
  errno = 0;
  if (realpath("/nelibc-no-such-realpath-entry", realpath_result) != NULL || errno != ENOENT)
    return 497;
  errno = 0;
  if (isatty(-1) != 0 || errno != EBADF) return 488;
  int tty_probe = openat(AT_FDCWD, "/dev/ptmx", O_RDWR | O_NOCTTY | O_CLOEXEC);
  if (tty_probe >= 0) {
    if (isatty(tty_probe) != 1 || close(tty_probe) != 0) return 489;
  }
  int ioctl_pipe[2];
  if (pipe(ioctl_pipe) != 0 || write(ioctl_pipe[1], "x", 1) != 1) return 490;
  int ioctl_available = -1;
  if (ioctl(ioctl_pipe[0], FIONREAD, &ioctl_available) != 0 || ioctl_available != 1) return 491;
  errno = 0;
  if (ioctl(-1, FIONREAD, &ioctl_available) != -1 || errno != EBADF) return 492;
  if (close(ioctl_pipe[0]) != 0 || close(ioctl_pipe[1]) != 0) return 493;
  int socket_pair[2];
  if (socketpair(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0, socket_pair) != 0) return 530;
  static const char socket_pair_payload[] = "socketpair";
  char socket_pair_received[sizeof socket_pair_payload];
  if (send(socket_pair[0], socket_pair_payload, sizeof socket_pair_payload, 0) !=
          (ssize_t)sizeof socket_pair_payload ||
      recv(socket_pair[1], socket_pair_received, sizeof socket_pair_received, MSG_WAITALL) !=
          (ssize_t)sizeof socket_pair_received ||
      memcmp(socket_pair_payload, socket_pair_received, sizeof socket_pair_payload) != 0)
    return 531;
  if (shutdown(socket_pair[0], SHUT_WR) != 0 || recv(socket_pair[1], socket_pair_received,
      sizeof socket_pair_received, 0) != 0 || close(socket_pair[0]) != 0 ||
      close(socket_pair[1]) != 0) return 532;

  int server_socket = socket(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0);
  if (server_socket < 0) return 533;
  struct sockaddr_un unix_address = {.sun_family = AF_UNIX};
  char unix_name[32];
  int unix_name_length = snprintf(unix_name, sizeof unix_name, "nelibc-%ld", (long)getpid());
  if (unix_name_length <= 0 || (size_t)unix_name_length >= sizeof unix_name) return 534;
  memcpy(&unix_address.sun_path[1], unix_name, (size_t)unix_name_length);
  socklen_t unix_address_length = (socklen_t)(offsetof(struct sockaddr_un, sun_path) +
                                               1 + (size_t)unix_name_length);
  if (bind(server_socket, (struct sockaddr *)&unix_address, unix_address_length) != 0 ||
      listen(server_socket, 1) != 0) return 535;
  struct sockaddr_un bound_address;
  socklen_t bound_address_length = sizeof bound_address;
  if (getsockname(server_socket, (struct sockaddr *)&bound_address, &bound_address_length) != 0 ||
      bound_address.sun_family != AF_UNIX) return 536;
  int client_socket = socket(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0);
  if (client_socket < 0 || connect(client_socket, (struct sockaddr *)&unix_address,
                                   unix_address_length) != 0) return 537;
  socklen_t peer_address_length = sizeof bound_address;
  if (getpeername(client_socket, (struct sockaddr *)&bound_address, &peer_address_length) != 0 ||
      bound_address.sun_family != AF_UNIX) return 538;
  int accepted_socket = accept4(server_socket, NULL, NULL, SOCK_CLOEXEC);
  if (accepted_socket < 0) return 539;
  int socket_type = 0;
  socklen_t socket_type_length = sizeof socket_type;
  if (getsockopt(accepted_socket, SOL_SOCKET, SO_TYPE, &socket_type, &socket_type_length) != 0 ||
      socket_type != SOCK_STREAM) return 540;
  static const char socket_message[] = "message";
  struct iovec socket_send_iov = {.iov_base = (void *)socket_message,
                                  .iov_len = sizeof socket_message};
  struct msghdr socket_send_message = {.msg_iov = &socket_send_iov, .msg_iovlen = 1};
  if (sendmsg(client_socket, &socket_send_message, 0) != (ssize_t)sizeof socket_message)
    return 541;
  char socket_message_received[sizeof socket_message];
  struct iovec socket_recv_iov = {.iov_base = socket_message_received,
                                  .iov_len = sizeof socket_message_received};
  struct msghdr socket_recv_message = {.msg_iov = &socket_recv_iov, .msg_iovlen = 1};
  if (recvmsg(accepted_socket, &socket_recv_message, MSG_WAITALL) !=
          (ssize_t)sizeof socket_message_received ||
      memcmp(socket_message, socket_message_received, sizeof socket_message) != 0)
    return 542;
  if (shutdown(client_socket, SHUT_WR) != 0 || close(client_socket) != 0 ||
      close(accepted_socket) != 0 || close(server_socket) != 0) return 543;
  static const char uts_name_probe[] = "nelibc";
  int (*volatile sethostname_probe)(const char *, size_t) = sethostname;
  int (*volatile setdomainname_probe)(const char *, size_t) = setdomainname;
  errno = 0;
  if (sethostname_probe(uts_name_probe, SIZE_MAX) != -1 ||
      (errno != EINVAL && errno != EPERM && errno != ENOSYS)) return 479;
  errno = 0;
  if (setdomainname_probe(uts_name_probe, SIZE_MAX) != -1 ||
      (errno != EINVAL && errno != EPERM && errno != ENOSYS)) return 480;
  struct utsname uts_name;
  if (uname(&uts_name) != 0) return 481;
  char hostname_full[sizeof uts_name.nodename];
  if (gethostname(hostname_full, sizeof hostname_full) != 0 ||
      strcmp(hostname_full, uts_name.nodename) != 0) return 482;
  char hostname_short[3] = {'X', 'X', 'G'};
  errno = 0;
  if (gethostname(hostname_short, sizeof hostname_short - 1) != -1 || errno != ENAMETOOLONG ||
      hostname_short[0] != uts_name.nodename[0] || hostname_short[1] != uts_name.nodename[1] ||
      hostname_short[2] != 'G') return 483;
  char hostname_zero = 'X';
  errno = 0;
  if (gethostname(&hostname_zero, 0) != -1 || errno != ENAMETOOLONG || hostname_zero != 'X') return 486;
  char domainname_full[sizeof uts_name.domainname];
  if (getdomainname(domainname_full, sizeof domainname_full) != 0 ||
      strcmp(domainname_full, uts_name.domainname) != 0) return 484;
  char domainname_short[3] = {'X', 'X', 'G'};
  if (getdomainname(domainname_short, sizeof domainname_short - 1) != 0 ||
      domainname_short[0] != uts_name.domainname[0] || domainname_short[1] != uts_name.domainname[1] ||
      domainname_short[2] != 'G') return 485;
  char domainname_zero = 'X';
  if (getdomainname(&domainname_zero, 0) != 0 || domainname_zero != 'X') return 487;
  errno = 0;
  if (unshare(0x40000000) != -1 ||
      (errno != EINVAL && errno != EPERM && errno != ENOSYS)) return 477;
  errno = 0;
  if (setns(-1, 0) != -1 ||
      (errno != EBADF && errno != EPERM && errno != ENOSYS)) return 478;
  errno = 0;
  if (pivot_root("missing-nelibc-new-root", "missing-nelibc-old-root") != -1 ||
      (errno != ENOENT && errno != EPERM && errno != ENOSYS)) return 476;
  errno = 0;
  if (mount(NULL, NULL, NULL, 0, NULL) != -1 ||
      (errno != EFAULT && errno != EPERM && errno != ENOSYS)) return 473;
  errno = 0;
  if (umount2("missing-nelibc-umount2", MNT_DETACH) != -1 ||
      (errno != ENOENT && errno != EPERM && errno != ENOSYS)) return 474;
  errno = 0;
  if (umount("missing-nelibc-umount") != -1 ||
      (errno != ENOENT && errno != EPERM && errno != ENOSYS)) return 475;
  if (sync_symbol == NULL) return 290;
  char *integer_end = NULL;
  if (strtol("-0x2a!", &integer_end, 0) != -42 || integer_end == NULL || strcmp(integer_end, "!") != 0 ||
      atoi("17x") != 17 || abs(-6) != 6) return 31;
  int sortable[] = {5, -1, 8, 2};
  qsort(sortable, sizeof sortable / sizeof sortable[0], sizeof sortable[0], compare_smoke_ints);
  const int sorted[] = {-1, 2, 5, 8};
  for (size_t i = 0; i < sizeof sorted / sizeof sorted[0]; ++i)
    if (sortable[i] != sorted[i]) return 108;
  int sought = 5;
  if (bsearch(&sought, sortable, sizeof sortable / sizeof sortable[0], sizeof sortable[0],
              compare_smoke_ints) == NULL) return 109;
  char *float_end = NULL;
  if (strtod("-12.5e2!", &float_end) != -1250.0 || float_end == NULL || strcmp(float_end, "!") != 0 ||
      strtod("0x1.8p2", &float_end) != 6.0 || !isinf(strtod("inf", NULL)) ||
      !isnan(strtod("nan", NULL)) ||
      strtod("1.00000000000000011102230246251565404236316680908203125", NULL) != 1.0 ||
      strtod("4.9406564584124654e-324", NULL) == 0.0) return 30;
#ifdef NELIBC_STANDALONE
  char *narrow_end = NULL;
  if (strtof("0x1.8p2!", &narrow_end) != 6.0f || narrow_end == NULL || strcmp(narrow_end, "!") != 0) return 110;
  errno = 0;
  if (!isinf(strtof("1e100", NULL)) || errno != ERANGE) return 111;
  if (strtold("1e400", NULL) < 1.0e300L) return 112;
#endif
  if (!isalnum('A') || !isalnum('7') || isalnum('!') || !isspace('\n') ||
      isspace('x') || tolower('Q') != 'q' || toupper('q') != 'Q' ||
      !isxdigit('F') || isxdigit('G') || !isascii(127) || isascii(128)) return 32;
  char *duplicate = strdup("standalone and hybrid allocation");
  if (duplicate == 0 || strcmp(duplicate, "standalone and hybrid allocation") != 0) return 33;
  free(duplicate);
  char *bounded_duplicate = strndup("bounded", 3);
  if (bounded_duplicate == 0 || strcmp(bounded_duplicate, "bou") != 0) return 34;
  free(bounded_duplicate);
  char end_copy[8];
  if (stpcpy(end_copy, "end") != end_copy + 3 || strcmp(end_copy, "end") != 0) return 35;
  char bounded_copy[4];
  if (strlcpy(bounded_copy, "abcdef", sizeof bounded_copy) != 6 || strcmp(bounded_copy, "abc") != 0) return 39;
  char bounded_cat[5] = "ab";
  if (strlcat(bounded_cat, "cdef", sizeof bounded_cat) != 6 || strcmp(bounded_cat, "abcd") != 0) return 40;
  char padded_end[5];
  if (stpncpy(padded_end, "xy", sizeof padded_end) != padded_end + 2 || padded_end[2] != 0 || padded_end[4] != 0) return 36;
  unsigned char move_bytes[24];
  for (size_t i = 0; i < sizeof move_bytes; ++i) move_bytes[i] = (unsigned char)i;
  void *move_return = memmove(move_bytes + 3, move_bytes, 17);
  if (move_return != move_bytes + 3) return 41;
  for (size_t i = 0; i < 17; ++i) if (move_bytes[i + 3] != (unsigned char)i) return 42;
  unsigned char stop_source[17];
  unsigned char stop_dest[18];
  for (size_t i = 0; i < sizeof stop_source; ++i) stop_source[i] = 0x11;
  for (size_t i = 0; i < sizeof stop_dest; ++i) stop_dest[i] = 0xA5;
  stop_source[8] = 0xFE;
  if (memccpy(stop_dest, stop_source, 0xFE, sizeof stop_source) != stop_dest + 9 || stop_dest[9] != 0xA5) return 43;
  char field_text[] = "a,,b";
  char *field_state = field_text;
  if (strcmp(strsep(&field_state, ","), "a") != 0) return 37;
  if (strsep(&field_state, ",")[0] != 0 || strcmp(strsep(&field_state, ","), "b") != 0 || field_state != 0) return 38;
#ifdef NELIBC_STANDALONE
  if (!nelibc_tls_fixture_check() || !nelibc_tls_fixture_recheck()) return 27;
  if (!nelibc_tls_alignment_check() || !nelibc_tls_alignment_recheck()) return 28;
  for (int i = 0; i < 3; ++i) {
    atomic_int child_tls_ok = 0;
    errno = 91;
    if (nelibc_thread_run_once(tls_thread_entry, &child_tls_ok) != 0) return 29;
    if (!atomic_load_explicit(&child_tls_ok, memory_order_acquire) || errno != 91) return 30;
  }
  nelibc_thread thread = {0};
  atomic_int thread_gate = 0;
  if (nelibc_thread_create(&thread, thread_wait_for_gate, &thread_gate) != 0)
    return 32;
  nelibc_thread second_thread = {0};
  if (nelibc_thread_create(&second_thread, thread_wait_for_gate, &thread_gate) != 0)
    return 36;
  atomic_store_explicit(&thread_gate, 1, memory_order_release);
  int thread_result = -1;
  if (nelibc_thread_join(&thread, &thread_result) != 0 || thread_result != 73 || thread.state != 2 ||
      thread.stack != NULL || thread.tls != NULL)
    return 33;
  if (nelibc_thread_join(&second_thread, &thread_result) != 0 ||
      thread_result != 73 || second_thread.state != 2)
    return 37;
  if (nelibc_thread_join(&thread, &thread_result) != -1 || errno != EINVAL) return 34;
  if (nelibc_thread_run_once(thread_return_value, NULL) != 73) return 35;
  pthread_attr_t pthread_attr;
  if (pthread_attr_init(&pthread_attr) != 0) return 39;
  if (pthread_attr_setstacksize(&pthread_attr, 65536) != 0) return 40;
  size_t pthread_stacksize = 0;
  if (pthread_attr_getstacksize(&pthread_attr, &pthread_stacksize) != 0 ||
      pthread_stacksize != 65536)
    return 41;
  if (pthread_attr_setdetachstate(&pthread_attr, PTHREAD_CREATE_DETACHED) != 0)
    return 42;
  if (pthread_attr_setdetachstate(&pthread_attr, PTHREAD_CREATE_JOINABLE) != 0)
    return 46;
  pthread_t posix_thread;
  if (pthread_create(&posix_thread, &pthread_attr, pthread_return_self, NULL) != 0)
    return 43;
  void *posix_result = NULL;
  if (pthread_join(posix_thread, &posix_result) != 0 ||
      !pthread_equal(posix_thread, (pthread_t)posix_result) ||
      pthread_equal(posix_thread, pthread_self()))
    return 44;
  if (pthread_attr_destroy(&pthread_attr) != 0) return 45;
  if (getuid() == 0 && geteuid() == 0) {
    pid_t credential_child = fork();
    if (credential_child < 0) return 550;
    if (credential_child == 0) {
      int credential_result_a = 1;
      int credential_result_b = 1;
      nelibc_thread credential_thread_a = {0};
      nelibc_thread credential_thread_b = {0};
      if (nelibc_thread_create(&credential_thread_a,
                               credential_drop_thread_entry,
                               &credential_result_a) != 0 ||
          nelibc_thread_create(&credential_thread_b,
                               credential_drop_thread_entry,
                               &credential_result_b) != 0)
        _exit(70);
      int callback_result_b = -1;
      int callback_result_a = -1;
      if (nelibc_thread_join(&credential_thread_b, &callback_result_b) != 0 ||
          nelibc_thread_join(&credential_thread_a, &callback_result_a) != 0)
        _exit(72);
      if (credential_result_a == 77 || credential_result_b == 77 ||
          callback_result_a == 77 || callback_result_b == 77)
        _exit(77);
      _exit(credential_result_a == 0 && credential_result_b == 0 &&
                    callback_result_a == 0 && callback_result_b == 0 &&
                    geteuid() == (uid_t)65534
                ? 0
                : 71);
    }
    int credential_status = 0;
    if (waitpid(credential_child, &credential_status, 0) != credential_child ||
        !WIFEXITED(credential_status) ||
        (WEXITSTATUS(credential_status) != 0 && WEXITSTATUS(credential_status) != 77))
      return 551;
  }
  errno = 0;
  if (nelibc_thread_run_once(0, 0) != -1 || errno != EINVAL) return 31;
#endif
  if (getpid() <= 0) return 25;
#ifndef NELIBC_STANDALONE
  if (getpid() != (pid_t)syscall(SYS_getpid)) return 26;
#endif
  if (getuid() != (uid_t)nelibc_raw_syscall3(102, 0, 0, 0)) return 103;
  if (geteuid() != (uid_t)nelibc_raw_syscall3(107, 0, 0, 0)) return 104;
  if (getgid() != (gid_t)nelibc_raw_syscall3(104, 0, 0, 0)) return 105;
  if (getegid() != (gid_t)nelibc_raw_syscall3(108, 0, 0, 0)) return 106;
  uid_t real_uid, effective_uid, saved_uid;
  gid_t real_gid, effective_gid, saved_gid;
  if (getresuid(&real_uid, &effective_uid, &saved_uid) != 0 ||
      real_uid != getuid() || effective_uid != geteuid()) return 351;
  if (getresgid(&real_gid, &effective_gid, &saved_gid) != 0 ||
      real_gid != getgid() || effective_gid != getegid()) return 352;
  int supplementary_count = getgroups(0, NULL);
  if (supplementary_count < 0) return 357;
  size_t supplementary_capacity = supplementary_count > 0 ? (size_t)supplementary_count : 1;
  gid_t *supplementary_groups = malloc(supplementary_capacity * sizeof *supplementary_groups);
  if (supplementary_groups == NULL) return 358;
  int supplementary_result = getgroups(supplementary_count, supplementary_groups);
  free(supplementary_groups);
  if (supplementary_result != supplementary_count) return 359;
  errno = 0;
  volatile int invalid_group_count = -1;
  if (getgroups(invalid_group_count, NULL) != -1 || errno != EINVAL) return 360;
  struct rlimit nofile_limit;
  if (getrlimit(RLIMIT_NOFILE, &nofile_limit) != 0) return 361;
  if (setrlimit(RLIMIT_NOFILE, &nofile_limit) != 0) return 362;
  errno = 0;
  if (getrlimit(-1, &nofile_limit) != -1 || errno != EINVAL) return 363;
  errno = 0;
  if (setrlimit(-1, &nofile_limit) != -1 || errno != EINVAL) return 364;
  errno = 0;
  int current_priority = getpriority(PRIO_PROCESS, 0);
  if (current_priority < -20 || current_priority > 19) return 373;
  if (setpriority(PRIO_PROCESS, 0, current_priority) != 0) return 374;
  errno = 0;
  if (getpriority(12345, 0) != -1 || errno != EINVAL) return 375;
  errno = 0;
  if (setpriority(12345, 0, current_priority) != -1 || errno != EINVAL) return 376;
  if (sched_yield() != 0) return 377;
  cpu_set_t macro_affinity;
  CPU_ZERO(&macro_affinity);
  CPU_SET(17, &macro_affinity);
  if (!CPU_ISSET(17, &macro_affinity)) return 381;
  CPU_CLR(17, &macro_affinity);
  if (CPU_ISSET(17, &macro_affinity)) return 382;
  cpu_set_t current_affinity;
  if (sched_getaffinity(0, sizeof current_affinity, &current_affinity) != 0) return 378;
  int affinity_nonempty = 0;
  const unsigned char *affinity_bytes = (const unsigned char *)&current_affinity;
  for (size_t i = 0; i < sizeof current_affinity; ++i) affinity_nonempty |= affinity_bytes[i] != 0;
  if (!affinity_nonempty || sched_setaffinity(0, sizeof current_affinity, &current_affinity) != 0) return 379;
  errno = 0;
  if (sched_getaffinity(0, 0, &current_affinity) != -1 || errno != EINVAL) return 380;
  int wait_status = 0;
  errno = 0;
  if (waitpid(-1, &wait_status, WNOHANG) != -1 || errno != ECHILD) return 382;
  errno = 0;
  if (wait4(-1, &wait_status, 0x10000000, NULL) != -1 || errno != EINVAL) return 383;
  errno = 0;
  if (wait(NULL) != -1 || errno != ECHILD) return 384;
  char *missing_argv[] = {(char *)"nelibc-missing-executable", NULL};
  char *empty_environment[] = {NULL};
  errno = 0;
  if (execve("/nelibc-no-such-executable", missing_argv, empty_environment) != -1 || errno != ENOENT) return 385;
  errno = 0;
  if (execveat(AT_FDCWD, "/nelibc-no-such-executable", missing_argv,
               empty_environment, 0) != -1 || errno != ENOENT) return 392;
  int executable_fd = openat(AT_FDCWD, "/proc/self/exe", O_PATH | O_CLOEXEC, 0);
  if (executable_fd < 0) return 393;
  pid_t child_pid = fork();
  if (child_pid < 0) return 386;
  if (child_pid == 0) {
    char *execveat_argv[] = {(char *)"nelibc-execveat-child", (char *)"--execveat-child", NULL};
    if (execveat(executable_fd, "", execveat_argv, empty_environment, AT_EMPTY_PATH) != 0)
      _exit(43);
    _exit(44);
  }
  wait_status = 0;
  int execveat_wait_result = waitpid(child_pid, &wait_status, 0);
  int executable_close_result = close(executable_fd);
  if (execveat_wait_result != child_pid || !WIFEXITED(wait_status) ||
      WEXITSTATUS(wait_status) != 42 || executable_close_result != 0) return 394;
  char *spawn_argv[] = {(char *)"true", NULL};
  pid_t spawned_pid = -1;
  errno = 42;
  int spawn_result = posix_spawn(&spawned_pid, "/bin/true", NULL, NULL,
                                 spawn_argv, empty_environment);
  if (spawn_result != 0 || spawned_pid <= 0 ||
      waitpid(spawned_pid, &wait_status, 0) != spawned_pid ||
      !WIFEXITED(wait_status) || WEXITSTATUS(wait_status) != 0) return 402;
  errno = 42;
  if (posix_spawn(&spawned_pid, "/nelibc-no-such-executable", NULL, NULL,
                  missing_argv, empty_environment) != ENOENT) return 403;
  errno = 42;
  spawn_result = posix_spawnp(&spawned_pid, "true", NULL, NULL,
                              spawn_argv, empty_environment);
  if (spawn_result != 0 || spawned_pid <= 0 ||
      waitpid(spawned_pid, &wait_status, 0) != spawned_pid ||
      !WIFEXITED(wait_status) || WEXITSTATUS(wait_status) != 0) return 404;
  struct rusage process_usage;
  if (getrusage(RUSAGE_SELF, &process_usage) != 0 || process_usage.ru_maxrss < 0) return 367;
  errno = 0;
  if (getrusage(12345, &process_usage) != -1 || errno != EINVAL) return 368;
  struct tms process_times;
  if (times(&process_times) < 0 || process_times.tms_utime < 0 || process_times.tms_stime < 0) return 369;
  struct timeval wall_time;
  if (gettimeofday(&wall_time, NULL) != 0 || wall_time.tv_usec < 0 || wall_time.tv_usec >= 1000000) return 370;
  time_t stored_wall_seconds;
  time_t returned_wall_seconds = time(&stored_wall_seconds);
  if (returned_wall_seconds < 0 || stored_wall_seconds != returned_wall_seconds) return 371;
  time_t wall_delta = returned_wall_seconds - wall_time.tv_sec;
  if (wall_delta < 0 || wall_delta > 1 || time(NULL) < 0) return 372;
  if (setresuid((uid_t)-1, (uid_t)-1, (uid_t)-1) != 0) return 353;
  if (setreuid((uid_t)-1, (uid_t)-1) != 0) return 547;
  if (setresgid((gid_t)-1, (gid_t)-1, (gid_t)-1) != 0) return 354;
  if (setregid((gid_t)-1, (gid_t)-1) != 0) return 548;
  errno = 0;
  if (getresuid(NULL, &effective_uid, &saved_uid) != -1 || errno != EFAULT) return 355;
  errno = 0;
  if (getresgid(&real_gid, NULL, &saved_gid) != -1 || errno != EFAULT) return 356;
  if (getppid() != (pid_t)nelibc_raw_syscall3(110, 0, 0, 0)) return 107;
  if (gettid() != (pid_t)nelibc_raw_syscall3(186, 0, 0, 0)) return 108;
  if (getpgid(0) != (pid_t)nelibc_raw_syscall3(121, 0, 0, 0)) return 109;
  if (getpgrp() != (pid_t)nelibc_raw_syscall3(121, 0, 0, 0)) return 110;
  errno = 0;
  if (getpgid(-1) != -1 || errno != ESRCH) return 111;
  pid_t process_group_child = fork();
  if (process_group_child < 0) return 544;
  if (process_group_child == 0) {
    if (gettid() != getpid()) _exit(47);
    _exit(setpgid(0, 0) == 0 ? 45 : 46);
  }
  int process_group_status = 0;
  if (waitpid(process_group_child, &process_group_status, 0) != process_group_child ||
      !WIFEXITED(process_group_status) || WEXITSTATUS(process_group_status) != 45) return 545;
  errno = 0;
  if (setpgid(2147483647, 0) != -1 || errno != ESRCH) return 546;
  pid_t old_pgid = getpgrp();
  if (getsid(0) != (pid_t)nelibc_raw_syscall3(124, 0, 0, 0)) return 112;
  errno = 0;
  if (getsid(-1) != -1 || errno != ESRCH) return 113;
  errno = 0;
  pid_t new_sid = setsid();
  if (old_pgid == getpid()) {
    if (new_sid != -1 || errno != EPERM) return 114;
  } else if (new_sid != getpid() || getsid(0) != getpid() || getpgrp() != getpid()) {
    return 115;
  }
  if (kill(getpid(), 0) != 0) return 116;
  errno = 0;
  if (kill(2147483647, 0) != -1 || errno != ESRCH) return 117;
  errno = 0;
  if (kill(getpid(), NSIG) != -1 || errno != EINVAL) return 118;
  sigset_t usr1_set;
  struct {
    unsigned char before[8];
    sigset_t value;
    unsigned char after[8];
  } sigset_guard;
  memset(&sigset_guard, 0xA5, sizeof(sigset_guard));
  if (sigemptyset(&sigset_guard.value) != 0) return 132;
  for (size_t i = 0; i < sizeof(sigset_guard.before); ++i) {
    if (sigset_guard.before[i] != 0xA5 || sigset_guard.after[i] != 0xA5) return 133;
  }
  const unsigned char *empty_bytes = (const unsigned char *)&sigset_guard.value;
  for (size_t i = 0; i < sizeof(sigset_guard.value); ++i) {
    if (empty_bytes[i] != 0) return 134;
  }
  if (sigfillset(&sigset_guard.value) != 0) return 135;
  for (size_t i = 0; i < sizeof(sigset_guard.before); ++i) {
    if (sigset_guard.before[i] != 0xA5 || sigset_guard.after[i] != 0xA5) return 136;
  }
  if (sigemptyset(&usr1_set) != 0 || sigaddset(&usr1_set, SIGUSR1) != 0) return 119;
  sigset_t high_signal_set;
  if (sigemptyset(&high_signal_set) != 0 || sigaddset(&high_signal_set, 64) != 0 ||
      sigismember(&high_signal_set, 64) != 1) return 128;
  if (sigdelset(&high_signal_set, 64) != 0 || sigismember(&high_signal_set, 64) != 0) return 130;
  if (sigfillset(&high_signal_set) != 0 || sigismember(&high_signal_set, 64) != 1 ||
      sigdelset(&high_signal_set, 64) != 0 || sigismember(&high_signal_set, 64) != 0) return 131;
  errno = 0;
  if (sigaddset(&high_signal_set, NSIG) != -1 || errno != EINVAL) return 129;
  if (sigprocmask(SIG_BLOCK, &usr1_set, NULL) != 0) return 119;
  if (raise(SIGUSR1) != 0) return 120;
  struct sigaction installed_action;
  struct sigaction previous_action;
  struct sigaction observed_action;
  memset(&installed_action, 0, sizeof installed_action);
  installed_action.sa_handler = smoke_signal_handler;
  if (sigaction(SIGUSR2, &installed_action, &previous_action) != 0) return 124;
  if (raise(SIGUSR2) != 0 || handled_signal != SIGUSR2) return 125;
  if (sigaction(SIGUSR2, NULL, &observed_action) != 0 ||
      observed_action.sa_handler != smoke_signal_handler) return 126;
  if (sigaction(SIGUSR2, &previous_action, NULL) != 0) return 127;
  sigset_t pending_set;
  memset(&pending_set, 0xA5, sizeof pending_set);
  if (sigpending(&pending_set) != 0) return 121;
  if (sigismember(&pending_set, SIGUSR1) != 1) return 122;
  const unsigned long *pending_words = (const unsigned long *)&pending_set;
  for (size_t pending_word = 1;
       pending_word < sizeof pending_set / sizeof pending_words[0];
       ++pending_word) {
    if (pending_words[pending_word] != 0) return 137;
  }
  if ((pending_words[0] & (1UL << 31)) != 0) return 138;
  errno = 0;
  if (raise(NSIG) != -1 || errno != EINVAL) return 123;
  char cwd[4096];
  char short_cwd[1];
  if (getcwd(cwd, sizeof cwd) != cwd || cwd[0] != '/' || strlen(cwd) == 0) return 92;
  errno = 0;
  if (getcwd(short_cwd, sizeof short_cwd) != 0) return 93;
  if (errno != ERANGE) return 97;
  errno = 0;
  if (getcwd(short_cwd, 0) != 0 || errno != EINVAL) return 94;
  char cwd_check[sizeof cwd];
  if (chdir("/") != 0 || getcwd(cwd_check, sizeof cwd_check) != cwd_check || strcmp(cwd_check, "/") != 0) return 95;
  if (chdir(cwd) != 0 || getcwd(cwd_check, sizeof cwd_check) != cwd_check || strcmp(cwd_check, cwd) != 0) return 96;
  errno = 0;
  if (chdir((const char *)(uintptr_t)1) != -1 || errno != EFAULT) return 97;
  int rootfd = openat(AT_FDCWD, "/", O_RDONLY | O_DIRECTORY);
  if (rootfd < 0 || fchdir(rootfd) != 0) return 98;
  if (getcwd(cwd_check, sizeof cwd_check) != cwd_check || strcmp(cwd_check, "/") != 0) return 99;
  if (chdir(cwd) != 0 || close(rootfd) != 0) return 100;
  if (getcwd(cwd_check, sizeof cwd_check) != cwd_check || strcmp(cwd_check, cwd) != 0) return 101;
  errno = 0;
  if (fchdir(-1) != -1 || errno != EBADF) return 102;

  DIR *directory = opendir(".");
  if (directory == NULL) return 560;
  int directory_fd = dirfd(directory);
  if (directory_fd < 0 || (fcntl(directory_fd, F_GETFD) & FD_CLOEXEC) == 0) return 561;
  struct dirent *directory_entry = readdir(directory);
  if (directory_entry == NULL) return 562;
  char first_entry_name[256];
  strcpy(first_entry_name, directory_entry->d_name);
  long directory_cookie = telldir(directory);
  if (directory_cookie < 0) return 563;
  directory_entry = readdir(directory);
  if (directory_entry == NULL) return 564;
  char second_entry_name[256];
  strcpy(second_entry_name, directory_entry->d_name);
  seekdir(directory, directory_cookie);
  directory_entry = readdir(directory);
  if (directory_entry == NULL || strcmp(directory_entry->d_name, second_entry_name) != 0) return 565;
  rewinddir(directory);
  directory_entry = readdir(directory);
  if (directory_entry == NULL || strcmp(directory_entry->d_name, first_entry_name) != 0) return 566;
  rewinddir(directory);
  int saw_dot = 0;
  int saw_dotdot = 0;
  errno = 0;
  while ((directory_entry = readdir(directory)) != NULL) {
    if (strcmp(directory_entry->d_name, ".") == 0) saw_dot = 1;
    if (strcmp(directory_entry->d_name, "..") == 0) saw_dotdot = 1;
  }
  if (errno != 0 || !saw_dot || !saw_dotdot || closedir(directory) != 0) return 567;
  int fdopendir_fd = open(".", O_RDONLY | O_DIRECTORY);
  if (fdopendir_fd < 0) return 568;
  directory = fdopendir(fdopendir_fd);
  if (directory == NULL || dirfd(directory) != fdopendir_fd ||
      (fcntl(fdopendir_fd, F_GETFD) & FD_CLOEXEC) != 0 || closedir(directory) != 0) return 569;
  errno = 0;
  if (fcntl(fdopendir_fd, F_GETFD) != -1 || errno != EBADF) return 570;
  int not_directory_fd = open("/dev/null", O_RDONLY);
  if (not_directory_fd < 0) return 571;
  errno = 0;
  if (fdopendir(not_directory_fd) != NULL || errno != ENOTDIR) return 572;
  if (close(not_directory_fd) != 0) return 573;
  int path_directory_fd = open(".", O_PATH | O_DIRECTORY);
  if (path_directory_fd < 0) return 575;
  errno = 0;
  if (fdopendir(path_directory_fd) != NULL || errno != EBADF) return 576;
  if (close(path_directory_fd) != 0) return 577;
  errno = 0;
  if (opendir("/nelibc-no-such-directory") != NULL || errno != ENOENT) return 574;
  struct dirent **scanned_entries = NULL;
  int scanned_count = scandir(".", &scanned_entries, select_non_dot_entries, alphasort);
  if (scanned_count <= 0 || scanned_entries == NULL) return 578;
  for (int i = 0; i < scanned_count; ++i) {
    if (scanned_entries[i] == NULL || scanned_entries[i]->d_name[0] == '.' ||
        (i > 0 && strcmp(scanned_entries[i - 1]->d_name, scanned_entries[i]->d_name) > 0)) return 579;
  }
  for (int i = 0; i < scanned_count; ++i) free(scanned_entries[i]);
  free(scanned_entries);
  int scan_base_fd = open(".", O_RDONLY | O_DIRECTORY);
  if (scan_base_fd < 0) return 582;
  scanned_entries = NULL;
  int relative_scanned_count = scandirat(scan_base_fd, ".", &scanned_entries,
                                         select_non_dot_entries, alphasort);
  if (relative_scanned_count != scanned_count || scanned_entries == NULL) return 583;
  for (int i = 0; i < relative_scanned_count; ++i) {
    if (scanned_entries[i] == NULL || scanned_entries[i]->d_name[0] == '.' ||
        (i > 0 && strcmp(scanned_entries[i - 1]->d_name, scanned_entries[i]->d_name) > 0)) return 585;
  }
  for (int i = 0; i < relative_scanned_count; ++i) free(scanned_entries[i]);
  free(scanned_entries);
  if (close(scan_base_fd) != 0) return 584;
  errno = 0;
  if (scandirat(-1, ".", &scanned_entries, NULL, NULL) != -1 || errno != EBADF ||
      scanned_entries != NULL) return 586;
  static const struct {
    const char *left;
    const char *right;
    int expected_sign;
  } version_cases[] = {
    {"file2", "file10", -1}, {"file1", "file01", 1},
    {"file01", "file001", 1}, {"x009", "x010", -1},
    {"x1a", "x1", 1}, {"x10a", "x10b", -1},
    {"x0", "x00", 1}, {"x00a", "x00b", -1},
    {"a2b3", "a2b12", -1}, {"a", "a0", -1},
    {"a0001", "a001", -1}
  };
  for (size_t i = 0; i < sizeof version_cases / sizeof version_cases[0]; ++i) {
    struct dirent left_entry = {0};
    struct dirent right_entry = {0};
    strcpy(left_entry.d_name, version_cases[i].left);
    strcpy(right_entry.d_name, version_cases[i].right);
    const struct dirent *left_pointer = &left_entry;
    const struct dirent *right_pointer = &right_entry;
    int string_order = strverscmp(version_cases[i].left, version_cases[i].right);
    int string_sign = (string_order > 0) - (string_order < 0);
    if (string_sign != version_cases[i].expected_sign) return 587;
    int actual = versionsort(&left_pointer, &right_pointer);
    int actual_sign = (actual > 0) - (actual < 0);
    if (actual_sign != string_sign) {
      fprintf(stderr, "versionsort/strverscmp mismatch %s vs %s: %d vs %d\n",
              version_cases[i].left, version_cases[i].right, actual_sign, string_sign);
      return 588;
    }
  }
  errno = 0;
  int (*volatile scandir_call)(const char *, struct dirent ***,
                               int (*)(const struct dirent *),
                               int (*)(const struct dirent **, const struct dirent **)) = scandir;
  if (scandir_call(".", NULL, NULL, NULL) != -1 || errno != EINVAL) return 580;
  errno = 0;
  if (scandir_call("/nelibc-no-such-directory", &scanned_entries, NULL, NULL) != -1 || errno != ENOENT) return 581;

  struct utsname system_name;
  if (uname(&system_name) != 0 || strcmp(system_name.sysname, "Linux") != 0 ||
      strcmp(system_name.machine, "x86_64") != 0) return 182;
  errno = 0;
  if (uname((struct utsname *)(uintptr_t)1) != -1 || errno != EFAULT) return 183;

  mode_t previous_mask = umask(0077);
  mode_t mask_before_restore = umask(previous_mask);
  if (mask_before_restore != 0077) return 179;
  char umask_path[64];
  make_seek_path(umask_path);
  size_t umask_path_length = strlen(umask_path);
  static const char umask_suffix[] = ".umask";
  for (size_t i = 0; i < sizeof umask_suffix; ++i)
    umask_path[umask_path_length + i] = umask_suffix[i];
  previous_mask = umask(0077);
  int umask_fd = openat(AT_FDCWD, umask_path, O_CREAT | O_EXCL | O_WRONLY, 0666);
  mask_before_restore = umask(previous_mask);
  if (umask_fd < 0 || mask_before_restore != 0077) return 180;
  struct stat umask_status;
  int umask_status_result = fstat(umask_fd, &umask_status);
  int umask_close_result = close(umask_fd);
  int umask_unlink_result = unlinkat(AT_FDCWD, umask_path, 0);
  if (umask_status_result != 0 || (umask_status.st_mode & 0777) != 0600 ||
      umask_close_result != 0 || umask_unlink_result != 0) return 181;

  struct timespec monotonic_time;
  struct timespec clock_resolution;
  if (clock_gettime(CLOCK_MONOTONIC, &monotonic_time) != 0 ||
      monotonic_time.tv_nsec < 0 || monotonic_time.tv_nsec >= 1000000000L) return 163;
  if (clock_getres(CLOCK_MONOTONIC, &clock_resolution) != 0 ||
      clock_resolution.tv_sec < 0 || clock_resolution.tv_nsec < 0 ||
      clock_resolution.tv_nsec >= 1000000000L) return 164;
  struct timespec ignored_clock_value = {.tv_sec = 0, .tv_nsec = 0};
  errno = 0;
  if (clock_settime((clockid_t)-1, &ignored_clock_value) != -1 || errno != EINVAL)
    return 394;
  clockid_t process_cpu_clock = (clockid_t)-1;
  struct timespec process_cpu_time;
  if (clock_getcpuclockid(0, &process_cpu_clock) != 0 ||
      clock_gettime(process_cpu_clock, &process_cpu_time) != 0 ||
      process_cpu_time.tv_sec < 0 || process_cpu_time.tv_nsec < 0 ||
      process_cpu_time.tv_nsec >= 1000000000L)
    return 395;
  clockid_t explicit_process_cpu_clock = (clockid_t)-1;
  if (clock_getcpuclockid(getpid(), &explicit_process_cpu_clock) != 0 ||
      explicit_process_cpu_clock != process_cpu_clock)
    return 397;
  errno = EOVERFLOW;
  process_cpu_clock = 12345;
  if (clock_getcpuclockid((pid_t)-1, &process_cpu_clock) != EINVAL ||
      errno != EOVERFLOW || process_cpu_clock != 12345 ||
      clock_getcpuclockid(0, NULL) != EINVAL ||
      clock_getcpuclockid((pid_t)2147483647, &process_cpu_clock) != ESRCH ||
      errno != EOVERFLOW || process_cpu_clock != 12345)
    return 396;
  struct timespec utc_time = {.tv_sec = -1, .tv_nsec = -1};
  if (timespec_get(&utc_time, TIME_UTC) != TIME_UTC || utc_time.tv_sec < 0 ||
      utc_time.tv_nsec < 0 || utc_time.tv_nsec >= 1000000000L) return 389;
  struct timespec unsupported_time = {.tv_sec = 123, .tv_nsec = 456};
  if (timespec_get(&unsupported_time, 0) != 0 ||
      unsupported_time.tv_sec != 123 || unsupported_time.tv_nsec != 456) return 390;
  struct timespec utc_resolution = {.tv_sec = -1, .tv_nsec = -1};
  if (timespec_getres(&utc_resolution, TIME_UTC) != TIME_UTC ||
      utc_resolution.tv_sec < 0 || utc_resolution.tv_nsec < 0 ||
      utc_resolution.tv_nsec >= 1000000000L) return 391;
  struct timespec unsupported_resolution = {.tv_sec = 123, .tv_nsec = 456};
  if (timespec_getres(&unsupported_resolution, 0) != 0 ||
      unsupported_resolution.tv_sec != 123 || unsupported_resolution.tv_nsec != 456)
    return 392;
  if (timespec_getres(NULL, TIME_UTC) != TIME_UTC) return 393;
  struct tm utc_broken_down;
  time_t epoch = 0;
  if (gmtime_r(&epoch, &utc_broken_down) != &utc_broken_down ||
      utc_broken_down.tm_year != 70 || utc_broken_down.tm_mon != 0 ||
      utc_broken_down.tm_mday != 1 || utc_broken_down.tm_hour != 0 ||
      utc_broken_down.tm_min != 0 || utc_broken_down.tm_sec != 0 ||
      utc_broken_down.tm_wday != 4 || utc_broken_down.tm_yday != 0 ||
      utc_broken_down.tm_isdst != 0 || utc_broken_down.tm_gmtoff != 0 ||
      strcmp(utc_broken_down.tm_zone, "UTC") != 0) return 396;
  time_t before_epoch = (time_t)-1;
  if (gmtime_r(&before_epoch, &utc_broken_down) != &utc_broken_down ||
      utc_broken_down.tm_year != 69 || utc_broken_down.tm_mon != 11 ||
      utc_broken_down.tm_mday != 31 || utc_broken_down.tm_hour != 23 ||
      utc_broken_down.tm_min != 59 || utc_broken_down.tm_sec != 59 ||
      utc_broken_down.tm_wday != 3 || utc_broken_down.tm_yday != 364) return 397;
  time_t leap_day = (time_t)951782400;
  if (gmtime_r(&leap_day, &utc_broken_down) != &utc_broken_down ||
      utc_broken_down.tm_year != 100 || utc_broken_down.tm_mon != 1 ||
      utc_broken_down.tm_mday != 29 || utc_broken_down.tm_wday != 2 ||
      utc_broken_down.tm_yday != 59) return 398;
  if (gmtime(&epoch) == NULL || gmtime(&epoch)->tm_year != 70) return 399;
  if (setenv("TZ", "EST5", 1) != 0) return 400;
  if (localtime_r(&epoch, &utc_broken_down) != &utc_broken_down ||
      utc_broken_down.tm_year != 69 || utc_broken_down.tm_mon != 11 ||
      utc_broken_down.tm_mday != 31 || utc_broken_down.tm_hour != 19 ||
      utc_broken_down.tm_wday != 3 || utc_broken_down.tm_yday != 364 ||
      utc_broken_down.tm_isdst != 0 || utc_broken_down.tm_gmtoff != -18000 ||
      strcmp(utc_broken_down.tm_zone, "EST") != 0) return 401;
  if (timezone != 18000 || daylight != 0 || strcmp(tzname[0], "EST") != 0 ||
      strcmp(tzname[1], "EST") != 0) return 462;
  struct tm *static_local_epoch = localtime(&epoch);
  if (static_local_epoch == NULL || static_local_epoch->tm_hour != 19 ||
      static_local_epoch->tm_gmtoff != -18000 || strcmp(static_local_epoch->tm_zone, "EST") != 0)
    return 463;
  char formatted_time[128];
  if (strftime(formatted_time, sizeof formatted_time,
      "%Y-%m-%d %H:%M:%S %a %A %b %B %j %w %u %z %Z %%", &utc_broken_down) !=
      strlen("1969-12-31 19:00:00 Wed Wednesday Dec December 365 3 3 -0500 EST %") ||
      strcmp(formatted_time,
      "1969-12-31 19:00:00 Wed Wednesday Dec December 365 3 3 -0500 EST %") != 0)
    return 402;
  if (strftime(formatted_time, sizeof formatted_time, "%s|%EY|%Om|%k|%l",
      &utc_broken_down) != strlen("0|1969|12|19| 7") ||
      strcmp(formatted_time, "0|1969|12|19| 7") != 0) return 423;
  if (strftime(formatted_time, sizeof formatted_time, "%F|%T|%D|%R|%r|%c|%U|%W",
      &utc_broken_down) != strlen("1969-12-31|19:00:00|12/31/69|19:00|07:00:00 PM|Wed Dec 31 19:00:00 1969|52|52") ||
      strcmp(formatted_time,
      "1969-12-31|19:00:00|12/31/69|19:00|07:00:00 PM|Wed Dec 31 19:00:00 1969|52|52") != 0)
    return 406;
  char traditional_time[26];
  if (asctime_r(&utc_broken_down, traditional_time) != traditional_time ||
      strlen(traditional_time) != 25 ||
      strcmp(traditional_time, "Wed Dec 31 19:00:00 1969\n") != 0) return 432;
  char *static_asctime = asctime(&utc_broken_down);
  if (static_asctime == NULL ||
      strcmp(static_asctime, "Wed Dec 31 19:00:00 1969\n") != 0 ||
      ctime_r(&epoch, traditional_time) != traditional_time ||
      strcmp(traditional_time, "Wed Dec 31 19:00:00 1969\n") != 0 ||
      ctime(&epoch) != static_asctime ||
      strcmp(static_asctime, "Wed Dec 31 19:00:00 1969\n") != 0)
    return 433;
  time_t iso_2016_start = (time_t)1451606400;
  time_t iso_2017_start = (time_t)1483228800;
  time_t iso_2020_start = (time_t)1577664000;
  if (gmtime_r(&iso_2016_start, &utc_broken_down) == NULL ||
      strftime(formatted_time, sizeof formatted_time, "%G-W%V-%u %g", &utc_broken_down) != 13 ||
      strcmp(formatted_time, "2015-W53-5 15") != 0) return 419;
  if (gmtime_r(&iso_2017_start, &utc_broken_down) == NULL ||
      strftime(formatted_time, sizeof formatted_time, "%G-W%V-%u %g", &utc_broken_down) != 13 ||
      strcmp(formatted_time, "2016-W52-7 16") != 0) return 420;
  if (gmtime_r(&iso_2020_start, &utc_broken_down) == NULL ||
      strftime(formatted_time, sizeof formatted_time, "%G-W%V-%u %g", &utc_broken_down) != 13 ||
      strcmp(formatted_time, "2020-W01-1 20") != 0) return 421;
  if (setenv("TZ", "EST5", 1) != 0 || localtime_r(&epoch, &utc_broken_down) == NULL) return 422;
  char truncated_time[8] = "filled";
  if (strftime(truncated_time, sizeof truncated_time, "%Y-%m-%d", &utc_broken_down) != 0 ||
      truncated_time[0] != 0) return 403;
  if (localtime(&epoch) == NULL || localtime(&epoch)->tm_hour != 19) return 404;
  if (setenv("TZ", "America/Los_Angeles", 1) != 0) return 407;
  time_t summer_epoch = (time_t)1593561600;
  if (localtime_r(&epoch, &utc_broken_down) != &utc_broken_down ||
      utc_broken_down.tm_hour != 16 || utc_broken_down.tm_mday != 31 ||
      utc_broken_down.tm_isdst != 0 || utc_broken_down.tm_gmtoff != -28800 ||
      strcmp(utc_broken_down.tm_zone, "PST") != 0) return 408;
  if (localtime_r(&summer_epoch, &utc_broken_down) != &utc_broken_down ||
      utc_broken_down.tm_hour != 17 || utc_broken_down.tm_mday != 30 ||
      utc_broken_down.tm_isdst != 1 || utc_broken_down.tm_gmtoff != -25200 ||
      strcmp(utc_broken_down.tm_zone, "PDT") != 0) return 409;
  time_t footer_winter = (time_t)16726435200LL;
  time_t footer_summer = (time_t)16742073600LL;
  if (localtime_r(&footer_winter, &utc_broken_down) != &utc_broken_down ||
      utc_broken_down.tm_year != 600 || utc_broken_down.tm_mon != 0 ||
      utc_broken_down.tm_mday != 14 || utc_broken_down.tm_hour != 16 ||
      utc_broken_down.tm_isdst != 0 || utc_broken_down.tm_gmtoff != -28800 ||
      strcmp(utc_broken_down.tm_zone, "PST") != 0) return 410;
  if (localtime_r(&footer_summer, &utc_broken_down) != &utc_broken_down ||
      utc_broken_down.tm_year != 600 || utc_broken_down.tm_mon != 6 ||
      utc_broken_down.tm_mday != 14 || utc_broken_down.tm_hour != 17 ||
      utc_broken_down.tm_isdst != 1 || utc_broken_down.tm_gmtoff != -25200 ||
      strcmp(utc_broken_down.tm_zone, "PDT") != 0) return 411;
  if (setenv("TZ", "EST5EDT,M3.2.0/2,M11.1.0/2", 1) != 0 ||
      localtime_r(&summer_epoch, &utc_broken_down) != &utc_broken_down ||
      utc_broken_down.tm_hour != 20 || utc_broken_down.tm_mday != 30 ||
      utc_broken_down.tm_isdst != 1 || utc_broken_down.tm_gmtoff != -14400 ||
      strcmp(utc_broken_down.tm_zone, "EDT") != 0) return 412;
  if (setenv("TZ", "Australia/Sydney", 1) != 0 ||
      localtime_r(&footer_winter, &utc_broken_down) != &utc_broken_down ||
      utc_broken_down.tm_hour != 11 || utc_broken_down.tm_mday != 15 ||
      utc_broken_down.tm_isdst != 1 || utc_broken_down.tm_gmtoff != 39600 ||
      strcmp(utc_broken_down.tm_zone, "AEDT") != 0) return 414;
  if (localtime_r(&footer_summer, &utc_broken_down) != &utc_broken_down ||
      utc_broken_down.tm_hour != 10 || utc_broken_down.tm_mday != 15 ||
      utc_broken_down.tm_isdst != 0 || utc_broken_down.tm_gmtoff != 36000 ||
      strcmp(utc_broken_down.tm_zone, "AEST") != 0) return 415;
  if (setenv("TZ", "EST5EDT,J60/2,J300/2", 1) != 0 ||
      localtime_r(&footer_summer, &utc_broken_down) != &utc_broken_down ||
      utc_broken_down.tm_hour != 20 || utc_broken_down.tm_mday != 14 ||
      utc_broken_down.tm_isdst != 1 || utc_broken_down.tm_gmtoff != -14400 ||
      strcmp(utc_broken_down.tm_zone, "EDT") != 0) return 416;
  if (setenv("TZ", "EST5EDT,59/2,300/2", 1) != 0 ||
      localtime_r(&footer_summer, &utc_broken_down) != &utc_broken_down ||
      utc_broken_down.tm_hour != 20 || utc_broken_down.tm_mday != 14 ||
      utc_broken_down.tm_isdst != 1 || utc_broken_down.tm_gmtoff != -14400 ||
      strcmp(utc_broken_down.tm_zone, "EDT") != 0) return 417;
  if (setenv("TZ", "EST5EDT", 1) != 0 ||
      localtime_r(&footer_summer, &utc_broken_down) != &utc_broken_down ||
      utc_broken_down.tm_hour != 20 || utc_broken_down.tm_mday != 14 ||
      utc_broken_down.tm_isdst != 1 || utc_broken_down.tm_gmtoff != -14400 ||
      strcmp(utc_broken_down.tm_zone, "EDT") != 0) return 418;
  if (setenv("TZ", "EST5", 1) != 0) return 413;
  if (setenv("TZ", "UTC0", 1) != 0) return 424;
  tzset();
  if (timezone != 0 || daylight != 0 || strcmp(tzname[0], "UTC") != 0 ||
      strcmp(tzname[1], "UTC") != 0) return 454;
  if (setenv("TZ", "EST5EDT", 1) != 0) return 455;
  if (localtime_r(&epoch, &utc_broken_down) != &utc_broken_down) return 460;
  if (timezone != 18000 || daylight != 1 || strcmp(tzname[0], "EST") != 0 ||
      strcmp(tzname[1], "EDT") != 0) return 456;
  if (setenv("TZ", "America/Los_Angeles", 1) != 0) return 457;
  if (localtime_r(&epoch, &utc_broken_down) != &utc_broken_down) return 461;
  if (timezone != 28800 || daylight != 1 || strcmp(tzname[0], "PST") != 0 ||
      strcmp(tzname[1], "PDT") != 0) return 458;
  if (setenv("TZ", "UTC0", 1) != 0) return 459;
  tzset();
  struct tm normalized_input = {
    .tm_sec = 0, .tm_min = 0, .tm_hour = 0, .tm_mday = 1,
    .tm_mon = 12, .tm_year = 120, .tm_isdst = -1
  };
  if (mktime(&normalized_input) != (time_t)1609459200 ||
      normalized_input.tm_year != 121 || normalized_input.tm_mon != 0 ||
      normalized_input.tm_mday != 1 || normalized_input.tm_wday != 5 ||
      normalized_input.tm_yday != 0 || normalized_input.tm_isdst != 0) return 425;
  normalized_input.tm_mon = 12;
  normalized_input.tm_year = 120;
  normalized_input.tm_mday = 1;
  normalized_input.tm_hour = 0;
  normalized_input.tm_min = 0;
  normalized_input.tm_sec = 0;
  if (timegm(&normalized_input) != (time_t)1609459200 ||
      normalized_input.tm_year != 121 || normalized_input.tm_mon != 0 ||
      normalized_input.tm_mday != 1 || normalized_input.tm_wday != 5 ||
      normalized_input.tm_yday != 0 || normalized_input.tm_isdst != 0 ||
      normalized_input.tm_gmtoff != 0) return 432;
  memset(&normalized_input, 0, sizeof normalized_input);
  normalized_input.tm_isdst = -1;
  char *parsed_time_end = strptime("Sunday, February 29 2024 11:07:08 PMtail",
                                   "%A, %B %d %Y %I:%M:%S %p", &normalized_input);
  if (parsed_time_end == NULL || strcmp(parsed_time_end, "tail") != 0 ||
      normalized_input.tm_year != 124 || normalized_input.tm_mon != 1 ||
      normalized_input.tm_mday != 29 || normalized_input.tm_wday != 0 ||
      normalized_input.tm_hour != 23 || normalized_input.tm_min != 7 ||
      normalized_input.tm_sec != 8) return 433;
  memset(&normalized_input, 0, sizeof normalized_input);
  parsed_time_end = strptime("2024-02-29 23:07:08!", "%F %T", &normalized_input);
  if (parsed_time_end == NULL || strcmp(parsed_time_end, "!") != 0 ||
      normalized_input.tm_year != 124 || normalized_input.tm_mon != 1 ||
      normalized_input.tm_mday != 29 || normalized_input.tm_hour != 23 ||
      normalized_input.tm_min != 7 || normalized_input.tm_sec != 8) return 434;
  memset(&normalized_input, 0, sizeof normalized_input);
  parsed_time_end = strptime("Sun Feb 29 23:07:08 2024!", "%c", &normalized_input);
  if (parsed_time_end == NULL || strcmp(parsed_time_end, "!") != 0 ||
      normalized_input.tm_year != 124 || normalized_input.tm_mon != 1 ||
      normalized_input.tm_mday != 29 || normalized_input.tm_wday != 0 ||
      normalized_input.tm_hour != 23 || normalized_input.tm_min != 7 ||
      normalized_input.tm_sec != 8) return 435;
  memset(&normalized_input, 0, sizeof normalized_input);
  parsed_time_end = strptime("2020-W53-5", "%G-W%V-%u", &normalized_input);
  if (parsed_time_end == NULL || *parsed_time_end != '\0' ||
      normalized_input.tm_year != 121 || normalized_input.tm_mon != 0 ||
      normalized_input.tm_mday != 1 || normalized_input.tm_wday != 5 ||
      normalized_input.tm_yday != 0) return 436;
  memset(&normalized_input, 0, sizeof normalized_input);
  parsed_time_end = strptime("2024 01 0", "%Y %U %w", &normalized_input);
  if (parsed_time_end == NULL || *parsed_time_end != '\0' ||
      normalized_input.tm_year != 124 || normalized_input.tm_mon != 0 ||
      normalized_input.tm_mday != 7 || normalized_input.tm_wday != 0 ||
      normalized_input.tm_yday != 6) return 437;
  memset(&normalized_input, 0, sizeof normalized_input);
  parsed_time_end = strptime("2024 01 1", "%Y %W %w", &normalized_input);
  if (parsed_time_end == NULL || *parsed_time_end != '\0' ||
      normalized_input.tm_year != 124 || normalized_input.tm_mon != 0 ||
      normalized_input.tm_mday != 8 || normalized_input.tm_wday != 1 ||
      normalized_input.tm_yday != 7) return 438;
  memset(&normalized_input, 0, sizeof normalized_input);
  if (strptime("2021-W53-1", "%G-W%V-%u", &normalized_input) != NULL) return 439;
  if (strptime("2024 54 0", "%Y %U %w", &normalized_input) != NULL) return 440;
  if (strptime("2023 53 0", "%Y %U %w", &normalized_input) != NULL) return 441;
  memset(&normalized_input, 0, sizeof normalized_input);
  parsed_time_end = strptime("2024-02-29\t+0530!", "%C%y-%m-%d%t%z", &normalized_input);
  if (parsed_time_end == NULL || strcmp(parsed_time_end, "!") != 0 ||
      normalized_input.tm_year != 124 || normalized_input.tm_mon != 1 ||
      normalized_input.tm_mday != 29 || normalized_input.tm_gmtoff != 19800) return 442;
  memset(&normalized_input, 0, sizeof normalized_input);
  parsed_time_end = strptime(" 7:05", "%k:%M", &normalized_input);
  if (parsed_time_end == NULL || *parsed_time_end != '\0' ||
      normalized_input.tm_hour != 7 || normalized_input.tm_min != 5) return 443;
  memset(&normalized_input, 0, sizeof normalized_input);
  parsed_time_end = strptime("UTC!", "%Z", &normalized_input);
  if (parsed_time_end == NULL || strcmp(parsed_time_end, "!") != 0) return 444;
  memset(&normalized_input, 0, sizeof normalized_input);
  parsed_time_end = strptime("20 24!", "%EC %Oy", &normalized_input);
  if (parsed_time_end == NULL || strcmp(parsed_time_end, "!") != 0 ||
      normalized_input.tm_year != 124) return 445;
  memset(&normalized_input, 0, sizeof normalized_input);
  parsed_time_end = strptime("11:30 -07:00", "%l:%M %z", &normalized_input);
  if (parsed_time_end == NULL || *parsed_time_end != '\0' ||
      normalized_input.tm_hour != 11 || normalized_input.tm_min != 30 ||
      normalized_input.tm_gmtoff != -25200) return 446;
  memset(&normalized_input, 0, sizeof normalized_input);
  if (setenv("TZ", "America/Los_Angeles", 1) != 0) return 451;
  parsed_time_end = strptime("0!", "%s", &normalized_input);
  if (parsed_time_end == NULL || strcmp(parsed_time_end, "!") != 0 ||
      normalized_input.tm_year != 69 || normalized_input.tm_mon != 11 ||
      normalized_input.tm_mday != 31 || normalized_input.tm_hour != 16 ||
      normalized_input.tm_gmtoff != -28800 || normalized_input.tm_isdst != 0) return 452;
  if (setenv("TZ", "UTC0", 1) != 0) return 453;
  memset(&normalized_input, 0, sizeof normalized_input);
  parsed_time_end = strptime("-1!", "%s", &normalized_input);
  if (parsed_time_end == NULL || strcmp(parsed_time_end, "!") != 0 ||
      normalized_input.tm_year != 69 || normalized_input.tm_mon != 11 ||
      normalized_input.tm_mday != 31 || normalized_input.tm_hour != 23 ||
      normalized_input.tm_min != 59 || normalized_input.tm_sec != 59) return 447;
  if (strptime("999999999999999999999999", "%s", &normalized_input) != NULL) return 448;
  if (strptime("13/01/2024", "%m/%d/%Y", &normalized_input) != NULL) return 449;
  if (strptime("24:00", "%H:%M", &normalized_input) != NULL) return 450;
  if (gmtime_r(&before_epoch, &normalized_input) == NULL) return 430;
  normalized_input.tm_isdst = -1;
  errno = 0;
  if (mktime(&normalized_input) != (time_t)-1 || errno != 0 ||
      normalized_input.tm_year != 69 || normalized_input.tm_mon != 11 ||
      normalized_input.tm_mday != 31 || normalized_input.tm_hour != 23 ||
      normalized_input.tm_min != 59 || normalized_input.tm_sec != 59) return 431;
  if (setenv("TZ", "America/Los_Angeles", 1) != 0) return 426;
  struct tm summer_local = {
    .tm_sec = 0, .tm_min = 0, .tm_hour = 12, .tm_mday = 1,
    .tm_mon = 6, .tm_year = 120, .tm_isdst = -1
  };
  if (mktime(&summer_local) != (time_t)1593630000 || summer_local.tm_isdst != 1 ||
      summer_local.tm_hour != 12 || summer_local.tm_gmtoff != -25200) return 427;
  if (timezone != 28800 || daylight != 1 || strcmp(tzname[0], "PST") != 0 ||
      strcmp(tzname[1], "PDT") != 0 ||
      strftime(formatted_time, sizeof formatted_time, "%Z", &summer_local) != 3 ||
      strcmp(formatted_time, "PDT") != 0) return 464;
  struct tm repeated_standard = {
    .tm_sec = 0, .tm_min = 30, .tm_hour = 1, .tm_mday = 1,
    .tm_mon = 10, .tm_year = 120, .tm_isdst = 0
  };
  struct tm repeated_daylight = repeated_standard;
  repeated_daylight.tm_isdst = 1;
  if (mktime(&repeated_standard) != (time_t)1604223000 ||
      repeated_standard.tm_isdst != 0 || repeated_standard.tm_hour != 1 ||
      mktime(&repeated_daylight) != (time_t)1604219400 ||
      repeated_daylight.tm_isdst != 1 || repeated_daylight.tm_hour != 1) return 428;
  struct tm skipped_hour = {
    .tm_sec = 0, .tm_min = 30, .tm_hour = 2, .tm_mday = 8,
    .tm_mon = 2, .tm_year = 120, .tm_isdst = -1
  };
  if (mktime(&skipped_hour) != (time_t)1583663400 || skipped_hour.tm_hour != 3 ||
      skipped_hour.tm_min != 30 || skipped_hour.tm_isdst != 1) return 429;
  if (unsetenv("TZ") != 0 || localtime_r(&epoch, &utc_broken_down) == NULL ||
      utc_broken_down.tm_zone == NULL) return 405;
#ifdef NELIBC_TESTING
  char tz_signature_path[64];
  make_seek_path(tz_signature_path);
  size_t tz_signature_path_length = strlen(tz_signature_path);
  static const char tz_signature_suffix[] = ".tzsig";
  for (size_t i = 0; i < sizeof tz_signature_suffix; ++i)
    tz_signature_path[tz_signature_path_length + i] = tz_signature_suffix[i];
  int tz_signature_fd = openat(AT_FDCWD, tz_signature_path,
      O_CREAT | O_EXCL | O_RDWR, 0600);
  char tz_signature_before[56];
  char tz_signature_after[56];
  if (tz_signature_fd < 0 ||
      __nelibc_tz_file_signature(tz_signature_path, tz_signature_before) != 0 ||
      write(tz_signature_fd, "x", 1) != 1 ||
      __nelibc_tz_file_signature(tz_signature_path, tz_signature_after) != 0 ||
      memcmp(tz_signature_before, tz_signature_after, sizeof tz_signature_before) == 0 ||
      close(tz_signature_fd) != 0 || unlink(tz_signature_path) != 0) return 476;
  char tz_file_path[64];
  make_seek_path(tz_file_path);
  size_t tz_file_path_length = strlen(tz_file_path);
  static const char tz_file_suffix[] = ".tzif";
  for (size_t i = 0; i < sizeof tz_file_suffix; ++i)
    tz_file_path[tz_file_path_length + i] = tz_file_suffix[i];
  int tz_file_fd = openat(AT_FDCWD, tz_file_path,
      O_CREAT | O_EXCL | O_RDWR, 0600);
  char tz_file_spec[80];
  if (tz_file_fd < 0 || copy_path_to_fd(tz_file_fd,
      "/usr/share/zoneinfo/America/New_York") != 0 ||
      snprintf(tz_file_spec, sizeof tz_file_spec, ":%s", tz_file_path) < 0 ||
      setenv("TZ", tz_file_spec, 1) != 0 ||
      localtime_r(&epoch, &utc_broken_down) != &utc_broken_down ||
      utc_broken_down.tm_gmtoff != -18000 || utc_broken_down.tm_isdst != 0 ||
      timezone != 18000 || daylight != 1 ||
      strcmp(tzname[0], "EST") != 0 || strcmp(tzname[1], "EDT") != 0 ||
      copy_path_to_fd(tz_file_fd, "/usr/share/zoneinfo/UTC") != 0 ||
      localtime_r(&epoch, &utc_broken_down) != &utc_broken_down ||
      utc_broken_down.tm_gmtoff != 0 || utc_broken_down.tm_isdst != 0 ||
      timezone != 0 || daylight != 0 ||
      strcmp(tzname[0], "UTC") != 0 || strcmp(tzname[1], "UTC") != 0 ||
      unsetenv("TZ") != 0 || close(tz_file_fd) != 0 ||
      unlink(tz_file_path) != 0) return 477;
  char default_zone_path[64];
  make_seek_path(default_zone_path);
  size_t default_zone_path_length = strlen(default_zone_path);
  static const char default_zone_suffix[] = ".defaulttz";
  for (size_t i = 0; i < sizeof default_zone_suffix; ++i)
    default_zone_path[default_zone_path_length + i] = default_zone_suffix[i];
  int default_zone_fd = openat(AT_FDCWD, default_zone_path,
      O_CREAT | O_EXCL | O_RDWR, 0600);
  if (default_zone_fd < 0 || copy_path_to_fd(default_zone_fd,
      "/usr/share/zoneinfo/America/New_York") != 0 ||
      __nelibc_set_localtime_path(default_zone_path) != 0 ||
      localtime_r(&epoch, &utc_broken_down) != &utc_broken_down ||
      utc_broken_down.tm_gmtoff != -18000 || timezone != 18000 ||
      daylight != 1 || strcmp(tzname[0], "EST") != 0 ||
      strcmp(tzname[1], "EDT") != 0 ||
      copy_path_to_fd(default_zone_fd, "/usr/share/zoneinfo/UTC") != 0 ||
      localtime_r(&epoch, &utc_broken_down) != &utc_broken_down ||
      utc_broken_down.tm_gmtoff != 0 || timezone != 0 || daylight != 0 ||
      strcmp(tzname[0], "UTC") != 0 || strcmp(tzname[1], "UTC") != 0 ||
      __nelibc_set_localtime_path("/etc/localtime") != 0 ||
      localtime_r(&epoch, &utc_broken_down) != &utc_broken_down ||
      close(default_zone_fd) != 0 || unlink(default_zone_path) != 0) return 478;
#endif
  if (difftime((time_t)100, (time_t)90) != 10.0 ||
      difftime((time_t)90, (time_t)100) != -10.0) return 394;
  clock_t cpu_clock_before = clock();
  volatile unsigned long cpu_clock_work = 0;
  for (unsigned long index = 0; index < 100000; ++index) cpu_clock_work += index;
  clock_t cpu_clock_after = clock();
  if (cpu_clock_work == 0 || cpu_clock_before == (clock_t)-1 ||
      cpu_clock_after == (clock_t)-1 || cpu_clock_after < cpu_clock_before)
    return 395;
  struct timespec zero_sleep = {.tv_sec = 0, .tv_nsec = 0};
  struct timespec invalid_sleep = {.tv_sec = 0, .tv_nsec = 1000000000L};
  if (nanosleep(&zero_sleep, NULL) != 0) return 165;
  errno = 0;
  if (clock_nanosleep(CLOCK_MONOTONIC, 0, &zero_sleep, NULL) != 0 || errno != 0) return 386;
  if (clock_nanosleep(CLOCK_MONOTONIC, 0, &invalid_sleep, NULL) != EINVAL || errno != 0) return 387;
  if (sleep(0) != 0 || alarm(0) != 0) return 381;
  errno = 0;
  if (nanosleep(&invalid_sleep, NULL) != -1 || errno != EINVAL) return 166;
  unsigned char random_bytes[16];
  if (getrandom(random_bytes, sizeof random_bytes, 0) != (ssize_t)sizeof random_bytes) return 167;
  unsigned char entropy_bytes[257];
  if (getentropy(entropy_bytes, sizeof entropy_bytes - 1) != 0) return 213;
  errno = 0;
  if (getentropy(entropy_bytes, sizeof entropy_bytes) != -1 || errno != EIO) return 214;
  errno = 0;
  if (getentropy(NULL, 1) != -1 || errno != EFAULT) return 215;
  errno = 0;
  if (getrandom(random_bytes, sizeof random_bytes, ~0u) != -1 || errno != EINVAL) return 168;
  errno = 0;
  if (getrandom(random_bytes, sizeof random_bytes, NELIBC_TEST_EINTR_FLAGS) != -1 ||
      errno != EINTR) return 173;
  errno = 0;
  if (mmap(NULL, 0, PROT_READ, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0) != MAP_FAILED ||
      errno != EINVAL) return 174;
  unsigned char *random_pages = mmap(NULL, 8192, PROT_READ | PROT_WRITE,
                                     MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (random_pages == MAP_FAILED) return 169;
  if (madvise(random_pages, 8192, MADV_NORMAL) != 0) return 177;
  errno = 0;
  if (madvise(random_pages, 8192, -1) != -1 || errno != EINVAL) return 178;
  errno = 0;
  if (mprotect(random_pages + 1, 4096, PROT_NONE) != -1 || errno != EINVAL) return 175;
  errno = 0;
  if (munmap(random_pages, 0) != -1 || errno != EINVAL) return 176;
  if (mprotect(random_pages + 4096, 4096, PROT_NONE) != 0) return 170;
  ssize_t partial_random = getrandom(random_pages, 8192, GRND_NONBLOCK);
  if (partial_random <= 0 || partial_random >= 8192) return 171;
  if (munmap(random_pages, 8192) != 0) return 172;
  unsigned char *remap_pages = mmap(NULL, 4096, PROT_READ | PROT_WRITE,
                                    MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (remap_pages == MAP_FAILED) return 388;
  remap_pages[0] = 0x5a;
  unsigned char *grown_pages = mremap(remap_pages, 4096, 8192, MREMAP_MAYMOVE);
  if (grown_pages == MAP_FAILED || grown_pages[0] != 0x5a) return 388;
  unsigned char *fixed_pages = mmap(NULL, 8192, PROT_NONE,
                                    MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (fixed_pages == MAP_FAILED) return 389;
  unsigned char *fixed_result = mremap(grown_pages, 8192, 8192,
                                       MREMAP_MAYMOVE | MREMAP_FIXED, fixed_pages);
  if (fixed_result != fixed_pages || fixed_result[0] != 0x5a) return 390;
  if (munmap(fixed_result, 8192) != 0) return 391;

  static const char message[] = "nelibc raw syscall ok\n";
  long failed_write = nelibc_linux_write(-1, message, 1);
  if (failed_write != -9) return 1; /* Linux x86_64 returns -EBADF directly. */

  errno = 0;
  if (write(-1, message, 1) != -1 || errno != EBADF) return 2;

  errno = 0;
  if (read(-1, (void *)message, 1) != -1 || errno != EBADF) return 3;

  errno = 0;
  if (pread(-1, (void *)message, 1, 0) != -1 || errno != EBADF) return 57;
  errno = 0;
  if (pwrite(-1, message, 1, 0) != -1 || errno != EBADF) return 58;

  errno = 0;
  if (close(-1) != -1 || errno != EBADF) return 4;
#ifdef NELIBC_STANDALONE
  if (strcmp(strerror(errno), "Bad file descriptor") != 0) return 32;
#endif

  int pipefd[2] = {-1, -1};
  int duplicated_write_fd;
  static const char pipe_message[] = "pipe round trip";
  char pipe_buffer[sizeof pipe_message] = {0};
  char pipe_buffer_tail[sizeof pipe_message] = {0};
  errno = 0;
  if (pipe(0) != -1 || errno != EFAULT) return 49;
  int atomic_pipe[2] = {-1, -1};
  if (nelibc_smoke_pipe2(atomic_pipe, O_CLOEXEC | O_NONBLOCK) != 0) return 307;
  if ((fcntl(atomic_pipe[0], F_GETFD) & FD_CLOEXEC) == 0 ||
      (fcntl(atomic_pipe[1], F_GETFD) & FD_CLOEXEC) == 0 ||
      (fcntl(atomic_pipe[0], F_GETFL) & O_NONBLOCK) == 0 ||
      (fcntl(atomic_pipe[1], F_GETFL) & O_NONBLOCK) == 0) return 308;
  int range_pipe[2] = {-1, -1};
  if (pipe(range_pipe) != 0) return 313;
  if (close_range((unsigned int)range_pipe[0], (unsigned int)range_pipe[0],
                  CLOSE_RANGE_CLOEXEC) != 0 ||
      (fcntl(range_pipe[0], F_GETFD) & FD_CLOEXEC) == 0) return 314;
  if (close(range_pipe[0]) != 0 || close(range_pipe[1]) != 0) return 315;
  int counter_fd = eventfd(2, EFD_CLOEXEC | EFD_NONBLOCK);
  if (counter_fd < 0 || (fcntl(counter_fd, F_GETFD) & FD_CLOEXEC) == 0 ||
      (fcntl(counter_fd, F_GETFL) & O_NONBLOCK) == 0) return 317;
  eventfd_t counter_value = 0;
  if (eventfd_write(counter_fd, 3) != 0 || eventfd_read(counter_fd, &counter_value) != 0 ||
      counter_value != 5 || close(counter_fd) != 0) return 318;
  counter_fd = eventfd(2, EFD_SEMAPHORE | EFD_NONBLOCK);
  if (counter_fd < 0 || eventfd_read(counter_fd, &counter_value) != 0 ||
      counter_value != 1 || close(counter_fd) != 0) return 319;
  int memfd = memfd_create("nelibc-smoke", MFD_CLOEXEC | MFD_ALLOW_SEALING);
  if (memfd < 0 || (fcntl(memfd, F_GETFD) & FD_CLOEXEC) == 0) return 589;
  static const char memfd_message[] = "memfd round trip";
  char memfd_buffer[sizeof memfd_message] = {0};
  if (write(memfd, memfd_message, sizeof memfd_message) != sizeof memfd_message ||
      lseek(memfd, 0, SEEK_SET) != 0 ||
      read(memfd, memfd_buffer, sizeof memfd_buffer) != sizeof memfd_buffer ||
      memcmp(memfd_buffer, memfd_message, sizeof memfd_message) != 0 ||
      close(memfd) != 0) return 590;
  int pidfd = pidfd_open(getpid(), 0);
  if (pidfd < 0 || (fcntl(pidfd, F_GETFD) & FD_CLOEXEC) == 0) return 591;
  if (pidfd_send_signal(pidfd, 0, NULL, 0) != 0 || close(pidfd) != 0) return 592;
  errno = 0;
  if (pidfd_send_signal(-1, 0, NULL, 0) != -1 || errno != EBADF) return 593;
  int epoll_fd = epoll_create1(EPOLL_CLOEXEC);
  int epoll_source = eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK);
  if (epoll_fd < 0 || epoll_source < 0 ||
      (fcntl(epoll_fd, F_GETFD) & FD_CLOEXEC) == 0) return 594;
  struct epoll_event requested = {.events = EPOLLIN, .data.u64 = 0x4e454c494243ULL};
  struct epoll_event received = {0};
  eventfd_t ready_value = 1;
  if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, epoll_source, &requested) != 0 ||
      eventfd_write(epoll_source, ready_value) != 0 ||
      epoll_pwait(epoll_fd, &received, 1, 1000, NULL) != 1 ||
      (received.events & EPOLLIN) == 0 ||
      received.data.u64 != requested.data.u64 ||
      eventfd_read(epoll_source, &ready_value) != 0 || ready_value != 1 ||
      epoll_wait(epoll_fd, &received, 1, 0) != 0 ||
      epoll_ctl(epoll_fd, EPOLL_CTL_DEL, epoll_source, NULL) != 0 ||
      close(epoll_source) != 0) return 595;
  struct timespec no_wait = {.tv_sec = 0, .tv_nsec = 0};
  if (epoll_pwait2(epoll_fd, &received, 1, &no_wait, NULL) != 0 ||
      close(epoll_fd) != 0) return 596;
  char atomic_byte = 0;
  errno = 0;
  if (read(atomic_pipe[0], &atomic_byte, 1) != -1 || errno != EAGAIN) return 309;
  if (write(atomic_pipe[1], "p", 1) != 1 || read(atomic_pipe[0], &atomic_byte, 1) != 1 ||
      atomic_byte != 'p') return 310;
  if (close(atomic_pipe[0]) != 0 || close(atomic_pipe[1]) != 0) return 311;
  errno = 0;
  if (nelibc_smoke_pipe2(atomic_pipe, 0x80000000) != -1 || errno != EINVAL) return 312;
  errno = 0;
  if (nelibc_smoke_pipe2(NULL, 0) != -1 || errno != EFAULT) return 316;
  if (pipe(pipefd) != 0 || pipefd[0] < 0 || pipefd[1] < 0) return 44;
  duplicated_write_fd = dup(pipefd[1]);
  if (duplicated_write_fd < 0 || close(pipefd[1]) != 0) return 132;
  if (dup2(duplicated_write_fd, pipefd[1]) != pipefd[1]) return 133;
  if (close(duplicated_write_fd) != 0) return 134;
  if (dup3(pipefd[1], duplicated_write_fd, O_CLOEXEC) != duplicated_write_fd) return 135;
  if ((fcntl(duplicated_write_fd, F_GETFD) & FD_CLOEXEC) == 0) return 156;
  if (fcntl(duplicated_write_fd, F_SETFD, 0) != 0 ||
      (fcntl(duplicated_write_fd, F_GETFD) & FD_CLOEXEC) != 0) return 157;
  errno = 0;
  if (fcntl(-1, F_GETFD) != -1 || errno != EBADF) return 158;
  struct iovec write_iov[2] = {
      {.iov_base = (void *)pipe_message, .iov_len = 5},
      {.iov_base = (void *)(pipe_message + 5), .iov_len = sizeof pipe_message - 5}};
  struct iovec read_iov[2] = {
      {.iov_base = pipe_buffer, .iov_len = 5},
      {.iov_base = pipe_buffer_tail, .iov_len = sizeof pipe_message - 5}};
  long initial_writev = writev(duplicated_write_fd, write_iov, 2);
  if (initial_writev != (long)sizeof pipe_message) {
    return 45;
  }
  struct pollfd pipe_poll = {.fd = pipefd[0], .events = POLLIN, .revents = 0};
  if (poll(&pipe_poll, 1, 0) != 1 || (pipe_poll.revents & POLLIN) == 0) return 159;
  fd_set readable_fds;
  FD_ZERO(&readable_fds);
  FD_SET(pipefd[0], &readable_fds);
  struct timeval select_timeout = {.tv_sec = 0, .tv_usec = 0};
  if (select(pipefd[0] + 1, &readable_fds, NULL, NULL, &select_timeout) != 1 ||
      !FD_ISSET(pipefd[0], &readable_fds)) return 161;
  FD_CLR(pipefd[0], &readable_fds);
  if (FD_ISSET(pipefd[0], &readable_fds)) return 182;
  if (readv(pipefd[0], read_iov, 2) != (long)sizeof pipe_message) return 46;
  pipe_poll.revents = 0;
  if (poll(&pipe_poll, 1, 0) != 0 || pipe_poll.revents != 0) return 160;
  FD_ZERO(&readable_fds);
  FD_SET(pipefd[0], &readable_fds);
  select_timeout.tv_sec = 0;
  select_timeout.tv_usec = 0;
  if (select(pipefd[0] + 1, &readable_fds, NULL, NULL, &select_timeout) != 0) return 162;
  if (memcmp(pipe_buffer, pipe_message, 5) != 0 ||
      memcmp(pipe_buffer_tail, pipe_message + 5, sizeof pipe_message - 5) != 0) return 47;
  int saved_stdout = dup(1);
  if (saved_stdout < 0 || dup2(duplicated_write_fd, 1) != 1) return 163;
#ifndef NELIBC_STANDALONE
  if (setvbuf(stdout, NULL, _IOFBF, 4096) != 0 || printf("before") < 0) return 167;
#endif
  if (puts("puts") != 0 || putchar('!') != '!') return 164;
  if (fwide(stdout, 0) >= 0 || fwide(stdout, 1) >= 0) return 287;
  if (putchar_unlocked('U') != 'U' || fputs_unlocked("V", stdout) < 0) return 181;
  flockfile(stdout);
  if (ftrylockfile(stdout) != 0) return 169;
  funlockfile(stdout);
  funlockfile(stdout);
  if (fflush(NULL) != 0) return 168;
#ifndef NELIBC_STANDALONE
  if (fputs("fputs", stdout) < 0 || fputc('?', stdout) != '?') return 178;
  if (fflush(stdout) != 0) return 168;
#endif
  if (dup2(saved_stdout, 1) != 1 || close(saved_stdout) != 0) return 165;
#ifdef NELIBC_STANDALONE
  char stdio_buffer[sizeof "puts\n!UV" - 1] = {0};
  if (read(pipefd[0], stdio_buffer, sizeof stdio_buffer) != (long)sizeof stdio_buffer ||
      memcmp(stdio_buffer, "puts\n!UV", sizeof stdio_buffer) != 0) return 166;
  int narrow_scan_int = 0;
  int narrow_scan_auto = 0;
  int narrow_scan_octal = 0;
  unsigned int narrow_scan_hex = 0;
  if (sscanf("42 -17 0x2a 077", "%d %i %x %i", &narrow_scan_int,
             &narrow_scan_auto, &narrow_scan_hex, &narrow_scan_octal) != 4 ||
      narrow_scan_int != 42 || narrow_scan_auto != -17 ||
      narrow_scan_hex != 42 || narrow_scan_octal != 63)
    return 301;
  if (sscanf("", "%d", &narrow_scan_int) != EOF ||
      sscanf("x", "%d", &narrow_scan_int) != 0 ||
      sscanf("12x", "%d", &narrow_scan_int) != 1 || narrow_scan_int != 12)
    return 311;
  if (sscanf("a 19", "%*c %d", &narrow_scan_int) != 1 ||
      narrow_scan_int != 19)
    return 312;
  char narrow_scan_word[8] = {0};
  int narrow_scan_count = -1;
  if (sscanf("alpha rest", "%5s%n", narrow_scan_word, &narrow_scan_count) != 1 ||
      strcmp(narrow_scan_word, "alpha") != 0 || narrow_scan_count != 5)
    return 302;
  char narrow_scan_buffer[] = "xyz123!";
  FILE *narrow_scan_stream = fmemopen(narrow_scan_buffer,
                                      sizeof narrow_scan_buffer - 1, "r");
  memset(narrow_scan_word, 0, sizeof narrow_scan_word);
  if (narrow_scan_stream == NULL ||
      fscanf(narrow_scan_stream, "%3[a-z]", narrow_scan_word) != 1 ||
      strcmp(narrow_scan_word, "xyz") != 0 || fgetc(narrow_scan_stream) != '1' ||
      fclose(narrow_scan_stream) != 0)
    return 303;
  char *allocated_scan_word = NULL;
  if (sscanf("allocated tail", "%ms", &allocated_scan_word) != 1 ||
      allocated_scan_word == NULL || strcmp(allocated_scan_word, "allocated") != 0)
    return 320;
  free(allocated_scan_word);
  allocated_scan_word = NULL;
  char long_allocated_input[97];
  memset(long_allocated_input, 'q', sizeof long_allocated_input - 1);
  long_allocated_input[sizeof long_allocated_input - 1] = 0;
  if (sscanf(long_allocated_input, "%ms", &allocated_scan_word) != 1 ||
      allocated_scan_word == NULL || strlen(allocated_scan_word) != 96)
    return 323;
  free(allocated_scan_word);
  allocated_scan_word = NULL;
  if (sscanf("wide123", "%4m[a-z]", &allocated_scan_word) != 1 ||
      allocated_scan_word == NULL || strcmp(allocated_scan_word, "wide") != 0)
    return 321;
  free(allocated_scan_word);
  allocated_scan_word = NULL;
  int allocated_scan_number = 0;
  if (sscanf("word 41", "%m[a-z] %d", &allocated_scan_word,
             &allocated_scan_number) != 2 || allocated_scan_word == NULL ||
      strcmp(allocated_scan_word, "word") != 0 || allocated_scan_number != 41)
    return 325;
  free(allocated_scan_word);
  allocated_scan_word = NULL;
  if (sscanf("UTF8", "%3mc", &allocated_scan_word) != 1 ||
      allocated_scan_word == NULL || memcmp(allocated_scan_word, "UTF", 3) != 0)
    return 322;
  free(allocated_scan_word);
  int positional_scan_number = 0;
  char positional_scan_word[16] = {0};
  if (sscanf("41", "%1$d", &positional_scan_number,
             positional_scan_word) != 1 || positional_scan_number != 41)
    return 223;
  positional_scan_number = 0;
  memset(positional_scan_word, 0, sizeof positional_scan_word);
  int positional_scan_result =
      sscanf("word 41", "%2$s %1$d", &positional_scan_number,
             positional_scan_word);
  if (positional_scan_result != 2) return 220;
  if (positional_scan_number != 41) return 221;
  if (strcmp(positional_scan_word, "word") != 0) return 222;
  float narrow_scan_float = 0.0f;
  double narrow_scan_double = 0.0;
  if (sscanf("12.5 -2.25", "%f %lf", &narrow_scan_float,
             &narrow_scan_double) != 2 || narrow_scan_float != 12.5f ||
      narrow_scan_double != -2.25)
    return 304;
  long double narrow_scan_long_double = 0.0L;
  if (sscanf("0x1.8p+1 0x1.4p+2", "%la %Lf", &narrow_scan_double,
             &narrow_scan_long_double) != 2 || narrow_scan_double != 3.0 ||
      narrow_scan_long_double != 5.0L)
    return 313;
  if (setlocale(LC_CTYPE, "C.UTF-8") == NULL) return 267;
  wchar_t narrow_scan_wide = 0;
  if (sscanf("\303\251", "%lc", &narrow_scan_wide) != 1 ||
      narrow_scan_wide != 0xE9)
    return 305;
  wchar_t narrow_scan_wide_word[8] = {0};
  int narrow_scan_wide_result =
      sscanf("\303\251x rest", "%ls", narrow_scan_wide_word);
  if (narrow_scan_wide_result != 1) return 307;
  if (narrow_scan_wide_word[0] != 0xE9) return 308;
  if (narrow_scan_wide_word[1] != L'x') return 309;
  if (narrow_scan_wide_word[2] != 0) return 310;
  int wide_scanset_result = wide_scanset_smoke();
  if (wide_scanset_result != 0) return 314 + wide_scanset_result;
  char scan_bytes[] = "alpha \303\251Zrest";
  FILE *scan_stream = fmemopen(scan_bytes, sizeof scan_bytes - 1, "r");
  char narrow_word[8] = {0};
  wchar_t wide_pair[4] = {0};
  int scan_count = -1;
  if (scan_stream == NULL || fwide(scan_stream, 1) <= 0 ||
      fwscanf(scan_stream, L"%5s %2lc%n", narrow_word, wide_pair,
              &scan_count) != 2 || strcmp(narrow_word, "alpha") != 0 ||
      wide_pair[0] != 0xE9 || wide_pair[1] != L'Z' || wide_pair[2] != 0 ||
      scan_count != 8 || fclose(scan_stream) != 0)
    return 294;
  char scanset_bytes[] = "xyz123!";
  scan_stream = fmemopen(scanset_bytes, sizeof scanset_bytes - 1, "r");
  memset(narrow_word, 0, sizeof narrow_word);
  scan_count = -1;
  if (scan_stream == NULL || fwide(scan_stream, 1) <= 0 ||
      fwscanf(scan_stream, L"%3[a-z]%n", narrow_word, &scan_count) != 1 ||
      strcmp(narrow_word, "xyz") != 0 || scan_count != 3 ||
      fgetwc(scan_stream) != L'1' || fclose(scan_stream) != 0)
    return 295;
  char integer_scan_bytes[] = "42 -17 0x2a 077";
  scan_stream = fmemopen(integer_scan_bytes, sizeof integer_scan_bytes - 1, "r");
  int signed_decimal = 0, signed_auto = 0, signed_octal = 0;
  unsigned int unsigned_hex = 0;
  if (scan_stream == NULL || fwide(scan_stream, 1) <= 0 ||
      fwscanf(scan_stream, L"%d %i %x %i", &signed_decimal, &signed_auto,
              &unsigned_hex, &signed_octal) != 4 || signed_decimal != 42 ||
      signed_auto != -17 || unsigned_hex != 42 || signed_octal != 63 ||
      fclose(scan_stream) != 0)
    return 296;
  char float_scan_bytes[] = "12.5 -2.25 3e2";
  scan_stream = fmemopen(float_scan_bytes, sizeof float_scan_bytes - 1, "r");
  float scanned_float = 0;
  double scanned_double = 0;
  long double scanned_long_double = 0;
  if (scan_stream == NULL || fwide(scan_stream, 1) <= 0 ||
      fwscanf(scan_stream, L"%f %lf %Le", &scanned_float, &scanned_double,
              &scanned_long_double) != 3 || scanned_float != 12.5f ||
      scanned_double != -2.25 || scanned_long_double != 300.0L ||
      fclose(scan_stream) != 0)
    return 297;
  char hex_float_scan_bytes[] = "0x1.8p3";
  scan_stream = fmemopen(hex_float_scan_bytes, sizeof hex_float_scan_bytes - 1, "r");
  scanned_double = 0;
  if (scan_stream == NULL || fwide(scan_stream, 1) <= 0 ||
      fwscanf(scan_stream, L"%la", &scanned_double) != 1 ||
      scanned_double != 12.0 || fclose(scan_stream) != 0)
    return 299;
  int wide_string_integer = 0;
  wchar_t wide_string_word[16] = {0};
  if (swscanf(L"27 sample", L"%d %15ls", &wide_string_integer,
              wide_string_word) != 2 || wide_string_integer != 27 ||
      wcscmp(wide_string_word, L"sample") != 0)
    return 298;
  wide_string_integer = 0;
  if (swscanf(L"sample 27", L"%2$ls %1$d", &wide_string_integer,
              wide_string_word) != 2 || wide_string_integer != 27 ||
      wcscmp(wide_string_word, L"sample") != 0)
    return 326;
  wchar_t *allocated_wide_word = NULL;
  int allocated_wide_result = swscanf(L"allocated tail", L"%mls",
                                      &allocated_wide_word);
  if (allocated_wide_result != 1) return 327;
  if (allocated_wide_word == NULL) return 329;
  if (wcscmp(allocated_wide_word, L"allocated") != 0) return 330;
  free(allocated_wide_word);
  allocated_wide_word = NULL;
  int allocated_wide_scanset_result = swscanf(L"wide123", L"%ml[a-z]",
                                                &allocated_wide_word);
  if (allocated_wide_scanset_result != 1) return 328;
  if (allocated_wide_word == NULL) return 331;
  if (wcscmp(allocated_wide_word, L"wide") != 0) return 332;
  free(allocated_wide_word);
  allocated_wide_word = NULL;
  if (swscanf(L"wide", L"%4mlc", &allocated_wide_word) != 1 ||
      allocated_wide_word == NULL || allocated_wide_word[0] != L'w' ||
      allocated_wide_word[1] != L'i' || allocated_wide_word[2] != L'd' ||
      allocated_wide_word[3] != L'e')
    return 333;
  free(allocated_wide_word);
  wchar_t long_wide_scan_input[97];
  for (size_t i = 0; i < 96; ++i) long_wide_scan_input[i] = L'q';
  long_wide_scan_input[96] = L'\0';
  allocated_wide_word = NULL;
  if (swscanf(long_wide_scan_input, L"%mls", &allocated_wide_word) != 1 ||
      allocated_wide_word == NULL || wcslen(allocated_wide_word) != 96)
    return 337;
  free(allocated_wide_word);
  char *allocated_byte_word = NULL;
  if (swscanf(L"caf\u00e9 tail", L"%ms", &allocated_byte_word) != 1 ||
      allocated_byte_word == NULL || strcmp(allocated_byte_word, "caf\303\251") != 0)
    return 334;
  free(allocated_byte_word);
  allocated_byte_word = NULL;
  if (swscanf(L"wide123", L"%m[a-z]", &allocated_byte_word) != 1 ||
      allocated_byte_word == NULL || strcmp(allocated_byte_word, "wide") != 0)
    return 335;
  free(allocated_byte_word);
  allocated_byte_word = NULL;
  if (swscanf(L"wide", L"%4mc", &allocated_byte_word) != 1 ||
      allocated_byte_word == NULL || memcmp(allocated_byte_word, "wide", 4) != 0)
    return 336;
  free(allocated_byte_word);
  wchar_t long_multibyte_scan_input[41];
  for (size_t i = 0; i < 40; ++i) long_multibyte_scan_input[i] = 0xE9;
  long_multibyte_scan_input[40] = L'\0';
  allocated_byte_word = NULL;
  if (swscanf(long_multibyte_scan_input, L"%ms", &allocated_byte_word) != 1 ||
      allocated_byte_word == NULL || strlen(allocated_byte_word) != 80)
    return 338;
  free(allocated_byte_word);
  if (swscanf(L"", L"%d", &wide_string_integer) != EOF ||
      swscanf(L"", L"") != 0)
    return 300;
  char wide_stdio_bytes[32] = {0};
  FILE *wide_stream = fmemopen(wide_stdio_bytes, sizeof wide_stdio_bytes, "w+");
  if (wide_stream == NULL || fwide(wide_stream, 0) != 0) return 268;
  if (putwc(L'A', wide_stream) != L'A') return 271;
  if (putwc_unlocked(0xE9, wide_stream) != 0xE9) return 272;
  if (fputws_unlocked(L"\nB\n", wide_stream) != 0) return 273;
  if (fputws(L"Z", wide_stream) != 0 || fwide(wide_stream, 0) <= 0) return 274;
  if (fflush(wide_stream) != 0) return 275;
  if ((unsigned char)wide_stdio_bytes[0] != 'A' ||
      (unsigned char)wide_stdio_bytes[1] != 0xC3 ||
      (unsigned char)wide_stdio_bytes[2] != 0xA9) return 278;
  if (fclose(wide_stream) != 0) return 279;
  wide_stream = fmemopen(wide_stdio_bytes, sizeof wide_stdio_bytes, "r");
  if (wide_stream == NULL || fwide(wide_stream, 1) <= 0) return 280;
  wint_t first_wide = getwc(wide_stream);
  if (first_wide != L'A') return 281;
  if (getwc_unlocked(wide_stream) != 0xE9 || ungetwc(0xE9, wide_stream) != 0xE9 ||
      fgetwc(wide_stream) != 0xE9) return 277;
  wchar_t wide_line[4] = {0};
  if (fgetws(wide_line, 4, wide_stream) != wide_line) return 282;
  if (wide_line[0] != L'\n' || wide_line[1] != 0) return 283;
  if (fgetws_unlocked(wide_line, 4, wide_stream) != wide_line) return 284;
  if (wide_line[0] != L'B' || wide_line[1] != L'\n' || wide_line[2] != 0) return 285;
  if (fgetwc(wide_stream) != L'Z' || fgetwc(wide_stream) != WEOF ||
      fclose(wide_stream) != 0) return 286;
  wchar_t formatted_wide[32] = {0};
  int wide_count_before = -1;
  if (swprintf(formatted_wide, 32, L"\u00e9:%ls:%d:%c%n!", L"\u00fc", 42,
               0xE9, &wide_count_before) != 9 ||
      wcscmp(formatted_wide, L"\u00e9:\u00fc:42:\u00e9!") != 0 ||
      wide_count_before != 8)
    return 288;
  if (call_vswprintf(formatted_wide, 32, L"x%04d", 7) != 5 ||
      wcscmp(formatted_wide, L"x0007") != 0)
    return 289;
  FILE *formatted_stream = tmpfile();
  int stream_wide_count = -1;
  if (formatted_stream == NULL ||
      fwprintf(formatted_stream, L"%ls:%d%n", L"\u00c4", 42,
               &stream_wide_count) != 4 || stream_wide_count != 4 ||
      call_vfwprintf(formatted_stream, L"/%C:%ls", 0xE9, L"ok") != 5 ||
      fseek(formatted_stream, 0, SEEK_SET) != 0 ||
      fgetws(formatted_wide, 32, formatted_stream) != formatted_wide ||
      wcscmp(formatted_wide, L"\u00c4:42/\u00e9:ok") != 0 ||
      fclose(formatted_stream) != 0)
    return 292;
  wchar_t *wide_memory = NULL;
  size_t wide_memory_size = 99;
  FILE *wide_memory_stream = open_wmemstream(&wide_memory, &wide_memory_size);
  if (wide_memory_stream == NULL || wide_memory != NULL || wide_memory_size != 0 ||
      fwide(wide_memory_stream, 0) <= 0 ||
      fputws(L"wide memory stream", wide_memory_stream) != 0 ||
      fflush(wide_memory_stream) != 0 || wide_memory_size != 18 ||
      wcscmp(wide_memory, L"wide memory stream") != 0 ||
      fseek(wide_memory_stream, 5, SEEK_SET) != 0 ||
      fputwc(L'X', wide_memory_stream) != L'X' ||
      fflush(wide_memory_stream) != 0 || wide_memory_size != 18 ||
      wcscmp(wide_memory, L"wide Xemory stream") != 0 ||
      fclose(wide_memory_stream) != 0 ||
      wcscmp(wide_memory, L"wide Xemory stream") != 0)
    return 293;
  free(wide_memory);
  errno = 0;
  if (swprintf(formatted_wide, 32, L"%Lf", 1.0L) >= 0 || errno != EINVAL)
    return 291;
  wchar_t truncated_wide[3] = {L'X', L'X', L'X'};
  errno = 0;
  if (swprintf(truncated_wide, 3, L"abcd") >= 0 || errno != E2BIG ||
      truncated_wide[2] != 0)
    return 290;
  if (setlocale(LC_CTYPE, "C") == NULL) return 270;
#else
  char stdio_buffer[sizeof "beforeputs\n!UVfputs?"] = {0};
  if (read(pipefd[0], stdio_buffer, sizeof stdio_buffer - 1) != (long)(sizeof stdio_buffer - 1) ||
      memcmp(stdio_buffer, "beforeputs\n!UVfputs?", sizeof stdio_buffer - 1) != 0) return 166;

  FILE *temporary_stream = tmpfile();
  if (temporary_stream == 0 || fputs("stream", temporary_stream) < 0 ||
      fputc('!', temporary_stream) != '!' || fwrite("data", 2, 2, temporary_stream) != 2 ||
      fwrite(0, 0, SIZE_MAX, temporary_stream) != 0 ||
      fread(0, 0, SIZE_MAX, temporary_stream) != 0 ||
      fflush(temporary_stream) != 0 ||
      fseek(temporary_stream, 0, SEEK_SET) != 0) return 179;
#ifndef NELIBC_STANDALONE
  char unlocked_line[8] = {0};
  if (fgets_unlocked(unlocked_line, sizeof unlocked_line, temporary_stream) != unlocked_line ||
      strcmp(unlocked_line, "stream!") != 0 || fseek(temporary_stream, 0, SEEK_SET) != 0) return 182;
#endif
  char temporary_contents[sizeof "stream!data"] = {0};
  if (fgetc(temporary_stream) != 's' || getc(temporary_stream) != 't' ||
      ungetc('t', temporary_stream) != 't' || fgetc(temporary_stream) != 't' ||
      fseek(temporary_stream, 0, SEEK_SET) != 0 ||
      fread(temporary_contents, 2, 5, temporary_stream) != 5 ||
      memcmp(temporary_contents, "stream!dat", 10) != 0) return 179;
  char partial_element[2] = {0};
  if (fread(partial_element, 2, 1, temporary_stream) != 0 ||
      partial_element[0] != 'a' || !feof(temporary_stream) ||
      fseek(temporary_stream, 0, SEEK_SET) != 0 ||
      fread(temporary_contents, 1, sizeof temporary_contents - 1, temporary_stream) !=
          sizeof temporary_contents - 1 ||
      memcmp(temporary_contents, "stream!data", sizeof temporary_contents - 1) != 0 ||
      fgetc(temporary_stream) != EOF || !feof(temporary_stream) ||
      ungetc(EOF, temporary_stream) != EOF || !feof(temporary_stream) ||
      ungetc('!', temporary_stream) != '!' || feof(temporary_stream) ||
      fgetc(temporary_stream) != '!' || feof(temporary_stream) ||
      fclose(temporary_stream) != 0) return 180;

  int threaded_pipe[2];
  if (pipe(threaded_pipe) != 0 || fflush(stdout) != 0) return 169;
  saved_stdout = dup(1);
  if (saved_stdout < 0 || dup2(threaded_pipe[1], 1) != 1) return 170;
  pthread_t writers[2];
  if (pthread_create(&writers[0], 0, stdio_writer, (void *)"A") != 0 ||
      pthread_create(&writers[1], 0, stdio_writer, (void *)"B") != 0) return 171;
  void *writer_result[2] = {0, 0};
  if (pthread_join(writers[0], &writer_result[0]) != 0 ||
      pthread_join(writers[1], &writer_result[1]) != 0 ||
      writer_result[0] != 0 || writer_result[1] != 0 || fflush(stdout) != 0) return 172;
  if (dup2(saved_stdout, 1) != 1 || close(saved_stdout) != 0 || close(threaded_pipe[1]) != 0) return 173;
  char threaded_output[400];
  if (read(threaded_pipe[0], threaded_output, sizeof threaded_output) != (long)sizeof threaded_output) return 174;
  int a_lines = 0;
  int b_lines = 0;
  for (size_t i = 0; i < sizeof threaded_output; i += 2) {
    if (threaded_output[i + 1] != '\n') return 175;
    if (threaded_output[i] == 'A') ++a_lines;
    else if (threaded_output[i] == 'B') ++b_lines;
    else return 176;
  }
  if (a_lines != 100 || b_lines != 100 || close(threaded_pipe[0]) != 0) return 177;
#endif
  if (setlocale(LC_CTYPE, "C.UTF-8") == NULL) return 267;
  if (fnmatch("\303\251", "\303\251", 0) != 0 ||
      fnmatch("?", "\303\251", 0) != 0 ||
      fnmatch("??", "\303\251", 0) != FNM_NOMATCH ||
      fnmatch("*\303\251", "x\303\251", 0) != 0 ||
      fnmatch("[\303\251-\303\252]", "\303\252", 0) != 0 ||
      fnmatch("[[:alpha:]]", "\303\251", 0) != 0 ||
      fnmatch("\303\204", "\303\244", FNM_CASEFOLD) != 0 ||
      fnmatch("?", "\377", 0) != FNM_NOMATCH ||
      fnmatch("?", "\303\251", FNM_PATHNAME) != 0)
    return 340;
  if (setlocale(LC_CTYPE, "C") == NULL) return 270;
  int input_pipe[2];
  if (pipe(input_pipe) != 0 || write(input_pipe[1], "wxyz", 4) != 4 || close(input_pipe[1]) != 0) return 189;
  int saved_stdin = dup(0);
  if (saved_stdin < 0 || dup2(input_pipe[0], 0) != 0 || close(input_pipe[0]) != 0) return 190;
  if (fgetc(stdin) != 'w' || getc(stdin) != 'x' || ungetc('X', stdin) != 'X' ||
      getchar_unlocked() != 'X' || fgetc(stdin) != 'y') return 191;
  if (getchar_unlocked() != 'z' || ungetc('z', stdin) != 'z') return 194;
  char partial_input[2] = {0};
  if (fread(partial_input, 2, 1, stdin) != 0 || partial_input[0] != 'z' ||
      !feof(stdin) || ungetc(EOF, stdin) != EOF || !feof(stdin) ||
      ungetc('!', stdin) != '!' || feof(stdin) || getchar() != '!' ||
      getchar() != EOF || !feof(stdin)) return 193;
#ifdef NELIBC_STANDALONE
  errno = 0;
  if (fseek(stdin, 0, SEEK_SET) != -1 || !ferror(stdin) || errno != ESPIPE) return 207;
  rewind(stdin);
  if (ferror(stdin) || feof(stdin)) return 208;
#endif
  if (dup2(saved_stdin, 0) != 0 || close(saved_stdin) != 0) return 192;
  FILE *reopen_stream = tmpfile();
  if (reopen_stream == NULL) return 270;
  FILE *reopen_identity = reopen_stream;
  reopen_stream = freopen("/dev/null", "w", reopen_stream);
  if (reopen_stream == NULL || reopen_stream != reopen_identity ||
      fputs("reopened", reopen_stream) < 0 || fclose(reopen_stream) != 0) return 271;
  clearerr(stdin);
  if (feof(stdin)) return 194;
  errno = 0;
  if (dup2(pipefd[0], pipefd[0]) != pipefd[0] || dup3(pipefd[0], pipefd[0], 0) != -1 || errno != EINVAL) return 136;
  errno = 0;
  if (lseek(pipefd[0], 0, SEEK_SET) != (off_t)-1 || errno != ESPIPE) return 50;
  errno = 0;
  if (lseek(-1, 0, SEEK_SET) != (off_t)-1 || errno != EBADF) return 51;
  if (close(duplicated_write_fd) != 0 || close(pipefd[0]) != 0 || close(pipefd[1]) != 0) return 48;

  int positioned_result = positioned_io_smoke();
  if (positioned_result != 0) return positioned_result;
#ifdef NELIBC_STANDALONE
  char stdio_path[64];
  make_seek_path(stdio_path);
  size_t stdio_path_length = strlen(stdio_path);
  static const char stdio_suffix[] = ".stdio";
  for (size_t i = 0; i < sizeof stdio_suffix; ++i)
    stdio_path[stdio_path_length + i] = stdio_suffix[i];
  errno = 0;
  FILE *file = fopen(stdio_path, "wx+");
  if (file == 0) return errno == 0 ? 195 : errno;
  char custom_stdio_buffer[64];
  if (setvbuf(file, custom_stdio_buffer, _IOFBF, sizeof custom_stdio_buffer) != 0 ||
      file->user_buffer != (unsigned char *)custom_stdio_buffer ||
      file->buffer_size != sizeof custom_stdio_buffer) return 210;
  if (fwrite("abc", 1, 3, file) != 3 || ftell(file) != 3 || file->write_length != 3) return 201;
  if (memcmp(custom_stdio_buffer, "abc", 3) != 0 || setvbuf(file, NULL, 99, 0) == 0) return 211;
  if (fflush(NULL) != 0 || file->write_length != 0) return 206;
  if (setvbuf(file, NULL, _IOFBF, sizeof custom_stdio_buffer) != 0) return 212;
  setbuf(file, NULL);
  if (file->user_buffer != NULL ||
      file->buffer_size != 1 || (file->flags & NELIBC_FILE_UNBUFFERED) == 0) return 212;
  char positioned_contents[4] = {0};
  if (setvbuf(file, NULL, _IOFBF, BUFSIZ) != 0 || fseek(file, 0, SEEK_SET) != 0) return 222;
  fpos_t saved_position;
  if (fgetc(file) != 'a' || fgetpos(file, &saved_position) != 0 || fgetc(file) != 'b' ||
      fsetpos(file, &saved_position) != 0 || fgetc(file) != 'b' || ftell(file) != 2) return 223;
  if (fseek(file, 0, SEEK_SET) != 0 || fread(positioned_contents, 1, 1, file) != 1 ||
      ftell(file) != 1 || fflush(NULL) != 0 || file->read_length != 0 ||
      ftell(file) != 1 || fgetc(file) != 'b') return 209;
  if (fseek(file, 0, SEEK_SET) != 0 || fread(positioned_contents, 1, 3, file) != 3 ||
      memcmp(positioned_contents, "abc", 3) != 0 || fseek(file, -1, SEEK_END) != 0 ||
      ftell(file) != 2 || fgetc(file) != 'c') return 204;
  rewind(file);
  if (ftell(file) != 0 || fgetc(file) != 'a') return 205;
  if (fflush(file) != 0) return 202;
  if (fclose(file) != 0 || fflush(NULL) != 0) return 203;
  size_t format_path_length = strlen(stdio_path);
  static const char format_suffix[] = ".fmt";
  for (size_t i = 0; i < sizeof format_suffix; ++i)
    stdio_path[format_path_length + i] = format_suffix[i];
  errno = 0;
  file = fopen(stdio_path, "wx+");
  if (file == 0) return 224;
  int formatted = fprintf(file, "%#08x %s %c %% %ld\n%.2f %+.1f %.2e %.3g %.0a %A %a", 42u, "ok", '!', -12L, 1.256, -2.34, 12.5, 12345.0, 3.5, 1.5, 0x1p-1074);
  int variadic = smoke_vfprintf(file, "%d:%u", -7, 42u);
  if (formatted != 87) return formatted < 0 ? 225 : formatted;
  if (variadic != 5) return 227;
  if (fflush(file) != 0 || fseek(file, 0, SEEK_SET) != 0) return 228;
  char formatted_contents[128] = {0};
  if (fread(formatted_contents, 1, 92, file) != 92 ||
      memcmp(formatted_contents, "0x00002a ok ! % -12\n1.26 -2.3 1.25e+01 1.23e+04 0x1p+2 0X1.8P+0 0x0.0000000000001p-1022-7:42", 92) != 0)
    return 230;
  if (fclose(file) != 0) return 231;
  if (unlinkat(AT_FDCWD, stdio_path, 0) != 0) return 232;
  char truncated_format[5] = {'X', 'X', 'X', 'X', 'X'};
  char variadic_format[8] = {0};
  char zero_size_format[1] = {'X'};
  if (snprintf(truncated_format, sizeof truncated_format, "%.2f:%d", 3.5, 7) != 6 ||
      memcmp(truncated_format, "3.50", 4) != 0 || truncated_format[4] != 0 ||
      smoke_vsnprintf(variadic_format, sizeof variadic_format, "%s:%u", "ok", 9u) != 4 ||
      strcmp(variadic_format, "ok:9") != 0 ||
      snprintf(NULL, 0, "hello") != 5 ||
      snprintf(zero_size_format, 0, "hello") != 5 || zero_size_format[0] != 'X') return 229;
  char *allocated_format = NULL;
  if (asprintf(&allocated_format, "%s:%d", "allocated", 7) != 11 ||
      strcmp(allocated_format, "allocated:7") != 0) return 250;
  free(allocated_format);
  allocated_format = NULL;
  if (smoke_vasprintf(&allocated_format, "%#x:%s", 42u, "done") != 9 ||
      strcmp(allocated_format, "0x2a:done") != 0) return 251;
  free(allocated_format);
  int output_pipe[2];
  char descriptor_output[9] = {0};
  if (pipe(output_pipe) != 0 || dprintf(output_pipe[1], "fd:%d", 17) != 5 ||
      smoke_vdprintf(output_pipe[1], ":%u", 23u) != 3 || close(output_pipe[1]) != 0 ||
      read(output_pipe[0], descriptor_output, 8) != 8 || strcmp(descriptor_output, "fd:17:23") != 0 ||
      close(output_pipe[0]) != 0) return 230;
  int short_write_pipe[2];
  if (pipe(short_write_pipe) != 0) return 252;
  FILE *short_write_stream = fdopen(short_write_pipe[1], "w");
  if (short_write_stream == NULL || setvbuf(short_write_stream, NULL, _IONBF, 0) != 0) return 253;
  nelibc_test_short_write_on_fd(short_write_pipe[1], 1);
  errno = 0;
  int short_write_result = fprintf(short_write_stream, "%.40g", 1.2345678901234567);
  nelibc_test_disable_short_write();
  if (short_write_result != -1 || !ferror(short_write_stream) || errno != EIO) return 254;
  if (fclose(short_write_stream) != 0 || close(short_write_pipe[0]) != 0) return 255;
  if (pipe(short_write_pipe) != 0) return 256;
  short_write_stream = fdopen(short_write_pipe[1], "w");
  if (short_write_stream == NULL || setvbuf(short_write_stream, NULL, _IONBF, 0) != 0) return 257;
  nelibc_test_fail_next_mmap();
  errno = 0;
  if (fprintf(short_write_stream, "%.100000g", 1.2345678901234567) != -1 || errno != ENOMEM ||
      ferror(short_write_stream))
    return 258;
  if (fprintf(short_write_stream, "ok") != 2) return 261;
  if (ferror(short_write_stream)) return 266;
  (void)fclose(short_write_stream);
  char allocation_failure_output[3] = {0};
  if (read(short_write_pipe[0], allocation_failure_output, 2) != 2 ||
      memcmp(allocation_failure_output, "ok", 2) != 0) return 260;
  if (close(short_write_pipe[0]) != 0) return 263;
  file = fopen(stdio_path, "wx+");
  if (file == 0) return 231;
  for (int i = 0; i < 150; ++i) if (fputc('a', file) != 'a') return 232;
  if (fputc('\n', file) != '\n' || fputs("tailx", file) != 0 || fclose(file) != 0) return 233;
  file = fopen(stdio_path, "r");
  char *line_buffer = NULL;
  size_t line_capacity = 0;
  long line_length = getline(&line_buffer, &line_capacity, file);
  if (file == 0 || line_length != 151) return 235;
  if (line_capacity < 152 || line_buffer[150] != '\n' || line_buffer[151] != 0) return 236;
  if (getdelim(&line_buffer, &line_capacity, 'l', file) != 4 || strcmp(line_buffer, "tail") != 0) return 237;
  if (getline(&line_buffer, &line_capacity, file) != 1 || strcmp(line_buffer, "x") != 0) return 238;
  if (getline(&line_buffer, &line_capacity, file) != -1 || !feof(file)) return 239;
  if (fclose(file) != 0 || unlinkat(AT_FDCWD, stdio_path, 0) != 0) return 240;
  free(line_buffer);
  stdio_path[format_path_length] = 0;
  errno = 0;
  if (fopen(stdio_path, "wx") != 0 || errno != EEXIST) return 196;
  errno = 0;
  if (fopen(stdio_path, "q") != 0 || errno != EINVAL) return 197;
  file = fopen(stdio_path, "r");
  char file_contents[6] = {0};
  if (file == 0 || fgets(file_contents, 0, file) != NULL ||
      fgets(file_contents, 3, file) != file_contents || strcmp(file_contents, "ab") != 0 ||
      fgets(file_contents, sizeof file_contents, file) != file_contents ||
      strcmp(file_contents, "c") != 0 || !feof(file) ||
      fgets(file_contents, sizeof file_contents, file) != NULL || fclose(file) != 0) return 198;
  file = fopen(stdio_path, "a");
  if (file == 0 || setvbuf(file, NULL, _IOLBF, 0) != 0 || fputs("d", file) != 0 ||
      file->write_length != 1 || fputc('\n', file) != '\n' || file->write_length != 0 ||
      fclose(file) != 0) return 199;
  file = fopen(stdio_path, "r");
  if (file == 0 || fgets(file_contents, sizeof file_contents, file) != file_contents ||
      strcmp(file_contents, "abcd\n") != 0 ||
      fgets(file_contents, sizeof file_contents, file) != NULL ||
      fclose(file) != 0 || unlinkat(AT_FDCWD, stdio_path, 0) != 0) return 200;
  file = fopen(stdio_path, "wx+");
  if (file == NULL || fputs("old", file) != 0) return 264;
  FILE *reopened_stream = file;
  file = freopen(stdio_path, "w+", file);
  if (file == NULL || file != reopened_stream || ftell(file) != 0 ||
      fgetc(file) != EOF || fputs("new", file) != 0 || fclose(file) != 0) return 265;
  file = fopen(stdio_path, "r");
  char reopened_contents[4] = {0};
  if (file == NULL || fread(reopened_contents, 1, 3, file) != 3 ||
      memcmp(reopened_contents, "new", 3) != 0 || fclose(file) != 0 ||
      unlinkat(AT_FDCWD, stdio_path, 0) != 0) return 266;
  file = fopen(stdio_path, "w");
  if (file == NULL) return 267;
  errno = 0;
  if (freopen("/nelibc-no-such-directory/file", "r", file) != NULL || errno != ENOENT)
    return 268;
  if (unlinkat(AT_FDCWD, stdio_path, 0) != 0) return 269;
  char rename_source[64];
  char rename_destination[64];
  make_seek_path(rename_source);
  make_seek_path(rename_destination);
  size_t rename_source_length = strlen(rename_source);
  size_t rename_destination_length = strlen(rename_destination);
  static const char rename_source_suffix[] = ".rename-src";
  static const char rename_destination_suffix[] = ".rename-dst";
  for (size_t i = 0; i < sizeof rename_source_suffix; ++i)
    rename_source[rename_source_length + i] = rename_source_suffix[i];
  for (size_t i = 0; i < sizeof rename_destination_suffix; ++i)
    rename_destination[rename_destination_length + i] = rename_destination_suffix[i];
  FILE *rename_file = fopen(rename_source, "wx");
  if (rename_file == NULL || fputs("source", rename_file) != 0 || fclose(rename_file) != 0) return 241;
  rename_file = fopen(rename_destination, "wx");
  if (rename_file == NULL || fputs("old", rename_file) != 0 || fclose(rename_file) != 0 ||
      rename(rename_source, rename_destination) != 0) return 242;
  errno = 0;
  if (rename(rename_source, rename_destination) != -1 || errno != ENOENT) return 321;
  rename_file = fopen(rename_destination, "r");
  char renamed_contents[8] = {0};
  if (rename_file == NULL || fread(renamed_contents, 1, 6, rename_file) != 6 ||
      memcmp(renamed_contents, "source", 6) != 0 || fclose(rename_file) != 0 ||
      remove(rename_source) == 0 || remove(rename_destination) != 0) return 243;
  char remove_directory[64];
  make_seek_path(remove_directory);
  size_t remove_directory_length = strlen(remove_directory);
  static const char remove_directory_suffix[] = ".remove-dir";
  for (size_t i = 0; i < sizeof remove_directory_suffix; ++i)
    remove_directory[remove_directory_length + i] = remove_directory_suffix[i];
  if (mkdirat(AT_FDCWD, remove_directory, 0700) != 0 || remove(remove_directory) != 0) return 244;
  char fifo_path[64];
  char fifo_alias[64];
  make_seek_path(fifo_path);
  size_t fifo_path_length = strlen(fifo_path);
  for (size_t i = 0; i < fifo_path_length; ++i) fifo_alias[i] = fifo_path[i];
  static const char fifo_suffix[] = ".fifo";
  static const char fifo_at_suffix[] = ".fifo-at";
  for (size_t i = 0; i < sizeof fifo_suffix; ++i) fifo_alias[fifo_path_length + i] = fifo_suffix[i];
  for (size_t i = 0; i < sizeof fifo_at_suffix; ++i) fifo_path[fifo_path_length + i] = fifo_at_suffix[i];
  struct stat fifo_status;
  mode_t fifo_original_mask = umask(0);
  int fifo_at_result = mkfifoat(AT_FDCWD, fifo_path, 0600);
  int fifo_at_stat_result = fifo_at_result == 0 ? stat(fifo_path, &fifo_status) : -1;
  mode_t fifo_at_mode = fifo_at_stat_result == 0 ? fifo_status.st_mode : 0;
  int fifo_result = mkfifo(fifo_alias, 0620);
  int fifo_stat_result = fifo_result == 0 ? lstat(fifo_alias, &fifo_status) : -1;
  mode_t fifo_mode = fifo_stat_result == 0 ? fifo_status.st_mode : 0;
  umask(fifo_original_mask);
  if (fifo_at_result != 0 || fifo_at_stat_result != 0 || !S_ISFIFO(fifo_at_mode) ||
      (fifo_at_mode & 0777) != 0600) return 342;
  if (fifo_result != 0 || fifo_stat_result != 0 || !S_ISFIFO(fifo_mode) ||
      (fifo_mode & 0777) != 0620) return 343;
  errno = 0;
  if (mkfifo(fifo_alias, 0600) != -1 || errno != EEXIST) return 344;
  if (unlink(fifo_path) != 0 || unlink(fifo_alias) != 0) return 345;
  if (mknod(fifo_alias, S_IFIFO | 0600, 0) != 0 || lstat(fifo_alias, &fifo_status) != 0 ||
      !S_ISFIFO(fifo_status.st_mode) || (fifo_status.st_mode & 0777) != 0600) return 346;
  if (unlink(fifo_alias) != 0) return 347;
  if (mknodat(AT_FDCWD, fifo_path, S_IFIFO | 0600, 0) != 0 || stat(fifo_path, &fifo_status) != 0 ||
      !S_ISFIFO(fifo_status.st_mode) || (fifo_status.st_mode & 0777) != 0600) return 348;
  errno = 0;
  if (mknodat(-1, "missing-nelibc-mknodat-path", S_IFIFO | 0600, 0) != -1 || errno != EBADF) return 349;
  if (unlink(fifo_path) != 0) return 350;
  char timestamp_path[64];
  make_seek_path(timestamp_path);
  size_t timestamp_path_length = strlen(timestamp_path);
  static const char timestamp_suffix[] = ".utimens";
  for (size_t i = 0; i < sizeof timestamp_suffix; ++i)
    timestamp_path[timestamp_path_length + i] = timestamp_suffix[i];
  int timestamp_fd = openat(AT_FDCWD, timestamp_path, O_CREAT | O_EXCL | O_RDWR, 0600);
  if (timestamp_fd < 0) return 388;
  if (fchown(timestamp_fd, getuid(), getgid()) != 0) return 395;
  errno = 0;
  if (fchown(-1, getuid(), getgid()) != -1 || errno != EBADF) return 396;
  if (futimens(timestamp_fd, NULL) != 0) return 397;
  errno = 0;
  if (futimens(-1, NULL) != -1 || errno != EBADF) return 398;
  if (utimensat(AT_FDCWD, timestamp_path, NULL, 0) != 0) return 389;
  struct stat timestamp_status;
  if (stat(timestamp_path, &timestamp_status) != 0 || timestamp_status.st_mtim.tv_sec <= 0 ||
      close(timestamp_fd) != 0) return 390;
  struct timeval legacy_timestamps[2] = {
      {.tv_sec = 1000000000, .tv_usec = 123456},
      {.tv_sec = 1000000001, .tv_usec = 654321}};
  if (utimes(timestamp_path, legacy_timestamps) != 0 ||
      stat(timestamp_path, &timestamp_status) != 0 ||
      timestamp_status.st_atim.tv_sec != 1000000000 ||
      timestamp_status.st_atim.tv_nsec != 123456000 ||
      timestamp_status.st_mtim.tv_sec != 1000000001 ||
      timestamp_status.st_mtim.tv_nsec != 654321000)
    return 403;
  legacy_timestamps[0].tv_usec = 1000000;
  errno = 0;
  if (utimes(timestamp_path, legacy_timestamps) != -1 || errno != EINVAL ||
      utimes(timestamp_path, NULL) != 0)
    return 404;
  int timestamp_fd_for_futimes = open(timestamp_path, O_RDWR);
  if (timestamp_fd_for_futimes < 0) return 405;
  legacy_timestamps[0].tv_usec = 234567;
  legacy_timestamps[1].tv_usec = 765432;
  if (futimes(timestamp_fd_for_futimes, legacy_timestamps) != 0 ||
      fstat(timestamp_fd_for_futimes, &timestamp_status) != 0 ||
      timestamp_status.st_atim.tv_sec != 1000000000 ||
      timestamp_status.st_atim.tv_nsec != 234567000 ||
      timestamp_status.st_mtim.tv_sec != 1000000001 ||
      timestamp_status.st_mtim.tv_nsec != 765432000)
    return 406;
  legacy_timestamps[1].tv_usec = -1;
  errno = 0;
  if (futimes(timestamp_fd_for_futimes, legacy_timestamps) != -1 || errno != EINVAL ||
      futimes(timestamp_fd_for_futimes, NULL) != 0 || close(timestamp_fd_for_futimes) != 0)
    return 407;
  const char *timestamp_name = strrchr(timestamp_path, '/') + 1;
  int timestamp_directory = open("/tmp", O_RDONLY | O_DIRECTORY);
  if (timestamp_directory < 0) return 408;
  legacy_timestamps[0].tv_usec = 345678;
  legacy_timestamps[1].tv_usec = 876543;
  if (futimesat(timestamp_directory, timestamp_name, legacy_timestamps) != 0 ||
      stat(timestamp_path, &timestamp_status) != 0 ||
      timestamp_status.st_atim.tv_sec != 1000000000 ||
      timestamp_status.st_atim.tv_nsec != 345678000 ||
      timestamp_status.st_mtim.tv_sec != 1000000001 ||
      timestamp_status.st_mtim.tv_nsec != 876543000)
    return 409;
  legacy_timestamps[0].tv_usec = 1000000;
  errno = 0;
  if (futimesat(timestamp_directory, timestamp_name, legacy_timestamps) != -1 || errno != EINVAL ||
      futimesat(timestamp_directory, timestamp_name, NULL) != 0 || close(timestamp_directory) != 0 ||
      stat(timestamp_path, &timestamp_status) != 0)
    return 410;
  struct stat target_status_before_lutimes = timestamp_status;
  char timestamp_link_path[96];
  if (snprintf(timestamp_link_path, sizeof timestamp_link_path, "%s.link", timestamp_path) < 0 ||
      symlink(timestamp_path, timestamp_link_path) != 0)
    return 411;
  struct timeval link_times[2] = {
      {.tv_sec = 1000000008, .tv_usec = 333333},
      {.tv_sec = 1000000009, .tv_usec = 444444}};
  struct stat link_status;
  if (lutimes(timestamp_link_path, link_times) != 0 ||
      lstat(timestamp_link_path, &link_status) != 0 ||
      stat(timestamp_path, &timestamp_status) != 0 || !S_ISLNK(link_status.st_mode) ||
      link_status.st_atim.tv_sec != 1000000008 || link_status.st_atim.tv_nsec != 333333000 ||
      link_status.st_mtim.tv_sec != 1000000009 || link_status.st_mtim.tv_nsec != 444444000 ||
      timestamp_status.st_atim.tv_sec != target_status_before_lutimes.st_atim.tv_sec ||
      timestamp_status.st_atim.tv_nsec != target_status_before_lutimes.st_atim.tv_nsec ||
      timestamp_status.st_mtim.tv_sec != target_status_before_lutimes.st_mtim.tv_sec ||
      timestamp_status.st_mtim.tv_nsec != target_status_before_lutimes.st_mtim.tv_nsec)
    return 412;
  link_times[0].tv_usec = -1;
  errno = 0;
  if (lutimes(timestamp_link_path, link_times) != -1 || errno != EINVAL ||
      lutimes(timestamp_link_path, NULL) != 0 || unlink(timestamp_link_path) != 0)
    return 413;
  struct utimbuf seconds_timestamps = {.actime = 1000000010, .modtime = 1000000011};
  if (utime(timestamp_path, &seconds_timestamps) != 0 || stat(timestamp_path, &timestamp_status) != 0 ||
      timestamp_status.st_atim.tv_sec != 1000000010 || timestamp_status.st_atim.tv_nsec != 0 ||
      timestamp_status.st_mtim.tv_sec != 1000000011 || timestamp_status.st_mtim.tv_nsec != 0 ||
      utime(timestamp_path, NULL) != 0)
    return 414;
  errno = 0;
  if (utimensat(AT_FDCWD, timestamp_path, NULL, 0x40000000) != -1 || errno != EINVAL) return 391;
  if (unlink(timestamp_path) != 0) return 392;
  char allocation_path[64];
  make_seek_path(allocation_path);
  size_t allocation_path_length = strlen(allocation_path);
  static const char allocation_suffix[] = ".allocate";
  for (size_t i = 0; i < sizeof allocation_suffix; ++i)
    allocation_path[allocation_path_length + i] = allocation_suffix[i];
  int allocation_fd = openat(AT_FDCWD, allocation_path, O_CREAT | O_EXCL | O_RDWR, 0600);
  if (allocation_fd < 0) return 393;
  if (fallocate(allocation_fd, 0, 0, 4096) != 0) return 394;
  struct stat allocation_status;
  if (fstat(allocation_fd, &allocation_status) != 0 || allocation_status.st_size != 4096) return 395;
  if (posix_fallocate(allocation_fd, 4096, 4096) != 0 ||
      fstat(allocation_fd, &allocation_status) != 0 || allocation_status.st_size != 8192) return 396;
  if (posix_fadvise(allocation_fd, 0, 8192, POSIX_FADV_NORMAL) != 0 ||
      readahead(allocation_fd, 0, 8192) != 0) return 400;
  errno = 42;
  if (posix_fadvise(-1, 0, 4096, POSIX_FADV_NORMAL) != EBADF || errno != 42) return 401;
  errno = 0;
  if (readahead(-1, 0, 4096) != -1 || errno != EBADF) return 402;
  static const char xattr_name[] = "user.nelibc-smoke";
  static const char xattr_value[] = "ok";
  char xattr_readback[sizeof xattr_value];
  char xattr_names[256];
  if (fsetxattr(allocation_fd, xattr_name, xattr_value, sizeof xattr_value, XATTR_CREATE) != 0 ||
      fgetxattr(allocation_fd, xattr_name, xattr_readback, sizeof xattr_readback) != sizeof xattr_value ||
      memcmp(xattr_readback, xattr_value, sizeof xattr_value) != 0) return 423;
  if (fgetxattr(allocation_fd, xattr_name, NULL, 0) != sizeof xattr_value) return 442;
  errno = 0;
  if (fsetxattr(allocation_fd, xattr_name, xattr_value, sizeof xattr_value, XATTR_CREATE) != -1 || errno != EEXIST) return 443;
  if (fsetxattr(allocation_fd, xattr_name, xattr_value, sizeof xattr_value, XATTR_REPLACE) != 0) return 444;
  ssize_t xattr_names_length = flistxattr(allocation_fd, xattr_names, sizeof xattr_names);
  int xattr_name_found = 0;
  for (size_t xattr_name_position = 0; xattr_names_length >= 0 && xattr_name_position < (size_t)xattr_names_length;) {
    size_t listed_name_length = strlen(xattr_names + xattr_name_position);
    if (strcmp(xattr_names + xattr_name_position, xattr_name) == 0) xattr_name_found = 1;
    xattr_name_position += listed_name_length + 1;
  }
  if (xattr_names_length < 0 || flistxattr(allocation_fd, NULL, 0) != xattr_names_length ||
      !xattr_name_found || fremovexattr(allocation_fd, xattr_name) != 0) return 423;
  errno = 0;
  if (fgetxattr(allocation_fd, xattr_name, xattr_readback, sizeof xattr_readback) != -1 || errno != ENODATA) return 427;
  errno = 0;
  if (fremovexattr(allocation_fd, xattr_name) != -1 || errno != ENODATA) return 428;
  errno = 0;
  if (fsetxattr(-1, xattr_name, xattr_value, sizeof xattr_value, 0) != -1 || errno != EBADF) return 424;
  errno = 0;
  if (fgetxattr(-1, xattr_name, xattr_readback, sizeof xattr_readback) != -1 || errno != EBADF) return 426;
  errno = 0;
  if (flistxattr(-1, xattr_names, sizeof xattr_names) != -1 || errno != EBADF) return 429;
  errno = 0;
  if (fremovexattr(-1, xattr_name) != -1 || errno != EBADF) return 425;
  static const char path_xattr_name[] = "user.nelibc-path-smoke";
  if (setxattr(allocation_path, path_xattr_name, xattr_value, sizeof xattr_value, XATTR_CREATE) != 0 ||
      getxattr(allocation_path, path_xattr_name, xattr_readback, sizeof xattr_readback) != sizeof xattr_value ||
      memcmp(xattr_readback, xattr_value, sizeof xattr_value) != 0) return 430;
  if (getxattr(allocation_path, path_xattr_name, NULL, 0) != sizeof xattr_value) return 445;
  errno = 0;
  if (setxattr(allocation_path, path_xattr_name, xattr_value, sizeof xattr_value, XATTR_CREATE) != -1 || errno != EEXIST) return 446;
  if (setxattr(allocation_path, path_xattr_name, xattr_value, sizeof xattr_value, XATTR_REPLACE) != 0) return 447;
  ssize_t path_xattr_names_length = listxattr(allocation_path, xattr_names, sizeof xattr_names);
  int path_xattr_name_found = 0;
  for (size_t path_xattr_name_position = 0; path_xattr_names_length >= 0 && path_xattr_name_position < (size_t)path_xattr_names_length;) {
    size_t listed_name_length = strlen(xattr_names + path_xattr_name_position);
    if (strcmp(xattr_names + path_xattr_name_position, path_xattr_name) == 0) path_xattr_name_found = 1;
    path_xattr_name_position += listed_name_length + 1;
  }
  if (path_xattr_names_length < 0 || listxattr(allocation_path, NULL, 0) != path_xattr_names_length ||
      !path_xattr_name_found || removexattr(allocation_path, path_xattr_name) != 0) return 431;
  errno = 0;
  if (getxattr(allocation_path, path_xattr_name, xattr_readback, sizeof xattr_readback) != -1 || errno != ENODATA) return 432;
  errno = 0;
  if (removexattr(allocation_path, path_xattr_name) != -1 || errno != ENODATA) return 433;
  errno = 0;
  if (setxattr("missing-nelibc-xattr-path", path_xattr_name, xattr_value, sizeof xattr_value, 0) != -1 || errno != ENOENT) return 434;
  if (setxattr(allocation_path, path_xattr_name, xattr_value, sizeof xattr_value, XATTR_CREATE) != 0) return 435;
  char xattr_symlink_path[96];
  size_t xattr_symlink_path_length = strlen(allocation_path);
  memcpy(xattr_symlink_path, allocation_path, xattr_symlink_path_length);
  static const char xattr_symlink_suffix[] = ".link";
  memcpy(xattr_symlink_path + xattr_symlink_path_length, xattr_symlink_suffix, sizeof xattr_symlink_suffix);
  if (symlink(allocation_path, xattr_symlink_path) != 0 ||
      getxattr(xattr_symlink_path, path_xattr_name, xattr_readback, sizeof xattr_readback) != sizeof xattr_value ||
      memcmp(xattr_readback, xattr_value, sizeof xattr_value) != 0) return 436;
  errno = 0;
  if (lsetxattr(xattr_symlink_path, path_xattr_name, xattr_value, sizeof xattr_value, XATTR_CREATE) != -1 || errno != EPERM) return 437;
  errno = 0;
  if (lgetxattr(xattr_symlink_path, path_xattr_name, xattr_readback, sizeof xattr_readback) != -1 || errno != ENODATA) return 438;
  errno = 0;
  ssize_t symlink_xattr_names_length = llistxattr(xattr_symlink_path, xattr_names, sizeof xattr_names);
  if (symlink_xattr_names_length >= 0 &&
      llistxattr(xattr_symlink_path, NULL, 0) != symlink_xattr_names_length) return 448;
  int symlink_has_target_user_xattr = 0;
  for (size_t symlink_xattr_position = 0; symlink_xattr_names_length >= 0 && symlink_xattr_position < (size_t)symlink_xattr_names_length;) {
    size_t listed_name_length = strlen(xattr_names + symlink_xattr_position);
    if (strcmp(xattr_names + symlink_xattr_position, path_xattr_name) == 0) symlink_has_target_user_xattr = 1;
    symlink_xattr_position += listed_name_length + 1;
  }
  if (symlink_xattr_names_length < 0 || symlink_has_target_user_xattr) return 439;
  errno = 0;
  if (lremovexattr(xattr_symlink_path, path_xattr_name) != -1 || (errno != EPERM && errno != ENODATA)) return 440;
  if (unlink(xattr_symlink_path) != 0 || removexattr(allocation_path, path_xattr_name) != 0) return 441;
  static const char xattrat_name[] = "user.nelibc-xattrat-smoke";
  struct nelibc_xattr_args xattrat_args = {
    .value = (uint64_t)(uintptr_t)xattr_value,
    .size = sizeof xattr_value,
    .flags = XATTR_CREATE,
  };
  if (setxattrat(allocation_fd, "", AT_EMPTY_PATH, xattrat_name, &xattrat_args, sizeof xattrat_args) != 0) {
    if (errno != ENOSYS) return 449;
  } else {
    xattrat_args.value = (uint64_t)(uintptr_t)xattr_readback;
    xattrat_args.size = sizeof xattr_readback;
    xattrat_args.flags = 0;
    if (getxattrat(allocation_fd, "", AT_EMPTY_PATH, xattrat_name, &xattrat_args, sizeof xattrat_args) != sizeof xattr_value ||
        memcmp(xattr_readback, xattr_value, sizeof xattr_value) != 0) return 450;
    ssize_t xattrat_names_length = listxattrat(allocation_fd, "", AT_EMPTY_PATH, xattr_names, sizeof xattr_names);
    int xattrat_name_found = 0;
    for (size_t xattrat_name_position = 0; xattrat_names_length >= 0 && xattrat_name_position < (size_t)xattrat_names_length;) {
      size_t listed_name_length = strlen(xattr_names + xattrat_name_position);
      if (strcmp(xattr_names + xattrat_name_position, xattrat_name) == 0) xattrat_name_found = 1;
      xattrat_name_position += listed_name_length + 1;
    }
    if (xattrat_names_length < 0 || !xattrat_name_found ||
        removexattrat(allocation_fd, "", AT_EMPTY_PATH, xattrat_name) != 0) return 451;
    errno = 0;
    if (getxattrat(allocation_fd, "", AT_EMPTY_PATH, xattrat_name, &xattrat_args, sizeof xattrat_args) != -1 || errno != ENODATA) return 452;
  }
  _Static_assert(sizeof(struct nelibc_file_attr) == NELIBC_FILE_ATTR_SIZE, "Linux file_attr ABI size");
  struct nelibc_file_attr file_attributes;
  errno = 0;
  if (file_getattr(AT_FDCWD, allocation_path, &file_attributes, sizeof file_attributes, 0) != 0) {
    if (errno != ENOSYS && errno != EOPNOTSUPP && errno != ENOTSUP) return 453;
  } else {
    if (file_setattr(AT_FDCWD, allocation_path, &file_attributes, sizeof file_attributes, 0) != 0 &&
        errno != EOPNOTSUPP && errno != ENOTSUP) return 454;
  }
  _Static_assert(sizeof(struct nelibc_mount_attr) == NELIBC_MOUNT_ATTR_SIZE, "Linux mount_attr ABI size");
  _Static_assert(sizeof(struct nelibc_mnt_id_req) == NELIBC_MNT_ID_REQ_SIZE, "Linux mnt_id_req ABI size");
  struct nelibc_mnt_id_req mount_request = {0};
  mount_request.size = sizeof mount_request;
  mount_request.mnt_id = LSMT_ROOT;
  uint64_t mount_ids[16];
  errno = 0;
  long mount_count = listmount(&mount_request, mount_ids, sizeof mount_ids / sizeof mount_ids[0], 0);
  if (mount_count < 0) {
    if (errno != EPERM && errno != ENOSYS) return 469;
  } else if ((size_t)mount_count > sizeof mount_ids / sizeof mount_ids[0]) {
    return 470;
  } else if (mount_count > 0) {
    mount_request.mnt_id = mount_ids[0];
  }
  mount_request.param = STATMOUNT_SB_BASIC | STATMOUNT_MNT_BASIC | STATMOUNT_FS_TYPE;
  unsigned char statmount_buffer[sizeof(struct nelibc_statmount) + 1024];
  errno = 0;
  int statmount_result = statmount(&mount_request, (struct nelibc_statmount *)statmount_buffer,
                                   sizeof statmount_buffer, 0);
  if (statmount_result != 0) {
    if (errno != EPERM && errno != ENOSYS) return 471;
  } else {
    struct nelibc_statmount *mount_info = (struct nelibc_statmount *)statmount_buffer;
    if (mount_info->size < offsetof(struct nelibc_statmount, str) ||
        (mount_info->mask & STATMOUNT_SB_BASIC) == 0) return 472;
  }
  struct mount_attr mount_attributes = {0};
  errno = 0;
  if (mount_setattr(AT_FDCWD, "missing-nelibc-mount-setattr", 0, &mount_attributes, sizeof mount_attributes) != -1 ||
      (errno != ENOENT && errno != EPERM && errno != ENOSYS)) return 468;
  int mount_tree_fd = open_tree_attr(AT_FDCWD, "/", OPEN_TREE_CLOEXEC, NULL, 0);
  if (mount_tree_fd < 0) {
    if (errno != ENOSYS) return 455;
  } else {
    if (close(mount_tree_fd) != 0) return 456;
    errno = 0;
    if (open_tree_attr(AT_FDCWD, "missing-nelibc-open-tree-attr", OPEN_TREE_CLOEXEC, NULL, 0) != -1 || errno != ENOENT) return 457;
  }
  int open_tree_fd = open_tree(AT_FDCWD, "/", OPEN_TREE_CLOEXEC);
  if (open_tree_fd < 0) {
    if (errno != ENOSYS) return 458;
  } else {
    if (close(open_tree_fd) != 0) return 459;
    errno = 0;
    if (open_tree(AT_FDCWD, "missing-nelibc-open-tree", OPEN_TREE_CLOEXEC) != -1 || errno != ENOENT) return 460;
  }
  errno = 0;
  if (move_mount(AT_FDCWD, "missing-nelibc-move-mount", AT_FDCWD, "/", MOVE_MOUNT_F_SYMLINKS) != -1 ||
      (errno != ENOENT && errno != EPERM && errno != ENOSYS)) return 467;
  errno = 0;
  if (fsmount(-1, FSMOUNT_CLOEXEC, 0) != -1 || (errno != EPERM && errno != EBADF && errno != ENOSYS)) return 462;
  int fs_context_fd = fsopen("tmpfs", FSOPEN_CLOEXEC);
  if (fs_context_fd < 0) {
    if (errno != EPERM && errno != ENOSYS) return 463;
  } else {
    if (close(fs_context_fd) != 0) return 464;
  }
  errno = 0;
  if (fsconfig(-1, FSCONFIG_CMD_CREATE, NULL, NULL, 0) != -1 ||
      (errno != EPERM && errno != EBADF && errno != EINVAL && errno != ENOSYS)) return 465;
  errno = 0;
  if (fspick(AT_FDCWD, "missing-nelibc-fspick", FSPICK_NO_AUTOMOUNT) != -1 ||
      (errno != ENOENT && errno != EPERM && errno != ENOSYS)) return 466;
  static const char transfer_payload[] = "sf";
  if (pwrite(allocation_fd, transfer_payload, 2, 0) != 2) return 403;
  int transfer_pipe[2];
  if (pipe(transfer_pipe) != 0) return 404;
  off_t transfer_offset = 0;
  if (sendfile64(transfer_pipe[1], allocation_fd, &transfer_offset, 2) != 2 || transfer_offset != 2 ||
      lseek(allocation_fd, 0, SEEK_CUR) != 0 || close(transfer_pipe[1]) != 0) return 405;
  char transfer_result[2];
  if (read(transfer_pipe[0], transfer_result, sizeof transfer_result) != 2 ||
      memcmp(transfer_result, transfer_payload, 2) != 0 || close(transfer_pipe[0]) != 0) return 406;
  char range_copy_path[64];
  make_seek_path(range_copy_path);
  size_t range_copy_path_length = strlen(range_copy_path);
  static const char range_copy_suffix[] = ".range-copy";
  for (size_t i = 0; i < sizeof range_copy_suffix; ++i)
    range_copy_path[range_copy_path_length + i] = range_copy_suffix[i];
  int range_copy_fd = openat(AT_FDCWD, range_copy_path, O_CREAT | O_EXCL | O_RDWR, 0600);
  if (range_copy_fd < 0) return 408;
  off_t input_range_offset = 0, output_range_offset = 0;
  if (copy_file_range(allocation_fd, &input_range_offset, range_copy_fd, &output_range_offset, 2, 0) != 2 ||
      input_range_offset != 2 || output_range_offset != 2) return 409;
  char range_copy_result[2];
  if (pread(range_copy_fd, range_copy_result, sizeof range_copy_result, 0) != 2 ||
      memcmp(range_copy_result, transfer_payload, 2) != 0) return 410;
  int splice_pipe[2];
  static const char splice_payload[] = "sp";
  if (pipe(splice_pipe) != 0 || write(splice_pipe[1], splice_payload, 2) != 2) return 413;
  off_t splice_output_offset = 2;
  if (splice(splice_pipe[0], NULL, range_copy_fd, &splice_output_offset, 2, 0) != 2 ||
      splice_output_offset != 4 || close(splice_pipe[0]) != 0 || close(splice_pipe[1]) != 0) return 414;
  char splice_result[2];
  if (pread(range_copy_fd, splice_result, sizeof splice_result, 2) != 2 ||
      memcmp(splice_result, splice_payload, 2) != 0) return 415;
  int tee_source[2], tee_destination[2];
  static const char tee_payload[] = "te";
  if (pipe(tee_source) != 0 || pipe(tee_destination) != 0 ||
      write(tee_source[1], tee_payload, 2) != 2 || tee(tee_source[0], tee_destination[1], 2, 0) != 2) return 417;
  char tee_result_source[2], tee_result_destination[2];
  if (read(tee_source[0], tee_result_source, 2) != 2 || read(tee_destination[0], tee_result_destination, 2) != 2 ||
      memcmp(tee_result_source, tee_payload, 2) != 0 || memcmp(tee_result_destination, tee_payload, 2) != 0 ||
      close(tee_source[0]) != 0 || close(tee_source[1]) != 0 ||
      close(tee_destination[0]) != 0 || close(tee_destination[1]) != 0) return 418;
  errno = 0;
  if (tee(-1, -1, 1, 0) != -1 || errno != EBADF) return 419;
  int vmsplice_pipe[2];
  static const char vmsplice_payload[] = "vm";
  struct iovec vmsplice_vector = {(void *)vmsplice_payload, 2};
  if (pipe(vmsplice_pipe) != 0 || vmsplice(vmsplice_pipe[1], &vmsplice_vector, 1, 0) != 2) return 420;
  char vmsplice_result[2];
  if (read(vmsplice_pipe[0], vmsplice_result, 2) != 2 || memcmp(vmsplice_result, vmsplice_payload, 2) != 0 ||
      close(vmsplice_pipe[0]) != 0 || close(vmsplice_pipe[1]) != 0) return 421;
  errno = 0;
  if (vmsplice(-1, &vmsplice_vector, 1, 0) != -1 || errno != EBADF) return 422;
  errno = 0;
  if (splice(-1, NULL, range_copy_fd, NULL, 1, 0) != -1 || errno != EBADF) return 416;
  errno = 0;
  if (copy_file_range(-1, NULL, range_copy_fd, NULL, 1, 0) != -1 || errno != EBADF) return 411;
  if (close(range_copy_fd) != 0 || unlink(range_copy_path) != 0) return 412;
  errno = 0;
  if (sendfile(-1, allocation_fd, NULL, 1) != -1 || errno != EBADF) return 407;
  errno = 42;
  int posix_allocation_error = posix_fallocate(-1, 0, 4096);
  if (posix_allocation_error != EBADF || errno != 42) return 397;
  errno = 0;
  if (fallocate(-1, 0, 0, 4096) != -1 || errno != EBADF) return 398;
  if (close(allocation_fd) != 0 || unlink(allocation_path) != 0) return 399;
  FILE *temporary_file = tmpfile();
  struct stat temporary_status;
  char temporary_contents[8] = {0};
  if (temporary_file == NULL || fstat(temporary_file->fd, &temporary_status) != 0 ||
      temporary_status.st_nlink != 0 || fputs("tmpfile", temporary_file) != 0 ||
      ftell(temporary_file) != 7 || fseek(temporary_file, 0, SEEK_SET) != 0 ||
      fgets(temporary_contents, sizeof temporary_contents, temporary_file) != temporary_contents ||
      strcmp(temporary_contents, "tmpfile") != 0 || fclose(temporary_file) != 0) return 245;
  char memory_contents[16] = "hello";
  FILE *memory_stream = fmemopen(memory_contents, sizeof memory_contents, "r+");
  if (memory_stream == NULL) return 250;
  if (fseek(memory_stream, 0, SEEK_END) != 0) return 253;
  if (fputs("!", memory_stream) != 0) return 254;
  if (fflush(memory_stream) != 0) return 255;
  if (strcmp(memory_contents, "hello!") != 0) return 256;
  if (ftell(memory_stream) != 6) return 257;
  if (fseek(memory_stream, 0, SEEK_SET) != 0) return 258;
  char memory_readback[8] = {0};
  if (fgets(memory_readback, sizeof memory_readback, memory_stream) != memory_readback ||
      strcmp(memory_readback, "hello!") != 0 || fclose(memory_stream) != 0) return 251;
  memory_stream = fmemopen(NULL, 32, "w+");
  char owned_readback[8] = {0};
  if (memory_stream == NULL || fputs("owned", memory_stream) != 0 ||
      fseek(memory_stream, 0, SEEK_SET) != 0 ||
      fgets(owned_readback, sizeof owned_readback, memory_stream) != owned_readback ||
      strcmp(owned_readback, "owned") != 0 || fclose(memory_stream) != 0) return 252;
  char append_contents[12] = "tail";
  memory_stream = fmemopen(append_contents, sizeof append_contents, "a+");
  if (memory_stream == NULL || fseek(memory_stream, 0, SEEK_SET) != 0 ||
      fputs("!", memory_stream) != 0 || fflush(memory_stream) != 0 ||
      strcmp(append_contents, "tail!") != 0 || fclose(memory_stream) != 0) return 259;
  errno = 0;
  if (fmemopen(memory_contents, sizeof memory_contents, "rbb") != NULL || errno != EINVAL)
    return 260;
  char *dynamic_contents = NULL;
  size_t dynamic_size = 0;
  FILE *dynamic_stream = open_memstream(&dynamic_contents, &dynamic_size);
  if (dynamic_stream == NULL || fwrite("alpha", 1, 5, dynamic_stream) != 5 ||
      fflush(dynamic_stream) != 0 || dynamic_contents == NULL || dynamic_size != 5 ||
      strcmp(dynamic_contents, "alpha") != 0 || fseek(dynamic_stream, -2, SEEK_END) != 0 ||
      fwrite("B", 1, 1, dynamic_stream) != 1 || fflush(dynamic_stream) != 0 ||
      dynamic_size != 5 || strcmp(dynamic_contents, "alpBa") != 0 ||
      fseek(dynamic_stream, 2, SEEK_END) != 0 || fwrite("Z", 1, 1, dynamic_stream) != 1 ||
      fflush(dynamic_stream) != 0 || dynamic_size != 8 || dynamic_contents[8] != 0 ||
      dynamic_contents[5] != 0 || dynamic_contents[6] != 0 || dynamic_contents[7] != 'Z' ||
      fclose(dynamic_stream) != 0) return 261;
  free(dynamic_contents);
  dynamic_contents = NULL;
  dynamic_size = 99;
  dynamic_stream = open_memstream(&dynamic_contents, &dynamic_size);
  if (dynamic_stream == NULL || fflush(dynamic_stream) != 0 || dynamic_contents == NULL ||
      dynamic_size != 0 || fclose(dynamic_stream) != 0) return 262;
  free(dynamic_contents);
  dynamic_contents = NULL;
  dynamic_stream = open_memstream(&dynamic_contents, &dynamic_size);
  if (dynamic_stream == NULL || fputs("close", dynamic_stream) != 0 ||
      fclose(dynamic_stream) != 0 || dynamic_contents == NULL || dynamic_size != 5 ||
      strcmp(dynamic_contents, "close") != 0) return 263;
  free(dynamic_contents);
  int fdopen_pipe[2];
  if (pipe(fdopen_pipe) != 0) return 246;
  errno = 0;
  if (fdopen(fdopen_pipe[0], "w") != NULL || errno != EINVAL ||
      write(fdopen_pipe[1], "Z", 1) != 1 || close(fdopen_pipe[1]) != 0) return 247;
  FILE *descriptor_stream = fdopen(fdopen_pipe[0], "r");
  if (descriptor_stream == NULL || fileno(descriptor_stream) != fdopen_pipe[0] ||
      fgetc(descriptor_stream) != 'Z' || fclose(descriptor_stream) != 0) return 248;
  errno = 0;
  if (close(fdopen_pipe[0]) != -1 || errno != EBADF) return 249;
#endif
#ifdef NELIBC_STANDALONE
  FILE *extension_stream = tmpfile();
  char extension_storage[64];
  char extension_line[32] = {0};
  if (extension_stream == NULL) return 472;
  setbuffer(extension_stream, extension_storage, sizeof extension_storage);
  if (fputs_unlocked("standalone\n", extension_stream) != 0 ||
      fflush(extension_stream) != 0 || ftello(extension_stream) != 11 ||
      fseeko(extension_stream, 0, SEEK_SET) != 0 ||
      fgets_unlocked(extension_line, sizeof extension_line, extension_stream) != extension_line ||
      strcmp(extension_line, "standalone\n") != 0 || fclose(extension_stream) != 0)
    return 473;
  extension_stream = tmpfile();
  if (extension_stream == NULL) return 474;
  setlinebuf(extension_stream);
  if (fputs_unlocked("line\n", extension_stream) != 0 ||
      fflush(extension_stream) != 0 || fclose(extension_stream) != 0)
    return 475;
#endif
  char directory_path[64];
  make_seek_path(directory_path);
  size_t directory_length = strlen(directory_path);
  static const char directory_suffix[] = ".dir";
  for (size_t i = 0; i < sizeof directory_suffix; ++i)
    directory_path[directory_length + i] = directory_suffix[i];
  if (mkdir(directory_path, 0700) != 0) return 142;
  errno = 0;
  if (mkdir(directory_path, 0700) != -1 || errno != EEXIST) return 320;
  if (rmdir(directory_path) != 0) return 143;
  errno = 0;
  if (openat(-1, "dev/null", O_RDONLY) != -1 || errno != EBADF) return 55;
  errno = 0;
  if (openat(AT_FDCWD, (const char *)(uintptr_t)1, O_RDONLY) != -1 || errno != EFAULT) return 56;

  int error_pipe[2];
  if (pipe(error_pipe) != 0) return 181;
#ifndef NELIBC_STANDALONE
  if (fflush(stderr) != 0) return 185;
#endif
  int saved_stderr = dup(2);
  if (saved_stderr < 0 || dup2(error_pipe[1], 2) != 2) return 182;
  errno = ENOENT;
  perror("nelibc");
  if (errno != ENOENT) return 183;
  errno = ENOENT;
  perror("");
  if (errno != ENOENT) return 184;
#ifndef NELIBC_STANDALONE
  if (fflush(stderr) != 0) return 185;
#endif
  if (dup2(saved_stderr, 2) != 2 || close(saved_stderr) != 0 || close(error_pipe[1]) != 0) return 186;
  static const char perror_expected[] =
      "nelibc: No such file or directory\nNo such file or directory\n";
  char perror_output[sizeof perror_expected - 1];
  size_t perror_bytes = 0;
  while (perror_bytes < sizeof perror_output) {
    long received = read(error_pipe[0], perror_output + perror_bytes,
                         sizeof perror_output - perror_bytes);
    if (received <= 0) return 187;
    perror_bytes += (size_t)received;
  }
  if (memcmp(perror_output, perror_expected, sizeof perror_output) != 0 || close(error_pipe[0]) != 0) return 188;

  long written = nelibc_linux_write(1, message, sizeof message - 1);
  if (written != (long)(sizeof message - 1)) return 5;

  unsigned char *allocation = nelibc_malloc(37);
  if (allocation == 0 || (uintptr_t)allocation % _Alignof(max_align_t) != 0) return 6;
  for (size_t i = 0; i < 37; ++i) allocation[i] = (unsigned char)(i ^ 0x5A);
  for (size_t i = 0; i < 37; ++i) {
    if (allocation[i] != (unsigned char)(i ^ 0x5A)) return 7;
  }
  unsigned char *grown = nelibc_realloc(allocation, 64);
  if (grown != allocation) return 39;
  if (grown == 0 || (uintptr_t)grown % _Alignof(max_align_t) != 0) return 8;
  for (size_t i = 0; i < 37; ++i) {
    if (grown[i] != (unsigned char)(i ^ 0x5A)) return 9;
  }
  unsigned char *shrunk = nelibc_realloc(grown, 10);
  if (shrunk != grown) return 40;
  if (shrunk == 0) return 10;
  for (size_t i = 0; i < 10; ++i) {
    if (shrunk[i] != (unsigned char)(i ^ 0x5A)) return 11;
  }

  errno = 0;
  if (nelibc_realloc(shrunk, SIZE_MAX) != 0 || errno != ENOMEM) return 12;
  if (shrunk[0] != 0x5A) return 13; /* Failed realloc preserves the old block. */
  if (nelibc_realloc(shrunk, 0) != 0) return 14;
  unsigned char *from_null = nelibc_realloc(0, 8);
  if (from_null == 0) return 15;
  nelibc_free(from_null);
  nelibc_free(0);

  unsigned char *large_block = nelibc_malloc(4096);
  if (large_block == 0) return 41;
  large_block[0] = 0xA7;
  large_block[4095] = 0x5C;
  unsigned char *larger_block = nelibc_realloc(large_block, 8192);
  if (larger_block == 0 || larger_block[0] != 0xA7 ||
      larger_block[4095] != 0x5C) return 42;
  nelibc_free(larger_block);

#ifndef NELIBC_STANDALONE
  if (allocator_thread_smoke() != 0) return 38;
#endif

  unsigned char *zeroed = nelibc_calloc(32, 1);
  if (zeroed == 0) return 16;
  for (size_t i = 0; i < 32; ++i) {
    if (zeroed[i] != 0) return 17;
  }
  nelibc_free(zeroed);

  errno = 0;
  if (nelibc_malloc(SIZE_MAX) != 0 || errno != ENOMEM) return 18;
  errno = 0;
  if (nelibc_calloc(SIZE_MAX, 2) != 0 || errno != ENOMEM) return 19;

#ifdef NELIBC_STANDALONE
  unsigned char *standard = malloc(24);
  if (standard == 0 || (uintptr_t)standard % _Alignof(max_align_t) != 0) return 20;
  for (size_t i = 0; i < 24; ++i) standard[i] = (unsigned char)(255 - i);
  standard = realloc(standard, 40);
  if (standard == 0) return 21;
  for (size_t i = 0; i < 24; ++i) {
    if (standard[i] != (unsigned char)(255 - i)) return 22;
  }
  free(standard);

  unsigned char *array = reallocarray(0, 8, 3);
  if (array == 0) return 33;
  for (size_t i = 0; i < 24; ++i) array[i] = (unsigned char)(i + 7);
  unsigned char *larger_array = reallocarray(array, 16, 3);
  if (larger_array == 0) return 34;
  for (size_t i = 0; i < 24; ++i) {
    if (larger_array[i] != (unsigned char)(i + 7)) return 35;
  }
  volatile size_t overflow_count = SIZE_MAX;
  errno = 0;
  if (reallocarray(larger_array, overflow_count, 2) != 0 || errno != ENOMEM) return 36;
  for (size_t i = 0; i < 24; ++i) {
    if (larger_array[i] != (unsigned char)(i + 7)) return 37;
  }
  free(larger_array);

  unsigned char *standard_zeroed = calloc(16, 1);
  if (standard_zeroed == 0) return 23;
  for (size_t i = 0; i < 16; ++i) {
    if (standard_zeroed[i] != 0) return 24;
  }
  free(standard_zeroed);

  unsigned char *aligned = aligned_alloc(64, 128);
  if (aligned == 0 || (uintptr_t)aligned % 64 != 0) return 25;
  for (size_t i = 0; i < 128; ++i) aligned[i] = (unsigned char)(i ^ 0xA5);
  aligned = realloc(aligned, 256);
  if (aligned == 0 || (uintptr_t)aligned % _Alignof(max_align_t) != 0) return 26;
  for (size_t i = 0; i < 128; ++i) {
    if (aligned[i] != (unsigned char)(i ^ 0xA5)) return 27;
  }
  free(aligned);

  errno = 0;
  if (aligned_alloc(0, 0) != 0 || errno != EINVAL) return 28;
  errno = 0;
  if (aligned_alloc(64, 65) != 0 || errno != EINVAL) return 31;
  void *posix_aligned = (void *)(uintptr_t)0x1234;
  errno = EDOM;
  if (posix_memalign(&posix_aligned, 24, 64) != EINVAL ||
      posix_aligned != (void *)(uintptr_t)0x1234 || errno != EDOM) return 29;
  if (posix_memalign(&posix_aligned, 256, 300) != 0 ||
      (uintptr_t)posix_aligned % 256 != 0 || errno != EDOM) return 30;
  free(posix_aligned);
  posix_aligned = (void *)(uintptr_t)0x5678;
  if (posix_memalign(&posix_aligned, 256, SIZE_MAX) != ENOMEM ||
      posix_aligned != (void *)(uintptr_t)0x5678 || errno != EDOM) return 32;
#endif
  int fcntl_result = fcntl_argument_smoke();
  if (fcntl_result != 0) return fcntl_result;
  return 0;
}
