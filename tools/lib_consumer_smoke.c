#define _GNU_SOURCE
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <strings.h>
#include <limits.h>
#include <unistd.h>
#include <getopt.h>
#include <errno.h>
#include <signal.h>
#include <sys/wait.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <locale.h>
#include <wchar.h>
#include <wctype.h>
#include <semaphore.h>
#include <sys/stat.h>
#include "nelibc_alloc.h"
#ifndef NELIBC_STANDALONE
#include <sys/mman.h>
#include <pthread.h>
#endif

extern char **environ;
extern long nelibc_test_legacy_strtol(const char *, char **, int)
    __asm__("strtol");
extern long nelibc_test_legacy_wcstol(const wchar_t *, wchar_t **, int)
    __asm__("wcstol");

#if defined(NELIBC_STATIC_MUSL)
extern int nelibc_test_standard_strerror_r(int, char *, size_t)
    __asm__("strerror_r");
#endif

static void system_signal_probe(int signal_number) {
  (void)signal_number;
}

#ifdef NELIBC_STANDALONE
static int standalone_locale_smoke(void) {
  static const int categories[] = {
      LC_COLLATE, LC_CTYPE, LC_MONETARY, LC_NUMERIC, LC_TIME,
#ifdef LC_MESSAGES
      LC_MESSAGES,
#endif
  };
  static const char expected_utf8_all[] =
      "LC_CTYPE=C.UTF-8;LC_NUMERIC=C;LC_TIME=C;LC_COLLATE=C;"
      "LC_MONETARY=C;LC_MESSAGES=C";
  static const char expected_de_all[] =
      "LC_CTYPE=C;LC_NUMERIC=de_DE.UTF-8;LC_TIME=C;LC_COLLATE=C;"
      "LC_MONETARY=C;LC_MESSAGES=C";
  static const char expected_de_monetary_all[] =
      "LC_CTYPE=C;LC_NUMERIC=de_DE.UTF-8;LC_TIME=C;LC_COLLATE=C;"
      "LC_MONETARY=de_DE.UTF-8;LC_MESSAGES=C";
  const char *current = setlocale(LC_ALL, NULL);
  struct lconv *numeric = localeconv();
  if (numeric == NULL || numeric->decimal_point == NULL ||
      strcmp(numeric->decimal_point, ".") != 0 ||
      numeric->thousands_sep == NULL || numeric->thousands_sep[0] != '\0') return 9;
  if (current == NULL || strcmp(current, "C") != 0) return 1;
  for (size_t i = 0; i < sizeof categories / sizeof categories[0]; ++i) {
    current = setlocale(categories[i], NULL);
    if (current == NULL || strcmp(current, "C") != 0) return 2;
  }
  if (setlocale(LC_NUMERIC, "POSIX") == NULL ||
      setlocale(LC_NUMERIC, "unsupported_locale") != NULL ||
      strcmp(setlocale(LC_NUMERIC, NULL), "C") != 0) return 3;
  if (setlocale(LC_NUMERIC, "de_DE.UTF-8") == NULL ||
      strcmp(setlocale(LC_NUMERIC, NULL), "de_DE.UTF-8") != 0) return 10;
  if (strcmp(setlocale(LC_ALL, NULL), expected_de_all) != 0) return 15;
  numeric = localeconv();
  if (numeric == NULL || strcmp(numeric->decimal_point, ",") != 0 ||
      strcmp(numeric->thousands_sep, ".") != 0 || numeric->grouping[0] != 3) return 11;
  if (setlocale(LC_MONETARY, "de_DE.UTF-8") == NULL ||
      strcmp(setlocale(LC_MONETARY, NULL), "de_DE.UTF-8") != 0 ||
      strcmp(setlocale(LC_ALL, NULL), expected_de_monetary_all) != 0) return 29;
  numeric = localeconv();
  if (strcmp(numeric->int_curr_symbol, "EUR ") != 0 ||
      strcmp(numeric->currency_symbol, "\342\202\254") != 0 ||
      strcmp(numeric->mon_decimal_point, ",") != 0 ||
      strcmp(numeric->mon_thousands_sep, ".") != 0 ||
      numeric->mon_grouping[0] != 3 || numeric->frac_digits != 2 ||
      numeric->p_cs_precedes != 0 || numeric->p_sep_by_space != 1 ||
      numeric->n_sign_posn != 1) return 30;
  if (setlocale(LC_MONETARY, "C") == NULL ||
      strcmp(setlocale(LC_ALL, NULL), expected_de_all) != 0) return 31;
  char *end = NULL;
  if (strtod("12,5", &end) != 12.5 || end == NULL || *end != '\0') return 12;
  char formatted[16];
  if (snprintf(formatted, sizeof formatted, "%.1f", 12.5) != 4 ||
      strcmp(formatted, "12,5") != 0) return 13;
  if (snprintf(formatted, sizeof formatted, "%'.2f", 1234567.5) != 12 ||
      strcmp(formatted, "1.234.567,50") != 0) return 24;
  if (snprintf(formatted, sizeof formatted, "%'.8g", 1234567.5) != 11 ||
      strcmp(formatted, "1.234.567,5") != 0) return 25;
  if (snprintf(formatted, sizeof formatted, "%.4g", 9999.9) != 5 ||
      strcmp(formatted, "1e+04") != 0) return 26;
  if (snprintf(formatted, sizeof formatted, "%.4g", 0.000099999) != 6 ||
      strcmp(formatted, "0,0001") != 0) return 27;
  if (snprintf(formatted, sizeof formatted, "%.0f", 2.5) != 1 ||
      strcmp(formatted, "2") != 0 ||
      snprintf(formatted, sizeof formatted, "%.0f", 3.5) != 1 ||
      strcmp(formatted, "4") != 0 ||
      snprintf(formatted, sizeof formatted, "%.1f", 1.25) != 3 ||
      strcmp(formatted, "1,2") != 0) return 28;
  int formatted_length = snprintf(formatted, sizeof formatted, "%'d", 1234567);
  if (formatted_length != 9) return 18;
  if (strcmp(formatted, "1.234.567") != 0) return 19;
  formatted_length = snprintf(formatted, sizeof formatted, "%'010d", 1234);
  if (formatted_length != 10) return 20;
  if (strcmp(formatted, "000001.234") != 0) return 21;
  formatted_length = snprintf(formatted, sizeof formatted, "%'.10d", 1234);
  if (formatted_length != 11 || strcmp(formatted, "0000001.234") != 0) return 22;
  if (setlocale(LC_NUMERIC, "C") == NULL ||
      strcmp(localeconv()->decimal_point, ".") != 0) return 14;
  if (snprintf(formatted, sizeof formatted, "%'d", 1234567) != 7 ||
      strcmp(formatted, "1234567") != 0) return 23;
  if (setlocale(LC_CTYPE, "C.UTF-8") == NULL) return 4;
  current = setlocale(LC_ALL, NULL);
  if (current == NULL || strcmp(current, expected_utf8_all) != 0) return 5;
  char saved_locale[sizeof expected_utf8_all];
  size_t length = strlen(current);
  if (length >= sizeof saved_locale) return 6;
  memcpy(saved_locale, current, length + 1);
  if (setlocale(LC_CTYPE, "C") == NULL ||
      setlocale(LC_ALL, saved_locale) == NULL ||
      strcmp(setlocale(LC_CTYPE, NULL), "C.UTF-8") != 0) return 7;
  if (setlocale(LC_ALL, "C") == NULL ||
      strcmp(setlocale(LC_ALL, NULL), "C") != 0) return 8;
  return 0;
}
#endif

static int allocator_probe(void) {
  unsigned char *bytes = nelibc_malloc(7);
  if (bytes == NULL || (uintptr_t)bytes % sizeof(long double) != 0 ||
      !nelibc_allocator_owns(bytes)) return 1;
  for (size_t i = 0; i < 7; ++i) bytes[i] = (unsigned char)(0x30 + i);
  bytes = nelibc_realloc(bytes, 128);
  if (bytes == NULL) return 2;
  for (size_t i = 0; i < 7; ++i)
    if (bytes[i] != (unsigned char)(0x30 + i)) return 3;

  uint32_t *zeroed = nelibc_calloc(8, sizeof *zeroed);
  if (zeroed == NULL) return 4;
  for (size_t i = 0; i < 8; ++i)
    if (zeroed[i] != 0) return 5;

  uint32_t *standard_zeroed = calloc(8, sizeof *standard_zeroed);
  if (standard_zeroed == NULL || !nelibc_allocator_owns(standard_zeroed)) return 15;
  for (size_t i = 0; i < 8; ++i)
    if (standard_zeroed[i] != 0) return 16;
  errno = 0;
  if (calloc(SIZE_MAX, 2) != NULL || errno != ENOMEM) return 17;
  errno = 0;
  void *standard_aligned = aligned_alloc(64, 128);
  if (standard_aligned == NULL || (uintptr_t)standard_aligned % 64 != 0 ||
      !nelibc_allocator_owns(standard_aligned))
    return 18;
  free(standard_aligned);
  errno = 0;
  if (aligned_alloc(64, 127) != NULL || errno != EINVAL) return 19;

  void *aligned = nelibc_aligned_alloc(64, 128);
  if (aligned == NULL || (uintptr_t)aligned % 64 != 0) return 6;
  void *posix_aligned = NULL;
  errno = E2BIG;
  int posix_result = nelibc_posix_memalign(&posix_aligned, 128, 19);
  if (posix_result != 0 || (uintptr_t)posix_aligned % 128 != 0 || errno != E2BIG)
    return 7;
  void *standard_posix_aligned = NULL;
  errno = E2BIG;
  posix_result = posix_memalign(&standard_posix_aligned, 128, 19);
  if (posix_result != 0 || (uintptr_t)standard_posix_aligned % 128 != 0 ||
      errno != E2BIG || !nelibc_allocator_owns(standard_posix_aligned)) return 20;
  free(standard_posix_aligned);
  standard_posix_aligned = (void *)(uintptr_t)1;
  if (posix_memalign(&standard_posix_aligned, 3, 19) != EINVAL ||
      standard_posix_aligned != (void *)(uintptr_t)1) return 21;

  void *large = nelibc_malloc(32768);
  if (large == NULL || !nelibc_allocator_owns(large)) return 10;
  ((unsigned char *)large)[0] = 0x6d;
  ((unsigned char *)large)[32767] = 0xb2;
  void *large_grown = nelibc_realloc(large, 65537);
  if (large_grown == NULL ||
      ((unsigned char *)large_grown)[0] != 0x6d ||
      ((unsigned char *)large_grown)[32767] != 0xb2 ||
      !nelibc_allocator_owns(large_grown)) return 22;
  large = large_grown;
  nelibc_free(large);
  if (nelibc_allocator_owns(large)) return 11;

  void *aligned_direct = NULL;
  if (posix_memalign(&aligned_direct, 65536, 32768) != 0 ||
      aligned_direct == NULL || (uintptr_t)aligned_direct % 65536 != 0)
    return 25;
  ((unsigned char *)aligned_direct)[0] = 0x3c;
  ((unsigned char *)aligned_direct)[32767] = 0xc3;
  void *aligned_direct_grown = nelibc_realloc(aligned_direct, 131073);
  if (aligned_direct_grown == NULL ||
      (uintptr_t)aligned_direct_grown % 65536 != 0 ||
      ((unsigned char *)aligned_direct_grown)[0] != 0x3c ||
      ((unsigned char *)aligned_direct_grown)[32767] != 0xc3 ||
      !nelibc_allocator_owns(aligned_direct_grown)) return 26;
  nelibc_free(aligned_direct_grown);

  void *standard = malloc(32);
  if (standard == NULL) return 12;
  if (!nelibc_allocator_owns(standard)) return 13;
  free(standard);
  free(standard_zeroed);

  errno = 0;
  if (nelibc_calloc(SIZE_MAX, 2) != NULL || errno != ENOMEM) return 8;
  errno = 0;
  if (nelibc_reallocarray(NULL, SIZE_MAX, 2) != NULL || errno != ENOMEM) return 9;

  nelibc_free(bytes);
  nelibc_free(zeroed);
  nelibc_free(aligned);
  nelibc_free(posix_aligned);
  return 0;
}

static int semaphore_clock_probe(void) {
  sem_t semaphore;
  if (sem_init(&semaphore, 0, 1) != 0) return 1;

  struct timespec invalid = { .tv_sec = 0, .tv_nsec = 1000000000L };
  if (sem_clockwait(&semaphore, CLOCK_MONOTONIC, &invalid) != 0) return 2;

  struct timespec expired = { .tv_sec = 0, .tv_nsec = 0 };
  if (sem_clockwait(&semaphore, CLOCK_MONOTONIC, &expired) != -1 ||
      errno != ETIMEDOUT) return 3;
  if (sem_destroy(&semaphore) != 0) return 4;
  return 0;
}

#ifndef NELIBC_STANDALONE
static int atfork_probe_order[8];
static int atfork_probe_count;
static void atfork_probe_add(int value) {
  if (atfork_probe_count < 8) atfork_probe_order[atfork_probe_count++] = value;
}
static void atfork_probe_prepare_a(void) { atfork_probe_add(1); }
static void atfork_probe_prepare_b(void) { atfork_probe_add(2); }
static void atfork_probe_parent_a(void) { atfork_probe_add(3); }
static void atfork_probe_parent_b(void) { atfork_probe_add(4); }
static void atfork_probe_child_a(void) { atfork_probe_add(5); }
static void atfork_probe_child_b(void) { atfork_probe_add(6); }

static int pthread_protocol_probe(void) {
  pthread_mutexattr_t attributes;
  pthread_mutex_t mutex;
  int protocol = -1;
  if (pthread_mutexattr_init(&attributes) != 0 ||
      pthread_mutexattr_setprotocol(&attributes, PTHREAD_PRIO_INHERIT) != 0 ||
      pthread_mutexattr_getprotocol(&attributes, &protocol) != 0 ||
      protocol != PTHREAD_PRIO_INHERIT ||
      pthread_mutex_init(&mutex, &attributes) != 0 ||
      pthread_mutexattr_destroy(&attributes) != 0 ||
      pthread_mutex_lock(&mutex) != 0 || pthread_mutex_unlock(&mutex) != 0 ||
      pthread_mutex_destroy(&mutex) != 0)
    return 1;
  return 0;
}

static int pthread_sigmask_probe(void) {
  sigset_t signal_set;
  sigset_t previous_set;
  sigset_t observed_set;
  clockid_t cpu_clock;
  struct timespec cpu_time;
  if (sigemptyset(&signal_set) != 0 || sigaddset(&signal_set, SIGUSR1) != 0 ||
      pthread_sigmask(SIG_BLOCK, &signal_set, &previous_set) != 0 ||
      pthread_sigmask(0, NULL, &observed_set) != 0 ||
      sigismember(&observed_set, SIGUSR1) != 1 ||
      pthread_sigmask(SIG_SETMASK, &previous_set, NULL) != 0 ||
      pthread_kill(pthread_self(), 0) != 0 ||
      pthread_getcpuclockid(pthread_self(), &cpu_clock) != 0 ||
      clock_gettime(cpu_clock, &cpu_time) != 0)
    return 1;
  return 0;
}

static int archive_unistd_adapter_probe(void) {
  /* Force archive extraction of the variadic and credential C ABI adapters. */
  void (*volatile immediate_exit_symbol)(int) = _Exit;
  int (*volatile setuid_symbol)(uid_t) = setuid;
  int (*volatile seteuid_symbol)(uid_t) = seteuid;
  int (*volatile setgid_symbol)(gid_t) = setgid;
  int (*volatile setegid_symbol)(gid_t) = setegid;
  int (*volatile setreuid_symbol)(uid_t, uid_t) = setreuid;
  int (*volatile setregid_symbol)(gid_t, gid_t) = setregid;
  int (*volatile setresuid_symbol)(uid_t, uid_t, uid_t) = setresuid;
  int (*volatile setresgid_symbol)(gid_t, gid_t, gid_t) = setresgid;
  if (immediate_exit_symbol == NULL || setuid_symbol == NULL || seteuid_symbol == NULL ||
      setgid_symbol == NULL ||
      setegid_symbol == NULL || setreuid_symbol == NULL || setregid_symbol == NULL ||
      setresuid_symbol == NULL || setresgid_symbol == NULL)
    return 10;

  if (pthread_atfork(atfork_probe_prepare_a, atfork_probe_parent_a,
                     atfork_probe_child_a) != 0 ||
      pthread_atfork(atfork_probe_prepare_b, atfork_probe_parent_b,
                     atfork_probe_child_b) != 0)
    return 26;
  pid_t child = fork();
  if (child < 0) return 15;
  if (child == 0) {
    if (atfork_probe_count != 4 || atfork_probe_order[0] != 2 ||
        atfork_probe_order[1] != 1 || atfork_probe_order[2] != 5 ||
        atfork_probe_order[3] != 6)
      _Exit(38);
    _Exit(37);
  }
  int status = 0;
  if (waitpid(child, &status, 0) != child || !WIFEXITED(status) ||
      WEXITSTATUS(status) != 37 || atfork_probe_count != 4 ||
      atfork_probe_order[0] != 2 || atfork_probe_order[1] != 1 ||
      atfork_probe_order[2] != 3 || atfork_probe_order[3] != 4)
    return 16;
  atfork_probe_count = 0;

  if (fcntl(STDOUT_FILENO, F_GETFD) < 0) return 11;
  int large_file_fd = open64("/dev/null", O_RDONLY);
  if (large_file_fd < 0 || close(large_file_fd) != 0) return 22;
  large_file_fd = openat64(AT_FDCWD, "/dev/null", O_RDONLY);
  if (large_file_fd < 0 || close(large_file_fd) != 0) return 23;
  void *large_mapping = mmap64(NULL, 4096, PROT_READ | PROT_WRITE,
                               MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (large_mapping == MAP_FAILED || munmap(large_mapping, 4096) != 0) return 24;
  FILE *offset_file = tmpfile();
  if (offset_file == NULL) return 25;
  int offset_fd = fileno(offset_file);
  const char scalar_write[] = "N";
  char scalar_read = 0;
  struct iovec write_vector = {(void *)"L", 1};
  struct iovec read_vector = {&scalar_read, 1};
  if (offset_fd < 0 || pwrite64(offset_fd, scalar_write, 1, 0) != 1 ||
      pread64(offset_fd, &scalar_read, 1, 0) != 1 || scalar_read != 'N' ||
      pwritev64(offset_fd, &write_vector, 1, 1) != 1 ||
      preadv64(offset_fd, &read_vector, 1, 1) != 1 || scalar_read != 'L' ||
      lseek64(offset_fd, 0, SEEK_SET) != 0 ||
      ftruncate64(offset_fd, 0) != 0 || fclose(offset_file) != 0)
    return 26;
  unsigned char *mapping = mmap(NULL, 4096, PROT_READ | PROT_WRITE,
                                MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (mapping == MAP_FAILED) return 12;
  mapping[0] = 0x5a;
  unsigned char *grown = mremap(mapping, 4096, 8192, MREMAP_MAYMOVE);
  if (grown == MAP_FAILED) {
    munmap(mapping, 4096);
    return 13;
  }
  if (grown[0] != 0x5a || munmap(grown, 8192) != 0) return 14;

  unsigned char *array = reallocarray(NULL, 8, 3);
  if (array == NULL || !nelibc_allocator_owns(array)) return 17;
  for (size_t i = 0; i < 24; ++i) array[i] = (unsigned char)(i + 9);
  unsigned char *larger_array = reallocarray(array, 16, 3);
  if (larger_array == NULL || !nelibc_allocator_owns(larger_array)) return 18;
  for (size_t i = 0; i < 24; ++i)
    if (larger_array[i] != (unsigned char)(i + 9)) return 19;
  errno = 0;
  if (reallocarray(larger_array, SIZE_MAX, 2) != NULL || errno != ENOMEM)
    return 20;
  for (size_t i = 0; i < 24; ++i)
    if (larger_array[i] != (unsigned char)(i + 9)) return 21;
  free(larger_array);
  return 0;
}

typedef struct {
  pthread_mutex_t mutex;
  pthread_cond_t condition;
  int ready;
  int go;
} MbrlenThreadGate;

typedef struct {
  MbrlenThreadGate *gate;
  const char *prefix;
  const char *middle;
  const char *last;
  size_t prefix_result;
  size_t middle_result;
  size_t last_result;
} MbrlenThreadProbe;

static void *mbrlen_thread_worker(void *argument) {
  MbrlenThreadProbe *probe = argument;
  probe->prefix_result = mbrlen(probe->prefix, 1, NULL);
  pthread_mutex_lock(&probe->gate->mutex);
  ++probe->gate->ready;
  pthread_cond_broadcast(&probe->gate->condition);
  while (!probe->gate->go)
    pthread_cond_wait(&probe->gate->condition, &probe->gate->mutex);
  pthread_mutex_unlock(&probe->gate->mutex);
  probe->middle_result = mbrlen(probe->middle, 1, NULL);
  if (probe->last != NULL) probe->last_result = mbrlen(probe->last, 1, NULL);
  return NULL;
}

static int mbrlen_thread_isolation(void) {
  if (setlocale(LC_CTYPE, "C.UTF-8") == NULL) return 0;
  MbrlenThreadGate gate = {PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER, 0, 0};
  MbrlenThreadProbe probes[2] = {
    {&gate, "\xc3", "\xa9", NULL, 0, 0, 0},
    {&gate, "\xe2", "\x82", "\xac", 0, 0, 0}
  };
  pthread_t threads[2];
  if (pthread_create(&threads[0], NULL, mbrlen_thread_worker, &probes[0]) != 0)
    return 1;
  if (pthread_create(&threads[1], NULL, mbrlen_thread_worker, &probes[1]) != 0) {
    pthread_mutex_lock(&gate.mutex);
    gate.go = 1;
    pthread_cond_broadcast(&gate.condition);
    pthread_mutex_unlock(&gate.mutex);
    (void)pthread_join(threads[0], NULL);
    (void)setlocale(LC_CTYPE, "C");
    return 1;
  }
  pthread_mutex_lock(&gate.mutex);
  while (gate.ready != 2)
    pthread_cond_wait(&gate.condition, &gate.mutex);
  gate.go = 1;
  pthread_cond_broadcast(&gate.condition);
  pthread_mutex_unlock(&gate.mutex);
  int failed = 0;
  if (pthread_join(threads[0], NULL) != 0) failed = 1;
  if (pthread_join(threads[1], NULL) != 0) failed = 1;
  if (setlocale(LC_CTYPE, "C") == NULL) failed = 1;
  if (probes[0].prefix_result != (size_t)-2 || probes[0].middle_result != 1 ||
      probes[1].prefix_result != (size_t)-2 || probes[1].middle_result != (size_t)-2 ||
      probes[1].last_result != 1)
    failed = 1;
  return failed;
}

typedef struct {
  unsigned int id;
  int failed;
} AllocThreadProbe;

static void *allocator_thread_worker(void *argument) {
  AllocThreadProbe *probe = argument;
  for (size_t iteration = 0; iteration < 2500; ++iteration) {
    size_t size = 1 + (iteration * 37 + probe->id * 13) % 7000;
    unsigned char *bytes = nelibc_malloc(size);
    if (bytes == NULL) { probe->failed = 1; return NULL; }
    bytes[0] = (unsigned char)probe->id;
    if (size > 1) bytes[size - 1] = 0xa5;
    unsigned char *grown = nelibc_realloc(bytes, size + 1);
    if (grown == NULL) {
      nelibc_free(bytes);
      probe->failed = 2;
      return NULL;
    }
    if (grown[0] != (unsigned char)probe->id ||
        (size > 1 && grown[size - 1] != 0xa5)) {
      nelibc_free(grown);
      probe->failed = 3;
      return NULL;
    }
    nelibc_free(grown);

    /* Exercise concurrent direct-map publication, ownership reads, and free. */
    unsigned char *direct = nelibc_malloc(32768 + probe->id);
    if (direct == NULL || !nelibc_allocator_owns(direct)) {
      nelibc_free(direct);
      probe->failed = 6;
      return NULL;
    }
    direct[0] = (unsigned char)probe->id;
    direct[32767] = 0x5a;
    if (direct[0] != (unsigned char)probe->id || direct[32767] != 0x5a) {
      nelibc_free(direct);
      probe->failed = 7;
      return NULL;
    }
    unsigned char *direct_grown = nelibc_realloc(direct, 65536 + probe->id);
    if (direct_grown == NULL ||
        direct_grown[0] != (unsigned char)probe->id ||
        direct_grown[32767] != 0x5a ||
        !nelibc_allocator_owns(direct_grown)) {
      nelibc_free(direct_grown);
      probe->failed = 8;
      return NULL;
    }
    direct = direct_grown;
    nelibc_free(direct);

    uint32_t *zeroed = nelibc_calloc(4, sizeof *zeroed);
    if (zeroed == NULL) { probe->failed = 4; return NULL; }
    for (size_t index = 0; index < 4; ++index) {
      if (zeroed[index] != 0) {
        nelibc_free(zeroed);
        probe->failed = 5;
        return NULL;
      }
    }
    nelibc_free(zeroed);
  }
  return NULL;
}

static int allocator_thread_isolation(void) {
  for (unsigned int wave = 0; wave < 2; ++wave) {
    pthread_t threads[4];
    AllocThreadProbe probes[4];
    size_t created = 0;
    for (; created < 4; ++created) {
      probes[created].id = (unsigned int)created + 1;
      probes[created].failed = 0;
      if (pthread_create(&threads[created], NULL, allocator_thread_worker,
                         &probes[created]) != 0)
        break;
    }
    int failed = created != 4;
    for (size_t index = 0; index < created; ++index) {
      if (pthread_join(threads[index], NULL) != 0 || probes[index].failed != 0)
        failed = 1;
    }
    if (failed) return 1;
  }
  return 0;
}

static void *concurrent_system_worker(void *unused) {
  (void)unused;
  int status = system("sleep 0.02; kill -INT $PPID; exit 0");
  return (void *)(uintptr_t)(status == -1 || !WIFEXITED(status) || WEXITSTATUS(status) != 0);
}

static void *cancelled_system_worker(void *unused) {
  (void)unused;
  (void)system("exec sleep 5");
  return NULL;
}
#endif

static int tempfile_probe(void) {
  char invalid[] = "/tmp/nelibc-invalid-XXXXXXx";
  char plain[] = "/tmp/nelibc-mkstemp-XXXXXX";
  char suffix[] = "/tmp/nelibc-mkstemps-XXXXXX.txt";
  char append[] = "/tmp/nelibc-mkostemp-XXXXXX";
  char append_suffix[] = "/tmp/nelibc-mkostemps-XXXXXX.log";
  char directory[] = "/tmp/nelibc-mkdtemp-XXXXXX";
  struct stat state;
  int descriptor = -1;
  int created_plain = 0;
  int created_suffix = 0;
  int created_append = 0;
  int created_append_suffix = 0;
  int created_directory = 0;
  int result = 0;
  char byte = 'N';

  errno = 0;
  if (mkstemp(invalid) != -1 || errno != EINVAL ||
      strcmp(invalid, "/tmp/nelibc-invalid-XXXXXXx") != 0) {
    result = 1;
    goto done;
  }
  descriptor = mkstemp(plain);
  if (descriptor < 0) { result = 2; goto done; }
  created_plain = 1;
  if (fstat(descriptor, &state) != 0 || (state.st_mode & 0077) != 0 ||
      write(descriptor, &byte, 1) != 1 || lseek(descriptor, 0, SEEK_SET) != 0 ||
      read(descriptor, &byte, 1) != 1 || byte != 'N') {
    result = 3;
    goto done;
  }
  if (close(descriptor) != 0) { descriptor = -1; result = 4; goto done; }
  descriptor = -1;

  descriptor = mkstemps(suffix, 4);
  if (descriptor < 0) { result = 5; goto done; }
  created_suffix = 1;
  if (strcmp(suffix + strlen(suffix) - 4, ".txt") != 0 ||
      fstat(descriptor, &state) != 0 || (state.st_mode & 0077) != 0) {
    result = 6;
    goto done;
  }
  if (close(descriptor) != 0) { descriptor = -1; result = 7; goto done; }
  descriptor = -1;

  descriptor = mkostemp(append, O_APPEND | O_CLOEXEC);
  if (descriptor < 0) { result = 8; goto done; }
  created_append = 1;
  if ((fcntl(descriptor, F_GETFL) & O_APPEND) == 0 ||
      (fcntl(descriptor, F_GETFD) & FD_CLOEXEC) == 0) {
    result = 9;
    goto done;
  }
  if (close(descriptor) != 0) { descriptor = -1; result = 10; goto done; }
  descriptor = -1;

  /* The GNU form accepts the same extra open flags with a preserved suffix. */
  descriptor = mkostemps(append_suffix, 4, O_CLOEXEC);
  if (descriptor < 0) { result = 11; goto done; }
  created_append_suffix = 1;
  if (strcmp(append_suffix + strlen(append_suffix) - 4, ".log") != 0 ||
      (fcntl(descriptor, F_GETFD) & FD_CLOEXEC) == 0) {
    result = 12;
    goto done;
  }
  if (close(descriptor) != 0) { descriptor = -1; result = 13; goto done; }
  descriptor = -1;

  if (mkdtemp(directory) == NULL) { result = 14; goto done; }
  created_directory = 1;
  if (stat(directory, &state) != 0 || !S_ISDIR(state.st_mode) ||
      (state.st_mode & 0077) != 0) result = 15;

done:
  if (descriptor >= 0) close(descriptor);
  if (created_plain) unlink(plain);
  if (created_suffix) unlink(suffix);
  if (created_append) unlink(append);
  if (created_append_suffix) unlink(append_suffix);
  if (created_directory) rmdir(directory);
  return result;
}

int main(int argc, char **argv) {
  char format_count_buffer[32];
  int format_count = -1;
  if (snprintf(format_count_buffer, sizeof format_count_buffer,
               "prefix%n-suffix", &format_count) != 13 || format_count != 6 ||
      strcmp(format_count_buffer, "prefix-suffix") != 0) return 553;
  if (allocator_probe() != 0) return 83;
  int semaphore_result = semaphore_clock_probe();
  if (semaphore_result != 0) return 85 + semaphore_result;
  int tempfile_result = tempfile_probe();
  if (tempfile_result != 0) return 570 + tempfile_result;
  char cwd_buffer[4096];
  if (getcwd(cwd_buffer, sizeof cwd_buffer) == NULL) return 20;
  char *allocated_cwd = get_current_dir_name();
  if (allocated_cwd == NULL || strcmp(allocated_cwd, cwd_buffer) != 0) return 21;
  free(allocated_cwd);
#ifndef NELIBC_STANDALONE
  if (archive_unistd_adapter_probe() != 0) return 84;
  if (pthread_protocol_probe() != 0) return 86;
  if (pthread_sigmask_probe() != 0) return 87;
#endif
  if (ffs(0) != 0 || ffsl(0) != 0 || ffsll(0) != 0 ||
      ffs(-1) != 1 || ffsl(-1L) != 1 || ffsll(-1LL) != 1)
    return 38;
  for (int bit = 0; bit < (int)(sizeof(int) * CHAR_BIT); ++bit)
    if (ffs((int)(1U << bit)) != bit + 1) return 39;
  for (int bit = 0; bit < (int)(sizeof(long) * CHAR_BIT); ++bit)
    if (ffsl((long)(1UL << bit)) != bit + 1) return 40;
  for (int bit = 0; bit < (int)(sizeof(long long) * CHAR_BIT); ++bit)
    if (ffsll((long long)(1ULL << bit)) != bit + 1) return 41;
  char error_text[64];
  char short_error_text[2] = {'x', 'y'};
  if (nelibc_strerror_r(2, error_text, sizeof error_text) != 0 ||
      strcmp(error_text, "No such file or directory") != 0 ||
      nelibc_strerror_r(2, short_error_text, sizeof short_error_text) != ERANGE ||
      short_error_text[0] != 'x' || short_error_text[1] != 'y' ||
      nelibc_strerror_r(41, error_text, sizeof error_text) != EINVAL)
    return 36;
#if defined(NELIBC_STANDALONE) || defined(__GLIBC__)
  char *gnu_error_text = strerror_r(2, error_text, sizeof error_text);
  char *gnu_short_error_text = strerror_r(2, short_error_text, sizeof short_error_text);
  if (gnu_error_text == NULL ||
      strcmp(gnu_error_text, "No such file or directory") != 0 ||
      gnu_short_error_text == NULL ||
      strcmp(gnu_short_error_text, "No such file or directory") != 0)
    return 42;
#else
  if (strerror_r(2, error_text, sizeof error_text) != 0 ||
      strcmp(error_text, "No such file or directory") != 0)
    return 42;
#endif
#if defined(NELIBC_STATIC_MUSL)
  if (nelibc_test_standard_strerror_r(2, error_text, sizeof error_text) != 0 ||
      strcmp(error_text, "No such file or directory") != 0)
    return 43;
#endif
  unsigned int rand_r_seed = 0x12345678U;
  unsigned int rand_r_seed_copy = rand_r_seed;
  int rand_r_first = rand_r(&rand_r_seed);
  int rand_r_second = rand_r(&rand_r_seed);
  if (rand_r_first != 477687324 || rand_r_seed_copy != 0x12345678U ||
      rand_r(&rand_r_seed_copy) != rand_r_first || rand_r_seed_copy != 2441007757U ||
      rand_r_second != 1388675377 || rand_r_seed != 281628322U)
    return 37;
  if (argc == 2 && strcmp(argv[1], "--verify-unsetenv") == 0) {
    const char *preserved = getenv("NELIBC_KEEP");
    return getenv("NELIBC_CONSUMER_SETENV_PROBE") == NULL && preserved != NULL &&
           strcmp(preserved, "present") == 0 ? 0 : 9;
  }

  const char input[] = "Nelua libc";
  if (strcasestr(input, "LIBC") != &input[6] ||
      strcasestr(input, "") != input || strcasestr(input, "missing") != NULL)
    return 86;
  if (strchrnul(input, 'l') != &input[2] ||
      strchrnul(input, 'x') != &input[sizeof input - 1] ||
      strchrnul(input, 0) != &input[sizeof input - 1] ||
      strchrnul(input, 0x16c) != &input[2]) return 85;
  char copy[sizeof input];
  memcpy(copy, input, sizeof input);
  if (strlen(copy) != sizeof input - 1 || strcmp(copy, input) != 0) return 1;
  char overlap_copy[] = "abcdefg";
  bcopy(overlap_copy, overlap_copy + 1, 6);
  if (memcmp(overlap_copy, "aabcdef", 7) != 0) return 42;
  char overlap_copy_back[] = "abcdefg";
  bcopy(overlap_copy_back + 1, overlap_copy_back, 6);
  if (memcmp(overlap_copy_back, "bcdefg", 6) != 0) return 43;
  char zeroed[9] = "12345678";
  bzero(zeroed, sizeof zeroed);
  for (size_t zero_index = 0; zero_index < sizeof zeroed; ++zero_index)
    if (zeroed[zero_index] != 0) return 44;
  if (bcmp("equal", "equal", 5) != 0 || bcmp("same", "same", 0) != 0 ||
      bcmp("left", "lest", 4) == 0)
    return 45;
  unsigned char secret_bytes[] = {0xa5, 0x5a, 0xff, 0x81, 0x00};
  explicit_bzero(secret_bytes, sizeof secret_bytes);
  for (size_t secret_index = 0; secret_index < sizeof secret_bytes; ++secret_index)
    if (secret_bytes[secret_index] != 0) return 87;
  const char indexed_bytes[] = {'a', (char)0xff, 'b', 'a', 0};
  if (index(indexed_bytes, 0xff) != &indexed_bytes[1] ||
      rindex(indexed_bytes, 'a') != &indexed_bytes[3] ||
      rindex(indexed_bytes, 0) != &indexed_bytes[4] ||
      index(indexed_bytes, 'z') != NULL)
    return 46;
  wchar_t wide_memory[] = {L'A', 0x100, L'Z', 0};
  wchar_t wide_copy[4] = {0};
  if (wmemcpy(wide_copy, wide_memory, 4) != wide_copy ||
      wmemcmp(wide_copy, wide_memory, 4) != 0 ||
      wmemchr(wide_copy, 0x100, 4) != &wide_copy[1] ||
      wmemchr(wide_copy, L'?', 4) != NULL)
    return 47;
  wmemmove(wide_copy + 1, wide_copy, 3);
  if (wide_copy[0] != L'A' || wide_copy[1] != L'A' || wide_copy[2] != 0x100 ||
      wide_copy[3] != L'Z' || wmemcmp(wide_copy + 1, wide_memory, 3) != 0)
    return 48;
  if (wmemset(wide_copy, 0x1234, 4) != wide_copy)
    return 49;
  for (size_t wide_index = 0; wide_index < 4; ++wide_index)
    if (wide_copy[wide_index] != 0x1234) return 50;
  const wchar_t wide_text[] = {L'A', 0x100, L'B', 0};
  const wchar_t wide_text_less[] = {L'A', 0xff, L'B', 0};
  wchar_t wide_string[16] = {0};
  if (wcslen(wide_text) != 3 || wcscmp(wide_text, wide_text) != 0 ||
      wcscmp(wide_text, wide_text_less) <= 0 ||
      wcsncmp(wide_text, wide_text_less, 1) != 0 ||
      wcsncmp(wide_text, wide_text_less, 2) <= 0 ||
      wcsncmp(wide_text, wide_text_less, 0) != 0)
    return 51;
  if (wcschr(wide_text, 0x100) != &wide_text[1] ||
      wcschr(wide_text, 0) != &wide_text[3] ||
      wcschr(wide_text, L'X') != NULL ||
      wcsrchr(wide_text, L'A') != &wide_text[0] ||
      wcsrchr(wide_text, 0) != &wide_text[3] ||
      wcsrchr(wide_text, L'X') != NULL)
    return 52;
  if (wcscpy(wide_string, wide_text) != wide_string ||
      wcscmp(wide_string, wide_text) != 0 ||
      wcscat(wide_string, L"!") != wide_string ||
      wcslen(wide_string) != 4 || wide_string[3] != L'!')
    return 53;
  for (size_t wide_index = 0; wide_index < 8; ++wide_index) wide_string[wide_index] = 0x777;
  if (wcsncpy(wide_string, L"xy", 5) != wide_string ||
      wide_string[0] != L'x' || wide_string[1] != L'y' ||
      wide_string[2] != 0 || wide_string[3] != 0 || wide_string[4] != 0 ||
      wide_string[5] != 0x777)
    return 54;
  wcscpy(wide_string, L"Q");
  if (wcsncat(wide_string, wide_text, 2) != wide_string ||
      wide_string[0] != L'Q' || wide_string[1] != L'A' ||
      wide_string[2] != 0x100 || wide_string[3] != 0 ||
      wcsncat(wide_string, L"ignored", 0) != wide_string || wide_string[3] != 0)
    return 55;
  const wchar_t scan_text[] = L"wide text";
  if (wcsspn(L"  wide", L" ") != 2 ||
      wcscspn(L"wide text", L"tx") != 5 ||
      wcscspn(L"wide", L"") != 4 ||
      wcspbrk(scan_text, L"tx") != &scan_text[5] ||
      wcspbrk(L"wide", L"xyz") != NULL)
    return 56;
  wchar_t token_text[] = L"  alpha beta gamma  ";
  token_text[7] = 0x100;
  const wchar_t token_delimiters[] = {L' ', 0x100, 0};
  wchar_t *token_state = NULL;
  if (wcscmp(wcstok(token_text, token_delimiters, &token_state), L"alpha") != 0 ||
      wcscmp(wcstok(NULL, token_delimiters, &token_state), L"beta") != 0 ||
      wcscmp(wcstok(NULL, token_delimiters, &token_state), L"gamma") != 0 ||
      wcstok(NULL, token_delimiters, &token_state) != NULL)
    return 57;
  wchar_t wide_haystack[] = {L'x', L'x', 0x100, L'y', L'z', 0};
  const wchar_t wide_needle[] = {0x100, L'y', 0};
  if (wcsnlen(wide_haystack, 0) != 0 || wcsnlen(wide_haystack, 2) != 2 ||
      wcsnlen(wide_haystack, 6) != 5 ||
      wcsstr(wide_haystack, wide_needle) != &wide_haystack[2] ||
      wcsstr(wide_haystack, L"") != wide_haystack ||
      wcsstr(wide_haystack, L"xyz") != NULL)
    return 58;
  wchar_t *wide_duplicate = wcsdup(wide_haystack);
  if (wide_duplicate == NULL || wcscmp(wide_duplicate, wide_haystack) != 0) return 59;
  wide_duplicate[0] = L'Q';
  if (wide_haystack[0] != L'x') {
    free(wide_duplicate);
    return 60;
  }
  free(wide_duplicate);
  wchar_t *wide_end = NULL;
  const wchar_t wide_hex[] = L" -0x2aTail";
  errno = 0;
  if (wcstol(wide_hex, &wide_end, 0) != -42 || wide_end != &wide_hex[6] || errno != 0)
    return 61;
  const wchar_t wide_binary[] = L"0b101!";
  if (nelibc_test_legacy_wcstol(wide_binary, &wide_end, 0) != 0 ||
      wide_end != &wide_binary[1]) return 74;
  if (__isoc23_wcstol(wide_binary, &wide_end, 0) != 5 ||
      wide_end != &wide_binary[5]) return 75;
  if (nelibc_test_legacy_wcstol(wide_binary, &wide_end, 2) != 0 ||
      wide_end != &wide_binary[1]) return 78;
  if (__isoc23_wcstol(wide_binary, &wide_end, 2) != 5 ||
      wide_end != &wide_binary[5]) return 79;
  errno = 0;
  if (wcstoll(L"-9223372036854775808!", &wide_end, 10) != LLONG_MIN ||
      *wide_end != L'!' || errno != 0)
    return 62;
  errno = 0;
  if (wcstoul(L"-1!", &wide_end, 10) != ULONG_MAX || *wide_end != L'!' || errno != 0)
    return 63;
  errno = 0;
  if (wcstoull(L"0xffffffffffffffff!", &wide_end, 0) != ULLONG_MAX ||
      *wide_end != L'!' || errno != 0)
    return 64;
  if (__isoc23_wcstol(L"0b101!", &wide_end, 0) != 5 || *wide_end != L'!')
    return 68;
  if (__isoc23_wcstoll(L"0B1001!", &wide_end, 0) != 9 || *wide_end != L'!')
    return 69;
  if (__isoc23_wcstoul(L"0b101!", &wide_end, 0) != 5 || *wide_end != L'!')
    return 70;
  if (__isoc23_wcstoull(L"0B1001!", &wide_end, 0) != 9 || *wide_end != L'!')
    return 71;
  errno = 0;
  if (wcstol(L"9223372036854775808!", &wide_end, 10) != LONG_MAX ||
      *wide_end != L'!' || errno != ERANGE)
    return 65;
  errno = 0;
  if (wcstol(L"+Q", &wide_end, 10) != 0 || wide_end == NULL || *wide_end != L'+' || errno != 0)
    return 66;
  errno = 0;
  const wchar_t invalid_wide_base[] = L"12";
  if (wcstol(invalid_wide_base, &wide_end, 1) != 0 ||
      wide_end != invalid_wide_base || errno != EINVAL)
    return 67;

  char *allocated = malloc(sizeof input);
  if (allocated == NULL) return 2;
  memcpy(allocated, input, sizeof input);
  if (strcmp(allocated, input) != 0) {
    free(allocated);
    return 3;
  }
  free(allocated);

  char *end = NULL;
  long value = strtol("-42tail", &end, 10);
  if (value != -42 || end == NULL || strcmp(end, "tail") != 0) return 4;
  const char binary_text[] = "0b101!";
  if (nelibc_test_legacy_strtol(binary_text, &end, 0) != 0 ||
      end != &binary_text[1]) return 72;
  if (__isoc23_strtol(binary_text, &end, 0) != 5 ||
      end != &binary_text[5]) return 73;
  if (nelibc_test_legacy_strtol(binary_text, &end, 2) != 0 ||
      end != &binary_text[1]) return 76;
  if (__isoc23_strtol(binary_text, &end, 2) != 5 ||
      end != &binary_text[5]) return 77;
  if (atof("-0x1.8p+2") != -6.0) return 5;
  char *option_args[] = {(char *)"consumer", (char *)"-ab", (char *)"-cattached",
                         (char *)"operand", NULL};
  optind = 1;
  opterr = 0;
  optopt = 0;
  if (getopt(4, option_args, "abc:") != 'a' || getopt(4, option_args, "abc:") != 'b' ||
      getopt(4, option_args, "abc:") != 'c' || optarg == NULL ||
      strcmp(optarg, "attached") != 0 || getopt(4, option_args, "abc:") != -1 || optind != 3)
    return 26;
  char *separate_args[] = {(char *)"consumer", (char *)"-c", (char *)"separate", NULL};
  optind = 0;
  if (getopt(3, separate_args, "c:") != 'c' || optarg == NULL ||
      strcmp(optarg, "separate") != 0 || optind != 3) return 30;
  char *missing_args[] = {(char *)"consumer", (char *)"-c", NULL};
  optind = 1;
  if (getopt(2, missing_args, ":c:") != ':' || optopt != 'c' || optind != 2) return 27;
  char *unknown_args[] = {(char *)"consumer", (char *)"-z", NULL};
  optind = 1;
  if (getopt(2, unknown_args, "ab") != '?' || optopt != 'z' || optind != 2) return 28;
  char *end_options_args[] = {(char *)"consumer", (char *)"-a", (char *)"--",
                              (char *)"-b", NULL};
  optind = 1;
  if (getopt(4, end_options_args, "ab") != 'a' || getopt(4, end_options_args, "ab") != -1 ||
      optind != 3) return 29;
  const struct option long_options[] = {
      {"alpha", no_argument, NULL, 'a'},
      {"beta", required_argument, NULL, 'b'},
      {NULL, 0, NULL, 0},
  };
  char *long_args[] = {(char *)"consumer", (char *)"--alph", (char *)"--beta=value", NULL};
  int long_index = -1;
  optind = 0;
  if (getopt_long(3, long_args, "ab:", long_options, &long_index) != 'a' ||
      long_index != 0 || getopt_long(3, long_args, "ab:", long_options, &long_index) != 'b' ||
      long_index != 1 || optarg == NULL || strcmp(optarg, "value") != 0 ||
      getopt_long(3, long_args, "ab:", long_options, &long_index) != -1) return 31;
  char suboptions[] = "ro=fast,rw";
  char *suboption_cursor = suboptions;
  char *suboption_value = NULL;
  char *suboption_tokens[] = {"ro", "rw", NULL};
  if (getsubopt(&suboption_cursor, suboption_tokens, &suboption_value) != 0 ||
      suboption_value == NULL || strcmp(suboption_value, "fast") != 0 ||
      strcmp(suboption_cursor, "rw") != 0 ||
      getsubopt(&suboption_cursor, suboption_tokens, &suboption_value) != 1 ||
      suboption_value != NULL || suboption_cursor == NULL || *suboption_cursor != '\0' ||
      getsubopt(&suboption_cursor, suboption_tokens, &suboption_value) != -1) return 32;
  wchar_t wide_buffer[4] = {0};
  char multibyte_buffer[4] = {0};
  if (mblen("A", 1) != 1 || mbtowc(wide_buffer, "A", 1) != 1 ||
      wide_buffer[0] != L'A' || mbstowcs(wide_buffer, "BC", 4) != 2 ||
      wide_buffer[0] != L'B' || wide_buffer[1] != L'C' || wide_buffer[2] != 0 ||
      wcstombs(multibyte_buffer, wide_buffer, sizeof multibyte_buffer) != 2 ||
      multibyte_buffer[0] != 'B' || multibyte_buffer[1] != 'C' ||
      wctomb(multibyte_buffer, L'Z') != 1 || multibyte_buffer[0] != 'Z') return 33;
  if (btowc(-1) != WEOF || btowc('Q') != L'Q' || wctob(WEOF) != -1 ||
      wctob(L'Q') != 'Q' || wctob(0xe9) != -1) return 75;
  wchar_t wide_copy_endbuf[16] = {0};
  wchar_t *wide_copy_end = wcpcpy(wide_copy_endbuf, L"copy");
  wchar_t bounded_wide_copy[16];
  for (size_t i = 0; i < 16; ++i) bounded_wide_copy[i] = 0x777;
  wchar_t *bounded_wide_end = wcpncpy(bounded_wide_copy, L"X", 4);
  const wchar_t wide_search[] = L"wide string";
  if (wide_copy_end != wide_copy_endbuf + 4 || wcscmp(wide_copy_endbuf, L"copy") != 0 ||
      bounded_wide_end != bounded_wide_copy + 1 || bounded_wide_copy[0] != L'X' ||
      bounded_wide_copy[1] != 0 || bounded_wide_copy[2] != 0 ||
      bounded_wide_copy[3] != 0 || wcswcs(wide_search, L"string") != wide_search + 5 ||
      wcscasecmp(L"AbC", L"aBc") != 0 ||
      wcsncasecmp(L"AbD", L"aBc", 2) != 0 ||
      wcsncasecmp(L"AbD", L"aBc", 3) <= 0)
    return 76;
#ifndef NELIBC_STANDALONE
  if (mbrlen_thread_isolation()) return 82;
  if (allocator_thread_isolation()) return 84;
#endif
#ifdef NELIBC_STANDALONE
  int locale_smoke_result = standalone_locale_smoke();
  if (locale_smoke_result != 0) return 84 + locale_smoke_result;
  if (setlocale(LC_CTYPE, "C.UTF-8") == NULL) return 34;
  if (btowc(0xc3) != WEOF || wctob(0xe9) != -1) return 76;
  locale_t ctype_c = newlocale(LC_CTYPE_MASK, "C", (locale_t)0);
  locale_t ctype_utf8 = newlocale(LC_CTYPE_MASK, "C.UTF-8", ctype_c);
  locale_t all_utf8 = newlocale(LC_ALL_MASK, "C.UTF-8", (locale_t)0);
  locale_t ctype_current = duplocale(LC_GLOBAL_LOCALE);
  wctype_t alpha_class = wctype("alpha");
  wctype_t space_class = wctype_l("space", ctype_c);
  if (ctype_c == (locale_t)0 || ctype_utf8 == (locale_t)0 ||
      all_utf8 == (locale_t)0 || ctype_current == (locale_t)0 ||
      alpha_class == 0 || space_class == 0 || wctype_l("alpha", all_utf8) == 0 ||
      wctype("not-a-class") != 0 || iswctype(0x391, alpha_class) == 0 ||
      iswctype_l(0x391, alpha_class, ctype_c) != 0 ||
      iswctype_l(0x391, alpha_class, ctype_utf8) == 0 ||
      iswctype_l(0xa0, space_class, ctype_c) != 0 ||
      iswctype_l(0xa0, space_class, ctype_utf8) == 0 ||
      iswalnum_l(0x391, ctype_utf8) == 0 || iswalpha_l(0x391, ctype_utf8) == 0 ||
      iswblank_l(L'\t', ctype_c) == 0 || iswcntrl_l(L'\n', ctype_utf8) == 0 ||
      iswdigit_l(L'4', ctype_c) == 0 || iswgraph_l(L'!', ctype_c) == 0 ||
      iswlower_l(0x3b1, ctype_utf8) == 0 || iswprint_l(0x3b1, ctype_utf8) == 0 ||
      iswpunct_l(0x2014, ctype_utf8) == 0 || iswspace_l(0xa0, ctype_utf8) == 0 ||
      iswupper_l(0x391, ctype_utf8) == 0 || iswxdigit_l(L'F', ctype_c) == 0 ||
      towlower_l(0x391, ctype_c) != 0x391 ||
      towlower_l(0x391, ctype_utf8) != 0x3b1 ||
      towupper_l(0x3b1, ctype_utf8) != 0x391 ||
      iswalpha_l(0x391, ctype_current) == 0)
    return 80;
  locale_t bad_locale = newlocale(LC_CTYPE_MASK, "missing_LOCALE", ctype_c);
  if (bad_locale != (locale_t)0 || errno != EINVAL) return 81;
  freelocale(ctype_current);
  freelocale(all_utf8);
  freelocale(ctype_utf8);
  freelocale(ctype_c);
  if (towlower(0x391) != 0x3b1 || towupper(0x3b1) != 0x391 ||
      towlower(0x410) != 0x430 || towupper(0x430) != 0x410 ||
      towlower(0x10400) != 0x10428 || towupper(0x10428) != 0x10400 ||
      towlower(0x1f80) != 0x1f80 || towupper(0x1f80) != 0x1f88 ||
      towlower(WEOF) != WEOF || towupper(WEOF) != WEOF)
    return 78;
  if (!iswalpha(0x391) || !iswalpha(0x2160) || !iswlower(0x3b1) ||
      !iswlower(0x345) || !iswupper(0x391) || !iswupper(0x1c5) ||
      !iswdigit(L'7') || iswdigit(0x660) || iswalpha(0x660) ||
      !iswxdigit(L'F') || !iswspace(0xa0) || iswblank(0xa0) ||
      !iswblank(L' ') || !iswblank(L'\t') ||
      !iswspace(0x2028) || iswspace(0x1c) || !iswcntrl(0x2028) ||
      !iswprint(0x200d) || iswprint(0x2028) || iswprint(0x378) ||
      iswgraph(0xa0) || iswgraph(L' ') ||
      !iswalnum(0x391) || !iswpunct(0x2014) || iswpunct(0xa0) || iswalpha(WEOF) ||
      iswprint(0x110000))
    return 79;
  const wchar_t width_text[] = {L'A', 0x301, 0x4e00, 0};
  if (wcwidth(L'A') != 1 || wcwidth(0x301) != 0 || wcwidth(0x4e00) != 2 ||
      wcswidth(width_text, 3) != 3 || wcswidth(width_text, 2) != 1 ||
      wcwidth(L'\t') != -1 || wcwidth(0x0378) != -1 ||
      wcwidth(0xe000) != 1 || wcwidth(0x00ad) != 1 ||
      wcwidth(0x0600) != 1 || wcwidth(0x115f) != 2 ||
      wcwidth(0x1160) != 0 || wcwidth(0x3164) != 0 ||
      wcwidth(0xffa0) != 0 || wcwidth(0x2028) != -1)
    return 77;
  const char utf8_text[] = "\xc3\xa9";
  wchar_t unicode_character = 0;
  char utf8_output[4] = {0};
  if (mbtowc(&unicode_character, utf8_text, sizeof utf8_text - 1) != 2 ||
      unicode_character != 0xe9 || wcstombs(utf8_output, &unicode_character, 2) != 2 ||
      (unsigned char)utf8_output[0] != 0xc3 || (unsigned char)utf8_output[1] != 0xa9)
    return 35;
  mbstate_t decode_state = {0};
  const char utf8_lead[] = {(char)0xc3, 0};
  const char utf8_tail[] = {(char)0xa9, 0};
  mbstate_t length_state = {0};
  if (mbsinit(NULL) == 0 || mbsinit(&length_state) == 0 ||
      mbrlen(utf8_lead, 1, &length_state) != (size_t)-2 ||
      mbsinit(&length_state) != 0 || mbrlen(utf8_tail, 1, &length_state) != 1 ||
      mbsinit(&length_state) == 0)
    return 74;
  errno = 0;
  if (mbrlen(utf8_lead, 1, NULL) != (size_t)-2 || errno != 0) return 80;
  errno = 0;
  if (mbrlen(NULL, 0, NULL) != (size_t)-1 || errno != EILSEQ) return 81;
  if (mbrtowc(&unicode_character, utf8_lead, 1, &decode_state) != (size_t)-2 ||
      mbrtowc(&unicode_character, utf8_tail, 1, &decode_state) != 1 ||
      unicode_character != 0xe9)
    return 68;
  mbstate_t null_input_state = {0};
  if (mbrtowc(NULL, NULL, 0, &null_input_state) != 0 ||
      mbrtowc(&unicode_character, utf8_lead, 1, &null_input_state) != (size_t)-2)
    return 77;
  errno = 0;
  if (mbrtowc(NULL, NULL, 0, &null_input_state) != (size_t)-1 || errno != EILSEQ)
    return 78;
  null_input_state = (mbstate_t){0};
  if (mbrtowc(&unicode_character, utf8_text, sizeof utf8_text - 1,
          &null_input_state) != 2 || unicode_character != 0xe9)
    return 79;
  mbstate_t encode_state = {0};
  if (wcrtomb(utf8_output, 0x1f642, &encode_state) != 4 ||
      (unsigned char)utf8_output[0] != 0xf0 || (unsigned char)utf8_output[1] != 0x9f ||
      (unsigned char)utf8_output[2] != 0x99 || (unsigned char)utf8_output[3] != 0x82 ||
      wcrtomb(NULL, 0x1f642, &encode_state) != 1)
    return 69;
  const char *restart_decode = utf8_text;
  wchar_t restart_wide[2] = {0};
  mbstate_t restart_decode_state = {0};
  if (mbsrtowcs(NULL, &restart_decode, 0, &restart_decode_state) != 1 ||
      restart_decode != utf8_text ||
      mbsrtowcs(restart_wide, &restart_decode, 1, &restart_decode_state) != 1 ||
      restart_wide[0] != 0xe9 || restart_decode == NULL || *restart_decode != 0)
    return 70;
  const wchar_t *restart_encode = restart_wide;
  char restart_bytes[4] = {0};
  mbstate_t restart_encode_state = {0};
  if (wcsrtombs(NULL, &restart_encode, 0, &restart_encode_state) != 2 ||
      restart_encode != restart_wide ||
      wcsrtombs(restart_bytes, &restart_encode, sizeof restart_bytes,
          &restart_encode_state) != 2 ||
      (unsigned char)restart_bytes[0] != 0xc3 || (unsigned char)restart_bytes[1] != 0xa9 ||
      restart_encode != NULL)
    return 71;
  const char split_text[] = {(char)0xc3, (char)0xa9, 0};
  const char *split_cursor = split_text;
  wchar_t split_wide[2] = {0x777, 0x777};
  mbstate_t split_state = {0};
  if (mbsnrtowcs(split_wide, &split_cursor, 1, 2, &split_state) != 0 ||
      split_cursor != split_text + 1 || split_wide[0] != 0x777 ||
      mbsnrtowcs(split_wide, &split_cursor, 2, 2, &split_state) != 1 ||
      split_wide[0] != 0xe9 || split_cursor != NULL)
    return 72;
  const wchar_t bounded_wide[] = {L'A', 0xe9, 0};
  const wchar_t *bounded_wide_cursor = bounded_wide;
  mbstate_t bounded_encode_state = {0};
  char bounded_output[8] = {0};
  if (wcsnrtombs(NULL, &bounded_wide_cursor, 2, 0, &bounded_encode_state) != 3 ||
      bounded_wide_cursor != bounded_wide ||
      wcsnrtombs(bounded_output, &bounded_wide_cursor, 1, sizeof bounded_output,
          &bounded_encode_state) != 1 || bounded_wide_cursor != bounded_wide + 1 ||
      wcsnrtombs(bounded_output + 1, &bounded_wide_cursor, 2,
          sizeof bounded_output - 1, &bounded_encode_state) != 2 ||
      (unsigned char)bounded_output[0] != 'A' ||
      (unsigned char)bounded_output[1] != 0xc3 ||
      (unsigned char)bounded_output[2] != 0xa9 || bounded_output[3] != 0 ||
      bounded_wide_cursor != NULL)
    return 73;
  if (setlocale(LC_CTYPE, "C") == NULL || mbtowc(&unicode_character, utf8_text, 2) != -1 ||
      wcwidth(0x4e00) != -1 || wcwidth(L'A') != 1 ||
      wcswidth(width_text, 3) != -1 || towlower(L'A') != L'a' ||
      towupper(L'a') != L'A' || towlower(0x391) != 0x391 ||
      towupper(0x3b1) != 0x3b1 || iswalpha(L'A') == 0 ||
      iswalpha(0x391) != 0 || iswspace(L' ') == 0 || iswspace(0xa0) != 0)
    return 35;
#endif
  if (system(NULL) == 0) return 10;
  int shell_status;
  struct sigaction sigint_before, sigint_after, sigquit_before, sigquit_after;
  sigset_t mask_before, mask_after;
  if (sigaction(SIGINT, NULL, &sigint_before) != 0 ||
      sigaction(SIGQUIT, NULL, &sigquit_before) != 0 ||
      sigprocmask(SIG_BLOCK, NULL, &mask_before) != 0) return 13;
  if (sigismember(&mask_before, SIGCHLD) != 0) return 14;
  shell_status = system("kill -INT $PPID; kill -QUIT $PPID; exit 0");
  if (shell_status == -1 || !WIFEXITED(shell_status) || WEXITSTATUS(shell_status) != 0) return 15;
  if (sigaction(SIGINT, NULL, &sigint_after) != 0 ||
      sigaction(SIGQUIT, NULL, &sigquit_after) != 0 ||
      sigprocmask(SIG_BLOCK, NULL, &mask_after) != 0 ||
      sigint_after.sa_handler != sigint_before.sa_handler ||
      sigquit_after.sa_handler != sigquit_before.sa_handler ||
      sigismember(&mask_after, SIGCHLD) != sigismember(&mask_before, SIGCHLD)) return 16;
  struct sigaction custom_action = {0};
  custom_action.sa_handler = system_signal_probe;
  if (sigemptyset(&custom_action.sa_mask) != 0 ||
      sigaction(SIGINT, &custom_action, NULL) != 0) return 17;
  shell_status = system("exit 0");
  if (shell_status == -1 || !WIFEXITED(shell_status) || WEXITSTATUS(shell_status) != 0 ||
      sigaction(SIGINT, NULL, &sigint_after) != 0 ||
      sigint_after.sa_handler != system_signal_probe ||
      sigaction(SIGINT, &sigint_before, NULL) != 0) return 18;
#ifndef NELIBC_STANDALONE
  pthread_t system_threads[2];
  if (pthread_create(&system_threads[0], NULL, concurrent_system_worker, NULL) != 0 ||
      pthread_create(&system_threads[1], NULL, concurrent_system_worker, NULL) != 0) return 19;
  void *system_thread_result;
  if (pthread_join(system_threads[0], &system_thread_result) != 0 || system_thread_result != NULL ||
      pthread_join(system_threads[1], &system_thread_result) != 0 || system_thread_result != NULL) return 20;
  pthread_t cancelled_thread;
  if (pthread_create(&cancelled_thread, NULL, cancelled_system_worker, NULL) != 0) return 21;
  struct timespec cancellation_delay = {.tv_sec = 0, .tv_nsec = 50000000};
  nanosleep(&cancellation_delay, NULL);
  if (pthread_cancel(cancelled_thread) != 0 ||
      pthread_join(cancelled_thread, &system_thread_result) != 0 ||
      system_thread_result != PTHREAD_CANCELED) return 22;
  errno = 0;
  if (waitpid(-1, NULL, WNOHANG) != -1 || errno != ECHILD) return 25;
  if (sigaction(SIGINT, NULL, &sigint_after) != 0 ||
      sigaction(SIGQUIT, NULL, &sigquit_after) != 0 ||
      sigprocmask(SIG_BLOCK, NULL, &mask_after) != 0 ||
      sigint_after.sa_handler != sigint_before.sa_handler ||
      sigquit_after.sa_handler != sigquit_before.sa_handler ||
      sigismember(&mask_after, SIGCHLD) != sigismember(&mask_before, SIGCHLD)) return 23;
  shell_status = system("kill -INT $PPID; exit 0");
  if (shell_status == -1 || !WIFEXITED(shell_status) || WEXITSTATUS(shell_status) != 0) return 24;
#endif
  shell_status = system("exit 23");
  if (shell_status == -1 || !WIFEXITED(shell_status) || WEXITSTATUS(shell_status) != 23) return 11;
  if (setenv("NELIBC_CONSUMER_SETENV_PROBE", "first", 1) != 0 ||
      strcmp(getenv("NELIBC_CONSUMER_SETENV_PROBE"), "first") != 0 ||
      setenv("NELIBC_CONSUMER_SETENV_PROBE", "ignored", 0) != 0 ||
      strcmp(getenv("NELIBC_CONSUMER_SETENV_PROBE"), "first") != 0 ||
      setenv("NELIBC_CONSUMER_SETENV_PROBE", "second", 1) != 0 ||
      strcmp(getenv("NELIBC_CONSUMER_SETENV_PROBE"), "second") != 0) return 6;
  shell_status = system("test \"$NELIBC_CONSUMER_SETENV_PROBE\" = second");
  if (shell_status == -1 || !WIFEXITED(shell_status) || WEXITSTATUS(shell_status) != 0) return 12;
  if (unsetenv("NELIBC_CONSUMER_SETENV_PROBE") != 0 ||
      getenv("NELIBC_CONSUMER_SETENV_PROBE") != NULL ||
      unsetenv("NELIBC_CONSUMER_SETENV_PROBE") != 0) return 7;
  errno = 0;
  if (unsetenv("BAD=NAME") != -1 || errno != EINVAL) return 8;
  char caller_environment[] = "NELIBC_PUTENV_PROBE=initial";
  char *caller_value = strchr(caller_environment, '=') + 1;
  if (putenv(caller_environment) != 0 || getenv("NELIBC_PUTENV_PROBE") != caller_value)
    return 13;
  if (setenv("NELIBC_SETENV_AFTER_PUTENV", "kept", 1) != 0) return 14;
  strcpy(caller_value, "changed");
  if (strcmp(getenv("NELIBC_PUTENV_PROBE"), "changed") != 0) return 15;
  if (putenv("NELIBC_PUTENV_PROBE") != 0 ||
      getenv("NELIBC_PUTENV_PROBE") != NULL) return 16;
  errno = 0;
  if (putenv("=invalid") != -1 || errno != EINVAL) return 17;
  static char duplicate_first[] = "NELIBC_DUPLICATE=one";
  static char preserved_entry[] = "NELIBC_KEEP=present";
  static char duplicate_second[] = "NELIBC_DUPLICATE=two";
  char *duplicate_environment[] = {
      duplicate_first, preserved_entry, duplicate_second, NULL};
  environ = duplicate_environment;
  if (unsetenv("NELIBC_DUPLICATE") != 0 ||
      getenv("NELIBC_DUPLICATE") != NULL || environ[0] == NULL ||
      strcmp(environ[0], "NELIBC_KEEP=present") != 0 || environ[1] != NULL)
    return 11;
  if (clearenv() != 0 || environ != NULL || getenv("NELIBC_KEEP") != NULL)
    return 18;
  if (setenv("NELIBC_KEEP", "present", 1) != 0) return 19;
  char *child_argv[] = {argv[0], "--verify-unsetenv", NULL};
  execve(argv[0], child_argv, environ);
  return 12;
}
