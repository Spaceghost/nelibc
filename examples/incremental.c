#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "nelibc_alloc.h"

int main(void) {
  static const char input[] = "nelua-libc:42";
  char *copy = nelibc_malloc(sizeof input);
  if (copy == NULL) return 1;

  memcpy(copy, input, sizeof input);
  const size_t length = strlen(copy);
  char *number = strchr(copy, ':');
  if (number == NULL) {
    nelibc_free(copy);
    return 2;
  }

  errno = 0;
  char *end = NULL;
  const long value = strtol(number + 1, &end, 10);
  if (errno != 0 || end == number + 1 || *end != '\0' || value != 42) {
    nelibc_free(copy);
    return 3;
  }

  const double root = sqrt((double)value + 39.0);
  const double cube_root = cbrt(27.0);
  const float cube_root_float = cbrtf(-0.125f);
  const double modulo = fmod(-5.0, 2.0);
  const double rem = remainder(7.0, 2.0);
  const double low = fmin(5.0, 2.0);
  const double high = fmax(5.0, 2.0);
  const double positive_difference = fdim(7.0, 2.0);
  const float positive_difference_float = fdimf(7.0f, 2.0f);
  double integral = 0.0;
  const double fraction = modf(9.5, &integral);
  int exponent = 0;
  const double mantissa = frexp(8.0, &exponent);
  const double scaled = ldexp(0.75, 4);
  const float scaled_float_exp = ldexpf(0.75f, 4);
  const double scaled_binary = scalbn(0.75, 4);
  const float scaled_float = scalbnf(-0.75f, 4);
  const int binary_exponent = ilogb(8.0);
  const double extracted_exponent = logb(8.0);
  const double adjacent = nextafter(1.0, 2.0);
  const float adjacent_float = nextafterf(1.0f, 2.0f);
  const double toward = nexttoward(1.0, 1.0L + 0x1p-60L);
  const float toward_float = nexttowardf(1.0f, 1.0L + 0x1p-30L);
  const double rounded_current = rint(2.5);
  const float rounded_current_float = rintf(2.5f);
  const double nearby_current = nearbyint(-1.5);
  const float nearby_current_float = nearbyintf(-1.5f);
  const double exponential = exp(1.0);
  const float exponential_float = expf(1.0f);
  const double logarithm = log(2.0);
  const float logarithm_float = logf(2.0f);
  const double logarithm_binary = log2(8.0);
  const double logarithm_one_plus = log1p(0.5);
  const double exponential_minus_one = expm1(0.5);
  const double logarithm_base_ten = log10(100.0);
  const double exponential_base_ten = exp10(2.0);
  const double hypotenuse = hypot(3.0, 4.0);
  const float hypotenuse_float = hypotf(3.0f, 4.0f);
  const double sine = sin(1.0);
  const double cosine = cos(1.0);
  const float sine_float = sinf(1.0f);
  const float cosine_float = cosf(1.0f);
  const double tangent = tan(1.0);
  const float tangent_float = tanf(1.0f);
  const double hyperbolic_sine = sinh(1.0);
  const float hyperbolic_sine_float = sinhf(1.0f);
  const double hyperbolic_cosine = cosh(1.0);
  const float hyperbolic_cosine_float = coshf(1.0f);
  const double hyperbolic_tangent = tanh(1.0);
  const float hyperbolic_tangent_float = tanhf(1.0f);
  const double inverse_hyperbolic_sine = asinh(1.0);
  const float inverse_hyperbolic_sine_float = asinhf(1.0f);
  const double inverse_hyperbolic_cosine = acosh(2.0);
  const float inverse_hyperbolic_cosine_float = acoshf(2.0f);
  const double inverse_hyperbolic_tangent = atanh(0.5);
  const float inverse_hyperbolic_tangent_float = atanhf(0.5f);
  const double arc_tangent = atan(1.0);
  const float arc_tangent_float = atanf(1.0f);
  const double quadrant_tangent = atan2(1.0, -1.0);
  const float quadrant_tangent_float = atan2f(1.0f, -1.0f);
  const double power = pow(2.0, 10.0);
  const float power_float = powf(2.0f, 10.0f);
  const double long_scaled = scalbln(0.75, 4L);
  const float long_scaled_float = scalblnf(-0.75f, 4L);
  const long double long_double_scaled = ldexpl(1.5L, 4);
  const long double long_double_binary = scalbnl(-0.75L, -2);
  const long double long_double_long = scalblnl(0.5L, 4L);
  const long double long_double_truncated = truncl(-1.75L);
  const long double long_double_floor = floorl(-0.25L);
  const long double long_double_ceiling = ceill(-0.25L);
  const long double long_double_rounded = rintl(2.5L);
  const long double long_double_nearby = nearbyintl(-1.5L);
  const long double long_double_round = roundl(-0.5L);
  const long long long_double_llrint = llrintl(2.5L);
  const long double long_double_lround = lroundl(-1.5L);
  const long long long_double_llround = llroundl(2.5L);
  if (!(inverse_hyperbolic_sine > 0.881373587019542 && inverse_hyperbolic_sine < 0.881373587019544)) return 10;
  if (!(inverse_hyperbolic_sine_float > 0.8813735f && inverse_hyperbolic_sine_float < 0.8813737f)) return 11;
  if (!(inverse_hyperbolic_cosine > 1.316957896924816 && inverse_hyperbolic_cosine < 1.316957896924818)) return 12;
  if (!(inverse_hyperbolic_cosine_float > 1.3169578f && inverse_hyperbolic_cosine_float <= 1.3169580f)) return 13;
  if (!(inverse_hyperbolic_tangent > 0.549306144334054 && inverse_hyperbolic_tangent < 0.549306144334056)) return 14;
  if (!(inverse_hyperbolic_tangent_float > 0.5493060f && inverse_hyperbolic_tangent_float < 0.5493063f)) return 15;
  if (root != 9.0 || cube_root != 3.0 || cube_root_float != -0.5f ||
      modulo != -1.0 || rem != -1.0 || low != 2.0 || high != 5.0 ||
      positive_difference != 5.0 || positive_difference_float != 5.0f ||
      fraction != 0.5 || integral != 9.0 || mantissa != 0.5 || exponent != 4 ||
      scaled != 12.0 || scaled_float_exp != 12.0f || scaled_binary != 12.0 ||
      scaled_float != -12.0f || binary_exponent != 3 || extracted_exponent != 3.0 ||
      adjacent != 0x1.0000000000001p0 || adjacent_float != 0x1.000002p0f ||
      toward != 0x1.0000000000001p0 || toward_float != 0x1.000002p0f ||
      rounded_current != 2.0 || rounded_current_float != 2.0f ||
      nearby_current != -2.0 || nearby_current_float != -2.0f ||
      !(exponential > 2.71828182845904 && exponential < 2.71828182845905) ||
      !(exponential_float > 2.71828f && exponential_float < 2.71829f) ||
      !(logarithm > 0.69314718055994 && logarithm < 0.69314718055995) ||
      !(logarithm_float > 0.69314f && logarithm_float < 0.69315f) ||
      hypotenuse != 5.0 || hypotenuse_float != 5.0f ||
      !(sine > 0.84147098480789 && sine < 0.84147098480790) ||
      !(cosine > 0.54030230586813 && cosine < 0.54030230586814) ||
      !(sine_float > 0.8414708f && sine_float < 0.8414712f) ||
      !(cosine_float > 0.5403021f && cosine_float < 0.5403025f) ||
      !(tangent > 1.5574077246549 && tangent < 1.5574077246550) ||
      !(tangent_float > 1.5574076f && tangent_float < 1.5574079f) ||
      !(hyperbolic_sine > 1.1752011936437 && hyperbolic_sine < 1.1752011936439) ||
      !(hyperbolic_sine_float > 1.1752011f && hyperbolic_sine_float < 1.1752013f) ||
      !(hyperbolic_cosine > 1.5430806348151 && hyperbolic_cosine < 1.5430806348154) ||
      !(hyperbolic_cosine_float > 1.5430805f && hyperbolic_cosine_float < 1.5430808f) ||
      !(hyperbolic_tangent > 0.7615941559556 && hyperbolic_tangent < 0.7615941559559) ||
      !(hyperbolic_tangent_float > 0.7615941f && hyperbolic_tangent_float < 0.7615943f) ||
      !(inverse_hyperbolic_sine > 0.881373587019542 && inverse_hyperbolic_sine < 0.881373587019544) ||
      !(inverse_hyperbolic_sine_float > 0.8813735f && inverse_hyperbolic_sine_float < 0.8813737f) ||
      !(inverse_hyperbolic_cosine > 1.316957896924816 && inverse_hyperbolic_cosine < 1.316957896924818) ||
      !(inverse_hyperbolic_cosine_float > 1.3169578f && inverse_hyperbolic_cosine_float <= 1.3169580f) ||
      !(inverse_hyperbolic_tangent > 0.549306144334054 && inverse_hyperbolic_tangent < 0.549306144334056) ||
      !(inverse_hyperbolic_tangent_float > 0.5493060f && inverse_hyperbolic_tangent_float < 0.5493063f) ||
      !(arc_tangent > 0.78539816339744 && arc_tangent < 0.78539816339746) ||
      !(arc_tangent_float > 0.7853981f && arc_tangent_float < 0.7853983f) ||
      !(quadrant_tangent > 2.3561944901923 && quadrant_tangent < 2.3561944901924) ||
      !(quadrant_tangent_float > 2.3561943f && quadrant_tangent_float < 2.3561947f) ||
      power != 1024.0 || power_float != 1024.0f ||
      logarithm_binary != 3.0 ||
      !(logarithm_one_plus > 0.4054651081081643 && logarithm_one_plus < 0.4054651081081645) ||
      !(exponential_minus_one > 0.6487212707001279 && exponential_minus_one < 0.6487212707001283) ||
      !(logarithm_base_ten > 1.999999999999999 && logarithm_base_ten < 2.000000000000001) ||
      exponential_base_ten != 100.0 ||
      long_scaled != 12.0 || long_scaled_float != -12.0f ||
      long_double_scaled != 24.0L || long_double_binary != -0.1875L ||
      long_double_long != 8.0L || long_double_truncated != -1.0L ||
      long_double_floor != -1.0L || long_double_ceiling != -0.0L ||
      long_double_rounded != 2.0L || long_double_nearby != -2.0L ||
      long_double_round != -1.0L || long_double_llrint != 2 ||
      long_double_lround != -2.0L || long_double_llround != 3) {
    nelibc_free(copy);
    return 4;
  }

  printf("Nelua allocator + memory/string/strtol/math: %s -> %ld (%zu bytes, sqrt=%.0f cbrt=%.0f cbrtf=%.1f fmod=%.0f remainder=%.0f min=%.0f max=%.0f fdim=%.0f fdimf=%.0f modf=%.1f+%.1f frexp=%.1f*2^%d ldexp=%.0f scalbnf=%.0f scalbln=%.0f rint=%.0f rintf=%.0f nearbyint=%.0f nearbyintf=%.0f exp=%.9g expf=%.9g expm1=%.9g log=%.9g logf=%.9g log1p=%.9g log2=%.0f log10=%.0f exp10=%.0f pow=%.0f powf=%.0f hypot=%.0f hypotf=%.0f sin=%.9g cos=%.9g sinf=%.9g cosf=%.9g tan=%.9g tanf=%.9g sinh=%.9g sinhf=%.9g cosh=%.9g coshf=%.9g tanh=%.9g tanhf=%.9g asinh=%.9g asinhf=%.9g acosh=%.9g acoshf=%.9g atanh=%.9g atanhf=%.9g atan=%.9g atanf=%.9g atan2=%.9g atan2f=%.9g nextafter=%.17g nexttoward=%.17g nexttowardf=%.9g ilogb=%d logb=%.0f)\n",
         copy, value, length, root, cube_root, cube_root_float, modulo, rem, low, high, positive_difference, positive_difference_float, integral, fraction, mantissa, exponent, scaled, scaled_float, long_scaled, rounded_current, rounded_current_float, nearby_current, nearby_current_float, exponential, exponential_float, exponential_minus_one, logarithm, logarithm_float, logarithm_one_plus, logarithm_binary, logarithm_base_ten, exponential_base_ten, power, power_float, hypotenuse, hypotenuse_float, sine, cosine, sine_float, cosine_float, tangent, tangent_float, hyperbolic_sine, hyperbolic_sine_float, hyperbolic_cosine, hyperbolic_cosine_float, hyperbolic_tangent, hyperbolic_tangent_float, inverse_hyperbolic_sine, inverse_hyperbolic_sine_float, inverse_hyperbolic_cosine, inverse_hyperbolic_cosine_float, inverse_hyperbolic_tangent, inverse_hyperbolic_tangent_float, arc_tangent, arc_tangent_float, quadrant_tangent, quadrant_tangent_float, adjacent, toward, toward_float, binary_exponent, extracted_exponent);
  nelibc_free(copy);
  return 0;
}
