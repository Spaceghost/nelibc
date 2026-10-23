#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/eventfd.h>
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
#define ITERATIONS 100000u

static double now_seconds(void) {
  struct timespec ts;
  if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) return 0.0;
  return (double)ts.tv_sec + (double)ts.tv_nsec / 1000000000.0;
}

static void sort(double values[SAMPLES]) {
  for (size_t i = 1; i < SAMPLES; ++i) {
    double value = values[i];
    size_t j = i;
    while (j != 0 && values[j - 1] > value) {
      values[j] = values[j - 1];
      --j;
    }
    values[j] = value;
  }
}

static int report_create_close(void) {
  int (*volatile eventfd_call)(unsigned int, int) = eventfd;
  int (*volatile close_call)(int) = close;
  double samples[SAMPLES];
  for (size_t sample = 0; sample < SAMPLES; ++sample) {
    double start = now_seconds();
    for (size_t iteration = 0; iteration < ITERATIONS; ++iteration) {
      int fd = eventfd_call(0, EFD_CLOEXEC | EFD_NONBLOCK);
      if (fd < 0 || close_call(fd) != 0) {
        fprintf(stderr, "eventfd create/close failed: errno=%d\n", errno);
        return 1;
      }
    }
    samples[sample] = (now_seconds() - start) * 1e9 / ITERATIONS;
  }
  sort(samples);
  printf("%s,%s,%s,eventfd-create-close,%u,%.2f\n", BENCH_COMPILER,
         BENCH_LIBC, BENCH_IMPLEMENTATION, ITERATIONS, samples[SAMPLES / 2]);
  return 0;
}

static int report_counter_roundtrip(void) {
  int (*volatile eventfd_call)(unsigned int, int) = eventfd;
  int (*volatile close_call)(int) = close;
  int (*volatile write_call)(int, eventfd_t) = eventfd_write;
  int (*volatile read_call)(int, eventfd_t *) = eventfd_read;
  int fd = eventfd_call(0, EFD_CLOEXEC | EFD_NONBLOCK);
  if (fd < 0) {
    fprintf(stderr, "eventfd setup failed: errno=%d\n", errno);
    return 1;
  }

  double samples[SAMPLES];
  for (size_t sample = 0; sample < SAMPLES; ++sample) {
    double start = now_seconds();
    for (size_t iteration = 0; iteration < ITERATIONS; ++iteration) {
      eventfd_t value = 0;
      if (write_call(fd, 1) != 0 || read_call(fd, &value) != 0 || value != 1) {
        fprintf(stderr, "eventfd roundtrip failed: errno=%d value=%llu\n", errno,
                (unsigned long long)value);
        close_call(fd);
        return 1;
      }
    }
    samples[sample] = (now_seconds() - start) * 1e9 / ITERATIONS;
  }
  if (close_call(fd) != 0) return 1;
  sort(samples);
  printf("%s,%s,%s,eventfd-write-read,%u,%.2f\n", BENCH_COMPILER,
         BENCH_LIBC, BENCH_IMPLEMENTATION, ITERATIONS, samples[SAMPLES / 2]);
  return 0;
}

int main(void) {
  if (report_create_close() != 0) return 1;
  if (report_counter_roundtrip() != 0) return 1;
  return 0;
}
