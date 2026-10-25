#ifndef NELIBC_SPAWN_WRAPPER_H
#define NELIBC_SPAWN_WRAPPER_H

#ifdef NELIBC_STANDALONE

#include <sys/types.h>
#include <signal.h>
#include <sched.h>

/* These handles are private to the standalone profile. */
typedef struct { unsigned char __opaque[128]; } posix_spawnattr_t;
typedef struct {
  void *__first;
  void *__last;
  unsigned int __magic;
  unsigned char __reserved[44];
} posix_spawn_file_actions_t;

#define POSIX_SPAWN_RESETIDS 0x01
#define POSIX_SPAWN_SETPGROUP 0x02
#define POSIX_SPAWN_SETSIGDEF 0x04
#define POSIX_SPAWN_SETSIGMASK 0x08
#define POSIX_SPAWN_SETSCHEDPARAM 0x10
#define POSIX_SPAWN_SETSCHEDULER 0x20

#else

#include_next <spawn.h>

#endif

int posix_spawn(pid_t *restrict pid, const char *restrict path,
                const posix_spawn_file_actions_t *file_actions,
                const posix_spawnattr_t *restrict attr,
                char *const argv[restrict], char *const envp[restrict]);
int posix_spawnp(pid_t *restrict pid, const char *restrict file,
                 const posix_spawn_file_actions_t *file_actions,
                 const posix_spawnattr_t *restrict attr,
                 char *const argv[restrict], char *const envp[restrict]);

#ifdef NELIBC_STANDALONE
int posix_spawn_file_actions_init(posix_spawn_file_actions_t *actions);
int posix_spawn_file_actions_destroy(posix_spawn_file_actions_t *actions);
int posix_spawn_file_actions_addclose(posix_spawn_file_actions_t *actions,
                                      int fd);
int posix_spawn_file_actions_adddup2(posix_spawn_file_actions_t *actions,
                                     int fd, int newfd);
int posix_spawn_file_actions_addopen(posix_spawn_file_actions_t *actions,
                                     int fd, const char *path,
                                     int oflag, mode_t mode);
int posix_spawnattr_init(posix_spawnattr_t *attributes);
int posix_spawnattr_destroy(posix_spawnattr_t *attributes);
int posix_spawnattr_getflags(const posix_spawnattr_t *restrict attributes,
                             short *restrict flags);
int posix_spawnattr_setflags(posix_spawnattr_t *attributes, short flags);
int posix_spawnattr_getpgroup(const posix_spawnattr_t *restrict attributes,
                              pid_t *restrict pgroup);
int posix_spawnattr_setpgroup(posix_spawnattr_t *attributes, pid_t pgroup);
int posix_spawnattr_getsigdefault(const posix_spawnattr_t *restrict attributes,
                                  sigset_t *restrict signals);
int posix_spawnattr_setsigdefault(posix_spawnattr_t *restrict attributes,
                                  const sigset_t *restrict signals);
int posix_spawnattr_getsigmask(const posix_spawnattr_t *restrict attributes,
                               sigset_t *restrict signals);
int posix_spawnattr_setsigmask(posix_spawnattr_t *restrict attributes,
                               const sigset_t *restrict signals);
int posix_spawnattr_getschedpolicy(const posix_spawnattr_t *restrict attributes,
                                   int *restrict policy);
int posix_spawnattr_setschedpolicy(posix_spawnattr_t *attributes, int policy);
int posix_spawnattr_getschedparam(const posix_spawnattr_t *restrict attributes,
                                  struct sched_param *restrict parameter);
int posix_spawnattr_setschedparam(posix_spawnattr_t *restrict attributes,
                                  const struct sched_param *restrict parameter);
#endif

#endif
