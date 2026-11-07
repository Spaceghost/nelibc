#ifndef NELIBC_SYS_MSG_WRAPPER_H
#define NELIBC_SYS_MSG_WRAPPER_H

/* Hybrid builds use the host's <sys/msg.h>. The standalone profile: src/ipc_adapter.c, the kernel's x86-64 struct msqid64_ds. */
#ifdef NELIBC_STANDALONE

#include <sys/ipc.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned long msgqnum_t;
typedef unsigned long msglen_t;

struct msqid_ds {
  struct ipc_perm msg_perm;
  time_t msg_stime;
  time_t msg_rtime;
  time_t msg_ctime;
  unsigned long msg_cbytes;
  msgqnum_t msg_qnum;
  msglen_t msg_qbytes;
  pid_t msg_lspid;
  pid_t msg_lrpid;
  unsigned long __unused[2];
};

#define MSG_NOERROR 010000
#define MSG_EXCEPT  020000
#define MSG_COPY    040000
#define MSG_STAT     11
#define MSG_INFO     12
#define MSG_STAT_ANY 13

struct msginfo {
  int msgpool, msgmap, msgmax, msgmnb, msgmni, msgssz, msgtql;
  unsigned short msgseg;
};

int msgctl(int, int, struct msqid_ds *);
int msgget(key_t, int);
ssize_t msgrcv(int, void *, size_t, long, int);
int msgsnd(int, const void *, size_t, int);

#ifdef __cplusplus
}
#endif

#else

#include_next <sys/msg.h>

#endif

#endif
