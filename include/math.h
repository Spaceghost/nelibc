#ifndef NELIBC_MATH_WRAPPER_H
#define NELIBC_MATH_WRAPPER_H

#ifdef NELIBC_STANDALONE

#include <float.h>
extern int signgam;

float fabsf(float value);
double fabs(double value);
long double fabsl(long double value);
float copysignf(float magnitude, float sign);
double copysign(double magnitude, double sign);
long double copysignl(long double magnitude, long double sign);
float truncf(float value);
double trunc(double value);
float floorf(float value);
double floor(double value);
float ceilf(float value);
double ceil(double value);
float roundf(float value);
double round(double value);
float rintf(float value);
double rint(double value);
long lrintf(float value);
long lrint(double value);
long long llrintf(float value);
long long llrint(double value);
long lroundf(float value);
long lround(double value);
long long llroundf(float value);
long long llround(double value);
float nearbyintf(float value);
double nearbyint(double value);
float expf(float value);
double exp(double value);
float exp2f(float value);
double exp2(double value);
float exp10f(float value);
double exp10(double value);
float pow10f(float value);
double pow10(double value);
float nanf(const char *tag);
double nan(const char *tag);
int finitef(float value);
int finite(double value);
float dremf(float x, float y);
double drem(double x, double y);
float expm1f(float value);
double expm1(double value);
float powf(float value, float exponent);
double pow(double value, double exponent);
float sinhf(float value);
double sinh(double value);
float coshf(float value);
double cosh(double value);
float tanhf(float value);
double tanh(double value);
float asinhf(float value);
double asinh(double value);
float acoshf(float value);
double acosh(double value);
float atanhf(float value);
double atanh(double value);
float atanf(float value);
double atan(double value);
float atan2f(float y, float x);
double atan2(double y, double x);
float asinf(float value);
double asin(double value);
float acosf(float value);
double acos(double value);
float fmaf(float x, float y, float z);
double fma(double x, double y, double z);
float erff(float value);
double erf(double value);
float erfcf(float value);
double erfc(double value);
float jnf(int order, float value);
double jn(int order, double value);
float ynf(int order, float value);
double yn(int order, double value);
float j0f(float value);
double j0(double value);
float j1f(float value);
double j1(double value);
float y0f(float value);
double y0(double value);
float y1f(float value);
double y1(double value);
float tgammaf(float value);
double tgamma(double value);
float lgammaf(float value);
double lgamma(double value);
float lgammaf_r(float value, int *sign);
double lgamma_r(double value, int *sign);
float erff(float value);
double erf(double value);
float erfcf(float value);
double erfc(double value);
float jnf(int order, float value);
double jn(int order, double value);
float ynf(int order, float value);
double yn(int order, double value);
float j0f(float value);
double j0(double value);
float j1f(float value);
double j1(double value);
float y0f(float value);
double y0(double value);
float y1f(float value);
double y1(double value);
float tgammaf(float value);
double tgamma(double value);
float lgammaf(float value);
double lgamma(double value);
float lgammaf_r(float value, int *sign);
double lgamma_r(double value, int *sign);
float logf(float value);
double log(double value);
float log2f(float value);
double log2(double value);
float log10f(float value);
double log10(double value);
float log1pf(float value);
double log1p(double value);
float cbrtf(float value);
double cbrt(double value);
long double cbrtl(long double value);
float sinf(float value);
double sin(double value);
void sincosf(float value, float *sine, float *cosine);
void sincos(double value, double *sine, double *cosine);
float cosf(float value);
double cos(double value);
float tanf(float value);
double tan(double value);
float hypotf(float x, float y);
double hypot(double x, double y);
long double hypotl(long double x, long double y);
long double nextafterl(long double x, long double y);
long double atanl(long double value);
long double atan2l(long double y, long double x);
long double asinl(long double value);
long double acosl(long double value);
long double logl(long double value);
long double log2l(long double value);
long double log10l(long double value);
long double log1pl(long double value);
long double expl(long double value);
long double exp2l(long double value);
long double expm1l(long double value);
long double sinl(long double value);
long double cosl(long double value);
long double tanl(long double value);
long double tanhl(long double value);
long double sinhl(long double value);
long double coshl(long double value);
long double asinhl(long double value);
long double acoshl(long double value);
long double atanhl(long double value);
long double powl(long double value, long double exponent);
long double nanl(const char *tag);
void sincosl(long double value, long double *sine, long double *cosine);
float sqrtf(float value);
double sqrt(double value);
long double sqrtl(long double value);
long double fmodl(long double x, long double y);
long double remainderl(long double x, long double y);
float fmodf(float x, float y);
double fmod(double x, double y);
long double fmodl(long double x, long double y);
float remainderf(float x, float y);
double remainder(double x, double y);
long double remainderl(long double x, long double y);
float remquof(float x, float y, int *quotient);
double remquo(double x, double y, int *quotient);
float fminf(float x, float y);
double fmin(double x, double y);
long double fminl(long double x, long double y);
float fmaxf(float x, float y);
double fmax(double x, double y);
long double fmaxl(long double x, long double y);
float fdimf(float x, float y);
double fdim(double x, double y);
long double fdiml(long double x, long double y);
float frexpf(float value, int *exponent);
double frexp(double value, int *exponent);
long double frexpl(long double value, int *exponent);
float modff(float value, float *integral);
double modf(double value, double *integral);
long double modfl(long double value, long double *integral);
float ldexpf(float value, int exponent);
double ldexp(double value, int exponent);
float scalbnf(float value, int exponent);
double scalbn(double value, int exponent);
float scalblnf(float value, long exponent);
double scalbln(double value, long exponent);
int ilogbf(float value);
int ilogb(double value);
int ilogbl(long double value);
float logbf(float value);
double logb(double value);
long double logbl(long double value);
long double ldexpl(long double value, int exponent);
long double scalbnl(long double value, int exponent);
long double scalblnl(long double value, long exponent);
long double truncl(long double value);
long double floorl(long double value);
long double ceill(long double value);
long double rintl(long double value);
long double nearbyintl(long double value);
long double roundl(long double value);
long lrintl(long double value);
long long llrintl(long double value);
long lroundl(long double value);
long long llroundl(long double value);
float nextafterf(float from, float to);
double nextafter(double from, double to);
float nexttowardf(float from, long double to);
double nexttoward(double from, long double to);

#define FP_NAN 0
#define FP_INFINITE 1
#define FP_ZERO 2
#define FP_SUBNORMAL 3
#define FP_NORMAL 4
#define FP_ILOGB0 (-2147483647 - 1)
#define FP_ILOGBNAN (-2147483647 - 1)
#define isnan(value) ((value) != (value))
#define isinf(value) (((value) == (value)) && (value) != 0 && ((value) - (value)) != 0)
#define isfinite(value) (((value) == (value)) && !isinf(value))
#define fpclassify(value) \
  (isnan(value) ? FP_NAN : isinf(value) ? FP_INFINITE : \
   (value) == 0 ? FP_ZERO : \
   (sizeof(value) == sizeof(float) ? \
    ((value) < FLT_MIN && (value) > -FLT_MIN ? FP_SUBNORMAL : FP_NORMAL) : \
    sizeof(value) == sizeof(long double) ? \
    ((value) < LDBL_MIN && (value) > -LDBL_MIN ? FP_SUBNORMAL : FP_NORMAL) : \
    ((value) < DBL_MIN && (value) > -DBL_MIN ? FP_SUBNORMAL : FP_NORMAL)))
#if defined(__TINYC__)
#define signbit(value) ((value) < 0 || ((value) == 0 && (1.0 / (value)) < 0))
#else
#define signbit(value) __builtin_signbit(value)
#endif

#else

#include_next <math.h>
/* GNU math extensions implemented by Nelibc but not exposed by every
   system header feature-test configuration. */
float exp10f(float value);
double exp10(double value);
float pow10f(float value);
double pow10(double value);
int finitef(float value);
int finite(double value);
float dremf(float x, float y);
double drem(double x, double y);
void sincosf(float value, float *sine, float *cosine);
void sincos(double value, double *sine, double *cosine);

#endif

#endif
