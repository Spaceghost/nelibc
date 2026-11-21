#ifndef NELIBC_SYS_PRCTL_WRAPPER_H
#define NELIBC_SYS_PRCTL_WRAPPER_H

#ifdef NELIBC_STANDALONE

#include <linux/prctl.h>

#ifdef __cplusplus
extern "C" {
#endif
int prctl(int option, ...);
#ifdef __cplusplus
}
#endif

#else
#include_next <sys/prctl.h>
#endif

#endif
