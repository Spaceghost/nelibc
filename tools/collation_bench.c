#define _GNU_SOURCE 1
#include "string.h"
#include <locale.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>

extern int strcoll_l(const char *left, const char *right, locale_t locale);
extern size_t strxfrm_l(char *restrict dest, const char *restrict src,
                        size_t size, locale_t locale);

#ifndef BENCH_COMPILER
#define BENCH_COMPILER "unspecified"
#endif
#ifndef BENCH_LIBC
#define BENCH_LIBC "unspecified"
#endif

#define SAMPLES 5
#define ITERATIONS 10000
#define KEY_SIZE 512

static const char *const words[] = {
  "", "A", "a", "aa", "ab", "ae", "b", "ss", "z", "_", "10", "2",
  "\xc3\xa4", "\xc3\x9f", "\xc3\xa5", "\xc3\xb6"
};
static locale_t selected_locale;
static volatile uint64_t sink;

typedef int (*collate_fn)(const char *, const char *);
typedef size_t (*transform_fn)(char *, const char *, size_t);

static int host_collate(const char *left, const char *right) {
  return strcoll_l(left, right, selected_locale);
}

static int nelibc_collate(const char *left, const char *right) {
  return strcoll(left, right);
}

static size_t host_transform(char *dest, const char *src, size_t size) {
  return strxfrm_l(dest, src, size, selected_locale);
}

static size_t nelibc_transform(char *dest, const char *src, size_t size) {
  return strxfrm(dest, src, size);
}

static int signum(int value) { return (value > 0) - (value < 0); }

static double now_seconds(void) {
  struct timespec ts;
  if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) return 0.0;
  return (double)ts.tv_sec + (double)ts.tv_nsec / 1e9;
}

static int verify_current_locale(void) {
  char host_key[KEY_SIZE];
  char nelibc_key[KEY_SIZE];
  for (size_t i = 0; i < sizeof words / sizeof words[0]; ++i) {
    size_t host_length = host_transform(host_key, words[i], sizeof host_key);
    size_t nelibc_length = nelibc_transform(nelibc_key, words[i], sizeof nelibc_key);
    if (host_length != nelibc_length || host_length >= sizeof host_key ||
        host_key[host_length] != '\0' || nelibc_key[nelibc_length] != '\0') {
      fprintf(stderr, "strxfrm mismatch for '%s': host length=%zu Nelibc length=%zu\n", words[i], host_length, nelibc_length);
      return 1;
    }
    for (size_t j = 0; j < sizeof words / sizeof words[0]; ++j) {
      if (signum(host_collate(words[i], words[j])) !=
          signum(nelibc_collate(words[i], words[j]))) {
        fprintf(stderr, "strcoll mismatch for '%s' and '%s'\n", words[i], words[j]);
        return 1;
      }
    }
  }
  return 0;
}

static double median_collate_ns(collate_fn function) {
  double samples[SAMPLES];
  for (size_t sample = 0; sample < SAMPLES; ++sample) {
    uint64_t local = 0;
    double start = now_seconds();
    for (size_t i = 0; i < ITERATIONS; ++i) {
      size_t left = i % (sizeof words / sizeof words[0]);
      size_t right = (i * 7 + 3) % (sizeof words / sizeof words[0]);
      local += (uint64_t)(function(words[left], words[right]) + 256);
    }
    samples[sample] = (now_seconds() - start) * 1e9 / ITERATIONS;
    sink ^= local;
  }
  for (size_t i = 1; i < SAMPLES; ++i) {
    double value = samples[i];
    size_t j = i;
    while (j > 0 && samples[j - 1] > value) {
      samples[j] = samples[j - 1];
      --j;
    }
    samples[j] = value;
  }
  return samples[SAMPLES / 2];
}

static double median_transform_ns(transform_fn function) {
  double samples[SAMPLES];
  char key[KEY_SIZE];
  for (size_t sample = 0; sample < SAMPLES; ++sample) {
    uint64_t local = 0;
    double start = now_seconds();
    for (size_t i = 0; i < ITERATIONS; ++i) {
      size_t index = i % (sizeof words / sizeof words[0]);
      local += function(key, words[index], sizeof key) + (unsigned char)key[0];
    }
    samples[sample] = (now_seconds() - start) * 1e9 / ITERATIONS;
    sink ^= local;
  }
  for (size_t i = 1; i < SAMPLES; ++i) {
    double value = samples[i];
    size_t j = i;
    while (j > 0 && samples[j - 1] > value) {
      samples[j] = samples[j - 1];
      --j;
    }
    samples[j] = value;
  }
  return samples[SAMPLES / 2];
}

static void report(const char *mode, const char *implementation) {
  collate_fn compare = implementation[0] == 'h' ? host_collate : nelibc_collate;
  transform_fn transform = implementation[0] == 'h' ? host_transform : nelibc_transform;
  printf("%s,%s,%s,strcoll,%s,%zu,%.2f\n", BENCH_COMPILER, BENCH_LIBC,
         implementation, mode, ITERATIONS, median_collate_ns(compare));
  printf("%s,%s,%s,strxfrm,%s,%zu,%.2f\n", BENCH_COMPILER, BENCH_LIBC,
         implementation, mode, ITERATIONS, median_transform_ns(transform));
}

int main(int argc, char **argv) {
  const char *locale_name = argc > 1 ? argv[1] : "C";
  selected_locale = newlocale(LC_COLLATE_MASK, locale_name, (locale_t)0);
  if (selected_locale == (locale_t)0) {
    fprintf(stderr, "cannot create LC_COLLATE locale '%s'\n", locale_name);
    return 77;
  }
  if (setlocale(LC_COLLATE, locale_name) == (char *)0) {
    fprintf(stderr, "cannot select global LC_COLLATE locale '%s'\n", locale_name);
    freelocale(selected_locale);
    return 77;
  }

  puts("compiler,libc,implementation,function,locale_mode,iterations,median_ns_per_call");
  if (verify_current_locale() != 0) return 1;
  report("global", "host-strcoll_l");
  report("global", "nelibc");

  locale_t previous = uselocale(selected_locale);
  if (previous == (locale_t)0) {
    fprintf(stderr, "cannot select thread locale '%s'\n", locale_name);
    return 77;
  }
  if (verify_current_locale() != 0) return 1;
  report("thread", "host-strcoll_l");
  report("thread", "nelibc");
  uselocale(previous);
  freelocale(selected_locale);
  (void)sink;
  return 0;
}
