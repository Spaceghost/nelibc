#ifndef NELIBC_TGMATH_WRAPPER_H
#define NELIBC_TGMATH_WRAPPER_H

/* Type-generic math. Clang ships its own <tgmath.h> (overloadable
 * functions); with GCC the standalone profile defines the C11 macros over
 * GCC's __builtin_tgmath (GCC 8 and later), choosing among the float,
 * double, long double and complex functions of <math.h> and <complex.h>.
 * Hybrid builds use the host's header. */
#if !defined(NELIBC_STANDALONE) || defined(__clang__) || !defined(__GNUC__) || __GNUC__ < 8

#include_next <tgmath.h>

#else

#include <math.h>
#include <complex.h>

#define __TG_RC(fn, x) __builtin_tgmath(fn##f, fn, fn##l, c##fn##f, c##fn, c##fn##l, x)
#define __TG_RC2(fn, x, y) __builtin_tgmath(fn##f, fn, fn##l, c##fn##f, c##fn, c##fn##l, x, y)
#define __TG_R(fn, x) __builtin_tgmath(fn##f, fn, fn##l, x)
#define __TG_R2(fn, x, y) __builtin_tgmath(fn##f, fn, fn##l, x, y)
#define __TG_R3(fn, x, y, z) __builtin_tgmath(fn##f, fn, fn##l, x, y, z)
#define __TG_C(fn, x) __builtin_tgmath(c##fn##f, c##fn, c##fn##l, x)

/* real and complex */
#define acos(x)  __TG_RC(acos, x)
#define asin(x)  __TG_RC(asin, x)
#define atan(x)  __TG_RC(atan, x)
#define acosh(x) __TG_RC(acosh, x)
#define asinh(x) __TG_RC(asinh, x)
#define atanh(x) __TG_RC(atanh, x)
#define cos(x)   __TG_RC(cos, x)
#define sin(x)   __TG_RC(sin, x)
#define tan(x)   __TG_RC(tan, x)
#define cosh(x)  __TG_RC(cosh, x)
#define sinh(x)  __TG_RC(sinh, x)
#define tanh(x)  __TG_RC(tanh, x)
#define exp(x)   __TG_RC(exp, x)
#define log(x)   __TG_RC(log, x)
#define pow(x, y) __TG_RC2(pow, x, y)
#define sqrt(x)  __TG_RC(sqrt, x)
/* fabs of a complex value is cabs */
#define fabs(x)  __builtin_tgmath(fabsf, fabs, fabsl, cabsf, cabs, cabsl, x)

/* real only */
#define atan2(x, y)     __TG_R2(atan2, x, y)
#define cbrt(x)         __TG_R(cbrt, x)
#define ceil(x)         __TG_R(ceil, x)
#define copysign(x, y)  __TG_R2(copysign, x, y)
#define erf(x)          __TG_R(erf, x)
#define erfc(x)         __TG_R(erfc, x)
#define exp2(x)         __TG_R(exp2, x)
#define expm1(x)        __TG_R(expm1, x)
#define fdim(x, y)      __TG_R2(fdim, x, y)
#define floor(x)        __TG_R(floor, x)
#define fma(x, y, z)    __TG_R3(fma, x, y, z)
#define fmax(x, y)      __TG_R2(fmax, x, y)
#define fmin(x, y)      __TG_R2(fmin, x, y)
#define fmod(x, y)      __TG_R2(fmod, x, y)
#define frexp(x, e)     __TG_R2(frexp, x, e)
#define hypot(x, y)     __TG_R2(hypot, x, y)
#define ilogb(x)        __TG_R(ilogb, x)
#define ldexp(x, e)     __TG_R2(ldexp, x, e)
#define lgamma(x)       __TG_R(lgamma, x)
#define llrint(x)       __TG_R(llrint, x)
#define llround(x)      __TG_R(llround, x)
#define log10(x)        __TG_R(log10, x)
#define log1p(x)        __TG_R(log1p, x)
#define log2(x)         __TG_R(log2, x)
#define logb(x)         __TG_R(logb, x)
#define lrint(x)        __TG_R(lrint, x)
#define lround(x)       __TG_R(lround, x)
#define nearbyint(x)    __TG_R(nearbyint, x)
#define nextafter(x, y) __TG_R2(nextafter, x, y)
#define nexttoward(x, y) __TG_R2(nexttoward, x, y)
#define remainder(x, y) __TG_R2(remainder, x, y)
#define remquo(x, y, q) __TG_R3(remquo, x, y, q)
#define rint(x)         __TG_R(rint, x)
#define round(x)        __TG_R(round, x)
#define scalbln(x, n)   __TG_R2(scalbln, x, n)
#define scalbn(x, n)    __TG_R2(scalbn, x, n)
#define tgamma(x)       __TG_R(tgamma, x)
#define trunc(x)        __TG_R(trunc, x)

/* complex only */
#define carg(x)  __TG_C(arg, x)
#define cimag(x) __TG_C(imag, x)
#define conj(x)  __TG_C(onj, x)
#define cproj(x) __TG_C(proj, x)
#define creal(x) __TG_C(real, x)

#endif

#endif
