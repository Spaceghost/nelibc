#include <errno.h>
#include <spawn.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(void) {
  char *arguments[] = {(char *)"true", 0};
  char *environment[] = {0};
  pid_t child = -1;
  int status = 0;

  errno = 42;
  int result = posix_spawn(&child, "/bin/true", 0, 0, arguments, environment);
  if (result != 0 || errno != 42 || child <= 0 ||
      waitpid(child, &status, 0) != child || !WIFEXITED(status) ||
      WEXITSTATUS(status) != 0)
    return 1;

  errno = 42;
  result = posix_spawn(&child, "/nelibc-spawn-missing", 0, 0,
                       arguments, environment);
  if (result != ENOENT || errno != 42) return 2;

  errno = 42;
  result = posix_spawnp(&child, "true", 0, 0, arguments, environment);
  if (result != 0 || errno != 42 || child <= 0 ||
      waitpid(child, &status, 0) != child || !WIFEXITED(status) ||
      WEXITSTATUS(status) != 0)
    return 3;

  posix_spawn_file_actions_t unsupported_actions = {{0}};
  errno = 42;
  result = posix_spawn(&child, "/bin/true", &unsupported_actions, 0,
                       arguments, environment);
  if (result != ENOTSUP || errno != 42) return 4;
  return 0;
}
