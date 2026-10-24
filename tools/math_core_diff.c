#define _GNU_SOURCE
#include <math.h>
#include <errno.h>
#include <fenv.h>
#include <float.h>
#include <stdint.h>
#include <stdio.h>

extern float nelibc_fabsf(float);
extern double nelibc_fabs(double);
extern float nelibc_copysignf(float, float);
extern double nelibc_copysign(double, double);
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
extern float nelibc_nearbyintf(float);
extern double nelibc_nearbyint(double);
extern float nelibc_expf(float);
extern double nelibc_exp(double);
extern float nelibc_exp2f(float);
extern double nelibc_exp2(double);
extern float nelibc_exp10f(float);
extern double nelibc_exp10(double);
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
extern float nelibc_erff(float);
extern double nelibc_erf(double);
extern float nelibc_erfcf(float);
extern double nelibc_erfc(double);
extern float nelibc_sinf(float);
extern double nelibc_sin(double);
extern float nelibc_cosf(float);
extern double nelibc_cos(double);
extern float nelibc_tanf(float);
extern double nelibc_tan(double);
extern float nelibc_hypotf(float, float);
extern double nelibc_hypot(double, double);
extern float nelibc_sqrtf(float);
extern double nelibc_sqrt(double);
extern float nelibc_fmodf(float, float);
extern double nelibc_fmod(double, double);
extern float nelibc_remainderf(float, float);
extern double nelibc_remainder(double, double);
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
      !equal_double(scalbn(value, scale), nelibc_scalbn(value, scale))) {
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
      !equal_float(scalbnf(value, scale), nelibc_scalbnf(value, scale))) {
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

int main(void) {
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
  printf("100 binary32/binary64 core math functions matched; erf/erfc max ULP: %llu/%llu (float %u/%u); exp/log max ULP: %.0f/%.0f (float %.0f/%.0f); sin/cos/tan max ULP: %llu/%llu/%llu (float %u/%u/%u); pow max ULP: %llu (float %u); cbrt max ULP: %llu (float %u); hypot <=4 ULP; edge and 100,000 generated inputs\n",
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
