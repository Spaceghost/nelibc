/* Synchronization and pointer publication for the standalone allocator. */
#include <stdatomic.h>

static atomic_uint allocator_lock;
static atomic_uint direct_lock;
static _Atomic(void *) slab_head;
static _Atomic(void *) direct_head;

static void acquire(atomic_uint *lock) {
  while (atomic_exchange_explicit(lock, 1, memory_order_acquire))
    while (atomic_load_explicit(lock, memory_order_relaxed))
      __asm__ volatile("yield");
}

static void release(atomic_uint *lock) {
  atomic_store_explicit(lock, 0, memory_order_release);
}

void nelibc_allocator_lock(void) { acquire(&allocator_lock); }
void nelibc_allocator_unlock(void) { release(&allocator_lock); }

void nelibc_allocator_slab_registry_publish(void *head) {
  atomic_store_explicit(&slab_head, head, memory_order_release);
}

void *nelibc_allocator_slab_registry(void) {
  return atomic_load_explicit(&slab_head, memory_order_acquire);
}

void nelibc_direct_registry_publish(void *head) {
  atomic_store_explicit(&direct_head, head, memory_order_release);
}

void *nelibc_direct_registry_head(void) {
  return atomic_load_explicit(&direct_head, memory_order_acquire);
}

/* A writer can unmap a direct allocation after its last reader leaves. */
void nelibc_direct_registry_read_enter(void) { acquire(&direct_lock); }
void nelibc_direct_registry_read_leave(void) { release(&direct_lock); }
void nelibc_direct_registry_write_begin(void) { acquire(&direct_lock); }
void nelibc_direct_registry_write_end(void) { release(&direct_lock); }
