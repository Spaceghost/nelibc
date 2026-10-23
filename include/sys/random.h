#ifndef NELIBC_SYS_RANDOM_WRAPPER_H
#define NELIBC_SYS_RANDOM_WRAPPER_H

#ifdef NELIBC_STANDALONE
#include <stddef.h>
#include <sys/types.h>

#ifndef GRND_NONBLOCK
#define GRND_NONBLOCK 0x0001
#endif
#ifndef GRND_RANDOM
#define GRND_RANDOM 0x0002
#endif
#ifndef GRND_INSECURE
#define GRND_INSECURE 0x0004
#endif
#else
#include_next <sys/random.h>
#endif

/* Add Nelibc's Linux random-source declarations to the standard header. */
#include "nelibc_syscall.h"

#endif
