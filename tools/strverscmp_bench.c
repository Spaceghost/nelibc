#define _GNU_SOURCE
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
#define BENCH_IMPLEMENTATION "libc"
#endif

typedef int (*cmp_fn)(const char *, const char *);
static volatile int sink;
static const char *pairs[][2] = {
  {"file2", "file10"}, {"file1", "file01"}, {"x0", "x00"},
  {"a2b3", "a2b12"}, {"release-2026.10", "release-2026.9"},
  {"alpha", "alphabet"}, {"v00012", "v12"}, {"build9rc2", "build9rc10"}
};
static const int expected[] = {-1, 1, 1, -1, 1, -1, -1, -1};

static double now(void) {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return (double)ts.tv_sec + (double)ts.tv_nsec / 1e9;
}

int main(void) {
  enum { SAMPLES = 7, REPS = 1000000 };
  cmp_fn volatile fn = strverscmp;
  double samples[SAMPLES];
  for (size_t i = 0; i < sizeof(pairs) / sizeof(pairs[0]); ++i) {
    int result = fn(pairs[i][0], pairs[i][1]);
    int sign = (result > 0) - (result < 0);
    if (sign != expected[i]) {
      fprintf(stderr, "strverscmp mismatch: %s / %s: got %d expected %d\n",
              pairs[i][0], pairs[i][1], sign, expected[i]);
      return 1;
    }
  }
  for (size_t s = 0; s < SAMPLES; ++s) {
    int total = 0;
    double start = now();
    for (size_t i = 0; i < REPS; ++i) {
      size_t p = i & 7;
      total += fn(pairs[p][0], pairs[p][1]);
    }
    samples[s] = (now() - start) * 1e9 / REPS;
    sink ^= total;
  }
  for (size_t i = 1; i < SAMPLES; ++i) {
    double v = samples[i]; size_t j = i;
    while (j && samples[j - 1] > v) { samples[j] = samples[j - 1]; --j; }
    samples[j] = v;
  }
  printf("compiler,libc,implementation,function,iterations,median_ns_per_call\n");
  printf("%s,%s,%s,strverscmp,%d,%.2f\n", BENCH_COMPILER, BENCH_LIBC,
         BENCH_IMPLEMENTATION, REPS, samples[SAMPLES / 2]);
  return sink == 123456789;
}
