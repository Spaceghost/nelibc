#ifndef NELIBC_SYS_SEM_WRAPPER_H
#define NELIBC_SYS_SEM_WRAPPER_H

/* Hybrid builds use the host's <sys/sem.h>. The standalone profile: src/ipc_adapter.c, the kernel's x86-64 struct semid64_ds; union semun is for the program to declare, as POSIX says. */
#ifdef NELIBC_STANDALONE

#include <sys/ipc.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SEM_UNDO 0x1000
#define GETPID  11
#define GETVAL  12
#define GETALL  13
#define GETNCNT 14
#define GETZCNT 15
#define SETVAL  16
#define SETALL  17
#define SEM_STAT 18
#define SEM_INFO 19
#define SEM_STAT_ANY 20

struct semid_ds {
  struct ipc_perm sem_perm;
  time_t sem_otime;
  long __unused1;
  time_t sem_ctime;
  long __unused2;
  unsigned short sem_nsems;
  char __sem_nsems_pad[sizeof(long) - sizeof(short)];
  long __unused3;
  long __unused4;
};

struct sembuf {
  unsigned short sem_num;
  short sem_op;
  short sem_flg;
};

struct seminfo {
  int semmap, semmni, semmns, semmnu, semmsl, semopm, semume, semusz, semvmx, semaem;
};

struct timespec;
int semctl(int, int, int, ...);
int semget(key_t, int, int);
int semop(int, struct sembuf *, size_t);
int semtimedop(int, struct sembuf *, size_t, const struct timespec *);

#ifdef __cplusplus
}
#endif

#else

#include_next <sys/sem.h>

#endif

#endif
