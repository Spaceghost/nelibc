#ifndef NELIBC_SYS_IPC_WRAPPER_H
#define NELIBC_SYS_IPC_WRAPPER_H

/* Hybrid builds use the host's <sys/ipc.h>. The standalone profile provides System V IPC in src/ipc_adapter.c (after musl 1.2.5). */
#ifdef NELIBC_STANDALONE

#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

#define IPC_CREAT  01000
#define IPC_EXCL   02000
#define IPC_NOWAIT 04000

#define IPC_RMID 0
#define IPC_SET  1
#define IPC_STAT 2
#define IPC_INFO 3

#define IPC_PRIVATE ((key_t)0)

/* The kernel's struct ipc64_perm on x86-64. */
struct ipc_perm {
  key_t __ipc_perm_key;
  uid_t uid;
  gid_t gid;
  uid_t cuid;
  gid_t cgid;
  mode_t mode;
  int __ipc_perm_seq;
  long __pad1;
  long __pad2;
};
#define __key __ipc_perm_key
#define __seq __ipc_perm_seq

key_t ftok(const char *, int);

#ifdef __cplusplus
}
#endif

#else

#include_next <sys/ipc.h>

#endif

#endif
