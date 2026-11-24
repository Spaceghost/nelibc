#include <errno.h>
#include <float.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>

static void show(long double value, int err, int consumed) {
  printf("%La %.36Lg %d %d\n", value, value, err, consumed);
}

static uint64_t random_word(uint64_t *state) {
  *state ^= *state << 13;
  *state ^= *state >> 7;
  *state ^= *state << 17;
  return *state;
}

int main(void) {
  static const long double values[] = {
    0x1.0000000000000000000000000001p+0L, 0x1.ffffffffffffffffffffffffffffp+0L,
    0x1.8p+0L, 0x1.0000000000008p+0L,
    0.0L, -0.0L, -0x1.p+0L, 0x1p-16494L, 0x1p-16382L, LDBL_MAX,
    __builtin_infl(), -__builtin_infl(), __builtin_nanl("")
  };
  static const char *const inputs[] = {
    "1.0000000000000000000000000000000001925929944387235853",
    "1.00000000000000000000000000000000009629649721936179265",
    "1.00000000000000000000000000000000009629649721936179266",
    "0x1.0000000000000000000000000001p0x",
    "0x1.00000000000000000000000000008p0",
    "0x1.0000000000000000000000000000800001p0",
    "-0x1p-16494", "0x1p-16495", "0x1.000001p-16495",
    "0x1.fffffffffffffffffffffffffffep-16383",
    "0x1.ffffffffffffffffffffffffffffp-16383",
    "3.36210314311209350626267781732175260e-4932",
    "1e-4965", "1e-5000", "1e5000", "1e100000", "1e-100000",
    "-0", "-nan(7)", "inf", "0x", "?", "   -1.25rest"
  };
  for (unsigned i = 0; i < sizeof values / sizeof values[0]; ++i) {
    long double x = values[i];
    printf("%La %.14La %.0La %#.30LA\n", x, x, x, x);
    printf("%+.36Lg %.36Le %.37Lf\n", x, x, x);
  }
  printf("%.42e %.42g\n", 0x1.0111111111111p-1000, 0x1.0000000000001p0);
  for (unsigned i = 0; i < sizeof inputs / sizeof inputs[0]; ++i) {
    char *end;
    errno = 0;
    long double x = strtold(inputs[i], &end);
    show(x, errno, (int)(end - inputs[i]));
  }
  long double value = 0;
  if (sscanf("0x1.0000000000000000000000000001p0", "%La", &value) != 1) return 1;
  show(value, 0, 0);
  if (swscanf(L"0x1.0000000000000000000000000001p0", L"%La", &value) != 1) return 2;
  show(value, 0, 0);

  _Static_assert(LDBL_MANT_DIG == 113 && sizeof(long double) == 16, "requires binary128");
  uint64_t state = UINT64_C(0x240406564c53fca8);
  for (unsigned i = 0; i < 128; ++i) {
    union {
      uint64_t words[2];
      long double value;
    } sample;
    sample.words[0] = random_word(&state);
    sample.words[1] = random_word(&state);
    if ((sample.words[1] & UINT64_C(0x7fff000000000000)) == UINT64_C(0x7fff000000000000))
      continue;
    char decimal[96];
    int size = snprintf(decimal, sizeof decimal, "%.36Lg", sample.value);
    if (size < 0 || (unsigned)size >= sizeof decimal) return 3;
    char *end;
    value = strtold(decimal, &end);
    if (*end || value != sample.value) return 4;
    printf("%La %s\n", value, decimal);
  }
  return 0;
}
