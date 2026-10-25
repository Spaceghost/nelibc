#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <spawn.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>

typedef struct {
  int error;
} spawn_worker_state;

static void *spawn_worker(void *opaque) {
  spawn_worker_state *state = opaque;
  posix_spawn_file_actions_t actions;
  if (posix_spawn_file_actions_init(&actions) != 0 ||
      posix_spawn_file_actions_addopen(&actions, 9, "/dev/null", O_RDONLY, 0) != 0) {
    state->error = 1;
    return 0;
  }
  char *self_arguments[] = {(char *)"spawn-smoke", (char *)"--fd9-open", 0};
  char *true_arguments[] = {(char *)"true", 0};
  char *environment[] = {(char *)"PATH=/bin:/usr/bin", 0};
  for (int iteration = 0; iteration < 4; ++iteration) {
    pid_t child = -1;
    int error;
    if ((iteration & 1) == 0)
      error = posix_spawn(&child, "/proc/self/exe", &actions, 0,
                          self_arguments, environment);
    else
      error = posix_spawnp(&child, "true", &actions, 0,
                           true_arguments, environment);
    if (error != 0 || child <= 0) {
      state->error = 2;
      break;
    }
    int status = 0;
    if (waitpid(child, &status, 0) != child || !WIFEXITED(status) ||
        WEXITSTATUS(status) != 0) {
      state->error = 3;
      break;
    }
  }
  if (posix_spawn_file_actions_destroy(&actions) != 0 && state->error == 0)
    state->error = 4;
  return 0;
}

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
    if (strcmp(argv[1], "--cwd-tmp") == 0) {
      char current_directory[4096];
      return getcwd(current_directory, sizeof(current_directory)) != 0 &&
          strcmp(current_directory, "/tmp") == 0 ? 0 : 14;
    }
    if (strcmp(argv[1], "--attr-state") == 0) {
      sigset_t current_mask;
      struct sigaction current_action;
      if (getpgid(0) != getpid() ||
          sigprocmask(SIG_SETMASK, 0, &current_mask) != 0 ||
          sigismember(&current_mask, SIGUSR1) != 1 ||
          sigaction(SIGUSR1, 0, &current_action) != 0 ||
          current_action.sa_handler != SIG_DFL)
        return 13;
      return 0;
    }
    if (strcmp(argv[1], "--sched-fifo") == 0) {
      struct sched_param current_parameter;
      return sched_getscheduler(0) == SCHED_FIFO &&
          sched_getparam(0, &current_parameter) == 0 &&
          current_parameter.sched_priority == 1 ? 0 : 15;
    }
  }
  if (argc == 3 && strcmp(argv[1], "--identity") == 0) {
    if (getuid() != 0) return 40;
    if (strcmp(argv[2], "real") == 0) return geteuid() == 1 ? 0 : 41;
    if (strcmp(argv[2], "reset") == 0) return geteuid() == 0 ? 0 : 42;
    return 43;
  }
  if (argc == 2 && strcmp(argv[1], "--run-resetids") == 0) {
    char *identity_arguments[] = {(char *)"spawn-smoke", (char *)"--identity",
                                  (char *)"real", 0};
    char *environment[] = {0};
    pid_t identity_child = -1;
    int identity_status = 0;
    if (getuid() != 0 || geteuid() != 0 || seteuid(1) != 0 ||
        geteuid() != 1 ||
        posix_spawn(&identity_child, "/proc/self/exe", 0, 0, identity_arguments,
                    environment) != 0 ||
        waitpid(identity_child, &identity_status, 0) != identity_child ||
        !WIFEXITED(identity_status) || WEXITSTATUS(identity_status) != 0)
      return 44;
    posix_spawnattr_t reset_attributes;
    if (posix_spawnattr_init(&reset_attributes) != 0 ||
        posix_spawnattr_setflags(&reset_attributes, POSIX_SPAWN_RESETIDS) != 0)
      return 45;
    identity_arguments[2] = (char *)"reset";
    int spawn_error = posix_spawn(&identity_child, "/proc/self/exe", 0, &reset_attributes,
                                  identity_arguments, environment);
    int destroy_error = posix_spawnattr_destroy(&reset_attributes);
    if (spawn_error != 0) return 50 + spawn_error % 100;
    if (destroy_error != 0) return 47;
    if (waitpid(identity_child, &identity_status, 0) != identity_child) return 48;
    if (!WIFEXITED(identity_status)) return 49;
    if (WEXITSTATUS(identity_status) != 0) return 60 + WEXITSTATUS(identity_status);
    return 0;
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

  errno = 42;
  if (posix_spawn_file_actions_init(&actions) != 0 ||
      posix_spawn_file_actions_addchdir_np(&actions, 0) != EINVAL ||
      posix_spawn_file_actions_addfchdir_np(&actions, -1) != EINVAL || errno != 42 ||
      posix_spawn_file_actions_addchdir_np(&actions, "/") != 0 ||
      posix_spawn_file_actions_addchdir_np(&actions, "tmp") != 0 ||
      run_action_case(&actions, "--cwd-tmp") != 0 ||
      posix_spawn_file_actions_destroy(&actions) != 0)
    return 23;

  if (posix_spawn_file_actions_init(&actions) != 0) return 24;
  int directory_fd = open("/tmp", O_RDONLY | O_DIRECTORY);
  if (directory_fd < 0 ||
      posix_spawn_file_actions_addfchdir_np(&actions, directory_fd) != 0 ||
      run_action_case(&actions, "--cwd-tmp") != 0 || close(directory_fd) != 0 ||
      posix_spawn_file_actions_destroy(&actions) != 0)
    return 25;

  if (posix_spawn_file_actions_init(&actions) != 0) return 28;
  int regular_file_fd = open("/dev/null", O_RDONLY);
  if (regular_file_fd < 0 ||
      posix_spawn_file_actions_addfchdir_np(&actions, regular_file_fd) != 0)
    return 29;
  errno = 42;
  result = posix_spawn(&child, "/bin/true", &actions, 0, arguments, environment);
  if (result != ENOTDIR || errno != 42 || close(regular_file_fd) != 0 ||
      posix_spawn_file_actions_destroy(&actions) != 0)
    return 30;

  if (posix_spawn_file_actions_init(&actions) != 0 ||
      posix_spawn_file_actions_addchdir_np(&actions, "/nelibc-spawn-missing") != 0)
    return 26;
  errno = 42;
  result = posix_spawn(&child, "/bin/true", &actions, 0, arguments, environment);
  if (result != ENOENT || errno != 42 ||
      posix_spawn_file_actions_destroy(&actions) != 0)
    return 27;

  if (posix_spawn_file_actions_init(&actions) != 0 ||
      posix_spawn_file_actions_addopen(&actions, 9, "/nelibc-spawn-missing", O_RDONLY, 0) != 0)
    return 17;
  errno = 42;
  result = posix_spawn(&child, "/bin/true", &actions, 0, arguments, environment);
  if (result != ENOENT || errno != 42 ||
      posix_spawn_file_actions_destroy(&actions) != 0)
    return 18;

  posix_spawnattr_t attributes;
  short spawn_flags = 0;
  int spawn_policy = -1;
  struct sched_param spawn_parameter = {-1};
  pid_t process_group = -1;
  sigset_t signal_mask, signal_defaults;
  if (posix_spawnattr_init(&attributes) != 0 ||
      posix_spawnattr_setschedparam(&attributes, &spawn_parameter) != EINVAL)
    return 19;
  spawn_parameter.sched_priority = 0;
  if (
      posix_spawnattr_getflags(&attributes, &spawn_flags) != 0 || spawn_flags != 0 ||
      posix_spawnattr_getpgroup(&attributes, &process_group) != 0 || process_group != 0 ||
      sigemptyset(&signal_mask) != 0 || sigaddset(&signal_mask, SIGUSR1) != 0 ||
      sigemptyset(&signal_defaults) != 0 || sigaddset(&signal_defaults, SIGUSR1) != 0 ||
      posix_spawnattr_setsigmask(&attributes, &signal_mask) != 0 ||
      posix_spawnattr_setsigdefault(&attributes, &signal_defaults) != 0 ||
      posix_spawnattr_getsigmask(&attributes, &signal_mask) != 0 ||
      sigismember(&signal_mask, SIGUSR1) != 1 ||
      posix_spawnattr_getsigdefault(&attributes, &signal_defaults) != 0 ||
      sigismember(&signal_defaults, SIGUSR1) != 1 ||
      posix_spawnattr_setpgroup(&attributes, 0) != 0 ||
      posix_spawnattr_setpgroup(&attributes, -1) != EINVAL ||
      posix_spawnattr_setschedpolicy(&attributes, SCHED_OTHER) != 0 ||
      posix_spawnattr_getschedpolicy(&attributes, &spawn_policy) != 0 ||
      spawn_policy != SCHED_OTHER ||
      posix_spawnattr_setschedparam(&attributes, &spawn_parameter) != 0 ||
      posix_spawnattr_getschedparam(&attributes, &spawn_parameter) != 0 ||
      spawn_parameter.sched_priority != 0 ||
      posix_spawnattr_setflags(&attributes, POSIX_SPAWN_SETPGROUP |
          POSIX_SPAWN_SETSIGMASK | POSIX_SPAWN_SETSIGDEF |
          POSIX_SPAWN_SETSCHEDULER | POSIX_SPAWN_RESETIDS) != 0 ||
      posix_spawnattr_setflags(&attributes, 0x40) != EINVAL)
    return 22;
  struct sigaction ignored_action = {0};
  struct sigaction previous_action;
  ignored_action.sa_handler = SIG_IGN;
  if (sigaction(SIGUSR1, &ignored_action, &previous_action) != 0) return 20;
  char *attribute_arguments[] = {(char *)"spawn-smoke", (char *)"--attr-state", 0};
  errno = 42;
  result = posix_spawn(&child, "/proc/self/exe", 0, &attributes,
                       attribute_arguments, environment);
  int restore_result = sigaction(SIGUSR1, &previous_action, 0);
  if (result != 0 || errno != 42 || child <= 0 || restore_result != 0 ||
      waitpid(child, &status, 0) != child || !WIFEXITED(status) ||
      WEXITSTATUS(status) != 0 || posix_spawnattr_destroy(&attributes) != 0)
    return 21;

  if (posix_spawnattr_init(&attributes) != 0 ||
      posix_spawnattr_setschedpolicy(&attributes, SCHED_FIFO) != 0)
    return 31;
  spawn_parameter.sched_priority = 1;
  if (posix_spawnattr_setschedparam(&attributes, &spawn_parameter) != 0 ||
      posix_spawnattr_setflags(&attributes, POSIX_SPAWN_SETSCHEDULER) != 0)
    return 32;
  char *fifo_arguments[] = {(char *)"spawn-smoke", (char *)"--sched-fifo", 0};
  errno = 42;
  result = posix_spawn(&child, "/proc/self/exe", 0, &attributes,
                       fifo_arguments, environment);
  int fifo_errno = errno;
  if (posix_spawnattr_destroy(&attributes) != 0 ||
      (result == 0 && (fifo_errno != 42 || child <= 0 ||
       waitpid(child, &status, 0) != child || !WIFEXITED(status) ||
       WEXITSTATUS(status) != 0)) ||
      (result != 0 && (result != EPERM || fifo_errno != 42)))
    return 33;

  spawn_worker_state worker_states[4] = {{0}, {0}, {0}, {0}};
  pthread_t workers[4];
  int created_workers = 0;
  for (; created_workers < 4; ++created_workers) {
    if (pthread_create(&workers[created_workers], 0, spawn_worker,
                       &worker_states[created_workers]) != 0)
      break;
  }
  if (created_workers != 4) return 34;
  for (int worker_index = 0; worker_index < 4; ++worker_index) {
    if (pthread_join(workers[worker_index], 0) != 0 ||
        worker_states[worker_index].error != 0)
      return 35;
  }
  return 0;
}
