extern long nelibc_raw_syscall6(unsigned long, unsigned long, unsigned long,
                                unsigned long, unsigned long, unsigned long,
                                unsigned long);

static int current_tid(void) {
  return (int)nelibc_raw_syscall6(186, 0, 0, 0, 0, 0, 0);
}

void nelibc_stdio_lock(int *word, int *owner, int *depth) {
  int tid = current_tid();
  if (__atomic_load_n(owner, __ATOMIC_ACQUIRE) == tid) {
    __atomic_add_fetch(depth, 1, __ATOMIC_RELAXED);
    return;
  }
  for (;;) {
    int expected = 0;
    if (__atomic_compare_exchange_n(word, &expected, 1, 0,
                                    __ATOMIC_ACQUIRE, __ATOMIC_RELAXED)) {
      __atomic_store_n(depth, 1, __ATOMIC_RELAXED);
      __atomic_store_n(owner, tid, __ATOMIC_RELEASE);
      return;
    }
    while (__atomic_load_n(word, __ATOMIC_RELAXED) != 0)
      nelibc_raw_syscall6(202, (unsigned long)word, 128, 1, 0, 0, 0);
  }
}

int nelibc_stdio_trylock(int *word, int *owner, int *depth) {
  int tid = current_tid();
  if (__atomic_load_n(owner, __ATOMIC_ACQUIRE) == tid) {
    __atomic_add_fetch(depth, 1, __ATOMIC_RELAXED);
    return 0;
  }
  int expected = 0;
  if (!__atomic_compare_exchange_n(word, &expected, 1, 0,
                                   __ATOMIC_ACQUIRE, __ATOMIC_RELAXED))
    return 1;
  __atomic_store_n(depth, 1, __ATOMIC_RELAXED);
  __atomic_store_n(owner, tid, __ATOMIC_RELEASE);
  return 0;
}

void nelibc_stdio_unlock(int *word, int *owner, int *depth) {
  if (__atomic_load_n(owner, __ATOMIC_ACQUIRE) != current_tid()) return;
  if (__atomic_sub_fetch(depth, 1, __ATOMIC_RELAXED) != 0) return;
  __atomic_store_n(owner, 0, __ATOMIC_RELEASE);
  __atomic_store_n(word, 0, __ATOMIC_RELEASE);
  nelibc_raw_syscall6(202, (unsigned long)word, 129, 1, 0, 0, 0);
}
