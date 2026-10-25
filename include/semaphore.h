#ifndef NELIBC_SEMAPHORE_H
#define NELIBC_SEMAPHORE_H

#ifndef NELIBC_STANDALONE
#include_next <semaphore.h>
#if !defined(__GLIBC__)
#include <time.h>
/* Newer POSIX API, absent from older musl headers. */
#ifdef __cplusplus
extern "C" {
#endif
int sem_clockwait(sem_t *sem, clockid_t clock_id,
                  const struct timespec *abstime);
#ifdef __cplusplus
}
#endif
#endif
#else

#include <stdint.h>
#include <time.h>

typedef struct {
  uintptr_t __magic;
  int __value;
  int __sequence;
  int __waiters;
} sem_t;

#define SEM_FAILED ((sem_t *)0)
#ifdef SEM_VALUE_MAX
#undef SEM_VALUE_MAX
#endif
#define SEM_VALUE_MAX 2147483647

int sem_init(sem_t *sem, int pshared, unsigned int value);
int sem_destroy(sem_t *sem);
int sem_wait(sem_t *sem);
int sem_timedwait(sem_t *sem, const struct timespec *abstime);
int sem_clockwait(sem_t *sem, clockid_t clock_id,
                  const struct timespec *abstime);
int sem_trywait(sem_t *sem);
int sem_post(sem_t *sem);
int sem_getvalue(sem_t *sem, int *value);
sem_t *sem_open(const char *name, int oflag, ...);
int sem_close(sem_t *sem);
int sem_unlink(const char *name);

#endif
#endif
