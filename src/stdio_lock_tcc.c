#include <pthread.h>

static pthread_mutex_t nelibc_stdio_hybrid_mutex;
static pthread_once_t nelibc_stdio_hybrid_once = PTHREAD_ONCE_INIT;

static void nelibc_stdio_hybrid_initialize(void) {
  pthread_mutexattr_t attributes;
  (void)pthread_mutexattr_init(&attributes);
  (void)pthread_mutexattr_settype(&attributes, PTHREAD_MUTEX_RECURSIVE);
  (void)pthread_mutex_init(&nelibc_stdio_hybrid_mutex, &attributes);
  (void)pthread_mutexattr_destroy(&attributes);
}

static void nelibc_stdio_hybrid_ready(void) {
  (void)pthread_once(&nelibc_stdio_hybrid_once, nelibc_stdio_hybrid_initialize);
}

void nelibc_stdio_lock(int *word, int *owner, int *depth) {
  (void)word;
  (void)owner;
  (void)depth;
  nelibc_stdio_hybrid_ready();
  (void)pthread_mutex_lock(&nelibc_stdio_hybrid_mutex);
}

int nelibc_stdio_trylock(int *word, int *owner, int *depth) {
  (void)word;
  (void)owner;
  (void)depth;
  nelibc_stdio_hybrid_ready();
  return pthread_mutex_trylock(&nelibc_stdio_hybrid_mutex) == 0 ? 0 : 1;
}

void nelibc_stdio_unlock(int *word, int *owner, int *depth) {
  (void)word;
  (void)owner;
  (void)depth;
  (void)pthread_mutex_unlock(&nelibc_stdio_hybrid_mutex);
}
