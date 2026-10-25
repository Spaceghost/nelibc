#ifndef NELIBC_SYS_SIGNALFD_WRAPPER_H
#define NELIBC_SYS_SIGNALFD_WRAPPER_H

#ifdef NELIBC_STANDALONE
#include <stdint.h>
#include <signal.h>
#include <fcntl.h>

#ifndef SFD_CLOEXEC
#define SFD_CLOEXEC O_CLOEXEC
#endif
#ifndef SFD_NONBLOCK
#define SFD_NONBLOCK O_NONBLOCK
#endif

struct signalfd_siginfo {
  uint32_t ssi_signo;
  int32_t ssi_errno;
  int32_t ssi_code;
  uint32_t ssi_pid;
  uint32_t ssi_uid;
  int32_t ssi_fd;
  uint32_t ssi_tid;
  uint32_t ssi_band;
  uint32_t ssi_overrun;
  uint32_t ssi_trapno;
  int32_t ssi_status;
  int32_t ssi_int;
  uint64_t ssi_ptr;
  uint64_t ssi_utime;
  uint64_t ssi_stime;
  uint64_t ssi_addr;
  uint16_t ssi_addr_lsb;
  uint16_t __pad2;
  int32_t ssi_syscall;
  uint64_t ssi_call_addr;
  uint32_t ssi_arch;
  uint8_t __pad[28];
};
#else
#include_next <sys/signalfd.h>
#endif

#ifdef NELIBC_STANDALONE
int signalfd(int fd, const sigset_t *signal_mask, int flags);
#endif

#endif
