#ifndef NELIBC_SYS_SHM_WRAPPER_H
#define NELIBC_SYS_SHM_WRAPPER_H

/* Hybrid builds use the host's <sys/shm.h>. The standalone profile: src/ipc_adapter.c, the kernel's x86-64 struct shmid64_ds. */
#ifdef NELIBC_STANDALONE

#include <sys/ipc.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SHMLBA 4096

#define SHM_RDONLY 010000
#define SHM_RND    020000
#define SHM_REMAP  040000
#define SHM_EXEC   0100000

#define SHM_LOCK     11
#define SHM_UNLOCK   12
#define SHM_STAT     13
#define SHM_INFO     14
#define SHM_STAT_ANY 15
#define SHM_DEST      01000
#define SHM_LOCKED    02000
#define SHM_HUGETLB   04000
#define SHM_NORESERVE 010000

typedef unsigned long shmatt_t;

struct shmid_ds {
  struct ipc_perm shm_perm;
  size_t shm_segsz;
  time_t shm_atime;
  time_t shm_dtime;
  time_t shm_ctime;
  pid_t shm_cpid;
  pid_t shm_lpid;
  unsigned long shm_nattch;
  unsigned long __pad1;
  unsigned long __pad2;
};

void *shmat(int, const void *, int);
int shmctl(int, int, struct shmid_ds *);
int shmdt(const void *);
int shmget(key_t, size_t, int);

#ifdef __cplusplus
}
#endif

#else

#include_next <sys/shm.h>

#endif

#endif
