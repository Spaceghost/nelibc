#include <complex.h>
#include <float.h>
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
  float _Complex (*volatile call_csqrtf)(float _Complex) = csqrtf;
  double _Complex (*volatile call_csqrt)(double _Complex) = csqrt;
  long double _Complex (*volatile call_csqrtl)(long double _Complex) = csqrtl;
  float _Complex (*volatile call_cexpf)(float _Complex) = cexpf;
  double _Complex (*volatile call_cexp)(double _Complex) = cexp;
  long double _Complex (*volatile call_cexpl)(long double _Complex) = cexpl;
  float _Complex (*volatile call_clogf)(float _Complex) = clogf;
  double _Complex (*volatile call_clog)(double _Complex) = clog;
  long double _Complex (*volatile call_clogl)(long double _Complex) = clogl;
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
  float _Complex rootf = call_csqrtf(__builtin_complex(3.0f, 4.0f));
  double _Complex root = call_csqrt(__builtin_complex(-3.0, 4.0));
  long double _Complex rootl = call_csqrtl(__builtin_complex(3.0L, -4.0L));
  float _Complex cutf = call_csqrtf(__builtin_complex(-4.0f, -0.0f));
  double _Complex cut = call_csqrt(__builtin_complex(-4.0, -0.0));
  long double _Complex cutl = call_csqrtl(__builtin_complex(-4.0L, -0.0L));
  float _Complex zerof = call_csqrtf(__builtin_complex(-0.0f, -0.0f));
  double _Complex zero = call_csqrt(__builtin_complex(-0.0, -0.0));
  long double _Complex zerol = call_csqrtl(__builtin_complex(-0.0L, -0.0L));
  float _Complex largef = call_csqrtf(__builtin_complex(FLT_MAX, FLT_MAX));
  double _Complex large = call_csqrt(__builtin_complex(DBL_MAX, DBL_MAX));
  long double _Complex largel = call_csqrtl(__builtin_complex(LDBL_MAX, LDBL_MAX));
  float _Complex infnanf = call_csqrtf(__builtin_complex(INFINITY, NAN));
  double _Complex infnan = call_csqrt(__builtin_complex(-INFINITY, NAN));
  long double _Complex infnanl = call_csqrtl(__builtin_complex(INFINITY, NAN));
  if (call_crealf(rootf) != 2.0f || call_cimagf(rootf) != 1.0f ||
      call_creal(root) != 1.0 || call_cimag(root) != 2.0 ||
      call_creall(rootl) != 2.0L || call_cimagl(rootl) != -1.0L ||
      call_crealf(cutf) != 0.0f || call_cimagf(cutf) != -2.0f ||
      call_creal(cut) != 0.0 || call_cimag(cut) != -2.0 ||
      call_creall(cutl) != 0.0L || call_cimagl(cutl) != -2.0L ||
      call_crealf(zerof) != 0.0f || !signbit(call_cimagf(zerof)) ||
      call_creal(zero) != 0.0 || !signbit(call_cimag(zero)) ||
      call_creall(zerol) != 0.0L || !signbit(call_cimagl(zerol)) ||
      !isfinite(call_crealf(largef)) || !isfinite(call_cimagf(largef)) ||
      !isfinite(call_creal(large)) || !isfinite(call_cimag(large)) ||
      !isfinite(call_creall(largel)) || !isfinite(call_cimagl(largel)) ||
      !isinf(call_crealf(infnanf)) || !isnan(call_cimagf(infnanf)) ||
      !isnan(call_creal(infnan)) || !isinf(call_cimag(infnan)) ||
      !isinf(call_creall(infnanl)) || !isnan(call_cimagl(infnanl))) return 7;
  float _Complex expf_value = call_cexpf(__builtin_complex(1.0f, 1.0f));
  double _Complex exp_value = call_cexp(__builtin_complex(1.0, 1.0));
  long double _Complex expl_value = call_cexpl(__builtin_complex(1.0L, 1.0L));
  float _Complex expf_zero = call_cexpf(__builtin_complex(1.0f, -0.0f));
  double _Complex exp_zero = call_cexp(__builtin_complex(1.0, -0.0));
  long double _Complex expl_zero = call_cexpl(__builtin_complex(1.0L, -0.0L));
  float _Complex expf_infinity = call_cexpf(__builtin_complex(-INFINITY, -0.0f));
  double _Complex exp_infinity = call_cexp(__builtin_complex((double)INFINITY, -0.0));
  long double _Complex expl_infinity = call_cexpl(__builtin_complex(-(long double)INFINITY, -0.0L));
  double _Complex exp_nan = call_cexp(__builtin_complex(0.0, (double)INFINITY));
  if (fabsf(call_crealf(expf_value) - expf(1.0f) * cosf(1.0f)) > 2e-6f ||
      fabsf(call_cimagf(expf_value) - expf(1.0f) * sinf(1.0f)) > 2e-6f ||
      fabs(call_creal(exp_value) - exp(1.0) * cos(1.0)) > 2e-15 ||
      fabs(call_cimag(exp_value) - exp(1.0) * sin(1.0)) > 2e-15 ||
      fabsl(call_creall(expl_value) - expl(1.0L) * cosl(1.0L)) > 2e-18L ||
      fabsl(call_cimagl(expl_value) - expl(1.0L) * sinl(1.0L)) > 2e-18L ||
      call_crealf(expf_zero) != expf(1.0f) || !signbit(call_cimagf(expf_zero)) ||
      call_creal(exp_zero) != exp(1.0) || !signbit(call_cimag(exp_zero)) ||
      call_creall(expl_zero) != expl(1.0L) || !signbit(call_cimagl(expl_zero)) ||
      call_crealf(expf_infinity) != 0.0f || signbit(call_crealf(expf_infinity)) ||
      call_cimagf(expf_infinity) != 0.0f || !signbit(call_cimagf(expf_infinity)) ||
      !isinf(call_creal(exp_infinity)) || call_cimag(exp_infinity) != 0.0 ||
      !signbit(call_cimag(exp_infinity)) ||
      call_creall(expl_infinity) != 0.0L || signbit(call_creall(expl_infinity)) ||
      call_cimagl(expl_infinity) != 0.0L || !signbit(call_cimagl(expl_infinity)) ||
      !isnan(call_creal(exp_nan)) || !isnan(call_cimag(exp_nan))) return 8;
  float _Complex logf_value = call_clogf(__builtin_complex(3.0f, 4.0f));
  double _Complex log_value = call_clog(__builtin_complex(3.0, 4.0));
  long double _Complex logl_value = call_clogl(__builtin_complex(3.0L, 4.0L));
  float _Complex logf_cut = call_clogf(__builtin_complex(-4.0f, -0.0f));
  double _Complex log_cut = call_clog(__builtin_complex(-4.0, -0.0));
  long double _Complex logl_cut = call_clogl(__builtin_complex(-4.0L, -0.0L));
  float _Complex logf_large = call_clogf(__builtin_complex(FLT_MAX, FLT_MAX));
  double _Complex log_large = call_clog(__builtin_complex(DBL_MAX, DBL_MAX));
  long double _Complex logl_large = call_clogl(__builtin_complex(LDBL_MAX, LDBL_MAX));
  double _Complex log_infinity = call_clog(__builtin_complex((double)INFINITY, (double)NAN));
  double _Complex log_zero = call_clog(__builtin_complex(-0.0, -0.0));
  if (fabsf(call_crealf(logf_value) - logf(5.0f)) > 2e-6f ||
      fabsf(call_cimagf(logf_value) - atan2f(4.0f, 3.0f)) > 2e-6f ||
      fabs(call_creal(log_value) - log(5.0)) > 2e-15 ||
      fabs(call_cimag(log_value) - atan2(4.0, 3.0)) > 2e-15 ||
      fabsl(call_creall(logl_value) - logl(5.0L)) > 2e-18L ||
      fabsl(call_cimagl(logl_value) - atan2l(4.0L, 3.0L)) > 2e-18L ||
      fabsf(call_crealf(logf_cut) - logf(4.0f)) > 2e-6f ||
      fabsf(call_cimagf(logf_cut) + acosf(-1.0f)) > 2e-6f ||
      fabs(call_creal(log_cut) - log(4.0)) > 2e-15 ||
      fabs(call_cimag(log_cut) + acos(-1.0)) > 2e-15 ||
      fabsl(call_creall(logl_cut) - logl(4.0L)) > 2e-18L ||
      fabsl(call_cimagl(logl_cut) + acosl(-1.0L)) > 2e-18L ||
      !isfinite(call_crealf(logf_large)) || !isfinite(call_cimagf(logf_large)) ||
      !isfinite(call_creal(log_large)) || !isfinite(call_cimag(log_large)) ||
      !isfinite(call_creall(logl_large)) || !isfinite(call_cimagl(logl_large)) ||
      !isinf(call_creal(log_infinity)) || !isnan(call_cimag(log_infinity)) ||
      !isinf(call_creal(log_zero)) || !signbit(call_creal(log_zero)) ||
      fabs(call_cimag(log_zero) + acos(-1.0)) > 1e-15) return 9;
  return 0;
}
