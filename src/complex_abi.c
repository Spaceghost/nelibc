/* C complex ABI adapters. Arithmetic lives in math_core.nelua. */
#include <complex.h>

extern float nelibc_cabsf_parts(float real, float imaginary);
extern double nelibc_cabs_parts(double real, double imaginary);
extern long double nelibc_cabsl_parts(long double real, long double imaginary);

float cabsf(float _Complex value) {
  return nelibc_cabsf_parts(__real__ value, __imag__ value);
}

double cabs(double _Complex value) {
  return nelibc_cabs_parts(__real__ value, __imag__ value);
}

long double cabsl(long double _Complex value) {
  return nelibc_cabsl_parts(__real__ value, __imag__ value);
}
