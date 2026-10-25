/* C complex ABI adapters. Arithmetic lives in math_core.nelua. */
#include <complex.h>

extern float nelibc_cabsf_parts(float real, float imaginary);
extern double nelibc_cabs_parts(double real, double imaginary);
extern long double nelibc_cabsl_parts(long double real, long double imaginary);
extern float nelibc_conjf_imaginary(float imaginary);
extern double nelibc_conj_imaginary(double imaginary);
extern long double nelibc_conjl_imaginary(long double imaginary);

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
