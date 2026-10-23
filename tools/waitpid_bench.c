#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>

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
#define ITERATIONS 200000u

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

int main(void) {
  int (*volatile waitpid_call)(pid_t, int *, int) = waitpid;
  double samples[SAMPLES];
  for (size_t sample = 0; sample < SAMPLES; ++sample) {
    double start = now_seconds();
    for (size_t iteration = 0; iteration < ITERATIONS; ++iteration) {
      if (waitpid_call(-1, NULL, WNOHANG) != -1 || errno != ECHILD) return 1;
    }
    samples[sample] = (now_seconds() - start) * 1e9 / ITERATIONS;
  }
  sort(samples);
  printf("%s,%s,%s,waitpid-wnohang-echild,%u,%.2f\n", BENCH_COMPILER,
         BENCH_LIBC, BENCH_IMPLEMENTATION, ITERATIONS, samples[SAMPLES / 2]);
  return 0;
}
