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

typedef unsigned long long (*parse_fn)(const char *, char **, int);
typedef struct { const char *text; int base; } input_case;
static const input_case inputs[] = {
  {"123456789012345", 10}, {"-98765432101234", 10},
  {"0x123456789abcdef", 0}, {"777777777777777", 8},
  {"zzzzzzzzzzzz", 36}
};
static volatile unsigned long long sink;

static unsigned long long bench_strtol(const char *s, char **e, int b) { return (unsigned long long)strtol(s, e, b); }
static unsigned long long bench_strtoul(const char *s, char **e, int b) { return (unsigned long long)strtoul(s, e, b); }
static unsigned long long bench_strtoll(const char *s, char **e, int b) { return (unsigned long long)strtoll(s, e, b); }
static unsigned long long bench_strtoull(const char *s, char **e, int b) { return strtoull(s, e, b); }

static double now_seconds(void) {
  struct timespec ts;
  if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) return 0.0;
  return (double)ts.tv_sec + (double)ts.tv_nsec / 1e9;
}

static double median_ns(parse_fn fn) {
  double samples[SAMPLES];
  for (size_t sample = 0; sample < SAMPLES; ++sample) {
    unsigned long long local = 0;
    double start = now_seconds();
    for (size_t i = 0; i < ITERATIONS; ++i) {
      const input_case *item = &inputs[i % (sizeof inputs / sizeof inputs[0])];
      char *end;
      local += fn(item->text, &end, item->base) + (unsigned long long)(end - item->text);
    }
    samples[sample] = (now_seconds() - start) * 1e9 / ITERATIONS;
    sink ^= local;
  }
  for (size_t i = 1; i < SAMPLES; ++i) {
    double value = samples[i];
    size_t j = i;
    while (j && samples[j - 1] > value) { samples[j] = samples[j - 1]; --j; }
    samples[j] = value;
  }
  return samples[SAMPLES / 2];
}

static void report(const char *function, parse_fn fn) {
  printf("%s,%s,%s,%s,%zu,%.2f\n", BENCH_COMPILER, BENCH_LIBC,
         BENCH_IMPLEMENTATION, function, ITERATIONS, median_ns(fn));
}

int main(void) {
  puts("compiler,libc,implementation,function,iterations,median_ns_per_call");
  report("strtol", bench_strtol);
  report("strtoul", bench_strtoul);
  report("strtoll", bench_strtoll);
  report("strtoull", bench_strtoull);
  return 0;
}
