#ifndef NELIBC_SYS_MOUNT_H
#define NELIBC_SYS_MOUNT_H

#ifdef NELIBC_STANDALONE

#include <linux/mount.h>
#include "nelibc_syscall.h"

#ifndef MNT_FORCE
#define MNT_FORCE 1
#endif
#ifndef MNT_DETACH
#define MNT_DETACH 2
#endif
#ifndef MNT_EXPIRE
#define MNT_EXPIRE 4
#endif
#ifndef UMOUNT_NOFOLLOW
#define UMOUNT_NOFOLLOW 8
#endif

#else

#include_next <sys/mount.h>

#endif

#endif
