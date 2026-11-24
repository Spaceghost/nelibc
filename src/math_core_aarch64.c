/* AArch64 primitives for the float and double math entry points. */
#include <float.h>
#include <stdint.h>

_Static_assert(FLT_RADIX == 2 && DBL_MANT_DIG == 53 && DBL_MAX_EXP == 1024 &&
               sizeof(double) == sizeof(uint64_t), "requires binary64 double");

double nelibc_fp_barrier_hw(double value) {
  __asm__ volatile("" : "+w"(value));
  return value;
}

float nelibc_fp_barrierf_hw(float value) {
  __asm__ volatile("" : "+w"(value));
  return value;
}

double nelibc_sqrt_hw(double value) {
  double result;
  __asm__ volatile("fsqrt %d0, %d1" : "=w"(result) : "w"(value));
  return result;
}

float nelibc_sqrtf_hw(float value) {
  float result;
  __asm__ volatile("fsqrt %s0, %s1" : "=w"(result) : "w"(value));
  return result;
}

typedef union {
  double value;
  uint64_t bits;
} binary64;

static const uint64_t significand_bit = UINT64_C(1) << 52;

static uint64_t normalized_significand(uint64_t magnitude, int *exponent) {
  *exponent = (int)(magnitude >> 52);
  uint64_t significand = magnitude & (significand_bit - 1);
  if (*exponent) return significand | significand_bit;
  *exponent = 1;
  while (significand < significand_bit) {
    significand <<= 1;
    --*exponent;
  }
  return significand;
}

double nelibc_fmod_hw(double dividend, double divisor) {
  const uint64_t sign_bit = UINT64_C(1) << 63;
  const uint64_t infinity = UINT64_C(0x7ff0000000000000);
  uint64_t dividend_bits = ((binary64){.value = dividend}).bits;
  uint64_t x = dividend_bits & ~sign_bit;
  uint64_t y = ((binary64){.value = divisor}).bits & ~sign_bit;
  if (x > infinity || y > infinity) return dividend + divisor;
  if (x == infinity || y == 0) {
    volatile double zero = 0;
    return zero / zero;
  }
  if (x < y) return dividend;
  uint64_t sign = dividend_bits & sign_bit;
  if (x == y) return ((binary64){.bits = sign}).value;

  int x_exponent, y_exponent;
  uint64_t remainder = normalized_significand(x, &x_exponent);
  uint64_t modulus = normalized_significand(y, &y_exponent);
  /* Repeatedly reduce the binary significand before shifting to the
     divisor's scale. Integer arithmetic keeps the remainder exact. */
  for (; x_exponent > y_exponent; --x_exponent) {
    if (remainder >= modulus) remainder -= modulus;
    remainder <<= 1;
  }
  if (remainder >= modulus) remainder -= modulus;
  if (remainder == 0) return ((binary64){.bits = sign}).value;

  while (remainder < significand_bit) {
    remainder <<= 1;
    --y_exponent;
  }
  uint64_t magnitude = y_exponent > 0 ?
      ((uint64_t)y_exponent << 52) | (remainder & (significand_bit - 1)) :
      remainder >> (1 - y_exponent);
  return ((binary64){.bits = sign | magnitude}).value;
}
