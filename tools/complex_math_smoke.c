#include <complex.h>
#include <math.h>

int main(void) {
  float (*volatile call_cabsf)(float _Complex) = cabsf;
  double (*volatile call_cabs)(double _Complex) = cabs;
  long double (*volatile call_cabsl)(long double _Complex) = cabsl;
  float (*volatile call_crealf)(float _Complex) = crealf;
  double (*volatile call_creal)(double _Complex) = creal;
  long double (*volatile call_creall)(long double _Complex) = creall;
  float (*volatile call_cimagf)(float _Complex) = cimagf;
  double (*volatile call_cimag)(double _Complex) = cimag;
  long double (*volatile call_cimagl)(long double _Complex) = cimagl;
  float _Complex (*volatile call_conjf)(float _Complex) = conjf;
  double _Complex (*volatile call_conj)(double _Complex) = conj;
  long double _Complex (*volatile call_conjl)(long double _Complex) = conjl;
  float _Complex zf = __builtin_complex(3.0f, 4.0f);
  double _Complex z = __builtin_complex(-3.0, 4.0);
  long double _Complex zl = __builtin_complex(3.0L, -4.0L);
  if (call_cabsf(zf) != 5.0f || call_cabs(z) != 5.0 || call_cabsl(zl) != 5.0L)
    return 1;
  if (call_crealf(zf) != 3.0f || call_creal(z) != -3.0 ||
      call_creall(zl) != 3.0L || call_cimagf(zf) != 4.0f ||
      call_cimag(z) != 4.0 || call_cimagl(zl) != -4.0L) return 2;

  float _Complex zf_zero = __builtin_complex(-0.0f, -0.0f);
  double _Complex z_zero = __builtin_complex(-0.0, -0.0);
  long double _Complex zl_zero = __builtin_complex(-0.0L, -0.0L);
  float _Complex zf_conjugate = call_conjf(zf_zero);
  double _Complex z_conjugate = call_conj(z_zero);
  long double _Complex zl_conjugate = call_conjl(zl_zero);
  if (!signbit(call_crealf(zf_zero)) || !signbit(call_creal(z_zero)) ||
      !signbit(call_creall(zl_zero)) || !signbit(call_cimagf(zf_zero)) ||
      !signbit(call_cimag(z_zero)) || !signbit(call_cimagl(zl_zero)) ||
      !signbit(call_crealf(zf_conjugate)) || !signbit(call_creal(z_conjugate)) ||
      !signbit(call_creall(zl_conjugate)) || signbit(call_cimagf(zf_conjugate)) ||
      signbit(call_cimag(z_conjugate)) || signbit(call_cimagl(zl_conjugate)))
    return 3;

  float _Complex zf_infinity = __builtin_complex(INFINITY, NAN);
  double _Complex z_infinity = __builtin_complex(INFINITY, NAN);
  long double _Complex zl_infinity = __builtin_complex(INFINITY, NAN);
  if (!isinf(call_cabsf(zf_infinity)) || !isinf(call_cabs(z_infinity)) ||
      !isinf(call_cabsl(zl_infinity)) ||
      !isnan(call_cimagf(call_conjf(zf_infinity))) ||
      !isnan(call_cimag(call_conj(z_infinity))) ||
      !isnan(call_cimagl(call_conjl(zl_infinity)))) return 4;
  return 0;
}
