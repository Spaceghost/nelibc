#ifndef NELIBC_SYS_WAIT_WRAPPER_H
#define NELIBC_SYS_WAIT_WRAPPER_H

#include <sys/types.h>
#include <sys/resource.h>

#ifdef NELIBC_STANDALONE
#ifndef WNOHANG
#define WNOHANG 0x00000001
#endif
#ifndef WUNTRACED
#define WUNTRACED 0x00000002
#endif
#ifndef WSTOPPED
#define WSTOPPED WUNTRACED
#endif
#ifndef WEXITED
#define WEXITED 0x00000004
#endif
#ifndef WCONTINUED
#define WCONTINUED 0x00000008
#endif
#ifndef WNOWAIT
#define WNOWAIT 0x01000000
#endif
#ifndef WCOREDUMP
#define WCOREDUMP(status) ((status) & 0x80)
#endif
#ifndef WIFEXITED
#define WIFEXITED(status) (((status) & 0x7f) == 0)
#endif
#ifndef WEXITSTATUS
#define WEXITSTATUS(status) (((status) & 0xff00) >> 8)
#endif
#ifndef WIFSIGNALED
#define WIFSIGNALED(status) \
  ((((signed char)(((status) & 0x7f) + 1)) >> 1) > 0)
#endif
#ifndef WTERMSIG
#define WTERMSIG(status) ((status) & 0x7f)
#endif
#ifndef WIFSTOPPED
#define WIFSTOPPED(status) (((status) & 0xff) == 0x7f)
#endif
#ifndef WSTOPSIG
#define WSTOPSIG(status) WEXITSTATUS(status)
#endif
#ifndef WIFCONTINUED
#define WIFCONTINUED(status) ((status) == 0xffff)
#endif
#else
#include_next <sys/wait.h>
#endif

/* Linux wait4 backs all three exported Nelibc wait entry points. */
pid_t wait(int *status);
pid_t waitpid(pid_t pid, int *status, int options);
pid_t wait4(pid_t pid, int *status, int options, struct rusage *usage);

#endif
