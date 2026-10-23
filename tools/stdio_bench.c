#define _POSIX_C_SOURCE 200809L
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

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
#define FORMAT_ITERATIONS 50000u
#define FMEMOPEN_ITERATIONS 10000u

static int (* volatile putchar_call)(int) = putchar;
static int (* volatile puts_call)(const char *) = puts;
static int (* volatile fputc_call)(int, FILE *) = fputc;
static int (* volatile fputs_call)(const char *, FILE *) = fputs;
static int (* volatile snprintf_call)(char *, size_t, const char *, ...) = snprintf;
static int (* volatile sprintf_call)(char *, const char *, ...) = sprintf;
static int (* volatile vsprintf_call)(char *, const char *, va_list) = vsprintf;
static int (* volatile fprintf_call)(FILE *, const char *, ...) = fprintf;
static FILE *(* volatile fmemopen_call)(void *, size_t, const char *) = fmemopen;
static FILE *(* volatile tmpfile_call)(void) = tmpfile;
static size_t (* volatile fread_call)(void *, size_t, size_t, FILE *) = fread;
static size_t (* volatile fwrite_call)(const void *, size_t, size_t, FILE *) = fwrite;
static volatile int result_sink;
static unsigned char fread_input[ITERATIONS];

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

static double bench_putchar(void) {
  double samples[SAMPLES];
  for (size_t sample = 0; sample < SAMPLES; ++sample) {
    int local = 0;
    double start = now_seconds();
    for (size_t i = 0; i < ITERATIONS; ++i) {
      int result = putchar_call('x');
      if (result != 'x') return -1.0;
      local += result;
    }
    double elapsed = now_seconds() - start;
    if (fflush(stdout) != 0) return -1.0;
    samples[sample] = elapsed * 1e9 / ITERATIONS;
    result_sink ^= local;
  }
  sort(samples);
  return samples[SAMPLES / 2];
}

static double bench_puts(void) {
  static const char text[] = "nelibc stdio";
  double samples[SAMPLES];
  for (size_t sample = 0; sample < SAMPLES; ++sample) {
    int local = 0;
    double start = now_seconds();
    for (size_t i = 0; i < ITERATIONS; ++i) {
      int result = puts_call(text);
      if (result < 0) return -1.0;
      local += result;
    }
    double elapsed = now_seconds() - start;
    if (fflush(stdout) != 0) return -1.0;
    samples[sample] = elapsed * 1e9 / ITERATIONS;
    result_sink ^= local;
  }
  sort(samples);
  return samples[SAMPLES / 2];
}

static double bench_fputc(void) {
  double samples[SAMPLES];
  for (size_t sample = 0; sample < SAMPLES; ++sample) {
    int local = 0;
    double start = now_seconds();
    for (size_t i = 0; i < ITERATIONS; ++i) {
      int result = fputc_call('x', stdout);
      if (result != 'x') return -1.0;
      local += result;
    }
    double elapsed = now_seconds() - start;
    if (fflush(stdout) != 0) return -1.0;
    samples[sample] = elapsed * 1e9 / ITERATIONS;
    result_sink ^= local;
  }
  sort(samples);
  return samples[SAMPLES / 2];
}

static double bench_fputs(void) {
  static const char text[] = "nelibc stdio";
  double samples[SAMPLES];
  for (size_t sample = 0; sample < SAMPLES; ++sample) {
    int local = 0;
    double start = now_seconds();
    for (size_t i = 0; i < ITERATIONS; ++i) {
      int result = fputs_call(text, stdout);
      if (result < 0) return -1.0;
      local += result;
    }
    double elapsed = now_seconds() - start;
    if (fflush(stdout) != 0) return -1.0;
    samples[sample] = elapsed * 1e9 / ITERATIONS;
    result_sink ^= local;
  }
  sort(samples);
  return samples[SAMPLES / 2];
}

static double bench_snprintf(void) {
  double samples[SAMPLES];
  char buffer[64];
  for (size_t sample = 0; sample < SAMPLES; ++sample) {
    int local = 0;
    double start = now_seconds();
    for (unsigned int i = 0; i < FORMAT_ITERATIONS; ++i) {
      unsigned int value = i * 2654435761u;
      double fraction = (double)(i % 10000u) / 100.0;
      int result = snprintf_call(buffer, sizeof buffer, "%08x:%.2f", value,
                                fraction);
      if (result < 0 || result >= (int)sizeof buffer) return -1.0;
      local += result + buffer[0];
    }
    double elapsed = now_seconds() - start;
    samples[sample] = elapsed * 1e9 / FORMAT_ITERATIONS;
    result_sink ^= local;
  }
  sort(samples);
  return samples[SAMPLES / 2];
}

static int call_vsprintf(char *buffer, const char *format, ...) {
  va_list arguments;
  va_start(arguments, format);
  int result = vsprintf_call(buffer, format, arguments);
  va_end(arguments);
  return result;
}

static double bench_sprintf(void) {
  double samples[SAMPLES];
  char buffer[64];
  for (size_t sample = 0; sample < SAMPLES; ++sample) {
    int local = 0;
    double start = now_seconds();
    for (unsigned int i = 0; i < FORMAT_ITERATIONS; ++i) {
      unsigned int value = i * 2654435761u;
      double fraction = (double)(i % 10000u) / 100.0;
      int result = sprintf_call(buffer, "%08x:%.2f", value, fraction);
      if (result < 0 || result >= (int)sizeof buffer) return -1.0;
      local += result + buffer[0];
    }
    double elapsed = now_seconds() - start;
    samples[sample] = elapsed * 1e9 / FORMAT_ITERATIONS;
    result_sink ^= local;
  }
  sort(samples);
  return samples[SAMPLES / 2];
}

static double bench_vsprintf(void) {
  double samples[SAMPLES];
  char buffer[64];
  for (size_t sample = 0; sample < SAMPLES; ++sample) {
    int local = 0;
    double start = now_seconds();
    for (unsigned int i = 0; i < FORMAT_ITERATIONS; ++i) {
      unsigned int value = i * 2654435761u;
      double fraction = (double)(i % 10000u) / 100.0;
      int result = call_vsprintf(buffer, "%08x:%.2f", value, fraction);
      if (result < 0 || result >= (int)sizeof buffer) return -1.0;
      local += result + buffer[0];
    }
    double elapsed = now_seconds() - start;
    samples[sample] = elapsed * 1e9 / FORMAT_ITERATIONS;
    result_sink ^= local;
  }
  sort(samples);
  return samples[SAMPLES / 2];
}

static double bench_fprintf(void) {
  double samples[SAMPLES];
  for (size_t sample = 0; sample < SAMPLES; ++sample) {
    int local = 0;
    double start = now_seconds();
    for (unsigned int i = 0; i < FORMAT_ITERATIONS; ++i) {
      unsigned int value = i * 2654435761u;
      double fraction = (double)(i % 10000u) / 100.0;
      int result = fprintf_call(stdout, "%08x:%.2f", value, fraction);
      if (result < 0) return -1.0;
      local += result;
    }
    double elapsed = now_seconds() - start;
    if (fflush(stdout) != 0) return -1.0;
    samples[sample] = elapsed * 1e9 / FORMAT_ITERATIONS;
    result_sink ^= local;
  }
  sort(samples);
  return samples[SAMPLES / 2];
}

static double bench_fmemopen(void) {
  double samples[SAMPLES];
  for (size_t sample = 0; sample < SAMPLES; ++sample) {
    int local = 0;
    double start = now_seconds();
    for (unsigned int i = 0; i < FMEMOPEN_ITERATIONS; ++i) {
      char buffer[16];
      FILE *stream = fmemopen_call(buffer, sizeof buffer, "w+");
      if (!stream || fputc_call('x', stream) != 'x' || fclose(stream) != 0 ||
          buffer[0] != 'x') return -1.0;
      local += buffer[0];
    }
    double elapsed = now_seconds() - start;
    samples[sample] = elapsed * 1e9 / FMEMOPEN_ITERATIONS;
    result_sink ^= local;
  }
  sort(samples);
  return samples[SAMPLES / 2];
}

static double bench_fread(void) {
  double samples[SAMPLES];
  memset(fread_input, 'r', sizeof fread_input);
  for (size_t sample = 0; sample < SAMPLES; ++sample) {
    FILE *stream = tmpfile_call();
    if (!stream) return -1.0;
    if (fwrite_call(fread_input, 1, sizeof fread_input, stream) != sizeof fread_input ||
        fflush(stream) != 0 || fseek(stream, 0, SEEK_SET) != 0) {
      fclose(stream);
      return -1.0;
    }
    unsigned char value = 0;
    int local = 0;
    double start = now_seconds();
    for (unsigned int i = 0; i < ITERATIONS; ++i) {
      if (fread_call(&value, 1, 1, stream) != 1) {
        fclose(stream);
        return -1.0;
      }
      local += value;
    }
    double elapsed = now_seconds() - start;
    if (fclose(stream) != 0) return -1.0;
    samples[sample] = elapsed * 1e9 / ITERATIONS;
    result_sink ^= local;
  }
  sort(samples);
  return samples[SAMPLES / 2];
}

static double bench_fwrite(void) {
  double samples[SAMPLES];
  const unsigned char value = 'w';
  for (size_t sample = 0; sample < SAMPLES; ++sample) {
    FILE *stream = tmpfile_call();
    if (!stream) return -1.0;
    int local = 0;
    double start = now_seconds();
    for (unsigned int i = 0; i < ITERATIONS; ++i) {
      if (fwrite_call(&value, 1, 1, stream) != 1) {
        fclose(stream);
        return -1.0;
      }
      local += value;
    }
    double elapsed = now_seconds() - start;
    if (fflush(stream) != 0 || ftell(stream) != ITERATIONS || fclose(stream) != 0)
      return -1.0;
    samples[sample] = elapsed * 1e9 / ITERATIONS;
    result_sink ^= local;
  }
  sort(samples);
  return samples[SAMPLES / 2];
}

static int report(const char *function, unsigned int iterations, double median) {
  if (median < 0.0) return 1;
  printf("%s,%s,%s,%s,%u,%.2f\n", BENCH_COMPILER, BENCH_LIBC,
         BENCH_IMPLEMENTATION, function, iterations, median);
  return 0;
}

int main(void) {
  int saved_stdout = dup(STDOUT_FILENO);
  int null_fd = open("/dev/null", O_WRONLY);
  if (saved_stdout < 0 || null_fd < 0 || fflush(stdout) != 0 ||
      dup2(null_fd, STDOUT_FILENO) != STDOUT_FILENO) return 1;
  if (close(null_fd) != 0) return 2;

  double putchar_ns = bench_putchar();
  double puts_ns = bench_puts();
  double fputc_ns = bench_fputc();
  double fputs_ns = bench_fputs();
  double snprintf_ns = bench_snprintf();
  double sprintf_ns = bench_sprintf();
  double vsprintf_ns = bench_vsprintf();
  double fprintf_ns = bench_fprintf();
  double fmemopen_ns = bench_fmemopen();
  double fread_ns = bench_fread();
  double fwrite_ns = bench_fwrite();
  if (fflush(stdout) != 0 || dup2(saved_stdout, STDOUT_FILENO) != STDOUT_FILENO ||
      close(saved_stdout) != 0) return 3;

  if (report("putchar", ITERATIONS, putchar_ns) ||
      report("puts", ITERATIONS, puts_ns) ||
      report("fputc", ITERATIONS, fputc_ns) ||
      report("fputs", ITERATIONS, fputs_ns) ||
      report("snprintf", FORMAT_ITERATIONS, snprintf_ns) ||
      report("sprintf", FORMAT_ITERATIONS, sprintf_ns) ||
      report("vsprintf", FORMAT_ITERATIONS, vsprintf_ns) ||
      report("fprintf", FORMAT_ITERATIONS, fprintf_ns) ||
      report("fmemopen-open-write-close", FMEMOPEN_ITERATIONS, fmemopen_ns) ||
      report("fread-byte-tmpfile", ITERATIONS, fread_ns) ||
      report("fwrite-byte-tmpfile", ITERATIONS, fwrite_ns)) return 4;
  return result_sink == -1;
}
