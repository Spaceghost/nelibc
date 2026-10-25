#include <math.h>
#include <errno.h>
#include <float.h>
#include <stdint.h>
#include <limits.h>

typedef union {
  double value;
  uint64_t bits;
} double_bits;

typedef union {
  float value;
  uint32_t bits;
} float_bits;

static int is_nan_double(double value) { return value != value; }
static int is_nan_float(float value) { return value != value; }

static int same_double(double a, double b) {
  double_bits left = {.value = a};
  double_bits right = {.value = b};
  return left.bits == right.bits;
}

static int same_float(float a, float b) {
  float_bits left = {.value = a};
  float_bits right = {.value = b};
  return left.bits == right.bits;
}

int main(void) {
  if (!signbit(fminl(-0.0L, 0.0L)) || signbit(fmaxl(-0.0L, 0.0L)) ||
      fdiml(5.0L, 3.0L) != 2.0L || fdiml(3.0L, 5.0L) != 0.0L) return 179;
  if (cbrtl(-0.125L) != -0.5L || cbrtl(LDBL_MIN) <= 0.0L) return 180;
  if (!isnan(nanl("nelibc"))) return 181;
  if (hypotl(3.0L, 4.0L) != 5.0L || hypotl(0.0L, -0.0L) != 0.0L) return 171;
  if (!(nextafterl(1.0L, 2.0L) > 1.0L) ||
      !signbit(nextafterl(0.0L, -1.0L))) return 173;
  if (nexttowardl(1.0L, 2.0L) != nextafterl(1.0L, 2.0L) ||
      !signbit(nexttowardl(0.0L, -1.0L))) return 182;
  if (!(atanl(1.0L) > 0.785L && atanl(1.0L) < 0.786L) ||
      !(atan2l(1.0L, -1.0L) > 2.35L && atan2l(1.0L, -1.0L) < 2.36L)) return 174;
  if (!(asinl(0.5L) > 0.523L && asinl(0.5L) < 0.524L) ||
      !(acosl(-1.0L) > 3.14L && acosl(-1.0L) < 3.15L)) return 175;
  if (!(logl(2.0L) > 0.693L && logl(2.0L) < 0.694L) ||
      !(log2l(8.0L) > 2.999L && log2l(8.0L) < 3.001L) ||
      !(log10l(100.0L) > 1.999L && log10l(100.0L) < 2.001L)) return 176;
  if (!(log1pl(0x1p-60L) > 0.0L) || !(log1pl(-0.5L) < -0.69L)) return 177;
  if (!(expl(1.0L) > 2.718L && expl(1.0L) < 2.719L) ||
      exp2l(3.0L) != 8.0L ||
      !(sinl(1.0L) > 0.841L && sinl(1.0L) < 0.842L) ||
      !(cosl(1.0L) > 0.540L && cosl(1.0L) < 0.541L) ||
      !(tanl(1.0L) > 1.557L && tanl(1.0L) < 1.558L) ||
      !(tanhl(1.0L) > 0.761L && tanhl(1.0L) < 0.762L) ||
      !(sinhl(1.0L) > 1.175L && sinhl(1.0L) < 1.176L) ||
      !(coshl(1.0L) > 1.543L && coshl(1.0L) < 1.544L) ||
      !(asinhl(1.0L) > 0.881L && asinhl(1.0L) < 0.882L) ||
      !(acoshl(2.0L) > 1.316L && acoshl(2.0L) < 1.318L) ||
      !(atanhl(0.5L) > 0.549L && atanhl(0.5L) < 0.550L) ||
      powl(-2.0L, 5.0L) != -32.0L || powl(2.0L, -3.0L) != 0.125L ||
      !(expm1l(0x1p-60L) > 0.0L && expm1l(0x1p-60L) < 0x1p-59L)) return 178;
  errno = 0;
  if (!isinf(hypotl(LDBL_MAX, LDBL_MAX)) || errno != ERANGE) return 172;

  if (sqrtl(4.0L) != 2.0L || fabsl(-3.0L) != 3.0L ||
      copysignl(3.0L, -1.0L) != -3.0L) return 107;
  if (fmodl(5.5L, 2.0L) != 1.5L || remainderl(5.5L, 2.0L) != -0.5L) return 108;
  int long_exponent = 0;
  long double long_integral = 0.0L;
  if (frexpl(12.0L, &long_exponent) != 0.75L || long_exponent != 4 ||
      modfl(-12.75L, &long_integral) != -0.75L || long_integral != -12.0L) return 109;
  if (ilogbl(12.0L) != 3 || logbl(12.0L) != 3.0L ||
      ilogbl(0x1p-16445L) != -16445 || logbl(0x1p-16445L) != -16445.0L) return 110;
  if (ldexpl(1.5L, 4) != 24.0L || scalbnl(-0.75L, -2) != -0.1875L ||
      scalblnl(0.5L, 4) != 8.0L) return 111;
  if (truncl(-1.75L) != -1.0L || floorl(-0.25L) != -1.0L ||
      ceill(-0.25L) != -0.0L || rintl(2.5L) != 2.0L ||
      nearbyintl(-1.5L) != -2.0L || roundl(-0.5L) != -1.0L ||
      roundl(0.5L) != 1.0L || lrintl(2.5L) != 2 || llrintl(-1.5L) != -2 ||
      lroundl(-1.5L) != -2 || llroundl(2.5L) != 3)
    return 112;
  if (exp2(0.5) < 1.4142135623730948 || exp2(0.5) > 1.4142135623730952 ||
      exp2f(0.5f) != 1.4142135381698608f) return 137;
  if (log2(8.0) != 3.0 || log2f(0.5f) != -1.0f) return 136;
  if (!(expm1(0.5) > 0.6487212707001279 && expm1(0.5) < 0.6487212707001283) ||
      !(log1p(0.5) > 0.4054651081081643 && log1p(0.5) < 0.4054651081081645) ||
      expm1(0x1p-54) != 0x1p-54 || log1p(0x1p-54) != 0x1p-54 ||
      expm1f(0x1p-24f) != 0x1p-24f || log1pf(0x1p-24f) != 0x1p-24f) return 135;
  if (exp10(2.0) != 100.0 || exp10f(2.0f) != 100.0f ||
      !(log10(100.0) > 1.999999999999999 && log10(100.0) < 2.000000000000001) ||
      log10f(100.0f) != 2.0f) return 134;
  if (exp(0.0) != 1.0 || !(exp(1.0) > 2.71828182845904 && exp(1.0) < 2.71828182845905)) return 70;
  if (log(1.0) != 0.0 || !(log(2.0) > 0.69314718055994 && log(2.0) < 0.69314718055995)) return 71;
  if (!(expf(1.0f) > 2.71828f && expf(1.0f) < 2.71829f)) return 72;
  if (cbrt(-8.0) != -2.0 || cbrtf(-0.125f) != -0.5f) return 138;
  if (!same_double(cbrt(-0.0), -0.0) || !same_float(cbrtf(-0.0f), -0.0f)) return 139;
  if (!(cbrt(0x1p-1074) > 1.7e-108 && cbrt(0x1p-1074) < 1.71e-108) ||
      !(cbrtf(0x1p-149f) > 1.1e-15f && cbrtf(0x1p-149f) < 1.2e-15f)) return 140;
  if (!(cbrt(0x1.fffffffffffffp+1023) > 5.6e102 &&
        cbrt(0x1.fffffffffffffp+1023) < 5.7e102)) return 141;
  if (!(logf(2.0f) > 0.69314f && logf(2.0f) < 0.69315f)) return 73;
  if (!(sin(1.0) > 0.84147098480789 && sin(1.0) < 0.84147098480790)) return 83;
  if (!(cos(1.0) > 0.54030230586813 && cos(1.0) < 0.54030230586814)) return 84;
  if (!(sinf(1.0f) > 0.8414708f && sinf(1.0f) < 0.8414712f)) return 85;
  if (!(cosf(1.0f) > 0.5403021f && cosf(1.0f) < 0.5403025f)) return 86;
  if (!(tan(1.0) > 1.5574077246549 && tan(1.0) < 1.5574077246550)) return 90;
  if (!(tanf(1.0f) > 1.5574076f && tanf(1.0f) < 1.5574079f)) return 91;
  if (!(sinh(1.0) > 1.1752011936437 && sinh(1.0) < 1.1752011936439)) return 104;
  if (!(cosh(1.0) > 1.5430806348151 && cosh(1.0) < 1.5430806348154)) return 105;
  if (!(tanh(1.0) > 0.7615941559556 && tanh(1.0) < 0.7615941559559)) return 106;
  if (!(sinhf(1.0f) > 1.1752011f && sinhf(1.0f) < 1.1752013f)) return 107;
  if (!(coshf(1.0f) > 1.5430805f && coshf(1.0f) < 1.5430808f)) return 108;
  if (!(tanhf(1.0f) > 0.7615941f && tanhf(1.0f) < 0.7615943f)) return 109;
  if (!(asinh(1.0) > 0.881373587019542 && asinh(1.0) < 0.881373587019544) ||
      !(acosh(2.0) > 1.316957896924816 && acosh(2.0) < 1.316957896924818) ||
      !(atanh(0.5) > 0.549306144334054 && atanh(0.5) < 0.549306144334056)) return 124;
  if (!(asinhf(1.0f) > 0.8813735f && asinhf(1.0f) < 0.8813737f)) return 131;
  if (acoshf(2.0f) != acoshf(2.0f)) return 135;
  if (acoshf(2.0f) < 1.3169578f) return 136;
  if (acoshf(2.0f) > 1.3169580f) return 137;
  if (!(atanhf(0.5f) > 0.5493060f && atanhf(0.5f) < 0.5493063f)) return 133;
  if (!same_double(asinh(-0.0), -0.0) || !same_float(asinhf(-0.0f), -0.0f)) return 126;
  errno = 0;
  if (!is_nan_double(acosh(0.5)) || errno != EDOM) return 127;
  errno = 0;
  if (!isinf(atanh(1.0)) || atanh(1.0) < 0.0 || errno != ERANGE) return 128;
  errno = 0;
  if (!is_nan_float(atanhf(2.0f)) || errno != EDOM) return 129;
  if (!(atan(1.0) > 0.78539816339744 && atan(1.0) < 0.78539816339746)) return 114;
  if (!(atanf(1.0f) > 0.7853981f && atanf(1.0f) < 0.7853983f)) return 115;
  if (!(atan2(1.0, -1.0) > 2.3561944901923 && atan2(1.0, -1.0) < 2.3561944901924)) return 116;
  if (!(atan2f(1.0f, -1.0f) > 2.3561943f && atan2f(1.0f, -1.0f) < 2.3561947f)) return 117;
  if (!(asin(0.5) > 0.52359877559829 && asin(0.5) < 0.52359877559830) ||
      !(acos(0.5) > 1.04719755119659 && acos(0.5) < 1.04719755119660) ||
      !(asinf(-0.5f) < -0.5235987f && asinf(-0.5f) > -0.5235989f) ||
      !(acosf(-0.5f) > 2.0943950f && acosf(-0.5f) < 2.0943953f)) return 142;
  if (fma(0x1.0000000000001p0, 0x1.ffffffffffffep-1, -1.0) != -0x1p-104 ||
      fmaf(0x1.000002p0f, 0x1.fffffcp-1f, -1.0f) != -0x1p-46f)
    return 145;
  if (!(erf(1.0) > 0.8427007929497148 && erf(1.0) < 0.8427007929497150) ||
      !(erfc(1.0) > 0.1572992070502851 && erfc(1.0) < 0.1572992070502852)) return 146;
  if (!(erff(1.0f) > 0.8427007f && erff(1.0f) < 0.8427009f) ||
      !(erfcf(1.0f) > 0.1572991f && erfcf(1.0f) < 0.1572993f)) return 147;
  if (j0(0.0) != 1.0 || j0(-0.0) != 1.0 || j1(0.0) != 0.0 || !signbit(j1(-0.0))) return 152;
  if (fabs(jn(2, 1.0) - 0.1149034849319005) > 2e-14 || fabs(yn(2, 1.0) + 1.650682606816254) > 2e-13) return 160;
  if (fabsf(jnf(2, 1.0f) - 0.11490349f) > 2e-7f || fabsf(ynf(2, 1.0f) + 1.6506826f) > 2e-6f) return 161;
  if (jn(-3, 1.0) != -jn(3, 1.0) || yn(-3, 1.0) != -yn(3, 1.0)) return 162;
  if (jn(3, -1.0) != -jn(3, 1.0)) return 163;
  if (fabs(j0(1.0) - 0.7651976865579666) > 2e-15 || fabs(j1(1.0) - 0.4400505857449335) > 2e-15) return 153;
  if (fabs(y0(1.0) - 0.08825696421567696) > 2e-15 || fabs(y1(1.0) + 0.7812128213002887) > 2e-15) return 154;
  double pair_sin, pair_cos;
  float pair_sinf, pair_cosf;
  sincos(1.0, &pair_sin, &pair_cos);
  sincosf(1.0f, &pair_sinf, &pair_cosf);
  if (fabs(pair_sin - sin(1.0)) > 1e-15 || fabs(pair_cos - cos(1.0)) > 1e-15) return 164;
  if (fabsf(pair_sinf - sinf(1.0f)) > 1e-7f || fabsf(pair_cosf - cosf(1.0f)) > 1e-7f) return 165;
  if (lrint(2.5) != 2 || lrintf(-3.5f) != -4 || llrint(3.5) != 4 ||
      llrintf(-2.5f) != -2 || lround(2.5) != 3 || lroundf(-2.5f) != -3 ||
      llround(3.5) != 4 || llroundf(-3.5f) != -4) return 166;
  int quotient = 0;
  if (remquo(7.0, 2.0, &quotient) != -1.0 || quotient != 4 ||
      remquo(-7.0, 2.0, &quotient) != 1.0 || quotient != -4 ||
      remquof(7.0f, 2.0f, &quotient) != -1.0f || quotient != 4 ||
      remquof(-7.0f, 2.0f, &quotient) != 1.0f || quotient != -4 ||
      remquol(7.0L, 2.0L, &quotient) != -1.0L || quotient != 4 ||
      remquol(-7.0L, 2.0L, &quotient) != 1.0L || quotient != -4) return 167;
  double_bits extension_infinity = {.bits = UINT64_C(0x7FF0000000000000)};
  double_bits extension_nan = {.bits = UINT64_C(0x7FF8000000000000)};
  float_bits extension_infinityf = {.bits = UINT32_C(0x7F800000)};
  float_bits extension_nanf = {.bits = UINT32_C(0x7FC00000)};
  if (!finite(1.0) || finite(extension_infinity.value) || finite(extension_nan.value) ||
      !finitef(1.0f) || finitef(extension_infinityf.value) || finitef(extension_nanf.value) ||
      !isnan(nan("nelibc")) || !isnan(nanf("nelibc"))) return 168;
  if (pow10(2.0) != 100.0 || pow10f(-2.0f) != 0.01f ||
      drem(7.0, 2.0) != -1.0 || dremf(7.0f, 2.0f) != -1.0f) return 169;
  errno = 0;
  if (!isinf(y0(0.0)) || y0(0.0) > 0.0 || errno != ERANGE) return 157;
  errno = 0;
  if (!isinf(y1(0.0)) || y1(0.0) > 0.0 || errno != ERANGE) return 158;
  errno = 0;
  if (!is_nan_double(y0(-1.0)) || errno != EDOM) return 159;
  if (fabsf(j0f(1.0f) - 0.7651977f) > 2e-7f || fabsf(j1f(1.0f) - 0.4400506f) > 2e-7f) return 155;
  if (fabsf(y0f(1.0f) - 0.08825696f) > 2e-7f || fabsf(y1f(1.0f) + 0.7812128f) > 2e-7f) return 156;
  if (fabs(tgamma(5.0) - 24.0) > 0x1p-42 || fabs(tgamma(-0.5) + 3.544907701811032) > 0x1p-42) return 148;
  int gamma_sign = 0;
  if (fabs(lgamma_r(-0.5, &gamma_sign) - 1.2655121234846454) > 0x1p-45 || gamma_sign != -1) return 149;
  if (fabs(lgamma(-0.5) - 1.2655121234846454) > 0x1p-45 || signgam != -1) return 151;
  if (fabsf(tgammaf(5.0f) - 24.0f) > 0x1p-20f || fabsf(lgammaf(5.0f) - 3.1780539f) > 0x1p-20f) return 150;
  errno = 0;
  if (!is_nan_double(asin(2.0)) || errno != EDOM) return 143;
  errno = 0;
  if (!is_nan_float(acosf(-2.0f)) || errno != EDOM) return 144;
  if (pow(2.0, 10.0) != 1024.0 || powf(2.0f, 10.0f) != 1024.0f) return 121;
  if (!(pow(2.0, 0.5) > 1.41421356237309 && pow(2.0, 0.5) < 1.41421356237310) ||
      !(powf(2.0f, 0.5f) > 1.4142134f && powf(2.0f, 0.5f) < 1.4142137f)) return 122;
  if (pow(-2.0, 3.0) != -8.0 || pow(-2.0, 4.0) != 16.0 ||
      pow(-2.0, 0.5) == pow(-2.0, 0.5)) return 123;
  if (hypot(3.0, 4.0) != 5.0 || hypotf(3.0f, 4.0f) != 5.0f) return 78;
  if (fdim(7.0, 2.0) != 5.0 || fdimf(7.0f, 2.0f) != 5.0f) return 99;
  if (!same_double(fdim(-0.0, 0.0), 0.0) || !same_float(fdimf(-0.0f, 0.0f), 0.0f)) return 100;
  if (fdim(2.0, 7.0) != 0.0 || fdimf(2.0f, 7.0f) != 0.0f) return 101;
  errno = 0;
  if (!isinf(fdim(0x1.fffffffffffffp+1023, -0x1.fffffffffffffp+1023)) || errno != ERANGE) return 102;
  errno = 0;
  if (!isinf(fdimf(0x1.fffffep+127f, -0x1.fffffep+127f)) || errno != ERANGE) return 103;
  errno = 0;
  if (!is_nan_double(log(-1.0)) || errno != EDOM) return 74;
  errno = 0;
  if (!isinf(log(0.0)) || log(0.0) > 0.0 || errno != ERANGE) return 75;
  errno = 0;
  if (!isinf(exp(1000.0)) || exp(1000.0) < 0.0 || errno != ERANGE) return 76;
  errno = 0;
  if (exp(-1000.0) != 0.0 || errno != ERANGE) return 77;
  errno = 0;
  if (!isinf(hypot(0x1.fffffffffffffp+1023, 0x1.fffffffffffffp+1023)) || errno != ERANGE) return 79;
  if (hypot(0x1.fffffffffffffp+1023, 0x1p-1074) != 0x1.fffffffffffffp+1023) return 80;
  if (!same_double(fabs(-0.0), 0.0)) return 1;
  if (!same_double(copysign(3.5, -0.0), -3.5)) return 2;
  if (!same_double(trunc(-1.5), -1.0)) return 3;
  if (!same_double(trunc(-0.5), -0.0)) return 4;
  if (!same_double(floor(-0.25), -1.0)) return 5;
  if (!same_double(floor(-0.0), -0.0)) return 6;
  if (!same_double(ceil(-0.25), -0.0)) return 7;
  if (!same_double(ceil(0.25), 1.0)) return 8;
  if (!same_double(round(-0.5), -1.0)) return 9;
  if (!same_double(round(0.5), 1.0)) return 10;
  if (!same_double(rint(2.5), 2.0)) return 62;
  if (!same_double(rint(-1.5), -2.0)) return 63;
  if (!same_double(nearbyint(2.5), 2.0)) return 64;
  if (!same_double(nearbyint(-1.5), -2.0)) return 65;
  if (!same_float(fabsf(-0.0f), 0.0f)) return 11;
  if (!same_float(copysignf(3.5f, -0.0f), -3.5f)) return 12;
  if (!same_float(truncf(-0.5f), -0.0f)) return 13;
  if (!same_float(floorf(-0.25f), -1.0f)) return 14;
  if (!same_float(ceilf(-0.25f), -0.0f)) return 15;
  if (!same_float(roundf(-0.5f), -1.0f)) return 16;
  if (!same_float(rintf(2.5f), 2.0f)) return 66;
  if (!same_float(nearbyintf(-1.5f), -2.0f)) return 67;
  if (!same_double(sqrt(4.0), 2.0)) return 17;
  if (!same_double(sqrt(0.0), 0.0)) return 18;
  double_bits negative_infinity = {.bits = UINT64_C(0xFFF0000000000000)};
  double_bits positive_infinity = {.bits = UINT64_C(0x7FF0000000000000)};
  double_bits quiet_nan = {.bits = UINT64_C(0x7FF8000000000042)};
  double_bits signaling_nan = {.bits = UINT64_C(0x7FF0000000000042)};
  float_bits positive_infinity_f = {.bits = UINT32_C(0x7F800000)};
  float_bits negative_infinity_f = {.bits = UINT32_C(0xFF800000)};
  float_bits quiet_nan_f = {.bits = UINT32_C(0x7FC00042)};
  errno = 0;
  if (!is_nan_double(pow(-2.0, 0.5)) || errno != EDOM) return 124;
  if (pow(quiet_nan.value, 0.0) != 1.0 || pow(1.0, quiet_nan.value) != 1.0) return 125;
  if (!same_double(pow(-0.0, 3.0), -0.0) || !same_float(powf(-0.0f, 3.0f), -0.0f)) return 126;
  errno = 0;
  if (!isinf(pow(0.0, -2.0)) || signbit(pow(0.0, -2.0)) || errno != ERANGE) return 127;
  errno = 0;
  if (!isinf(pow(-0.0, -3.0)) || !signbit(pow(-0.0, -3.0)) || errno != ERANGE) return 128;
  errno = 0;
  if (!isinf(pow(2.0, 1024.0)) || errno != ERANGE) return 129;
  errno = 0;
  if (pow(2.0, -1075.0) != 0.0 || errno != ERANGE) return 130;
  errno = 0;
  if (!isinf(powf(2.0f, 128.0f)) || errno != ERANGE) return 131;
  errno = 0;
  if (powf(2.0f, -150.0f) != 0.0f || errno != ERANGE) return 132;
  if (sin(0.0) != 0.0 || !signbit(sin(-0.0)) || cos(0.0) != 1.0 || cos(-0.0) != 1.0) return 87;
  if (tan(0.0) != 0.0 || !signbit(tan(-0.0))) return 92;
  if (sinh(0.0) != 0.0 || !signbit(sinh(-0.0)) || cosh(0.0) != 1.0 ||
      tanh(0.0) != 0.0 || !signbit(tanh(-0.0))) return 110;
  if (!same_double(atan(-0.0), -0.0) || atan2(-0.0, 0.0) != -0.0 ||
      !signbit(atan2(-0.0, 0.0)) || !same_double(atan2(0.0, -0.0), 0x1.921fb54442d18p+1)) return 118;
  if (!same_float(atanf(-0.0f), -0.0f) || atan2f(-0.0f, 0.0f) != -0.0f ||
      !signbit(atan2f(-0.0f, 0.0f))) return 119;
  if (!isfinite(sin(0x1p900)) || !isfinite(cos(0x1p900)) ||
      sin(0x1p900) < -1.0 || sin(0x1p900) > 1.0 ||
      cos(0x1p900) < -1.0 || cos(0x1p900) > 1.0) return 88;
  errno = 0;
  if (!is_nan_double(sin(positive_infinity.value)) || errno != EDOM) return 89;
  errno = 0;
  if (!is_nan_double(tan(positive_infinity.value)) || errno != EDOM) return 93;
  if (!isinf(sinh(positive_infinity.value)) || !isinf(cosh(positive_infinity.value)) ||
      tanh(positive_infinity.value) != 1.0 || tanh(negative_infinity.value) != -1.0) return 111;
  if (!same_double(atan(positive_infinity.value), 0x1.921fb54442d18p+0) ||
      !same_double(atan(negative_infinity.value), -0x1.921fb54442d18p+0) ||
      !same_double(atan2(positive_infinity.value, positive_infinity.value), 0x1.921fb54442d18p-1) ||
      !same_double(atan2(positive_infinity.value, negative_infinity.value), 0x1.2D97C7F3321D2P+1) ||
      !same_double(atan2(negative_infinity.value, positive_infinity.value), -0x1.921fb54442d18p-1) ||
      !same_double(atan2(negative_infinity.value, negative_infinity.value), -0x1.2D97C7F3321D2P+1)) return 120;
  errno = 0;
  if (!isinf(sinh(1000.0)) || errno != ERANGE) return 112;
  errno = 0;
  if (!isinf(sinhf(100.0f)) || errno != ERANGE) return 113;
  if (hypot(positive_infinity.value, quiet_nan.value) != positive_infinity.value) return 81;
  errno = 0;
  if (!isinf(hypotf(0x1.fffffep+127f, 0x1.fffffep+127f)) || errno != ERANGE) return 82;
  double_bits maximum_finite = {.bits = UINT64_C(0x7FEFFFFFFFFFFFFF)};
  double_bits minimum_normal = {.bits = UINT64_C(0x0010000000000000)};
  double_bits maximum_subnormal = {.bits = UINT64_C(0x000FFFFFFFFFFFFF)};
  float_bits maximum_finite_f = {.bits = UINT32_C(0x7F7FFFFF)};
  float_bits minimum_normal_f = {.bits = UINT32_C(0x00800000)};
  float_bits maximum_subnormal_f = {.bits = UINT32_C(0x007FFFFF)};
  errno = 0;
  if (!is_nan_double(sqrt(-1.0))) return 19;
  if (errno != EDOM) return 46;
  if (!is_nan_double(sqrt(negative_infinity.value))) return 20;
  if (!same_float(sqrtf(9.0f), 3.0f)) return 21;
  if (!is_nan_float(sqrtf(-1.0f))) return 22;
  if (!isinf(negative_infinity.value) || isfinite(negative_infinity.value)) return 23;
  if (fpclassify(0.0) != FP_ZERO || fpclassify(1.0f) != FP_NORMAL) return 24;
  if (fpclassify(1.0e-40f) != FP_SUBNORMAL) return 25;
  if (!same_double(fmod(5.0, 2.0), 1.0)) return 26;
  errno = 0;
  if (!is_nan_double(fmod(1.0, 0.0)) || errno != EDOM) return 47;
  if (!same_double(remainder(7.0, 2.0), -1.0)) return 27;
  errno = 0;
  if (!is_nan_double(remainder(1.0, 0.0)) || errno != EDOM) return 48;
  if (!same_double(fmod(-5.0, 2.0), -1.0)) return 28;
  if (!same_double(remainder(-5.0, 2.0), -1.0)) return 29;
  if (!same_double(fmin(0.0, -0.0), -0.0)) return 30;
  if (!same_double(fmin(-0.0, 0.0), -0.0)) return 31;
  if (!same_double(fmax(-0.0, 0.0), 0.0)) return 32;
  if (!same_double(fmax(0.0, -0.0), 0.0)) return 33;
  if (!same_double(fmin(negative_infinity.value, 2.0), negative_infinity.value)) return 34;
  if (!same_double(fmax(1.0, 2.0), 2.0)) return 35;
  if (!same_double(fmin(quiet_nan.value, 2.0), 2.0)) return 41;
  if (!same_double(fmax(2.0, quiet_nan.value), 2.0)) return 42;
  if (!is_nan_double(fmin(quiet_nan.value, quiet_nan.value))) return 43;
  if (!is_nan_double(fmin(signaling_nan.value, 2.0))) return 44;
  if (!is_nan_double(fmax(2.0, signaling_nan.value))) return 45;
  if (!same_float(fmodf(5.0f, 2.0f), 1.0f)) return 36;
  if (!same_float(remainderf(7.0f, 2.0f), -1.0f)) return 37;
  if (!same_float(fminf(0.0f, -0.0f), -0.0f)) return 38;
  if (!same_float(fminf(-0.0f, 0.0f), -0.0f)) return 39;
  if (!same_float(fmaxf(0.0f, -0.0f), 0.0f)) return 40;
  int exponent = 0;
  if (!same_double(frexp(8.0, &exponent), 0.5) || exponent != 4) return 49;
  double_bits smallest_subnormal = {.bits = UINT64_C(1)};
  if (!same_double(frexp(smallest_subnormal.value, &exponent), 0.5) || exponent != -1073) return 50;
  if (!same_float(frexpf(0x1p-149f, &exponent), 0.5f) || exponent != -148) return 51;
  if (!same_double(frexp(negative_infinity.value, &exponent), negative_infinity.value) || exponent != 0) return 52;
  double integral = 0.0;
  if (!same_double(modf(-3.25, &integral), -0.25) || !same_double(integral, -3.0)) return 53;
  if (!same_double(modf(-0.0, &integral), -0.0) || !same_double(integral, -0.0)) return 54;
  if (!same_double(modf(negative_infinity.value, &integral), -0.0) || !same_double(integral, negative_infinity.value)) return 55;
  float integral_f = 0.0f;
  if (!same_float(modff(-2.75f, &integral_f), -0.75f) || !same_float(integral_f, -2.0f)) return 56;
  if (!same_double(ldexp(0.5, -1073), smallest_subnormal.value)) return 57;
  if (!same_float(ldexpf(1.0f, -149), 0x1p-149f)) return 58;
  if (!same_double(scalbn(0.75, 4), 12.0)) return 59;
  if (!same_float(scalbnf(-0.75f, 4), -12.0f)) return 60;
  if (!same_double(scalbln(0.75, 4), 12.0) || !same_float(scalblnf(-0.75f, 4), -12.0f)) return 98;
  errno = 0;
  if (!isinf(ldexp(1.0, 2048)) || errno != ERANGE) return 61;
  errno = 0;
  if (ldexp(-1.0, -2048) != -0.0 || errno != ERANGE) return 62;
  errno = 0;
  if (!isinf(ldexpf(1.0f, 2048)) || errno != ERANGE) return 63;
  errno = 0;
  if (ldexpf(-1.0f, -2048) != -0.0f || errno != ERANGE) return 64;
  errno = 0;
  if (!same_double(ldexp(1.0, -1074), smallest_subnormal.value) || errno != 0) return 65;
  errno = 0;
  if (!same_double(ldexp(1.25, -1074), smallest_subnormal.value) || errno != 0) return 66;
  errno = 0;
  if (!same_float(ldexpf(1.0f, -149), 0x1p-149f) || errno != 0) return 67;
  errno = 0;
  if (!same_float(ldexpf(1.25f, -149), 0x1p-149f) || errno != 0) return 68;
  if (ilogb(8.0) != 3 || ilogbf(0.125f) != -3) return 70;
  if (ilogb(smallest_subnormal.value) != -1074 || ilogbf(0x1p-149f) != -149) return 71;
  if (ilogb(0.0) != FP_ILOGB0 || ilogbf(-0.0f) != FP_ILOGB0) return 72;
  if (ilogb(negative_infinity.value) != 2147483647 || ilogbf(positive_infinity_f.value) != 2147483647) return 73;
  if (ilogb(quiet_nan.value) != FP_ILOGBNAN) return 74;
  if (!isinf(logb(0.0)) || !signbit(logb(0.0)) || !isinf(logbf(0.0f)) || !signbit(logbf(0.0f))) return 75;
  if (!isinf(logb(negative_infinity.value)) || signbit(logb(negative_infinity.value)) ||
      !isinf(logbf(negative_infinity_f.value)) || signbit(logbf(negative_infinity_f.value))) return 76;
  if (logb(0.125) != -3.0 || logbf(8.0f) != 3.0f) return 77;
  if (!is_nan_double(logb(quiet_nan.value))) return 78;
  errno = 0;
  if (!same_double(nextafter(0.0, 1.0), smallest_subnormal.value) || errno != 0) return 79;
  if (!same_double(nextafter(-0.0, -1.0), -smallest_subnormal.value)) return 80;
  errno = 0;
  if (nextafter(smallest_subnormal.value, 0.0) != 0.0 || errno != ERANGE) return 96;
  if (!same_double(nextafter(1.0, 2.0), 0x1.0000000000001p0)) return 81;
  if (!same_double(nextafter(1.0, 0.0), 0x1.fffffffffffffp-1)) return 82;
  if (!same_double(nextafter(1.0, 1.0), 1.0)) return 83;
  if (!same_double(nextafter(positive_infinity.value, 0.0), maximum_finite.value)) return 84;
  errno = 0;
  if (!isinf(nextafter(maximum_finite.value, positive_infinity.value)) || errno != ERANGE) return 85;
  errno = 0;
  if (!same_double(nextafter(minimum_normal.value, 0.0), maximum_subnormal.value) || errno != ERANGE) return 86;
  if (!same_float(nextafterf(0.0f, 1.0f), 0x1p-149f)) return 87;
  if (!same_float(nextafterf(-0.0f, -1.0f), -0x1p-149f)) return 88;
  errno = 0;
  if (nextafterf(0x1p-149f, 0.0f) != 0.0f || errno != ERANGE) return 97;
  if (!same_float(nextafterf(1.0f, 2.0f), 0x1.000002p0f)) return 89;
  if (!same_float(nextafterf(1.0f, 0.0f), 0x1.fffffep-1f)) return 90;
  if (!same_float(nextafterf(positive_infinity_f.value, 0.0f), maximum_finite_f.value)) return 91;
  errno = 0;
  if (!isinf(nextafterf(maximum_finite_f.value, positive_infinity_f.value)) || errno != ERANGE) return 92;
  errno = 0;
  if (!same_float(nextafterf(minimum_normal_f.value, 0.0f), maximum_subnormal_f.value) || errno != ERANGE) return 93;
  errno = 0;
  if (!isinf(scalbln(0.5, LONG_MAX)) || errno != ERANGE) return 99;
  errno = 0;
  if (scalbln(0.5, LONG_MIN) != 0.0 || errno != ERANGE) return 100;
  errno = 0;
  if (!isinf(scalblnf(0.5f, LONG_MAX)) || errno != ERANGE) return 101;
  errno = 0;
  if (scalblnf(0.5f, LONG_MIN) != 0.0f || errno != ERANGE) return 102;
  if (!same_double(nexttoward(1.0, 1.0L + 0x1p-60L), 0x1.0000000000001p0)) return 103;
  if (!same_double(nexttoward(1.0, 1.0L - 0x1p-60L), 0x1.fffffffffffffp-1)) return 104;
  if (!same_float(nexttowardf(1.0f, 1.0L + 0x1p-30L), 0x1.000002p0f)) return 105;
  if (!same_float(nexttowardf(1.0f, 1.0L - 0x1p-30L), 0x1.fffffep-1f)) return 106;
  if (!is_nan_double(nextafter(quiet_nan.value, 1.0))) return 94;
  if (!is_nan_float(nextafterf(1.0f, quiet_nan_f.value))) return 95;
  return 0;
}
