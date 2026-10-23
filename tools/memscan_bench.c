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

#define MAX_SIZE 65536
#define SAMPLES 7
#define WORK_BYTES (8u * 1024u * 1024u)

typedef void *(*find_fn)(const void *, int, size_t);
static unsigned char haystack[MAX_SIZE] __attribute__((aligned(64)));
static volatile uintptr_t sink;

static double now_seconds(void) {
  struct timespec ts;
  if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) return 0.0;
  return (double)ts.tv_sec + (double)ts.tv_nsec / 1000000000.0;
}

static void sort(double values[SAMPLES]) {
  for (size_t i = 1; i < SAMPLES; ++i) {
    double value = values[i];
    size_t j = i;
    while (j > 0 && values[j - 1] > value) {
      values[j] = values[j - 1];
      --j;
    }
    values[j] = value;
  }
}

static double measure(find_fn fn, size_t size, int target, size_t position,
                      int absent) {
  size_t reps = WORK_BYTES / size;
  if (reps < 128) reps = 128;
  double samples[SAMPLES];
  for (size_t sample = 0; sample < SAMPLES; ++sample) {
    for (size_t i = 0; i < size; ++i) haystack[i] = 0x11;
    if (!absent) haystack[position] = (unsigned char)target;
    uintptr_t local = 0;
    double start = now_seconds();
    for (size_t i = 0; i < reps; ++i) {
      local ^= (uintptr_t)fn(haystack, target, size);
    }
    samples[sample] = (now_seconds() - start) * 1e9 / (double)reps;
    sink ^= local;
  }
  sort(samples);
  return samples[SAMPLES / 2];
}

int main(void) {
  static const size_t sizes[] = {64, 1024, 2048, 4096, 8192, 65536};
  static const char *case_names[] = {"first", "middle", "last", "absent"};
  static find_fn functions[] = {memchr, memrchr};
  static const char *function_names[] = {"memchr", "memrchr"};
  puts("compiler,libc,implementation,function,size_bytes,case,iterations,median_ns_per_call");
  for (size_t si = 0; si < sizeof sizes / sizeof sizes[0]; ++si) {
    size_t size = sizes[si];
    size_t positions[] = {0, size / 2, size - 1, 0};
    size_t reps = WORK_BYTES / size;
    if (reps < 128) reps = 128;
    for (size_t fi = 0; fi < 2; ++fi) {
      for (size_t ci = 0; ci < 4; ++ci) {
        double ns = measure(functions[fi], size, 0xE7, positions[ci], ci == 3);
        printf("%s,%s,%s,%s,%zu,%s,%zu,%.2f\n", BENCH_COMPILER,
               BENCH_LIBC, BENCH_IMPLEMENTATION, function_names[fi], size,
               case_names[ci], reps, ns);
      }
    }
  }
  return sink == UINTPTR_MAX;
}
