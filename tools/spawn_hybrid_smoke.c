#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <spawn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

static int run_child(char *const arguments[],
                     const posix_spawn_file_actions_t *actions) {
  pid_t child = -1;
  char *environment[] = {(char *)"PATH=/bin:/usr/bin", NULL};
  int error = posix_spawn(&child, "/proc/self/exe", actions, NULL,
                          arguments, environment);
  if (error != 0 || child <= 0) return error ? error : ECHILD;
  int status = 0;
  if (waitpid(child, &status, 0) != child || !WIFEXITED(status)) return ECHILD;
  return WEXITSTATUS(status) == 0 ? 0 : EIO;
}

int main(int argc, char **argv) {
  if (argc == 2 && strcmp(argv[1], "--fd9") == 0)
    return fcntl(9, F_GETFD) >= 0 ? 0 : 1;
  if (argc == 2 && strcmp(argv[1], "--cwd") == 0) {
    char cwd[4096];
    return getcwd(cwd, sizeof(cwd)) && strcmp(cwd, "/tmp") == 0 ? 0 : 2;
  }

  char *true_arguments[] = {(char *)"true", NULL};
  char *environment[] = {(char *)"PATH=/bin:/usr/bin", NULL};
  pid_t child = -1;
  int error = posix_spawnp(&child, "true", NULL, NULL, true_arguments,
                           environment);
  int status = 0;
  if (error != 0 || child <= 0 || waitpid(child, &status, 0) != child ||
      !WIFEXITED(status) || WEXITSTATUS(status) != 0)
    return 10;

  posix_spawn_file_actions_t actions;
  if (posix_spawn_file_actions_init(&actions) != 0 ||
      posix_spawn_file_actions_addopen(&actions, 9, "/dev/null", O_RDONLY, 0) != 0)
    return 11;
  char *fd_arguments[] = {(char *)"spawn-hybrid-smoke", (char *)"--fd9", NULL};
  int result = run_child(fd_arguments, &actions);
  if (posix_spawn_file_actions_destroy(&actions) != 0 || result != 0)
    return 12;

  if (posix_spawn_file_actions_init(&actions) != 0 ||
      posix_spawn_file_actions_addchdir_np(&actions, "/tmp") != 0)
    return 13;
  char *cwd_arguments[] = {(char *)"spawn-hybrid-smoke", (char *)"--cwd", NULL};
  result = run_child(cwd_arguments, &actions);
  if (posix_spawn_file_actions_destroy(&actions) != 0 || result != 0)
    return 14;

  if (posix_spawn_file_actions_init(&actions) != 0 ||
      posix_spawn_file_actions_addopen(&actions, 9, "/nelibc-spawn-missing",
                                       O_RDONLY, 0) != 0)
    return 15;
  child = -1;
  char *arguments[] = {(char *)"true", NULL};
  error = posix_spawn(&child, "/bin/true", &actions, NULL, arguments,
                      environment);
  int destroy_error = posix_spawn_file_actions_destroy(&actions);
  if (error != ENOENT || child != -1 || destroy_error != 0) return 16;
  return 0;
}
