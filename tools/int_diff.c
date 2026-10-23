#include "nelibc_stdlib.h"
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CASES 1200
static uint64_t state = UINT64_C(0xd1b54a32d192ed03);
static unsigned long long cases_run;

static uint64_t random_u64(void) {
  state ^= state << 13; state ^= state >> 7; state ^= state << 17;
  return state;
}

static ptrdiff_t checked_end_offset(const char *text, const char *end, int error) {
  /* Host libcs differ in endptr placement for overflow and no-conversion cases. */
  if (error == ERANGE || end == text) return -1;
  return end - text;
}

static int emit_one(const char *text, int base) {
  char *end;
  printf("%d:%s:", base, text);
  errno = 0; long value_l = strtol(text, &end, base); int error = errno; printf("%ld,%td,%d;", value_l, checked_end_offset(text, end, error), error);
  errno = 0; long long value_ll = strtoll(text, &end, base); error = errno; printf("%lld,%td,%d;", value_ll, checked_end_offset(text, end, error), error);
  errno = 0; unsigned long value_ul = strtoul(text, &end, base); error = errno; printf("%lu,%td,%d;", value_ul, checked_end_offset(text, end, error), error);
  errno = 0; unsigned long long value_ull = strtoull(text, &end, base); error = errno; printf("%llu,%td,%d\n", value_ull, checked_end_offset(text, end, error), error);
  ++cases_run;
  return 0;
}

int main(void) {
  static const int bases[] = {0, 2, 3, 8, 10, 16, 36};
  static const char *fixed[] = {"", "-0", "0x", "0xg", "077tail", "-9223372036854775809!", "18446744073709551616!", "999999999999999999999999999999999999999999999999999999999999", "0"};
  for (size_t i = 0; i < sizeof fixed / sizeof fixed[0]; ++i)
    if (emit_one(fixed[i], bases[i % (sizeof bases / sizeof bases[0])])) return 1;
  for (size_t i = 0; i < CASES; ++i) {
    char text[128]; size_t p = 0;
    int base = bases[random_u64() % (sizeof bases / sizeof bases[0])];
    text[p++] = (random_u64() & 1) ? ' ' : '\t';
    if (random_u64() & 1) text[p++] = (random_u64() & 1) ? '-' : '+';
    if (base == 0) {
      unsigned prefix = random_u64() % 3;
      if (prefix == 1) { text[p++]='0'; text[p++]='x'; base=16; }
      else if (prefix == 2) { text[p++]='0'; base=8; }
      else base=10;
    }
    size_t n = 1 + random_u64() % 60;
    for (size_t j = 0; j < n; ++j) text[p++] = "0123456789abcdefghijklmnopqrstuvwxyz"[random_u64() % (unsigned)base];
    text[p++] = '!'; text[p] = 0;
    if (emit_one(text, base)) return 1;
  }
  printf("%llu integer conversion input cases emitted\n", cases_run);
  return 0;
}
