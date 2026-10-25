#ifndef NELIBC_SPAWN_WRAPPER_H
#define NELIBC_SPAWN_WRAPPER_H

#ifdef NELIBC_STANDALONE

#include <sys/types.h>

/* These opaque handles are reserved for the standalone spawn implementation. */
typedef struct { unsigned char __opaque[128]; } posix_spawnattr_t;
typedef struct { unsigned char __opaque[64]; } posix_spawn_file_actions_t;

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

#endif
