#define _POSIX_C_SOURCE 200809L
#include "string.h"
#include <stdint.h>
#include <stdio.h>
#include <time.h>

#ifndef BENCH_COMPILER
#define BENCH_COMPILER "unspecified"
#endif
#ifndef BENCH_LIBC
#define BENCH_LIBC "unspecified"
#endif
#ifndef BENCH_IMPLEMENTATION
#define BENCH_IMPLEMENTATION "unspecified"
#endif

#define SAMPLES 7
#define WORK_BYTES (64u * 1024u * 1024u)

typedef int (*compare_fn)(const void *, const void *, size_t);

static unsigned char left[65536] __attribute__((aligned(64)));
static unsigned char right[65536] __attribute__((aligned(64)));
static volatile uintptr_t sink;

static double now_seconds(void) {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
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

static double median_compare(compare_fn fn, size_t size, size_t iterations) {
  double samples[SAMPLES];
  for (size_t sample = 0; sample < SAMPLES; ++sample) {
    uintptr_t local = 0;
    double start = now_seconds();
    for (size_t i = 0; i < iterations; ++i) local += (uintptr_t)fn(left, right, size);
    samples[sample] = (now_seconds() - start) * 1e9 / (double)iterations;
    sink ^= local;
  }
  sort(samples);
  return samples[SAMPLES / 2];
}

int main(void) {
  static const size_t sizes[] = {64, 4096, 65536};
  static const char *const cases[] = {
      "memcmp", "memcmp-first-diff", "memcmp-middle-diff", "memcmp-last-diff"};
  for (size_t i = 0; i < sizeof left; ++i) left[i] = (unsigned char)(i * 37u + 11u);

  puts("compiler,libc,implementation,function,size_bytes,iterations,median_ns_per_call");
  for (size_t si = 0; si < sizeof sizes / sizeof sizes[0]; ++si) {
    size_t size = sizes[si];
    size_t iterations = WORK_BYTES / size;
    for (size_t i = 0; i < size; ++i) right[i] = left[i];

    printf("%s,%s,%s,%s,%zu,%zu,%.2f\n", BENCH_COMPILER, BENCH_LIBC,
           BENCH_IMPLEMENTATION, cases[0], size, iterations,
           median_compare(memcmp, size, iterations));

    size_t positions[] = {0, size / 2, size - 1};
    for (size_t position = 0; position < 3; ++position) {
      size_t offset = positions[position];
      right[offset] ^= 1;
      printf("%s,%s,%s,%s,%zu,%zu,%.2f\n", BENCH_COMPILER, BENCH_LIBC,
             BENCH_IMPLEMENTATION, cases[position + 1], size, iterations,
             median_compare(memcmp, size, iterations));
      right[offset] ^= 1;
    }
  }
  return sink == SIZE_MAX;
}
