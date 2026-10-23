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

static int measure(size_t request_size) {
  int (*volatile getentropy_call)(void *, size_t) = getentropy;
  unsigned char buffer[256];
  double samples[SAMPLES];
  for (size_t sample = 0; sample < SAMPLES; ++sample) {
    double start = now_seconds();
    for (size_t iteration = 0; iteration < ITERATIONS; ++iteration) {
      if (getentropy_call(buffer, request_size) != 0) {
        fprintf(stderr, "getentropy failed: errno=%d\n", errno);
        return 1;
      }
    }
    samples[sample] = (now_seconds() - start) * 1e9 / ITERATIONS;
  }
  sort(samples);
  printf("%s,%s,%s,getentropy-%zu-bytes,%u,%.2f\n", BENCH_COMPILER,
         BENCH_LIBC, BENCH_IMPLEMENTATION, request_size, ITERATIONS,
         samples[SAMPLES / 2]);
  return 0;
}

int main(void) {
  if (measure(32) != 0) return 1;
  if (measure(256) != 0) return 1;
  return 0;
}
