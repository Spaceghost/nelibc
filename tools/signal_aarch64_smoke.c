#include <errno.h>
#include <signal.h>

static volatile sig_atomic_t deliveries;

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
  return sigaction(0, &action, 0) == -1 && errno == EINVAL ? 0 : 6;
}
