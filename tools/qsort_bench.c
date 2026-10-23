#define _POSIX_C_SOURCE 200809L
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
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
#define MAX_ITEMS 4096
#define SEARCH_ITERATIONS 1000000u

static void (* volatile qsort_call)(void *, size_t, size_t,
                                     int (*)(const void *, const void *)) = qsort;
static void *(* volatile bsearch_call)(const void *, const void *, size_t,
                                        size_t,
                                        int (*)(const void *, const void *)) = bsearch;
static int source[MAX_ITEMS];
static int work[MAX_ITEMS];
static volatile unsigned int sink;

static int compare_int(const void *left, const void *right) {
  int a = *(const int *)left;
  int b = *(const int *)right;
  return (a > b) - (a < b);
}

static double now_seconds(void) {
  struct timespec ts;
  if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) return 0.0;
  return (double)ts.tv_sec + (double)ts.tv_nsec / 1e9;
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

static double bench_qsort(size_t count, unsigned int iterations) {
  double samples[SAMPLES];
  for (size_t sample = 0; sample < SAMPLES; ++sample) {
    unsigned int local = 0;
    double start = now_seconds();
    for (unsigned int iteration = 0; iteration < iterations; ++iteration) {
      for (size_t i = 0; i < count; ++i) work[i] = source[i];
      qsort_call(work, count, sizeof work[0], compare_int);
      local ^= (unsigned int)work[count / 2];
    }
    samples[sample] = (now_seconds() - start) * 1e9 / iterations;
    sink ^= local;
  }
  sort(samples);
  return samples[SAMPLES / 2];
}

static double bench_bsearch(unsigned int iterations) {
  double samples[SAMPLES];
  for (size_t sample = 0; sample < SAMPLES; ++sample) {
    unsigned int local = 0;
    double start = now_seconds();
    for (unsigned int iteration = 0; iteration < iterations; ++iteration) {
      int key = (int)(iteration % (MAX_ITEMS * 2u)) - MAX_ITEMS / 2;
      int *found = bsearch_call(&key, work, MAX_ITEMS, sizeof work[0], compare_int);
      local ^= found ? (unsigned int)*found : UINT32_C(0x9e3779b9);
    }
    samples[sample] = (now_seconds() - start) * 1e9 / iterations;
    sink ^= local;
  }
  sort(samples);
  return samples[SAMPLES / 2];
}

static void report(const char *function, unsigned int iterations, double median) {
  printf("%s,%s,%s,%s,%u,%.2f\n", BENCH_COMPILER, BENCH_LIBC,
         BENCH_IMPLEMENTATION, function, iterations, median);
}

int main(void) {
  uint32_t state = UINT32_C(0x51534f52);
  for (size_t i = 0; i < MAX_ITEMS; ++i) {
    state = state * UINT32_C(1664525) + UINT32_C(1013904223);
    source[i] = (int)(state % (MAX_ITEMS * 4u)) - MAX_ITEMS * 2;
  }
  for (size_t i = 0; i < MAX_ITEMS; ++i) work[i] = source[i];
  qsort_call(work, MAX_ITEMS, sizeof work[0], compare_int);

  puts("compiler,libc,implementation,function,iterations,median_ns_per_call");
  report("qsort-16", 40000u, bench_qsort(16, 40000u));
  report("qsort-256", 4000u, bench_qsort(256, 4000u));
  report("qsort-1024", 1000u, bench_qsort(1024, 1000u));
  report("qsort-2048", 500u, bench_qsort(2048, 500u));
  report("qsort-4096", 250u, bench_qsort(MAX_ITEMS, 250u));
  report("bsearch-4096", SEARCH_ITERATIONS, bench_bsearch(SEARCH_ITERATIONS));
  return sink == UINT32_C(0xdeadbeef);
}
