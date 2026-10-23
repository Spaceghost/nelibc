#ifndef NELIBC_SYS_TYPES_WRAPPER_H
#define NELIBC_SYS_TYPES_WRAPPER_H

#ifdef NELIBC_STANDALONE

/* Linux x86-64 public scalar types used by Nelibc's standalone headers. */
#ifndef __blkcnt_t_defined
typedef long blkcnt_t;
#define __blkcnt_t_defined 1
#endif
#ifndef __blksize_t_defined
typedef long blksize_t;
#define __blksize_t_defined 1
#endif
#ifndef __clock_t_defined
typedef long clock_t;
#define __clock_t_defined 1
#endif
#ifndef __clockid_t_defined
typedef int clockid_t;
#define __clockid_t_defined 1
#endif
#ifndef __dev_t_defined
typedef unsigned long dev_t;
#define __dev_t_defined 1
#endif
#ifndef __gid_t_defined
typedef unsigned int gid_t;
#define __gid_t_defined 1
#endif
#ifndef __id_t_defined
typedef unsigned int id_t;
#define __id_t_defined 1
#endif
#ifndef __ino_t_defined
typedef unsigned long ino_t;
#define __ino_t_defined 1
#endif
#ifndef __key_t_defined
typedef int key_t;
#define __key_t_defined 1
#endif
#ifndef __mode_t_defined
typedef unsigned int mode_t;
#define __mode_t_defined 1
#endif
#ifndef __nlink_t_defined
typedef unsigned long nlink_t;
#define __nlink_t_defined 1
#endif
#ifndef __off_t_defined
typedef long off_t;
#define __off_t_defined 1
#endif
#ifndef __pid_t_defined
typedef int pid_t;
#define __pid_t_defined 1
#endif
#ifndef __ssize_t_defined
typedef long ssize_t;
#define __ssize_t_defined 1
#endif
#ifndef __suseconds_t_defined
typedef long suseconds_t;
#define __suseconds_t_defined 1
#endif
#ifndef __time_t_defined
typedef long time_t;
#define __time_t_defined 1
#endif
#ifndef __uid_t_defined
typedef unsigned int uid_t;
#define __uid_t_defined 1
#endif
#ifndef __useconds_t_defined
typedef unsigned int useconds_t;
#define __useconds_t_defined 1
#endif
#ifndef __fsblkcnt_t_defined
typedef unsigned long fsblkcnt_t;
#define __fsblkcnt_t_defined 1
#endif
#ifndef __fsfilcnt_t_defined
typedef unsigned long fsfilcnt_t;
#define __fsfilcnt_t_defined 1
#endif
#ifndef __socklen_t_defined
typedef unsigned int socklen_t;
#define __socklen_t_defined 1
#endif

#else

#include_next <sys/types.h>

#endif

#endif
