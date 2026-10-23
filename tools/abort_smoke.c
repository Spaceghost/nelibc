#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>

int main(void) {
  pid_t exit_child = fork();
  if (exit_child < 0) return 1;
  if (exit_child == 0) _Exit(37);

  int status = 0;
  if (wait(&status) != exit_child ||
      !WIFEXITED(status) || WEXITSTATUS(status) != 37)
    return 2;

  pid_t usage_child = fork();
  if (usage_child < 0) return 3;
  if (usage_child == 0) _Exit(23);

  struct rusage usage = {0};
  status = 0;
  if (wait4(usage_child, &status, 0, &usage) != usage_child ||
      !WIFEXITED(status) || WEXITSTATUS(status) != 23 ||
      usage.ru_utime.tv_sec < 0 || usage.ru_stime.tv_sec < 0)
    return 4;

  pid_t child = fork();
  if (child < 0) return 5;
  if (child == 0) {
    abort();
    _Exit(2);
  }

  status = 0;
  if (waitpid(child, &status, 0) != child) return 6;
  return WIFSIGNALED(status) && WTERMSIG(status) == SIGABRT ? 0 : 7;
}
