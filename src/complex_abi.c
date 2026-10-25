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
