#include <errno.h>
#include <fcntl.h>
#include <spawn.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>

static int run_action_case(posix_spawn_file_actions_t *actions, const char *mode) {
  char *arguments[] = {(char *)"spawn-smoke", (char *)mode, 0};
  char *environment[] = {0};
  pid_t child = -1;
  int result = posix_spawn(&child, "/proc/self/exe", actions, 0,
                           arguments, environment);
  if (result != 0 || child <= 0) return result ? result : ECHILD;
  int status = 0;
  if (waitpid(child, &status, 0) != child || !WIFEXITED(status)) return ECHILD;
  return WEXITSTATUS(status) == 0 ? 0 : EIO;
}

int main(int argc, char **argv) {
  if (argc == 2) {
    if (strcmp(argv[1], "--fd3-closed") == 0) {
      errno = 0;
      return fcntl(3, F_GETFD) == -1 && errno == EBADF ? 0 : 10;
    }
    if (strcmp(argv[1], "--fd9-open") == 0) return fcntl(9, F_GETFD) >= 0 ? 0 : 11;
    if (strcmp(argv[1], "--fd9-closed") == 0) {
      errno = 0;
      return fcntl(9, F_GETFD) == -1 && errno == EBADF ? 0 : 12;
    }
  }
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

  posix_spawn_file_actions_t actions;
  errno = 42;
  if (posix_spawn_file_actions_init(&actions) != 0 ||
      posix_spawn_file_actions_addclose(&actions, -1) != EINVAL ||
      posix_spawn_file_actions_adddup2(&actions, -1, 9) != EINVAL || errno != 42)
    return 4;
  int inherited_fd = open("/dev/null", O_RDONLY);
  if (inherited_fd < 0 || inherited_fd != 3 ||
      posix_spawn_file_actions_addclose(&actions, inherited_fd) != 0)
    return 5;
  errno = 42;
  result = run_action_case(&actions, "--fd3-closed");
  if (result != 0 || errno != 42 ||
      posix_spawn_file_actions_destroy(&actions) != 0 || close(inherited_fd) != 0)
    return 6;

  if (posix_spawn_file_actions_init(&actions) != 0) return 7;
  inherited_fd = open("/dev/null", O_RDONLY);
  if (inherited_fd < 0 ||
      posix_spawn_file_actions_adddup2(&actions, inherited_fd, 9) != 0)
    return 8;
  if (run_action_case(&actions, "--fd9-open") != 0 ||
      posix_spawn_file_actions_destroy(&actions) != 0 || close(inherited_fd) != 0)
    return 9;

  if (posix_spawn_file_actions_init(&actions) != 0 ||
      posix_spawn_file_actions_addopen(&actions, 9, "/dev/null", O_RDONLY, 0) != 0)
    return 13;
  if (run_action_case(&actions, "--fd9-open") != 0 ||
      posix_spawn_file_actions_destroy(&actions) != 0)
    return 14;

  if (posix_spawn_file_actions_init(&actions) != 0 ||
      posix_spawn_file_actions_addopen(&actions, 9, "/dev/null", O_RDONLY, 0) != 0 ||
      posix_spawn_file_actions_addclose(&actions, 9) != 0)
    return 15;
  if (run_action_case(&actions, "--fd9-closed") != 0 ||
      posix_spawn_file_actions_destroy(&actions) != 0)
    return 16;

  if (posix_spawn_file_actions_init(&actions) != 0 ||
      posix_spawn_file_actions_addopen(&actions, 9, "/nelibc-spawn-missing", O_RDONLY, 0) != 0)
    return 17;
  errno = 42;
  result = posix_spawn(&child, "/bin/true", &actions, 0, arguments, environment);
  if (result != ENOENT || errno != 42 ||
      posix_spawn_file_actions_destroy(&actions) != 0)
    return 18;
  return 0;
}
