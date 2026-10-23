#define _GNU_SOURCE
#include <linux/perf_event.h>
#include <sys/ioctl.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#ifdef BENCH_NELIBC
int nelibc_strcasecmp(const char *, const char *);
#define CALL(a, b) nelibc_strcasecmp((a), (b))
#else
#include <strings.h>
#define CALL(a, b) strcasecmp((a), (b))
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

static char text[12][2][512] __attribute__((aligned(4096)));
static volatile unsigned long long sink;

static int open_counter(unsigned config, int group_fd) {
  struct perf_event_attr attr;
  memset(&attr, 0, sizeof(attr));
  attr.type = PERF_TYPE_HARDWARE;
  attr.size = sizeof(attr);
  attr.config = config;
  attr.disabled = group_fd < 0;
  attr.exclude_kernel = 1;
  attr.exclude_hv = 1;
  attr.read_format = PERF_FORMAT_GROUP;
  return (int)syscall(__NR_perf_event_open, &attr, 0, -1, group_fd, 0);
}

int main(void) {
  for (int row = 0; row < 12; ++row) {
    for (int j = 0; j < 256; ++j) {
      text[row][0][j] = (char)('A' + j % 26);
      text[row][1][j] = (char)('a' + j % 26);
    }
    text[row][0][256] = text[row][1][256] = '\0';
  }
  text[11][1][255] = '!';

  int cycles = open_counter(PERF_COUNT_HW_CPU_CYCLES, -1);
  int instructions = open_counter(PERF_COUNT_HW_INSTRUCTIONS, cycles);
  int branches = open_counter(PERF_COUNT_HW_BRANCH_INSTRUCTIONS, cycles);
  int misses = open_counter(PERF_COUNT_HW_BRANCH_MISSES, cycles);
  if (cycles < 0 || instructions < 0 || branches < 0 || misses < 0) {
    perror("perf_event_open");
    return 2;
  }

  puts("compiler,libc,implementation,case,cycles,instructions,branches,branch_misses");
  for (int mode = 0; mode < 2; ++mode) {
    int row = mode ? 11 : 10;
    unsigned long long sum = 0;
    unsigned long long values[5] = {0};
    for (int i = 0; i < 10000; ++i)
      sum += (unsigned)CALL(text[row][0], text[row][1]);
    ioctl(cycles, PERF_EVENT_IOC_RESET, PERF_IOC_FLAG_GROUP);
    ioctl(cycles, PERF_EVENT_IOC_ENABLE, PERF_IOC_FLAG_GROUP);
    for (int i = 0; i < 1000000; ++i)
      sum += (unsigned)CALL(text[row][0], text[row][1]);
    ioctl(cycles, PERF_EVENT_IOC_DISABLE, PERF_IOC_FLAG_GROUP);
    if (read(cycles, values, sizeof(values)) != sizeof(values)) {
      perror("read perf counters");
      return 2;
    }
    sink ^= sum;
    printf("%s,%s,%s,%s,%llu,%llu,%llu,%llu\n",
           BENCH_COMPILER, BENCH_LIBC, BENCH_IMPLEMENTATION,
           mode ? "late-mismatch-256" : "equal-256",
           values[1] / 1000000, values[2] / 1000000,
           values[3] / 1000000, values[4] / 1000000);
  }
  close(cycles);
  close(instructions);
  close(branches);
  close(misses);
  return sink == 1;
}
