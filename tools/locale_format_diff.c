#define _GNU_SOURCE
#include <locale.h>
#include <stdint.h>
#include <stdio.h>

static int emit(const char *format, long long value) {
  char output[1024];
  int length = snprintf(output, sizeof output, format, value);
  if (length < 0 || length >= (int)sizeof output) return 1;
  if (printf("%s|%d\n", output, length) < 0) return 1;
  return 0;
}

static int emit_float(const char *format, double value) {
  char output[1024];
  int length = snprintf(output, sizeof output, format, value);
  if (length < 0 || length >= (int)sizeof output) return 1;
  if (printf("%s|%d\n", output, length) < 0) return 1;
  return 0;
}

static uint64_t next_random(uint64_t *state) {
  *state ^= *state << 7;
  *state ^= *state >> 9;
  *state ^= *state << 8;
  return *state;
}

static int emit_general(double value, int precision) {
  char output[20000];
  int length = snprintf(output, sizeof output, "%'.*g", precision, value);
  if (length < 0 || length >= (int)sizeof output) return 1;
  if (printf("%s|%d\n", output, length) < 0) return 1;
  return 0;
}

static int emit_general_alternate(double value, int precision) {
  char output[20000];
  int length = snprintf(output, sizeof output, "%#'.*g", precision, value);
  if (length < 0 || length >= (int)sizeof output) return 1;
  if (printf("%s|%d\n", output, length) < 0) return 1;
  return 0;
}

static int emit_fixed(double value, int precision) {
  char output[1024];
  int length = snprintf(output, sizeof output, "%'.*f", precision, value);
  if (length < 0 || length >= (int)sizeof output) return 1;
  if (printf("%s|%d\n", output, length) < 0) return 1;
  return 0;
}

static int emit_scientific(double value, int precision) {
  char output[1024];
  int length = snprintf(output, sizeof output, "%.*e", precision, value);
  if (length < 0 || length >= (int)sizeof output) return 1;
  if (printf("%s|%d\n", output, length) < 0) return 1;
  return 0;
}

int main(void) {
  if (setlocale(LC_NUMERIC, "de_DE.UTF-8") == NULL &&
      setlocale(LC_NUMERIC, "de_DE.utf8") == NULL &&
      setlocale(LC_NUMERIC, "de_DE") == NULL) return 77;
  struct lconv *numeric = localeconv();
  if (numeric == NULL || numeric->decimal_point == NULL ||
      numeric->thousands_sep == NULL || numeric->grouping == NULL) return 1;
  if (emit("%'lld", 1234567890LL) ||
      emit("%'010lld", 1234LL) ||
      emit("%'.12lld", 1234LL) ||
      emit("%+'lld", -1234567LL) ||
      emit("%'lld", 123LL) ||
      emit("%'020lld", -1234567LL) ||
      emit_float("%'f", 1234567.5) ||
      emit_float("%'012.1f", -1234.5) ||
      emit_float("%'.8g", 1234567.5) ||
      emit_float("%'.6g", 999999.9) ||
      emit_float("%'.4g", 9999.9) ||
      emit_float("%'.4g", 0.00009999) ||
      emit_float("%'.4g", 0.000099999) ||
      emit_general(0.0001, 17) ||
      emit_general(0.00009999999999999999, 17) ||
      emit_general(1.2345678901234567e-4, 16) ||
      emit_general(1.2345678901234567, 40) ||
      emit_general(0.0001, 40) ||
      emit_general(1.0, 1000) ||
      emit_general(0.0001, 2000) ||
      emit_general(1.0, 20000) ||
      emit_general(1.0, 100000) ||
      emit_general_alternate(12345.0, 40) ||
      emit_general_alternate(1.0e25, 1200) ||
      emit_float("%+'050.40g", -12345.0) ||
      emit_float("%'.0f", 2.5) ||
      emit_float("%'.0f", 3.5) ||
      emit_float("%'.1f", 1.25) ||
      emit_float("%'.2f", 1234567890123456.5) ||
      emit_float("%'.3e", 1234567.5) ||
      emit_float("%.3e", 9.9995) ||
      emit_float("%.0e", 9.5) ||
      emit_float("%.6e", 4.9406564584124654e-324) ||
      emit_float("%.6E", 1.7976931348623157e308) ||
      emit_float("%'.0f", 1e20) ||
      emit_float("%'.2f", 1.7976931348623157e308)) return 1;
  uint64_t random_state = UINT64_C(0x9e3779b97f4a7c15);
  for (unsigned int index = 0; index < 4096; ++index) {
    uint64_t bits = next_random(&random_state);
    uint64_t exponent = next_random(&random_state) % 2047;
    bits = (bits & ~(UINT64_C(0x7ff) << 52)) | (exponent << 52);
    union { uint64_t bits; double value; } sample = {.bits = bits};
    int precision = 1 + (int)(next_random(&random_state) % 17);
    if (emit_general(sample.value, precision) != 0) return 1;
  }
  for (unsigned int index = 0; index < 4096; ++index) {
    uint64_t bits = next_random(&random_state);
    uint64_t exponent = (next_random(&random_state) % 74) + (1023 - 20);
    bits = (bits & ~(UINT64_C(0x7ff) << 52)) | (exponent << 52);
    union { uint64_t bits; double value; } sample = {.bits = bits};
    int precision = (int)(next_random(&random_state) % 3);
    if (emit_fixed(sample.value, precision) != 0) return 1;
  }
  for (unsigned int index = 0; index < 4096; ++index) {
    uint64_t bits = next_random(&random_state);
    uint64_t exponent = next_random(&random_state) % 2047;
    bits = (bits & ~(UINT64_C(0x7ff) << 52)) | (exponent << 52);
    union { uint64_t bits; double value; } sample = {.bits = bits};
    int precision = (int)(next_random(&random_state) % 17);
    if (emit_scientific(sample.value, precision) != 0) return 1;
  }
  for (unsigned int index = 0; index < 512; ++index) {
    uint64_t bits = next_random(&random_state);
    uint64_t exponent = next_random(&random_state) % 2047;
    bits = (bits & ~(UINT64_C(0x7ff) << 52)) | (exponent << 52);
    union { uint64_t bits; double value; } sample = {.bits = bits};
    int precision = 18 + (int)(next_random(&random_state) % 23);
    if (emit_general(sample.value, precision) != 0) return 1;
  }
  return 0;
}
