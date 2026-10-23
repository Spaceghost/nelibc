#ifndef NELIBC_THREAD_H
#define NELIBC_THREAD_H

#include <stddef.h>

/* Linux x86_64 standalone bootstrap API. Handles may be active concurrently;
   join services serialized process-wide credential requests. */
typedef struct nelibc_thread {
  int (*start)(void *);
  void *arg;
  int result;
  void *stack;
  void *tls;
  size_t tls_size;
  volatile int tid;
  int state;
  size_t stack_size;
  void *cleanup_stack;
  size_t cleanup_stack_size;
  int *tid_word;
} nelibc_thread;

/* Initialize the handle to zero before its first create. */
int nelibc_thread_create(nelibc_thread *thread, int (*start)(void *), void *arg);
/* Returns 0 after join and writes the callback result; returns -1 on error. */
int nelibc_thread_join(nelibc_thread *thread, int *result);
/* Synchronous convenience wrapper around create and join. */
int nelibc_thread_run_once(int (*start)(void *), void *arg);

#endif
