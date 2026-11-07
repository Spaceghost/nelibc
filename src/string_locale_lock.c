#ifndef NELIBC_FREESTANDING
#include <pthread.h>
#endif

#ifndef NELIBC_FREESTANDING
static pthread_mutex_t nelibc_locale_cache_mutex = PTHREAD_MUTEX_INITIALIZER;
#endif

char **nelibc_strtok_state(void) {
#if defined(NELIBC_FREESTANDING)
  static _Thread_local char *state __attribute__((tls_model("local-exec")));
#elif defined(__TINYC__)
  static __thread char *state;
#else
  static _Thread_local char *state;
#endif
  return &state;
}

/* strsignal(3) composes "Real-time signal N" and "Unknown signal N" here. */
char *nelibc_strsignal_buffer(void) {
#if defined(NELIBC_FREESTANDING)
  static _Thread_local char buffer[32] __attribute__((tls_model("local-exec")));
#elif defined(__TINYC__)
  static __thread char buffer[32];
#else
  static _Thread_local char buffer[32];
#endif
  return buffer;
}

#ifndef NELIBC_FREESTANDING
void nelibc_locale_cache_lock(void) {
  (void)pthread_mutex_lock(&nelibc_locale_cache_mutex);
}

void nelibc_locale_cache_unlock(void) {
  (void)pthread_mutex_unlock(&nelibc_locale_cache_mutex);
}
#endif
