#!/usr/bin/env python3
"""The Nelibc platform-operation vocabulary.

The Nelua modules and C adapters ask the platform for operations by
`NELIBC_SYS_*` number (`nelibc_sys3`/`nelibc_sys6`, include/nelibc_sys.h).
That vocabulary is Nelibc's own: a backend maps it to what its platform
has. src/sys/linux_x86_64_ops.c maps it to Linux x86-64 syscall numbers
through the table this script generates; the host profile
(src/host/syscall_host.c) answers the operations from a callback table and
never sees a Linux number.

This script is the single source of the vocabulary. It writes:
  include/nelibc_sys.h          the enum and the entry points
  src/sys/linux_x86_64_ops.h    the Linux x86-64 number per operation
  src/*.nelua                   the `SYS_*` constant block each module uses
                                (between the two marker comments)
Run it after editing OPERATIONS; `--check` fails if any output is stale.
"""
from __future__ import annotations
import argparse
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]

# (name, Linux x86-64 syscall number). The order fixes the NELIBC_SYS_*
# values; append new operations at the end to keep generated C stable.
OPERATIONS = [
    ("read", 0), ("write", 1), ("open", 2), ("close", 3), ("stat", 4),
    ("fstat", 5), ("lstat", 6), ("poll", 7), ("lseek", 8), ("mmap", 9),
    ("mprotect", 10), ("munmap", 11), ("brk", 12), ("rt_sigaction", 13),
    ("rt_sigprocmask", 14), ("ioctl", 16), ("pread64", 17), ("pwrite64", 18),
    ("readv", 19), ("writev", 20), ("access", 21), ("pipe", 22),
    ("select", 23), ("sched_yield", 24), ("mremap", 25), ("msync", 26),
    ("madvise", 28), ("dup", 32), ("dup2", 33), ("pause", 34),
    ("nanosleep", 35), ("getitimer", 36), ("alarm", 37), ("setitimer", 38),
    ("getpid", 39), ("sendfile", 40), ("socket", 41), ("connect", 42),
    ("accept", 43), ("sendto", 44), ("recvfrom", 45), ("sendmsg", 46),
    ("recvmsg", 47), ("shutdown", 48), ("bind", 49), ("listen", 50),
    ("getsockname", 51), ("getpeername", 52), ("socketpair", 53),
    ("setsockopt", 54), ("getsockopt", 55), ("clone", 56), ("fork", 57),
    ("vfork", 58), ("execve", 59), ("exit", 60), ("wait4", 61), ("kill", 62),
    ("uname", 63), ("fcntl", 72), ("flock", 73), ("fsync", 74),
    ("fdatasync", 75), ("truncate", 76), ("ftruncate", 77), ("getcwd", 79),
    ("chdir", 80), ("fchdir", 81), ("readlink", 89), ("chmod", 90),
    ("fchmod", 91), ("fchown", 93), ("umask", 95), ("gettimeofday", 96),
    ("getrlimit", 97), ("getrusage", 98), ("sysinfo", 99), ("times", 100),
    ("getuid", 102), ("getgid", 104), ("setuid", 105), ("setgid", 106),
    ("geteuid", 107), ("getegid", 108), ("setpgid", 109), ("getppid", 110),
    ("getpgrp", 111), ("setsid", 112), ("setreuid", 113), ("setregid", 114),
    ("getgroups", 115), ("setgroups", 116), ("setresuid", 117),
    ("getresuid", 118), ("setresgid", 119), ("getresgid", 120),
    ("getpgid", 121), ("getsid", 124), ("rt_sigpending", 127),
    ("rt_sigtimedwait", 128), ("rt_sigsuspend", 130), ("sigaltstack", 131),
    ("statfs", 137), ("fstatfs", 138), ("getpriority", 140),
    ("setpriority", 141), ("sched_setparam", 142), ("sched_getparam", 143),
    ("sched_setscheduler", 144), ("sched_getscheduler", 145),
    ("sched_get_priority_max", 146), ("sched_get_priority_min", 147),
    ("sched_rr_get_interval", 148), ("pivot_root", 155), ("prctl", 157),
    ("setrlimit", 160), ("sync", 162), ("mount", 165), ("umount2", 166),
    ("sethostname", 170), ("setdomainname", 171), ("gettid", 186),
    ("readahead", 187), ("setxattr", 188), ("lsetxattr", 189),
    ("fsetxattr", 190), ("getxattr", 191), ("lgetxattr", 192),
    ("fgetxattr", 193), ("listxattr", 194), ("llistxattr", 195),
    ("flistxattr", 196), ("removexattr", 197), ("lremovexattr", 198),
    ("fremovexattr", 199), ("tkill", 200), ("time", 201), ("futex", 202),
    ("sched_setaffinity", 203), ("sched_getaffinity", 204),
    ("getdents64", 217), ("set_tid_address", 218), ("fadvise64", 221),
    ("timer_create", 222), ("timer_settime", 223), ("timer_gettime", 224),
    ("timer_getoverrun", 225), ("timer_delete", 226), ("clock_settime", 227),
    ("clock_gettime", 228), ("clock_getres", 229), ("clock_nanosleep", 230),
    ("exit_group", 231), ("epoll_wait", 232), ("epoll_ctl", 233),
    ("tgkill", 234), ("mq_open", 240), ("mq_unlink", 241),
    ("mq_timedsend", 242), ("mq_timedreceive", 243), ("mq_notify", 244),
    ("mq_getsetattr", 245), ("waitid", 247), ("openat", 257),
    ("mkdirat", 258), ("mknodat", 259), ("fchownat", 260),
    ("newfstatat", 262), ("unlinkat", 263), ("renameat", 264),
    ("linkat", 265), ("symlinkat", 266), ("readlinkat", 267),
    ("fchmodat", 268), ("faccessat", 269), ("pselect6", 270), ("ppoll", 271),
    ("unshare", 272), ("set_robust_list", 273), ("get_robust_list", 274),
    ("splice", 275), ("tee", 276), ("vmsplice", 278), ("utimensat", 280),
    ("epoll_pwait", 281), ("timerfd_create", 283), ("fallocate", 285),
    ("timerfd_settime", 286), ("timerfd_gettime", 287), ("accept4", 288),
    ("signalfd4", 289), ("eventfd2", 290), ("epoll_create1", 291),
    ("dup3", 292), ("pipe2", 293), ("preadv", 295), ("pwritev", 296),
    ("prlimit64", 302), ("syncfs", 306), ("setns", 308), ("getrandom", 318),
    ("memfd_create", 319), ("execveat", 322), ("copy_file_range", 326),
    ("preadv2", 327), ("pwritev2", 328), ("statx", 332),
    ("pidfd_send_signal", 424), ("open_tree", 428), ("move_mount", 429),
    ("fsopen", 430), ("fsconfig", 431), ("fsmount", 432), ("fspick", 433),
    ("pidfd_open", 434), ("close_range", 436), ("faccessat2", 439),
    ("epoll_pwait2", 441), ("mount_setattr", 442), ("fchmodat2", 452),
    ("statmount", 457), ("listmount", 458), ("setxattrat", 463),
    ("getxattrat", 464), ("listxattrat", 465), ("removexattrat", 466),
    ("open_tree_attr", 467), ("file_getattr", 468), ("file_setattr", 469),
    ("shmget", 29), ("shmat", 30), ("shmctl", 31), ("semget", 64),
    ("semop", 65), ("semctl", 66), ("shmdt", 67), ("msgget", 68),
    ("msgsnd", 69), ("msgrcv", 70), ("msgctl", 71), ("semtimedop", 220),
    ("renameat2", 316),
]

NELUA_MODULES = [
    "src/unistd.nelua",
    "src/thread.nelua",
    "src/spawn_standalone.nelua",
    "src/alloc.nelua",
    "src/sys/linux_x86_64.nelua",
]

BLOCK_BEGIN = "-- nelibc_sys operations this module uses; tools/sys_ops.py keeps the block current. (begin)"
BLOCK_END = "-- (end nelibc_sys operations)"
GENERATED = "Generated by tools/sys_ops.py; edit OPERATIONS there, not this file."


def header() -> str:
    lines = [
        "/* Nelibc platform operations: the vocabulary the libc modules use to ask",
        " * the platform for work. The numbers are Nelibc's own, not any kernel's.",
        " * A backend answers them: src/sys/linux_x86_64_ops.c maps each to the",
        " * Linux x86-64 syscall it names; src/host/syscall_host.c answers them",
        " * from a nelibc_host callback table. The operation names say which",
        " * Linux syscall's semantics and argument shapes the modules expect;",
        " * a backend for another kernel translates where its kernel differs.",
        " *",
        f" * {GENERATED}",
        " */",
        "#ifndef NELIBC_SYS_H",
        "#define NELIBC_SYS_H",
        "",
        "#ifdef __cplusplus",
        'extern "C" {',
        "#endif",
        "",
        "enum nelibc_sys_op {",
    ]
    for index, (name, _) in enumerate(OPERATIONS):
        lines.append(f"  NELIBC_SYS_{name.upper()} = {index},")
    lines += [
        f"  NELIBC_SYS_COUNT = {len(OPERATIONS)}",
        "};",
        "",
        "/* Perform an operation. Results follow the Linux convention the modules",
        " * were written against: a value, or a negative errno. An operation the",
        " * backend does not provide returns -ENOSYS (-38). */",
        "long nelibc_sys3(unsigned long op, unsigned long a1, unsigned long a2,",
        "                 unsigned long a3);",
        "long nelibc_sys6(unsigned long op, unsigned long a1, unsigned long a2,",
        "                 unsigned long a3, unsigned long a4, unsigned long a5,",
        "                 unsigned long a6);",
        "",
        "/* Linux x86-64 backend only: the syscall number behind an operation, or",
        " * ~0UL for an operation outside the table. */",
        "unsigned long nelibc_sys_linux_x86_64_number(unsigned long op);",
        "",
        "#ifdef __cplusplus",
        "}",
        "#endif",
        "",
        "#endif",
        "",
    ]
    return "\n".join(lines)


def linux_table() -> str:
    lines = [
        "/* Linux x86-64 syscall number per Nelibc operation, indexed by",
        " * NELIBC_SYS_*.",
        f" * {GENERATED}",
        " */",
        "static const unsigned short nelibc_sys_linux_x86_64_numbers[NELIBC_SYS_COUNT] = {",
    ]
    for name, number in OPERATIONS:
        lines.append(f"  {number}, /* {name} */")
    lines += ["};", ""]
    return "\n".join(lines)


def module_block(text: str) -> str:
    known = {name.upper(): index for index, (name, _) in enumerate(OPERATIONS)}
    used = sorted({m for m in re.findall(r"\bSYS_([A-Z0-9_]+)\b", text) if m in known},
                  key=lambda n: known[n])
    lines = [BLOCK_BEGIN]
    for name in used:
        lines.append(f"local SYS_{name}: culong <comptime> = {known[name]}")
    lines.append(BLOCK_END)
    return "\n".join(lines)


def with_block(text: str) -> str:
    begin = text.find(BLOCK_BEGIN)
    end = text.find(BLOCK_END)
    if begin < 0 or end < 0:
        raise SystemExit("module has no nelibc_sys block markers")
    end += len(BLOCK_END)
    outside = text[:begin] + text[end:]
    return text[:begin] + module_block(outside) + text[end:]


def outputs() -> dict[Path, str]:
    result = {
        ROOT / "include/nelibc_sys.h": header(),
        ROOT / "src/sys/linux_x86_64_ops.h": linux_table(),
    }
    for module in NELUA_MODULES:
        path = ROOT / module
        result[path] = with_block(path.read_text())
    return result


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--check", action="store_true", help="fail if any output is stale")
    args = parser.parse_args()
    stale = []
    for path, text in outputs().items():
        current = path.read_text() if path.exists() else None
        if current == text:
            continue
        if args.check:
            stale.append(path)
        else:
            path.write_text(text)
            print(f"wrote {path.relative_to(ROOT)}")
    if stale:
        for path in stale:
            print(f"stale: {path.relative_to(ROOT)}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
