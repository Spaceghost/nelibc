#ifndef NELIBC_THREAD_LINUX_AARCH64_H
#define NELIBC_THREAD_LINUX_AARCH64_H

/* Keep pthread state before the ABI thread pointer and TLS image. */
#define NELIBC_AARCH64_THREAD_SLOTS 5
extern unsigned long nelibc_aarch64_pagesize;
void **nelibc_aarch64_thread_slots(void *tp);

struct nelibc_aarch64_tls {
  void *mapping;
  unsigned long size;
  void *tp;
};

/* Allocate a fresh, zeroed TLS block and copy the executable's template.
 * The caller owns the mapping. Returns a negative Linux error on failure. */
long nelibc_aarch64_allocate_tls(struct nelibc_aarch64_tls *tls);

/* Start a kernel thread on a 16-byte-aligned stack with a fresh TLS block.
 * Linux sets tid in the parent and clears it when the thread exits. */
long nelibc_clone_thread(int (*start)(void *), void *arg, void *stack_top,
                         void *tp, int *tid);

#endif
