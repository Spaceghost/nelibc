#ifndef NELIBC_COMPLEX_WRAPPER_H
#define NELIBC_COMPLEX_WRAPPER_H

#ifdef NELIBC_STANDALONE

#define complex _Complex
#define _Complex_I (__extension__ 1.0iF)
#define I _Complex_I

float cabsf(float complex value);
double cabs(double complex value);
long double cabsl(long double complex value);

#else

#include_next <complex.h>

#endif

#endif
