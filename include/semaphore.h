#ifndef NELIBC_SEMAPHORE_H
#define NELIBC_SEMAPHORE_H

#ifndef NELIBC_STANDALONE
#include_next <semaphore.h>
#else

#include <stdint.h>
struct timespec;

typedef struct {
  uintptr_t __magic;
  int __value;
  int __sequence;
  int __waiters;
} sem_t;

#define SEM_FAILED ((sem_t *)0)
#define SEM_VALUE_MAX 2147483647

int sem_init(sem_t *sem, int pshared, unsigned int value);
int sem_destroy(sem_t *sem);
int sem_wait(sem_t *sem);
int sem_timedwait(sem_t *sem, const struct timespec *abstime);
int sem_trywait(sem_t *sem);
int sem_post(sem_t *sem);
int sem_getvalue(sem_t *sem, int *value);

#endif
#endif
