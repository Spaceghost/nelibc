#ifndef NELIBC_SYS_EPOLL_WRAPPER_H
#define NELIBC_SYS_EPOLL_WRAPPER_H

#ifdef NELIBC_STANDALONE
#include <stdint.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>

typedef union epoll_data {
  void *ptr;
  int fd;
  uint32_t u32;
  uint64_t u64;
} epoll_data_t;

#if defined(__TINYC__)
/* TinyCC accepts #pragma pack but silently ignores GCC's packed attribute. */
#pragma pack(push, 1)
#define NELIBC_EPOLL_PACKED
#else
#define NELIBC_EPOLL_PACKED __attribute__((__packed__))
#endif
struct epoll_event {
  uint32_t events;
  epoll_data_t data;
} NELIBC_EPOLL_PACKED;
#if defined(__TINYC__)
#pragma pack(pop)
#endif

#ifndef EPOLLIN
#define EPOLLIN 0x001
#endif
#ifndef EPOLLOUT
#define EPOLLOUT 0x004
#endif
#ifndef EPOLLPRI
#define EPOLLPRI 0x002
#endif
#ifndef EPOLLERR
#define EPOLLERR 0x008
#endif
#ifndef EPOLLHUP
#define EPOLLHUP 0x010
#endif
#ifndef EPOLLRDHUP
#define EPOLLRDHUP 0x2000
#endif
#ifndef EPOLLRDNORM
#define EPOLLRDNORM 0x040
#define EPOLLRDBAND 0x080
#define EPOLLWRNORM 0x100
#define EPOLLWRBAND 0x200
#define EPOLLMSG 0x400
#define EPOLLWAKEUP (1U << 29)
#define EPOLLONESHOT (1U << 30)
#define EPOLLET (1U << 31)
#endif
#ifndef EPOLL_CLOEXEC
#define EPOLL_CLOEXEC O_CLOEXEC
#endif
#ifndef EPOLL_CTL_ADD
#define EPOLL_CTL_ADD 1
#define EPOLL_CTL_DEL 2
#define EPOLL_CTL_MOD 3
#endif
#else
#if defined(__TINYC__)
/* glibc marks epoll_event packed; TinyCC ignores that attribute. */
#pragma pack(push, 1)
#endif
#include_next <sys/epoll.h>
#if defined(__TINYC__)
#pragma pack(pop)
#endif
#include <signal.h>
#include <time.h>
#endif

int epoll_create1(int flags);
int epoll_ctl(int epfd, int operation, int fd, struct epoll_event *event);
int epoll_wait(int epfd, struct epoll_event *events, int maxevents,
               int timeout);
int epoll_pwait(int epfd, struct epoll_event *events, int maxevents,
                int timeout, const sigset_t *signal_mask);
int epoll_pwait2(int epfd, struct epoll_event *events, int maxevents,
                 const struct timespec *timeout,
                 const sigset_t *signal_mask);

#endif
