#include <fenv.h>
#include <float.h>
#include <stdint.h>

enum {
  ROUNDING_MASK = FE_TOWARDZERO,
  TRAP_SHIFT = 8,
  TRAP_MASK = FE_ALL_EXCEPT << TRAP_SHIFT,
  INPUT_DENORMAL_TRAP = 1 << 15
};

static unsigned int control(void) {
  uint64_t value;
  __asm__ volatile("mrs %0, fpcr" : "=r"(value) : : "memory");
  return (unsigned int)value;
}

static unsigned int status(void) {
  uint64_t value;
  __asm__ volatile("mrs %0, fpsr" : "=r"(value) : : "memory");
  return (unsigned int)value;
}

static void set_control(unsigned int value) {
  __asm__ volatile("msr fpcr, %0" : : "r"((uint64_t)value) : "memory");
}

static void set_status(unsigned int value) {
  __asm__ volatile("msr fpsr, %0" : : "r"((uint64_t)value) : "memory");
}

int feclearexcept(int exceptions) {
  set_status(status() & ~(exceptions & FE_ALL_EXCEPT));
  return 0;
}

int fegetexceptflag(fexcept_t *flag, int exceptions) {
  if (!flag) return 1;
  *flag = status() & exceptions & FE_ALL_EXCEPT;
  return 0;
}

static void divide(float left, float right) {
  float discarded;
  __asm__ volatile("fdiv %s0, %s1, %s2" : "=w"(discarded) : "w"(left), "w"(right));
}

static void multiply(float left, float right) {
  float discarded;
  __asm__ volatile("fmul %s0, %s1, %s2" : "=w"(discarded) : "w"(left), "w"(right));
}

int feraiseexcept(int exceptions) {
  unsigned int flags = exceptions & FE_ALL_EXCEPT;
  unsigned int enabled = flags & (control() >> TRAP_SHIFT);
  unsigned int raised = status() | flags;
  if (enabled & FE_INVALID) divide(0.0f, 0.0f);
  if (enabled & FE_DIVBYZERO) divide(1.0f, 0.0f);
  if (enabled & FE_OVERFLOW) multiply(FLT_MAX, 2.0f);
  if (enabled & FE_UNDERFLOW) multiply(FLT_MIN, FLT_MIN);
  if (enabled & FE_INEXACT) divide(1.0f, 3.0f);
  set_status(raised);
  return 0;
}

int fesetexceptflag(const fexcept_t *flag, int exceptions) {
  if (!flag) return 1;
  unsigned int mask = exceptions & FE_ALL_EXCEPT;
  set_status((status() & ~mask) | (*flag & mask));
  return 0;
}

int fetestexcept(int exceptions) {
  return (int)(status() & exceptions & FE_ALL_EXCEPT);
}

int fegetround(void) {
  return (int)(control() & ROUNDING_MASK);
}

int fesetround(int rounding) {
  if (rounding & ~ROUNDING_MASK) return 1;
  set_control((control() & ~ROUNDING_MASK) | (unsigned int)rounding);
  return 0;
}

int fegetenv(fenv_t *environment) {
  if (!environment) return 1;
  environment->__fpcr = control();
  environment->__fpsr = status();
  return 0;
}

int feholdexcept(fenv_t *environment) {
  if (fegetenv(environment)) return 1;
  set_control(environment->__fpcr & ~(TRAP_MASK | INPUT_DENORMAL_TRAP));
  set_status(environment->__fpsr & ~FE_ALL_EXCEPT);
  return 0;
}

int fesetenv(const fenv_t *environment) {
  if (!environment) return 1;
  if (environment == FE_DFL_ENV || environment == FE_NOMASK_ENV) {
    unsigned int wanted = environment == FE_NOMASK_ENV ? TRAP_MASK : 0;
    set_control(wanted);
    set_status(0);
    /* AArch64 cores can implement the trap-enable bits as read-as-zero. */
    return (control() & TRAP_MASK) != wanted;
  }
  set_control(environment->__fpcr);
  set_status(environment->__fpsr);
  return 0;
}

int feupdateenv(const fenv_t *environment) {
  int raised = fetestexcept(FE_ALL_EXCEPT);
  if (fesetenv(environment)) return 1;
  return feraiseexcept(raised);
}
