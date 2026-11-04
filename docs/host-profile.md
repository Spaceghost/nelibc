# Host profile

The host profile is Nelibc for an environment that is not Linux user space:
a kernel, a unikernel, or a test harness that wants to be the operating system.
It is the standalone x86-64 archive with two things removed, the Linux entry
point and the `syscall` instruction, and one thing added, a dispatcher that
answers Linux x86-64 syscall numbers from a table of host callbacks. The Nelua
modules are the standalone objects, unchanged.

The first consumer is [Writ](https://github.com/Spaceghost/writ): its
freestanding Bend runtime design names Nelibc as the candidate libc beneath
the unmodified Bend-generated evaluator on a Drift-style kernel. The second is
[Drift](https://github.com/Spaceghost/drift): the callback table follows the
shape of Drift's `drift_hw`, so one platform layer can supply both.

## Contract

`include/nelibc_host.h` declares `nelibc_host`:

| Callback | Required | Backs |
| --- | --- | --- |
| `console_write(fd, buffer, length)` | yes | `write`/`writev` on descriptors 1 and 2, all stdio output |
| `memory_reserve(length, alignment)` | yes | anonymous `mmap`, thread stacks, the allocator, `mremap` growth; must return zero-filled memory |
| `memory_protect`, `memory_release` | no | `mprotect`, `munmap`; absent means success without effect |
| `clock_monotonic(&ns)` | no | `clock_gettime` for monotonic, boot, and CPU clocks; `nanosleep` and futex timeouts; absent means `EINVAL` |
| `clock_realtime(&ns)` | no | `CLOCK_REALTIME`, `gettimeofday`, `time`; falls back to the monotonic clock |
| `sleep(ns)` | no | sleeps and timed waits; absent means a spin on the monotonic clock |
| `thread_pointer_set(tp)` | yes | the x86-64 FS base for `errno` and the rest of static TLS (`arch_prctl` in Linux terms, MSR `0xC0000100` in a kernel) |
| `exit(status)` | yes | `_exit`, `exit_group`, fatal signals, and the profile's own stops |
| `unsupported(number)` | no | called once per syscall number that returns `ENOSYS` |
| `environment` | no | `getenv`; a `NAME=value` vector or NULL |
| `cpu_count` | no | `sched_getaffinity` and `sysconf(_SC_NPROCESSORS_ONLN)`; 0 means 1 |
| `tls` | yes if the program has TLS | the PT_TLS image (start, filesz, memsz, align) |

`nelibc_host_boot(host)` binds the table, builds the initial thread's TLS block
from the template, installs the thread pointer, and initializes stdio.
`nelibc_host_run(host, main, argc, argv)` does that and then runs `main`
through the libc `exit`, so `atexit` handlers run and stdio flushes before
`host->exit`.

### What the dispatcher answers

`src/host/syscall_host.c` is the only place a host sees Linux numbers. It
implements, from the table above:

- Console: `write`, `writev` on 1 and 2; `read` on 0 returns end of file.
- Memory: anonymous `mmap` (hints ignored, `MAP_FIXED` and file mappings
  refused), `mprotect`, `munmap`, `mremap` with `MREMAP_MAYMOVE`, `madvise`,
  `msync`, `brk` refused.
- Time: `clock_gettime`, `clock_getres`, `nanosleep`, `clock_nanosleep`,
  `gettimeofday`, `time`, `times`.
- Self-pipe: `pipe`, `pipe2`, `fcntl` (`F_GETFL`, `F_SETFL`, `F_GETFD`,
  `F_SETFD`, `F_DUPFD`), `dup`, `dup2`, `dup3`, `close`, `read`, `write`,
  `poll`, `ppoll`, `select`, `pselect6` over an in-memory ring per pipe.
- Signals: `rt_sigaction`, `rt_sigprocmask`, `sigaltstack` record state;
  `kill`, and `tkill`/`tgkill` aimed at the calling lane, deliver inline, so
  `raise` and `abort` behave as on Linux and a default terminating action
  ends the program with status 128 + signal. `tkill`/`tgkill` aimed at
  another lane queue the signal for delivery when that lane next runs and
  end a blocking wait it is in with `EINTR`; signal 0 is the liveness probe.
- Threads: `clone` through `nelibc_clone_thread` (the wrapper `pthread_create`
  uses), `futex` (wait, wake, wake-op, requeue, cmp-requeue, and the
  priority-inheritance lock, unlock, and trylock operations), `sched_yield`,
  `sched_getaffinity`, scheduler parameter queries, `set_tid_address`,
  `set_robust_list`/`get_robust_list` with the robust-list walk at thread
  exit, `arch_prctl(ARCH_SET_FS/ARCH_GET_FS)`. See "Lanes" below.
- Identity and limits: `getpid` 1, `gettid` per lane (the initial lane is
  1), uid/gid 0, `uname`, `getrlimit`/`prlimit64` with fixed values,
  `getrusage` zeros, `getcwd` "/", `umask`, `prctl(PR_SET_NAME/PR_GET_NAME)`.
- Files: every path operation returns `ENOENT`; `lseek`, `pread`, `pwrite`
  return `ESPIPE`; `ioctl` returns `ENOTTY`.
- Process: `exit` ends the calling lane (and the program, through
  `host->exit`, when it was the last one); `exit_group` calls `host->exit`;
  `fork`, raw `clone`, `execve`, `wait4` return `ENOSYS` or `ECHILD`.

Anything else returns `ENOSYS` after `host->unsupported`.

### Lanes

The host supplies one hardware thread; the profile multiplexes every thread
the program creates onto it as a cooperative lane:

- `pthread_create` works. `nelibc_clone_thread` makes the new lane on the
  stack and TLS block `thread.nelua` prepared, writes the thread id to the
  `CLONE_PARENT_SETTID` word, and runs the new lane first, until it blocks,
  yields, or exits; the creator then continues. (Linux promises no order;
  this one keeps a main thread that never waits from starving the workers
  it made, which is how the Bend pool starts.)
- A lane runs until it blocks in the dispatcher (a futex wait, `nanosleep`,
  a read on an empty pipe, a write on a full one, `poll`/`select` with
  nothing ready), calls `sched_yield`, or exits. The dispatcher then saves
  the callee-saved registers and stack pointer, installs the next lane's
  thread pointer through `thread_pointer_set`, and resumes it. Nothing
  preempts a lane: a loop that never enters the dispatcher starves the
  others, as on a one-CPU kernel without a timer interrupt.
- Futex wake, requeue, and the PI unlock move lanes between the blocked and
  runnable sets; `FUTEX_LOCK_PI` records the owner's thread id and the
  waiters bit exactly as the kernel does, so `thread.nelua`'s mutex fast
  paths stay valid. A thread exit clears the `CLONE_CHILD_CLEARTID` word,
  wakes its joiner, and walks the robust list.
- Timed waits sleep through `host->sleep` only when no lane is runnable;
  otherwise other lanes run until the deadline. Up to 64 lanes exist at
  once (`NELIBC_HOST_LANE_COUNT`); `pthread_create` reports `EAGAIN` past
  that.
- A wait that no lane could end, such as a futex wait without a timeout when
  every other lane is also blocked without one, stops the program through
  `host->exit` with status `NELIBC_HOST_EXIT_DEADLOCK` (125) after a message
  on descriptor 2.
- `sigaltstack` and the signal mask are per lane; a new lane inherits its
  creator's mask and starts without an alternate stack, as on Linux.

Kernel (preemptive) threads remain possible later because the host still
sees only `nelibc_clone_thread`, `futex`, and `exit`; a host that supplies
real threads would replace the lane scheduler in `syscall_host.c` and keep
the rest.

### Faults

The profile records `sigaction` and `sigaltstack`; the host's exception path
delivers. `nelibc_host_fault(signal, code, address, context)` is the entry a kernel
calls from its page-fault, general-protection, divide-error, or
invalid-opcode handler with the Linux signal the fault maps to (`SIGSEGV`,
`SIGBUS`, `SIGFPE`, `SIGILL`), a `si_code`, and the faulting address. The
profile runs the recorded handler on the recorded alternate stack when the
action has `SA_ONSTACK` and the fault did not already occur on that stack,
with a `siginfo_t` carrying `si_signo`, `si_code`, and `si_addr` under
`SA_SIGINFO`, and `context` passed through as the handler's third argument
(the host's machine context: the Linux harness passes the kernel's
`ucontext_t`, a kernel passes its trap frame or NULL). A handler that returns makes the call return 0 and the host
resumes the faulting instruction, so the handler must have repaired the
fault, as on Linux. A handler that exits or unwinds never comes back. With no
handler, `SIG_DFL`, or `SIG_IGN`, the profile stops through `host->exit`
with status 128 + signal after a message on descriptor 2. That is Bend's
fail-stop message for stack overflow: it needs `sigaltstack` plus a real
guard page, so `memory_protect` is what makes the guard real; a host that
leaves it NULL has advisory guards only.

`tools/host_linux.c` shows the host side: it catches the four signals from
Linux on a private alternate stack and forwards them, with `SA_NODEFER` so a
handler that unwinds leaves the signal deliverable. `tools/host_smoke.c`
takes two `SIGSEGV`s on a `PROT_NONE` page: the first handler unprotects the
page and returns, and the write resumes; the second runs on the recorded
alternate stack, checks `si_addr`, and exits through the libc so `atexit`
still runs.

## Build and check

```sh
make host-matrix                                   # archives, link checks, hosted smoke (Zig and GCC)
make host-bend-hello-zig BEND=/path/to/bend        # Bend 2.0.39 IO.print through the profile
make host-bend-parallel-zig BEND=/path/to/bend     # Bend fork-join workers as lanes, 65536
make host-bend-run-zig BEND_C=program.c BEND_ARGS='+RTS --threads 1 --gpu off'
```

`build/libnelibc-host-zig.a` and `build/libnelibc-host-gcc.a` are the standalone
archives minus `start-linux-x86_64-standalone-*.o`, with the weak raw-syscall
aliases stripped from the assembly helper object and `host-syscall-*.o` plus
`host-boot-*.o` added. `host-check-*` links each archive whole with `-r` and
requires `nm -u` to be empty, the same gate Drift's `native-check` applies to
its drivers: nothing outside the archive and the host table is needed.

`host-smoke-*` is a hosted consumer. `tools/host_linux.c` is the "kernel": it
fills the table with raw Linux syscalls for console, memory, clock, thread
pointer, and exit, reads PT_TLS from the ELF header, and calls
`nelibc_host_run`. `tools/host_smoke.c` is the program: stdio, environment,
allocator growth, zero-filled reservations, formatting, clocks and sleeps, the
self-pipe with `select` and `O_NONBLOCK`, signal registration and inline
`raise`, affinity, pthreads as lanes (a thousand mutex-and-condition turns
with a worker lane, a pipe handoff, join with a result, per-lane TLS and
`gettid`, a detached lane), faults, `atexit`, and a nonzero exit status.
The linked executable must also have no undefined symbols.

Consumers compile against `include/` with `-DNELIBC_STANDALONE
-ffreestanding -fno-builtin -nostdlib`. The Bend-generated file additionally
needs a clang-based driver (`zig cc`) for `musttail`.

## Evidence

Recorded on 2026-11-04 in an Incus container (Debian 12, bak) with Nelua
`3c29419` (Spaceghost/nelua-lang, the Drift pin), Zig 0.17.0, GCC 12.2, Bend
2.0.39 (release archive sha256 `6cf462fd…c7ab`):

- `make host-check-zig` and `make host-check-gcc`: no undefined symbols in
  either host archive, and the disassembly of the whole-archive link holds
  no `syscall` instruction outside `nelibc_rt_sigreturn` (the restorer the
  Linux profiles install; the dispatcher never installs it).
- `make host-smoke-zig` and `make host-smoke-gcc`: both print the three
  smoke lines and the `atexit` line and exit 7; both executables have no
  undefined symbols. The smoke's lane section (worker lane, pipe handoff,
  join, detached lane) passed on both.
- `make host-snapshot-matrix`: the generated-C snapshot rebuilt with Zig and
  with GCC passes the same checks, including the `syscall` scan.
- The Bend hello program linked against `libnelibc-host-zig.a` printed `hi`
  and exited 0 at `+RTS --threads 1`, `2`, `4`, and `8`; its worker pool
  never opens for a sequential program, so that run created no lanes.
- `tools/host_bend_parallel.bend` (a fork-join `pow2(16n)`, 65536) compiled
  with Bend 2.0.39, linked the same way, and run with `NELIBC_HOST_STATS=1`
  printed `65536` and exited 0 at every count; the harness reported 0, 1,
  3, and 7 lanes created and 0, 2, 6, and 14 lane switches at `--threads
  1`, `2`, `4`, and `8`. Each worker ran first to its pool wait and the main
  lane then finished the rows itself, so this shows the pool's threads
  being created and scheduled as lanes, not a parallel speedup.
- `make host-bend-hello-zig`: the emitted `IO.print("hi")` program linked
  against `libnelibc-host-zig.a` and `tools/host_linux.c` prints `hi` and
  exits 0.
- `make host-bend-run-zig BEND_C=example.c BEND_ARGS='+RTS --threads 1 --gpu off'`
  with `example.c` emitted from Writ `7ad85be`'s `Example.bend`: the program
  prints its 16 `PASS` lines (the negotiation, capability, commit, and
  revocation checks) and exits 0.

This is hosted execution under Linux. It shows the libc surface works above
the table and that the unmodified Bend-generated C links and runs against it.
It is not a boot, not a kernel, and not evidence for any Writ host
qualification; those need the serial-console runs the Writ specification
describes on an actual x86-64 image.

## Working with Drift

Drift's kernel example (`examples/kernel`) already supplies register access,
DMA memory, a PIT-calibrated TSC clock, and serial output to its drivers
through `drift_hw`. The same code supplies `nelibc_host`: `console_write` is
the serial routine, `clock_monotonic` is the TSC clock, `memory_reserve` is an
arena, `exit` is the debug-exit port. Two gaps remain before Bend runs there:
Drift's reference kernel is 32-bit Multiboot and the Bend term is a 64-bit
word, so a long-mode variant is needed; and that kernel must enable SSE and
AVX state (`CR4.OSFXSR`, `OSXSAVE`, `XCR0`) because the standalone memory
routines probe for AVX2 with `cpuid`/`xgetbv`. Both are kernel work, not libc
work, and are tracked in Writ's freestanding runtime tasks.

## Generated-C snapshot

`make host-snapshot` runs `tools/host_snapshot.sh`, which lowers every Nelua
module of the host archive to C with the pinned Nelua (`nelua -c -r`) and
writes `build/host-snapshot/` with those files, the C and assembly sources
the archive links with, a copy of `include/`, a `SNAPSHOT` provenance line
(Nelibc commit, Nelua version, time), and a plain `Makefile`. From the
snapshot a consumer needs a C compiler, `ar`, and `objcopy` only:

```sh
make -C build/host-snapshot CC='zig cc -mcpu=x86_64' O=obj-zig
make -C build/host-snapshot CC=gcc O=obj-gcc
```

`make host-snapshot-matrix` builds the archive from the snapshot with Zig and
with GCC, links each whole with `-r` and requires `nm -u` to be empty, then
links and runs the hosted smoke against it. Recorded on 2026-11-04 in the
same container as the evidence above (main `f1c716c` plus the `.quad`
spelling of the fenv constants): both compilers pass. The snapshot is a
build artifact, not checked in; regenerate it from the commit you want to
ship.

## Not done

- Kernel (preemptive) threads behind the table; lanes are cooperative.
- The signal mask is recorded, not enforced: a blocked signal is still
  delivered by `raise`, `kill`, and `tkill`.
- Any architecture other than x86-64.
