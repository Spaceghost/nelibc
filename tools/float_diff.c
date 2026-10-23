#define _GNU_SOURCE
#include "nelibc_stdlib.h"
#include "float_long_cases.h"
#include <errno.h>
#include <float.h>
#include <locale.h>

/* This differential executable links only the float object, without the adapter. */
int nelibc_radix_character(void) { return '.'; }
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static locale_t c_locale;
static uint64_t random_state = UINT64_C(0x9e3779b97f4a7c15);
static unsigned long long cases_run;
#ifdef NELIBC_RENAMED_FLOAT
static unsigned long long range_errno_variances;
#endif

#ifdef NELIBC_RENAMED_FLOAT
float nelibc_strtof(const char *, char **);
double nelibc_strtod(const char *, char **);
long double nelibc_strtold(const char *, char **);
#define NELIBC_STRTOF nelibc_strtof
#define NELIBC_STRTOD nelibc_strtod
#define NELIBC_STRTOLD nelibc_strtold
#else
#define NELIBC_STRTOF strtof
#define NELIBC_STRTOD strtod
#define NELIBC_STRTOLD strtold
#endif

static uint64_t next_random(void) {
  random_state ^= random_state << 7;
  random_state ^= random_state >> 9;
  random_state ^= random_state << 8;
  return random_state;
}

static int same_float(float left, float right) {
  if (isnan(left) || isnan(right)) return isnan(left) && isnan(right);
  if (left != right) return 0;
  return left != 0.0f || signbit(left) == signbit(right);
}

static int same_double(double left, double right) {
  if (isnan(left) || isnan(right)) return isnan(left) && isnan(right);
  if (left != right) return 0;
  return left != 0.0 || signbit(left) == signbit(right);
}

static int same_long_double(long double left, long double right) {
  if (isnan(left) || isnan(right)) return isnan(left) && isnan(right);
  if (left != right) return 0;
  return left != 0.0L || signbit(left) == signbit(right);
}

#ifdef NELIBC_RENAMED_FLOAT
static int same_range_errno(int ours, int reference, int is_range_result) {
  if (ours == reference) return 1;
  if ((math_errhandling & MATH_ERRNO) != 0 || !is_range_result) return 0;
  if ((ours == ERANGE && reference == 0) || (ours == 0 && reference == ERANGE)) {
    ++range_errno_variances;
    return 1;
  }
  return 0;
}

static int float_is_range_result(float value) {
  return value == 0.0f || value > FLT_MAX || value < -FLT_MAX ||
         (value > -FLT_MIN && value < FLT_MIN);
}

static int double_is_range_result(double value) {
  return value == 0.0 || value > DBL_MAX || value < -DBL_MAX ||
         (value > -DBL_MIN && value < DBL_MIN);
}

static int long_double_is_range_result(long double value) {
  return value == 0.0L || value > LDBL_MAX || value < -LDBL_MAX ||
         (value > -LDBL_MIN && value < LDBL_MIN);
}
#define SAME_ERRNO(ours, reference, range_result) \
  same_range_errno((ours), (reference), (range_result))
#else
#define SAME_ERRNO(ours, reference, range_result) ((ours) == (reference))
#endif

static int compare_one(const char *text) {
  char *ours_end = NULL;
  char *reference_end = NULL;
  int ours_errno;
  int reference_errno;

  errno = 0;
  float ours_float = NELIBC_STRTOF(text, &ours_end);
  ours_errno = errno;
  errno = 0;
  float reference_float = strtof_l(text, &reference_end, c_locale);
  reference_errno = errno;
  if (!same_float(ours_float, reference_float) || ours_end - text != reference_end - text ||
      !SAME_ERRNO(ours_errno, reference_errno, float_is_range_result(ours_float))) {
    fprintf(stderr, "strtof mismatch case=%llu errno=%d/%d value=%a/%a end=%td/%td input=%s\n",
            cases_run, ours_errno, reference_errno, (double)ours_float, (double)reference_float,
            ours_end - text, reference_end - text, text);
    return 1;
  }

  errno = 0;
  double ours_double = NELIBC_STRTOD(text, &ours_end);
  ours_errno = errno;
  errno = 0;
  double reference_double = strtod_l(text, &reference_end, c_locale);
  reference_errno = errno;
  if (!same_double(ours_double, reference_double) || ours_end - text != reference_end - text ||
      !SAME_ERRNO(ours_errno, reference_errno, double_is_range_result(ours_double))) {
    fprintf(stderr, "strtod mismatch case=%llu errno=%d/%d value=%a/%a end=%td/%td input=%s\n",
            cases_run, ours_errno, reference_errno, ours_double, reference_double,
            ours_end - text, reference_end - text, text);
    return 1;
  }

  errno = 0;
  long double ours_long = NELIBC_STRTOLD(text, &ours_end);
  ours_errno = errno;
  errno = 0;
  long double reference_long = strtold_l(text, &reference_end, c_locale);
  reference_errno = errno;
  if (!same_long_double(ours_long, reference_long) || ours_end - text != reference_end - text ||
      !SAME_ERRNO(ours_errno, reference_errno, long_double_is_range_result(ours_long))) {
    fprintf(stderr, "strtold mismatch case=%llu errno=%d/%d value=%La/%La end=%td/%td input=%s\n",
            cases_run, ours_errno, reference_errno, ours_long, reference_long,
            ours_end - text, reference_end - text, text);
    return 1;
  }

  ++cases_run;
  return 0;
}

static size_t make_decimal(char *buffer, size_t capacity) {
  size_t position = 0;
  size_t digits = 1 + (size_t)(next_random() % 300);
  size_t point = (size_t)(next_random() % (digits + 1));
  int exponent = (int)(next_random() % 11001) - 5500;
  if (next_random() & 1) buffer[position++] = '-';
  for (size_t i = 0; i < digits; ++i) {
    if (i == point && point < digits) buffer[position++] = '.';
    buffer[position++] = (char)('0' + next_random() % 10);
  }
  if (point == digits) buffer[position++] = '.';
  position += (size_t)snprintf(buffer + position, capacity - position, "e%+d", exponent);
  return position;
}

static size_t make_hexadecimal(char *buffer, size_t capacity) {
  static const char digits_table[] = "0123456789abcdef";
  size_t position = 0;
  size_t digits = 1 + (size_t)(next_random() % 300);
  size_t point = (size_t)(next_random() % (digits + 1));
  int exponent = (int)(next_random() % 36001) - 18000;
  if (next_random() & 1) buffer[position++] = '-';
  buffer[position++] = '0';
  buffer[position++] = 'x';
  for (size_t i = 0; i < digits; ++i) {
    if (i == point && point < digits) buffer[position++] = '.';
    buffer[position++] = digits_table[next_random() & 15];
  }
  if (point == digits) buffer[position++] = '.';
  position += (size_t)snprintf(buffer + position, capacity - position, "p%+d", exponent);
  return position;
}

int main(void) {
  static const char *fixed_cases[] = {
      "0", "-0", "inf", "-infinity", "nan(payload)", "1e+", "0x1p+",
      "1.00000000000000011102230246251565404236316680908203125",
      "1.00000000000000011102230246251565404236316680908203126",
      "1.000000059604644775390625", "1.000000059604644775390626",
      "1.0000000000000000000542101086242752217003726400434970855712890625",
      "1.0000000000000000000542101086242752217003726400434970855712890626",
      "1.99999999999999999994578989137572477829962735995650291442871093749",
      "1.9999999999999999999457898913757247782996273599565029144287109375",
      "1.99999999999999999994578989137572477829962735995650291442871093751",
      /* x87 predecessor of 2, midpoint tie, and just above the tie. */
      "0x1.fffffffffffffffep0", "0x1.ffffffffffffffffp0",
      "0x1.ffffffffffffffff1p0",
      "0x1.00000000000008p0", "0x1.0000000000000800000000000001p0",
      "0x1.0000000000000001p0", "0x1.0000000000000001000001p0",
      "0x1p-16446", "0x1.0000000000000001p-16446",
      "1e400", "1e-5000", "1e5000", "1e-9999",
      "1e12000", "1e-12000",
      NELIBC_LONG_DECIMAL_FRACTION, NELIBC_LONG_DECIMAL_INTEGER,
      NULL};
  c_locale = newlocale(LC_ALL_MASK, "C", (locale_t)0);
  if (c_locale == (locale_t)0) return 2;
  for (size_t i = 0; fixed_cases[i] != NULL; ++i) {
    if (compare_one(fixed_cases[i])) return 1;
  }

  char text[20000];
  for (unsigned int i = 0; i < 600; ++i) {
    make_decimal(text, sizeof text);
    if (compare_one(text)) return 1;
    make_hexadecimal(text, sizeof text);
    if (compare_one(text)) return 1;
  }

  static const char *double_midpoint =
      "1.00000000000000011102230246251565404236316680908203125";
  size_t position = strlen(double_midpoint);
  memcpy(text, double_midpoint, position);
  memset(text + position, '0', 7000);
  position += 7000;
  text[position++] = '1';
  text[position] = 0;
  if (compare_one(text)) return 1;

#if LDBL_MANT_DIG == 64
  static const char *long_double_midpoint =
      "1.0000000000000000000542101086242752217003726400434970855712890625";
  position = strlen(long_double_midpoint);
  memcpy(text, long_double_midpoint, position);
  memset(text + position, '0', 7000);
  position += 7000;
  text[position++] = '1';
  text[position] = 0;
  if (compare_one(text)) return 1;

  position = strlen(long_double_midpoint);
  memcpy(text, long_double_midpoint, position);
  memset(text + position, '0', 15000);
  position += 15000;
  text[position] = 0;
  if (compare_one(text)) return 1;

  position = strlen(long_double_midpoint);
  memcpy(text, long_double_midpoint, position);
  text[position - 1] = '4';
  memset(text + position, '0', 15000);
  position += 15000;
  text[position] = 0;
  if (compare_one(text)) return 1;

  position = strlen(long_double_midpoint);
  memcpy(text, long_double_midpoint, position);
  memset(text + position, '0', 15000);
  position += 15000;
  text[position++] = '1';
  text[position] = 0;
  if (compare_one(text)) return 1;
#endif

  text[0] = '1';
  memset(text + 1, '0', 8000);
  position = 8001;
  position += (size_t)snprintf(text + position, sizeof text - position, "e-8000");
  if (compare_one(text)) return 1;

  static const char *hex_double_midpoint = "0x1.00000000000008";
  position = strlen(hex_double_midpoint);
  memcpy(text, hex_double_midpoint, position);
  memset(text + position, '0', 6000);
  position += 6000;
  position += (size_t)snprintf(text + position, sizeof text - position, "p0");
  if (compare_one(text)) return 1;

  position = strlen(hex_double_midpoint);
  memcpy(text, hex_double_midpoint, position);
  text[position - 1] = '7';
  memset(text + position, '0', 6000);
  position += 6000;
  position += (size_t)snprintf(text + position, sizeof text - position, "p0");
  if (compare_one(text)) return 1;

  position = strlen(hex_double_midpoint);
  memcpy(text, hex_double_midpoint, position);
  memset(text + position, '0', 6000);
  position += 6000;
  text[position++] = '1';
  position += (size_t)snprintf(text + position, sizeof text - position, "p0");
  if (compare_one(text)) return 1;

#if LDBL_MANT_DIG == 64
  static const char *hex_long_double_midpoint = "0x1.0000000000000001";
  position = strlen(hex_long_double_midpoint);
  memcpy(text, hex_long_double_midpoint, position);
  memset(text + position, '0', 15000);
  position += 15000;
  position += (size_t)snprintf(text + position, sizeof text - position, "p0");
  if (compare_one(text)) return 1;

  position = strlen(hex_long_double_midpoint);
  memcpy(text, hex_long_double_midpoint, position);
  text[position - 1] = '0';
  memset(text + position, 'f', 15000);
  position += 15000;
  position += (size_t)snprintf(text + position, sizeof text - position, "p0");
  if (compare_one(text)) return 1;

  position = strlen(hex_long_double_midpoint);
  memcpy(text, hex_long_double_midpoint, position);
  memset(text + position, '0', 15000);
  position += 15000;
  text[position++] = '1';
  position += (size_t)snprintf(text + position, sizeof text - position, "p0");
  if (compare_one(text)) return 1;
#endif

  text[0] = '0';
  text[1] = 'x';
  text[2] = '1';
  memset(text + 3, '0', 6000);
  position = 6003;
  position += (size_t)snprintf(text + position, sizeof text - position, "p-24000");
  if (compare_one(text)) return 1;

  text[0] = '1';
  memset(text + 1, '0', 8000);
  position = 8001;
  position += (size_t)snprintf(text + position, sizeof text - position, "e-12900");
  if (compare_one(text)) return 1;

  memset(text, '1', 8000);
  position = 8000;
  position += (size_t)snprintf(text + position, sizeof text - position, "e-12900");
  if (compare_one(text)) return 1;

  memset(text, '1', 14800);
  position = 14800;
  position += (size_t)snprintf(text + position, sizeof text - position, "e-9999");
  if (compare_one(text)) return 1;

  text[0] = '0';
  text[1] = 'x';
  text[2] = '1';
  memset(text + 3, '0', 6000);
  position = 6003;
  position += (size_t)snprintf(text + position, sizeof text - position, "p-30000");
  if (compare_one(text)) return 1;

  position = (size_t)snprintf(text, sizeof text, "0x123456789abcdef08");
  memset(text + position, '0', 5982);
  position += 5982;
  text[position++] = '1';
  position += (size_t)snprintf(text + position, sizeof text - position, "p-40000");
  if (compare_one(text)) return 1;

  freelocale(c_locale);
#ifdef NELIBC_RENAMED_FLOAT
  printf("float differential cases passed: %llu; errno variances allowed by math_errhandling: %llu\n",
         cases_run, range_errno_variances);
#else
  printf("float differential cases passed: %llu\n", cases_run);
#endif
  return 0;
}
