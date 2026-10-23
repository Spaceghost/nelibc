#define _GNU_SOURCE
#include "nelibc_stdlib.h"
#include "float_long_cases.h"
#include <locale.h>
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

#ifdef BENCH_LONG_DECIMALS
#define SAMPLES 5
#define ITERATIONS 160
#else
#define SAMPLES 7
#define ITERATIONS 4000
#endif

typedef long double (*parse_fn)(const char *, char **);
static locale_t c_locale;
static volatile long double sink;

/* Match the hybrid locale adapter; the benchmark parses in the initial C locale. */
int nelibc_radix_character(void) {
  struct lconv *data = localeconv();
  return data != NULL && data->decimal_point != NULL && data->decimal_point[0]
      ? (unsigned char)data->decimal_point[0] : '.';
}

#ifdef BENCH_LONG_DECIMALS
static const char *inputs[] = {
    NELIBC_LONG_DECIMAL_FRACTION,
    NELIBC_LONG_DECIMAL_INTEGER};
#else
static const char *inputs[] = {
    "123.4567890123456789", "-0.00000000000000000012345",
    "1.234567890123456789e+120", "0x1.abcdef0123456789p-100",
    "3.1415926535897932384626433832795028841971"};
#endif

#ifndef BENCH_HOST_ONLY
static long double ours_f(const char *s, char **e) { return strtof(s, e); }
static long double ours_d(const char *s, char **e) { return strtod(s, e); }
static long double ours_ld(const char *s, char **e) { return strtold(s, e); }
#endif
#ifndef BENCH_NELIBC_ONLY
static long double host_f(const char *s, char **e) { return strtof_l(s, e, c_locale); }
static long double host_d(const char *s, char **e) { return strtod_l(s, e, c_locale); }
static long double host_ld(const char *s, char **e) { return strtold_l(s, e, c_locale); }
#endif

static double now_seconds(void) {
  struct timespec ts;
  if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) return 0.0;
  return (double)ts.tv_sec + (double)ts.tv_nsec / 1e9;
}

static double median_ns(parse_fn fn) {
  double samples[SAMPLES];
  for (size_t sample = 0; sample < SAMPLES; ++sample) {
    long double local = 0.0L;
    double start = now_seconds();
    for (size_t i = 0; i < ITERATIONS; ++i) {
      const char *input = inputs[i % (sizeof inputs / sizeof inputs[0])];
      char *end;
      local += fn(input, &end) + (long double)(end - input);
    }
    samples[sample] = (now_seconds() - start) * 1e9 / ITERATIONS;
    sink += local;
  }
  for (size_t i = 1; i < SAMPLES; ++i) {
    double value = samples[i];
    size_t j = i;
    while (j && samples[j - 1] > value) { samples[j] = samples[j - 1]; --j; }
    samples[j] = value;
  }
  return samples[SAMPLES / 2];
}

static void report(const char *impl, const char *function, parse_fn fn) {
  printf("%s,%s,%s,%s,%zu,%.2f\n", BENCH_COMPILER, BENCH_LIBC, impl,
         function, ITERATIONS, median_ns(fn));
}

int main(void) {
  c_locale = newlocale(LC_NUMERIC_MASK, "C", (locale_t)0);
  if (!c_locale) return 2;
  puts("compiler,libc,implementation,function,iterations,median_ns_per_call");
#if !defined(BENCH_NELIBC_ONLY) && !defined(BENCH_HOST_ONLY)
  report("host-libc", "strtof", host_f); report("nelibc", "strtof", ours_f);
  report("host-libc", "strtod", host_d); report("nelibc", "strtod", ours_d);
  report("host-libc", "strtold", host_ld); report("nelibc", "strtold", ours_ld);
#elif defined(BENCH_HOST_ONLY) && defined(BENCH_NELIBC_ONLY)
#error "Select at most one of BENCH_HOST_ONLY and BENCH_NELIBC_ONLY"
#elif defined(BENCH_HOST_ONLY)
  report("host-libc", "strtof", host_f);
  report("host-libc", "strtod", host_d);
  report("host-libc", "strtold", host_ld);
#else
  report("nelibc", "strtof", ours_f);
  report("nelibc", "strtod", ours_d);
  report("nelibc", "strtold", ours_ld);
#endif
  freelocale(c_locale);
  return sink == -1.0L;
}
