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
#define BENCH_IMPLEMENTATION "unspecified"
#endif

typedef char *(*strtok_fn)(char *, const char *);
typedef char *(*strtok_r_fn)(char *, const char *, char **);
static volatile unsigned long sink;

static double now(void) {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return (double)ts.tv_sec + (double)ts.tv_nsec / 1e9;
}

static double run_strtok(strtok_fn fn) {
  enum { SAMPLES = 7, REPS = 200000 };
  double samples[SAMPLES];
  for (size_t s = 0; s < SAMPLES; ++s) {
    unsigned long total = 0;
    double start = now();
    for (size_t i = 0; i < REPS; ++i) {
      char text[] = "alpha,beta,gamma";
      char *token = fn(text, ",");
      if (token == NULL || token[0] != 'a') return -1.0;
      total += (unsigned char)token[0];
      token = fn(NULL, ",");
      if (token == NULL || token[0] != 'b') return -1.0;
      total += (unsigned char)token[0];
      token = fn(NULL, ",");
      if (token == NULL || token[0] != 'g') return -1.0;
      total += (unsigned char)token[0];
    }
    samples[s] = (now() - start) * 1e9 / (REPS * 3);
    sink ^= total;
  }
  for (size_t i = 1; i < SAMPLES; ++i) {
    double value = samples[i];
    size_t j = i;
    while (j && samples[j - 1] > value) {
      samples[j] = samples[j - 1];
      --j;
    }
    samples[j] = value;
  }
  return samples[SAMPLES / 2];
}

static double run_strtok_r(strtok_r_fn fn) {
  enum { SAMPLES = 7, REPS = 200000 };
  double samples[SAMPLES];
  for (size_t s = 0; s < SAMPLES; ++s) {
    unsigned long total = 0;
    double start = now();
    for (size_t i = 0; i < REPS; ++i) {
      char text[] = "alpha,beta,gamma";
      char *state = NULL;
      char *token = fn(text, ",", &state);
      if (token == NULL || token[0] != 'a') return -1.0;
      total += (unsigned char)token[0];
      token = fn(NULL, ",", &state);
      if (token == NULL || token[0] != 'b') return -1.0;
      total += (unsigned char)token[0];
      token = fn(NULL, ",", &state);
      if (token == NULL || token[0] != 'g') return -1.0;
      total += (unsigned char)token[0];
    }
    samples[s] = (now() - start) * 1e9 / (REPS * 3);
    sink ^= total;
  }
  for (size_t i = 1; i < SAMPLES; ++i) {
    double value = samples[i];
    size_t j = i;
    while (j && samples[j - 1] > value) {
      samples[j] = samples[j - 1];
      --j;
    }
    samples[j] = value;
  }
  return samples[SAMPLES / 2];
}

int main(void) {
  double strtok_ns = run_strtok(strtok);
  if (strtok_ns < 0) return 1;
  printf("compiler,libc,implementation,function,iterations,median_ns_per_call\n");
  printf("%s,%s,%s,strtok,600000,%.2f\n", BENCH_COMPILER, BENCH_LIBC,
         BENCH_IMPLEMENTATION, strtok_ns);
#ifdef BENCH_NELIBC_STRTOK_R
  double strtok_r_ns = run_strtok_r(strtok_r);
  if (strtok_r_ns < 0) return 2;
  printf("%s,%s,%s,strtok_r-control,600000,%.2f\n", BENCH_COMPILER,
         BENCH_LIBC, BENCH_IMPLEMENTATION, strtok_r_ns);
#endif
  return 0;
}
