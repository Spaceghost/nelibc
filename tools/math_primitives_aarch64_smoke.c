#include <fenv.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>

extern double nelibc_fmod_hw(double, double);
extern double nelibc_sqrt_hw(double);
extern float nelibc_sqrtf_hw(float);

typedef union {double value; uint64_t bits;} Binary64;
static uint64_t seed = UINT64_C(0x72086a95adfc0987);
static uint64_t sample(void) {
  seed ^= seed << 13;
  seed ^= seed >> 7;
  seed ^= seed << 17;
  return seed;
}
static int verify(double x, double y) {
  Binary64 want = {.value = fmod(x, y)}, got = {.value = nelibc_fmod_hw(x, y)};
  return (isnan(want.value) && isnan(got.value)) || want.bits == got.bits;
}
int main(void) {
  const double edge[] = {-INFINITY, -0x1.fffffffffffffp+1023, -1, -0.0,
                         0, 0x1p-1074, 0x1.ffffffffffffep-1023,
                         0x1p-1022, 0x1.fffffffffffffp+1023, INFINITY, NAN};
  for (size_t a = 0; a < sizeof edge / sizeof edge[0]; ++a)
    for (size_t b = 0; b < sizeof edge / sizeof edge[0]; ++b)
      if (!verify(edge[a], edge[b])) return 1;
  for (size_t i = 0; i < 50000; ++i) {
    Binary64 a = {.bits = sample()}, b = {.bits = sample()};
    if (!verify(a.value, b.value)) return 2;
  }
  feclearexcept(FE_ALL_EXCEPT);
  if (!isnan(nelibc_fmod_hw(2.0, 0.0)) || !fetestexcept(FE_INVALID)) return 3;
  if (nelibc_sqrt_hw(81.0) != 9.0 || nelibc_sqrtf_hw(49.0f) != 7.0f ||
      !signbit(nelibc_sqrt_hw(-0.0))) return 4;
  puts("AArch64 floating-point primitives passed 50,121 reference comparisons");
}
