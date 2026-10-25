#ifndef NELIBC_SYS_SOCKET_WRAPPER_H
#define NELIBC_SYS_SOCKET_WRAPPER_H

#ifdef NELIBC_STANDALONE

#include <stddef.h>

#ifndef __socklen_t_defined
typedef unsigned int socklen_t;
#define __socklen_t_defined 1
#endif
typedef unsigned short sa_family_t;

struct sockaddr {
  sa_family_t sa_family;
  char sa_data[14];
};

struct sockaddr_storage {
  sa_family_t ss_family;
  char __data[118];
  unsigned long __align;
};

struct iovec;
struct msghdr {
  void *msg_name;
  socklen_t msg_namelen;
  struct iovec *msg_iov;
  size_t msg_iovlen;
  void *msg_control;
  size_t msg_controllen;
  int msg_flags;
};

struct cmsghdr {
  size_t cmsg_len;
  int cmsg_level;
  int cmsg_type;
};

struct linger {
  int l_onoff;
  int l_linger;
};

#define AF_UNSPEC 0
#define AF_UNIX 1
#define AF_LOCAL AF_UNIX
#define AF_INET 2
#define AF_NETLINK 16
#define AF_INET6 10
#define PF_UNSPEC AF_UNSPEC
#define PF_UNIX AF_UNIX
#define PF_LOCAL AF_LOCAL
#define PF_INET AF_INET
#define PF_NETLINK AF_NETLINK
#define PF_INET6 AF_INET6

#define SOCK_STREAM 1
#define SOCK_DGRAM 2
#define SOCK_RAW 3
#define SOCK_RDM 4
#define SOCK_SEQPACKET 5
#define SOCK_DCCP 6
#define SOCK_PACKET 10
#define SOCK_CLOEXEC 02000000
#define SOCK_NONBLOCK 00004000

#define SHUT_RD 0
#define SHUT_WR 1
#define SHUT_RDWR 2

#define MSG_OOB 0x01
#define MSG_PEEK 0x02
#define MSG_DONTROUTE 0x04
#define MSG_CTRUNC 0x08
#define MSG_TRUNC 0x20
#define MSG_DONTWAIT 0x40
#define MSG_EOR 0x80
#define MSG_WAITALL 0x100
#define MSG_NOSIGNAL 0x4000

#define SOMAXCONN 4096

#define CMSG_ALIGN(length) (((length) + sizeof(size_t) - 1) & ~(sizeof(size_t) - 1))
#define CMSG_DATA(header) ((unsigned char *)(header) + CMSG_ALIGN(sizeof(struct cmsghdr)))
#define CMSG_LEN(length) (CMSG_ALIGN(sizeof(struct cmsghdr)) + (length))
#define CMSG_SPACE(length) (CMSG_ALIGN(sizeof(struct cmsghdr)) + CMSG_ALIGN(length))

#include <asm/socket.h>
#include <sys/uio.h>

#else

#include_next <sys/socket.h>

#endif

/* Add Nelibc's Linux syscall declarations to the standard socket API header. */
#include "nelibc_syscall.h"

#endif
