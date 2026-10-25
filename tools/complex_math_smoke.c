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
  float _Complex (*volatile call_cprojf)(float _Complex) = cprojf;
  double _Complex (*volatile call_cproj)(double _Complex) = cproj;
  long double _Complex (*volatile call_cprojl)(long double _Complex) = cprojl;
  float (*volatile call_cargf)(float _Complex) = cargf;
  double (*volatile call_carg)(double _Complex) = carg;
  long double (*volatile call_cargl)(long double _Complex) = cargl;
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

  float _Complex zf_projected =
      call_cprojf(__builtin_complex((float)INFINITY, (float)-NAN));
  double _Complex z_projected =
      call_cproj(__builtin_complex(7.0, (double)-INFINITY));
  long double _Complex zl_projected =
      call_cprojl(__builtin_complex((long double)-INFINITY, (long double)NAN));
  float _Complex zf_finite_projected = call_cprojf(zf);
  double _Complex z_finite_projected = call_cproj(z);
  long double _Complex zl_finite_projected = call_cprojl(zl);
  if (!isinf(call_crealf(zf_projected)) || signbit(call_crealf(zf_projected)) ||
      !isinf(call_creal(z_projected)) || signbit(call_creal(z_projected)) ||
      !isinf(call_creall(zl_projected)) || signbit(call_creall(zl_projected)) ||
      call_cimagf(zf_projected) != 0.0f || !signbit(call_cimagf(zf_projected)) ||
      call_cimag(z_projected) != 0.0 || !signbit(call_cimag(z_projected)) ||
      call_cimagl(zl_projected) != 0.0L || signbit(call_cimagl(zl_projected)) ||
      call_crealf(zf_finite_projected) != 3.0f ||
      call_cimagf(zf_finite_projected) != 4.0f ||
      call_creal(z_finite_projected) != -3.0 ||
      call_cimag(z_finite_projected) != 4.0 ||
      call_creall(zl_finite_projected) != 3.0L ||
      call_cimagl(zl_finite_projected) != -4.0L) return 5;

  float anglef = call_cargf(__builtin_complex(1.0f, 1.0f));
  double angle = call_carg(__builtin_complex(1.0, 1.0));
  long double anglel = call_cargl(__builtin_complex(1.0L, 1.0L));
  float anglef_axis = call_cargf(__builtin_complex(-1.0f, 0.0f));
  double angle_axis = call_carg(__builtin_complex(-1.0, -0.0));
  long double anglel_axis = call_cargl(__builtin_complex(-1.0L, 0.0L));
  float anglef_zero = call_cargf(__builtin_complex(1.0f, -0.0f));
  double angle_zero = call_carg(__builtin_complex(1.0, -0.0));
  long double anglel_zero = call_cargl(__builtin_complex(1.0L, -0.0L));
  if (fabsf(anglef - atan2f(1.0f, 1.0f)) > 1e-6f ||
      fabs(angle - atan2(1.0, 1.0)) > 1e-15 ||
      fabsl(anglel - atan2l(1.0L, 1.0L)) > 1e-18L ||
      fabsf(anglef_axis - acosf(-1.0f)) > 1e-6f ||
      fabs(angle_axis + acos(-1.0)) > 1e-15 ||
      fabsl(anglel_axis - acosl(-1.0L)) > 1e-18L ||
      !signbit(anglef_zero) || !signbit(angle_zero) || !signbit(anglel_zero))
    return 6;
  return 0;
}
