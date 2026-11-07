#ifndef NELIBC_MATH_WRAPPER_H
#define NELIBC_MATH_WRAPPER_H

#ifdef NELIBC_STANDALONE

#include <float.h>
extern int signgam;

#if defined(__GNUC__) || defined(__clang__)
#define HUGE_VALF (__builtin_huge_valf())
#define HUGE_VAL (__builtin_huge_val())
#define HUGE_VALL (__builtin_huge_vall())
#define INFINITY (__builtin_inff())
#define NAN (__builtin_nanf(""))
#else
#define HUGE_VALF (1e10000f)
#define HUGE_VAL (1e10000)
#define HUGE_VALL (1e10000L)
#define INFINITY HUGE_VALF
#define NAN (0.0f / 0.0f)
#endif

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
long double fmal(long double x, long double y, long double z);
long double exp10l(long double value);
long double pow10l(long double value);
long double erfl(long double value);
long double erfcl(long double value);
long double tgammal(long double value);
long double lgammal(long double value);
long double lgammal_r(long double value, int *sign);
double scalb(double value, double exponent);
float scalbf(float value, float exponent);
double significand(double value);
float significandf(float value);
int __fpclassify(double value);
int __fpclassifyf(float value);
int __fpclassifyl(long double value);
int __signbit(double value);
int __signbitf(float value);
int __signbitl(long double value);
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
long double nexttowardl(long double x, long double y);
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
long double remquol(long double x, long double y, int *quotient);
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
/* C17 7.12.3: the classification and comparison macros evaluate their
 * argument once and raise no floating-point exception (an infinity minus
 * itself would raise FE_INVALID; an ordered comparison of a NaN raises it
 * too). GCC and clang provide exactly that as builtins. */
#if !defined(__TINYC__)
#define fpclassify(value) \
  __builtin_fpclassify(FP_NAN, FP_INFINITE, FP_NORMAL, FP_SUBNORMAL, FP_ZERO, value)
#define isnan(value) __builtin_isnan(value)
#define isinf(value) __builtin_isinf(value)
#define isfinite(value) __builtin_isfinite(value)
#define isnormal(value) __builtin_isnormal(value)
#define signbit(value) __builtin_signbit(value)
#define isgreater(x, y) __builtin_isgreater(x, y)
#define isgreaterequal(x, y) __builtin_isgreaterequal(x, y)
#define isless(x, y) __builtin_isless(x, y)
#define islessequal(x, y) __builtin_islessequal(x, y)
#define islessgreater(x, y) __builtin_islessgreater(x, y)
#define isunordered(x, y) __builtin_isunordered(x, y)
#else
/* TinyCC has no such builtins: these evaluate the argument more than once
 * and isinf/isfinite raise FE_INVALID on an infinity. */
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
#define isnormal(value) (fpclassify(value) == FP_NORMAL)
#define signbit(value) ((value) < 0 || ((value) == 0 && (1.0 / (value)) < 0))
#define isunordered(x, y) (isnan(x) || isnan(y))
#define isgreater(x, y) (!isunordered(x, y) && (x) > (y))
#define isgreaterequal(x, y) (!isunordered(x, y) && (x) >= (y))
#define isless(x, y) (!isunordered(x, y) && (x) < (y))
#define islessequal(x, y) (!isunordered(x, y) && (x) <= (y))
#define islessgreater(x, y) (!isunordered(x, y) && ((x) < (y) || (x) > (y)))
#endif

/* SSE arithmetic on x86-64: FLT_EVAL_METHOD 0. */
typedef float float_t;
typedef double double_t;

/* Nelibc's math functions set errno and raise the exception (as glibc). */
#define MATH_ERRNO 1
#define MATH_ERREXCEPT 2
#define math_errhandling (MATH_ERRNO | MATH_ERREXCEPT)

/* XSI constants, in the namespaces where glibc and musl define them. */
#if defined(_XOPEN_SOURCE) || defined(_GNU_SOURCE) || defined(_BSD_SOURCE) || \
    defined(_DEFAULT_SOURCE) || (!defined(_POSIX_C_SOURCE) && !defined(__STRICT_ANSI__))
#define M_E 2.7182818284590452354
#define M_LOG2E 1.4426950408889634074
#define M_LOG10E 0.43429448190325182765
#define M_LN2 0.69314718055994530942
#define M_LN10 2.30258509299404568402
#define M_PI 3.14159265358979323846
#define M_PI_2 1.57079632679489661923
#define M_PI_4 0.78539816339744830962
#define M_1_PI 0.31830988618379067154
#define M_2_PI 0.63661977236758134308
#define M_2_SQRTPI 1.12837916709551257390
#define M_SQRT2 1.41421356237309504880
#define M_SQRT1_2 0.70710678118654752440
#define MAXFLOAT 3.40282346638528859812e+38F
#endif

#else

#include_next <math.h>
/* GNU math extensions implemented by Nelibc but not exposed by every
   system header feature-test configuration. */
float exp10f(float value);
double exp10(double value);
float pow10f(float value);
double pow10(double value);
long double exp10l(long double value);
long double pow10l(long double value);
long double erfl(long double value);
long double erfcl(long double value);
long double tgammal(long double value);
long double lgammal(long double value);
long double lgammal_r(long double value, int *sign);
int __fpclassify(double value);
int __fpclassifyf(float value);
int __fpclassifyl(long double value);
int __signbit(double value);
int __signbitf(float value);
int __signbitl(long double value);
int finitef(float value);
int finite(double value);
float dremf(float x, float y);
double drem(double x, double y);
double scalb(double value, double exponent);
float scalbf(float value, float exponent);
double significand(double value);
float significandf(float value);
void sincosf(float value, float *sine, float *cosine);
void sincos(double value, double *sine, double *cosine);

#endif

#endif
