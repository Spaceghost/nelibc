#define _GNU_SOURCE
#include <math.h>
#include <errno.h>
#include <fenv.h>
#include <float.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>

extern float nelibc_fabsf(float);
extern double nelibc_fabs(double);
extern float nelibc_copysignf(float, float);
extern double nelibc_copysign(double, double);
extern long double nelibc_fabsl(long double);
extern long double nelibc_copysignl(long double, long double);
extern float nelibc_truncf(float);
extern double nelibc_trunc(double);
extern float nelibc_floorf(float);
extern double nelibc_floor(double);
extern float nelibc_ceilf(float);
extern double nelibc_ceil(double);
extern float nelibc_roundf(float);
extern double nelibc_round(double);
extern float nelibc_rintf(float);
extern double nelibc_rint(double);
extern long nelibc_lrint(double);
extern long nelibc_lrintf(float);
extern long long nelibc_llrint(double);
extern long long nelibc_llrintf(float);
extern long nelibc_lround(double);
extern long nelibc_lroundf(float);
extern long long nelibc_llround(double);
extern long long nelibc_llroundf(float);
extern float nelibc_nearbyintf(float);
extern double nelibc_nearbyint(double);
extern float nelibc_expf(float);
extern double nelibc_exp(double);
extern float nelibc_exp2f(float);
extern double nelibc_exp2(double);
extern float nelibc_exp10f(float);
extern double nelibc_exp10(double);
extern float nelibc_pow10f(float);
extern double nelibc_pow10(double);
extern float nelibc_dremf(float, float);
extern double nelibc_drem(double, double);
extern int nelibc_finitef(float);
extern int nelibc_finite(double);
extern float nelibc_nanf(const char *);
extern double nelibc_nan(const char *);
extern float nelibc_expm1f(float);
extern double nelibc_expm1(double);
extern float nelibc_cbrtf(float);
extern double nelibc_cbrt(double);
extern float nelibc_powf(float, float);
extern double nelibc_pow(double, double);
extern float nelibc_logf(float);
extern double nelibc_log(double);
extern float nelibc_log2f(float);
extern double nelibc_log2(double);
extern float nelibc_log10f(float);
extern double nelibc_log10(double);
extern float nelibc_log1pf(float);
extern double nelibc_log1p(double);
extern float nelibc_sinhf(float);
extern double nelibc_sinh(double);
extern float nelibc_coshf(float);
extern double nelibc_cosh(double);
extern float nelibc_tanhf(float);
extern double nelibc_tanh(double);
extern float nelibc_asinhf(float);
extern double nelibc_asinh(double);
extern float nelibc_acoshf(float);
extern double nelibc_acosh(double);
extern float nelibc_atanhf(float);
extern double nelibc_atanh(double);
extern float nelibc_atanf(float);
extern double nelibc_atan(double);
extern float nelibc_atan2f(float, float);
extern double nelibc_atan2(double, double);
extern float nelibc_asinf(float);
extern double nelibc_asin(double);
extern float nelibc_acosf(float);
extern double nelibc_acos(double);
extern float nelibc_fmaf(float, float, float);
extern double nelibc_fma(double, double, double);
extern long double nelibc_fmal(long double, long double, long double);
extern float nelibc_erff(float);
extern double nelibc_erf(double);
extern float nelibc_erfcf(float);
extern double nelibc_erfc(double);
extern double nelibc_jn(int, double);
extern float nelibc_jnf(int, float);
extern double nelibc_yn(int, double);
extern float nelibc_ynf(int, float);
extern double nelibc_j0(double);
extern float nelibc_j0f(float);
extern double nelibc_j1(double);
extern float nelibc_j1f(float);
extern double nelibc_y0(double);
extern float nelibc_y0f(float);
extern double nelibc_y1(double);
extern float nelibc_y1f(float);
extern double nelibc_tgamma(double);
extern float nelibc_tgammaf(float);
extern double nelibc_lgamma(double);
extern float nelibc_lgammaf(float);
extern double nelibc_lgamma_r(double, int *);
extern float nelibc_lgammaf_r(float, int *);
extern float nelibc_sinf(float);
extern double nelibc_sin(double);
extern void nelibc_sincos(double, double *, double *);
extern void nelibc_sincosf(float, float *, float *);
extern void sincos(double, double *, double *);
extern void sincosf(float, float *, float *);
extern float nelibc_cosf(float);
extern double nelibc_cos(double);
extern float nelibc_tanf(float);
extern double nelibc_tan(double);
extern float nelibc_hypotf(float, float);
extern double nelibc_hypot(double, double);
extern long double nelibc_hypotl(long double, long double);
extern long double nelibc_nextafterl(long double, long double);
extern long double nelibc_nexttowardl(long double, long double);
extern long double nelibc_atanl(long double);
extern long double nelibc_atan2l(long double, long double);
extern long double nelibc_asinl(long double);
extern long double nelibc_acosl(long double);
extern long double nelibc_logl(long double);
extern long double nelibc_log2l(long double);
extern long double nelibc_log10l(long double);
extern long double nelibc_log1pl(long double);
extern long double nelibc_expl(long double);
extern long double nelibc_exp2l(long double);
extern long double nelibc_expm1l(long double);
extern long double nelibc_sinl(long double);
extern long double nelibc_cosl(long double);
extern long double nelibc_tanl(long double);
extern long double nelibc_tanhl(long double);
extern long double nelibc_sinhl(long double);
extern long double nelibc_coshl(long double);
extern long double nelibc_asinhl(long double);
extern long double nelibc_acoshl(long double);
extern long double nelibc_atanhl(long double);
extern long double nelibc_powl(long double, long double);
extern long double nelibc_exp10l(long double);
extern long double nelibc_pow10l(long double);
extern long double nelibc_erfl(long double);
extern long double nelibc_erfcl(long double);
extern long double nelibc_tgammal(long double);
extern long double nelibc_lgammal(long double);
extern long double nelibc_lgammal_r(long double, int *);
extern int nelibc___fpclassify(double);
extern int nelibc___fpclassifyf(float);
extern int nelibc___fpclassifyl(long double);
extern int nelibc___signbit(double);
extern int nelibc___signbitf(float);
extern int nelibc___signbitl(long double);
extern long double nelibc_nanl(const char *);
extern long double nelibc_fminl(long double, long double);
extern long double nelibc_fmaxl(long double, long double);
extern long double nelibc_fdiml(long double, long double);
extern long double nelibc_cbrtl(long double);
extern void nelibc_sincosl(long double, long double *, long double *);
extern float nelibc_sqrtf(float);
extern double nelibc_sqrt(double);
extern long double nelibc_sqrtl(long double);
extern long double nelibc_fmodl(long double, long double);
extern long double nelibc_remainderl(long double, long double);
extern long double nelibc_frexpl(long double, int *);
extern long double nelibc_modfl(long double, long double *);
extern int nelibc_ilogbl(long double);
extern long double nelibc_logbl(long double);
extern long double nelibc_ldexpl(long double, int);
extern long double nelibc_scalbnl(long double, int);
extern long double nelibc_scalblnl(long double, long);
extern long double nelibc_truncl(long double);
extern long double nelibc_floorl(long double);
extern long double nelibc_ceill(long double);
extern long double nelibc_rintl(long double);
extern long double nelibc_nearbyintl(long double);
extern long double nelibc_roundl(long double);
extern long nelibc_lrintl(long double);
extern long long nelibc_llrintl(long double);
extern long nelibc_lroundl(long double);
extern long long nelibc_llroundl(long double);
extern float nelibc_fmodf(float, float);
extern double nelibc_fmod(double, double);
extern float nelibc_remainderf(float, float);
extern double nelibc_remainder(double, double);
extern float nelibc_remquof(float, float, int *);
extern double nelibc_remquo(double, double, int *);
extern long double nelibc_remquol(long double, long double, int *);
extern float nelibc_fminf(float, float);
extern double nelibc_fmin(double, double);
extern float nelibc_fmaxf(float, float);
extern double nelibc_fmax(double, double);
extern float nelibc_fdimf(float, float);
extern double nelibc_fdim(double, double);
extern float nelibc_frexpf(float, int *);
extern double nelibc_frexp(double, int *);
extern float nelibc_modff(float, float *);
extern double nelibc_modf(double, double *);
extern float nelibc_ldexpf(float, int);
extern double nelibc_ldexp(double, int);
extern float nelibc_scalbnf(float, int);
extern double nelibc_scalbn(double, int);
extern double nelibc_scalb(double, double);
extern float nelibc_scalbf(float, float);
extern double nelibc_significand(double);
extern float nelibc_significandf(float);
extern int nelibc_ilogb(double);
extern int nelibc_ilogbf(float);
extern double nelibc_logb(double);
extern float nelibc_logbf(float);
extern double nelibc_nextafter(double, double);
extern float nelibc_nextafterf(float, float);
extern double nelibc_scalbln(double, long);
extern float nelibc_scalblnf(float, long);
extern double nelibc_nexttoward(double, long double);
extern float nelibc_nexttowardf(float, long double);

int *nelibc_errno_location(void) { return &errno; }

typedef union {
  uint64_t bits;
  double value;
} double_bits;

typedef union {
  uint32_t bits;
  float value;
} float_bits;

static uint64_t state = UINT64_C(0xD1B54A32D192ED03);
static uint64_t max_exp_ulps, max_log_ulps;
static uint64_t max_erf_ulps, max_erfc_ulps;
static uint32_t max_erff_ulps, max_erfcf_ulps;
static uint32_t max_expf_ulps, max_logf_ulps;
static uint64_t max_sin_ulps, max_cos_ulps;
static uint32_t max_sinf_ulps, max_cosf_ulps;
static uint64_t max_tan_ulps;
static uint32_t max_tanf_ulps;
static uint64_t max_pow_ulps;
static uint32_t max_powf_ulps;
static uint64_t max_cbrt_ulps;
static uint32_t max_cbrtf_ulps;

static uint64_t next_bits(void) {
  state ^= state >> 12;
  state ^= state << 25;
  state ^= state >> 27;
  return state * UINT64_C(0x2545F4914F6CDD1D);
}

static int equal_double(double expected, double actual) {
  if (isnan(expected)) return isnan(actual);
  double_bits a = {.value = expected};
  double_bits b = {.value = actual};
  return a.bits == b.bits;
}

static int equal_float(float expected, float actual) {
  if (isnan(expected)) return isnan(actual);
  float_bits a = {.value = expected};
  float_bits b = {.value = actual};
  return a.bits == b.bits;
}

static uint64_t ordered_double_bits(double value) {
  double_bits bits = {.value = value};
  return (bits.bits >> 63) ? ~bits.bits : (bits.bits | UINT64_C(0x8000000000000000));
}

static uint32_t ordered_float_bits(float value) {
  float_bits bits = {.value = value};
  return (bits.bits >> 31) ? ~bits.bits : (bits.bits | UINT32_C(0x80000000));
}

static uint64_t ulps_double(double expected, double actual) {
  const uint64_t a = ordered_double_bits(expected);
  const uint64_t b = ordered_double_bits(actual);
  return a > b ? a - b : b - a;
}

static uint32_t ulps_float(float expected, float actual) {
  const uint32_t a = ordered_float_bits(expected);
  const uint32_t b = ordered_float_bits(actual);
  return a > b ? a - b : b - a;
}

static int check_sin_cos(double value, float valuef);

static int check_atan(double value, float valuef) {
  const double expected = atan(value);
  const double actual = nelibc_atan(value);
  if (isnan(expected) != isnan(actual) ||
      (!isnan(expected) && ulps_double(expected, actual) > 4)) {
    fprintf(stderr, "atan mismatch x=%.17g expected=%.17g actual=%.17g\n",
            value, expected, actual);
    return 1;
  }
  const float expectedf = atanf(valuef);
  const float actualf = nelibc_atanf(valuef);
  if (isnan(expectedf) != isnan(actualf) ||
      (!isnan(expectedf) && ulps_float(expectedf, actualf) > 4)) {
    fprintf(stderr, "atanf mismatch x=%.9g expected=%.9g actual=%.9g\n",
            valuef, expectedf, actualf);
    return 1;
  }
  return 0;
}

static int check_inverse_trig(double value, float valuef) {
  const double expected_asin = asin(value), actual_asin = nelibc_asin(value);
  const double expected_acos = acos(value), actual_acos = nelibc_acos(value);
  if (isnan(expected_asin) != isnan(actual_asin) ||
      (!isnan(expected_asin) && ulps_double(expected_asin, actual_asin) > 4) ||
      isnan(expected_acos) != isnan(actual_acos) ||
      (!isnan(expected_acos) && ulps_double(expected_acos, actual_acos) > 4)) {
    fprintf(stderr, "inverse trig mismatch x=%.17g asin=%.17g/%.17g acos=%.17g/%.17g\n",
            value, expected_asin, actual_asin, expected_acos, actual_acos);
    return 1;
  }
  const float expected_asinf = asinf(valuef), actual_asinf = nelibc_asinf(valuef);
  const float expected_acosf = acosf(valuef), actual_acosf = nelibc_acosf(valuef);
  if (isnan(expected_asinf) != isnan(actual_asinf) ||
      (!isnan(expected_asinf) && ulps_float(expected_asinf, actual_asinf) > 4) ||
      isnan(expected_acosf) != isnan(actual_acosf) ||
      (!isnan(expected_acosf) && ulps_float(expected_acosf, actual_acosf) > 4)) {
    fprintf(stderr, "inverse trig float mismatch x=%.9g asinf=%.9g/%.9g acosf=%.9g/%.9g\n",
            valuef, expected_asinf, actual_asinf, expected_acosf, actual_acosf);
    return 1;
  }
  return 0;
}

static int check_fma(double x, double y, double z,
                     float xf, float yf, float zf) {
  const double expected = fma(x, y, z), actual = nelibc_fma(x, y, z);
  if (isnan(expected) != isnan(actual) ||
      (!isnan(expected) && !equal_double(expected, actual))) {
    fprintf(stderr, "fma mismatch x=%a y=%a z=%a expected=%a actual=%a\n",
            x, y, z, expected, actual);
    return 1;
  }
  const float expectedf = fmaf(xf, yf, zf), actualf = nelibc_fmaf(xf, yf, zf);
  if (isnan(expectedf) != isnan(actualf) ||
      (!isnan(expectedf) && !equal_float(expectedf, actualf))) {
    fprintf(stderr, "fmaf mismatch x=%a y=%a z=%a expected=%a actual=%a\n",
            xf, yf, zf, expectedf, actualf);
    return 1;
  }
  return 0;
}

static int check_classification(void) {
  static const double values[] = {
      0.0, -0.0, 0x1p-1074, -0x1p-1074, DBL_MIN, -DBL_MIN,
      1.0, -1.0, INFINITY, -INFINITY, NAN};
  static const float valuesf[] = {
      0.0f, -0.0f, 0x1p-149f, -0x1p-149f, FLT_MIN, -FLT_MIN,
      1.0f, -1.0f, INFINITY, -INFINITY, NAN};
  static const long double valuesl[] = {
      0.0L, -0.0L, 0x1p-16445L, -0x1p-16445L, LDBL_MIN, -LDBL_MIN,
      1.0L, -1.0L, INFINITY, -INFINITY, NAN};
  for (size_t i = 0; i < sizeof values / sizeof values[0]; ++i) {
    if (nelibc___fpclassify(values[i]) != __fpclassify(values[i]) ||
        (!!nelibc___signbit(values[i])) != (!!__signbit(values[i])) ||
        nelibc___fpclassifyf(valuesf[i]) != __fpclassifyf(valuesf[i]) ||
        (!!nelibc___signbitf(valuesf[i])) != (!!__signbitf(valuesf[i])) ||
        nelibc___fpclassifyl(valuesl[i]) != __fpclassifyl(valuesl[i]) ||
        (!!nelibc___signbitl(valuesl[i])) != (!!__signbitl(valuesl[i])) ) {
      fprintf(stderr, "classification/signbit mismatch at %zu: d=%d/%d,%d/%d f=%d/%d,%d/%d l=%d/%d,%d/%d\n",
              i, nelibc___fpclassify(values[i]), __fpclassify(values[i]),
              nelibc___signbit(values[i]), __signbit(values[i]),
              nelibc___fpclassifyf(valuesf[i]), __fpclassifyf(valuesf[i]),
              nelibc___signbitf(valuesf[i]), __signbitf(valuesf[i]),
              nelibc___fpclassifyl(valuesl[i]), __fpclassifyl(valuesl[i]),
              nelibc___signbitl(valuesl[i]), __signbitl(valuesl[i]));
      return 1;
    }
  }
  return 0;
}

static int check_fmal(long double x, long double y, long double z) {
  feclearexcept(FE_ALL_EXCEPT);
  errno = 0;
  const long double expected = fmal(x, y, z);
  const int expected_flags = fetestexcept(FE_ALL_EXCEPT);
  const int expected_errno = errno;
  feclearexcept(FE_ALL_EXCEPT);
  errno = 0;
  const long double actual = nelibc_fmal(x, y, z);
  const int actual_flags = fetestexcept(FE_ALL_EXCEPT);
  const int actual_errno = errno;
  /* musl's fmal can report FE_INEXACT for exact cancellation cases, so it is
     not a reliable fenv oracle. Keep value comparisons in every profile. */
#ifdef NELIBC_MUSL_PROFILE
  const int flags_match = 1;
#else
  const int flags_match = expected_flags == actual_flags;
#endif
  if (isnan(expected) != isnan(actual) ||
      (!isnan(expected) && (expected != actual ||
       (expected == 0.0L && signbit(expected) != signbit(actual)))) ||
      !flags_match) {
    fprintf(stderr, "fmal mismatch x=%.21La y=%.21La z=%.21La expected=%.21La/%x/%d actual=%.21La/%x/%d mode=%d\n",
            x, y, z, expected, expected_flags, expected_errno,
            actual, actual_flags, actual_errno, fegetround());
    return 1;
  }
  return 0;
}

static int check_hyperbolic(double value, float valuef) {
  const double expected[] = {sinh(value), cosh(value), tanh(value)};
  const double actual[] = {nelibc_sinh(value), nelibc_cosh(value), nelibc_tanh(value)};
  const char *names[] = {"sinh", "cosh", "tanh"};
  for (size_t i = 0; i < 3; ++i) {
    if (isnan(expected[i]) != isnan(actual[i]) ||
        (!isnan(expected[i]) && ulps_double(expected[i], actual[i]) > 4)) {
      fprintf(stderr, "%s mismatch x=%.17g expected=%.17g actual=%.17g\n",
              names[i], value, expected[i], actual[i]);
      return 1;
    }
  }
  const float expectedf[] = {sinhf(valuef), coshf(valuef), tanhf(valuef)};
  const float actualf[] = {nelibc_sinhf(valuef), nelibc_coshf(valuef), nelibc_tanhf(valuef)};
  const char *namesf[] = {"sinhf", "coshf", "tanhf"};
  for (size_t i = 0; i < 3; ++i) {
    if (isnan(expectedf[i]) != isnan(actualf[i]) ||
        (!isnan(expectedf[i]) && ulps_float(expectedf[i], actualf[i]) > 4)) {
      fprintf(stderr, "%s mismatch x=%.9g expected=%.9g actual=%.9g\n",
              namesf[i], valuef, expectedf[i], actualf[i]);
      return 1;
    }
  }
  return 0;
}

static int check_exp_log(double value, float valuef) {
  const double expected_exp = exp(value);
  const double actual_exp = nelibc_exp(value);
  if (!isnan(expected_exp) && !isnan(actual_exp) &&
      ulps_double(expected_exp, actual_exp) > max_exp_ulps)
    max_exp_ulps = ulps_double(expected_exp, actual_exp);
  if ((!isnan(expected_exp) && !isnan(actual_exp) && ulps_double(expected_exp, actual_exp) > 4) ||
      (isnan(expected_exp) != isnan(actual_exp))) {
    fprintf(stderr, "exp mismatch x=%.17g expected=%.17g actual=%.17g ulps=%llu\n",
            value, expected_exp, actual_exp,
            (unsigned long long)ulps_double(expected_exp, actual_exp));
    return 1;
  }
  const float expected_expf = expf(valuef);
  const float actual_expf = nelibc_expf(valuef);
  if (!isnan(expected_expf) && !isnan(actual_expf) &&
      ulps_float(expected_expf, actual_expf) > max_expf_ulps)
    max_expf_ulps = ulps_float(expected_expf, actual_expf);
  if ((!isnan(expected_expf) && !isnan(actual_expf) && ulps_float(expected_expf, actual_expf) > 4) ||
      (isnan(expected_expf) != isnan(actual_expf))) {
    fprintf(stderr, "expf mismatch x=%.9g expected=%.9g actual=%.9g ulps=%u\n",
            valuef, expected_expf, actual_expf,
            ulps_float(expected_expf, actual_expf));
    return 1;
  }
  const double expected_exp2 = exp2(value), actual_exp2 = nelibc_exp2(value);
  const float expected_exp2f = exp2f(valuef), actual_exp2f = nelibc_exp2f(valuef);
  if ((!isnan(expected_exp2) && !isnan(actual_exp2) && ulps_double(expected_exp2, actual_exp2) > 4) ||
      (!isnan(expected_exp2f) && !isnan(actual_exp2f) && ulps_float(expected_exp2f, actual_exp2f) > 4) ||
      isnan(expected_exp2) != isnan(actual_exp2) || isnan(expected_exp2f) != isnan(actual_exp2f)) {
    fprintf(stderr, "exp2 mismatch input=%.17g/%.9g expected=%.17g actual=%.17g\n",
            value, valuef, expected_exp2, actual_exp2);
    return 1;
  }
  const double log_input = fabs(value);
  const float log_inputf = fabsf(valuef);
  const double expected_log = log(log_input);
  const double actual_log = nelibc_log(log_input);
  if (!isnan(expected_log) && !isnan(actual_log) &&
      ulps_double(expected_log, actual_log) > max_log_ulps)
    max_log_ulps = ulps_double(expected_log, actual_log);
  if ((!isnan(expected_log) && !isnan(actual_log) && ulps_double(expected_log, actual_log) > 4) ||
      (isnan(expected_log) != isnan(actual_log))) {
    fprintf(stderr, "log mismatch x=%.17g expected=%.17g actual=%.17g ulps=%llu\n",
            log_input, expected_log, actual_log,
            (unsigned long long)ulps_double(expected_log, actual_log));
    return 1;
  }
  const float expected_logf = logf(log_inputf);
  const float actual_logf = nelibc_logf(log_inputf);
  if (!isnan(expected_logf) && !isnan(actual_logf) &&
      ulps_float(expected_logf, actual_logf) > max_logf_ulps)
    max_logf_ulps = ulps_float(expected_logf, actual_logf);
  if ((!isnan(expected_logf) && !isnan(actual_logf) && ulps_float(expected_logf, actual_logf) > 4) ||
      (isnan(expected_logf) != isnan(actual_logf))) {
    fprintf(stderr, "logf mismatch x=%.9g expected=%.9g actual=%.9g ulps=%u\n",
            log_inputf, expected_logf, actual_logf,
            ulps_float(expected_logf, actual_logf));
    return 1;
  }
  const double expected_log2 = log2(log_input), actual_log2 = nelibc_log2(log_input);
  const float expected_log2f = log2f(log_inputf), actual_log2f = nelibc_log2f(log_inputf);
  if ((!isnan(expected_log2) && !isnan(actual_log2) && log_input >= DBL_MIN &&
       ulps_double(expected_log2, actual_log2) > 4) ||
      (!isnan(expected_log2f) && !isnan(actual_log2f) && ulps_float(expected_log2f, actual_log2f) > 4) ||
      (log_input > 0.0 && log_input < DBL_MIN &&
       (!(actual_log2 <= -1022.0 && actual_log2 >= -1075.0))) ||
      isnan(expected_log2) != isnan(actual_log2) || isnan(expected_log2f) != isnan(actual_log2f)) {
    fprintf(stderr, "log2 mismatch input=%.17g/%.9g expected=%.17g actual=%.17g log=%.17g expectedlog=%.17g\n",
            log_input, log_inputf, expected_log2, actual_log2, actual_log, expected_log);
    return 1;
  }
  const double expected_expm1 = expm1(value), actual_expm1 = nelibc_expm1(value);
  const float expected_expm1f = expm1f(valuef), actual_expm1f = nelibc_expm1f(valuef);
  const double expected_log1p = log1p(value), actual_log1p = nelibc_log1p(value);
  const float expected_log1pf = log1pf(valuef), actual_log1pf = nelibc_log1pf(valuef);
  const double expected_exp10 = exp10(value), actual_exp10 = nelibc_exp10(value);
  const float expected_exp10f = exp10f(valuef), actual_exp10f = nelibc_exp10f(valuef);
  const double expected_log10 = log10(log_input), actual_log10 = nelibc_log10(log_input);
  const float expected_log10f = log10f(log_inputf), actual_log10f = nelibc_log10f(log_inputf);
  if (isnan(expected_expm1) != isnan(actual_expm1) ||
      (!isnan(expected_expm1) && !isinf(expected_expm1) && ulps_double(expected_expm1, actual_expm1) > 4) ||
      isnan(expected_expm1f) != isnan(actual_expm1f) ||
      (!isnan(expected_expm1f) && !isinf(expected_expm1f) && ulps_float(expected_expm1f, actual_expm1f) > 4) ||
      isnan(expected_log1p) != isnan(actual_log1p) ||
      (!isnan(expected_log1p) && !isinf(expected_log1p) && ulps_double(expected_log1p, actual_log1p) > 4) ||
      isnan(expected_log1pf) != isnan(actual_log1pf) ||
      (!isnan(expected_log1pf) && !isinf(expected_log1pf) && ulps_float(expected_log1pf, actual_log1pf) > 4) ||
      isnan(expected_exp10) != isnan(actual_exp10) ||
      (!isnan(expected_exp10) && !isinf(expected_exp10) && ulps_double(expected_exp10, actual_exp10) > 8) ||
      isnan(expected_exp10f) != isnan(actual_exp10f) ||
      (!isnan(expected_exp10f) && !isinf(expected_exp10f) && ulps_float(expected_exp10f, actual_exp10f) > 8) ||
      isnan(expected_log10) != isnan(actual_log10) ||
      (!isnan(expected_log10) && !isinf(expected_log10) && log_input >= DBL_MIN &&
       ulps_double(expected_log10, actual_log10) > 4) ||
      (log_input > 0.0 && log_input < DBL_MIN &&
       (!(actual_log10 <= -307.0 && actual_log10 >= -324.0))) ||
      isnan(expected_log10f) != isnan(actual_log10f) ||
      (!isnan(expected_log10f) && !isinf(expected_log10f) && ulps_float(expected_log10f, actual_log10f) > 4)) {
    fprintf(stderr, "expm1/log1p/exp10/log10 mismatch x=%.17g/%.9g expm1 %.17g %.17g expm1f %.9g %.9g log1p %.17g %.17g log1pf %.9g %.9g exp10 %.17g %.17g exp10f %.9g %.9g log10 %.17g %.17g log10f %.9g %.9g\n",
            value, valuef, expected_expm1, actual_expm1, expected_expm1f, actual_expm1f,
            expected_log1p, actual_log1p, expected_log1pf, actual_log1pf,
            expected_exp10, actual_exp10, expected_exp10f, actual_exp10f,
            expected_log10, actual_log10, expected_log10f, actual_log10f);
    return 1;
  }
  const double expected_cbrt = cbrt(value);
  const double actual_cbrt = nelibc_cbrt(value);
  if (isnan(expected_cbrt) != isnan(actual_cbrt) ||
      (!isnan(expected_cbrt) && ulps_double(expected_cbrt, actual_cbrt) > 4)) {
    fprintf(stderr, "cbrt binary64 mismatch x=%.17g expected=%.17g actual=%.17g ulps=%llu\n",
            value, expected_cbrt, actual_cbrt,
            (unsigned long long)ulps_double(expected_cbrt, actual_cbrt));
    return 1;
  }
  if (!isnan(expected_cbrt) && ulps_double(expected_cbrt, actual_cbrt) > max_cbrt_ulps)
    max_cbrt_ulps = ulps_double(expected_cbrt, actual_cbrt);
  const float expected_cbrtf = cbrtf(valuef);
  const float actual_cbrtf = nelibc_cbrtf(valuef);
  if (isnan(expected_cbrtf) != isnan(actual_cbrtf) ||
      (!isnan(expected_cbrtf) && ulps_float(expected_cbrtf, actual_cbrtf) > 4)) {
    fprintf(stderr, "cbrtf binary32 mismatch x=%.9g expected=%.9g actual=%.9g ulps=%u\n",
            valuef, expected_cbrtf, actual_cbrtf,
            ulps_float(expected_cbrtf, actual_cbrtf));
    return 1;
  }
  if (!isnan(expected_cbrtf) && ulps_float(expected_cbrtf, actual_cbrtf) > max_cbrtf_ulps)
    max_cbrtf_ulps = ulps_float(expected_cbrtf, actual_cbrtf);
  const double expected_asinh = asinh(value);
  const double actual_asinh = nelibc_asinh(value);
  /* Zig's bundled musl acosh also returns finite values below its x >= 1
     domain; use the C-specified NaN result for that input range. */
  const double expected_acosh = value < 1.0 ? NAN : acosh(value);
  const double actual_acosh = nelibc_acosh(value);
  const double expected_atanh = atanh(value);
  const double actual_atanh = nelibc_atanh(value);
  if (isnan(expected_asinh) != isnan(actual_asinh) ||
      (!isnan(expected_asinh) && ulps_double(expected_asinh, actual_asinh) > 8) ||
      isnan(expected_acosh) != isnan(actual_acosh) ||
      (!isnan(expected_acosh) && ulps_double(expected_acosh, actual_acosh) > 8) ||
      isnan(expected_atanh) != isnan(actual_atanh) ||
      (!isnan(expected_atanh) && ulps_double(expected_atanh, actual_atanh) > 8)) {
    fprintf(stderr, "inverse hyperbolic binary64 mismatch x=%.17g asinh %.17g/%.17g acosh %.17g/%.17g atanh %.17g/%.17g\n",
            value, expected_asinh, actual_asinh, expected_acosh, actual_acosh,
            expected_atanh, actual_atanh);
    return 1;
  }
  const float expected_asinhf = asinhf(valuef);
  const float actual_asinhf = nelibc_asinhf(valuef);
  /* Zig's bundled musl acoshf returns a finite value for negative inputs;
     the C domain is x >= 1, so use the specified result for that region. */
  const float expected_acoshf = valuef < 1.0f ? NAN : acoshf(valuef);
  const float actual_acoshf = nelibc_acoshf(valuef);
  const float expected_atanhf = atanhf(valuef);
  const float actual_atanhf = nelibc_atanhf(valuef);
  if (isnan(expected_asinhf) != isnan(actual_asinhf) ||
      (!isnan(expected_asinhf) && ulps_float(expected_asinhf, actual_asinhf) > 8) ||
      isnan(expected_acoshf) != isnan(actual_acoshf) ||
      (!isnan(expected_acoshf) && ulps_float(expected_acoshf, actual_acoshf) > 8) ||
      isnan(expected_atanhf) != isnan(actual_atanhf) ||
      (!isnan(expected_atanhf) && ulps_float(expected_atanhf, actual_atanhf) > 8)) {
    fprintf(stderr, "inverse hyperbolic binary32 mismatch x=%.9g asinh %.9g/%.9g acosh %.9g/%.9g atanh %.9g/%.9g\n",
            valuef, expected_asinhf, actual_asinhf, expected_acoshf, actual_acoshf,
            expected_atanhf, actual_atanhf);
    return 1;
  }
  if (check_sin_cos(value, valuef)) return 1;
  if (check_hyperbolic(value, valuef)) return 1;
  return check_atan(value, valuef);
}

static int check_log10_edge(double value) {
  const double expected = log10(value), actual = nelibc_log10(value);
  if (isnan(expected) != isnan(actual) ||
      (!isnan(expected) && !isinf(expected) && value >= DBL_MIN &&
       ulps_double(expected, actual) > 4) ||
      (value > 0.0 && value < DBL_MIN &&
       !(actual >= -324.0 && actual <= -307.0))) {
    fprintf(stderr, "log10 binary64 edge mismatch x=%.17g expected=%.17g actual=%.17g\n",
            value, expected, actual);
    return 1;
  }
  const float inputf = (float)value;
  const float expectedf = log10f(inputf), actualf = nelibc_log10f(inputf);
  if (isnan(expectedf) != isnan(actualf) ||
      (!isnan(expectedf) && !isinf(expectedf) && ulps_float(expectedf, actualf) > 4)) {
    fprintf(stderr, "log10 binary32 edge mismatch x=%.9g expected=%.9g actual=%.9g\n",
            inputf, expectedf, actualf);
    return 1;
  }
  return 0;
}

static int check_sin_cos(double value, float valuef) {
  const double expected_sin = sin(value);
  const double actual_sin = nelibc_sin(value);
  const double expected_cos = cos(value);
  const double actual_cos = nelibc_cos(value);
  double pair_sin, pair_cos, expected_pair_sin, expected_pair_cos;
  sincos(value, &expected_pair_sin, &expected_pair_cos);
  nelibc_sincos(value, &pair_sin, &pair_cos);
  if (isnan(expected_pair_sin) != isnan(pair_sin) ||
      (!isnan(expected_pair_sin) && ulps_double(expected_pair_sin, pair_sin) > 4) ||
      isnan(expected_pair_cos) != isnan(pair_cos) ||
      (!isnan(expected_pair_cos) && ulps_double(expected_pair_cos, pair_cos) > 4)) {
    fprintf(stderr, "sincos mismatch x=%.17g expected=(%.17g,%.17g) actual=(%.17g,%.17g)\n",
            value, expected_pair_sin, expected_pair_cos, pair_sin, pair_cos);
    return 1;
  }
  if (isnan(expected_sin) != isnan(actual_sin) ||
      (!isnan(expected_sin) && ulps_double(expected_sin, actual_sin) > 4)) {
    fprintf(stderr, "sin mismatch x=%.17g expected=%.17g actual=%.17g ulps=%llu\n",
            value, expected_sin, actual_sin,
            (unsigned long long)ulps_double(expected_sin, actual_sin));
    return 1;
  }
  if (isnan(expected_cos) != isnan(actual_cos) ||
      (!isnan(expected_cos) && ulps_double(expected_cos, actual_cos) > 4)) {
    fprintf(stderr, "cos mismatch x=%.17g expected=%.17g actual=%.17g ulps=%llu\n",
            value, expected_cos, actual_cos,
            (unsigned long long)ulps_double(expected_cos, actual_cos));
    return 1;
  }
  if (!isnan(expected_sin) && ulps_double(expected_sin, actual_sin) > max_sin_ulps)
    max_sin_ulps = ulps_double(expected_sin, actual_sin);
  if (!isnan(expected_cos) && ulps_double(expected_cos, actual_cos) > max_cos_ulps)
    max_cos_ulps = ulps_double(expected_cos, actual_cos);

  const float expected_sinf = sinf(valuef);
  const float actual_sinf = nelibc_sinf(valuef);
  const float expected_cosf = cosf(valuef);
  const float actual_cosf = nelibc_cosf(valuef);
  float pair_sinf, pair_cosf, expected_pair_sinf, expected_pair_cosf;
  sincosf(valuef, &expected_pair_sinf, &expected_pair_cosf);
  nelibc_sincosf(valuef, &pair_sinf, &pair_cosf);
  if (isnan(expected_pair_sinf) != isnan(pair_sinf) ||
      (!isnan(expected_pair_sinf) && ulps_float(expected_pair_sinf, pair_sinf) > 4) ||
      isnan(expected_pair_cosf) != isnan(pair_cosf) ||
      (!isnan(expected_pair_cosf) && ulps_float(expected_pair_cosf, pair_cosf) > 4)) {
    fprintf(stderr, "sincosf mismatch x=%.9g expected=(%.9g,%.9g) actual=(%.9g,%.9g)\n",
            valuef, expected_pair_sinf, expected_pair_cosf, pair_sinf, pair_cosf);
    return 1;
  }
  if (isnan(expected_sinf) != isnan(actual_sinf) ||
      (!isnan(expected_sinf) && ulps_float(expected_sinf, actual_sinf) > 4)) {
    fprintf(stderr, "sinf mismatch x=%.9g expected=%.9g actual=%.9g ulps=%u\n",
            valuef, expected_sinf, actual_sinf,
            ulps_float(expected_sinf, actual_sinf));
    return 1;
  }
  if (isnan(expected_cosf) != isnan(actual_cosf) ||
      (!isnan(expected_cosf) && ulps_float(expected_cosf, actual_cosf) > 4)) {
    fprintf(stderr, "cosf mismatch x=%.9g expected=%.9g actual=%.9g ulps=%u\n",
            valuef, expected_cosf, actual_cosf,
            ulps_float(expected_cosf, actual_cosf));
    return 1;
  }
  if (!isnan(expected_sinf) && ulps_float(expected_sinf, actual_sinf) > max_sinf_ulps)
    max_sinf_ulps = ulps_float(expected_sinf, actual_sinf);
  if (!isnan(expected_cosf) && ulps_float(expected_cosf, actual_cosf) > max_cosf_ulps)
    max_cosf_ulps = ulps_float(expected_cosf, actual_cosf);
  const double expected_tan = tan(value);
  const double actual_tan = nelibc_tan(value);
  if (isnan(expected_tan) != isnan(actual_tan) ||
      (!isnan(expected_tan) && ulps_double(expected_tan, actual_tan) > 4)) {
    fprintf(stderr, "tan mismatch x=%.17g expected=%.17g actual=%.17g ulps=%llu\n",
            value, expected_tan, actual_tan,
            (unsigned long long)ulps_double(expected_tan, actual_tan));
    return 1;
  }
  if (!isnan(expected_tan) && ulps_double(expected_tan, actual_tan) > max_tan_ulps)
    max_tan_ulps = ulps_double(expected_tan, actual_tan);
  const float expected_tanf = tanf(valuef);
  const float actual_tanf = nelibc_tanf(valuef);
  if (isnan(expected_tanf) != isnan(actual_tanf) ||
      (!isnan(expected_tanf) && ulps_float(expected_tanf, actual_tanf) > 4)) {
    fprintf(stderr, "tanf mismatch x=%.9g expected=%.9g actual=%.9g ulps=%u\n",
            valuef, expected_tanf, actual_tanf,
            ulps_float(expected_tanf, actual_tanf));
    return 1;
  }
  if (!isnan(expected_tanf) && ulps_float(expected_tanf, actual_tanf) > max_tanf_ulps)
    max_tanf_ulps = ulps_float(expected_tanf, actual_tanf);
  return 0;
}

static int check_double(uint64_t input_bits) {
  double_bits input = {.bits = input_bits};
  const double value = input.value;
  const double expected[] = {
      fabs(value), trunc(value), floor(value), ceil(value), round(value),
      copysign(value, -1.0), sqrt(value)};
  const double actual[] = {
      nelibc_fabs(value), nelibc_trunc(value), nelibc_floor(value),
      nelibc_ceil(value), nelibc_round(value), nelibc_copysign(value, -1.0),
      nelibc_sqrt(value)};
  for (size_t i = 0; i < sizeof expected / sizeof expected[0]; ++i) {
    if (!equal_double(expected[i], actual[i])) {
      fprintf(stderr, "binary64 mismatch case=%016llx operation=%zu\n",
              (unsigned long long)input_bits, i);
      return 1;
    }
  }
  int expected_exponent = 0, actual_exponent = 0;
  double expected_fraction = frexp(value, &expected_exponent);
  double actual_fraction = nelibc_frexp(value, &actual_exponent);
  if (expected_exponent != actual_exponent || !equal_double(expected_fraction, actual_fraction)) {
    fprintf(stderr, "frexp mismatch input=%016llx expected=%.17g/%d actual=%.17g/%d\n",
            (unsigned long long)input_bits, expected_fraction, expected_exponent,
            actual_fraction, actual_exponent);
    return 1;
  }
  double expected_integral = 0.0, actual_integral = 0.0;
  expected_fraction = modf(value, &expected_integral);
  actual_fraction = nelibc_modf(value, &actual_integral);
  if (!equal_double(expected_integral, actual_integral) || !equal_double(expected_fraction, actual_fraction)) {
    fprintf(stderr, "modf mismatch input=%016llx\n", (unsigned long long)input_bits);
    return 1;
  }
  const int scale = (int)((input_bits >> 32) % 4097) - 2048;
  if (!equal_double(ldexp(value, scale), nelibc_ldexp(value, scale)) ||
      !equal_double(scalbn(value, scale), nelibc_scalbn(value, scale)) ||
      !equal_double(scalb(value, (double)scale), nelibc_scalb(value, (double)scale))) {
    fprintf(stderr, "ldexp/scalbn mismatch input=%016llx exponent=%d\n",
            (unsigned long long)input_bits, scale);
    return 1;
  }
  const long wide_scale = (long)((input_bits >> 16) % 8193) - 4096;
  if (!equal_double(scalbln(value, wide_scale), nelibc_scalbln(value, wide_scale))) {
    fprintf(stderr, "scalbln mismatch input=%016llx exponent=%ld\n",
            (unsigned long long)input_bits, wide_scale);
    return 1;
  }
  if (ilogb(value) != nelibc_ilogb(value) || !equal_double(logb(value), nelibc_logb(value))) {
    fprintf(stderr, "ilogb/logb mismatch input=%016llx\n", (unsigned long long)input_bits);
    return 1;
  }
  if (isfinite(value) && value != 0.0 &&
      !equal_double(significand(value), nelibc_significand(value))) {
    fprintf(stderr, "significand mismatch input=%016llx\n", (unsigned long long)input_bits);
    return 1;
  }
  return 0;
}

static int check_float(uint32_t input_bits) {
  float_bits input = {.bits = input_bits};
  const float value = input.value;
  const float expected[] = {
      fabsf(value), truncf(value), floorf(value), ceilf(value), roundf(value),
      copysignf(value, -1.0f), sqrtf(value)};
  const float actual[] = {
      nelibc_fabsf(value), nelibc_truncf(value), nelibc_floorf(value),
      nelibc_ceilf(value), nelibc_roundf(value), nelibc_copysignf(value, -1.0f),
      nelibc_sqrtf(value)};
  for (size_t i = 0; i < sizeof expected / sizeof expected[0]; ++i) {
    if (!equal_float(expected[i], actual[i])) {
      fprintf(stderr, "binary32 mismatch case=%08x operation=%zu\n",
              input_bits, i);
      return 1;
    }
  }
  int expected_exponent = 0, actual_exponent = 0;
  float expected_fraction = frexpf(value, &expected_exponent);
  float actual_fraction = nelibc_frexpf(value, &actual_exponent);
  if (expected_exponent != actual_exponent || !equal_float(expected_fraction, actual_fraction)) {
    fprintf(stderr, "frexpf mismatch input=%08x expected=%g/%d actual=%g/%d\n",
            input_bits, expected_fraction, expected_exponent, actual_fraction, actual_exponent);
    return 1;
  }
  float expected_integral = 0.0f, actual_integral = 0.0f;
  expected_fraction = modff(value, &expected_integral);
  actual_fraction = nelibc_modff(value, &actual_integral);
  if (!equal_float(expected_integral, actual_integral) || !equal_float(expected_fraction, actual_fraction)) {
    fprintf(stderr, "modff mismatch input=%08x\n", input_bits);
    return 1;
  }
  const int scale = (int)(input_bits % 1025) - 512;
  if (!equal_float(ldexpf(value, scale), nelibc_ldexpf(value, scale)) ||
      !equal_float(scalbnf(value, scale), nelibc_scalbnf(value, scale)) ||
      !equal_float(scalbf(value, (float)scale), nelibc_scalbf(value, (float)scale))) {
    fprintf(stderr, "ldexpf/scalbnf mismatch input=%08x exponent=%d\n", input_bits, scale);
    return 1;
  }
  const long wide_scale = (long)(input_bits % 2049) - 1024;
  if (!equal_float(scalblnf(value, wide_scale), nelibc_scalblnf(value, wide_scale))) {
    fprintf(stderr, "scalblnf mismatch input=%08x exponent=%ld\n", input_bits, wide_scale);
    return 1;
  }
  if (ilogbf(value) != nelibc_ilogbf(value) || !equal_float(logbf(value), nelibc_logbf(value))) {
    fprintf(stderr, "ilogbf/logbf mismatch input=%08x\n", input_bits);
    return 1;
  }
  if (isfinite(value) && value != 0.0f &&
      !equal_float(significandf(value), nelibc_significandf(value))) {
    fprintf(stderr, "significandf mismatch input=%08x\n", input_bits);
    return 1;
  }
  return 0;
}

static int check_math_extensions(void) {
  const double values[] = {0.0, -0.0, 0x1p-1074, DBL_MIN, DBL_MAX, INFINITY, -INFINITY, NAN};
  const float valuesf[] = {0.0f, -0.0f, 0x1p-149f, FLT_MIN, FLT_MAX, INFINITY, -INFINITY, NAN};
  errno = 0;
  if (!isnan(nelibc_scalb(1.0, 0.5)) || errno != EDOM) {
    fprintf(stderr, "scalb fractional exponent did not report a domain error\n");
    return 1;
  }
  errno = 0;
  if (!isnan(nelibc_scalbf(1.0f, 0.5f)) || errno != EDOM) {
    fprintf(stderr, "scalbf fractional exponent did not report a domain error\n");
    return 1;
  }
  errno = 0;
  if (!isinf(nelibc_scalb(DBL_MAX, 1.0)) || errno != ERANGE) {
    fprintf(stderr, "scalb overflow did not report a range error\n");
    return 1;
  }
  errno = 0;
  if (nelibc_scalb(0x1p-1074, -1.0) != 0.0 || errno != ERANGE) {
    fprintf(stderr, "scalb underflow did not report a range error\n");
    return 1;
  }
  const double scales[] = {-INFINITY, -2048.0, -1.5, -1.0, 0.0, 1.0, 1024.0, INFINITY, NAN};
  for (size_t i = 0; i < sizeof values / sizeof values[0]; ++i) {
    for (size_t j = 0; j < sizeof scales / sizeof scales[0]; ++j) {
      if (!equal_double(scalb(values[i], scales[j]), nelibc_scalb(values[i], scales[j])) ||
          !equal_float(scalbf(valuesf[i], (float)scales[j]),
                       nelibc_scalbf(valuesf[i], (float)scales[j]))) {
        fprintf(stderr, "scalb special-value mismatch x-index=%zu n-index=%zu\n", i, j);
        return 1;
      }
    }
  }
  for (size_t i = 0; i < sizeof values / sizeof values[0]; ++i) {
    if (!!finite(values[i]) != !!nelibc_finite(values[i]) ||
        !!finitef(valuesf[i]) != !!nelibc_finitef(valuesf[i])) {
      fprintf(stderr, "finite classification mismatch index=%zu\n", i);
      return 1;
    }
    const double expected = exp10(values[i]);
    const double actual = nelibc_pow10(values[i]);
    if (isnan(expected) != isnan(actual) ||
        (!isnan(expected) && ulps_double(expected, actual) > 1)) {
      fprintf(stderr, "pow10 mismatch x=%.17g expected=%.17g actual=%.17g\n",
              values[i], expected, actual);
      return 1;
    }
    const float expectedf = exp10f(valuesf[i]);
    const float actualf = nelibc_pow10f(valuesf[i]);
    if (isnan(expectedf) != isnan(actualf) ||
        (!isnan(expectedf) && ulps_float(expectedf, actualf) > 1)) {
      fprintf(stderr, "pow10f mismatch x=%.9g expected=%.9g actual=%.9g\n",
              valuesf[i], expectedf, actualf);
      return 1;
    }
  }
  if (!isnan(nan("nelibc")) || !isnan(nelibc_nan("nelibc")) ||
      !isnan(nanf("nelibc")) || !isnan(nelibc_nanf("nelibc"))) {
    fprintf(stderr, "nan constructors did not return NaN\n");
    return 1;
  }
  static const double x[] = {5.0, -5.0, 7.0, -7.0, 1e100, -1e-100};
  static const double y[] = {2.0, 2.0, 2.0, -2.0, 0.125, 3.0};
  for (size_t i = 0; i < sizeof x / sizeof x[0]; ++i) {
    if (!equal_double(drem(x[i], y[i]), nelibc_drem(x[i], y[i])) ||
        !equal_float(dremf((float)x[i], (float)y[i]),
                     nelibc_dremf((float)x[i], (float)y[i]))) {
      fprintf(stderr, "drem alias mismatch index=%zu\n", i);
      return 1;
    }
  }
  return 0;
}

static int check_pair_double(uint64_t left_bits, uint64_t right_bits) {
  double_bits left = {.bits = left_bits};
  double_bits right = {.bits = right_bits};
  const double x = left.value;
  const double y = right.value;
  const double expected[] = {fmod(x, y), remainder(x, y), fmin(x, y), fmax(x, y), fdim(x, y), hypot(x, y)};
  const double actual[] = {nelibc_fmod(x, y), nelibc_remainder(x, y),
                           nelibc_fmin(x, y), nelibc_fmax(x, y), nelibc_fdim(x, y), nelibc_hypot(x, y)};
  const int signaling_nan =
      ((left_bits & UINT64_C(0x7FF0000000000000)) == UINT64_C(0x7FF0000000000000) &&
       (left_bits & UINT64_C(0x000FFFFFFFFFFFFF)) != 0 &&
       (left_bits & UINT64_C(0x0008000000000000)) == 0) ||
      ((right_bits & UINT64_C(0x7FF0000000000000)) == UINT64_C(0x7FF0000000000000) &&
       (right_bits & UINT64_C(0x000FFFFFFFFFFFFF)) != 0 &&
       (right_bits & UINT64_C(0x0008000000000000)) == 0);
  for (size_t i = 0; i < sizeof expected / sizeof expected[0]; ++i) {
    if (i >= 2 && i <= 3 && signaling_nan) continue;
    if (i == 5) {
      if (isnan(expected[i]) != isnan(actual[i]) ||
          (!isnan(expected[i]) && ulps_double(expected[i], actual[i]) > 4)) {
        fprintf(stderr, "hypot binary64 mismatch x=%016llx y=%016llx expected=%.17g actual=%.17g\n",
                (unsigned long long)left_bits, (unsigned long long)right_bits,
                expected[i], actual[i]);
        return 1;
      }
      continue;
    }
    const int equal_zero_sign_choice = i >= 2 && i <= 3 && expected[i] == 0.0 && actual[i] == 0.0;
    if (i == 4 && isnan(expected[i]) && isnan(actual[i])) continue;
    if (!equal_zero_sign_choice && !equal_double(expected[i], actual[i])) {
      fprintf(stderr, "binary64 pair mismatch x=%016llx y=%016llx operation=%zu\n",
              (unsigned long long)left_bits, (unsigned long long)right_bits, i);
      double_bits e = {.value = expected[i]};
      double_bits a = {.value = actual[i]};
      fprintf(stderr, "expected=%016llx actual=%016llx\n",
              (unsigned long long)e.bits, (unsigned long long)a.bits);
      return 1;
    }
  }
  if (isfinite(x) && !isnan(y) && y != 0.0) {
    int expected_quotient = 0, actual_quotient = 0;
    const double expected_remainder = remquo(x, y, &expected_quotient);
    const double actual_remainder = nelibc_remquo(x, y, &actual_quotient);
    if (!equal_double(expected_remainder, actual_remainder) ||
        expected_quotient % 8 != actual_quotient % 8) {
      fprintf(stderr, "remquo mismatch x=%.17g y=%.17g remainder=%.17g/%.17g quotient=%d/%d\n",
              x, y, expected_remainder, actual_remainder,
              expected_quotient, actual_quotient);
      return 1;
    }
  }
  if (!equal_double(nextafter(x, y), nelibc_nextafter(x, y))) {
    fprintf(stderr, "nextafter mismatch x=%016llx y=%016llx\n",
            (unsigned long long)left_bits, (unsigned long long)right_bits);
    return 1;
  }
  const double expected_atan2 = atan2(x, y);
  const double actual_atan2 = nelibc_atan2(x, y);
  if (isnan(expected_atan2) != isnan(actual_atan2) ||
      (!isnan(expected_atan2) && ulps_double(expected_atan2, actual_atan2) > 4)) {
    fprintf(stderr, "atan2 binary64 mismatch y=%.17g x=%.17g expected=%.17g actual=%.17g\n",
            x, y, expected_atan2, actual_atan2);
    return 1;
  }
  const double expected_pow = pow(x, y);
  const double actual_pow = nelibc_pow(x, y);
  if (!isnan(expected_pow) && ulps_double(expected_pow, actual_pow) > max_pow_ulps)
    max_pow_ulps = ulps_double(expected_pow, actual_pow);
  if (isnan(expected_pow) != isnan(actual_pow) ||
      (!isnan(expected_pow) && ulps_double(expected_pow, actual_pow) > 16)) {
    fprintf(stderr, "pow binary64 mismatch x=%.17g y=%.17g expected=%.17g actual=%.17g ulps=%llu\n",
            x, y, expected_pow, actual_pow,
            (unsigned long long)ulps_double(expected_pow, actual_pow));
    return 1;
  }
  return 0;
}

static int check_pair_float(uint32_t left_bits, uint32_t right_bits) {
  float_bits left = {.bits = left_bits};
  float_bits right = {.bits = right_bits};
  const float x = left.value;
  const float y = right.value;
  const float expected[] = {fmodf(x, y), remainderf(x, y), fminf(x, y), fmaxf(x, y), fdimf(x, y), hypotf(x, y)};
  const float actual[] = {nelibc_fmodf(x, y), nelibc_remainderf(x, y),
                          nelibc_fminf(x, y), nelibc_fmaxf(x, y), nelibc_fdimf(x, y), nelibc_hypotf(x, y)};
  const int signaling_nan =
      ((left_bits & UINT32_C(0x7F800000)) == UINT32_C(0x7F800000) &&
       (left_bits & UINT32_C(0x007FFFFF)) != 0 &&
       (left_bits & UINT32_C(0x00400000)) == 0) ||
      ((right_bits & UINT32_C(0x7F800000)) == UINT32_C(0x7F800000) &&
       (right_bits & UINT32_C(0x007FFFFF)) != 0 &&
       (right_bits & UINT32_C(0x00400000)) == 0);
  for (size_t i = 0; i < sizeof expected / sizeof expected[0]; ++i) {
    if (i >= 2 && i <= 3 && signaling_nan) continue;
    if (i == 5) {
      if (isnan(expected[i]) != isnan(actual[i]) ||
          (!isnan(expected[i]) && ulps_float(expected[i], actual[i]) > 4)) {
        fprintf(stderr, "hypot binary32 mismatch x=%08x y=%08x expected=%.9g actual=%.9g\n",
                left_bits, right_bits, expected[i], actual[i]);
        return 1;
      }
      continue;
    }
    const int equal_zero_sign_choice = i >= 2 && i <= 3 && expected[i] == 0.0f && actual[i] == 0.0f;
    if (i == 4 && isnan(expected[i]) && isnan(actual[i])) continue;
    if (!equal_zero_sign_choice && !equal_float(expected[i], actual[i])) {
      fprintf(stderr, "binary32 pair mismatch x=%08x y=%08x operation=%zu\n",
              left_bits, right_bits, i);
      float_bits e = {.value = expected[i]};
      float_bits a = {.value = actual[i]};
      fprintf(stderr, "expected=%08x actual=%08x\n", e.bits, a.bits);
      return 1;
    }
  }
  if (isfinite(x) && !isnan(y) && y != 0.0f) {
    int expected_quotient = 0, actual_quotient = 0;
    const float expected_remainder = remquof(x, y, &expected_quotient);
    const float actual_remainder = nelibc_remquof(x, y, &actual_quotient);
    if (!equal_float(expected_remainder, actual_remainder) ||
        expected_quotient % 8 != actual_quotient % 8) {
      fprintf(stderr, "remquof mismatch x=%.9g y=%.9g remainder=%.9g/%.9g quotient=%d/%d\n",
              x, y, expected_remainder, actual_remainder,
              expected_quotient, actual_quotient);
      return 1;
    }
  }
  if (!equal_float(nextafterf(x, y), nelibc_nextafterf(x, y))) {
    fprintf(stderr, "nextafterf mismatch x=%08x y=%08x\n", left_bits, right_bits);
    return 1;
  }
  const float expected_atan2 = atan2f(x, y);
  const float actual_atan2 = nelibc_atan2f(x, y);
  if (isnan(expected_atan2) != isnan(actual_atan2) ||
      (!isnan(expected_atan2) && ulps_float(expected_atan2, actual_atan2) > 4)) {
    fprintf(stderr, "atan2f binary32 mismatch y=%.9g x=%.9g expected=%.9g actual=%.9g\n",
            x, y, expected_atan2, actual_atan2);
    return 1;
  }
  const float expected_pow = powf(x, y);
  const float actual_pow = nelibc_powf(x, y);
  if (!isnan(expected_pow) && ulps_float(expected_pow, actual_pow) > max_powf_ulps)
    max_powf_ulps = ulps_float(expected_pow, actual_pow);
  if (isnan(expected_pow) != isnan(actual_pow) ||
      (!isnan(expected_pow) && ulps_float(expected_pow, actual_pow) > 16)) {
    fprintf(stderr, "powf binary32 mismatch x=%.9g y=%.9g expected=%.9g actual=%.9g ulps=%u\n",
            x, y, expected_pow, actual_pow, ulps_float(expected_pow, actual_pow));
    return 1;
  }
  return 0;
}

static int check_unary_errno(const char *name, double (*reference)(double),
                             double (*replacement)(double), double value) {
  errno = 0;
  volatile double ignored_reference = reference(value);
  errno = 0;
  volatile double ignored_replacement = replacement(value);
  const int actual_errno = errno;
  (void)ignored_reference;
  (void)ignored_replacement;
  if ((math_errhandling & MATH_ERRNO) && actual_errno != EDOM) {
    fprintf(stderr, "%s errno mismatch expected=%d actual=%d\n", name, EDOM, actual_errno);
    return 1;
  }
  return 0;
}

static int check_errno_value(const char *name, double (*reference)(double),
                             double (*replacement)(double), double value,
                             int expected_errno) {
  errno = 0;
  volatile double ignored_reference = reference(value);
  errno = 0;
  volatile double ignored_replacement = replacement(value);
  const int actual_errno = errno;
  (void)ignored_reference;
  (void)ignored_replacement;
  if ((math_errhandling & MATH_ERRNO) && actual_errno != expected_errno) {
    fprintf(stderr, "%s errno mismatch expected=%d actual=%d\n",
            name, expected_errno, actual_errno);
    return 1;
  }
  return 0;
}

static int check_unary_flags(const char *name, double (*reference)(double),
                             double (*replacement)(double), double value) {
  feclearexcept(FE_ALL_EXCEPT);
  volatile double ignored_reference = reference(value);
  const int expected_flags = fetestexcept(FE_ALL_EXCEPT);
  feclearexcept(FE_ALL_EXCEPT);
  volatile double ignored_replacement = replacement(value);
  const int actual_flags = fetestexcept(FE_ALL_EXCEPT);
  (void)ignored_reference;
  (void)ignored_replacement;
  /* FE_DENORMAL (x86 status bit 1) is a nonstandard extension whose reporting
     differs between the supported libcs. */
  const int comparable_flags = FE_ALL_EXCEPT & ~0x02;
  if ((expected_flags & comparable_flags) != (actual_flags & comparable_flags)) {
    fprintf(stderr, "%s exception flags mismatch value=%.17g expected=%x actual=%x\n",
            name, value, expected_flags, actual_flags);
    return 1;
  }
  return 0;
}

static int check_float_errno_value(const char *name, float (*reference)(float),
                                   float (*replacement)(float), float value,
                                   int expected_errno) {
  errno = 0;
  volatile float ignored_reference = reference(value);
  errno = 0;
  volatile float ignored_replacement = replacement(value);
  const int actual_errno = errno;
  (void)ignored_reference;
  (void)ignored_replacement;
  if ((math_errhandling & MATH_ERRNO) && actual_errno != expected_errno) {
    fprintf(stderr, "%s errno mismatch expected=%d actual=%d\n",
            name, expected_errno, actual_errno);
    return 1;
  }
  return 0;
}

static int check_binary_float_errno_value(const char *name,
                                          float (*reference)(float, float),
                                          float (*replacement)(float, float),
                                          float x, float y, int expected_errno) {
  errno = 0;
  volatile float ignored_reference = reference(x, y);
  errno = 0;
  volatile float ignored_replacement = replacement(x, y);
  const int actual_errno = errno;
  (void)ignored_reference;
  (void)ignored_replacement;
  if ((math_errhandling & MATH_ERRNO) && actual_errno != expected_errno) {
    fprintf(stderr, "%s errno mismatch expected=%d actual=%d\n",
            name, expected_errno, actual_errno);
    return 1;
  }
  return 0;
}

static int check_float_flags(const char *name, float (*reference)(float),
                             float (*replacement)(float), float value) {
  feclearexcept(FE_ALL_EXCEPT);
  volatile float ignored_reference = reference(value);
  const int expected_flags = fetestexcept(FE_ALL_EXCEPT);
  feclearexcept(FE_ALL_EXCEPT);
  volatile float ignored_replacement = replacement(value);
  const int actual_flags = fetestexcept(FE_ALL_EXCEPT);
  (void)ignored_reference;
  (void)ignored_replacement;
  const int comparable_flags = FE_ALL_EXCEPT & ~0x02;
  if ((expected_flags & comparable_flags) != (actual_flags & comparable_flags)) {
    fprintf(stderr, "%s exception flags mismatch value=%.9g expected=%x actual=%x\n",
            name, value, expected_flags, actual_flags);
    return 1;
  }
  return 0;
}

static int check_binary_errno(const char *name,
                             double (*reference)(double, double),
                             double (*replacement)(double, double),
                             double x, double y) {
  errno = 0;
  volatile double ignored_reference = reference(x, y);
  errno = 0;
  volatile double ignored_replacement = replacement(x, y);
  const int actual_errno = errno;
  (void)ignored_reference;
  (void)ignored_replacement;
  if ((math_errhandling & MATH_ERRNO) && actual_errno != EDOM) {
    fprintf(stderr, "%s errno mismatch expected=%d actual=%d\n", name, EDOM, actual_errno);
    return 1;
  }
  return 0;
}

static int check_binary_errno_value(const char *name,
                                    double (*reference)(double, double),
                                    double (*replacement)(double, double),
                                    double x, double y, int expected_errno) {
  errno = 0;
  volatile double ignored_reference = reference(x, y);
  errno = 0;
  volatile double ignored_replacement = replacement(x, y);
  const int actual_errno = errno;
  (void)ignored_reference;
  (void)ignored_replacement;
  if ((math_errhandling & MATH_ERRNO) && actual_errno != expected_errno) {
    fprintf(stderr, "%s errno mismatch expected=%d actual=%d\n",
            name, expected_errno, actual_errno);
    return 1;
  }
  return 0;
}

static int check_rint_mode(int mode, double value, float valuef) {
  const int double_flag_mask = isnan(value) ? (FE_ALL_EXCEPT & ~FE_INVALID) : FE_ALL_EXCEPT;
  const int float_flag_mask = isnan(valuef) ? (FE_ALL_EXCEPT & ~FE_INVALID) : FE_ALL_EXCEPT;
  if (fesetround(mode) != 0) {
    fprintf(stderr, "fesetround failed for mode=%d\n", mode);
    return 1;
  }
  feclearexcept(FE_ALL_EXCEPT);
  const double expected = rint(value);
  const int expected_flags = fetestexcept(FE_ALL_EXCEPT);
  feclearexcept(FE_ALL_EXCEPT);
  const double actual = nelibc_rint(value);
  const int actual_flags = fetestexcept(FE_ALL_EXCEPT);
  if (!equal_double(expected, actual) ||
      (expected_flags & double_flag_mask) != (actual_flags & double_flag_mask)) {
    fprintf(stderr, "rint mismatch mode=%d value=%.17g expected=%.17g flags=%x actual=%.17g flags=%x\n",
            mode, value, expected, expected_flags, actual, actual_flags);
    return 1;
  }

  feclearexcept(FE_ALL_EXCEPT);
  const float expectedf = rintf(valuef);
  const int expectedf_flags = fetestexcept(FE_ALL_EXCEPT);
  feclearexcept(FE_ALL_EXCEPT);
  const float actualf = nelibc_rintf(valuef);
  const int actualf_flags = fetestexcept(FE_ALL_EXCEPT);
  if (!equal_float(expectedf, actualf) ||
      (expectedf_flags & float_flag_mask) != (actualf_flags & float_flag_mask)) {
    fprintf(stderr, "rintf mismatch mode=%d value=%.9g expected=%.9g flags=%x actual=%.9g flags=%x\n",
            mode, valuef, expectedf, expectedf_flags, actualf, actualf_flags);
    return 1;
  }

  feclearexcept(FE_ALL_EXCEPT);
  feraiseexcept(FE_DIVBYZERO);
  const double nearby_expected = nearbyint(value);
  const int nearby_expected_flags = fetestexcept(FE_ALL_EXCEPT);
  feclearexcept(FE_ALL_EXCEPT);
  feraiseexcept(FE_DIVBYZERO);
  const double nearby_actual = nelibc_nearbyint(value);
  const int nearby_actual_flags = fetestexcept(FE_ALL_EXCEPT);
  if (!equal_double(nearby_expected, nearby_actual) ||
      (nearby_expected_flags & double_flag_mask) != (nearby_actual_flags & double_flag_mask)) {
    fprintf(stderr, "nearbyint mismatch mode=%d value=%.17g expected flags=%x actual flags=%x\n",
            mode, value, nearby_expected_flags, nearby_actual_flags);
    return 1;
  }

  feclearexcept(FE_ALL_EXCEPT);
  feraiseexcept(FE_DIVBYZERO);
  const float nearby_expectedf = nearbyintf(valuef);
  const int nearby_expectedf_flags = fetestexcept(FE_ALL_EXCEPT);
  feclearexcept(FE_ALL_EXCEPT);
  feraiseexcept(FE_DIVBYZERO);
  const float nearby_actualf = nelibc_nearbyintf(valuef);
  const int nearby_actualf_flags = fetestexcept(FE_ALL_EXCEPT);
  if (!equal_float(nearby_expectedf, nearby_actualf) ||
      (nearby_expectedf_flags & float_flag_mask) != (nearby_actualf_flags & float_flag_mask)) {
    fprintf(stderr, "nearbyintf mismatch mode=%d value=%.9g expected flags=%x actual flags=%x\n",
            mode, valuef, nearby_expectedf_flags, nearby_actualf_flags);
    return 1;
  }

  if (isfinite(value) && fabs(value) < 0x1p63 &&
      isfinite(valuef) && fabsf(valuef) < 0x1p63f) {
    feclearexcept(FE_ALL_EXCEPT);
    const long lrint_expected = lrint(value);
    const int lrint_expected_flags = fetestexcept(FE_ALL_EXCEPT);
    feclearexcept(FE_ALL_EXCEPT);
    const long lrint_actual = nelibc_lrint(value);
    const int lrint_actual_flags = fetestexcept(FE_ALL_EXCEPT);
    feclearexcept(FE_ALL_EXCEPT);
    const long lrintf_expected = lrintf(valuef);
    const int lrintf_expected_flags = fetestexcept(FE_ALL_EXCEPT);
    feclearexcept(FE_ALL_EXCEPT);
    const long lrintf_actual = nelibc_lrintf(valuef);
    const int lrintf_actual_flags = fetestexcept(FE_ALL_EXCEPT);
    if (lrint_expected != lrint_actual || lrintf_expected != lrintf_actual ||
        (lrint_expected_flags & (FE_INVALID | FE_INEXACT)) !=
            (lrint_actual_flags & (FE_INVALID | FE_INEXACT)) ||
        (lrintf_expected_flags & (FE_INVALID | FE_INEXACT)) !=
            (lrintf_actual_flags & (FE_INVALID | FE_INEXACT)) ||
        llrint(value) != nelibc_llrint(value) || llrintf(valuef) != nelibc_llrintf(valuef) ||
        lround(value) != nelibc_lround(value) || lroundf(valuef) != nelibc_lroundf(valuef) ||
        llround(value) != nelibc_llround(value) || llroundf(valuef) != nelibc_llroundf(valuef)) {
      fprintf(stderr, "integer rounding mismatch mode=%d x=%.17g xf=%.9g lrint=%ld/%ld flags=%x/%x lrintf=%ld/%ld flags=%x/%x llrint=%lld/%lld llrintf=%lld/%lld lround=%ld/%ld lroundf=%ld/%ld llround=%lld/%lld llroundf=%lld/%lld\n",
              mode, value, valuef, lrint_expected, lrint_actual,
              lrint_expected_flags, lrint_actual_flags, lrintf_expected,
              lrintf_actual, lrintf_expected_flags, lrintf_actual_flags,
              llrint(value), nelibc_llrint(value), llrintf(valuef),
              nelibc_llrintf(valuef), lround(value), nelibc_lround(value),
              lroundf(valuef), nelibc_lroundf(valuef), llround(value),
              nelibc_llround(value), llroundf(valuef), nelibc_llroundf(valuef));
      return 1;
    }
    feclearexcept(FE_ALL_EXCEPT);
    (void)lround(value);
    feclearexcept(FE_ALL_EXCEPT);
    (void)nelibc_lround(value);
    if (fetestexcept(FE_INEXACT)) {
      fprintf(stderr, "lround raised FE_INEXACT x=%.17g\n", value);
      return 1;
    }
    feclearexcept(FE_ALL_EXCEPT);
    (void)nelibc_lroundf(valuef);
    if (fetestexcept(FE_INEXACT)) {
      fprintf(stderr, "lroundf raised FE_INEXACT x=%.9g\n", valuef);
      return 1;
    }
    feclearexcept(FE_ALL_EXCEPT);
    (void)nelibc_llround(value);
    if (fetestexcept(FE_INEXACT)) {
      fprintf(stderr, "llround raised FE_INEXACT x=%.17g\n", value);
      return 1;
    }
    feclearexcept(FE_ALL_EXCEPT);
    (void)llroundf(valuef);
    feclearexcept(FE_ALL_EXCEPT);
    (void)nelibc_llroundf(valuef);
    if (fetestexcept(FE_INEXACT)) {
      fprintf(stderr, "llroundf raised FE_INEXACT x=%.9g\n", valuef);
      return 1;
    }
  }

  feclearexcept(FE_ALL_EXCEPT);
  feraiseexcept(FE_INEXACT | FE_DIVBYZERO);
  const double exact_nearby_expected = nearbyint(2.0);
  const int exact_nearby_expected_flags = fetestexcept(FE_ALL_EXCEPT);
  feclearexcept(FE_ALL_EXCEPT);
  feraiseexcept(FE_INEXACT | FE_DIVBYZERO);
  const double exact_nearby_actual = nelibc_nearbyint(2.0);
  const int exact_nearby_actual_flags = fetestexcept(FE_ALL_EXCEPT);
  if (!equal_double(exact_nearby_expected, exact_nearby_actual) ||
      exact_nearby_expected_flags != exact_nearby_actual_flags) {
    fprintf(stderr, "nearbyint did not preserve preexisting exception flags\n");
    return 1;
  }
  return 0;
}

static int check_integer_rounding_range(void) {
  if (lrint(-0x1p63) != LONG_MIN || llrint(-0x1p63) != LLONG_MIN ||
      lrintf(-0x1p63f) != LONG_MIN || llrintf(-0x1p63f) != LLONG_MIN ||
      lround(-0x1p63) != LONG_MIN || llround(-0x1p63) != LLONG_MIN ||
      lroundf(-0x1p63f) != LONG_MIN || llroundf(-0x1p63f) != LLONG_MIN ||
      nelibc_lrint(-0x1p63) != LONG_MIN || nelibc_llrint(-0x1p63) != LLONG_MIN ||
      nelibc_lrintf(-0x1p63f) != LONG_MIN || nelibc_llrintf(-0x1p63f) != LLONG_MIN ||
      nelibc_lround(-0x1p63) != LONG_MIN || nelibc_llround(-0x1p63) != LLONG_MIN ||
      nelibc_lroundf(-0x1p63f) != LONG_MIN || nelibc_llroundf(-0x1p63f) != LLONG_MIN)
    return 1;
#define CHECK_INVALID_INTEGER_CONVERSION(expression) do { \
    feclearexcept(FE_ALL_EXCEPT); \
    volatile long long result = (long long)(expression); \
    (void)result; \
    if (!(fetestexcept(FE_INVALID))) return 1; \
  } while (0)
  CHECK_INVALID_INTEGER_CONVERSION(lrint(0x1p63));
  CHECK_INVALID_INTEGER_CONVERSION(lrintf(0x1p63f));
  CHECK_INVALID_INTEGER_CONVERSION(llrint(0x1p63));
  CHECK_INVALID_INTEGER_CONVERSION(llrintf(0x1p63f));
  CHECK_INVALID_INTEGER_CONVERSION(lround(0x1p63));
  CHECK_INVALID_INTEGER_CONVERSION(lroundf(0x1p63f));
  CHECK_INVALID_INTEGER_CONVERSION(llround(0x1p63));
  CHECK_INVALID_INTEGER_CONVERSION(llroundf(0x1p63f));
  CHECK_INVALID_INTEGER_CONVERSION(nelibc_lrint(0x1p63));
  CHECK_INVALID_INTEGER_CONVERSION(nelibc_lrintf(0x1p63f));
  CHECK_INVALID_INTEGER_CONVERSION(nelibc_llrint(0x1p63));
  CHECK_INVALID_INTEGER_CONVERSION(nelibc_llrintf(0x1p63f));
  CHECK_INVALID_INTEGER_CONVERSION(nelibc_lround(0x1p63));
  CHECK_INVALID_INTEGER_CONVERSION(nelibc_lroundf(0x1p63f));
  CHECK_INVALID_INTEGER_CONVERSION(nelibc_llround(0x1p63));
  CHECK_INVALID_INTEGER_CONVERSION(nelibc_llroundf(0x1p63f));
#undef CHECK_INVALID_INTEGER_CONVERSION
  feclearexcept(FE_ALL_EXCEPT);
  return 0;
}

static int check_bessel(double x, float xf) {
  const double ax = fabs(x), ay = fabs((double)xf);
  const double j0e = j0(x), j1e = j1(x), y0e = y0(ax), y1e = y1(ax);
  const float j0ef = j0f(xf), j1ef = j1f(xf), y0ef = y0f((float)ay), y1ef = y1f((float)ay);
  const double j0a = nelibc_j0(x), j1a = nelibc_j1(x), y0a = nelibc_y0(ax), y1a = nelibc_y1(ax);
  const float j0af = nelibc_j0f(xf), j1af = nelibc_j1f(xf), y0af = nelibc_y0f((float)ay), y1af = nelibc_y1f((float)ay);
  const double tolerance = 2e-9;
  if ((!isnan(j0e) && fabs(j0a-j0e) > tolerance*(1+fabs(j0e))) ||
      (!isnan(j1e) && fabs(j1a-j1e) > tolerance*(1+fabs(j1e))) ||
      (!isnan(y0e) && fabs(y0a-y0e) > tolerance*(1+fabs(y0e))) ||
      (!isnan(y1e) && fabs(y1a-y1e) > tolerance*(1+fabs(y1e))) ||
      (!isnan(j0ef) && fabsf(j0af-j0ef) > 2e-6f*(1+fabsf(j0ef))) ||
      (!isnan(j1ef) && fabsf(j1af-j1ef) > 2e-6f*(1+fabsf(j1ef))) ||
      (!isnan(y0ef) && fabsf(y0af-y0ef) > 2e-6f*(1+fabsf(y0ef))) ||
      (!isnan(y1ef) && fabsf(y1af-y1ef) > 2e-6f*(1+fabsf(y1ef)))) {
    fprintf(stderr, "Bessel mismatch x=%.17g xf=%.9g J0 %.17g/%.17g J1 %.17g/%.17g Y0 %.17g/%.17g Y1 %.17g/%.17g\n",
      x, xf, j0e,j0a,j1e,j1a,y0e,y0a,y1e,y1a);
    return 1;
  }
  return 0;
}

static int check_bessel_order(int n, double x, float xf) {
  const double je = jn(n, x), ye = yn(n, fabs(x));
  const float jef = jnf(n, xf), yef = ynf(n, fabsf(xf));
  const double ja = nelibc_jn(n, x), ya = nelibc_yn(n, fabs(x));
  const float jaf = nelibc_jnf(n, xf), yaf = nelibc_ynf(n, fabsf(xf));
  if ((!isnan(je) && !isinf(je) && fabs(ja-je) > 3e-8*(1+fabs(je))) ||
      (isinf(je) && ja != je) ||
      (!isnan(ye) && !isinf(ye) && fabs(ya-ye) > 3e-8*(1+fabs(ye))) ||
      (isinf(ye) && ya != ye) ||
      (!isnan(jef) && !isinf(jef) && fabsf(jaf-jef) > 3e-5f*(1+fabsf(jef))) ||
      (isinf(jef) && jaf != jef) ||
      (!isnan(yef) && !isinf(yef) && fabsf(yaf-yef) > 3e-5f*(1+fabsf(yef))) ||
      (isinf(yef) && yaf != yef)) {
    fprintf(stderr, "jn/yn mismatch n=%d x=%.17g xf=%.9g J %.17g/%.17g Y %.17g/%.17g\n", n,x,xf,je,ja,ye,ya);
    return 1;
  }
  return 0;
}

static int check_gamma(double x, float xf) {
  const double ge = tgamma(x), le = lgamma(x);
  const float gef = tgammaf(xf), lef = lgammaf(xf);
  const double ga = nelibc_tgamma(x), la = nelibc_lgamma(x);
  const int lgamma_sign = signgam;
  const float gaf = nelibc_tgammaf(xf), laf = nelibc_lgammaf(xf);
  const int lgammaf_sign = signgam;
  int sign = 0, signf = 0;
  const double lar = nelibc_lgamma_r(x, &sign);
  const float lafr = nelibc_lgammaf_r(xf, &signf);
  uint64_t ug = ulps_double(ge, ga), ul = ulps_double(le, la), ur = ulps_double(le, lar);
  uint32_t ugf = ulps_float(gef, gaf), ulf = ulps_float(lef, laf), urf = ulps_float(lef, lafr);
  if ((!isnan(ge) && fabs(ga - ge) > 2e-13 * (1.0 + fabs(ge))) || (!isnan(le) && (fabs(la - le) > 2e-12 * (1.0 + fabs(le)) || fabs(lar - le) > 2e-12 * (1.0 + fabs(le)))) ||
      (!isnan(gef) && fabsf(gaf - gef) > 2e-5f * (1.0f + fabsf(gef))) || (!isnan(lef) && (fabsf(laf - lef) > 2e-5f * (1.0f + fabsf(lef)) || fabsf(lafr - lef) > 2e-5f * (1.0f + fabsf(lef))))) {
    fprintf(stderr, "gamma mismatch x=%.17g xf=%.9g values gamma %.17g/%.17g lgamma %.17g/%.17g: tgamma=%llu/%u lgamma=%llu/%u\n", x, xf, ge, ga, le, la,
      (unsigned long long)ug, ugf, (unsigned long long)ul, ulf);
    return 1;
  }
  if (!isnan(le) && ((x < 0 && sign != (sin(M_PI * x) > 0 ? 1 : -1)) || (x > 0 && sign != 1))) {
    fprintf(stderr, "lgamma_r sign mismatch x=%.17g sign=%d\n", x, sign);
    return 1;
  }
  if (!isnan(le) && x != 0.0 && lgamma_sign != sign) {
    fprintf(stderr, "lgamma signgam mismatch x=%.17g: %d/%d\n", x, lgamma_sign, sign);
    return 1;
  }
  if (!isnan(lef) && xf != 0.0f && lgammaf_sign != signf) {
    fprintf(stderr, "lgammaf signgam mismatch x=%.9g: %d/%d\n", xf, lgammaf_sign, signf);
    return 1;
  }
  return 0;
}

static int check_erf(double x, float xf) {
  const double erf_expected = erf(x), erfc_expected = erfc(x);
  const float erff_expected = erff(xf), erfcf_expected = erfcf(xf);
  const double erf_actual = nelibc_erf(x), erfc_actual = nelibc_erfc(x);
  const float erff_actual = nelibc_erff(xf), erfcf_actual = nelibc_erfcf(xf);
  uint64_t de = ulps_double(erf_expected, erf_actual), dc = ulps_double(erfc_expected, erfc_actual);
  uint32_t fe = ulps_float(erff_expected, erff_actual), fc = ulps_float(erfcf_expected, erfcf_actual);
  if ((!isnan(erf_expected) && de > 4) || (!isnan(erfc_expected) && dc > 4) ||
      (!isnan(erff_expected) && fe > 4) || (!isnan(erfcf_expected) && fc > 4)) {
    fprintf(stderr, "erf family mismatch x=%.17g xf=%.9g: %llu/%llu %u/%u ULP\n", x, xf,
      (unsigned long long)de, (unsigned long long)dc, fe, fc);
    return 1;
  }
  if (de > max_erf_ulps) max_erf_ulps = de;
  if (dc > max_erfc_ulps) max_erfc_ulps = dc;
  if (fe > max_erff_ulps) max_erff_ulps = fe;
  if (fc > max_erfcf_ulps) max_erfcf_ulps = fc;
  return 0;
}

static int long_double_close(long double actual, long double expected) {
  if (actual == expected) return 1;
  if (isnan(expected)) return isnan(actual);
  if (!isfinite(expected) || !isfinite(actual) || expected == 0.0L) return 0;
  return fabsl(actual - expected) <= 8.0L * LDBL_EPSILON * fabsl(expected);
}

static int long_double_math_close(long double actual, long double expected) {
  if (actual == expected) return 1;
  if (isnan(expected)) return isnan(actual);
  if (!isfinite(expected) || !isfinite(actual) || expected == 0.0L) return 0;
  return fabsl(actual - expected) <= 128.0L * LDBL_EPSILON * fabsl(expected);
}

static int check_long_double_erf(void) {
  static const long double values[] = {
      -INFINITY, -2000.0L, -120.0L, -100.0L, -8.0L, -4.0L, -1.5L,
      -1.0L, -0.125L, -0.0L, 0.0L, 0x1p-16445L, 0.125L, 1.0L,
      1.5L, 4.0L, 8.0L, 100.0L, 120.0L, 2000.0L, INFINITY, NAN};
  for (size_t i = 0; i < sizeof values / sizeof values[0]; ++i) {
    const long double x = values[i];
    const long double expected_erf = erfl(x);
    const long double expected_erfc = erfcl(x);
    const long double actual_erf = nelibc_erfl(x);
    const long double actual_erfc = nelibc_erfcl(x);
    if (!long_double_math_close(actual_erf, expected_erf) ||
        !long_double_math_close(actual_erfc, expected_erfc) ||
        (expected_erf == 0.0L && signbit(actual_erf) != signbit(expected_erf))) {
      fprintf(stderr, "erfl/erfcl mismatch at case %zu: erf %.21Le/%.21Le erfc %.21Le/%.21Le\n",
              i, actual_erf, expected_erf, actual_erfc, expected_erfc);
      return 1;
    }
  }
  for (size_t i = 0; i < 512; ++i) {
    const long double x = (long double)((long long)(i * 17 % 2401) - 1200) / 100.0L;
    if (!long_double_math_close(nelibc_erfl(x), erfl(x)) ||
        !long_double_math_close(nelibc_erfcl(x), erfcl(x))) {
      fprintf(stderr, "erfl/erfcl generated-input mismatch i=%zu x=%.18Le\n", i, x);
      return 1;
    }
  }
  return 0;
}

static int long_double_gamma_close(long double actual, long double expected) {
  if (actual == expected) return 1;
  if (isnan(expected)) return isnan(actual);
  if (!isfinite(expected) || !isfinite(actual) || expected == 0.0L) return 0;
  return fabsl(actual - expected) <= 4096.0L * LDBL_EPSILON * fabsl(expected);
}

static int check_long_double_gamma(void) {
  static const long double values[] = {
      -INFINITY, -20.5L, -10.25L, -5.5L, -3.5L, -1.5L, -0.5L,
      -0x1p-16445L, -0x1p-100L, -0.0L, 0.0L, 0x1p-16445L, 0x1p-100L, 0.5L, 1.0L, 1.5L,
      2.5L, 5.5L, 10.25L, 16.0L, 100.0L, 1000.0L, INFINITY, NAN};
  for (size_t i = 0; i < sizeof values / sizeof values[0]; ++i) {
    const long double x = values[i];
    int expected_sign = 0, actual_sign = 0;
    const long double expected_lgamma = lgammal_r(x, &expected_sign);
    const long double actual_lgamma = nelibc_lgammal_r(x, &actual_sign);
    const long double expected_gamma = tgammal(x);
    const long double actual_gamma = nelibc_tgammal(x);
    if (!long_double_gamma_close(actual_lgamma, expected_lgamma) ||
        (isfinite(expected_lgamma) && actual_sign != expected_sign) ||
        !long_double_gamma_close(actual_gamma, expected_gamma)) {
      fprintf(stderr, "long-double gamma mismatch at case %zu x=%.18Le: lgamma %.21Le/%.21Le sign %d/%d gamma %.21Le/%.21Le\n",
              i, x, actual_lgamma, expected_lgamma, actual_sign, expected_sign,
              actual_gamma, expected_gamma);
      return 1;
    }
  }
  for (size_t i = 0; i < 256; ++i) {
    const long double x = -10.0L + (long double)(i + 1) * 0.08L;
    if (x == truncl(x)) continue;
    if (!long_double_gamma_close(nelibc_lgammal(x), lgammal(x)) ||
        !long_double_gamma_close(nelibc_tgammal(x), tgammal(x))) {
      fprintf(stderr, "long-double gamma generated-input mismatch i=%zu x=%.18Le\n", i, x);
      return 1;
    }
  }
  return 0;
}

static int long_double_hyperbolic_close(long double actual, long double expected) {
  if (actual == expected) return 1;
  if (isnan(expected)) return isnan(actual);
  if (!isfinite(expected) || !isfinite(actual) || expected == 0.0L) return 0;
#ifdef NELIBC_MUSL_PROFILE
  if (fabsl(expected) > 1000.0L) {
    /* musl sinh near binary80 overflow differs from glibc by thousands of ULP. */
    return fabsl(actual - expected) <= 8192.0L * LDBL_EPSILON * fabsl(expected);
  }
#endif
  return fabsl(actual - expected) <= 8.0L * LDBL_EPSILON * fabsl(expected);
}

static int long_double_trig_close(long double actual, long double expected) {
  if (actual == expected) return 1;
  if (!isfinite(actual) || !isfinite(expected) || expected == 0.0L) return 0;
#ifdef NELIBC_MUSL_PROFILE
  /* musl's binary80 trig references differ by tens of ULP for large inputs. */
  const long double tolerance = 64.0L * LDBL_EPSILON;
#else
  const long double tolerance = 8.0L * LDBL_EPSILON;
#endif
  return fabsl(actual - expected) <= tolerance * fabsl(expected);
}

static int long_double_tan_close(long double actual, long double expected) {
  if (actual == expected) return 1;
  if (!isfinite(actual) || !isfinite(expected) || expected == 0.0L) return 0;
#ifdef NELIBC_MUSL_PROFILE
  /* musl's large-input tanl reference differs by more than 64 ULP. */
  const long double tolerance = 128.0L * LDBL_EPSILON;
#else
  const long double tolerance = 8.0L * LDBL_EPSILON;
#endif
  return fabsl(actual - expected) <= tolerance * fabsl(expected);
}

int main(void) {
  if (check_classification()) return 1;
  if (check_long_double_erf()) return 1;
  if (check_long_double_gamma()) return 1;
  const long double hypot_values[] = {
      0.0L, -0.0L, nextafterl(0.0L, 1.0L), LDBL_MIN, 0.5L, 1.0L, 3.0L,
      0x1p+1000L, LDBL_MAX / 4.0L, LDBL_MAX};
  for (size_t i = 0; i < sizeof(hypot_values) / sizeof(hypot_values[0]); ++i) {
    for (size_t j = 0; j < sizeof(hypot_values) / sizeof(hypot_values[0]); ++j) {
      long double expected_hypot = hypotl(hypot_values[i], hypot_values[j]);
      long double actual_hypot = nelibc_hypotl(hypot_values[i], hypot_values[j]);
      if (actual_hypot != expected_hypot) {
        long double error = fabsl(actual_hypot - expected_hypot);
        long double tolerance = LDBL_EPSILON * expected_hypot * 2.0L;
        if (expected_hypot < LDBL_MIN || error > tolerance) {
          fprintf(stderr, "hypotl mismatch values=%zu/%zu\n", i, j);
          return 1;
        }
      }
    }
  }
  union {
    long double value;
    struct { uint64_t significand; uint16_t sign_exponent; } parts;
  } quiet_nan;
  quiet_nan.parts.significand = UINT64_C(0xc000000000000000);
  quiet_nan.parts.sign_exponent = UINT16_C(0x7fff);
  long double infinity_nan_hypot = nelibc_hypotl(INFINITY, quiet_nan.value);
  int hypot_is_infinite = isinf(infinity_nan_hypot);
  if (!hypot_is_infinite) {
    fprintf(stderr, "hypotl infinity/NaN precedence failed\n");
    return 1;
  }
  if (!isnan(nelibc_hypotl(quiet_nan.value, 1.0L))) {
    fprintf(stderr, "hypotl NaN propagation failed\n");
    return 1;
  }
  union {
    long double value;
    struct { uint64_t significand; uint16_t sign_exponent; } parts;
  } signaling_nan;
  signaling_nan.parts.significand = UINT64_C(0x8000000000000001);
  signaling_nan.parts.sign_exponent = UINT16_C(0x7fff);
  long double signaling_hypot = nelibc_hypotl(signaling_nan.value, 1.0L);
  if (!isnan(signaling_hypot)) {
    fprintf(stderr, "hypotl signaling NaN propagation failed\n");
    return 1;
  }
  const long double minmax_values[] = {
      -INFINITY, -LDBL_MAX, -1.0L, -0.0L, 0.0L, nextafterl(0.0L, 1.0L),
      1.0L, LDBL_MAX, INFINITY};
  for (size_t i = 0; i < sizeof(minmax_values) / sizeof(minmax_values[0]); ++i) {
    for (size_t j = 0; j < sizeof(minmax_values) / sizeof(minmax_values[0]); ++j) {
      long double expected_min = fminl(minmax_values[i], minmax_values[j]);
      long double actual_min = nelibc_fminl(minmax_values[i], minmax_values[j]);
      long double expected_max = fmaxl(minmax_values[i], minmax_values[j]);
      long double actual_max = nelibc_fmaxl(minmax_values[i], minmax_values[j]);
      long double expected_dim = fdiml(minmax_values[i], minmax_values[j]);
      long double actual_dim = nelibc_fdiml(minmax_values[i], minmax_values[j]);
      if (!long_double_close(actual_min, expected_min) ||
          !long_double_close(actual_max, expected_max) ||
          !long_double_close(actual_dim, expected_dim) ||
          (expected_dim == 0.0L && signbit(actual_dim))) {
        fprintf(stderr, "fminl/fmaxl/fdiml edge mismatch values=%zu/%zu\n", i, j);
        return 1;
      }
    }
  }
  for (size_t i = 0; i < 4096; ++i) {
    long double x = (long double)((long long)((i * 7919) % 20001) - 10000) / 7.0L;
    long double y = (long double)((long long)((i * 3571) % 20003) - 10001) / 11.0L;
    if (nelibc_fminl(x, y) != fminl(x, y) ||
        nelibc_fmaxl(x, y) != fmaxl(x, y) ||
        nelibc_fdiml(x, y) != fdiml(x, y)) {
      fprintf(stderr, "fminl/fmaxl/fdiml generated-input mismatch i=%zu\n", i);
      return 1;
    }
  }
  if (!signbit(nelibc_fminl(-0.0L, 0.0L)) ||
      !signbit(nelibc_fminl(0.0L, -0.0L)) ||
      signbit(nelibc_fmaxl(-0.0L, 0.0L)) ||
      signbit(nelibc_fmaxl(0.0L, -0.0L))) {
    fprintf(stderr, "long-double min/max signed-zero behavior failed\n");
    return 1;
  }
  if (nelibc_fminl(quiet_nan.value, 2.0L) != 2.0L ||
      nelibc_fminl(2.0L, quiet_nan.value) != 2.0L ||
      nelibc_fmaxl(quiet_nan.value, 2.0L) != 2.0L ||
      nelibc_fmaxl(2.0L, quiet_nan.value) != 2.0L ||
      !isnan(nelibc_fminl(quiet_nan.value, quiet_nan.value)) ||
      !isnan(nelibc_fmaxl(quiet_nan.value, quiet_nan.value)) ||
      !isnan(nelibc_fdiml(quiet_nan.value, 2.0L))) {
    fprintf(stderr, "long-double min/max/dim NaN handling failed\n");
    return 1;
  }
  feclearexcept(FE_ALL_EXCEPT);
  if (!isnan(nelibc_fminl(signaling_nan.value, 1.0L)) ||
      fetestexcept(FE_INVALID) == 0) {
    fprintf(stderr, "fminl signaling NaN handling failed\n");
    return 1;
  }
  feclearexcept(FE_ALL_EXCEPT);
  if (!isnan(nelibc_fmaxl(1.0L, signaling_nan.value)) ||
      fetestexcept(FE_INVALID) == 0) {
    fprintf(stderr, "fmaxl signaling NaN handling failed\n");
    return 1;
  }
  feclearexcept(FE_ALL_EXCEPT);
  if (!isnan(nelibc_fdiml(signaling_nan.value, 1.0L)) ||
      fetestexcept(FE_INVALID) == 0) {
    fprintf(stderr, "fdiml signaling NaN handling failed\n");
    return 1;
  }
  const long double cbrtl_values[] = {
      -INFINITY, -LDBL_MAX, -8.0L, -1.0L, -LDBL_MIN,
      -nextafterl(0.0L, 1.0L), -0.0L, 0.0L, nextafterl(0.0L, 1.0L),
      LDBL_MIN, 0.125L, 1.0L, 2.0L, 8.0L, LDBL_MAX, INFINITY};
  for (size_t i = 0; i < sizeof(cbrtl_values) / sizeof(cbrtl_values[0]); ++i) {
    long double actual = nelibc_cbrtl(cbrtl_values[i]);
    long double expected = cbrtl(cbrtl_values[i]);
    if (!long_double_math_close(actual, expected) ||
        (expected == 0.0L && signbit(actual) != signbit(expected))) {
      fprintf(stderr, "cbrtl edge mismatch i=%zu actual=%.21Le expected=%.21Le\n",
              i, actual, expected);
      return 1;
    }
  }
  for (size_t i = 0; i < 512; ++i) {
    int exponent = -16000 + (int)i * 62;
    long double value = ldexpl(1.25L + (long double)(i % 7) / 16.0L, exponent);
    if (i & 1) value = -value;
    if (!long_double_math_close(nelibc_cbrtl(value), cbrtl(value))) {
      fprintf(stderr, "cbrtl exponent-range mismatch i=%zu\n", i);
      return 1;
    }
  }
  for (size_t i = 0; i < 4096; ++i) {
    long double value = (long double)((long long)((i * 79) % 20001) - 10000) / 17.0L;
    if (!long_double_math_close(nelibc_cbrtl(value), cbrtl(value))) {
      fprintf(stderr, "cbrtl generated-input mismatch i=%zu\n", i);
      return 1;
    }
  }
  if (!isnan(nelibc_cbrtl(quiet_nan.value)) ||
      !isnan(nelibc_cbrtl(signaling_nan.value)) ||
      !signbit(nelibc_cbrtl(-0.0L))) {
    fprintf(stderr, "cbrtl NaN/signed-zero handling failed\n");
    return 1;
  }
  if (!isnan(nelibc_nanl("nelibc")) || signbit(nelibc_nanl(""))) {
    fprintf(stderr, "nanl canonical quiet NaN handling failed\n");
    return 1;
  }
  errno = 0;
  if (!isinf(nelibc_fdiml(LDBL_MAX, -LDBL_MAX)) || errno != ERANGE) {
    fprintf(stderr, "fdiml overflow handling failed\n");
    return 1;
  }
  const long double atan_values[] = {
      -INFINITY, -LDBL_MAX, -1.0L, -LDBL_MIN, -0.0L,
      0.0L, LDBL_MIN, 1.0L, LDBL_MAX, INFINITY};
  for (size_t i = 0; i < sizeof(atan_values) / sizeof(atan_values[0]); ++i) {
    long double expected_atan = atanl(atan_values[i]);
    long double actual_atan = nelibc_atanl(atan_values[i]);
    if (!long_double_close(actual_atan, expected_atan) ||
        (expected_atan == 0.0L && signbit(expected_atan) != signbit(actual_atan))) {
      fprintf(stderr, "atanl mismatch value=%zu: %.21La != %.21La\n",
        i, actual_atan, expected_atan);
      return 1;
    }
    for (size_t j = 0; j < sizeof(atan_values) / sizeof(atan_values[0]); ++j) {
      long double expected_atan2 = atan2l(atan_values[i], atan_values[j]);
      long double actual_atan2 = nelibc_atan2l(atan_values[i], atan_values[j]);
      if (!long_double_close(actual_atan2, expected_atan2) ||
          (expected_atan2 == 0.0L && signbit(expected_atan2) != signbit(actual_atan2))) {
        fprintf(stderr, "atan2l mismatch values=%zu/%zu\n", i, j);
        return 1;
      }
    }
  }
  for (size_t i = 0; i < 4096; ++i) {
    long double y = (long double)((long long)((i * 97) % 2001) - 1000) / 37.0L;
    long double x = (long double)((long long)((i * 53) % 2003) - 1001) / 43.0L;
    if (!long_double_close(nelibc_atanl(y), atanl(y)) ||
        !long_double_close(nelibc_atan2l(y, x), atan2l(y, x))) {
      fprintf(stderr, "long-double arctangent generated-input mismatch i=%zu\n", i);
      return 1;
    }
  }
  if (!isnan(nelibc_atanl(quiet_nan.value)) ||
      !isnan(nelibc_atan2l(quiet_nan.value, 1.0L)) ||
      !isnan(nelibc_asinl(quiet_nan.value)) ||
      !isnan(nelibc_acosl(quiet_nan.value)) ||
      !isnan(nelibc_logl(quiet_nan.value)) ||
      !isnan(nelibc_log2l(quiet_nan.value)) ||
      !isnan(nelibc_log10l(quiet_nan.value)) ||
      !isnan(nelibc_log1pl(quiet_nan.value)) ||
      !isnan(nelibc_expl(quiet_nan.value)) ||
      !isnan(nelibc_exp2l(quiet_nan.value)) ||
      !isnan(nelibc_expm1l(quiet_nan.value)) ||
      !isnan(nelibc_sinl(quiet_nan.value)) ||
      !isnan(nelibc_cosl(quiet_nan.value)) ||
      !isnan(nelibc_tanhl(quiet_nan.value)) ||
      !isnan(nelibc_sinhl(quiet_nan.value)) ||
      !isnan(nelibc_coshl(quiet_nan.value)) ||
      !isnan(nelibc_asinhl(quiet_nan.value)) ||
      !isnan(nelibc_acoshl(quiet_nan.value)) ||
      !isnan(nelibc_atanhl(quiet_nan.value))) {
    fprintf(stderr, "long-double arctangent NaN propagation failed\n");
    return 1;
  }
  const long double powl_bases[] = {
      -INFINITY, -LDBL_MAX, -3.0L, -2.0L, -1.0L, -0.0L, 0.0L,
      nextafterl(0.0L, 1.0L), LDBL_MIN, 0.5L, 1.0L, 2.0L, 3.0L,
      LDBL_MAX, INFINITY};
  const long double powl_exponents[] = {
      -INFINITY, -65.0L, -5.0L, -3.0L, -2.0L, -1.0L, -0.5L,
      -0.0L, 0.0L, 0.5L, 1.0L, 2.0L, 3.0L, 5.0L, 64.0L,
      65.0L, INFINITY};
  for (size_t i = 0; i < sizeof(powl_bases) / sizeof(powl_bases[0]); ++i) {
    for (size_t j = 0; j < sizeof(powl_exponents) / sizeof(powl_exponents[0]); ++j) {
      long double actual = nelibc_powl(powl_bases[i], powl_exponents[j]);
      long double expected = powl(powl_bases[i], powl_exponents[j]);
      if (!long_double_math_close(actual, expected) ||
          (expected == 0.0L && signbit(actual) != signbit(expected))) {
        fprintf(stderr, "powl edge mismatch base=%zu exponent=%zu actual=%.21Le expected=%.21Le\n",
                i, j, actual, expected);
        return 1;
      }
    }
  }
  for (size_t i = 0; i < 2048; ++i) {
    long double base = 0.125L + (long double)((i * 37) % 4097) / 512.0L;
    long double exponent = (long double)((long long)((i * 53) % 129) - 64) / 4.0L;
    if (!long_double_math_close(nelibc_powl(base, exponent), powl(base, exponent))) {
      fprintf(stderr, "powl positive generated-input mismatch i=%zu\n", i);
      return 1;
    }
    long double negative_base = -0.5L - (long double)((i * 29) % 1025) / 256.0L;
    long double integer_exponent = (long double)((long long)((i * 71) % 65) - 32);
    if (!long_double_math_close(nelibc_powl(negative_base, integer_exponent),
                           powl(negative_base, integer_exponent))) {
      fprintf(stderr, "powl negative generated-input mismatch i=%zu\n", i);
      return 1;
    }
  }
  const long double exp10l_values[] = {
      -INFINITY, -5000.0L, -100.0L, -1.0L, -0.0L,
      0.0L, 1.0L, 10.0L, 100.0L, -4930.0L, -4940.0L,
      -4945.0L, -4950.0L, -4951.0L, -4952.0L,
      5000.0L, INFINITY, NAN};
  for (size_t i = 0; i < sizeof exp10l_values / sizeof exp10l_values[0]; ++i) {
    const long double value = exp10l_values[i];
    const long double expected = exp10l(value);
    const long double actual = nelibc_exp10l(value);
    const long double alias = nelibc_pow10l(value);
    const int near_subnormal = value <= -4940.0L && value >= -4952.0L;
    const int actual_close = near_subnormal
        ? fabsl(actual - expected) <= 0x1p-16445L
        : long_double_math_close(actual, expected);
    const int alias_close = near_subnormal
        ? fabsl(alias - expected) <= 0x1p-16445L
        : long_double_math_close(alias, expected);
    if (!actual_close || !alias_close ||
        (expected == 0.0L && (signbit(actual) != signbit(expected) ||
                              signbit(alias) != signbit(expected)))) {
      fprintf(stderr, "exp10l/pow10l mismatch input=%zu actual=%.21Le alias=%.21Le expected=%.21Le\n",
              i, actual, alias, expected);
      return 1;
    }
  }
  errno = 0;
  if (nelibc_exp10l(1.0L) != 10.0L || nelibc_pow10l(0.0L) != 1.0L ||
      !isinf(nelibc_exp10l(5000.0L)) || errno != ERANGE) {
    fprintf(stderr, "exp10l/pow10l exact values or overflow handling failed\n");
    return 1;
  }
  if (!isnan(nelibc_powl(-2.0L, 0.5L)) ||
      !isnan(nelibc_powl(quiet_nan.value, 2.0L)) ||
      nelibc_powl(quiet_nan.value, 0.0L) != 1.0L ||
      nelibc_powl(1.0L, quiet_nan.value) != 1.0L) {
    fprintf(stderr, "powl NaN/domain special cases failed\n");
    return 1;
  }
  if (nelibc_powl(-1.0L, 0x1p63L + 1.0L) != -1.0L ||
      nelibc_powl(-1.0L, 0x1p64L) != 1.0L) {
    fprintf(stderr, "powl binary80 integer parity failed\n");
    return 1;
  }
  errno = 0;
  if (!isnan(nelibc_powl(-2.0L, 0.5L)) || errno != EDOM) {
    fprintf(stderr, "powl negative-base domain handling failed\n");
    return 1;
  }
  errno = 0;
  if (!isinf(nelibc_powl(0.0L, -2.0L)) || signbit(nelibc_powl(0.0L, -2.0L)) ||
      errno != ERANGE) {
    fprintf(stderr, "powl zero pole handling failed\n");
    return 1;
  }
  errno = 0;
  if (!isinf(nelibc_powl(LDBL_MAX, 2.0L)) || errno != ERANGE) {
    fprintf(stderr, "powl overflow handling failed\n");
    return 1;
  }
  errno = 0;
  if (nelibc_powl(LDBL_MIN, 2.0L) != 0.0L || errno != ERANGE) {
    fprintf(stderr, "powl underflow handling failed\n");
    return 1;
  }
  feclearexcept(FE_ALL_EXCEPT);
  errno = 0;
  volatile long double host_powl_identity = powl(LDBL_MIN, 1.0L);
  int host_powl_identity_flags = fetestexcept(FE_ALL_EXCEPT);
  int host_powl_identity_errno = errno;
  feclearexcept(FE_ALL_EXCEPT);
  errno = 0;
  volatile long double nelibc_powl_identity = nelibc_powl(LDBL_MIN, 1.0L);
  if (nelibc_powl_identity != host_powl_identity ||
      fetestexcept(FE_ALL_EXCEPT) != host_powl_identity_flags ||
      errno != host_powl_identity_errno) {
    fprintf(stderr, "powl exponent-one identity raised unexpected flags or errno\n");
    return 1;
  }
  feclearexcept(FE_ALL_EXCEPT);
  errno = 0;
  volatile long double host_powl_exact_subnormal = powl(2.0L, -16445.0L);
  int host_powl_exact_subnormal_errno = errno;
  feclearexcept(FE_ALL_EXCEPT);
  errno = 0;
  volatile long double nelibc_powl_exact_subnormal = nelibc_powl(2.0L, -16445.0L);
  if (nelibc_powl_exact_subnormal != host_powl_exact_subnormal ||
      errno != host_powl_exact_subnormal_errno) {
    fprintf(stderr, "powl exact subnormal result or errno mismatch\n");
    return 1;
  }
  feclearexcept(FE_ALL_EXCEPT);
  errno = 0;
  volatile long double host_powl_negative_subnormal = powl(-0.5L, 16445.0L);
  int host_powl_negative_subnormal_errno = errno;
  feclearexcept(FE_ALL_EXCEPT);
  errno = 0;
  volatile long double nelibc_powl_negative_subnormal = nelibc_powl(-0.5L, 16445.0L);
  if (nelibc_powl_negative_subnormal != host_powl_negative_subnormal ||
      errno != host_powl_negative_subnormal_errno) {
    fprintf(stderr, "powl exact negative subnormal result or errno mismatch\n");
    return 1;
  }
  feclearexcept(FE_ALL_EXCEPT);
  errno = 0;
  if (nelibc_powl(LDBL_MAX, -2.0L) != 0.0L || errno != ERANGE ||
      (fetestexcept(FE_UNDERFLOW) == 0) || (fetestexcept(FE_OVERFLOW) != 0)) {
    fprintf(stderr, "powl negative-exponent underflow flags failed\n");
    return 1;
  }
  for (size_t i = 0; i < sizeof(atan_values) / sizeof(atan_values[0]); ++i) {
    long double expected_asin = asinl(atan_values[i]);
    long double actual_asin = nelibc_asinl(atan_values[i]);
    long double expected_acos = acosl(atan_values[i]);
    long double actual_acos = nelibc_acosl(atan_values[i]);
    if (!long_double_close(actual_asin, expected_asin) ||
        !long_double_close(actual_acos, expected_acos) ||
        (expected_asin == 0.0L && signbit(expected_asin) != signbit(actual_asin))) {
      fprintf(stderr, "asinl/acosl edge mismatch value=%zu\n", i);
      return 1;
    }
  }
  for (size_t i = 0; i < 4096; ++i) {
    long double value = (long double)((long long)((i * 71) % 2001) - 1000) / 1000.0L;
    if (!long_double_close(nelibc_asinl(value), asinl(value)) ||
        !long_double_close(nelibc_acosl(value), acosl(value))) {
      fprintf(stderr, "asinl/acosl generated-input mismatch i=%zu\n", i);
      return 1;
    }
  }
  errno = 0;
  if (!isnan(nelibc_asinl(2.0L)) || errno != EDOM) {
    fprintf(stderr, "asinl domain handling failed\n");
    return 1;
  }
  errno = 0;
  if (!isnan(nelibc_acosl(-2.0L)) || errno != EDOM) {
    fprintf(stderr, "acosl domain handling failed\n");
    return 1;
  }
  const long double asinhl_values[] = {
      -INFINITY, -LDBL_MAX, -0x1p8191L, -1000.0L, -1.0L, -LDBL_MIN,
      -0.0L, 0.0L, nextafterl(0.0L, 1.0L), LDBL_MIN, 1.0L, 1000.0L,
      0x1p8191L, LDBL_MAX, INFINITY};
  for (size_t i = 0; i < sizeof(asinhl_values) / sizeof(asinhl_values[0]); ++i) {
    long double actual = nelibc_asinhl(asinhl_values[i]);
    long double expected = asinhl(asinhl_values[i]);
    if (!long_double_close(actual, expected) ||
        (expected == 0.0L && signbit(actual) != signbit(expected))) {
      fprintf(stderr, "asinhl edge mismatch i=%zu actual=%.21Le expected=%.21Le\n",
              i, actual, expected);
      return 1;
    }
  }
  for (size_t i = 0; i < 4096; ++i) {
    long double value = (long double)((long long)((i * 83) % 20001) - 10000) / 10.0L;
    if (!long_double_close(nelibc_asinhl(value), asinhl(value))) {
      fprintf(stderr, "asinhl generated-input mismatch i=%zu\n", i);
      return 1;
    }
  }
  if (nelibc_asinhl(-0.0L) != 0.0L || !signbit(nelibc_asinhl(-0.0L))) {
    fprintf(stderr, "asinhl signed-zero behavior failed\n");
    return 1;
  }
  const long double acoshl_values[] = {
      1.0L, nextafterl(1.0L, 2.0L), 1.0001L, 1.5L, 2.0L, 1000.0L,
      0x1p8191L, LDBL_MAX, INFINITY};
  for (size_t i = 0; i < sizeof(acoshl_values) / sizeof(acoshl_values[0]); ++i) {
    if (!long_double_close(nelibc_acoshl(acoshl_values[i]), acoshl(acoshl_values[i]))) {
      fprintf(stderr, "acoshl edge mismatch i=%zu actual=%.21Le expected=%.21Le\n",
              i, nelibc_acoshl(acoshl_values[i]), acoshl(acoshl_values[i]));
      return 1;
    }
  }
  for (size_t i = 0; i < 2048; ++i) {
    long double value = 1.0L + (long double)i / 8.0L;
    if (!long_double_close(nelibc_acoshl(value), acoshl(value))) {
      fprintf(stderr, "acoshl generated-input mismatch i=%zu\n", i);
      return 1;
    }
  }
  errno = 0;
  if (!isnan(nelibc_acoshl(0.5L)) || errno != EDOM) {
    fprintf(stderr, "acoshl domain handling failed\n");
    return 1;
  }
  if (nelibc_acoshl(1.0L) != 0.0L || signbit(nelibc_acoshl(1.0L))) {
    fprintf(stderr, "acoshl boundary behavior failed\n");
    return 1;
  }
  const long double atanhl_values[] = {
      -INFINITY, -1.0L, nextafterl(-1.0L, 0.0L), -0.5L, -LDBL_MIN,
      -0.0L, 0.0L, nextafterl(0.0L, 1.0L), LDBL_MIN, 0.5L,
      nextafterl(1.0L, 0.0L), 1.0L, nextafterl(1.0L, INFINITY), INFINITY};
  for (size_t i = 0; i < sizeof(atanhl_values) / sizeof(atanhl_values[0]); ++i) {
    if (!long_double_close(nelibc_atanhl(atanhl_values[i]), atanhl(atanhl_values[i]))) {
      fprintf(stderr, "atanhl edge mismatch i=%zu actual=%.21Le expected=%.21Le\n",
              i, nelibc_atanhl(atanhl_values[i]), atanhl(atanhl_values[i]));
      return 1;
    }
  }
  for (size_t i = 0; i < 4096; ++i) {
    long double value = -0.999L + (long double)i * (1.998L / 4095.0L);
    if (!long_double_close(nelibc_atanhl(value), atanhl(value))) {
      fprintf(stderr, "atanhl generated-input mismatch i=%zu\n", i);
      return 1;
    }
  }
  if (nelibc_atanhl(-0.0L) != 0.0L || !signbit(nelibc_atanhl(-0.0L))) {
    fprintf(stderr, "atanhl signed-zero behavior failed\n");
    return 1;
  }
  errno = 0;
  if (!isinf(nelibc_atanhl(1.0L)) || signbit(nelibc_atanhl(1.0L)) || errno != ERANGE) {
    fprintf(stderr, "atanhl positive pole behavior failed\n");
    return 1;
  }
  errno = 0;
  if (!isinf(nelibc_atanhl(-1.0L)) || !signbit(nelibc_atanhl(-1.0L)) || errno != ERANGE) {
    fprintf(stderr, "atanhl negative pole behavior failed\n");
    return 1;
  }
  errno = 0;
  if (!isnan(nelibc_atanhl(1.5L)) || errno != EDOM) {
    fprintf(stderr, "atanhl domain handling failed\n");
    return 1;
  }
  const long double log_values[] = {
      nextafterl(0.0L, 1.0L), LDBL_MIN, 0.125L, 0.5L, 1.0L, 2.0L, 10.0L,
      LDBL_MAX, INFINITY, 0.0L, -0.0L, -1.0L, -INFINITY};
  for (size_t i = 0; i < sizeof(log_values) / sizeof(log_values[0]); ++i) {
    if (!long_double_close(nelibc_logl(log_values[i]), logl(log_values[i])) ||
        !long_double_close(nelibc_log2l(log_values[i]), log2l(log_values[i])) ||
        !long_double_close(nelibc_log10l(log_values[i]), log10l(log_values[i]))) {
      fprintf(stderr, "long-double logarithm edge mismatch value=%zu\n", i);
      return 1;
    }
  }
  for (size_t i = 0; i < 512; ++i) {
    int exponent = -16000 + (int)i * 62;
    long double value = ldexpl(1.25L, exponent);
    if (!long_double_close(nelibc_logl(value), logl(value)) ||
        !long_double_close(nelibc_log2l(value), log2l(value)) ||
        !long_double_close(nelibc_log10l(value), log10l(value))) {
      fprintf(stderr, "long-double logarithm generated-input mismatch i=%zu\n", i);
      return 1;
    }
  }
  feclearexcept(FE_ALL_EXCEPT);
  errno = 0;
  if (!isinf(nelibc_logl(0.0L)) || signbit(nelibc_logl(0.0L)) == 0 ||
      errno != ERANGE || fetestexcept(FE_DIVBYZERO) == 0) {
    fprintf(stderr, "logl zero range behavior failed\n");
    return 1;
  }
  feclearexcept(FE_ALL_EXCEPT);
  errno = 0;
  if (!isnan(nelibc_log2l(-1.0L)) || errno != EDOM ||
      fetestexcept(FE_INVALID) == 0) {
    fprintf(stderr, "log2l negative domain behavior failed\n");
    return 1;
  }
  errno = 0;
  if (!isnan(nelibc_log10l(-1.0L)) || errno != EDOM) {
    fprintf(stderr, "log10l negative domain behavior failed\n");
    return 1;
  }
  const long double log1p_values[] = {
      -INFINITY, -1.5L, -1.0L, nextafterl(-1.0L, 0.0L), -0.75L, -0.5L,
      nextafterl(-0.5L, 0.0L), -0x1p-60L, -nextafterl(0.0L, 1.0L),
      -0.0L, 0.0L, nextafterl(0.0L, 1.0L), 0x1p-60L, 0.5L,
      nextafterl(0.5L, 0.0L), 0.75L, LDBL_MAX, INFINITY};
  for (size_t i = 0; i < sizeof(log1p_values) / sizeof(log1p_values[0]); ++i) {
    if (!long_double_close(nelibc_log1pl(log1p_values[i]), log1pl(log1p_values[i]))) {
      fprintf(stderr, "log1pl edge mismatch value=%zu\n", i);
      return 1;
    }
  }
  for (size_t i = 0; i < 512; ++i) {
    int exponent = -16000 + (int)i * 31;
    long double small = ldexpl(1.25L, exponent);
    if (!long_double_close(nelibc_log1pl(small), log1pl(small)) ||
        !long_double_close(nelibc_log1pl(-small), log1pl(-small))) {
      fprintf(stderr, "log1pl generated-input mismatch i=%zu\n", i);
      return 1;
    }
  }
  feclearexcept(FE_ALL_EXCEPT);
  errno = 0;
  if (!isinf(nelibc_log1pl(-1.0L)) || signbit(nelibc_log1pl(-1.0L)) == 0 ||
      errno != ERANGE || fetestexcept(FE_DIVBYZERO) == 0) {
    fprintf(stderr, "log1pl pole behavior failed\n");
    return 1;
  }
  feclearexcept(FE_ALL_EXCEPT);
  errno = 0;
  if (!isnan(nelibc_log1pl(-1.5L)) || errno != EDOM ||
      fetestexcept(FE_INVALID) == 0) {
    fprintf(stderr, "log1pl domain behavior failed\n");
    return 1;
  }
  const long double exp_values[] = {
      -INFINITY, -12000.0L, -1000.0L, -1.0L, -0.0L, 0.0L,
      1.0L, 10.0L, 1000.0L, 11000.0L, 11350.0L, 12000.0L, INFINITY};
  for (size_t i = 0; i < sizeof(exp_values) / sizeof(exp_values[0]); ++i) {
    long double expected_expm1 = expm1l(exp_values[i]);
    if (exp_values[i] > 48.0L) expected_expm1 = expl(exp_values[i]);
    if (!long_double_close(nelibc_expl(exp_values[i]), expl(exp_values[i])) ||
        !long_double_math_close(nelibc_expm1l(exp_values[i]), expected_expm1)) {
      fprintf(stderr, "long-double exponential edge mismatch value=%zu input=%.6Lf expl=%.21Lg/%.21Lg expm1l=%.21Lg/%.21Lg\n",
              i, exp_values[i], nelibc_expl(exp_values[i]), expl(exp_values[i]),
              nelibc_expm1l(exp_values[i]), expm1l(exp_values[i]));
      return 1;
    }
  }
  const long double exp2_values[] = {
      -INFINITY, -16445.0L, -10000.5L, -1.0L, -0.0L, 0.0L,
      0.5L, 1.0L, 10.0L, 1000.0L, 16383.5L, 16384.0L, INFINITY};
  for (size_t i = 0; i < sizeof(exp2_values) / sizeof(exp2_values[0]); ++i) {
    if (!long_double_close(nelibc_exp2l(exp2_values[i]), exp2l(exp2_values[i]))) {
      fprintf(stderr, "exp2l edge mismatch value=%zu input=%.6Lf actual=%.21Lg expected=%.21Lg\n",
              i, exp2_values[i], nelibc_exp2l(exp2_values[i]), exp2l(exp2_values[i]));
      return 1;
    }
  }
  for (size_t i = 0; i < 512; ++i) {
    long double value = -16000.0L + (long double)i * 63.0L;
    if (!long_double_close(nelibc_exp2l(value), exp2l(value))) {
      fprintf(stderr, "exp2l generated-input mismatch i=%zu\n", i);
      return 1;
    }
  }
  const long double trigl_values[] = {
      0.0L, -0.0L, LDBL_MIN, -LDBL_MIN, 0.1L, -1.0L,
      0x1.921fb54442d1846ap+0L,
      nextafterl(0x1.921fb54442d1846ap+0L, -INFINITY),
      nextafterl(0x1.921fb54442d1846ap+0L, INFINITY),
      1.0L, 1.0e6L, 1.0e20L,
      1.0e100L, 0x1p1000L, LDBL_MAX, -LDBL_MAX};
  for (size_t i = 0; i < sizeof(trigl_values) / sizeof(trigl_values[0]); ++i) {
    long double sine, cosine;
    nelibc_sincosl(trigl_values[i], &sine, &cosine);
    if (!long_double_trig_close(nelibc_sinl(trigl_values[i]), sinl(trigl_values[i])) ||
        !long_double_trig_close(nelibc_cosl(trigl_values[i]), cosl(trigl_values[i])) ||
        !long_double_tan_close(nelibc_tanl(trigl_values[i]), tanl(trigl_values[i])) ||
        !long_double_trig_close(sine, sinl(trigl_values[i])) ||
        !long_double_trig_close(cosine, cosl(trigl_values[i]))) {
      fprintf(stderr, "long-double trig edge mismatch i=%zu input=%.6Le sin=%.21Le/%.21Le cos=%.21Le/%.21Le tan=%.21Le/%.21Le\n",
              i, trigl_values[i], sine, sinl(trigl_values[i]), cosine, cosl(trigl_values[i]),
              nelibc_tanl(trigl_values[i]), tanl(trigl_values[i]));
      return 1;
    }
  }
  if (nelibc_sinl(-0.0L) != 0.0L || !signbit(nelibc_sinl(-0.0L)) ||
      nelibc_cosl(-0.0L) != 1.0L || nelibc_tanl(-0.0L) != 0.0L ||
      !signbit(nelibc_tanl(-0.0L))) {
    fprintf(stderr, "long-double trig signed-zero behavior failed sin=%.21La sign=%d cos=%.21La\n",
            nelibc_sinl(-0.0L), signbit(nelibc_sinl(-0.0L)), nelibc_cosl(-0.0L));
    return 1;
  }
  for (int exponent = 0; exponent < LDBL_MAX_EXP - 2; exponent += 61) {
    long double value = ldexpl(1.234567890123456789L, exponent);
    if (!long_double_trig_close(nelibc_sinl(value), sinl(value)) ||
        !long_double_trig_close(nelibc_cosl(value), cosl(value)) ||
        !long_double_tan_close(nelibc_tanl(value), tanl(value))) {
      fprintf(stderr, "long-double trig generated-input mismatch exponent=%d sin=%.21Le/%.21Le cos=%.21Le/%.21Le tan=%.21Le/%.21Le\n",
              exponent, nelibc_sinl(value), sinl(value), nelibc_cosl(value), cosl(value),
              nelibc_tanl(value), tanl(value));
      return 1;
    }
  }
  errno = 0;
  if (!isnan(nelibc_sinl(INFINITY)) || errno != EDOM) {
    fprintf(stderr, "long-double trig infinite-domain behavior failed\n");
    return 1;
  }
  errno = 0;
  if (!isnan(nelibc_tanl(INFINITY)) || errno != EDOM) {
    fprintf(stderr, "long-double tan infinite-domain behavior failed\n");
    return 1;
  }
  const long double tanh_values[] = {
      -INFINITY, -LDBL_MAX, -40.0L, -24.0L, -10.0L, -1.0L, -0.1L,
      -LDBL_MIN, -0.0L, 0.0L, LDBL_MIN, 0.1L, 1.0L, 10.0L, 24.0L,
      40.0L, LDBL_MAX, INFINITY};
  for (size_t i = 0; i < sizeof(tanh_values) / sizeof(tanh_values[0]); ++i) {
    if (!long_double_close(nelibc_tanhl(tanh_values[i]), tanhl(tanh_values[i]))) {
      fprintf(stderr, "long-double tanh edge mismatch i=%zu input=%.6Le actual=%.21Le expected=%.21Le\n",
              i, tanh_values[i], nelibc_tanhl(tanh_values[i]), tanhl(tanh_values[i]));
      return 1;
    }
  }
  if (nelibc_tanhl(-0.0L) != 0.0L || !signbit(nelibc_tanhl(-0.0L))) {
    fprintf(stderr, "long-double tanh signed-zero behavior failed\n");
    return 1;
  }
  for (size_t i = 0; i < 1024; ++i) {
    long double value = -40.0L + (long double)i * (80.0L / 1023.0L);
    if (!long_double_close(nelibc_tanhl(value), tanhl(value))) {
      fprintf(stderr, "long-double tanh generated-input mismatch i=%zu input=%.6Le\n", i, value);
      return 1;
    }
  }
  errno = 0;
  if (nelibc_tanhl(LDBL_MAX) != 1.0L || errno != 0) {
    fprintf(stderr, "long-double tanh saturation raised errno\n");
    return 1;
  }
  const long double hyperbolic_values[] = {
      -INFINITY, -LDBL_MAX, -11357.3L, -11357.0L, -11356.0L, -40.0L,
      -1.0L, -LDBL_MIN, -0.0L, 0.0L, nextafterl(0.0L, 1.0L), LDBL_MIN,
      1.0L, 40.0L, 11356.0L, 11357.0L, 11357.3L, LDBL_MAX, INFINITY};
  for (size_t i = 0; i < sizeof(hyperbolic_values) / sizeof(hyperbolic_values[0]); ++i) {
    if (!long_double_hyperbolic_close(nelibc_sinhl(hyperbolic_values[i]), sinhl(hyperbolic_values[i])) ||
        !long_double_hyperbolic_close(nelibc_coshl(hyperbolic_values[i]), coshl(hyperbolic_values[i]))) {
      fprintf(stderr, "long-double hyperbolic edge mismatch i=%zu input=%.6Le sinh=%.21Le/%.21Le cosh=%.21Le/%.21Le\n",
              i, hyperbolic_values[i], nelibc_sinhl(hyperbolic_values[i]),
              sinhl(hyperbolic_values[i]), nelibc_coshl(hyperbolic_values[i]),
              coshl(hyperbolic_values[i]));
      return 1;
    }
  }
  if (nelibc_sinhl(-0.0L) != 0.0L || !signbit(nelibc_sinhl(-0.0L)) ||
      nelibc_coshl(-0.0L) != 1.0L) {
    fprintf(stderr, "long-double hyperbolic signed-zero behavior failed\n");
    return 1;
  }
  for (size_t i = 0; i < 1024; ++i) {
    long double value = -45.0L + (long double)i * (90.0L / 1023.0L);
    if (!long_double_hyperbolic_close(nelibc_sinhl(value), sinhl(value)) ||
        !long_double_hyperbolic_close(nelibc_coshl(value), coshl(value))) {
      fprintf(stderr, "long-double hyperbolic generated-input mismatch i=%zu input=%.6Le sinh=%.21Le/%.21Le cosh=%.21Le/%.21Le\n",
              i, value, nelibc_sinhl(value), sinhl(value), nelibc_coshl(value), coshl(value));
      return 1;
    }
  }
  feclearexcept(FE_ALL_EXCEPT);
  errno = 0;
  if (!isinf(nelibc_sinhl(LDBL_MAX)) || errno != ERANGE ||
      fetestexcept(FE_OVERFLOW) == 0) {
    fprintf(stderr, "long-double sinhl overflow behavior failed\n");
    return 1;
  }
  feclearexcept(FE_ALL_EXCEPT);
  errno = 0;
  if (!isinf(nelibc_coshl(LDBL_MAX)) || errno != ERANGE ||
      fetestexcept(FE_OVERFLOW) == 0) {
    fprintf(stderr, "long-double coshl overflow behavior failed\n");
    return 1;
  }
  for (size_t i = 0; i < 512; ++i) {
    long double value = -11000.0L + (long double)i * 43.0L;
    if (!long_double_close(nelibc_expl(value), expl(value))) {
      fprintf(stderr, "expl generated-input mismatch i=%zu\n", i);
      return 1;
    }
  }
  for (size_t i = 0; i < 512; ++i) {
    int exponent = -16000 + (int)i * 31;
    long double small = ldexpl(1.25L, exponent);
    if (!long_double_close(nelibc_expm1l(small), expm1l(small)) ||
        !long_double_close(nelibc_expm1l(-small), expm1l(-small))) {
      fprintf(stderr, "expm1l generated-input mismatch i=%zu\n", i);
      return 1;
    }
  }
  feclearexcept(FE_ALL_EXCEPT);
  errno = 0;
  if (!isinf(nelibc_expl(12000.0L)) || errno != ERANGE ||
      fetestexcept(FE_OVERFLOW) == 0) {
    fprintf(stderr, "expl overflow behavior failed\n");
    return 1;
  }
  feclearexcept(FE_ALL_EXCEPT);
  errno = 0;
  if (nelibc_expl(-20000.0L) != 0.0L || errno != ERANGE ||
      fetestexcept(FE_UNDERFLOW) == 0) {
    fprintf(stderr, "expl underflow behavior failed\n");
    return 1;
  }
  errno = 0;
  if (nelibc_expm1l(-1000.0L) != -1.0L || errno != 0) {
    fprintf(stderr, "expm1l negative asymptote behavior failed\n");
    return 1;
  }
  int saved_rounding_before_expm1l = fegetround();
  if (fesetround(FE_UPWARD) != 0 ||
      nelibc_expm1l(-1000.0L) != nextafterl(-1.0L, 0.0L)) {
    fprintf(stderr, "expm1l upward rounding behavior failed actual=%.21La expected=%.21La\n",
            nelibc_expm1l(-1000.0L), nextafterl(-1.0L, 0.0L));
    return 1;
  }
  if (fesetround(saved_rounding_before_expm1l) != 0) return 1;
  const long double nextafter_minimum = nextafterl(0.0L, 1.0L);
  const long double nextafter_negative_minimum = nextafterl(0.0L, -1.0L);
  const long double nextafter_inputs[] = {
      0.0L, -0.0L, nextafter_minimum, nextafter_negative_minimum,
      LDBL_MIN, -LDBL_MIN, 1.0L, -1.0L, LDBL_MAX, -LDBL_MAX,
      INFINITY, -INFINITY};
  const long double nextafter_targets[] = {
      0.0L, -0.0L, 1.0L, -1.0L, LDBL_MIN, -LDBL_MIN, INFINITY, -INFINITY};
  for (size_t i = 0; i < sizeof(nextafter_inputs) / sizeof(nextafter_inputs[0]); ++i) {
    for (size_t j = 0; j < sizeof(nextafter_targets) / sizeof(nextafter_targets[0]); ++j) {
      long double expected_next = nextafterl(nextafter_inputs[i], nextafter_targets[j]);
      /* musl 1.2.5 steps one value too far for -LDBL_MIN toward signed zero. */
      if (nextafter_inputs[i] == -LDBL_MIN && nextafter_targets[j] > -LDBL_MIN)
        expected_next = -nextafterl(LDBL_MIN, 0.0L);
      long double actual_next = nelibc_nextafterl(nextafter_inputs[i], nextafter_targets[j]);
      if (actual_next != expected_next ||
          (actual_next == 0.0L && signbit(actual_next) != signbit(expected_next))) {
        union { long double value; struct { uint64_t significand; uint16_t sign_exponent; } parts; } debug_actual;
        union { long double value; struct { uint64_t significand; uint16_t sign_exponent; } parts; } debug_expected;
        debug_actual.value = actual_next;
        debug_expected.value = expected_next;
        fprintf(stderr, "nextafterl mismatch values=%zu/%zu: %.21La != %.21La\n",
          i, j, actual_next, expected_next);
        fprintf(stderr, "actual encoding=%016llx/%04x\n",
          (unsigned long long)debug_actual.parts.significand, debug_actual.parts.sign_exponent);
        fprintf(stderr, "expected encoding=%016llx/%04x\n",
          (unsigned long long)debug_expected.parts.significand, debug_expected.parts.sign_exponent);
        return 1;
      }
      long double actual_toward = nelibc_nexttowardl(nextafter_inputs[i], nextafter_targets[j]);
      long double expected_toward = expected_next;
      if (actual_toward != expected_toward ||
          (actual_toward == 0.0L && signbit(actual_toward) != signbit(expected_toward))) {
        fprintf(stderr, "nexttowardl mismatch values=%zu/%zu: %.21La != %.21La\n",
                i, j, actual_toward, expected_toward);
        return 1;
      }
    }
  }
  errno = 0;
  feclearexcept(FE_ALL_EXCEPT);
  volatile long double host_nextafter_zero = nextafterl(0.0L, 1.0L);
  int host_nextafter_zero_flags = fetestexcept(FE_UNDERFLOW | FE_INEXACT);
  int host_nextafter_zero_errno = errno;
  errno = 0;
  feclearexcept(FE_ALL_EXCEPT);
  volatile long double nelibc_nextafter_zero = nelibc_nextafterl(0.0L, 1.0L);
  int nelibc_nextafter_zero_flags = fetestexcept(FE_UNDERFLOW | FE_INEXACT);
  if (nelibc_nextafter_zero != nextafter_minimum ||
      host_nextafter_zero != nelibc_nextafter_zero ||
      host_nextafter_zero_flags != (FE_UNDERFLOW | FE_INEXACT) ||
      nelibc_nextafter_zero_flags != host_nextafter_zero_flags ||
      errno != host_nextafter_zero_errno) {
    fprintf(stderr, "nextafterl zero/subnormal handling failed\n");
    return 1;
  }
  errno = 0;
  feclearexcept(FE_ALL_EXCEPT);
  volatile long double host_nextafter_overflow = nextafterl(LDBL_MAX, INFINITY);
  int host_nextafter_overflow_flags = fetestexcept(FE_OVERFLOW | FE_INEXACT);
  errno = 0;
  feclearexcept(FE_ALL_EXCEPT);
  volatile long double nelibc_nextafter_overflow = nelibc_nextafterl(LDBL_MAX, INFINITY);
  int nelibc_nextafter_overflow_flags = fetestexcept(FE_OVERFLOW | FE_INEXACT);
  /* musl and glibc differ on ERANGE here; compare their required flags. */
  if (!isinf(nelibc_nextafter_overflow) ||
      host_nextafter_overflow != nelibc_nextafter_overflow ||
      host_nextafter_overflow_flags != (FE_OVERFLOW | FE_INEXACT) ||
      nelibc_nextafter_overflow_flags != host_nextafter_overflow_flags) {
    fprintf(stderr, "nextafterl overflow flags failed host=%x Nelibc=%x\n",
            host_nextafter_overflow_flags, nelibc_nextafter_overflow_flags);
    return 1;
  }
  feclearexcept(FE_ALL_EXCEPT);
  volatile long double host_nextafter_min_normal = nextafterl(LDBL_MIN, 0.0L);
  int host_nextafter_min_normal_flags = fetestexcept(FE_UNDERFLOW | FE_INEXACT);
  feclearexcept(FE_ALL_EXCEPT);
  volatile long double nelibc_nextafter_min_normal = nelibc_nextafterl(LDBL_MIN, 0.0L);
  int nelibc_nextafter_min_normal_flags = fetestexcept(FE_UNDERFLOW | FE_INEXACT);
  if (host_nextafter_min_normal != nelibc_nextafter_min_normal ||
      host_nextafter_min_normal_flags != (FE_UNDERFLOW | FE_INEXACT) ||
      nelibc_nextafter_min_normal_flags != host_nextafter_min_normal_flags) {
    fprintf(stderr, "nextafterl normal-to-subnormal flags failed host=%x Nelibc=%x\n",
            host_nextafter_min_normal_flags, nelibc_nextafter_min_normal_flags);
    return 1;
  }
  feclearexcept(FE_ALL_EXCEPT);
  volatile long double host_nextafter_min_normal_up =
      nextafterl(host_nextafter_min_normal, LDBL_MIN);
  int host_nextafter_min_normal_up_flags = fetestexcept(FE_UNDERFLOW | FE_INEXACT);
  feclearexcept(FE_ALL_EXCEPT);
  volatile long double nelibc_nextafter_min_normal_up =
      nelibc_nextafterl(host_nextafter_min_normal, LDBL_MIN);
  int nelibc_nextafter_min_normal_up_flags = fetestexcept(FE_UNDERFLOW | FE_INEXACT);
  if (host_nextafter_min_normal_up != LDBL_MIN ||
      nelibc_nextafter_min_normal_up != host_nextafter_min_normal_up ||
      host_nextafter_min_normal_up_flags != 0 ||
      nelibc_nextafter_min_normal_up_flags != host_nextafter_min_normal_up_flags) {
    fprintf(stderr, "nextafterl subnormal-to-normal flags failed host=%x Nelibc=%x\n",
            host_nextafter_min_normal_up_flags, nelibc_nextafter_min_normal_up_flags);
    return 1;
  }
  errno = 0;
  if (!isinf(nelibc_hypotl(LDBL_MAX, LDBL_MAX)) || errno != ERANGE) {
    fprintf(stderr, "hypotl overflow handling failed\n");
    return 1;
  }

  /* Preserve x87 precision beyond binary64 and the complete sign behavior. */
  long double extended = 1.0L + 0x1p-60L;
  if (nelibc_fabsl(-extended) != extended ||
      nelibc_copysignl(extended, -1.0L) != -extended ||
      !signbit(nelibc_copysignl(0.0L, -1.0L)) ||
      nelibc_sqrtl(extended) != sqrtl(extended) ||
      nelibc_fmodl(extended, 1.0L) != 0x1p-60L ||
      nelibc_remainderl(extended, 1.0L) != 0x1p-60L ||
      nelibc_fmodl(5.5L, 2.0L) != 1.5L ||
      nelibc_remainderl(5.5L, 2.0L) != -0.5L) {
    fprintf(stderr, "long-double fabs/copysign/sqrt/remainder failed\n");
    return 1;
  }
  errno = 0;
  if (!isnan(nelibc_fmodl(1.0L, 0.0L)) || errno != EDOM) {
    fprintf(stderr, "long-double fmod domain handling failed\n");
    return 1;
  }
  errno = 0;
  if (!isnan(nelibc_remainderl(1.0L, 0.0L)) || errno != EDOM) {
    fprintf(stderr, "long-double remainder domain handling failed\n");
    return 1;
  }
  const long double remquol_inputs[][2] = {
      {7.0L, 2.0L}, {-7.0L, 2.0L}, {7.0L, -2.0L}, {-7.0L, -2.0L},
      {5.5L, 2.0L}, {-0.0L, 1.0L}, {LDBL_MIN, LDBL_MIN},
      {extended, INFINITY}, {extended, 0.75L}, {-extended, 0.75L},
      {0x1.0000000000000001p+70L, 3.0L}, {LDBL_MAX, 0x1p-1000L}};
  for (size_t i = 0; i < sizeof remquol_inputs / sizeof remquol_inputs[0]; ++i) {
    int expected_quotient = 0, actual_quotient = 0;
    long double expected = remquol(remquol_inputs[i][0], remquol_inputs[i][1],
                                   &expected_quotient);
    long double actual = nelibc_remquol(remquol_inputs[i][0], remquol_inputs[i][1],
                                        &actual_quotient);
    if (actual != expected || (actual == 0.0L && signbit(actual) != signbit(expected)) ||
        expected_quotient % 8 != actual_quotient % 8) {
      fprintf(stderr, "remquol mismatch case=%zu remainder=%.21La/%.21La quotient=%d/%d\n",
              i, expected, actual, expected_quotient, actual_quotient);
      return 1;
    }
  }
  const long double extended_values[] = {
      0.0L, -0.0L, 0.75L, -12.75L, extended, -extended,
      0x1p-16445L, LDBL_MAX, INFINITY, NAN};
  for (size_t i = 0; i < sizeof(extended_values) / sizeof(extended_values[0]); ++i) {
    long double value = extended_values[i];
    int expected_exponent = 0, actual_exponent = 0;
    long double expected_fraction = frexpl(value, &expected_exponent);
    long double actual_fraction = nelibc_frexpl(value, &actual_exponent);
    if ((!isnan(value) && (actual_fraction != expected_fraction ||
                           (isfinite(value) && actual_exponent != expected_exponent))) ||
        (isnan(value) && !isnan(actual_fraction))) {
      fprintf(stderr, "frexpl mismatch at vector %zu: expected %.21Lg/%d actual %.21Lg/%d\n",
              i, expected_fraction, expected_exponent, actual_fraction, actual_exponent);
      return 1;
    }
    long double expected_integral = 0.0L, actual_integral = 0.0L;
    expected_fraction = modfl(value, &expected_integral);
    actual_fraction = nelibc_modfl(value, &actual_integral);
    if ((!isnan(value) && (actual_fraction != expected_fraction ||
                           actual_integral != expected_integral)) ||
        (isnan(value) && (!isnan(actual_fraction) || !isnan(actual_integral)))) {
      fprintf(stderr, "modfl mismatch at vector %zu: expected %.21Lg + %.21Lg actual %.21Lg + %.21Lg\n",
              i, expected_integral, expected_fraction, actual_integral, actual_fraction);
      return 1;
    }
    int expected_ilogb = ilogbl(value);
    int actual_ilogb = nelibc_ilogbl(value);
    long double expected_logb = logbl(value);
    long double actual_logb = nelibc_logbl(value);
    if (actual_ilogb != expected_ilogb ||
        (isnan(value) ? !isnan(actual_logb) : actual_logb != expected_logb)) {
      fprintf(stderr, "ilogbl/logbl mismatch at vector %zu: %d/%Lg expected %d/%Lg\n",
              i, actual_ilogb, actual_logb, expected_ilogb, expected_logb);
      return 1;
    }
  }
  if (nelibc_ldexpl(1.5L, 4) != ldexpl(1.5L, 4) ||
      nelibc_scalbnl(-0.75L, -2) != scalbnl(-0.75L, -2) ||
      nelibc_scalblnl(0.5L, 4) != scalblnl(0.5L, 4) ||
      nelibc_ldexpl(1.0L, LDBL_MIN_EXP - LDBL_MANT_DIG) != 0x1p-16445L) {
    fprintf(stderr, "long-double scaling mismatch\n");
    return 1;
  }
  const long double rounding_values[] = {
      0.0L, -0.0L, 0.25L, -0.25L, 0.499999999999L, -0.499999999999L,
      0.5L, -0.5L, 1.5L, -1.5L,
      0x1p-16445L, LDBL_MAX, INFINITY, NAN};
  for (size_t i = 0; i < sizeof(rounding_values) / sizeof(rounding_values[0]); ++i) {
    const long double value = rounding_values[i];
    const long double expected[] = {truncl(value), floorl(value), ceill(value)};
    const long double actual[] = {
        nelibc_truncl(value), nelibc_floorl(value), nelibc_ceill(value)};
    for (size_t j = 0; j < 3; ++j) {
      if ((!isnan(value) && actual[j] != expected[j]) ||
          (isnan(value) && !isnan(actual[j]))) {
        fprintf(stderr, "long-double directed rounding mismatch at %zu/%zu\n", i, j);
        return 1;
      }
    }
    const long double rounded_expected = roundl(value);
    const long double rounded_actual = nelibc_roundl(value);
    if ((!isnan(value) && rounded_actual != rounded_expected) ||
        (isnan(value) && !isnan(rounded_actual))) {
      fprintf(stderr, "long-double round mismatch at %zu\n", i);
      return 1;
    }
  }
  int long_saved_rounding = fegetround();
  const int long_rounding_modes[] = {FE_TONEAREST, FE_DOWNWARD, FE_UPWARD, FE_TOWARDZERO};
  const long double long_integer_rounding_values[] = {
      -12.75L, -1.5L, -0.5L, -0.25L, 0.25L, 0.5L, 1.5L, 12.75L};
  for (size_t i = 0; i < sizeof(long_rounding_modes) / sizeof(long_rounding_modes[0]); ++i) {
    if (fesetround(long_rounding_modes[i]) != 0) return 1;
    if (nelibc_rintl(2.5L) != rintl(2.5L) ||
        nelibc_rintl(-1.5L) != rintl(-1.5L) ||
        nelibc_nearbyintl(2.5L) != nearbyintl(2.5L) ||
        nelibc_lrintl(2.5L) != lrintl(2.5L) ||
        nelibc_llrintl(-1.5L) != llrintl(-1.5L) ||
        nelibc_lroundl(1.5L) != 2 || nelibc_llroundl(-1.5L) != -2 ||
        nelibc_roundl(0.5L) != 1.0L || nelibc_roundl(-0.5L) != -1.0L) {
      fprintf(stderr, "long-double active-mode rounding mismatch\n");
      return 1;
    }
    for (size_t j = 0; j < sizeof(long_integer_rounding_values) / sizeof(long_integer_rounding_values[0]); ++j) {
      const long double value = long_integer_rounding_values[j];
      if (nelibc_lroundl(value) != lroundl(value) ||
          nelibc_llroundl(value) != llroundl(value)) {
        fprintf(stderr, "long-double round-to-integer mismatch at mode %zu/value %zu\n", i, j);
        return 1;
      }
    }
  }
  if (fesetround(long_saved_rounding) != 0) return 1;
  feclearexcept(FE_ALL_EXCEPT);
  (void)nelibc_nearbyintl(2.25L);
  if (fetestexcept(FE_INEXACT) != 0) {
    fprintf(stderr, "nearbyintl unexpectedly raised FE_INEXACT\n");
    return 1;
  }
  feraiseexcept(FE_INEXACT);
  (void)nelibc_nearbyintl(2.25L);
  if (fetestexcept(FE_INEXACT) == 0) {
    fprintf(stderr, "nearbyintl cleared pre-existing FE_INEXACT\n");
    return 1;
  }
  feclearexcept(FE_ALL_EXCEPT);
  (void)nelibc_llroundl(2.25L);
  if (fetestexcept(FE_INEXACT) != 0) {
    fprintf(stderr, "llroundl unexpectedly raised FE_INEXACT\n");
    return 1;
  }
  feraiseexcept(FE_INEXACT);
  (void)nelibc_lroundl(-2.25L);
  if (fetestexcept(FE_INEXACT) == 0) {
    fprintf(stderr, "lroundl cleared pre-existing FE_INEXACT\n");
    return 1;
  }
  feclearexcept(FE_ALL_EXCEPT);
  (void)nelibc_llroundl(LDBL_MAX);
  if (fetestexcept(FE_INVALID) == 0) {
    fprintf(stderr, "llroundl overflow did not raise FE_INVALID\n");
    return 1;
  }
  feclearexcept(FE_ALL_EXCEPT);
  errno = 0;
  if (!isinf(nelibc_ldexpl(1.0L, LDBL_MAX_EXP)) || errno != ERANGE) {
    fprintf(stderr, "ldexpl overflow errno mismatch\n");
    return 1;
  }
  errno = 0;
  if (nelibc_scalblnl(0.5L, LONG_MIN) != 0.0L || errno != ERANGE) {
    fprintf(stderr, "scalblnl underflow errno mismatch\n");
    return 1;
  }
  int saved_rounding_before_modfl = fegetround();
  if (fesetround(FE_UPWARD) != 0) return 1;
  long double integral_part = 0.0L;
  if (nelibc_modfl(-12.75L, &integral_part) != -0.75L ||
      integral_part != -12.0L || fegetround() != FE_UPWARD) {
    fprintf(stderr, "modfl rounding-mode independence failed\n");
    return 1;
  }
  if (fesetround(saved_rounding_before_modfl) != 0) return 1;
  feclearexcept(FE_ALL_EXCEPT);
  (void)nelibc_modfl(12.75L, &integral_part);
  if (fetestexcept(FE_INEXACT) != 0) {
    fprintf(stderr, "modfl unexpectedly raised FE_INEXACT\n");
    return 1;
  }
  feraiseexcept(FE_INEXACT);
  (void)nelibc_modfl(12.75L, &integral_part);
  if (fetestexcept(FE_INEXACT) == 0) {
    fprintf(stderr, "modfl cleared a pre-existing FE_INEXACT\n");
    return 1;
  }
  feclearexcept(FE_ALL_EXCEPT);

  if (check_bessel_order(-100, 0, 0.0f) || check_bessel_order(-100, -0, -0.0f)) return 1;
  if (check_bessel_order(-100, 0.01, 0.01f) || check_bessel_order(-100, -0.01, -0.01f)) return 1;
  if (check_bessel_order(-100, 0.5, 0.5f) || check_bessel_order(-100, -0.5, -0.5f)) return 1;
  if (check_bessel_order(-100, 1, 1.0f) || check_bessel_order(-100, -1, -1.0f)) return 1;
  if (check_bessel_order(-100, 1.25, 1.25f) || check_bessel_order(-100, -1.25, -1.25f)) return 1;
  if (check_bessel_order(-100, 2, 2.0f) || check_bessel_order(-100, -2, -2.0f)) return 1;
  if (check_bessel_order(-100, 3.8317059702075125, 3.83170597f) || check_bessel_order(-100, -3.8317059702075125, -3.83170597f)) return 1;
  if (check_bessel_order(-100, 8, 8.0f) || check_bessel_order(-100, -8, -8.0f)) return 1;
  if (check_bessel_order(-100, 11.999999000000001, 11.999999f) || check_bessel_order(-100, -11.999999000000001, -11.999999f)) return 1;
  if (check_bessel_order(-100, 12, 12.0f) || check_bessel_order(-100, -12, -12.0f)) return 1;
  if (check_bessel_order(-100, 20, 20.0f) || check_bessel_order(-100, -20, -20.0f)) return 1;
  if (check_bessel_order(-100, 50, 50.0f) || check_bessel_order(-100, -50, -50.0f)) return 1;
  if (check_bessel_order(-100, 100, 100.0f) || check_bessel_order(-100, -100, -100.0f)) return 1;
  if (check_bessel_order(-100, 1000000, 1000000.0f) || check_bessel_order(-100, -1000000, -1000000.0f)) return 1;
  if (check_bessel_order(-20, 0, 0.0f) || check_bessel_order(-20, -0, -0.0f)) return 1;
  if (check_bessel_order(-20, 0.01, 0.01f) || check_bessel_order(-20, -0.01, -0.01f)) return 1;
  if (check_bessel_order(-20, 0.5, 0.5f) || check_bessel_order(-20, -0.5, -0.5f)) return 1;
  if (check_bessel_order(-20, 1, 1.0f) || check_bessel_order(-20, -1, -1.0f)) return 1;
  if (check_bessel_order(-20, 1.25, 1.25f) || check_bessel_order(-20, -1.25, -1.25f)) return 1;
  if (check_bessel_order(-20, 2, 2.0f) || check_bessel_order(-20, -2, -2.0f)) return 1;
  if (check_bessel_order(-20, 3.8317059702075125, 3.83170597f) || check_bessel_order(-20, -3.8317059702075125, -3.83170597f)) return 1;
  if (check_bessel_order(-20, 8, 8.0f) || check_bessel_order(-20, -8, -8.0f)) return 1;
  if (check_bessel_order(-20, 11.999999000000001, 11.999999f) || check_bessel_order(-20, -11.999999000000001, -11.999999f)) return 1;
  if (check_bessel_order(-20, 12, 12.0f) || check_bessel_order(-20, -12, -12.0f)) return 1;
  if (check_bessel_order(-20, 20, 20.0f) || check_bessel_order(-20, -20, -20.0f)) return 1;
  if (check_bessel_order(-20, 50, 50.0f) || check_bessel_order(-20, -50, -50.0f)) return 1;
  if (check_bessel_order(-20, 100, 100.0f) || check_bessel_order(-20, -100, -100.0f)) return 1;
  if (check_bessel_order(-20, 1000000, 1000000.0f) || check_bessel_order(-20, -1000000, -1000000.0f)) return 1;
  if (check_bessel_order(-5, 0, 0.0f) || check_bessel_order(-5, -0, -0.0f)) return 1;
  if (check_bessel_order(-5, 0.01, 0.01f) || check_bessel_order(-5, -0.01, -0.01f)) return 1;
  if (check_bessel_order(-5, 0.5, 0.5f) || check_bessel_order(-5, -0.5, -0.5f)) return 1;
  if (check_bessel_order(-5, 1, 1.0f) || check_bessel_order(-5, -1, -1.0f)) return 1;
  if (check_bessel_order(-5, 1.25, 1.25f) || check_bessel_order(-5, -1.25, -1.25f)) return 1;
  if (check_bessel_order(-5, 2, 2.0f) || check_bessel_order(-5, -2, -2.0f)) return 1;
  if (check_bessel_order(-5, 3.8317059702075125, 3.83170597f) || check_bessel_order(-5, -3.8317059702075125, -3.83170597f)) return 1;
  if (check_bessel_order(-5, 8, 8.0f) || check_bessel_order(-5, -8, -8.0f)) return 1;
  if (check_bessel_order(-5, 11.999999000000001, 11.999999f) || check_bessel_order(-5, -11.999999000000001, -11.999999f)) return 1;
  if (check_bessel_order(-5, 12, 12.0f) || check_bessel_order(-5, -12, -12.0f)) return 1;
  if (check_bessel_order(-5, 20, 20.0f) || check_bessel_order(-5, -20, -20.0f)) return 1;
  if (check_bessel_order(-5, 50, 50.0f) || check_bessel_order(-5, -50, -50.0f)) return 1;
  if (check_bessel_order(-5, 100, 100.0f) || check_bessel_order(-5, -100, -100.0f)) return 1;
  if (check_bessel_order(-5, 1000000, 1000000.0f) || check_bessel_order(-5, -1000000, -1000000.0f)) return 1;
  if (check_bessel_order(-2, 0, 0.0f) || check_bessel_order(-2, -0, -0.0f)) return 1;
  if (check_bessel_order(-2, 0.01, 0.01f) || check_bessel_order(-2, -0.01, -0.01f)) return 1;
  if (check_bessel_order(-2, 0.5, 0.5f) || check_bessel_order(-2, -0.5, -0.5f)) return 1;
  if (check_bessel_order(-2, 1, 1.0f) || check_bessel_order(-2, -1, -1.0f)) return 1;
  if (check_bessel_order(-2, 1.25, 1.25f) || check_bessel_order(-2, -1.25, -1.25f)) return 1;
  if (check_bessel_order(-2, 2, 2.0f) || check_bessel_order(-2, -2, -2.0f)) return 1;
  if (check_bessel_order(-2, 3.8317059702075125, 3.83170597f) || check_bessel_order(-2, -3.8317059702075125, -3.83170597f)) return 1;
  if (check_bessel_order(-2, 8, 8.0f) || check_bessel_order(-2, -8, -8.0f)) return 1;
  if (check_bessel_order(-2, 11.999999000000001, 11.999999f) || check_bessel_order(-2, -11.999999000000001, -11.999999f)) return 1;
  if (check_bessel_order(-2, 12, 12.0f) || check_bessel_order(-2, -12, -12.0f)) return 1;
  if (check_bessel_order(-2, 20, 20.0f) || check_bessel_order(-2, -20, -20.0f)) return 1;
  if (check_bessel_order(-2, 50, 50.0f) || check_bessel_order(-2, -50, -50.0f)) return 1;
  if (check_bessel_order(-2, 100, 100.0f) || check_bessel_order(-2, -100, -100.0f)) return 1;
  if (check_bessel_order(-2, 1000000, 1000000.0f) || check_bessel_order(-2, -1000000, -1000000.0f)) return 1;
  if (check_bessel_order(-1, 0, 0.0f) || check_bessel_order(-1, -0, -0.0f)) return 1;
  if (check_bessel_order(-1, 0.01, 0.01f) || check_bessel_order(-1, -0.01, -0.01f)) return 1;
  if (check_bessel_order(-1, 0.5, 0.5f) || check_bessel_order(-1, -0.5, -0.5f)) return 1;
  if (check_bessel_order(-1, 1, 1.0f) || check_bessel_order(-1, -1, -1.0f)) return 1;
  if (check_bessel_order(-1, 1.25, 1.25f) || check_bessel_order(-1, -1.25, -1.25f)) return 1;
  if (check_bessel_order(-1, 2, 2.0f) || check_bessel_order(-1, -2, -2.0f)) return 1;
  if (check_bessel_order(-1, 3.8317059702075125, 3.83170597f) || check_bessel_order(-1, -3.8317059702075125, -3.83170597f)) return 1;
  if (check_bessel_order(-1, 8, 8.0f) || check_bessel_order(-1, -8, -8.0f)) return 1;
  if (check_bessel_order(-1, 11.999999000000001, 11.999999f) || check_bessel_order(-1, -11.999999000000001, -11.999999f)) return 1;
  if (check_bessel_order(-1, 12, 12.0f) || check_bessel_order(-1, -12, -12.0f)) return 1;
  if (check_bessel_order(-1, 20, 20.0f) || check_bessel_order(-1, -20, -20.0f)) return 1;
  if (check_bessel_order(-1, 50, 50.0f) || check_bessel_order(-1, -50, -50.0f)) return 1;
  if (check_bessel_order(-1, 100, 100.0f) || check_bessel_order(-1, -100, -100.0f)) return 1;
  if (check_bessel_order(-1, 1000000, 1000000.0f) || check_bessel_order(-1, -1000000, -1000000.0f)) return 1;
  if (check_bessel_order(0, 0, 0.0f) || check_bessel_order(0, -0, -0.0f)) return 1;
  if (check_bessel_order(0, 0.01, 0.01f) || check_bessel_order(0, -0.01, -0.01f)) return 1;
  if (check_bessel_order(0, 0.5, 0.5f) || check_bessel_order(0, -0.5, -0.5f)) return 1;
  if (check_bessel_order(0, 1, 1.0f) || check_bessel_order(0, -1, -1.0f)) return 1;
  if (check_bessel_order(0, 1.25, 1.25f) || check_bessel_order(0, -1.25, -1.25f)) return 1;
  if (check_bessel_order(0, 2, 2.0f) || check_bessel_order(0, -2, -2.0f)) return 1;
  if (check_bessel_order(0, 3.8317059702075125, 3.83170597f) || check_bessel_order(0, -3.8317059702075125, -3.83170597f)) return 1;
  if (check_bessel_order(0, 8, 8.0f) || check_bessel_order(0, -8, -8.0f)) return 1;
  if (check_bessel_order(0, 11.999999000000001, 11.999999f) || check_bessel_order(0, -11.999999000000001, -11.999999f)) return 1;
  if (check_bessel_order(0, 12, 12.0f) || check_bessel_order(0, -12, -12.0f)) return 1;
  if (check_bessel_order(0, 20, 20.0f) || check_bessel_order(0, -20, -20.0f)) return 1;
  if (check_bessel_order(0, 50, 50.0f) || check_bessel_order(0, -50, -50.0f)) return 1;
  if (check_bessel_order(0, 100, 100.0f) || check_bessel_order(0, -100, -100.0f)) return 1;
  if (check_bessel_order(0, 1000000, 1000000.0f) || check_bessel_order(0, -1000000, -1000000.0f)) return 1;
  if (check_bessel_order(1, 0, 0.0f) || check_bessel_order(1, -0, -0.0f)) return 1;
  if (check_bessel_order(1, 0.01, 0.01f) || check_bessel_order(1, -0.01, -0.01f)) return 1;
  if (check_bessel_order(1, 0.5, 0.5f) || check_bessel_order(1, -0.5, -0.5f)) return 1;
  if (check_bessel_order(1, 1, 1.0f) || check_bessel_order(1, -1, -1.0f)) return 1;
  if (check_bessel_order(1, 1.25, 1.25f) || check_bessel_order(1, -1.25, -1.25f)) return 1;
  if (check_bessel_order(1, 2, 2.0f) || check_bessel_order(1, -2, -2.0f)) return 1;
  if (check_bessel_order(1, 3.8317059702075125, 3.83170597f) || check_bessel_order(1, -3.8317059702075125, -3.83170597f)) return 1;
  if (check_bessel_order(1, 8, 8.0f) || check_bessel_order(1, -8, -8.0f)) return 1;
  if (check_bessel_order(1, 11.999999000000001, 11.999999f) || check_bessel_order(1, -11.999999000000001, -11.999999f)) return 1;
  if (check_bessel_order(1, 12, 12.0f) || check_bessel_order(1, -12, -12.0f)) return 1;
  if (check_bessel_order(1, 20, 20.0f) || check_bessel_order(1, -20, -20.0f)) return 1;
  if (check_bessel_order(1, 50, 50.0f) || check_bessel_order(1, -50, -50.0f)) return 1;
  if (check_bessel_order(1, 100, 100.0f) || check_bessel_order(1, -100, -100.0f)) return 1;
  if (check_bessel_order(1, 1000000, 1000000.0f) || check_bessel_order(1, -1000000, -1000000.0f)) return 1;
  if (check_bessel_order(2, 0, 0.0f) || check_bessel_order(2, -0, -0.0f)) return 1;
  if (check_bessel_order(2, 0.01, 0.01f) || check_bessel_order(2, -0.01, -0.01f)) return 1;
  if (check_bessel_order(2, 0.5, 0.5f) || check_bessel_order(2, -0.5, -0.5f)) return 1;
  if (check_bessel_order(2, 1, 1.0f) || check_bessel_order(2, -1, -1.0f)) return 1;
  if (check_bessel_order(2, 1.25, 1.25f) || check_bessel_order(2, -1.25, -1.25f)) return 1;
  if (check_bessel_order(2, 2, 2.0f) || check_bessel_order(2, -2, -2.0f)) return 1;
  if (check_bessel_order(2, 3.8317059702075125, 3.83170597f) || check_bessel_order(2, -3.8317059702075125, -3.83170597f)) return 1;
  if (check_bessel_order(2, 8, 8.0f) || check_bessel_order(2, -8, -8.0f)) return 1;
  if (check_bessel_order(2, 11.999999000000001, 11.999999f) || check_bessel_order(2, -11.999999000000001, -11.999999f)) return 1;
  if (check_bessel_order(2, 12, 12.0f) || check_bessel_order(2, -12, -12.0f)) return 1;
  if (check_bessel_order(2, 20, 20.0f) || check_bessel_order(2, -20, -20.0f)) return 1;
  if (check_bessel_order(2, 50, 50.0f) || check_bessel_order(2, -50, -50.0f)) return 1;
  if (check_bessel_order(2, 100, 100.0f) || check_bessel_order(2, -100, -100.0f)) return 1;
  if (check_bessel_order(2, 1000000, 1000000.0f) || check_bessel_order(2, -1000000, -1000000.0f)) return 1;
  if (check_bessel_order(5, 0, 0.0f) || check_bessel_order(5, -0, -0.0f)) return 1;
  if (check_bessel_order(5, 0.01, 0.01f) || check_bessel_order(5, -0.01, -0.01f)) return 1;
  if (check_bessel_order(5, 0.5, 0.5f) || check_bessel_order(5, -0.5, -0.5f)) return 1;
  if (check_bessel_order(5, 1, 1.0f) || check_bessel_order(5, -1, -1.0f)) return 1;
  if (check_bessel_order(5, 1.25, 1.25f) || check_bessel_order(5, -1.25, -1.25f)) return 1;
  if (check_bessel_order(5, 2, 2.0f) || check_bessel_order(5, -2, -2.0f)) return 1;
  if (check_bessel_order(5, 3.8317059702075125, 3.83170597f) || check_bessel_order(5, -3.8317059702075125, -3.83170597f)) return 1;
  if (check_bessel_order(5, 8, 8.0f) || check_bessel_order(5, -8, -8.0f)) return 1;
  if (check_bessel_order(5, 11.999999000000001, 11.999999f) || check_bessel_order(5, -11.999999000000001, -11.999999f)) return 1;
  if (check_bessel_order(5, 12, 12.0f) || check_bessel_order(5, -12, -12.0f)) return 1;
  if (check_bessel_order(5, 20, 20.0f) || check_bessel_order(5, -20, -20.0f)) return 1;
  if (check_bessel_order(5, 50, 50.0f) || check_bessel_order(5, -50, -50.0f)) return 1;
  if (check_bessel_order(5, 100, 100.0f) || check_bessel_order(5, -100, -100.0f)) return 1;
  if (check_bessel_order(5, 1000000, 1000000.0f) || check_bessel_order(5, -1000000, -1000000.0f)) return 1;
  if (check_bessel_order(20, 0, 0.0f) || check_bessel_order(20, -0, -0.0f)) return 1;
  if (check_bessel_order(20, 0.01, 0.01f) || check_bessel_order(20, -0.01, -0.01f)) return 1;
  if (check_bessel_order(20, 0.5, 0.5f) || check_bessel_order(20, -0.5, -0.5f)) return 1;
  if (check_bessel_order(20, 1, 1.0f) || check_bessel_order(20, -1, -1.0f)) return 1;
  if (check_bessel_order(20, 1.25, 1.25f) || check_bessel_order(20, -1.25, -1.25f)) return 1;
  if (check_bessel_order(20, 2, 2.0f) || check_bessel_order(20, -2, -2.0f)) return 1;
  if (check_bessel_order(20, 3.8317059702075125, 3.83170597f) || check_bessel_order(20, -3.8317059702075125, -3.83170597f)) return 1;
  if (check_bessel_order(20, 8, 8.0f) || check_bessel_order(20, -8, -8.0f)) return 1;
  if (check_bessel_order(20, 11.999999000000001, 11.999999f) || check_bessel_order(20, -11.999999000000001, -11.999999f)) return 1;
  if (check_bessel_order(20, 12, 12.0f) || check_bessel_order(20, -12, -12.0f)) return 1;
  if (check_bessel_order(20, 20, 20.0f) || check_bessel_order(20, -20, -20.0f)) return 1;
  if (check_bessel_order(20, 50, 50.0f) || check_bessel_order(20, -50, -50.0f)) return 1;
  if (check_bessel_order(20, 100, 100.0f) || check_bessel_order(20, -100, -100.0f)) return 1;
  if (check_bessel_order(20, 1000000, 1000000.0f) || check_bessel_order(20, -1000000, -1000000.0f)) return 1;
  if (check_bessel_order(100, 0, 0.0f) || check_bessel_order(100, -0, -0.0f)) return 1;
  if (check_bessel_order(100, 0.01, 0.01f) || check_bessel_order(100, -0.01, -0.01f)) return 1;
  if (check_bessel_order(100, 0.5, 0.5f) || check_bessel_order(100, -0.5, -0.5f)) return 1;
  if (check_bessel_order(100, 1, 1.0f) || check_bessel_order(100, -1, -1.0f)) return 1;
  if (check_bessel_order(100, 1.25, 1.25f) || check_bessel_order(100, -1.25, -1.25f)) return 1;
  if (check_bessel_order(100, 2, 2.0f) || check_bessel_order(100, -2, -2.0f)) return 1;
  if (check_bessel_order(100, 3.8317059702075125, 3.83170597f) || check_bessel_order(100, -3.8317059702075125, -3.83170597f)) return 1;
  if (check_bessel_order(100, 8, 8.0f) || check_bessel_order(100, -8, -8.0f)) return 1;
  if (check_bessel_order(100, 11.999999000000001, 11.999999f) || check_bessel_order(100, -11.999999000000001, -11.999999f)) return 1;
  if (check_bessel_order(100, 12, 12.0f) || check_bessel_order(100, -12, -12.0f)) return 1;
  if (check_bessel_order(100, 20, 20.0f) || check_bessel_order(100, -20, -20.0f)) return 1;
  if (check_bessel_order(100, 50, 50.0f) || check_bessel_order(100, -50, -50.0f)) return 1;
  if (check_bessel_order(100, 100, 100.0f) || check_bessel_order(100, -100, -100.0f)) return 1;
  if (check_bessel_order(100, 1000000, 1000000.0f) || check_bessel_order(100, -1000000, -1000000.0f)) return 1;
  static const double bessel_edges[] = {0x1p-1074, 0x1p-54, 0.1, 0.5, 1.0, 1.25, 2.0, 3.8317059702075125, 5.520078110286311, 8.0, 11.999999, 12.0, 16.0, 20.0, 50.0, 100.0, 1e6, 1e100};
  for (size_t i=0; i<sizeof bessel_edges/sizeof bessel_edges[0]; ++i) {
    double x=bessel_edges[i]; float xf=(float)x;
    if (check_bessel(x,xf) || check_bessel(-x,-xf)) return 1;
  }
  if (check_gamma(0.5, 0.5f) || check_gamma(1.0, 1.0f) || check_gamma(2.0, 2.0f) ||
      check_gamma(5.5, 5.5f) || check_gamma(-0.5, -0.5f) || check_gamma(-1.5, -1.5f) ||
      check_gamma(-2.5, -2.5f) || check_gamma(20.25, 20.25f) || check_gamma(-10.25, -10.25f) ||
      check_gamma(0x1p-50, 0x1p-50f) || check_gamma(-0x1p-50, -0x1p-50f)) return 1;
  if (check_erf(-30.0, -30.0f) || check_erf(-6.0, -6.0f) ||
      check_erf(-2.857142857142857, -2.8571429f) || check_erf(-1.25, -1.25f) ||
      check_erf(-0.84375, -0.84375f) || check_erf(-0.25, -0.25f) ||
      check_erf(-0x1p-1074, -0x1p-149f) || check_erf(-0.0, -0.0f) ||
      check_erf(0.0, 0.0f) || check_erf(0.25, 0.25f) ||
      check_erf(0.84375, 0.84375f) || check_erf(1.0, 1.0f) ||
      check_erf(1.25, 1.25f) || check_erf(2.857142857142857, 2.8571429f) ||
      check_erf(6.0, 6.0f) || check_erf(28.0, 28.0f) || check_erf(30.0, 30.0f) ||
      check_erf(INFINITY, INFINITY) || check_erf(-INFINITY, -INFINITY) ||
      check_erf(NAN, NAN)) return 1;
  static const uint64_t edge64[] = {
      0, UINT64_C(0x8000000000000000), 1, UINT64_C(0x8000000000000001),
      UINT64_C(0x3FDFFFFFFFFFFFFF), UINT64_C(0x3FE0000000000000),
      UINT64_C(0x3FE0000000000001), UINT64_C(0x3FC0000000000000),
      UINT64_C(0x3FF8000000000000), UINT64_C(0x4034000000000000),
      UINT64_C(0x4034000000000001), UINT64_C(0x4086280000000000),
      UINT64_C(0x40862CCCCCCCCCCD), UINT64_C(0x40862E42FEFA39EF),
      UINT64_C(0x4086300000000000),
      UINT64_C(0x3FDC000000000000), UINT64_C(0x3FF2FFFFFFFFFFFF),
      UINT64_C(0x3FF3000000000000), UINT64_C(0x40037FFFFFFFFFFF),
      UINT64_C(0x4003800000000000), UINT64_C(0x4008000000000000),
      UINT64_C(0x4004000000000000), UINT64_C(0xBFDFFFFFFFFFFFFF),
      UINT64_C(0xBFE0000000000000), UINT64_C(0xBFF8000000000000),
      UINT64_C(0x7FEFFFFFFFFFFFFF), UINT64_C(0xFFEFFFFFFFFFFFFF),
      UINT64_C(0x7FF0000000000000), UINT64_C(0xFFF0000000000000),
      UINT64_C(0x7FF8000000000001), UINT64_C(0xFFF8000000000001),
  };
  static const uint32_t edge32[] = {
      0, UINT32_C(0x80000000), 1, UINT32_C(0x80000001),
      UINT32_C(0x3EFFFFFF), UINT32_C(0x3F000000), UINT32_C(0x3F000001),
      UINT32_C(0x3E000000), UINT32_C(0x40200000), UINT32_C(0x41A00000),
      UINT32_C(0x41A00001), UINT32_C(0x42B16666), UINT32_C(0x42B1999A),
      UINT32_C(0x42B20000), UINT32_C(0x3EE00000), UINT32_C(0x3F980000),
      UINT32_C(0x401C0000), UINT32_C(0x40400000), UINT32_C(0xBEFFFFFF),
      UINT32_C(0xBF000000), UINT32_C(0xBFC00000), UINT32_C(0x7F7FFFFF),
      UINT32_C(0xFF7FFFFF), UINT32_C(0x7F800000), UINT32_C(0xFF800000),
      UINT32_C(0x7FC00001), UINT32_C(0xFFC00001),
  };
  static const uint64_t pair_edges[][2] = {
      {UINT64_C(0x4014000000000000), UINT64_C(0x4000000000000000)}, /* 5, 2 */
      {UINT64_C(0x401C000000000000), UINT64_C(0x4000000000000000)}, /* 7, 2 */
      {UINT64_C(0xC014000000000000), UINT64_C(0x4000000000000000)}, /* -5, 2 */
      {UINT64_C(0xC014000000000000), UINT64_C(0xC000000000000000)}, /* -5, -2 */
      {UINT64_C(0x0000000000000001), UINT64_C(0x0000000000000002)},
      {UINT64_C(0x7FEFFFFFFFFFFFFF), UINT64_C(0x0000000000000001)},
      {UINT64_C(0xFFEFFFFFFFFFFFFF), UINT64_C(0x0000000000000001)},
      {UINT64_C(0x7FEFFFFFFFFFFFFF), UINT64_C(0x0010000000000000)},
      {UINT64_C(0x8000000000000000), UINT64_C(0x0000000000000000)},
      {UINT64_C(0x7FF0000000000000), UINT64_C(0x4000000000000000)},
      {UINT64_C(0x4010000000000000), UINT64_C(0x7FF0000000000000)},
      {UINT64_C(0x4010000000000000), UINT64_C(0x0000000000000000)},
      {UINT64_C(0x7FF8000000000001), UINT64_C(0x4010000000000000)},
      {UINT64_C(0x7FF0000000000042), UINT64_C(0x4010000000000000)},
      {UINT64_C(0x0000000000000000), UINT64_C(0x8000000000000000)},
      {UINT64_C(0x7FF0000000000000), UINT64_C(0x7FF0000000000000)},
      {UINT64_C(0x7FF0000000000000), UINT64_C(0xFFF0000000000000)},
      {UINT64_C(0xFFF0000000000000), UINT64_C(0x7FF0000000000000)},
      {UINT64_C(0xFFF0000000000000), UINT64_C(0xFFF0000000000000)},
      {UINT64_C(0xC000000000000000), UINT64_C(0x4008000000000000)}, /* (-2)^3 */
      {UINT64_C(0xC000000000000000), UINT64_C(0x4004000000000000)}, /* negative fractional base */
      {UINT64_C(0x0000000000000000), UINT64_C(0xBFF0000000000000)}, /* zero to negative */
      {UINT64_C(0x8000000000000000), UINT64_C(0x4008000000000000)}, /* negative zero, odd */
  };
  static const uint32_t pair_edges32[][2] = {
      {UINT32_C(0x40A00000), UINT32_C(0x40000000)}, /* 5, 2 */
      {UINT32_C(0x40E00000), UINT32_C(0x40000000)}, /* 7, 2 */
      {UINT32_C(0xC0A00000), UINT32_C(0x40000000)}, /* -5, 2 */
      {UINT32_C(0xC0A00000), UINT32_C(0xC0000000)}, /* -5, -2 */
      {UINT32_C(0x00000001), UINT32_C(0x00000002)},
      {UINT32_C(0x7F7FFFFF), UINT32_C(0x00000001)},
      {UINT32_C(0xFF7FFFFF), UINT32_C(0x00000001)},
      {UINT32_C(0x7F7FFFFF), UINT32_C(0x00800000)},
      {UINT32_C(0x80000000), UINT32_C(0x00000000)},
      {UINT32_C(0x7F800000), UINT32_C(0x40000000)},
      {UINT32_C(0x40800000), UINT32_C(0x7F800000)},
      {UINT32_C(0x40800000), UINT32_C(0x00000000)},
      {UINT32_C(0x7FC00001), UINT32_C(0x40800000)},
      {UINT32_C(0x7F800042), UINT32_C(0x40800000)},
      {UINT32_C(0x00000000), UINT32_C(0x80000000)},
      {UINT32_C(0x7F800000), UINT32_C(0x7F800000)},
      {UINT32_C(0x7F800000), UINT32_C(0xFF800000)},
      {UINT32_C(0xFF800000), UINT32_C(0x7F800000)},
      {UINT32_C(0xFF800000), UINT32_C(0xFF800000)},
      {UINT32_C(0xC0000000), UINT32_C(0x40400000)}, /* (-2)^3 */
      {UINT32_C(0xC0000000), UINT32_C(0x40200000)}, /* negative fractional base */
      {UINT32_C(0x00000000), UINT32_C(0xBF800000)}, /* zero to negative */
      {UINT32_C(0x80000000), UINT32_C(0x40400000)}, /* negative zero, odd */
  };
  for (size_t i = 0; i < sizeof edge64 / sizeof edge64[0]; ++i)
    if (check_double(edge64[i]) || check_exp_log(((double_bits){.bits = edge64[i]}).value,
                                                 (float)((double_bits){.bits = edge64[i]}).value)) return 1;
  for (size_t i = 0; i < sizeof edge32 / sizeof edge32[0]; ++i)
    if (check_float(edge32[i]) || check_exp_log((double)((float_bits){.bits = edge32[i]}).value,
                                                ((float_bits){.bits = edge32[i]}).value)) return 1;
  for (size_t i = 0; i < sizeof pair_edges / sizeof pair_edges[0]; ++i)
    if (check_pair_double(pair_edges[i][0], pair_edges[i][1])) return 1;
  for (size_t i = 0; i < sizeof pair_edges32 / sizeof pair_edges32[0]; ++i)
    if (check_pair_float(pair_edges32[i][0], pair_edges32[i][1])) return 1;
  if (check_math_extensions()) return 1;
  const double round_values[] = {
      0.0, -0.0, 0.1, -0.1, 0.5, -0.5, 1.5, -1.5,
      4503599627370495.5, -4503599627370495.5,
      0x1p-1074, -0x1p-1074, INFINITY, -INFINITY, NAN,
  };
  const float round_valuesf[] = {
      0.0f, -0.0f, 0.1f, -0.1f, 0.5f, -0.5f, 1.5f, -1.5f,
      8388607.5f, -8388607.5f, 0x1p-149f, -0x1p-149f,
      INFINITY, -INFINITY, NAN,
  };
  static const int rounding_modes[] = {
      FE_TONEAREST, FE_DOWNWARD, FE_UPWARD, FE_TOWARDZERO,
  };
  double_bits signaling64 = {.bits = UINT64_C(0x7FF0000000000042)};
  float_bits signaling32 = {.bits = UINT32_C(0x7F800042)};
  for (size_t mode = 0; mode < sizeof rounding_modes / sizeof rounding_modes[0]; ++mode)
    for (size_t i = 0; i < sizeof round_values / sizeof round_values[0]; ++i)
      if (check_rint_mode(rounding_modes[mode], round_values[i], round_valuesf[i]))
        return 1;
  for (size_t mode = 0; mode < sizeof rounding_modes / sizeof rounding_modes[0]; ++mode)
    if (check_rint_mode(rounding_modes[mode], signaling64.value, signaling32.value))
      return 1;
  if (check_integer_rounding_range()) {
    fprintf(stderr, "integer rounding range or FE_INVALID behavior mismatch\n");
    return 1;
  }
  if (fesetround(FE_TONEAREST) != 0) return 1;
  static const double exp_edges[] = {
      -746.0, -745.1332191019411, -100.0, -1.0, 0.0, 1.0, 20.0,
      700.0, 709.0, 709.782712893384, 710.0,
      0x1.62e42fefa39efp-1, -0x1.62e42fefa39efp-1,
  };
  static const double inverse_trig_edges[] = {
      -INFINITY, -1.0000000000000002, -1.0, -0x1.fffffffffffffp-1,
      -0.75, -0.5, -0x1p-52, -0.0, 0.0, 0x1p-52, 0.5, 0.75,
      0x1.fffffffffffffp-1, 1.0, 1.0000000000000002, INFINITY, NAN,
  };
  for (size_t i = 0; i < sizeof inverse_trig_edges / sizeof inverse_trig_edges[0]; ++i)
    if (check_inverse_trig(inverse_trig_edges[i], (float)inverse_trig_edges[i])) return 1;
  for (size_t i = 0; i < 100000; ++i) {
    const double value = 2.0 * (double)(next_bits() >> 11) * 0x1p-53 - 1.0;
    const float valuef = 2.0f * (float)(next_bits() >> 40) * 0x1p-24f - 1.0f;
    if (check_inverse_trig(value, valuef)) return 1;
  }
  static const double fma_edges[][3] = {
      {0x1.0000000000001p0, 0x1.ffffffffffffep-1, -1.0},
      {0x1p-1022, 0.5, -0x1p-1023},
      {0x1.fffffffffffffp1023, 2.0, -0x1.fffffffffffffp1023},
      {-0.0, 1.0, -0.0},
  };
  for (size_t i = 0; i < sizeof fma_edges / sizeof fma_edges[0]; ++i)
    if (check_fma(fma_edges[i][0], fma_edges[i][1], fma_edges[i][2],
                  (float)fma_edges[i][0], (float)fma_edges[i][1],
                  (float)fma_edges[i][2])) return 1;
  const long double fmal_edges[][3] = {
      {1.0L + 0x1p-63L, 1.0L - 0x1p-63L, -1.0L},
      {LDBL_MAX, 2.0L, -LDBL_MAX}, {LDBL_MAX, 2.0L, 0.0L},
      {0x1p-16445L, 0.5L, 0.0L}, {-0x1p-16445L, 0.5L, 0.0L},
      {LDBL_MIN, 0.5L, 0.0L},
      {LDBL_MIN, 1.0L - 0x1p-64L, 0.0L},
      {1.0L, 1.0L, -1.0L},
      {INFINITY, 1.0L, -INFINITY}, {0.0L, INFINITY, 1.0L},
  };
  const int fmal_rounding_modes[] = {
      FE_TONEAREST, FE_DOWNWARD, FE_UPWARD, FE_TOWARDZERO};
  for (size_t mode = 0; mode < sizeof fmal_rounding_modes / sizeof fmal_rounding_modes[0]; ++mode) {
    if (fesetround(fmal_rounding_modes[mode]) != 0) return 1;
    for (size_t i = 0; i < sizeof fmal_edges / sizeof fmal_edges[0]; ++i)
      if (check_fmal(fmal_edges[i][0], fmal_edges[i][1], fmal_edges[i][2])) return 1;
    for (size_t i = 0; i < 32; ++i) {
      const long double random_x = 1.0L + (long double)(next_bits() & UINT64_C(0x7FFFFFFFFFFFFFFF)) * 0x1p-63L;
      const long double random_y = 1.0L + (long double)(next_bits() & UINT64_C(0x7FFFFFFFFFFFFFFF)) * 0x1p-63L;
      const long double random_z = 1.0L + (long double)(next_bits() & UINT64_C(0x7FFFFFFFFFFFFFFF)) * 0x1p-63L;
      const int exponent_x = (int)(next_bits() % 30001) - 15000;
      const int exponent_y = (int)(next_bits() % 30001) - 15000;
      const int exponent_z = (int)(next_bits() % 30001) - 15000;
      long double x_value = ldexpl(random_x, exponent_x);
      long double y_value = ldexpl(random_y, exponent_y);
      long double z_value = ldexpl(random_z, exponent_z);
      if (next_bits() & 1) x_value = -x_value;
      if (next_bits() & 1) y_value = -y_value;
      if (next_bits() & 1) z_value = -z_value;
      if (check_fmal(x_value, y_value, z_value)) return 1;
    }
  }
  if (fesetround(FE_TONEAREST) != 0) return 1;
  for (size_t i = 0; i < sizeof exp_edges / sizeof exp_edges[0]; ++i)
    if (check_exp_log(exp_edges[i], (float)exp_edges[i])) return 1;
  const double log10_edges[] = {
      0.0, 0x1p-1074, nextafter(DBL_MIN, 0.0), DBL_MIN, 1e-300, 1e-100,
      1e-10, 0.1, nextafter(1.0, 0.0), 1.0, nextafter(1.0, 2.0),
      nextafter(10.0, 0.0), 10.0, nextafter(10.0, INFINITY), 100.0,
      1e100, 1e300, DBL_MAX,
  };
  for (size_t i = 0; i < sizeof log10_edges / sizeof log10_edges[0]; ++i)
    if (check_log10_edge(log10_edges[i])) return 1;
  const double inverse_hyperbolic_edges[] = {
      -1.0, nextafter(-1.0, 0.0), -0.5, -DBL_MIN, -0.0, 0.0,
      DBL_MIN, 0.5, nextafter(1.0, 0.0), 1.0, nextafter(1.0, 2.0),
      2.0, DBL_MAX,
  };
  for (size_t i = 0; i < sizeof inverse_hyperbolic_edges / sizeof inverse_hyperbolic_edges[0]; ++i) {
    const double value = inverse_hyperbolic_edges[i];
    if (check_exp_log(value, (float)value)) return 1;
  }
  if (check_exp_log(nextafter(1.0, 0.0), nextafterf(1.0f, 0.0f)) ||
      check_exp_log(nextafter(1.0, 2.0), nextafterf(1.0f, 2.0f)) ||
      check_exp_log(DBL_MAX, FLT_MAX) ||
      check_exp_log(DBL_MIN, FLT_MIN) ||
      check_exp_log(0x1p-1074, 0x1p-149f)) return 1;
  for (size_t i = 0; i < 100000; ++i) {
    const uint64_t bits = next_bits();
    uint64_t other = next_bits();
    uint64_t pair_left64 = bits;
    uint32_t left32 = (uint32_t)(bits >> 17);
    uint32_t right32 = (uint32_t)(other >> 19);
    /* Keep random NaNs quiet; signaling-NaN result/exception policy varies by libm. */
    if ((bits & UINT64_C(0x7FF0000000000000)) == UINT64_C(0x7FF0000000000000) &&
        (bits & UINT64_C(0x000FFFFFFFFFFFFF)) != 0)
      left32 = (left32 & UINT32_C(0x803FFFFF)) | UINT32_C(0x7FC00000);
    if ((pair_left64 & UINT64_C(0x7FF0000000000000)) == UINT64_C(0x7FF0000000000000) &&
        (pair_left64 & UINT64_C(0x000FFFFFFFFFFFFF)) != 0)
      pair_left64 |= UINT64_C(0x0008000000000000);
    if ((other & UINT64_C(0x7FF0000000000000)) == UINT64_C(0x7FF0000000000000) &&
        (other & UINT64_C(0x000FFFFFFFFFFFFF)) != 0)
      other |= UINT64_C(0x0008000000000000);
    if ((left32 & UINT32_C(0x7F800000)) == UINT32_C(0x7F800000) &&
        (left32 & UINT32_C(0x007FFFFF)) != 0)
      left32 |= UINT32_C(0x00400000);
    if ((right32 & UINT32_C(0x7F800000)) == UINT32_C(0x7F800000) &&
        (right32 & UINT32_C(0x007FFFFF)) != 0)
      right32 |= UINT32_C(0x00400000);
    if (check_double(bits) || check_float((uint32_t)(bits >> 17)) ||
        check_fma(((double_bits){.bits = bits}).value,
                  ((double_bits){.bits = other}).value,
                  ((double_bits){.bits = next_bits()}).value,
                  ((float_bits){.bits = (uint32_t)(bits >> 17)}).value,
                  ((float_bits){.bits = left32}).value,
                  ((float_bits){.bits = right32}).value) ||
        check_inverse_trig(((double_bits){.bits = bits}).value,
                           ((float_bits){.bits = (uint32_t)(bits >> 17)}).value) ||
        check_exp_log(((double_bits){.bits = bits}).value,
                      ((float_bits){.bits = (uint32_t)(bits >> 17)}).value) ||
        check_pair_double(pair_left64, other) ||
        check_pair_float(left32, right32) ||
        check_gamma((((double_bits){.bits = bits}).value / DBL_MAX) * 20.0,
                   (((float_bits){.bits = left32}).value / FLT_MAX) * 20.0f) ||
        ((i % 100 == 0) && check_bessel_order((int)(next_bits()%41)-20,
          ((((bits & UINT64_C(0x000FFFFFFFFFFFFF)) / (double)UINT64_C(0x0010000000000000)) * 30.0)),
          (float)((((left32 & UINT32_C(0x007FFFFF)) / (float)UINT32_C(0x00800000)) * 30.0f)))) ||
        check_bessel(((((bits & UINT64_C(0x000FFFFFFFFFFFFF)) / (double)UINT64_C(0x0010000000000000)) * 30.0) * ((bits >> 63) ? -1.0 : 1.0)),
                     (float)((((left32 & UINT32_C(0x007FFFFF)) / (float)UINT32_C(0x00800000)) * 30.0f) * ((left32 >> 31) ? -1.0f : 1.0f))) ||
        check_erf((((double_bits){.bits = bits}).value / DBL_MAX) * 28.0,
                  (((float_bits){.bits = (uint32_t)(bits >> 17)}).value / FLT_MAX) * 28.0f)) return 1;
  }
  double_bits infinity = {.bits = UINT64_C(0x7FF0000000000000)};
  const long double above_double = 1.0L + 0x1p-60L;
  const long double below_double = 1.0L - 0x1p-60L;
  const long double above_float = 1.0L + 0x1p-30L;
  const long double below_float = 1.0L - 0x1p-30L;
  if (!equal_double(nexttoward(1.0, above_double), nelibc_nexttoward(1.0, above_double)) ||
      !equal_double(nexttoward(1.0, below_double), nelibc_nexttoward(1.0, below_double)) ||
      !equal_float(nexttowardf(1.0f, above_float), nelibc_nexttowardf(1.0f, above_float)) ||
      !equal_float(nexttowardf(1.0f, below_float), nelibc_nexttowardf(1.0f, below_float))) {
    fprintf(stderr, "nexttoward lost the long-double direction\n");
    return 1;
  }
  if (check_unary_errno("sqrt", sqrt, nelibc_sqrt, -1.0) ||
      check_unary_errno("sin-infinity", sin, nelibc_sin, infinity.value) ||
      check_unary_errno("cos-infinity", cos, nelibc_cos, infinity.value) ||
      check_unary_errno("tan-infinity", tan, nelibc_tan, infinity.value) ||
      check_errno_value("log-zero", log, nelibc_log, 0.0, ERANGE) ||
      check_errno_value("log-negative", log, nelibc_log, -1.0, EDOM) ||
      check_errno_value("log10-zero", log10, nelibc_log10, 0.0, ERANGE) ||
      check_errno_value("log10-negative", log10, nelibc_log10, -1.0, EDOM) ||
      check_errno_value("exp-overflow", exp, nelibc_exp, 1000.0, ERANGE) ||
      check_errno_value("exp-underflow", exp, nelibc_exp, -1000.0, ERANGE) ||
      check_errno_value("sinh-overflow", sinh, nelibc_sinh, 1000.0, ERANGE) ||
      check_errno_value("cosh-overflow", cosh, nelibc_cosh, 1000.0, ERANGE) ||
      check_float_errno_value("logf-zero", logf, nelibc_logf, 0.0f, ERANGE) ||
      check_float_errno_value("logf-negative", logf, nelibc_logf, -1.0f, EDOM) ||
      check_float_errno_value("expf-overflow", expf, nelibc_expf, 100.0f, ERANGE) ||
      check_float_errno_value("expf-underflow", expf, nelibc_expf, -100.0f, ERANGE) ||
      check_float_errno_value("sinhf-overflow", sinhf, nelibc_sinhf, 100.0f, ERANGE) ||
      check_float_errno_value("coshf-overflow", coshf, nelibc_coshf, 100.0f, ERANGE) ||
      check_errno_value("acosh-domain", acosh, nelibc_acosh, 0.5, EDOM) ||
      check_errno_value("atanh-pole", atanh, nelibc_atanh, 1.0, ERANGE) ||
      check_errno_value("atanh-domain", atanh, nelibc_atanh, 2.0, EDOM) ||
      check_errno_value("asin-domain", asin, nelibc_asin, 2.0, EDOM) ||
      check_errno_value("acos-domain", acos, nelibc_acos, -2.0, EDOM) ||
      check_float_errno_value("acoshf-domain", acoshf, nelibc_acoshf, 0.5f, EDOM) ||
      check_float_errno_value("atanhf-pole", atanhf, nelibc_atanhf, 1.0f, ERANGE) ||
      check_float_errno_value("atanhf-domain", atanhf, nelibc_atanhf, 2.0f, EDOM) ||
      check_float_errno_value("asinf-domain", asinf, nelibc_asinf, 2.0f, EDOM) ||
      check_float_errno_value("acosf-domain", acosf, nelibc_acosf, -2.0f, EDOM) ||
      check_unary_flags("sin-infinity", sin, nelibc_sin, infinity.value) ||
      check_unary_flags("cos-infinity", cos, nelibc_cos, infinity.value) ||
      check_unary_flags("tan-infinity", tan, nelibc_tan, infinity.value) ||
      check_unary_flags("log-zero", log, nelibc_log, 0.0) ||
      check_unary_flags("log-negative", log, nelibc_log, -1.0) ||
      check_unary_flags("exp-overflow", exp, nelibc_exp, 1000.0) ||
      check_unary_flags("exp-underflow", exp, nelibc_exp, -1000.0) ||
      check_float_flags("logf-zero", logf, nelibc_logf, 0.0f) ||
      check_float_flags("logf-negative", logf, nelibc_logf, -1.0f) ||
      check_float_flags("expf-overflow", expf, nelibc_expf, 100.0f) ||
      check_float_flags("expf-underflow", expf, nelibc_expf, -100.0f) ||
      check_binary_errno("fmod-zero", fmod, nelibc_fmod, 1.0, 0.0) ||
      check_binary_errno("fmod-infinity", fmod, nelibc_fmod, infinity.value, 2.0) ||
      check_binary_errno("remainder-zero", remainder, nelibc_remainder, 1.0, 0.0) ||
      check_binary_errno("remainder-infinity", remainder, nelibc_remainder, infinity.value, 2.0) ||
      check_binary_errno_value("pow-domain", pow, nelibc_pow, -2.0, 0.5, EDOM) ||
      check_binary_errno_value("pow-zero-pole", pow, nelibc_pow, 0.0, -2.0, ERANGE) ||
      check_binary_errno_value("pow-overflow", pow, nelibc_pow, 2.0, 1024.0, ERANGE) ||
      check_binary_errno_value("pow-underflow", pow, nelibc_pow, 2.0, -1075.0, ERANGE) ||
      check_binary_float_errno_value("powf-overflow", powf, nelibc_powf, 2.0f, 128.0f, ERANGE) ||
      check_binary_float_errno_value("powf-underflow", powf, nelibc_powf, 2.0f, -150.0f, ERANGE))
    return 1;
  double atanh_edge = 1.0;
  float atanhf_edge = 1.0f;
  for (unsigned int i = 0; i < 32; ++i) {
    atanh_edge = nextafter(atanh_edge, 0.0);
    atanhf_edge = nextafterf(atanhf_edge, 0.0f);
    for (int sign = -1; sign <= 1; sign += 2) {
      double x = sign < 0 ? -atanh_edge : atanh_edge;
      float xf = sign < 0 ? -atanhf_edge : atanhf_edge;
      double expected = atanh(x), actual = nelibc_atanh(x);
      float expectedf = atanhf(xf), actualf = nelibc_atanhf(xf);
      if (ulps_double(expected, actual) > 8 || ulps_float(expectedf, actualf) > 8) {
        fprintf(stderr, "atanh endpoint mismatch step=%u sign=%d double %.17g/%.17g float %.9g/%.9g\n",
                i + 1, sign, expected, actual, expectedf, actualf);
        return 1;
      }
    }
  }
  printf("116 binary32/binary64 core math functions matched; erf/erfc max ULP: %llu/%llu (float %u/%u); exp/log max ULP: %.0f/%.0f (float %.0f/%.0f); sin/cos/tan max ULP: %llu/%llu/%llu (float %u/%u/%u); pow max ULP: %llu (float %u); cbrt max ULP: %llu (float %u); hypot <=4 ULP; edge and 100,000 generated inputs\n",
         (unsigned long long)max_erf_ulps, (unsigned long long)max_erfc_ulps, max_erff_ulps, max_erfcf_ulps,
         (double)max_exp_ulps, (double)max_log_ulps,
         (double)max_expf_ulps, (double)max_logf_ulps,
         (unsigned long long)max_sin_ulps, (unsigned long long)max_cos_ulps,
         (unsigned long long)max_tan_ulps,
         max_sinf_ulps, max_cosf_ulps, max_tanf_ulps,
         (unsigned long long)max_pow_ulps, max_powf_ulps,
         (unsigned long long)max_cbrt_ulps, max_cbrtf_ulps);
  return 0;
}
