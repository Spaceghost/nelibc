#include <fenv.h>
#include <float.h>

_Static_assert(sizeof(fenv_t) == 8, "AArch64 FPCR/FPSR layout");
_Static_assert(sizeof(fexcept_t) == 4, "AArch64 exception flags layout");

static double add_half_ulp(void) {
  volatile double one = 1.0;
  volatile double half_ulp = DBL_EPSILON / 2;
  return one + half_ulp;
}

int main(void) {
  if (fesetenv(FE_DFL_ENV) || fegetround() != FE_TONEAREST ||
      fetestexcept(FE_ALL_EXCEPT)) return 1;
  if (add_half_ulp() != 1.0 || fetestexcept(FE_INEXACT) != FE_INEXACT ||
      fesetround(FE_UPWARD) || add_half_ulp() != 1.0 + DBL_EPSILON) return 2;
  if (fesetround(1) == 0 || fegetround() != FE_UPWARD ||
      feclearexcept(FE_ALL_EXCEPT)) return 3;

  fenv_t saved;
  fexcept_t flags;
  if (feraiseexcept(FE_DIVBYZERO) || feholdexcept(&saved) ||
      fetestexcept(FE_ALL_EXCEPT) || fegetround() != FE_UPWARD ||
      feraiseexcept(FE_OVERFLOW) ||
      fegetexceptflag(&flags, FE_OVERFLOW | FE_INEXACT) ||
      flags != FE_OVERFLOW || feupdateenv(&saved) ||
      fetestexcept(FE_ALL_EXCEPT) != (FE_DIVBYZERO | FE_OVERFLOW)) return 4;
  if (fesetenv(FE_DFL_ENV) || fesetexceptflag(&flags, FE_ALL_EXCEPT) ||
      fetestexcept(FE_ALL_EXCEPT) != FE_OVERFLOW) return 5;
  if (fesetenv(&saved) || fegetround() != FE_UPWARD ||
      fetestexcept(FE_ALL_EXCEPT) != FE_DIVBYZERO) return 6;
  return fesetenv(FE_DFL_ENV);
}
