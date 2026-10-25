#ifndef NELIBC_COMPLEX_WRAPPER_H
#define NELIBC_COMPLEX_WRAPPER_H

#ifdef NELIBC_STANDALONE

#define complex _Complex
#define _Complex_I (__extension__ 1.0iF)
#define I _Complex_I

float cabsf(float complex value);
double cabs(double complex value);
long double cabsl(long double complex value);
float crealf(float complex value);
double creal(double complex value);
long double creall(long double complex value);
float cimagf(float complex value);
double cimag(double complex value);
long double cimagl(long double complex value);
float complex conjf(float complex value);
double complex conj(double complex value);
long double complex conjl(long double complex value);
float complex cprojf(float complex value);
double complex cproj(double complex value);
long double complex cprojl(long double complex value);

#else

#include_next <complex.h>

#endif

#endif
