#define _GNU_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

#ifndef BENCH_COMPILER
#define BENCH_COMPILER "unspecified"
#endif
#ifndef BENCH_LIBC
#define BENCH_LIBC "unspecified"
#endif
#ifndef BENCH_IMPLEMENTATION
#define BENCH_IMPLEMENTATION "host-libc"
#endif

#define SAMPLES 7
#define FIXED_ITERATIONS 250000u
#define DYNAMIC_ITERATIONS 10000u

typedef struct {
  const char *name;
  int selector;
  unsigned iterations;
} Query;

static const Query queries[] = {
  {"arg-max", _SC_ARG_MAX, DYNAMIC_ITERATIONS},
  {"child-max", _SC_CHILD_MAX, DYNAMIC_ITERATIONS},
  {"clock-ticks", _SC_CLK_TCK, FIXED_ITERATIONS},
  {"open-max", _SC_OPEN_MAX, DYNAMIC_ITERATIONS},
  {"page-size", _SC_PAGESIZE, FIXED_ITERATIONS},
  {"groups-max", _SC_NGROUPS_MAX, FIXED_ITERATIONS},
  {"processors-conf", _SC_NPROCESSORS_CONF, DYNAMIC_ITERATIONS},
  {"processors-online", _SC_NPROCESSORS_ONLN, DYNAMIC_ITERATIONS},
  {"physical-pages", _SC_PHYS_PAGES, DYNAMIC_ITERATIONS},
  {"available-pages", _SC_AVPHYS_PAGES, DYNAMIC_ITERATIONS},
  {"stream-max", _SC_STREAM_MAX, FIXED_ITERATIONS},
  {"tzname-max", _SC_TZNAME_MAX, FIXED_ITERATIONS},
  {"posix-version", _SC_VERSION, FIXED_ITERATIONS},
  {"realtime-signals", _SC_REALTIME_SIGNALS, FIXED_ITERATIONS},
  {"async-io", _SC_ASYNCHRONOUS_IO, FIXED_ITERATIONS},
  {"delaytimer-max", _SC_DELAYTIMER_MAX, FIXED_ITERATIONS},
  {"mq-priority-max", _SC_MQ_PRIO_MAX, FIXED_ITERATIONS},
  {"signal-queue-max", _SC_SIGQUEUE_MAX, FIXED_ITERATIONS},
  {"sem-nsems-max", _SC_SEM_NSEMS_MAX, FIXED_ITERATIONS},
  {"sem-value-max", _SC_SEM_VALUE_MAX, FIXED_ITERATIONS},
  {"bc-base-max", _SC_BC_BASE_MAX, FIXED_ITERATIONS},
  {"bc-dim-max", _SC_BC_DIM_MAX, FIXED_ITERATIONS},
  {"bc-scale-max", _SC_BC_SCALE_MAX, FIXED_ITERATIONS},
  {"bc-string-max", _SC_BC_STRING_MAX, FIXED_ITERATIONS},
  {"coll-weights-max", _SC_COLL_WEIGHTS_MAX, FIXED_ITERATIONS},
  {"regex-repeat-max", _SC_RE_DUP_MAX, FIXED_ITERATIONS},
  {"iov-max", _SC_IOV_MAX, FIXED_ITERATIONS},
  {"login-name-max", _SC_LOGIN_NAME_MAX, FIXED_ITERATIONS},
  {"tty-name-max", _SC_TTY_NAME_MAX, FIXED_ITERATIONS},
  {"thread-destructors", _SC_THREAD_DESTRUCTOR_ITERATIONS, FIXED_ITERATIONS},
  {"thread-keys-max", _SC_THREAD_KEYS_MAX, FIXED_ITERATIONS},
  {"thread-stack-min", _SC_THREAD_STACK_MIN, FIXED_ITERATIONS},
  {"thread-count-max", _SC_THREAD_THREADS_MAX, FIXED_ITERATIONS},
  {"xopen-version", _SC_XOPEN_VERSION, FIXED_ITERATIONS},
  {"xopen-unix", _SC_XOPEN_UNIX, FIXED_ITERATIONS},
  {"nzero", _SC_NZERO, FIXED_ITERATIONS},
  {"symlink-max", _SC_SYMLOOP_MAX, FIXED_ITERATIONS},
  {"hostname-max", _SC_HOST_NAME_MAX, FIXED_ITERATIONS},
  {"ipv6", _SC_IPV6, FIXED_ITERATIONS},
  {"raw-sockets", _SC_RAW_SOCKETS, FIXED_ITERATIONS},
};

static double now_seconds(void) {
  struct timespec ts;
  if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) return 0.0;
  return (double)ts.tv_sec + (double)ts.tv_nsec / 1000000000.0;
}

static void sort(double values[SAMPLES]) {
  for (size_t i = 1; i < SAMPLES; ++i) {
    double value = values[i];
    size_t j = i;
    while (j && values[j - 1] > value) {
      values[j] = values[j - 1];
      --j;
    }
    values[j] = value;
  }
}

static volatile long sink;

static int measure(const Query *query) {
  long (*volatile call)(int) = sysconf;
  errno = 0;
  long value = call(query->selector);
  if (value < 0 && errno != 0) {
    fprintf(stderr, "%s failed: errno=%d\n", query->name, errno);
    return 1;
  }
  double samples[SAMPLES];
  for (size_t sample = 0; sample < SAMPLES; ++sample) {
    long local = 0;
    errno = 0;
    double start = now_seconds();
    for (unsigned iteration = 0; iteration < query->iterations; ++iteration) {
      long result = call(query->selector);
      if (result < 0 && errno != 0) {
        fprintf(stderr, "%s failed during timing: errno=%d\n", query->name, errno);
        return 1;
      }
      local += result;
    }
    samples[sample] = (now_seconds() - start) * 1e9 / query->iterations;
    sink ^= local;
  }
  sort(samples);
  printf("%s,%s,%s,%s,%ld,%u,%.2f\n", BENCH_COMPILER, BENCH_LIBC,
         BENCH_IMPLEMENTATION, query->name, value, query->iterations,
         samples[SAMPLES / 2]);
  return 0;
}

int main(void) {
  puts("compiler,libc,implementation,query,value,iterations,median_ns_per_call");
  for (size_t i = 0; i < sizeof(queries) / sizeof(queries[0]); ++i)
    if (measure(&queries[i]) != 0) return 1;
  return 0;
}
