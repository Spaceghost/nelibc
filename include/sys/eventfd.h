#ifndef NELIBC_SYS_EVENTFD_WRAPPER_H
#define NELIBC_SYS_EVENTFD_WRAPPER_H

#ifdef NELIBC_STANDALONE
#include <stdint.h>
typedef uint64_t eventfd_t;
#ifndef EFD_SEMAPHORE
#define EFD_SEMAPHORE 1
#endif
#ifndef EFD_CLOEXEC
#define EFD_CLOEXEC 02000000
#endif
#ifndef EFD_NONBLOCK
#define EFD_NONBLOCK 00004000
#endif
#else
#include_next <sys/eventfd.h>
#endif

#ifndef EFD_SEMAPHORE
#define EFD_SEMAPHORE 1
#endif
#ifndef EFD_CLOEXEC
#define EFD_CLOEXEC 02000000
#endif
#ifndef EFD_NONBLOCK
#define EFD_NONBLOCK 00004000
#endif

int eventfd(unsigned int initial_value, int flags);
int eventfd_read(int fd, eventfd_t *value);
int eventfd_write(int fd, eventfd_t value);

#endif
