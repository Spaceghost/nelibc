/* C complex ABI adapters. Arithmetic lives in math_core.nelua. */
#include <complex.h>

extern float nelibc_cabsf_parts(float real, float imaginary);
extern double nelibc_cabs_parts(double real, double imaginary);
extern long double nelibc_cabsl_parts(long double real, long double imaginary);
extern float nelibc_conjf_imaginary(float imaginary);
extern double nelibc_conj_imaginary(double imaginary);
extern long double nelibc_conjl_imaginary(long double imaginary);
extern float nelibc_cprojf_real(float real, float imaginary);
extern double nelibc_cproj_real(double real, double imaginary);
extern long double nelibc_cprojl_real(long double real, long double imaginary);
extern float nelibc_cprojf_imaginary(float real, float imaginary);
extern double nelibc_cproj_imaginary(double real, double imaginary);
extern long double nelibc_cprojl_imaginary(long double real, long double imaginary);
extern float nelibc_cargf(float real, float imaginary);
extern double nelibc_carg(double real, double imaginary);
extern long double nelibc_cargl(long double real, long double imaginary);
extern void nelibc_csqrtf_parts(float real, float imaginary, float *out_real, float *out_imaginary);
extern void nelibc_csqrt_parts(double real, double imaginary, double *out_real, double *out_imaginary);
extern void nelibc_csqrtl_parts(long double real, long double imaginary, long double *out_real, long double *out_imaginary);
extern void nelibc_cexpf_parts(float real, float imaginary, float *out_real, float *out_imaginary);
extern void nelibc_cexp_parts(double real, double imaginary, double *out_real, double *out_imaginary);
extern void nelibc_cexpl_parts(long double real, long double imaginary, long double *out_real, long double *out_imaginary);
extern void nelibc_clogf_parts(float real, float imaginary, float *out_real, float *out_imaginary);
extern void nelibc_clog_parts(double real, double imaginary, double *out_real, double *out_imaginary);
extern void nelibc_clogl_parts(long double real, long double imaginary, long double *out_real, long double *out_imaginary);
extern void nelibc_csinparts_f(float real, float imaginary, float *out_real, float *out_imaginary);
extern void nelibc_csinparts(double real, double imaginary, double *out_real, double *out_imaginary);
extern void nelibc_csinparts_l(long double real, long double imaginary, long double *out_real, long double *out_imaginary);
extern void nelibc_ccosparts_f(float real, float imaginary, float *out_real, float *out_imaginary);
extern void nelibc_ccosparts(double real, double imaginary, double *out_real, double *out_imaginary);
extern void nelibc_ccosparts_l(long double real, long double imaginary, long double *out_real, long double *out_imaginary);
extern void nelibc_csinhparts_f(float real, float imaginary, float *out_real, float *out_imaginary);
extern void nelibc_csinhparts(double real, double imaginary, double *out_real, double *out_imaginary);
extern void nelibc_csinhparts_l(long double real, long double imaginary, long double *out_real, long double *out_imaginary);
extern void nelibc_ccoshparts_f(float real, float imaginary, float *out_real, float *out_imaginary);
extern void nelibc_ccoshparts(double real, double imaginary, double *out_real, double *out_imaginary);
extern void nelibc_ccoshparts_l(long double real, long double imaginary, long double *out_real, long double *out_imaginary);
extern void nelibc_ctanparts_f(float real, float imaginary, float *out_real, float *out_imaginary);
extern void nelibc_ctanparts(double real, double imaginary, double *out_real, double *out_imaginary);
extern void nelibc_ctanparts_l(long double real, long double imaginary, long double *out_real, long double *out_imaginary);
extern void nelibc_ctanhparts_f(float real, float imaginary, float *out_real, float *out_imaginary);
extern void nelibc_ctanhparts(double real, double imaginary, double *out_real, double *out_imaginary);
extern void nelibc_ctanhparts_l(long double real, long double imaginary, long double *out_real, long double *out_imaginary);
extern void nelibc_catanparts_f(float real, float imaginary, float *out_real, float *out_imaginary);
extern void nelibc_catanparts(double real, double imaginary, double *out_real, double *out_imaginary);
extern void nelibc_catanparts_l(long double real, long double imaginary, long double *out_real, long double *out_imaginary);
extern void nelibc_catanhparts_f(float real, float imaginary, float *out_real, float *out_imaginary);
extern void nelibc_catanhparts(double real, double imaginary, double *out_real, double *out_imaginary);
extern void nelibc_catanhparts_l(long double real, long double imaginary, long double *out_real, long double *out_imaginary);
extern void nelibc_casinparts_f(float real, float imaginary, float *out_real, float *out_imaginary);
extern void nelibc_casinparts(double real, double imaginary, double *out_real, double *out_imaginary);
extern void nelibc_casinparts_l(long double real, long double imaginary, long double *out_real, long double *out_imaginary);
extern void nelibc_cacosparts_f(float real, float imaginary, float *out_real, float *out_imaginary);
extern void nelibc_cacosparts(double real, double imaginary, double *out_real, double *out_imaginary);
extern void nelibc_cacosparts_l(long double real, long double imaginary, long double *out_real, long double *out_imaginary);
extern void nelibc_casinhparts_f(float real, float imaginary, float *out_real, float *out_imaginary);
extern void nelibc_casinhparts(double real, double imaginary, double *out_real, double *out_imaginary);
extern void nelibc_casinhparts_l(long double real, long double imaginary, long double *out_real, long double *out_imaginary);
extern void nelibc_cacoshparts_f(float real, float imaginary, float *out_real, float *out_imaginary);
extern void nelibc_cacoshparts(double real, double imaginary, double *out_real, double *out_imaginary);
extern void nelibc_cacoshparts_l(long double real, long double imaginary, long double *out_real, long double *out_imaginary);

float cabsf(float _Complex value) {
  return nelibc_cabsf_parts(__real__ value, __imag__ value);
}

double cabs(double _Complex value) {
  return nelibc_cabs_parts(__real__ value, __imag__ value);
}

long double cabsl(long double _Complex value) {
  return nelibc_cabsl_parts(__real__ value, __imag__ value);
}

float crealf(float _Complex value) {
  return __real__ value;
}

double creal(double _Complex value) {
  return __real__ value;
}

long double creall(long double _Complex value) {
  return __real__ value;
}

float cimagf(float _Complex value) {
  return __imag__ value;
}

double cimag(double _Complex value) {
  return __imag__ value;
}

long double cimagl(long double _Complex value) {
  return __imag__ value;
}

float _Complex conjf(float _Complex value) {
  return __builtin_complex(__real__ value,
                           nelibc_conjf_imaginary(__imag__ value));
}

double _Complex conj(double _Complex value) {
  return __builtin_complex(__real__ value,
                           nelibc_conj_imaginary(__imag__ value));
}

long double _Complex conjl(long double _Complex value) {
  return __builtin_complex(__real__ value,
                           nelibc_conjl_imaginary(__imag__ value));
}

float _Complex cprojf(float _Complex value) {
  return __builtin_complex(
      nelibc_cprojf_real(__real__ value, __imag__ value),
      nelibc_cprojf_imaginary(__real__ value, __imag__ value));
}

double _Complex cproj(double _Complex value) {
  return __builtin_complex(
      nelibc_cproj_real(__real__ value, __imag__ value),
      nelibc_cproj_imaginary(__real__ value, __imag__ value));
}

long double _Complex cprojl(long double _Complex value) {
  return __builtin_complex(
      nelibc_cprojl_real(__real__ value, __imag__ value),
      nelibc_cprojl_imaginary(__real__ value, __imag__ value));
}

float cargf(float _Complex value) {
  return nelibc_cargf(__real__ value, __imag__ value);
}

double carg(double _Complex value) {
  return nelibc_carg(__real__ value, __imag__ value);
}

long double cargl(long double _Complex value) {
  return nelibc_cargl(__real__ value, __imag__ value);
}

float _Complex csqrtf(float _Complex value) {
  float real, imaginary;
  nelibc_csqrtf_parts(__real__ value, __imag__ value, &real, &imaginary);
  return __builtin_complex(real, imaginary);
}

double _Complex csqrt(double _Complex value) {
  double real, imaginary;
  nelibc_csqrt_parts(__real__ value, __imag__ value, &real, &imaginary);
  return __builtin_complex(real, imaginary);
}

long double _Complex csqrtl(long double _Complex value) {
  long double real, imaginary;
  nelibc_csqrtl_parts(__real__ value, __imag__ value, &real, &imaginary);
  return __builtin_complex(real, imaginary);
}

float _Complex cexpf(float _Complex value) {
  float real, imaginary;
  nelibc_cexpf_parts(__real__ value, __imag__ value, &real, &imaginary);
  return __builtin_complex(real, imaginary);
}

double _Complex cexp(double _Complex value) {
  double real, imaginary;
  nelibc_cexp_parts(__real__ value, __imag__ value, &real, &imaginary);
  return __builtin_complex(real, imaginary);
}

long double _Complex cexpl(long double _Complex value) {
  long double real, imaginary;
  nelibc_cexpl_parts(__real__ value, __imag__ value, &real, &imaginary);
  return __builtin_complex(real, imaginary);
}

float _Complex clogf(float _Complex value) {
  float real, imaginary;
  nelibc_clogf_parts(__real__ value, __imag__ value, &real, &imaginary);
  return __builtin_complex(real, imaginary);
}

double _Complex clog(double _Complex value) {
  double real, imaginary;
  nelibc_clog_parts(__real__ value, __imag__ value, &real, &imaginary);
  return __builtin_complex(real, imaginary);
}

long double _Complex clogl(long double _Complex value) {
  long double real, imaginary;
  nelibc_clogl_parts(__real__ value, __imag__ value, &real, &imaginary);
  return __builtin_complex(real, imaginary);
}

float _Complex csinf(float _Complex value) {
  float real, imaginary;
  nelibc_csinparts_f(__real__ value, __imag__ value, &real, &imaginary);
  return __builtin_complex(real, imaginary);
}

double _Complex csin(double _Complex value) {
  double real, imaginary;
  nelibc_csinparts(__real__ value, __imag__ value, &real, &imaginary);
  return __builtin_complex(real, imaginary);
}

long double _Complex csinl(long double _Complex value) {
  long double real, imaginary;
  nelibc_csinparts_l(__real__ value, __imag__ value, &real, &imaginary);
  return __builtin_complex(real, imaginary);
}

float _Complex ccosf(float _Complex value) {
  float real, imaginary;
  nelibc_ccosparts_f(__real__ value, __imag__ value, &real, &imaginary);
  return __builtin_complex(real, imaginary);
}

double _Complex ccos(double _Complex value) {
  double real, imaginary;
  nelibc_ccosparts(__real__ value, __imag__ value, &real, &imaginary);
  return __builtin_complex(real, imaginary);
}

long double _Complex ccosl(long double _Complex value) {
  long double real, imaginary;
  nelibc_ccosparts_l(__real__ value, __imag__ value, &real, &imaginary);
  return __builtin_complex(real, imaginary);
}

float _Complex csinhf(float _Complex value) {
  float real, imaginary;
  nelibc_csinhparts_f(__real__ value, __imag__ value, &real, &imaginary);
  return __builtin_complex(real, imaginary);
}

double _Complex csinh(double _Complex value) {
  double real, imaginary;
  nelibc_csinhparts(__real__ value, __imag__ value, &real, &imaginary);
  return __builtin_complex(real, imaginary);
}

long double _Complex csinhl(long double _Complex value) {
  long double real, imaginary;
  nelibc_csinhparts_l(__real__ value, __imag__ value, &real, &imaginary);
  return __builtin_complex(real, imaginary);
}

float _Complex ccoshf(float _Complex value) {
  float real, imaginary;
  nelibc_ccoshparts_f(__real__ value, __imag__ value, &real, &imaginary);
  return __builtin_complex(real, imaginary);
}

double _Complex ccosh(double _Complex value) {
  double real, imaginary;
  nelibc_ccoshparts(__real__ value, __imag__ value, &real, &imaginary);
  return __builtin_complex(real, imaginary);
}

long double _Complex ccoshl(long double _Complex value) {
  long double real, imaginary;
  nelibc_ccoshparts_l(__real__ value, __imag__ value, &real, &imaginary);
  return __builtin_complex(real, imaginary);
}

float _Complex ctanf(float _Complex value) {
  float real, imaginary;
  nelibc_ctanparts_f(__real__ value, __imag__ value, &real, &imaginary);
  return __builtin_complex(real, imaginary);
}

double _Complex ctan(double _Complex value) {
  double real, imaginary;
  nelibc_ctanparts(__real__ value, __imag__ value, &real, &imaginary);
  return __builtin_complex(real, imaginary);
}

long double _Complex ctanl(long double _Complex value) {
  long double real, imaginary;
  nelibc_ctanparts_l(__real__ value, __imag__ value, &real, &imaginary);
  return __builtin_complex(real, imaginary);
}

float _Complex ctanhf(float _Complex value) {
  float real, imaginary;
  nelibc_ctanhparts_f(__real__ value, __imag__ value, &real, &imaginary);
  return __builtin_complex(real, imaginary);
}

double _Complex ctanh(double _Complex value) {
  double real, imaginary;
  nelibc_ctanhparts(__real__ value, __imag__ value, &real, &imaginary);
  return __builtin_complex(real, imaginary);
}

long double _Complex ctanhl(long double _Complex value) {
  long double real, imaginary;
  nelibc_ctanhparts_l(__real__ value, __imag__ value, &real, &imaginary);
  return __builtin_complex(real, imaginary);
}

float _Complex catanf(float _Complex value) {
  float real, imaginary;
  nelibc_catanparts_f(__real__ value, __imag__ value, &real, &imaginary);
  return __builtin_complex(real, imaginary);
}

double _Complex catan(double _Complex value) {
  double real, imaginary;
  nelibc_catanparts(__real__ value, __imag__ value, &real, &imaginary);
  return __builtin_complex(real, imaginary);
}

long double _Complex catanl(long double _Complex value) {
  long double real, imaginary;
  nelibc_catanparts_l(__real__ value, __imag__ value, &real, &imaginary);
  return __builtin_complex(real, imaginary);
}

float _Complex catanhf(float _Complex value) {
  float real, imaginary;
  nelibc_catanhparts_f(__real__ value, __imag__ value, &real, &imaginary);
  return __builtin_complex(real, imaginary);
}

double _Complex catanh(double _Complex value) {
  double real, imaginary;
  nelibc_catanhparts(__real__ value, __imag__ value, &real, &imaginary);
  return __builtin_complex(real, imaginary);
}

long double _Complex catanhl(long double _Complex value) {
  long double real, imaginary;
  nelibc_catanhparts_l(__real__ value, __imag__ value, &real, &imaginary);
  return __builtin_complex(real, imaginary);
}

float _Complex casinf(float _Complex value) {
  float real, imaginary;
  nelibc_casinparts_f(__real__ value, __imag__ value, &real, &imaginary);
  return __builtin_complex(real, imaginary);
}

double _Complex casin(double _Complex value) {
  double real, imaginary;
  nelibc_casinparts(__real__ value, __imag__ value, &real, &imaginary);
  return __builtin_complex(real, imaginary);
}

long double _Complex casinl(long double _Complex value) {
  long double real, imaginary;
  nelibc_casinparts_l(__real__ value, __imag__ value, &real, &imaginary);
  return __builtin_complex(real, imaginary);
}

float _Complex cacosf(float _Complex value) {
  float real, imaginary;
  nelibc_cacosparts_f(__real__ value, __imag__ value, &real, &imaginary);
  return __builtin_complex(real, imaginary);
}

double _Complex cacos(double _Complex value) {
  double real, imaginary;
  nelibc_cacosparts(__real__ value, __imag__ value, &real, &imaginary);
  return __builtin_complex(real, imaginary);
}

long double _Complex cacosl(long double _Complex value) {
  long double real, imaginary;
  nelibc_cacosparts_l(__real__ value, __imag__ value, &real, &imaginary);
  return __builtin_complex(real, imaginary);
}

float _Complex casinhf(float _Complex value) {
  float real, imaginary;
  nelibc_casinhparts_f(__real__ value, __imag__ value, &real, &imaginary);
  return __builtin_complex(real, imaginary);
}

double _Complex casinh(double _Complex value) {
  double real, imaginary;
  nelibc_casinhparts(__real__ value, __imag__ value, &real, &imaginary);
  return __builtin_complex(real, imaginary);
}

long double _Complex casinhl(long double _Complex value) {
  long double real, imaginary;
  nelibc_casinhparts_l(__real__ value, __imag__ value, &real, &imaginary);
  return __builtin_complex(real, imaginary);
}

float _Complex cacoshf(float _Complex value) {
  float real, imaginary;
  nelibc_cacoshparts_f(__real__ value, __imag__ value, &real, &imaginary);
  return __builtin_complex(real, imaginary);
}

double _Complex cacosh(double _Complex value) {
  double real, imaginary;
  nelibc_cacoshparts(__real__ value, __imag__ value, &real, &imaginary);
  return __builtin_complex(real, imaginary);
}

long double _Complex cacoshl(long double _Complex value) {
  long double real, imaginary;
  nelibc_cacoshparts_l(__real__ value, __imag__ value, &real, &imaginary);
  return __builtin_complex(real, imaginary);
}
