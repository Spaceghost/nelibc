#ifndef NELIBC_SIGNAL_WRAPPER_H
#define NELIBC_SIGNAL_WRAPPER_H

#ifdef NELIBC_STANDALONE

#include <sys/types.h>

typedef int sig_atomic_t;
typedef void (*sighandler_t)(int);
typedef struct { unsigned long __bits[16]; } sigset_t;
#ifndef __siginfo_t_defined
#define __siginfo_t_defined 1
typedef struct siginfo {
  int si_signo;
  int si_errno;
  int si_code;
  int __pad0;
  union {
    int _pad[28];
    struct {
      pid_t si_pid;
      uid_t si_uid;
      int si_status;
      long si_utime;
      long si_stime;
    } _sigchld;
  } _sifields;
} siginfo_t;
#define si_pid _sifields._sigchld.si_pid
#define si_uid _sifields._sigchld.si_uid
#define si_status _sifields._sigchld.si_status
#define si_utime _sifields._sigchld.si_utime
#define si_stime _sifields._sigchld.si_stime
#endif
union sigval {
  int sival_int;
  void *sival_ptr;
};
struct sigevent {
  union sigval sigev_value;
  int sigev_signo;
  int sigev_notify;
  union {
    int _pad[12];
    int _tid;
    struct {
      void (*_function)(union sigval);
      void *_attribute;
    } _thread;
  } _sigev_un;
};
#define sigev_notify_function _sigev_un._thread._function
#define sigev_notify_attributes _sigev_un._thread._attribute
#define sigev_notify_thread_id _sigev_un._tid

struct sigaction {
  sighandler_t sa_handler;
  sigset_t sa_mask;
  int sa_flags;
  void (*sa_restorer)(void);
};

#define SIG_DFL ((sighandler_t)0)
#define SIG_IGN ((sighandler_t)1)
#define SIG_ERR ((sighandler_t)-1)

#define SIGHUP 1
#define SIGINT 2
#define SIGQUIT 3
#define SIGILL 4
#define SIGTRAP 5
#define SIGABRT 6
#define SIGBUS 7
#define SIGFPE 8
#define SIGKILL 9
#define SIGUSR1 10
#define SIGSEGV 11
#define SIGUSR2 12
#define SIGPIPE 13
#define SIGALRM 14
#define SIGTERM 15
#define SIGSTKFLT 16
#define SIGCHLD 17
#define SIGCONT 18
#define SIGSTOP 19
#define SIGTSTP 20
#define SIGTTIN 21
#define SIGTTOU 22
#define SIGURG 23
#define SIGXCPU 24
#define SIGXFSZ 25
#define SIGVTALRM 26
#define SIGPROF 27
#define SIGWINCH 28
#define SIGIO 29
#define SIGPOLL SIGIO
#define SIGPWR 30
#define SIGSYS 31
#define SIGIOT SIGABRT
#define SIGCLD SIGCHLD
#define NSIG 65
#define SIGRTMIN 34
#define SIGRTMAX 64

#define SIG_BLOCK 0
#define SIG_UNBLOCK 1
#define SIG_SETMASK 2
#define CLD_EXITED 1
#define CLD_KILLED 2
#define CLD_DUMPED 3
#define CLD_TRAPPED 4
#define CLD_STOPPED 5
#define CLD_CONTINUED 6
#define SIGEV_SIGNAL 0
#define SIGEV_NONE 1
#define SIGEV_THREAD 2
#define SIGEV_THREAD_ID 4

#define SA_NOCLDSTOP 0x00000001
#define SA_NOCLDWAIT 0x00000002
#define SA_SIGINFO 0x00000004
#define SA_RESTORER 0x04000000
#define SA_ONSTACK 0x08000000
#define SA_RESTART 0x10000000
#define SA_NODEFER 0x40000000
#define SA_RESETHAND 0x80000000U

int kill(pid_t process, int signal);
int raise(int signal);
int sigprocmask(int how, const sigset_t *set, sigset_t *old_set);
int pthread_sigmask(int how, const sigset_t *set, sigset_t *old_set);
int sigpending(sigset_t *set);
int sigismember(const sigset_t *set, int signal);
int sigemptyset(sigset_t *set);
int sigaddset(sigset_t *set, int signal);
int sigdelset(sigset_t *set, int signal);
int sigfillset(sigset_t *set);
int sigaction(int signal, const struct sigaction *action,
              struct sigaction *old_action);

#else

#include_next <signal.h>
#include "nelibc_syscall.h"

#endif

#endif
