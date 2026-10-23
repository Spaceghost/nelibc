#define _POSIX_C_SOURCE 200809L
#include "nelibc_stdlib.h"
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
#define ITERATIONS 1000000
typedef int (*rand_r_fn)(unsigned int *);
static volatile unsigned int sink;

static double now_seconds(void) {
  struct timespec ts;
  if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) return 0.0;
  return (double)ts.tv_sec + (double)ts.tv_nsec / 1e9;
}

static double median_ns(rand_r_fn function) {
  double samples[SAMPLES];
  for (size_t sample = 0; sample < SAMPLES; ++sample) {
    unsigned int state = 0x12345678U;
    unsigned int local = 0;
    double start = now_seconds();
    for (size_t i = 0; i < ITERATIONS; ++i) local ^= (unsigned int)function(&state);
    samples[sample] = (now_seconds() - start) * 1e9 / ITERATIONS;
    sink ^= local ^ state;
  }
  for (size_t i = 1; i < SAMPLES; ++i) {
    double value = samples[i];
    size_t j = i;
    while (j && samples[j - 1] > value) { samples[j] = samples[j - 1]; --j; }
    samples[j] = value;
  }
  return samples[SAMPLES / 2];
}

int main(void) {
  puts("compiler,libc,implementation,function,iterations,median_ns_per_call");
  printf("%s,%s,%s,rand_r,%d,%.2f\n", BENCH_COMPILER, BENCH_LIBC,
         BENCH_IMPLEMENTATION, ITERATIONS, median_ns(rand_r));
  return sink == 0xdeadbeefU;
}
