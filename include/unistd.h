#ifndef NELIBC_UNISTD_WRAPPER_H
#define NELIBC_UNISTD_WRAPPER_H

#include <stddef.h>

/* Hybrid callers keep host declarations while Nelibc replaces functions. */
#ifdef NELIBC_STANDALONE
#ifndef STDIN_FILENO
#define STDIN_FILENO 0
#endif
#ifndef STDOUT_FILENO
#define STDOUT_FILENO 1
#endif
#ifndef STDERR_FILENO
#define STDERR_FILENO 2
#endif

extern char **environ;

#ifndef F_OK
#define F_OK 0
#endif
#ifndef X_OK
#define X_OK 1
#endif
#ifndef W_OK
#define W_OK 2
#endif
#ifndef R_OK
#define R_OK 4
#endif
#ifndef _SC_CLK_TCK
#define _SC_CLK_TCK 2
#endif
#ifndef _POSIX_PRIORITY_SCHEDULING
#define _POSIX_PRIORITY_SCHEDULING 200809L
#endif
#ifndef _POSIX_THREAD_PRIORITY_SCHEDULING
#define _POSIX_THREAD_PRIORITY_SCHEDULING 200809L
#endif
#ifndef _CS_PATH
#define _CS_PATH 0
#endif
#ifndef _PC_LINK_MAX
#define _PC_LINK_MAX 0
#endif
#ifndef _PC_MAX_CANON
#define _PC_MAX_CANON 1
#endif
#ifndef _PC_MAX_INPUT
#define _PC_MAX_INPUT 2
#endif
#ifndef _PC_NAME_MAX
#define _PC_NAME_MAX 3
#endif
#ifndef _PC_PATH_MAX
#define _PC_PATH_MAX 4
#endif
#ifndef _PC_PIPE_BUF
#define _PC_PIPE_BUF 5
#endif
#ifndef _PC_CHOWN_RESTRICTED
#define _PC_CHOWN_RESTRICTED 6
#endif
#ifndef _PC_NO_TRUNC
#define _PC_NO_TRUNC 7
#endif
#ifndef _PC_VDISABLE
#define _PC_VDISABLE 8
#endif
#ifndef _PC_SYNC_IO
#define _PC_SYNC_IO 9
#endif
#ifndef _PC_ASYNC_IO
#define _PC_ASYNC_IO 10
#endif
#ifndef _PC_PRIO_IO
#define _PC_PRIO_IO 11
#endif
#ifndef _PC_SOCK_MAXBUF
#define _PC_SOCK_MAXBUF 12
#endif
#ifndef _PC_FILESIZEBITS
#define _PC_FILESIZEBITS 13
#endif
#ifndef _PC_REC_INCR_XFER_SIZE
#define _PC_REC_INCR_XFER_SIZE 14
#endif
#ifndef _PC_REC_MAX_XFER_SIZE
#define _PC_REC_MAX_XFER_SIZE 15
#endif
#ifndef _PC_REC_MIN_XFER_SIZE
#define _PC_REC_MIN_XFER_SIZE 16
#endif
#ifndef _PC_REC_XFER_ALIGN
#define _PC_REC_XFER_ALIGN 17
#endif
#ifndef _PC_ALLOC_SIZE_MIN
#define _PC_ALLOC_SIZE_MIN 18
#endif
#ifndef _PC_SYMLINK_MAX
#define _PC_SYMLINK_MAX 19
#endif
#ifndef _PC_2_SYMLINKS
#define _PC_2_SYMLINKS 20
#endif
#ifndef _SC_ARG_MAX
#define _SC_ARG_MAX 0
#endif
#ifndef _SC_CHILD_MAX
#define _SC_CHILD_MAX 1
#endif
#ifndef _SC_OPEN_MAX
#define _SC_OPEN_MAX 4
#endif
#ifndef _SC_NPROCESSORS_CONF
#define _SC_NPROCESSORS_CONF 83
#endif
#ifndef _SC_NPROCESSORS_ONLN
#define _SC_NPROCESSORS_ONLN 84
#endif
#ifndef _SC_PHYS_PAGES
#define _SC_PHYS_PAGES 85
#endif
#ifndef _SC_AVPHYS_PAGES
#define _SC_AVPHYS_PAGES 86
#endif
#ifndef _SC_NGROUPS_MAX
#define _SC_NGROUPS_MAX 3
#endif
#ifndef _SC_REALTIME_SIGNALS
#define _SC_REALTIME_SIGNALS 9
#endif
#ifndef _SC_PRIORITY_SCHEDULING
#define _SC_PRIORITY_SCHEDULING 10
#endif
#ifndef _SC_TIMERS
#define _SC_TIMERS 11
#endif
#ifndef _SC_ASYNCHRONOUS_IO
#define _SC_ASYNCHRONOUS_IO 12
#endif
#ifndef _SC_PRIORITIZED_IO
#define _SC_PRIORITIZED_IO 13
#endif
#ifndef _SC_SYNCHRONIZED_IO
#define _SC_SYNCHRONIZED_IO 14
#endif
#ifndef _SC_FSYNC
#define _SC_FSYNC 15
#endif
#ifndef _SC_MAPPED_FILES
#define _SC_MAPPED_FILES 16
#endif
#ifndef _SC_MEMLOCK
#define _SC_MEMLOCK 17
#endif
#ifndef _SC_MEMLOCK_RANGE
#define _SC_MEMLOCK_RANGE 18
#endif
#ifndef _SC_MEMORY_PROTECTION
#define _SC_MEMORY_PROTECTION 19
#endif
#ifndef _SC_MESSAGE_PASSING
#define _SC_MESSAGE_PASSING 20
#endif
#ifndef _SC_SEMAPHORES
#define _SC_SEMAPHORES 21
#endif
#ifndef _SC_SHARED_MEMORY_OBJECTS
#define _SC_SHARED_MEMORY_OBJECTS 22
#endif
#ifndef _SC_AIO_LISTIO_MAX
#define _SC_AIO_LISTIO_MAX 23
#endif
#ifndef _SC_AIO_MAX
#define _SC_AIO_MAX 24
#endif
#ifndef _SC_AIO_PRIO_DELTA_MAX
#define _SC_AIO_PRIO_DELTA_MAX 25
#endif
#ifndef _SC_DELAYTIMER_MAX
#define _SC_DELAYTIMER_MAX 26
#endif
#ifndef _SC_MQ_OPEN_MAX
#define _SC_MQ_OPEN_MAX 27
#endif
#ifndef _SC_MQ_PRIO_MAX
#define _SC_MQ_PRIO_MAX 28
#endif
#ifndef _SC_RTSIG_MAX
#define _SC_RTSIG_MAX 31
#endif
#ifndef _SC_SEM_NSEMS_MAX
#define _SC_SEM_NSEMS_MAX 32
#endif
#ifndef _SC_SEM_VALUE_MAX
#define _SC_SEM_VALUE_MAX 33
#endif
#ifndef _SC_SIGQUEUE_MAX
#define _SC_SIGQUEUE_MAX 34
#endif
#ifndef _SC_TIMER_MAX
#define _SC_TIMER_MAX 35
#endif
#ifndef _SC_BC_BASE_MAX
#define _SC_BC_BASE_MAX 36
#endif
#ifndef _SC_BC_DIM_MAX
#define _SC_BC_DIM_MAX 37
#endif
#ifndef _SC_BC_SCALE_MAX
#define _SC_BC_SCALE_MAX 38
#endif
#ifndef _SC_BC_STRING_MAX
#define _SC_BC_STRING_MAX 39
#endif
#ifndef _SC_COLL_WEIGHTS_MAX
#define _SC_COLL_WEIGHTS_MAX 40
#endif
#ifndef _SC_EXPR_NEST_MAX
#define _SC_EXPR_NEST_MAX 42
#endif
#ifndef _SC_LINE_MAX
#define _SC_LINE_MAX 43
#endif
#ifndef _SC_RE_DUP_MAX
#define _SC_RE_DUP_MAX 44
#endif
#ifndef _SC_2_VERSION
#define _SC_2_VERSION 46
#endif
#ifndef _SC_2_C_BIND
#define _SC_2_C_BIND 47
#endif
#ifndef _SC_2_C_DEV
#define _SC_2_C_DEV 48
#endif
#ifndef _SC_2_FORT_DEV
#define _SC_2_FORT_DEV 49
#endif
#ifndef _SC_2_FORT_RUN
#define _SC_2_FORT_RUN 50
#endif
#ifndef _SC_2_SW_DEV
#define _SC_2_SW_DEV 51
#endif
#ifndef _SC_2_LOCALEDEF
#define _SC_2_LOCALEDEF 52
#endif
#ifndef _SC_IOV_MAX
#define _SC_IOV_MAX 60
#endif
#ifndef _SC_THREADS
#define _SC_THREADS 67
#endif
#ifndef _SC_THREAD_SAFE_FUNCTIONS
#define _SC_THREAD_SAFE_FUNCTIONS 68
#endif
#ifndef _SC_GETGR_R_SIZE_MAX
#define _SC_GETGR_R_SIZE_MAX 69
#endif
#ifndef _SC_GETPW_R_SIZE_MAX
#define _SC_GETPW_R_SIZE_MAX 70
#endif
#ifndef _SC_LOGIN_NAME_MAX
#define _SC_LOGIN_NAME_MAX 71
#endif
#ifndef _SC_TTY_NAME_MAX
#define _SC_TTY_NAME_MAX 72
#endif
#ifndef _SC_THREAD_DESTRUCTOR_ITERATIONS
#define _SC_THREAD_DESTRUCTOR_ITERATIONS 73
#endif
#ifndef _SC_THREAD_KEYS_MAX
#define _SC_THREAD_KEYS_MAX 74
#endif
#ifndef _SC_THREAD_STACK_MIN
#define _SC_THREAD_STACK_MIN 75
#endif
#ifndef _SC_THREAD_THREADS_MAX
#define _SC_THREAD_THREADS_MAX 76
#endif
#ifndef _SC_THREAD_ATTR_STACKADDR
#define _SC_THREAD_ATTR_STACKADDR 77
#endif
#ifndef _SC_THREAD_ATTR_STACKSIZE
#define _SC_THREAD_ATTR_STACKSIZE 78
#endif
#ifndef _SC_THREAD_PRIORITY_SCHEDULING
#define _SC_THREAD_PRIORITY_SCHEDULING 79
#endif
#ifndef _SC_THREAD_PRIO_INHERIT
#define _SC_THREAD_PRIO_INHERIT 80
#endif
#ifndef _SC_THREAD_PRIO_PROTECT
#define _SC_THREAD_PRIO_PROTECT 81
#endif
#ifndef _SC_THREAD_PROCESS_SHARED
#define _SC_THREAD_PROCESS_SHARED 82
#endif
#ifndef _SC_XOPEN_VERSION
#define _SC_XOPEN_VERSION 89
#endif
#ifndef _SC_XOPEN_XCU_VERSION
#define _SC_XOPEN_XCU_VERSION 90
#endif
#ifndef _SC_XOPEN_UNIX
#define _SC_XOPEN_UNIX 91
#endif
#ifndef _SC_XOPEN_CRYPT
#define _SC_XOPEN_CRYPT 92
#endif
#ifndef _SC_XOPEN_ENH_I18N
#define _SC_XOPEN_ENH_I18N 93
#endif
#ifndef _SC_XOPEN_SHM
#define _SC_XOPEN_SHM 94
#endif
#ifndef _SC_2_CHAR_TERM
#define _SC_2_CHAR_TERM 95
#endif
#ifndef _SC_2_UPE
#define _SC_2_UPE 97
#endif
#ifndef _SC_XOPEN_XPG2
#define _SC_XOPEN_XPG2 98
#endif
#ifndef _SC_XOPEN_XPG3
#define _SC_XOPEN_XPG3 99
#endif
#ifndef _SC_XOPEN_XPG4
#define _SC_XOPEN_XPG4 100
#endif
#ifndef _SC_NZERO
#define _SC_NZERO 109
#endif
#ifndef _SC_XBS5_ILP32_OFF32
#define _SC_XBS5_ILP32_OFF32 125
#endif
#ifndef _SC_XBS5_ILP32_OFFBIG
#define _SC_XBS5_ILP32_OFFBIG 126
#endif
#ifndef _SC_XBS5_LP64_OFF64
#define _SC_XBS5_LP64_OFF64 127
#endif
#ifndef _SC_XBS5_LPBIG_OFFBIG
#define _SC_XBS5_LPBIG_OFFBIG 128
#endif
#ifndef _SC_XOPEN_LEGACY
#define _SC_XOPEN_LEGACY 129
#endif
#ifndef _SC_XOPEN_REALTIME
#define _SC_XOPEN_REALTIME 130
#endif
#ifndef _SC_XOPEN_REALTIME_THREADS
#define _SC_XOPEN_REALTIME_THREADS 131
#endif
#ifndef _SC_ADVISORY_INFO
#define _SC_ADVISORY_INFO 132
#endif
#ifndef _SC_BARRIERS
#define _SC_BARRIERS 133
#endif
#ifndef _SC_CLOCK_SELECTION
#define _SC_CLOCK_SELECTION 137
#endif
#ifndef _SC_CPUTIME
#define _SC_CPUTIME 138
#endif
#ifndef _SC_THREAD_CPUTIME
#define _SC_THREAD_CPUTIME 139
#endif
#ifndef _SC_MONOTONIC_CLOCK
#define _SC_MONOTONIC_CLOCK 149
#endif
#ifndef _SC_READER_WRITER_LOCKS
#define _SC_READER_WRITER_LOCKS 153
#endif
#ifndef _SC_SPIN_LOCKS
#define _SC_SPIN_LOCKS 154
#endif
#ifndef _SC_REGEXP
#define _SC_REGEXP 155
#endif
#ifndef _SC_SHELL
#define _SC_SHELL 157
#endif
#ifndef _SC_SPAWN
#define _SC_SPAWN 159
#endif
#ifndef _SC_SPORADIC_SERVER
#define _SC_SPORADIC_SERVER 160
#endif
#ifndef _SC_THREAD_SPORADIC_SERVER
#define _SC_THREAD_SPORADIC_SERVER 161
#endif
#ifndef _SC_TIMEOUTS
#define _SC_TIMEOUTS 164
#endif
#ifndef _SC_TYPED_MEMORY_OBJECTS
#define _SC_TYPED_MEMORY_OBJECTS 165
#endif
#ifndef _SC_SYMLOOP_MAX
#define _SC_SYMLOOP_MAX 173
#endif
#ifndef _SC_STREAMS
#define _SC_STREAMS 174
#endif
#ifndef _SC_HOST_NAME_MAX
#define _SC_HOST_NAME_MAX 180
#endif
#ifndef _SC_TRACE
#define _SC_TRACE 181
#endif
#ifndef _SC_TRACE_EVENT_FILTER
#define _SC_TRACE_EVENT_FILTER 182
#endif
#ifndef _SC_TRACE_INHERIT
#define _SC_TRACE_INHERIT 183
#endif
#ifndef _SC_TRACE_LOG
#define _SC_TRACE_LOG 184
#endif
#ifndef _SC_IPV6
#define _SC_IPV6 235
#endif
#ifndef _SC_RAW_SOCKETS
#define _SC_RAW_SOCKETS 236
#endif
#ifndef _SC_V7_ILP32_OFF32
#define _SC_V7_ILP32_OFF32 237
#endif
#ifndef _SC_V7_ILP32_OFFBIG
#define _SC_V7_ILP32_OFFBIG 238
#endif
#ifndef _SC_V7_LP64_OFF64
#define _SC_V7_LP64_OFF64 239
#endif
#ifndef _SC_V7_LPBIG_OFFBIG
#define _SC_V7_LPBIG_OFFBIG 240
#endif
#ifndef _SC_SS_REPL_MAX
#define _SC_SS_REPL_MAX 241
#endif
#ifndef _SC_TRACE_EVENT_NAME_MAX
#define _SC_TRACE_EVENT_NAME_MAX 242
#endif
#ifndef _SC_TRACE_NAME_MAX
#define _SC_TRACE_NAME_MAX 243
#endif
#ifndef _SC_TRACE_SYS_MAX
#define _SC_TRACE_SYS_MAX 244
#endif
#ifndef _SC_TRACE_USER_EVENT_MAX
#define _SC_TRACE_USER_EVENT_MAX 245
#endif
#ifndef _SC_XOPEN_STREAMS
#define _SC_XOPEN_STREAMS 246
#endif
#ifndef _SC_THREAD_ROBUST_PRIO_INHERIT
#define _SC_THREAD_ROBUST_PRIO_INHERIT 247
#endif
#ifndef _SC_THREAD_ROBUST_PRIO_PROTECT
#define _SC_THREAD_ROBUST_PRIO_PROTECT 248
#endif
#ifndef _SC_STREAM_MAX
#define _SC_STREAM_MAX 5
#endif
#ifndef _SC_TZNAME_MAX
#define _SC_TZNAME_MAX 6
#endif
#ifndef _SC_JOB_CONTROL
#define _SC_JOB_CONTROL 7
#endif
#ifndef _SC_SAVED_IDS
#define _SC_SAVED_IDS 8
#endif
#ifndef _SC_VERSION
#define _SC_VERSION 29
#endif
#ifndef _SC_PAGESIZE
#define _SC_PAGESIZE 30
#endif
#ifndef _SC_PAGE_SIZE
#define _SC_PAGE_SIZE _SC_PAGESIZE
#endif
extern char *optarg;
extern int optind;
extern int opterr;
extern int optopt;
#else
#include_next <unistd.h>
#endif

int getopt(int argc, char *const argv[], const char *option_string);
int getpagesize(void);
long sysconf(int name);
size_t confstr(int name, char *buffer, size_t length);
long pathconf(const char *path, int name);
long fpathconf(int fd, int name);

/* Nelibc's Linux syscall implementations and extensions, in both modes. */
#include "nelibc_syscall.h"

#endif
