#define _GNU_SOURCE
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#ifndef _CS_PATH
#define _CS_PATH 0
#endif
#ifndef BENCH_COMPILER
#define BENCH_COMPILER "unknown"
#endif
#ifndef BENCH_LIBC
#define BENCH_LIBC "unknown"
#endif
#ifndef BENCH_IMPLEMENTATION
#define BENCH_IMPLEMENTATION "unknown"
#endif

enum { SAMPLES = 7, ITERATIONS = 1000000 };
static size_t (*volatile call_confstr)(int, char *, size_t) = confstr;

static double now_ns(void) {
  struct timespec t;
  clock_gettime(CLOCK_MONOTONIC, &t);
  return (double)t.tv_sec * 1e9 + (double)t.tv_nsec;
}

static int validate(size_t required, const char *path) {
  char buf[64];
  const size_t lengths[] = {0, 1, 2, 4, 14, sizeof buf};
  for (size_t i = 0; i < sizeof lengths / sizeof lengths[0]; ++i) {
    memset(buf, 0x7f, sizeof buf);
    size_t n = call_confstr(_CS_PATH, buf, lengths[i]);
    if (n != required) return 1;
    if (lengths[i] > 0) {
      size_t copied = lengths[i] < required ? lengths[i] : required;
      if (buf[copied - 1] != '\0') return 2;
      if (memcmp(buf, path, copied - 1) != 0) return 3;
    }
  }
  errno = 0;
  if (call_confstr(-1, buf, sizeof buf) != 0 || errno != EINVAL) return 4;
  return 0;
}

int main(void) {
  char path[64] = {0};
  size_t required = call_confstr(_CS_PATH, path, sizeof path);
  if (required == 0 || required > sizeof path || path[required - 1] != '\0') {
    fprintf(stderr, "confstr returned invalid path/length\n");
    return 1;
  }
  int error = validate(required, path);
  if (error) {
    fprintf(stderr, "confstr validation failed (%d)\n", error);
    return 1;
  }
  double samples[SAMPLES];
  for (int s = 0; s < SAMPLES; ++s) {
    double start = now_ns();
    for (int i = 0; i < ITERATIONS; ++i) call_confstr(_CS_PATH, NULL, 0);
    samples[s] = (now_ns() - start) / ITERATIONS;
  }
  for (int i = 0; i < SAMPLES; ++i)
    for (int j = i + 1; j < SAMPLES; ++j)
      if (samples[j] < samples[i]) { double x = samples[i]; samples[i] = samples[j]; samples[j] = x; }
  printf("compiler,libc,implementation,function,path,required_bytes,iterations,median_ns_per_call\n");
  printf("%s,%s,%s,confstr,\"%s\",%zu,%d,%.2f\n", BENCH_COMPILER, BENCH_LIBC,
         BENCH_IMPLEMENTATION, path, required, ITERATIONS, samples[SAMPLES / 2]);
  return 0;
}
