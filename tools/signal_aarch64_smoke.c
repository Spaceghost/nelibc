#include <errno.h>
#include <setjmp.h>
#include <signal.h>

static volatile sig_atomic_t deliveries;
static sigjmp_buf signal_jump;

_Static_assert(__builtin_offsetof(struct __jmp_buf_tag, __fl) == 176 &&
               __builtin_offsetof(struct __jmp_buf_tag, __ss) == 184,
               "AArch64 jmp_buf offsets");

static void jump_from_signal(int signum) {
  siglongjmp(signal_jump, signum == SIGUSR2 ? -7 : 1);
}

static __attribute__((noinline, noreturn)) void jump_with_changed_registers(jmp_buf jump) {
  __asm__ volatile("mov x19, xzr\n\tfmov d8, xzr" : : : "x19", "v8");
  longjmp(jump, 42);
}

static int check_saved_registers(void) {
  jmp_buf jump;
  __asm__ volatile("mov x19, #17\n\tfmov d8, x19" : : : "x19", "v8");
  int result = setjmp(jump);
  if (!result) jump_with_changed_registers(jump);
  unsigned long integer, vector;
  __asm__ volatile("mov %0, x19\n\tfmov %1, d8" : "=r"(integer), "=r"(vector));
  return result == 42 && integer == 17 && vector == 17;
}

static int check_jump(void) {
  jmp_buf jump;
  int result = _setjmp(jump);
  if (!result) _longjmp(jump, 0);
  if (result != 1) return 0;

  struct sigaction action = {.sa_handler = jump_from_signal}, original;
  sigset_t selected, previous, mask;
  if (sigemptyset(&action.sa_mask) || sigaction(SIGUSR2, &action, &original) ||
      sigemptyset(&selected) || sigaddset(&selected, SIGUSR2) ||
      sigprocmask(SIG_UNBLOCK, &selected, &previous)) return 0;

  result = sigsetjmp(signal_jump, 1);
  if (!result) {
    raise(SIGUSR2);
    return 0;
  }
  if (result != -7 || sigprocmask(SIG_SETMASK, 0, &mask) ||
      sigismember(&mask, SIGUSR2)) return 0;

  result = sigsetjmp(signal_jump, 0);
  if (!result) {
    raise(SIGUSR2);
    return 0;
  }
  return result == -7 && !sigprocmask(SIG_SETMASK, 0, &mask) &&
         sigismember(&mask, SIGUSR2) == 1 &&
         !sigaction(SIGUSR2, &original, 0) &&
         !sigprocmask(SIG_SETMASK, &previous, 0);
}

static void handle_user_signal(int signum, siginfo_t *info, void *context) {
  if (signum == SIGUSR1 && info && info->si_signo == SIGUSR1 && context)
    ++deliveries;
}

int main(void) {
  struct sigaction action = {.sa_sigaction = handle_user_signal,
                            .sa_flags = SA_SIGINFO | SA_RESTART};
  struct sigaction original, active;
  sigset_t block, previous;
  if (sigemptyset(&action.sa_mask) || sigaddset(&action.sa_mask, SIGUSR2) ||
      sigaction(SIGUSR1, &action, &original)) return 1;
  if (sigaction(SIGUSR1, 0, &active) ||
      active.sa_sigaction != handle_user_signal ||
      (active.sa_flags & (SA_SIGINFO | SA_RESTART)) != (SA_SIGINFO | SA_RESTART) ||
      sigismember(&active.sa_mask, SIGUSR2) != 1) return 2;

  if (sigemptyset(&block) || sigaddset(&block, SIGUSR1) ||
      sigprocmask(SIG_BLOCK, &block, &previous) ||
      raise(SIGUSR1) || deliveries != 0) return 3;
  if (sigprocmask(SIG_SETMASK, &previous, 0) || deliveries != 1 ||
      raise(SIGUSR1) || deliveries != 2) return 4;

  if (sigaction(SIGUSR1, &original, 0) || sigaction(SIGUSR1, 0, &active) ||
      active.sa_handler != original.sa_handler) return 5;
  if (sigaction(0, &action, 0) != -1 || errno != EINVAL) return 6;
  if (!check_jump()) return 7;
  return check_saved_registers() ? 0 : 8;
}
