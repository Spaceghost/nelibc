#ifndef NELIBC_FENV_H
#define NELIBC_FENV_H

#ifdef NELIBC_STANDALONE

/* x86-64 FXSAVE state. The alignment is required by FXSAVE/FXRSTOR. */
typedef unsigned short fexcept_t;
typedef struct __attribute__((aligned(16))) {
  unsigned char __state[512];
} fenv_t;

#define FE_INVALID    0x01
#define FE_DIVBYZERO  0x04
#define FE_OVERFLOW   0x08
#define FE_UNDERFLOW  0x10
#define FE_INEXACT    0x20
#define FE_ALL_EXCEPT (FE_INVALID | FE_DIVBYZERO | FE_OVERFLOW | FE_UNDERFLOW | FE_INEXACT)

#define FE_TONEAREST  0x0000
#define FE_DOWNWARD   0x0400
#define FE_UPWARD     0x0800
#define FE_TOWARDZERO 0x0c00

#define FE_DFL_ENV ((const fenv_t *)(-1))
/* GNU-compatible environment with all IEEE exception traps unmasked. */
#define FE_NOMASK_ENV ((const fenv_t *)(-2))

int feclearexcept(int exceptions);
int fegetexceptflag(fexcept_t *flag, int exceptions);
int feraiseexcept(int exceptions);
int fesetexceptflag(const fexcept_t *flag, int exceptions);
int fetestexcept(int exceptions);
int fegetround(void);
int fesetround(int rounding);
int fegetenv(fenv_t *environment);
int feholdexcept(fenv_t *environment);
int fesetenv(const fenv_t *environment);
int feupdateenv(const fenv_t *environment);

#else

#include_next <fenv.h>

#endif

#endif
