# nelibc

A C ABI libc whose functions are replaced incrementally by implementations in
Nelua. The project starts as a library linked against the host C runtime, then
removes that dependency one layer at a time until the runtime and startup path
can stand alone.

## Start here

Build and run a small C program with Nelibc replacing selected functions:
`make dogfood-zig`. The [quickstart](docs/quickstart.md) has commands for the
GCC, musl, and TinyCC profiles. The [core roadmap](docs/core-roadmap.md) lists
the highest-priority gaps and explains the limits of the API coverage count.

## License

Nelibc is distributed under the MIT License. See
[`THIRD_PARTY_NOTICES.md`](THIRD_PARTY_NOTICES.md) for the fdlibm-derived math
constants and kernels, whose original notice is preserved.

## Replacement model

Each function has a stable C ABI and an independent conformance/benchmark
entry. During bootstrap, exported Nelua symbols can coexist with the host
libc. A function becomes eligible for replacement when its implementation,
edge cases, ABI, and cross-libc behavior are established. Keep the compiler
runtime and platform syscall layer separate from the public libc surface.

The initial C complex-math slice replaces `cabs*`, `creal*`, `cimag*`,
`conj*`, `cproj*`, `carg*`, `csqrt*`, `cexp*`, `clog*`, `csin*`, `ccos*`,
`csinh*`, and `ccosh*`.
Nelua computes magnitude, conjugation, projection, arguments, complex square
roots, exponentials, logarithms, circular and hyperbolic sines and cosines; a
small C shim handles the x86-64 complex-value ABI.
`make complex-math-smoke-matrix` covers four hybrid profiles and three
standalone archives. TinyCC builds the Nelua core, while
Zig cc compiles the shim and complex-valued caller because TinyCC does not
parse `_Complex`.

## Build matrix

Zig is the default C toolchain (`zig cc`). `make smoke-zig` and
`make smoke-gcc` build and run the ABI smoke program against the host libc;
`make smoke-zig-musl` does the same with Zig's static x86_64 Linux musl target.
`make api-coverage` builds the seven host and standalone archives, then writes a
declaration/export/smoke-reference matrix to `build/api-coverage.csv`; see
[`docs/api-coverage.md`](docs/api-coverage.md) for its scope and status meanings.
`make api-coverage-available` writes a hybrid-only matrix for Zig/glibc,
GCC/glibc, and Zig/musl when TinyCC is unavailable.
`make shootout-zig`, `make shootout-gcc`, and `make shootout-zig-musl` compare
the memory functions from the selected libc with the Nelua object linked into
the same C benchmark, including the scan routines and four `memmove` placement
cases. Both runs use `-O2 -fno-builtin`; output is CSV with
compiler, libc, implementation, function, size, iteration count, and median
nanoseconds per call. Each row is measured over seven samples. Results are
microbenchmarks, so compare repeated runs on the same machine. Optional
`make smoke-tcc` runs the Nelua memory API against bytewise references and
guard pages, including 64-byte copies at all 16 source and destination
alignments near both page edges. It also checks the TCC SSE2 helpers against
host libc (offsets 0–15, lengths 0–512), plus a glibc hybrid stdio probe and
the ABI smoke; it exercises
Nelua-backed `getchar`, `putchar`, `puts`, `fgetc`, `getc`,
`ungetc`, `fread`, `fwrite`, `fputc`, `fputs`, `freopen`, and `perror`. Run it
when TinyCC is installed. `make shootout-tcc` and `make shootout-musl-gcc`
cover TinyCC and a native musl toolchain when installed. The TCC hybrid builds
most C/Nelua objects with TinyCC; Zig cc compiles Nelua's generated C for the
formatter and stdlib hot paths and assembles the x86-64 SSE2 memory helper.
TinyCC 0.9.27 does not support C TLS or link the TLS relocations in the shared
environment adapter, so Zig cc builds that adapter and performs final links for
the TCC smoke consumers; their C harnesses and the remaining supported C/Nelua
objects are still compiled with TinyCC. TCC archive consumers also link
TinyCC's `libtcc1.a` for compiler runtime helpers.
GCC and Zig cc use a bounded SSE2 `memset` path for lengths
of at least 16 bytes, with Nelua handling shorter fills. The four-way memory
shootout is archived in
`benchmarks/results/2026-10-22-memory-shootout-sse2-memset-matrix.csv`.
`make smoke-syscall-tcc` also builds and runs the POSIX
syscall layer under TinyCC, linking the x86-64 syscall assembly with Zig cc.
TinyCC silently ignores GCC's `packed` attribute on `epoll_event`, so the
project header uses `#pragma pack` around that ABI declaration for TinyCC; the
syscall harness asserts its 12-byte size and data offset of 4.
The 2026-10-21 reference CSVs and host details are recorded in
`benchmarks/results/`.
`make smoke-matrix` runs the available Zig/GCC/TinyCC, glibc/musl, syscall,
freestanding, standalone, and floating-point differential checks. Its Zig/glibc,
GCC/glibc, and Zig/musl smoke targets run `tools/memory_diff.c`, which checks
the exported memory API against bytewise reference implementations and guard
pages, including `explicit_bzero` at both page boundaries, long periodic needles,
and 256 deterministic randomized
`memmem` cases with inserted matches and guaranteed-absent needles. The same
`memmem` differential harness also runs under TinyCC via `make smoke-tcc`.
Guard-page `memmove` checks exercise forward and backward overlap at 2,048,
2,049, and 4,096 bytes, with one range ending at the page boundary.
`make float-diff-matrix` compares `strtof`, `strtod`, and `strtold` against the
host libc for 1,250 fixed, generated, and over-capacity inputs on x87 hosts
(1,243 otherwise), under Zig/glibc, GCC/glibc, Zig/musl, and TinyCC/glibc. It
also directly probes the internal BigUInt divider's 64-bit quotient and
normalization-capacity rejection paths on all four builds. Its radix smoke
uses a comma-decimal callback to check `strtof`, `strtod`, `strtold`, hexadecimal
input, and end pointers under Zig/glibc, GCC/glibc, Zig/musl, and TinyCC/glibc.
`make float-shootout-matrix` compares those three
parsers against locale-specific host libc parsers over five decimal and
hexadecimal inputs, reporting seven-sample median nanoseconds per call for
Zig/glibc, GCC/glibc, Zig/musl, and TinyCC/glibc. The matrix writes its merged
CSV to `build/float-shootout-matrix.csv`; the reference run is
`benchmarks/results/2026-10-21-float-inline-digit-shootout.csv`. The current
deferred-BigUInt matrix is archived at
`benchmarks/results/2026-10-22-float-deferred-biguint-matrix.csv`; the five-run
TinyCC comparison is at
`benchmarks/results/2026-10-22-float-tcc-deferred-biguint-shootout.csv`.
Seven repeated musl parser pairs are in
`benchmarks/results/2026-10-22-float-musl-deferred-biguint-shootout.csv`.
The TinyCC-specific virtual remainder compare is measured in 12 alternating
pairs per compiler/libc build; see
`benchmarks/results/2026-10-22-float-virtual-remainder-compare-matrix.csv`.
Eight TinyCC pairs on long decimals are archived in
`benchmarks/results/2026-10-22-float-virtual-remainder-long-tcc-ab.csv`.
`make float-long-shootout-matrix` runs the same comparison on two roughly
600-digit decimal inputs, with five-sample medians of 160 calls, to expose
BigUInt scaling costs across those compiler/libc combinations. It writes
`build/float-long-shootout-matrix.csv`.
`make float-shootout-tcc-opt` repeats the TinyCC/glibc comparison with `-O0`
through `-O3` and writes `build/float-shootout-tcc-opt.csv`.
`make int-shootout-matrix` measures `strtol`, `strtoul`, `strtoll`, and
`strtoull` on five representative inputs with Zig/glibc, GCC/glibc, Zig/musl,
and TinyCC/glibc. Each row is a seven-sample median over 1,000,000 calls per
sample. The merged CSV is `build/int-shootout-matrix.csv`; the
2026-10-22 reference results are in `benchmarks/results/2026-10-22-int-shootout.csv`.
`make wide-int-diff-matrix` compares all four wide integer parsers against the
host libc over 1,210 deterministic inputs with Zig/glibc, GCC/glibc, Zig/musl,
and TinyCC/glibc.
`make wide-string-diff-matrix` compares the wide memory, copy, search, span,
concatenation, and tokenization functions over 256 deterministic cases with
the same compiler/libc combinations; `make lib-consumer-matrix` includes both
wide-character differential matrices.
`make rand-r-shootout-matrix` compares host and Nelibc `rand_r` with Zig/glibc,
GCC/glibc, Zig/musl, and TinyCC/glibc; it writes
`build/rand-r-shootout-matrix.csv`. The 2026-10-22 reference run is recorded in
`benchmarks/results/2026-10-22-rand-r-shootout.csv`.
`make rand-r-diff-matrix` checks 16,384 calls against an independent SplitMix64
reference across those same four hybrid builds; it is part of
`make lib-consumer-matrix`. The standalone consumer matrix runs the same
sequence check against Zig and GCC standalone archives and verifies that each
test executable has no undefined symbols.
`make int-diff-matrix` checks the `strto*` integer conversions against the
host libc in separate executables over fixed and generated inputs for
Zig/glibc, GCC/glibc, Zig/musl, and TinyCC/glibc. It compares values and
`errno`, plus end pointers when the result is in range; libc implementations
differ in end pointer placement on overflow and no-conversion inputs. Both
differential matrices are part of `make smoke-matrix`.
`make shootout-matrix` runs the memory, float, integer, qsort/bsearch, and stdio shootouts
with Zig/glibc, GCC/glibc, Zig/musl, and TinyCC/glibc. The memory results are
saved to `build/mem-shootout-matrix.csv`; string, float, and integer shootout
legs use their own matrix targets where available. The October 22, 2026 full
run's memory results are in `benchmarks/results/2026-10-22-memory-shootout.csv`;
the current `rep movsb` comparison is in
`benchmarks/results/2026-10-22-memory-shootout-repmovsb-matrix.csv`;
the October 24 four-profile run with needle-length-labeled substring cases is
`benchmarks/results/2026-10-24-memory-shootout-labelled-cases.csv`;
the follow-up run after adding a legal-window tail-byte filter to `memmem` is
`benchmarks/results/2026-10-24-memory-shootout-memmem-tail-filter.csv`. Across
the four profiles, the filter makes the measured 64 KiB periodic-absent cases
return in roughly 0.28–0.36 microseconds; the previous TinyCC Nelibc measurements
were roughly 0.26–0.48 milliseconds. Tail-hit cases retain the existing Two-Way
search and remain comparable to the prior run. The follow-up includes the
benchmark's host and Nelibc rows for each compiler/libc profile.
string, float, and integer CSVs are in
`benchmarks/results/2026-10-22-{strstr,strcasecmp,float,int,stdio}-shootout.csv`.
`make lib-zig`, `make lib-gcc`, `make lib-zig-musl`, and `make lib-tcc` build
modular static archives in `build/`. Each archive contains the currently
implemented memory, string, ctype, integer/float conversion, prefixed Nelua
allocator, POSIX syscall wrappers, hybrid stdio wrappers, Linux x86-64 syscall
backend, host errno, locale and environment adapters, and the C ABI adapters
for variadic `fcntl`/`mremap` and credential setters for that compiler/libc
pair. The stdio wrappers use each host libc’s `FILE` accessors and
unlocked operations through a small C adapter. Glibc builds skip FILE locking
while `__libc_single_threaded` is true and keep the lock token through each
operation if a thread starts concurrently; musl keeps its host FILE locks.
Hybrid archives export `waitpid` through the C ABI adapter and Nelua's
cancellation-aware wait syscall, so host `system()` cleanup handlers run when
a waiting thread is canceled. Nelua cancellation
syscalls use host-specific backends: glibc builds resolve versioned
cancellation-aware wrappers with `RTLD_NEXT`, while static musl builds use
musl's cancellation-aware syscall entry point. If a glibc symbol version is
unavailable, that call falls back to the raw syscall cancellation bridge.
Standalone archives export the same Nelua syscall path without pthread
dependencies. `make unistd-cancel-matrix` checks blocked descriptor, socket,
poll, wait, and FIFO-open calls, plus cancellation during repeated `fsync`,
across Zig/glibc, GCC/glibc, Zig/musl, and TinyCC/glibc.
The allocator is available through `include/nelibc_alloc.h` as
`nelibc_malloc` and related prefixed calls. Zig/glibc, GCC/glibc,
TinyCC/glibc, and Zig/musl hybrid archives export Nelua `malloc`. The glibc
profiles also export `free` and `realloc`; static musl routes those calls
through linker wrappers that inspect allocation ownership and send host-owned
pointers to musl's allocator. `realloc(NULL, size)` uses Nelibc in every
profile, matching public `malloc`. Nelua `reallocarray(NULL, count, size)`
inherits the same behavior.
Hybrid `calloc`, `aligned_alloc`, and `posix_memalign` allocate through Nelibc;
`posix_memalign` preserves its POSIX return-code and `errno` behavior.

Static musl has many libc objects with unresolved allocator calls. Linking
Nelua `malloc` makes those calls resolve to Nelibc; `free` and `realloc` remain
ownership-dispatched so allocations made by musl's `calloc` and other host
paths still reach musl. A static-musl consumer must use
`MUSL_STDIO_WRAP_FLAGS`. The full ABI consumer and an isolated probe covering
locale, directory, regex, word expansion, resolver, environment, and stdio
allocations pass with this routing. The archives replace only the functions
they define; uncovered functions and locale data still come from the host
runtime during bootstrap. `make lib-smoke-matrix` links the ABI smoke program
through each archive.
Glibc hybrid archives provide Nelua `atexit` through the host's
`__cxa_atexit`, passing the executable or shared object's DSO handle. Static
musl keeps its native `atexit`, because its libc archive already defines that
symbol strongly. Glibc hybrid archives also provide Nelua `fclose` through
glibc's `_IO_fclose`, preserving host stream ownership. The musl hybrid routes
`fclose` through a Nelua `__wrap_fclose` entry point to musl's native function.
Musl `localeconv` also passes through a Nelua wrapper that returns the host's
`struct lconv`; its libc specific layout stays owned by musl.
Musl `setlocale` passes through Nelua to update that same host locale state.
Glibc and TinyCC/glibc hybrids use Nelua entry points backed by a cached
`RTLD_NEXT` lookup, so the host locale state and `struct lconv` remain intact.
The musl hybrid also exports `atexit` through a Nelua wrapper to musl's
registration function; the callback-order smoke test covers this path.
Hybrid `exit` is a Nelua entry point that forwards to the host termination
function, preserving host callback ordering and stdio cleanup.
`make lib-stdio-smoke-available` exercises the hybrid standard-stream wrappers
with installed Zig/glibc, GCC/glibc, and Zig/musl toolchains plus the fmemopen
differential checks. `make lib-stdio-smoke-matrix` also includes TinyCC.
against glibc, musl, and TinyCC/glibc archives. Hybrid archives also provide
Nelua `rewind`, which seeks host streams to the beginning and clears their
EOF/error indicators without depending on host `FILE` layout. Nelua `fdopen`
uses each host libc's native `FILE` constructor, so the returned stream remains
owned by that libc.
Glibc hybrid archives also export Nelua `flockfile`, `ftrylockfile`, and
`funlockfile` wrappers through glibc's `_IO_*` lock entry points. The musl
hybrid routes these functions through Nelua `__wrap_*` entry points to musl's
native lock functions.
Glibc hybrid `fseek` and `ftell` call Nelua `fseeko` and `ftello` entries,
which forward to the host's distinct large-file functions without exposing
host `FILE` layout. TinyCC/glibc uses the same forwarding path.
The musl hybrid routes all four functions through `__wrap_*` entries to their
`__real_*` implementations, avoiding duplicate symbols from musl's shared
`fseeko`/`ftello` archive objects.
Glibc hybrid `fgetpos` and `fsetpos` use glibc's `_IO_*` entry points through a
C adapter, keeping the host's `fpos_t` representation opaque to Nelua. Musl
keeps its native position functions because its `fpos_t` uses a different
representation.
Glibc hybrid `setvbuf` uses `_IO_setvbuf`; hybrid `setbuf` is a Nelua wrapper
that uses the host's `BUFSIZ` value and buffering mode. Musl keeps native
`setvbuf` semantics through the `__wrap_setvbuf` Nelua entry point; `setbuf`
uses the same wrapper with musl's `BUFSIZ` value and buffering mode.
Hybrid `setbuffer` and `setlinebuf` are Nelua wrappers over `setvbuf`, so
buffering modes remain delegated to the selected host runtime.
`include/stdio.h` declares Nelibc's `fseeko`/`ftello` entry points under
`_LARGEFILE_SOURCE` or `_GNU_SOURCE`, and unlocked I/O plus `setbuffer`/
`setlinebuf` under `_DEFAULT_SOURCE` or `_GNU_SOURCE`. The stdio smoke matrix
checks unlocked string input/output through Nelua across all four profiles.
Hybrid `open_memstream` keeps the `FILE` host-owned through `fopencookie` while
Nelua manages its growable buffer and seek position. The result buffer remains
host-allocated and can be released with the host's `free` after `fclose`.
Hybrid `fmemopen` uses the same host cookie-stream constructor, with Nelua
callbacks managing caller-provided or internally allocated memory buffers.
`make fmemopen-diff-matrix` compares mode parsing, zero-size streams, buffer
termination, reads, append writes, and seeking against the matching host libc
under Zig/glibc, GCC/glibc, and Zig/musl.
Hybrid `fopen` parses the mode and opens the descriptor in Nelua, then hands it
to the host `fdopen` constructor. If stream construction fails, it closes the
descriptor and preserves the host error code.
Hybrid `tmpfile` creates an exclusive temporary file, unlinks it immediately,
and gives the descriptor to the host `fdopen` constructor so stream ownership
stays with the host libc.
Nelua `printf`, `fprintf`, `vfprintf`, `dprintf`, `vdprintf`, `snprintf`,
`vsnprintf`, `sprintf`, and `vsprintf`. Hybrid `asprintf` and `vasprintf` also
format through Nelua `vsnprintf`, with host-allocated results that callers can
release with host `free`. The host stream bridge writes through the host's
unlocked `FILE` operations while holding its lock, so the host `FILE` ABI stays
unchanged. The formatter object keeps its nine standard symbols public in
hybrid builds, alongside its internal host-stream bridge.
`make stdio-format-diff-matrix` compares `sprintf` and `vsprintf` output with
host libc across integer, floating, length-modifier, and `%n` cases. This
comparison exposed and fixed `%n` count storage for the supported Linux x86-64
length modifiers. `make
locale-format-hybrid-diff-matrix` compares the hybrid formatting functions
against glibc, musl, and TinyCC host formatting over localized and randomized
integer, fixed, scientific, and general floating-point cases.
`make stdio-shootout-available` compares host and Nelibc `putchar`, `puts`,
`fputc`, `fputs`, `snprintf`, `sprintf`, `vsprintf`, `fprintf`, `fmemopen`,
`fread`, and `fwrite` across the available Zig/glibc, GCC/glibc, and Zig/musl
profiles. `make stdio-shootout-matrix` also includes TinyCC. Both report
seven-sample medians. The available results are written to
`build/stdio-shootout-available.csv`; the 2026-10-22 reference is
`benchmarks/results/2026-10-22-stdio-shootout-available.csv`.
The `fmemopen` workload runs 10,000 open/write/close cycles per sample. `fread`
and `fwrite` each run 200,000 one-byte operations on temporary files. The
fmemopen-specific comparison is recorded in
`benchmarks/results/2026-10-22-fmemopen-shootout.csv`.
The TinyCC hybrid formatter compiles Nelua's generated C with Zig cc `-O2`:
TinyCC's code generator produced a roughly 10 KiB stack frame in `vfprintf`
and formatting was about four times slower than glibc. The optimized object
cuts the measured `snprintf` gap to about 1.4x and `fprintf` to about 1.7x.
`make io-shootout-matrix` compares host and Nelibc scalar `read`/`write` and
`readv`/`writev` calls with one, two, and four one-byte vectors, plus
`pread`/`pwrite` and `preadv`/`pwritev` on an unlinked temporary file, plus
`preadv2`/`pwritev2` with flags zero. It reports seven-sample medians of
200,000 calls across the same compiler/libc combinations. It writes
`build/io-shootout-matrix.csv`; the reference run is
`benchmarks/results/2026-10-22-io-shootout.csv`.
For an ordinary host-linked consumer exercising memory, string, integer conversion,
allocation, `setenv`/`unsetenv`/`putenv`, and environment propagation across `execve`, run `make lib-zig`, then:

```sh
zig cc -pthread -Iinclude -fno-builtin tools/lib_consumer_smoke.c \
  build/libnelibc-zig.a -o build/lib-consumer
build/lib-consumer
```

`make lib-consumer-matrix` runs the same consumer against Zig/glibc, GCC/glibc,
Zig/musl, and TinyCC/glibc archives. It checks the prefixed allocator's
alignment, `calloc` zeroing, `realloc` preservation, overflow handling, and
`posix_memalign` errno behavior, plus concurrent allocation/reallocation/free
stress from four threads on hybrid builds. The musl command uses
`zig cc -target x86_64-linux-musl -static` and
`build/libnelibc-zig-musl.a`.
The consumer matrix runs `strerror_r` differential coverage against glibc's
XSI entry point for shared errno values, plus known-message and
undersized-buffer cases. Hybrid archives now export the standard symbol from
Nelua with the selected libc ABI: GNU builds return `char *`, while musl
returns the POSIX `int` status. The tests call the actual archive symbol for
both forms. Standalone headers map XSI calls to the POSIX implementation and
`_GNU_SOURCE` calls to the separate `nelibc_gnu_strerror_r` implementation.
`make lib-standalone-zig`, `make lib-standalone-gcc`, and
`make lib-standalone-tcc` build Linux x86-64 standalone archives. The TCC
target compiles the Nelua runtime modules with TinyCC, then uses Zig cc for the
x86-64 assembly, C adapters that need GNU atomic builtins, and the freestanding
link. Its archive includes the TinyCC runtime helpers required by its objects.
TLS fixtures and syscall interposition hooks stay in the separate test harness.
`make smoke-lib-standalone-matrix` links that harness with each archive under
`-nostdlib`, checks that the executable has no undefined symbols, and runs it.
`make lib-consumer-standalone-matrix` runs the public consumer against the
Zig-, GCC-, and TinyCC-built standalone archives. The TinyCC archive is linked
with Zig cc, checked for unresolved symbols, and run; the consumer exercises
the standard standalone allocator entry points as well as the rest of the ABI.
The same matrix links and runs `tools/pthread_standalone_smoke.c` against all
three archives to verify pthread join, exit, detach, deferred cancellation,
condition-wait wakeup, and cleanup-handler paths through the shipped headers
and libraries. Named semaphore coverage includes concurrent creators, repeated
opens, cross-process posts, cancellation, and unlinking while handles remain
open.
For a consumer executable, run `make lib-standalone-zig`, then:

```sh
zig cc -DNELIBC_STANDALONE -nostdlib -static -no-pie -fno-pie \
  -fno-stack-protector -ffreestanding -fno-builtin -fno-sanitize=all \
  -Wl,-e,_start -Wl,--build-id=none -Wl,--whole-archive \
  build/libnelibc-standalone-zig.a -Wl,--no-whole-archive \
  -Iinclude tools/lib_consumer_smoke.c -o build/lib-consumer-standalone
nm -u build/lib-consumer-standalone
build/lib-consumer-standalone
```

The undefined-symbol check should print nothing. `make lib-consumer-standalone-matrix` runs this consumer with Zig and GCC. Both
archive link checks are part of `make smoke-matrix`.
`make memcmp-shootout-matrix` compares host libc, the SSE2 baseline, and the
current CPUID-dispatched `memcmp` across Zig/glibc, GCC/glibc, Zig/musl, and
TinyCC/glibc. `tools/memory_diff.c` includes long-buffer `memcmp` checks at
vector and dispatch boundaries, with mismatches across four pointer offsets;
these run in `make smoke-matrix` and `make smoke-tcc`.
`make alloc-shootout-matrix` compares host allocation calls with the prefixed
Nelua allocator under Zig/glibc, GCC/glibc, Zig/musl, and TinyCC/glibc. It
measures `malloc`/`free`, `calloc`/`free`, double-size and same-class
`realloc`/`free`, and 64-byte `aligned_alloc`/`free` at 64, 128, and 4096
bytes; it also measures ownership checks for slab and direct-map pointers, plus
hybrid bridge `free`/`realloc` for host-owned and Nelibc-owned pointers. Each
row is the median of seven 2,000-call samples. The initial
per-allocation-mapping baseline is
`benchmarks/results/2026-10-21-alloc-shootout.csv`; the current size-class
pool results are `benchmarks/results/2026-10-21-alloc-tcc-branches-shootout.csv`.
The baseline 2026-10-22 run is in
`benchmarks/results/2026-10-22-alloc-shootout.csv`; the per-thread batch
prototype is in `benchmarks/results/2026-10-22-alloc-batched-shootout.csv`.
The 2026-10-22 run after adding slab/direct-map provenance tracking is
`benchmarks/results/2026-10-22-alloc-shootout-provenance.csv`; it measures the
prefixed allocator paths, not the hybrid ownership bridge.
The public-symbol shootout is
`benchmarks/results/2026-10-22-alloc-shootout-public-current.csv`. It links the
hybrid archives and compares host allocator entry points, prefixed Nelua calls,
and actual public Nelua allocator symbols. At 64 bytes, public malloc/free
medians are 11.4 ns with Zig/glibc, 10.2 ns with GCC/glibc, and 28.5 ns with
TinyCC/glibc, versus 5.0, 4.2, and 5.8 ns for the host. Those values predate
the static-musl public `malloc` export. The prefixed musl allocator measured
7.7 ns versus 21.5 ns for musl's native `malloc`. The
lock-free slab registry lowered 64-byte Nelua ownership checks to 3.1–6.5 ns
across the four profiles. These
are seven-sample medians of 2,000 calls and machine-specific.
The refreshed prefixed threaded shootout is
`benchmarks/results/2026-10-22-alloc-thread-shootout-fastpath-current.csv`.
It enables thread-local cursors in this benchmark because it calls only the
prefixed allocator with the guarded TSD initialization path. At eight workers
and 64 bytes, Nelua records 2.11 ns per pair with Zig/glibc, 1.04 ns with
GCC/glibc, 1.61 ns with Zig/musl, and 4.33 ns with TinyCC/glibc. These numbers do not represent public malloc interposition.
A focused Zig/musl rerun is in
`benchmarks/results/2026-10-22-alloc-shootout-zig-musl-current.csv` and
`benchmarks/results/2026-10-22-alloc-thread-shootout-zig-musl-current.csv`.
At 64 bytes, Nelua's prefixed malloc/free measures 10.32 ns versus musl's
26.68 ns; at eight threads the pair throughput is 1.14 ns versus 3.64 ns.
These results compare prefixed allocator calls and predate public musl
`malloc` interposition.
The current public musl comparison is
`benchmarks/results/2026-10-22-alloc-public-zig-musl-shootout.csv`, generated
by `make alloc-public-zig-musl-shootout`. It runs 24 alternating process pairs;
each row is the median of seven 2,000-call samples. At 64 bytes, public Nelibc
`malloc/free` is 10.55 ns versus 14.39 ns for musl, and aligned allocation is
11.16 ns versus 14.27 ns. Grow-realloc is faster through 4 KiB; the 32 KiB
case is 4.25 us versus 4.20 us for musl after adding an `mremap`
fast path for direct mappings. These measurements are machine-specific.
The refreshed direct-map bridge rows in the public-symbol CSV show 32 KiB
realloc/free at 2.36–2.40 us across all four profiles, down from 15.4–15.9 us
before page-rounded direct-map capacity was used for realloc reuse. The bridge
case allocates 32 KiB, grows it by 16 bytes, then frees it; page slack lets
realloc keep the original mapping.
The bridge run is in
`benchmarks/results/2026-10-22-alloc-hybrid-bridge-shootout.csv`.
After consolidating slab descriptors into one list, the follow-up run is
`benchmarks/results/2026-10-22-alloc-hybrid-bridge-single-slab-list.csv`.
TinyCC's 64-byte host-pointer ownership check fell from 19.63 ns to 10.14 ns
in these runs.
The follow-up after removing redundant range-base locals from the ownership
checks is `benchmarks/results/2026-10-22-alloc-hybrid-bridge-no-range-locals.csv`.
This reduced TinyCC's ownership-check stack frame from 48 to 32 bytes; timings
varied between runs and are retained as measurements rather than a claimed speedup.
The `-fomit-frame-pointer` TinyCC comparison is
`benchmarks/results/2026-10-22-alloc-tcc-frame-pointer-shootout.csv`. TinyCC
0.9.28rc accepts `-O2` and `-fomit-frame-pointer`, but its extended help says
optimization levels are unavailable; `-On` only defines `__OPTIMIZE__`, and
linker `-O` is ignored. `-O0`, `-O2`, and `-fomit-frame-pointer` emitted
identical ownership-check assembly, so the timing differences were not treated
as a code-generation improvement.
On Zig/glibc, GCC/glibc, and Zig/musl the batch path improves 64–128-byte
`malloc/free` by 23–32%. TinyCC hybrid builds use C `__thread` storage for
allocator cursors, with pthread TSD retained only to register thread-exit
cleanup. Unused per-thread batches return to the shared pools at thread exit.
`make alloc-thread-shootout-matrix` measures aggregate `malloc/free` pair
throughput at 1, 2, 4, and 8 threads for the same sizes and toolchain/libc
matrix. It writes `build/alloc-thread-shootout-matrix.csv`; each row reports
median elapsed nanoseconds per pair normalized across all worker threads.
`make waitpid-shootout-matrix` compares host and Nelibc
`waitpid(-1, NULL, WNOHANG)` when no child exists, with seven samples of
200,000 calls under Zig/glibc, GCC/glibc, Zig/musl, and TinyCC/glibc. It writes
`build/waitpid-shootout-matrix.csv`; the 2026-10-22 results are in
`benchmarks/results/2026-10-22-waitpid-shootout.csv`. The cancellation-aware
wrapper adds 2.35–6.50 ns per call (1.6–4.5%) in this nonblocking `ECHILD` case.
`make eventfd-shootout-matrix` measures eventfd create/close and counter
write/read under those same four compiler/libc pairs, with seven samples of
100,000 operations. It writes `build/eventfd-shootout-matrix.csv`; the
2026-10-22 run is in `benchmarks/results/2026-10-22-eventfd-shootout.csv`.
`make getentropy-shootout-matrix` measures 32-byte and maximum-size 256-byte
`getentropy` calls with seven samples of 100,000 calls across the same matrix. Results are in
`benchmarks/results/2026-10-22-getentropy-shootout.csv`.
`make process-id-shootout-matrix` compares `getpid`, `getppid`, and `gettid`
with seven samples of 500,000 calls across the same matrix. Results are in
`benchmarks/results/2026-10-22-process-id-shootout.csv`; musl's host `gettid`
uses a thread-local cached TID, while Nelibc currently enters the kernel.
`make dirent-shootout-matrix` compares `opendir`/`readdir`/`closedir` scans
over 64 cached entries, open/close, and scans on a reused stream, with seven
samples of 1,000 operations across the same matrix. Results are in
`benchmarks/results/2026-10-22-dirent-shootout.csv`.
`make memscan-shootout-matrix` measures `memchr` and `memrchr` against those
same four compiler/libc combinations and writes its CSV to `build/`.
`make memscan-threshold-shootout` sweeps the AVX2 threshold and 128/256-byte
block sizes under Zig/glibc.
The x86-64 helpers use runtime-detected 256-byte AVX2 blocks for scans of at
least 2 KiB; SSE2 and scalar paths handle shorter scans and CPUs without AVX2.
`make strstr-shootout-matrix` compares host `strstr` and Nelua `strstr` under
Zig/glibc, GCC/glibc, Zig/musl, and TinyCC/glibc, with tail matches and
periodic absent needles from 4 to 256 bytes.
`make strcasecmp-shootout-matrix` compares host and Nelua `strcasecmp` and
`strncasecmp` over equal, early-mismatch, late-mismatch, and bounded-prefix
inputs, including first-byte and late bounded mismatches plus 95-byte and
256-byte equal and late-mismatch strings, under those same four
compiler/libc combinations. It writes the merged
CSV to `build/strcasecmp-shootout-matrix.csv`; the 2026-10-22 reference run is
`benchmarks/results/2026-10-22-strcasecmp-shootout.csv`. Timed inputs are copied
to page-aligned storage at 64-byte offsets so linker placement cannot trigger
page-edge SIMD fallbacks for only one compiler build.
The SSE2 fallback checks the original signed bytes against ASCII bounds, so
its vector loop no longer XORs each input byte with the high bit. Exhaustive
and randomized/page-edge differentials pass on all four rows. Three-run cycle
counter averages for 256-byte equal strings fell from 129–132 to 118–121 cycles
with this change. Its earlier shootout is preserved as
`benchmarks/results/2026-10-22-strcasecmp-before-sse2-fold-optimization.csv`.
The default x86-64 path now checks CPUID and XCR0 once, then uses page-safe,
256-byte AVX2 blocks when supported, with 128/64/32-byte page-edge bridges and
an SSE2 fallback.
The AVX2 path passed the four-row exhaustive and randomized/page-edge
differentials. A first-byte XOR fast path now detects bytes equal under ASCII
case folding and sends ordinary mismatches directly to scalar folding. On the
Zig/glibc row, 256-byte equal strings measured 46 cycles and 196 instructions
over three counter runs, down from 49 cycles and 201 instructions before this
change. Fresh exhaustive and randomized/page-edge comparisons pass for
Zig/glibc, GCC/glibc, Zig/musl, and the TinyCC-generated Nelibc objects. The
TinyCC row uses the refreshed assembly helper with a GCC benchmark harness
because the TinyCC executable is unavailable. The updated shootouts are archived in
`benchmarks/results/2026-10-22-strcasecmp-xor-prefix-shootout.csv`; the three
Zig/glibc counter runs are in
`benchmarks/results/2026-10-22-strcasecmp-xor-prefix-perf.csv`. TinyCC is not
installed in this environment, so the TinyCC compiler itself was not rerun. The
previous AVX2 results remain in
`benchmarks/results/2026-10-22-strcasecmp-avx2-perf-comparison.csv`. The prior
SSE2-only shootout is preserved as
`benchmarks/results/2026-10-22-strcasecmp-before-avx2-shootout.csv`.
The repeated glibc host baseline is 44 cycles; prior Zig/GCC Nelibc rows were
49 cycles, while the TinyCC-built object measured through GCC was 55 cycles.
The previous AVX2 loop measurements are preserved as
`benchmarks/results/2026-10-22-strcasecmp-before-avx2-256-unroll-shootout.csv`.
`make strcasecmp-sse2-fallback` checks the non-AVX2 object against the host
implementation and rejects any YMM instructions; the full shootout matrix
includes this fallback check.
`make strcasecmp-perf-matrix` records cycles, instructions, branches, and
branch misses for the page-aligned 256-byte equal and late-mismatch cases;
the 2026-10-22 counter run is archived at
`benchmarks/results/2026-10-22-strcasecmp-perf.csv`; the SSE2-only run is
preserved as `benchmarks/results/2026-10-22-strcasecmp-before-avx2-perf.csv`.
TinyCC-built Nelibc objects in the latest archives were measured through a
GCC-built harness because TinyCC was unavailable in the collection shell; the
harness compiler is identified in each CSV row.
`make strverscmp-shootout-matrix` compares host and Nelua GNU `strverscmp`
across Zig/glibc, GCC/glibc, Zig/musl, and TinyCC/glibc. It checks expected
ordering for numeric runs and leading zeros, then measures seven samples of
one million comparisons. Results go to `build/strverscmp-shootout-matrix.csv`;
the available 2026-10-22 reference pairs are in
`benchmarks/results/2026-10-22-strverscmp-shootout.csv`.
`make strtok-shootout-matrix` compares host and Nelua `strtok` across
Zig/glibc, GCC/glibc, Zig/musl, and TinyCC/glibc. Nelibc's `strtok_r` is also
measured as a control for the thread-local state lookup. Seven-sample medians
are recorded in `benchmarks/results/2026-10-22-strtok-shootout.csv`; TinyCC's
Nelua path remains slower in this run, while the other three builds are faster.
The initial measurements are preserved in
`benchmarks/results/2026-10-22-strtok-shootout-initial.csv`. The TinyCC follow-up
inlines token advancement into `strtok`, reducing its median from 21.89 to
20.27 ns per token; TinyCC still trails host `strtok` at 13.41 ns.
`make sysconf-shootout-matrix` measures 40 fixed and dynamic `sysconf` selectors
against host libc under Zig/glibc, GCC/glibc, Zig/musl, and TinyCC/glibc. Each
query reports its returned value and the median of seven samples; dynamic-limit
selectors use 10,000 calls per sample, and fixed selectors use 250,000. The
available 2026-10-22 reference pairs are in
`benchmarks/results/2026-10-22-sysconf-shootout.csv`.
`make confstr-shootout-matrix` validates POSIX `confstr` length, truncation,
NUL termination, and invalid-selector behavior before timing `_CS_PATH` under
Zig/glibc, GCC/glibc, Zig/musl, and TinyCC/glibc. `_CS_PATH` is
implementation-defined: this host's glibc reports `/usr/bin`, while musl and
Nelibc report `/bin:/usr/bin`. The saved 2026-10-22 run is
`benchmarks/results/2026-10-22-confstr-shootout.csv`.
`make pathconf-shootout-matrix` checks all 21 Linux `_PC_*` selectors through
`pathconf` and `fpathconf`, validates invalid descriptors and regular-file
async-I/O behavior, and measures all four compiler/libc combinations on `.`,
`/proc`, `/dev/shm`, and `/boot`. These mounts provide Btrfs, procfs, tmpfs,
and ext4 coverage here. Set `PATHCONF_PATHS` to probe other mounted paths.
The parity gate checks shared values; implementation-specific values remain in
the CSV. Glibc reports `LINK_MAX=65535` on Btrfs and `127` on procfs/tmpfs;
it reports `65000` on ext4. Musl reports `8` on all four. `FILESIZEBITS` is
`255` for glibc on Btrfs, `32` on procfs/tmpfs, and `64` on ext4; musl reports
`64` on all four. Glibc leaves `SYNC_IO` and the
transfer increment/maximum indeterminate; musl reports `SYNC_IO=1` and `4096`
for those transfer values. Musl also leaves async I/O indeterminate for a
regular file, while glibc and Nelibc report it supported. For an invalid
descriptor, musl returns `NAME_MAX` without setting `errno`; Nelibc returns
`-1` with `EBADF`. Results are saved in
`benchmarks/results/2026-10-22-pathconf-shootout.csv`.
Each matrix leg also compares 3,670,016 cases against a separate host-libc
executable in the C locale: all 256-by-256 byte pairs over lowercase,
uppercase, alternating, mixed-word, punctuation, high-byte, and combined
punctuation/high-byte prefixes at offsets 0, 1, 7, 15, 16, 31, 63, and 127.
NUL is included in the byte pairs, so each offset checks equal terminators and
either side terminating before the other. The sweep checks full-string
comparison and bounds before, at, and after the injected byte.
Each leg also compares 50,000 deterministic randomized cases with lengths up
to 1,280 bytes, equal strings, byte mismatches, early termination, and bounds
around each mismatch or terminator, plus 10,000 cases with both strings ending
near protected page edges and 10,000 identical-pointer and 10,000 overlapping-
pointer cases over shared buffers. It also compares 10,000 identical-pointer
and 10,000 distinct overlapping-pointer cases at protected page edges. Half
of the overlapping page-edge cases differ at byte zero; the others compare
case-fold-equal first bytes and differ at byte one. Both pointers share the
same backing buffer and terminating NUL. The page-edge cases place each terminator from
the final accessible byte to 16 bytes before the guard, with independent
offsets for each string. They also check bounded comparisons just before, at,
and just after each string's terminator. In unequal-length page-edge cases,
the shorter string's NUL is at the last accessible byte while the longer
string has a nonzero byte there; both comparison bytes are included in the
differential output.
The x86-64 candidate uses branchless ASCII-fold first-byte checks followed by
page-boundary-checked SSE2 blocks; the benchmark also tests strings ending at
protected page edges. Its current shootout is recorded in
`benchmarks/results/2026-10-21-strcasecmp-sse2-exhaustive-shootout.csv`.

## Milestones

1. **Host-linked ABI slice:** export a small libc function from Nelua and link
   it into a C program on glibc, musl, and another available libc.
2. **Freestanding core:** implement memory and string primitives, with a
   syscall boundary that does not depend on libc.
3. **Runtime replacement:** allocation, process startup/exit, errno, and I/O.
4. **Self-hosted libc:** rebuild and run the same conformance suite with
   nelibc replacing the host libc.

## Current slice

`src/string.nelua` implements `strlen`, `strnlen`, `strcmp`, GNU `strverscmp`, POSIX `strcasecmp` and `strncasecmp`, locale-aware `strcoll` and `strxfrm`, `strncmp`,
`strdup`, `strndup`, `strcpy`, BSD `strlcpy` and `strlcat`, POSIX `stpcpy`, `strcat`,
`strncat`, `strncpy`,
`stpncpy` (GNU/POSIX), `strchr`, GNU `strchrnul`, `strrchr`, `strstr`, GNU `strcasestr`,
`strspn`, `strcspn`, `strpbrk`, `strtok`, `strtok_r`, BSD/GNU `strsep`, Linux
`strerror` strings for errno values 0 through 134 where Linux defines a value,
and the POSIX integer-returning `strerror_r` interface with caller-buffer and
range checking;
case-insensitive comparisons currently fold ASCII letters, matching the POSIX C locale.
`src/memory.nelua` implements `memcpy`, `memmove`, POSIX `memccpy`, `memset`,
`explicit_bzero` with volatile stores, `memcmp`, `memchr`, wide-character memory
and string functions, and the GNU/musl `memrchr` and `memmem` extensions.
For `memmove`, disjoint ranges now take Nelibc's `memcpy` path while overlapping
ranges retain forward or backward copying. The two repeated memory shootouts
show Zig/glibc 64-byte disjoint-after calls dropping from 12.11 ns to 2.74–2.95
ns; Zig/musl drops from 11.79 ns to 2.91 ns. The four-toolchain shootouts use
`rep movsb` for `memcpy` ranges of 256 bytes or more; TinyCC exports the public symbol directly
from the assembly helper to avoid its generated wrapper. TinyCC uses four
unaligned SSE2 moves for exactly 64 bytes and `rep movsb` otherwise. Three
TinyCC-driver shootout repeats measure 2.47–2.51 ns at 64 bytes, versus
4.21–4.30 ns for its direct `rep movsb` baseline; host libc measures
2.27–2.30 ns. The SSE2 path is substantially faster than Nelibc's old path,
though host libc remains 8–10% faster. `make shootout-tcc-memcpy-variants`
repeats this comparison. Zig/musl 4 KiB calls
improved from about 50 ns to 19.22 ns; GCC's 4 KiB calls improved from
58.79–59.32 ns to 18.33–19.25 ns. TinyCC's direct entry measures
17.26–18.73 ns at 4 KiB, near host libc.
TinyCC's `memset` uses SSE2 through 256 bytes and `rep stosb` for larger
ranges; repeated 64-byte fills improved from about 10 ns to 4.5 ns.
BSD `bcopy` and `bzero` wrappers reuse the overlap-safe move and byte-fill
implementations; `bcmp` reuses Nelua `memcmp`. `<strings.h>` declares these
alongside `ffs` variants and the POSIX `index`/`rindex` aliases.
`memmem` uses a Two-Way search with a bad-character shift table for needles
of 8 bytes or longer; differential coverage includes periodic and long needles
through 512 bytes.
On x86-64, `strlen` uses aligned SSE2 scans, switching to page-safe 256-byte
blocks for long strings, with a scalar fallback for other architectures.
`strstr` uses bounded `memchr` candidates for short or rare
first bytes, and Two-Way `memmem` when longer needles have frequent candidates.
`include/string.h` declares the C ABI, and
`tools/abi_smoke.c` exercises the exported functions through linked C calls.
`src/unistd.nelua` also implements Linux directory streams with `getdents64`:
`opendir`, `fdopendir`, `readdir`, `closedir`, `dirfd`, `rewinddir`, `telldir`,
`seekdir`, `scandir`, GNU `scandirat`, locale-aware `alphasort`, and GNU
`versionsort`. Both scan functions support optional filtering and sorting
callbacks and return separately allocated entry copies. The hybrid and standalone
`include/dirent.h` declarations are checked alongside runtime smoke across
Zig/glibc, GCC/glibc, Zig/musl,
TinyCC/glibc, and both standalone compiler builds.
Hybrid `strcoll` and `strxfrm` are implemented in `src/string_locale.nelua`.
They call the host's locale-specific entry points with the current thread
locale, so host `setlocale`/`uselocale` changes are honored. Non-C global
locales use an eight-entry process-lifetime cache; long names or a full cache
use temporary locale objects. `src/string_locale_lock.c` supplies only the
portable pthread mutex used to protect that cache. `make collation-shootout-matrix` measures host and Nelibc
`strcoll`/`strxfrm` in global and thread-specific locale modes; set
`COLLATION_LOCALE=de_DE.utf8` to select another installed locale.
Standalone Nelibc still uses C-locale unsigned-byte ordering. Its `setlocale`
shim accepts `C`/`POSIX` for every locale category, `C.UTF-8` for `LC_CTYPE`,
and `de_DE.UTF-8` (or `de_DE`) for `LC_NUMERIC` and `LC_MONETARY`. `localeconv`, `strtod`, and
floating `printf` conversions use the selected numeric radix. The POSIX
apostrophe flag groups integer conversions and fixed-form floating conversions
using the locale's grouping pattern and separator.
Mixed `LC_ALL` state is returned in a restorable composite form. German
monetary data includes the euro symbol, two fractional digits, and German
placement/sign conventions. Other monetary locales, non-C collation, and
date/time locale data are not part of the standalone
runtime. `make locale-format-diff-matrix` compares German grouped integer and
floating formatting, including 4,096 deterministic `%f`, `%e`, and `%g` samples
plus 512 high-precision `%g` samples, subnormal, exponent-carry, `DBL_MAX`, and
streamed `%g` precision-100000 cases, with host libc for Zig, GCC, and TinyCC builds.
`tools/collation_diff.c` checks `strcoll` and `strxfrm` against the host's
explicit-locale `strcoll_l` and `strxfrm_l` references over 256 string pairs.
`make collation-diff-matrix` runs the C-locale checks with Zig/glibc,
GCC/glibc, Zig/musl, and TinyCC/glibc; pass a locale name to a built checker
to probe other collations where that libc provides locale data.
`src/ctype.nelua` provides C-locale ASCII classification and case conversion;
`include/ctype.h` declares the functions, including the common `isascii` and
`toascii` extensions. Character values outside EOF and the unsigned-char range
are outside the C standard contract.
`src/stdlib.nelua` adds `atof` as a `strtod` wrapper; signed and unsigned
integer parsing (`strtol`/`strtoll`,
`strtoul`/`strtoull`), including glibc C23 entry points that accept `0b`/
`0B` prefixes while legacy entry points keep the pre-C23 parsing rule, and
the corresponding wide-character parsers (`wcstol`, `wcstoll`, `wcstoul`,
`wcstoull`) with matching C23 aliases,
decimal convenience conversions, integer absolute-value functions, `getenv`,
`setenv`, `unsetenv`, `putenv`, `realpath`, and `system`. `system` runs
`/bin/sh -c` with the current environment and returns the child wait status;
it blocks `SIGCHLD`, temporarily ignores `SIGINT` and `SIGQUIT` in the parent,
restores the previous mask and dispositions, and restores them in the child
before `execve`. Concurrent calls serialize signal changes with a pthread
mutex in hybrid builds and an atomic futex lock in standalone builds. Hybrid
cancellation cleanup kills and reaps the shell child, restores signal state,
and releases the mutex. The
environment functions scan the process `environ` vector, and `realpath`
resolves paths through Nelibc syscalls.
The Nelua `reallocarray` wrapper detects size multiplication overflow, then
uses the build's matching `realloc`: host allocation in hybrid mode and
Nelibc allocation in standalone mode.
`getopt` parses POSIX short-option clusters, attached or separate required
arguments, GNU optional arguments, and the `--` terminator. GNU ordering
permutes non-options by default; `POSIXLY_CORRECT` or a leading `+` stops at
the first non-option, and a leading `-` returns non-options as code 1. A
leading colon selects the alternate missing-argument result. `getopt_long`
supports exact and unambiguous abbreviated long names, flag options, and
required or optional arguments. `getopt_long_only` also accepts single-dash
long names and falls back to short-option parsing when needed.
`getsubopt` splits a writable comma-separated option list in place and returns
the matching token index and optional value. `mblen`, `mbtowc`, `mbrtowc`,
`mbrlen`, `mbsinit`, `btowc`, `wctob`,
`wctomb`, `wcrtomb`, `mbstowcs`, `wcstombs`, `mbsrtowcs`, `mbsnrtowcs`,
`wcsrtombs`, and `wcsnrtombs` use Nelua's C-locale and UTF-8 codecs.
Restartable conversions preserve `mbstate_t` and support bounded input and
output. Hybrid builds fall back to host conversion primitives for other locale
encodings. Standalone mode supports C/POSIX in all locale categories, UTF-8
`LC_CTYPE`, and German numeric and monetary data through `setlocale`; non-C
collation and date/time locale data remain unavailable.
`make multibyte-diff-matrix` compares these conversions with the host libc in C
and C.UTF-8, plus ISO-8859-1 and state-preserving EUC-JP conversions when the
host exposes those codesets. Cases include incomplete and invalid sequences
and output limits. The matrix uses Zig/glibc, GCC/glibc, Zig/musl, and
TinyCC/glibc.
`setenv` validates names, honors `overwrite`, and reports `EINVAL` or `ENOMEM`;
`unsetenv` removes every matching entry and succeeds when none exists.
`putenv` stores the caller's string directly, so later edits remain visible;
a string without `=` removes that name. A small C ABI adapter reads and updates
the vector, and standalone startup initializes it from the initial process
stack.
Parsing handles end pointers, invalid bases, unsigned negation, and range errors;
decimal, octal, and hexadecimal digit loops use radix-specific fast paths.
`div`, `ldiv`, and `lldiv` return the standard quotient/remainder records with
C truncation-toward-zero semantics, including negative operands.
POSIX `ffs` and GNU `ffsl`/`ffsll` return one-based least-significant-set-bit
positions, with zero mapping to zero.
`qsort` uses insertion sort for arrays of up to 16 elements, two-way
introsort through 4,096 elements in Zig, musl, and TinyCC builds. GCC builds
use introsort for larger arrays too. Other builds switch to heapsort above
4,096 elements. All builds use heapsort for elements over 64 bytes, and
introsort falls back to heapsort when its depth limit is reached.
`bsearch` uses binary search; both accept the standard C comparator callback.
GNU `qsort_r` accepts caller-owned comparator context and uses Nelua insertion
sort for small elements plus an in-place heapsort for larger arrays. The smoke
target checks independent ascending/descending contexts and both sorting paths.
`make qsort-diff-matrix` compares their output on 256 deterministic random
arrays plus sorted, reverse-sorted, all-equal, organ-pipe, and alternating-key
patterns up to 4,096 items. Cases cover algorithm boundaries, ascending and
descending comparators, and present and absent searches under Zig/glibc,
GCC/glibc, Zig/musl, and TinyCC/glibc.
The Nelibc runs also assert that organ-pipe 1,024-item cases reach introsort's
heapsort depth fallback. The assertion
uses a diagnostic-only object; regular libc objects localize the counter
symbol and do not export it.
`make qsort-shootout-matrix` measures seven-sample medians for sorting 16,
256, 1,024, 2,048, and 4,096 integers and searching a sorted 4,096-integer
array. It is also included in `make shootout-matrix`; the current results are archived at
`benchmarks/results/2026-10-22-qsort-shootout.csv`.
`rand` and `srand` provide repeatable per-thread xorshift sequences; POSIX
`rand_r` advances caller-owned state with Zig's SplitMix64 sequence used by
the bundled musl-target libc shim.
`include/stdlib.h` adds the Nelibc floating conversion, integer, random, sort, and search
declarations before forwarding to the host `<stdlib.h>`, preserving host APIs
and feature-mode macros while allowing these Nelua exports to replace them.
`src/stdlib_float.nelua` adds `strtod`, `strtof`, and `strtold` for decimal and
hexadecimal forms, including `inf`, `nan`, end pointers, and range errors.
All three functions use exact rational conversion with ties-to-even rounding
when the rational fits the fixed 2,048-limb workspace. Parsing retains up to
1,400 limbs (about 13,500 decimal digits) and tracks a sticky tail for longer
inputs; the differential suite includes long values just above binary rounding
midpoints.
Correct rounding is not yet guaranteed for every over-capacity input. The final
`strtold` path retains at most 20 significant decimal digits or 16 significant
hexadecimal digits.
The `musl-gcc` and TinyCC targets are available when those tools are installed.

`src/sys/linux_x86_64.nelua` adds raw three- and six-argument Linux syscall
boundaries;
`src/sys/linux_x86_64.S` adapts the C ABI to the x86_64 kernel register ABI.
`src/unistd.nelua` exports POSIX `read`, `write`, `pread`, `pwrite`, `preadv`, `pwritev`, `preadv2`, `pwritev2`, `readlink`, `close`, Linux `close_range`, `eventfd` and its GNU read/write helpers, `pipe`, and Linux `pipe2`,
`isatty` via Linux `TCGETS`, and variadic `ioctl` via the Linux syscall ABI,
`dup`, `dup2`, Linux-specific `dup3`, and `fcntl` (with its variadic C ABI
adapter in `src/fcntl_adapter.c`),
`readv`, `writev`,
`sendfile64` through the same x86_64 syscall implementation as `sendfile`,
`openat64` through the same x86_64 syscall implementation as `openat`,
and x86_64 large-file aliases for `open`, `pread`/`preadv`, `pwrite`/`pwritev`,
`lseek`, `ftruncate`, and `mmap`,
which share `off_t`'s 64-bit ABI on x86-64. `include/fcntl.h` and
`include/nelibc_syscall.h` declare the `*64` names under `_GNU_SOURCE` or
`_LARGEFILE64_SOURCE`; the consumer matrix exercises the aliases directly in
all four compiler/libc profiles,
`poll`,
`select`,
`nanosleep`, POSIX `clock_nanosleep`, `clock_gettime`, `clock_getres`, C11
`timespec_get`, C23 `timespec_getres`, Linux `getrandom`, and POSIX `getentropy`,
`sleep`, `alarm`, and `pause`,
process wait calls `wait`, `waitpid`, and `wait4`,
process creation calls `fork`, `execve`, Linux `execveat`, and `_exit`,
socket creation and communication calls `socket`, `socketpair`, `bind`, `connect`, `listen`, `accept`, `accept4`, `send`, `recv`, `sendto`, `recvfrom`, `sendmsg`, `recvmsg`, `shutdown`, `getsockname`, `getpeername`, `setsockopt`, and `getsockopt`,
`clock`, `difftime`, UTC `gmtime` and `gmtime_r`, TZif transition-table
`localtime` and `localtime_r` with POSIX footer recurrence and POSIX `TZ` rules,
`tzset` with `timezone`, `daylight`, and `tzname`,
`mktime`, GNU `timegm`, C-locale `strptime` (including `%F`, `%T`, `%c`, `%U`,
`%W`, `%G`, `%g`, `%V`, `%z`, `%Z`, `%C`, and GNU `%s`)
and `strftime`, `asctime`/`asctime_r`,
`ctime`/`ctime_r`, plus
`gettimeofday` and `time`,
`mmap`, variadic `mremap` (with its optional fixed destination adapter), `mprotect`, `munmap`, and `madvise`,
`getrlimit` and `setrlimit`,
`getpriority` and `setpriority`,
Linux scheduling calls `sched_yield`, `sched_getaffinity`, and `sched_setaffinity`,
`getrusage`, and `times`,
`umask`,
`uname`,
`lseek`, `ftruncate`, `truncate`, `fsync`, `fdatasync`, `sync`, `syncfs`, `openat`, `creat`, `unlinkat`, `mkdirat`, `renameat`, `linkat`, `chmod`, `fchmod`, `fchmodat`, `access`, `faccessat`, Linux `faccessat2`, `fstatat`, `fstat`, `stat`, `lstat`, `symlinkat`,
Linux `fallocate` and POSIX `posix_fallocate`,
POSIX `posix_fadvise` and Linux `readahead`,
Linux `sendfile`,
Linux `copy_file_range`,
Linux `splice`,
Linux `tee`,
Linux `vmsplice`,
Linux `setxattr`, `getxattr`, `listxattr`, `removexattr`, `lsetxattr`, `lgetxattr`, `llistxattr`, `lremovexattr`, `fsetxattr`, `fgetxattr`, `flistxattr`, and `fremovexattr`,
Linux 6.17+ `setxattrat`, `getxattrat`, `listxattrat`, and `removexattrat`,
Linux 6.17+ `file_getattr` and `file_setattr`,
Linux 6.15+ `open_tree_attr`,
Linux `mount_setattr`,
Linux `mount`, `umount`, and `umount2`,
Linux `pivot_root`,
Linux `unshare` and `setns`,
Linux `sethostname` and `setdomainname`,
POSIX `gethostname` and Linux `getdomainname`,
Linux `open_tree`,
Linux `move_mount`,
Linux `fsmount`,
Linux `fsopen` and `fsconfig`,
Linux `fspick`,
Linux 6.8+ `statmount` and `listmount`,
`readlinkat`, `symlink`, `link`, `unlink`, `rmdir`, `mkdir`, `mknod`, `mknodat`, `mkfifo`, `mkfifoat`, `rename`, `chown`, `lchown`, `fchownat`, `getcwd`, BSD/GNU `getpagesize`, POSIX `pathconf` and `fpathconf` for all 21 Linux `_PC_*` selectors, POSIX `confstr` for `_CS_PATH`, and 40 `sysconf` selectors covering resource limits, physical and available memory pages, processor counts, POSIX feature flags, thread limits, X/Open values, realtime signals, message queues, semaphores, and filesystem/network limits; `chdir`, `fchdir`, `getuid`, `geteuid`, `getgid`,
`fchown`, `futimens` via `utimensat(AT_EMPTY_PATH)`, and `utimensat`,
`getegid`, `getpid`, `getppid`, `getpgid`, `getpgrp`, `setpgid`, `getsid`, `setsid`, `kill`,
`getresuid`, `setresuid`, `setreuid`, `getresgid`, `setresgid`, `setregid`,
`getgroups`, Linux-specific `setgroups`,
`raise`, `sigprocmask`, `sigpending`, `sigismember`, `sigemptyset`, `sigfillset`,
`sigaddset`, `sigdelset`,
and Linux-specific `gettid`,
translating raw kernel errors into
`-1` plus the host libc's thread-local `errno`. Its remaining
bootstrap imports are `__errno_location` and the assembly syscall shim.
`include/sys/mman.h` exposes the implemented mapping calls through their
standard header. Hybrid builds inherit the host declarations; standalone builds
provide Linux protection, mapping, huge-page, sync, lock, remapping, and advice
constants. The
`unistd-header-smoke` compile matrix checks those constants and public function
signatures for Zig/glibc, Zig/musl, GCC/glibc, TinyCC/glibc, and standalone
Zig/GCC profiles.
`include/sys/wait.h` similarly exposes the implemented `wait`, `waitpid`, and
Linux `wait4` functions, along with Linux wait options and process-status
macros in standalone builds. The same header smoke checks its declarations and
status macros in the available hybrid and standalone profiles.
`include/fcntl.h` uses Linux UAPI constants and Nelibc's own file-call
declarations in standalone mode; hybrid builds forward to the selected libc.
The header smoke checks the `fcntl` and `openat` signatures.
`include/poll.h` does the same for `poll`, preserving the target's `pollfd`,
`nfds_t`, and event constants; the header smoke checks its signature and
`POLLIN`/`POLLOUT` values.
`include/sys/socket.h` defines Linux socket address, message, and control
header layouts plus the core domain, type, and message flags in standalone
mode; socket-option values come from Linux UAPI headers. Hybrid builds retain
the target socket ABI. Header and syscall smokes check layouts, signatures,
socket pairs, local stream sockets, and message send/receive.
`include/sys/time.h` preserves the target's `timeval` ABI and adds Nelibc's
`gettimeofday` declaration. Standalone mode supplies `timeval`, `timezone`, and
the timer arithmetic macros without forwarding to a host `sys/time.h`; the
header smoke checks their x86-64 layouts and the `gettimeofday` signature.
`include/sys/types.h` now owns the Linux x86-64 scalar types needed by
standalone syscall and system headers; hybrid builds continue to forward to the
selected libc's `<sys/types.h>`.
`include/time.h`, `include/signal.h`, `include/stdlib.h`, and `include/locale.h`
provide standalone time, signal, standard-library, and locale declarations
without forwarding to glibc; hybrid builds retain the target headers. The
header smoke checks their public layouts and constants.
`include/sched.h` defines the Linux CPU affinity set and the scheduler calls
Nelibc implements. Standalone `<sys/sendfile.h>`, `<sys/xattr.h>`, and
`<sys/mount.h>` use Linux UAPI layouts and constants rather than libc-private
headers.
`include/sys/resource.h` defines Linux x86-64 `rlimit` and `rusage` structures,
resource selectors, and Nelibc's implemented resource-call declarations in
standalone mode. Hybrid builds forward to the target header; the header smoke
checks resource constants, structure layouts, and call signatures.
`include/sys/times.h` preserves `struct tms` and `clock_t` from the target
headers and exposes Nelibc's `times` function; the header smoke checks the
x86-64 structure layout.
`include/sys/select.h` defines the Linux x86-64 1024-descriptor `fd_set` ABI
and its `FD_*` macros in standalone mode, and exposes the Nelibc `select`
implementation. The header smoke checks the descriptor-set size and signature;
syscall smokes exercise setting, clearing, and querying descriptors.
`include/sys/uio.h` preserves the target `iovec` layout and exposes the
vectored I/O calls; the header smoke checks the layout and `readv`, `writev`,
`preadv`, and `pwritev` signatures.
`include/errno.h` forwards the host error constants during bootstrap; in
standalone mode it uses Nelibc's Linux error-number definitions and maps
`errno` through Nelibc's own TLS accessor.
`src/credential_adapter.c` delegates hybrid UID/GID setter calls to the next
host-libc implementation when `dlsym(RTLD_NEXT)` is available, preserving the
host's thread-wide credential synchronization. The Zig static musl profile uses
musl's internal `__setxid` and `__synccall` coordination routines. This profile
is tied to those musl internals. Standalone uses direct syscalls and synchronizes
credentials across the active parent and child threads. Its pthread subset is
documented with the standalone thread runtime below.
`include/unistd.h` forwards to the host header and adds Nelibc's syscall API in
hybrid builds. Standalone mode skips the host `unistd.h` and exposes Nelibc's
implemented calls, POSIX `getopt` globals, and the standard descriptor and
access-mode constants.
`include/sys/stat.h` uses the target header for hybrid builds. Standalone builds
define the Linux x86-64 `struct stat` and `statx` UAPI layouts and mode macros
without including the host `sys/stat.h`; the header smoke checks their sizes,
offsets, and call signatures. The `fcntl.h` standalone wrapper loads this ABI
first to prevent glibc's internal fcntl headers from supplying another stat
layout. Standalone Zig/GCC syscall smokes run metadata operations against the
project layouts with no undefined symbols.
`include/sys/utsname.h` similarly defines Linux's six-field `struct utsname`
for standalone mode. Header and runtime syscall smokes check its layout and
`uname` result.
`include/sys/un.h` defines Linux's `sockaddr_un` and `UNIX_PATH_MAX` for
standalone mode. The header smoke checks its size and path offset, and the
standalone syscall smoke exercises socket-pair and local stream operations.
`include/sys/random.h` exposes Nelibc's `getrandom` and `getentropy` calls and
the Linux `GRND_*` flags in standalone mode. The header matrix checks their
declarations and flag values; standalone syscall smokes exercise blocking,
nonblocking, invalid-flag, and `getentropy` limit cases.
`include/sys/ioctl.h` exposes the Linux ioctl request encodings from the
compiler sysroot in standalone mode and adds Nelibc's variadic syscall
declaration. Header and runtime smokes check `FIONREAD` on a pipe.
`include/poll.h` defines Linux's `pollfd`, `nfds_t`, and poll event values in
standalone mode; `<sys/poll.h>` forwards to that same ABI. The header smoke
checks the structure layout and the syscall smokes exercise `poll` on pipes.
`include/sys/uio.h` defines Linux's `iovec` layout in standalone mode and
exposes Nelibc's vectored-I/O calls; the header smoke checks the layout and
signatures.
`include/sys/times.h` defines Linux's four-clock `struct tms` layout in
standalone mode; the header and syscall smokes check its fields and `times`.
`include/wchar.h` declares the wide-character APIs implemented by Nelibc and
provides standalone `wchar_t`-based state types. GNU/C23 modes also declare
the four `__isoc23_wcsto*` conversion entry points; the consumer matrix checks
their `0b`/`0B` prefix parsing across all hybrid profiles.
The Linux-specific `pipe2` wrapper passes its flags to the kernel during pipe
creation, so `O_CLOEXEC` and `O_NONBLOCK` are applied atomically with descriptor
creation. `close_range` supports the Linux `CLOSE_RANGE_UNSHARE` and
`CLOSE_RANGE_CLOEXEC` flags and is available on kernels starting with 5.9.
`<sys/eventfd.h>` exposes `eventfd` flags and declarations in hybrid and
standalone builds; Nelua implements the counter descriptor and the eight-byte
`eventfd_read`/`eventfd_write` helpers.
`<sys/epoll.h>` supplies the Linux x86_64 packed event ABI and Nelua implements
`epoll_create1`, `epoll_ctl`, `epoll_wait`, and signal-mask-aware
`epoll_pwait`, plus nanosecond-timeout `epoll_pwait2`. The syscall smoke
registers an eventfd, waits for readability, and checks the returned data token
through the wait entry points.
`<sys/pidfd.h>` declares `pidfd_open` and `pidfd_send_signal`, backed by the
Linux x86_64 syscalls. The syscall smoke checks the close-on-exec flag,
signal-zero validation, and bad-descriptor `errno` handling.
`<sys/mman.h>` also declares Linux `memfd_create`; Nelua creates the anonymous
file descriptor with the x86_64 syscall, including `MFD_CLOEXEC` and
`MFD_ALLOW_SEALING` support. The syscall smoke verifies close-on-exec and a
write/read round trip in glibc, musl, and standalone builds.
`src/signal_adapter.c` exports `sigaction`, translating glibc and musl action
structures to the Linux x86_64 kernel layout; the assembly syscall shim
provides the signal-return trampoline required by the kernel.
`getentropy` uses the raw `getrandom` syscall without entering a cancellation
point, retries `EINTR` and short reads, and rejects requests over 256 bytes
with `EIO`. The syscall smoke checks full 256-byte reads, the size limit, and
bad-buffer errors across the hybrid and standalone builds. The syscall smoke uses a protected mapping to verify partial `getrandom`
returns and `tools/syscall_interpose_smoke.c` injects `EINTR` to check errno
translation.
`nelibc_linux_write` exposes raw results for low-level callers. Run `make smoke-syscall-zig`,
`make smoke-syscall-gcc`, or `make smoke-syscall-zig-musl`. This layer is Linux
x86_64-specific; the smoke executable still uses host C startup code.

`make smoke-freestanding-zig` and `make smoke-freestanding-gcc` link the string
and memory objects with a custom `_start`, `-nostdlib`, and no undefined
symbols. These targets cover the freestanding string and memory core. The GCC target disables loop-to-memory-call
recognition so the compiler cannot turn nelibc's own `memset` loop into a call
to itself.

`make smoke-standalone-zig`, `make smoke-standalone-gcc`, and
`make smoke-standalone-tcc` link the syscall, allocator, string, memory, and
standalone errno objects into a static `-nostdlib` executable. TinyCC does not
emit thread-relative references for Nelua TLS globals, so the standalone
TinyCC profile builds its errno and TLS fixture objects with Zig cc. `_start`
reads the executable's `PT_TLS` segment from
the initial stack's auxiliary vector, maps storage for the full TLS image,
copies initialized bytes, zeros the remainder, and sets FS with
`ARCH_SET_FS` before `main`. The initial thread gets a minimal self-pointer
TCB; dynamic TLS modules and a DTV are not implemented.
`tools/tls_fixture.nelua` makes the standalone smoke exercise both initialized
`.tdata` and zero-filled `.tbss`; `tools/tls_alignment_fixture.c` checks a
64-byte TLS alignment. Startup rounds the TLS image to `p_align` when matching
the linker-generated local-exec offsets. `src/thread.nelua` adds an
experimental `nelibc_thread_create`/`nelibc_thread_join` API: it uses Linux
`clone`, gives the child a copied static TLS image and separate stack, and
joins through `child_tid` plus futex. Join writes the callback result through
an output pointer and returns `0` or `-1`; `nelibc_thread_run_once` remains as
a synchronous wrapper that returns the callback result. The smoke verifies
the parent can do work after create returns. It also checks that errno and
Nelua TLS changes in the child do not affect the parent. Multiple child
handles may run concurrently; joins use timed futex waits to service serialized
process-wide credential requests from any child.
Standalone also exports a POSIX thread subset: `pthread_create`,
`pthread_join`, `pthread_detach`, `pthread_exit`, `pthread_once`,
`pthread_key_create`, `pthread_key_delete`, `pthread_getspecific`,
`pthread_setspecific`, `pthread_self`, `pthread_equal`, `pthread_sigmask`,
`pthread_kill`, `pthread_atfork`, `pthread_getcpuclockid`,
detach-state, stack-size,
caller-provided stack, guard-size, scope, and scheduling attributes,
normal/recursive/error-checking, robust, and priority-inheritance mutexes,
recursive-reader read-write locks,
condition variables with realtime or monotonic timed-wait clocks, reusable
barriers, and detached workers. Read-write locks support timed operations and
process-shared futexes. Readers
may pass queued writers, so a continuous reader stream can starve writers.
Priority-inheritance mutexes use Linux PI futex operations; priority-protect
mutexes are unsupported. Thread-specific key values use per-thread storage, key
generations prevent deleted keys from aliasing reused slots, and worker exit runs
up to four destructor passes before releasing that storage.
Detached workers release their stack and TLS at exit; process-wide credential
changes from a detached worker return `ENOTSUP` because no parent thread
services the rendezvous after detachment.
Deferred cancellation is supported through `pthread_cancel`,
`pthread_setcancelstate`, and `pthread_testcancel`. Condition waits are
cancellation points: a request wakes the target futex, reacquires the mutex,
then runs cleanup handlers before thread-specific data destructors. Applications
can use `pthread_cleanup_push`/`pthread_cleanup_pop` on created threads.
Blocking calls routed through Nelibc's cancellable syscall wrappers (including
read/write, socket I/O, poll/select, waits, sleeps, openat, and fsync) are
interrupted for enabled cancellation and unwind cleanup handlers. Ordinary
CPU-bound code must call `pthread_testcancel`. Linux signal 32 is reserved for
this mechanism; Nelibc rejects attempts to install or block it.
`pthread_setcanceltype` and asynchronous cancellation are supported. Scheduler
attributes cover inherited or explicit `SCHED_OTHER`, `SCHED_FIFO`, and
`SCHED_RR`; explicit permission failures are reported by `pthread_create`
before the worker routine starts. Caller-provided stack memory remains owned
by the application and must outlive the thread; Nelibc does not add a guard
page to that memory. Cleanup handlers on the initial thread are also supported.
Robust mutexes recover owner death through Linux robust futex lists for both
private and process-shared locks, including a forked child; callers must call
`pthread_mutex_consistent` before unlocking recovered state. Each
created thread has room for 32 pending cleanup handlers.
Standalone unnamed POSIX semaphores (`sem_init`, `sem_destroy`, `sem_wait`,
`sem_timedwait`, `sem_trywait`, `sem_post`, and `sem_getvalue`) are implemented
with futex-backed waits; blocked `sem_wait` calls are deferred-cancellation
points. `sem_timedwait` is also a cancellation point. Named semaphores use
`/dev/shm` backing files and support `sem_open`, `sem_close`, and `sem_unlink`.
Semaphore waits allow immediate acquisition even with an invalid absolute
timeout, and named opens ignore creation attributes for an existing name.
`sem_clockwait` supports absolute realtime and monotonic deadlines.
Hybrid glibc archives fast-path `sem_trywait` before delegating blocking waits
to glibc, preserving POSIX's immediate-acquisition rule for invalid deadlines.
Those consumers should link with `-pthread -ldl`. The hybrid musl archive supplies the
API on musl versions without it; its monotonic implementation uses 1 ms polling
through musl's public semaphore functions, avoiding assumptions about musl's
private `sem_t` layout. The musl smoke checks cancellation while blocked.
`nm -u` must be empty.
The allocator exports only `nelibc_*` names in hybrid builds. The standalone-only wrappers
in `src/alloc/standalone_exports.nelua` provide standard `malloc`, `calloc`,
`realloc`, `reallocarray`, `free`, C11 `aligned_alloc`, and POSIX `posix_memalign`; the C smoke
calls them with no host libc linked.

The standalone-only `src/stdlib_exit_standalone.nelua` implements `atexit` and
`exit`. The shared `src/unistd.nelua` exports `_exit` and `_Exit` through the
Linux `exit_group` syscall so termination includes every thread. `exit` invokes
up to 32 registered handlers in reverse registration order, flushes stdio, then
uses `_exit`; standalone startup routes the return value from `main` through
`exit`. Hybrid builds keep the host `exit` runtime. Run
`make exit-smoke-standalone-matrix` to check explicit
exit, return from `main`, `_Exit`, handler order, and registry capacity with
Zig cc and GCC. The shared `abort` wrapper raises `SIGABRT` and calls `_Exit`
if the signal handler returns. `make abort-smoke-matrix` checks `wait`,
`wait4` with resource usage, and `waitpid` status handling through
`<sys/wait.h>`, including normal child exit and `SIGABRT`, with Zig/glibc,
GCC/glibc, Zig/musl, TinyCC/glibc, and standalone Zig/GCC builds. Standalone
links also check that the executable has no undefined symbols.

`src/alloc.nelua` provides `nelibc_malloc`, `nelibc_calloc`, `nelibc_realloc`,
`nelibc_free`, `nelibc_reallocarray`, `nelibc_aligned_alloc`, and
`nelibc_posix_memalign`. Requests whose header, payload, and alignment slack fit
use synchronized mmap slabs with 64-byte through 16384-byte size classes;
larger or over-aligned requests use their own anonymous mapping. A header
pointer immediately before each user pointer lets `free` and `realloc`
distinguish pooled blocks from direct maps. The prefixed API keeps allocations
separate from the host allocator during hybrid builds. Hybrid profiles enable the thread-local cursor fast path. During first-use
pthread key setup, a reentrancy guard sends nested allocations to shared pools
until cleanup registration completes; this avoids recursive key setup while
public `malloc` is interposed.
The smoke program checks
max alignment, 64-byte through 64 KiB alignment, invalid arguments, POSIX error
preservation, zeroing, resize preservation, overflow handling (including
`reallocarray` multiplication overflow), freeing, and concurrent hybrid
allocations from four pthreads. `realloc` reuses the current block when its
class or mapping has enough recorded capacity. Direct mappings grow with
`mremap` when possible; their stored alignment lets the returned user pointer
remain aligned if the mapping moves. A shifted payload is moved within the
resized mapping, and allocation-copy-free remains the fallback.
`nelibc_allocator_owns` checks whether an address falls within a recorded
Nelibc slab or direct mapping without reading memory around that address. It
walks the immutable slab registry without locking. Direct-map lookups use a
reader gate; free closes the gate, drains active readers, unlinks the mapping,
and keeps the gate closed until `munmap` finishes. The current 32 KiB lookup
shootout measures 11.55–13.10 ns across the four compiler/libc profiles.
Hybrid builds also provide `nelibc_hybrid_free` and `nelibc_hybrid_realloc`;
they route Nelibc-owned pointers to the Nelua allocator and all other pointers
to the host allocator. Hybrid `getline`/`getdelim` buffers use the selected
profile's public `realloc` path and can be released through its matching `free`
path. Static musl consumers must use `MUSL_STDIO_WRAP_FLAGS` for that dispatch.

`src/stdio_hybrid.nelua` exports `getchar`, `putchar`, `puts`, `fputc`, `fputs`,
`getchar_unlocked`, `putchar_unlocked`, `fgetc`, `getc`, `fgets`, `getdelim`,
`fgets_unlocked`, `fputs_unlocked`, `getline`, `ungetc`, `fread`, `fwrite`,
`fflush`, `fileno`, `feof`, `ferror`, `clearerr`, `perror`, and `remove`. The Nelua wrappers use host stream accessors
and unlocked operations under locks when the standard function locks. The two
unlocked character functions intentionally omit locking, without exposing
glibc or musl `FILE` internals to Nelua code. `perror` preserves the caller's
errno; `remove` calls Nelibc's `unlinkat` implementation for both files and
directories. `getdelim`/`getline` allocate
with the host `realloc`, so returned buffers remain compatible with host `free`.
The musl hybrid routes `ungetc`, `fread`, `fwrite`, `fflush`, `fileno`, `feof`,
`ferror`, `clearerr`, `fgetpos`, `fsetpos`, `fseek`, and `ftell` through Nelua
`__wrap_*` entry points to avoid colliding with musl's strong definitions and
weak aliases. It also wraps `fseeko` and `ftello`, keeping the 64-bit `off_t`
interface routed through Nelua while preserving musl's implementation.
`setvbuf` also routes through a Nelua wrapper to preserve musl's caller-buffer
and mode behavior.
Bulk I/O and status wrappers forward to musl's aliases; `ungetc`, `fgetpos`, and
`fsetpos` forward through linker-provided `__real_*` symbols. Nelua treats
`fpos_t` as opaque, preserving musl's position layout. These preserve host
behavior while routing consumer references through Nelibc. Static musl
consumers must use the matching `-Wl,--wrap` flags; the Makefile provides them
through `MUSL_STDIO_WRAP_FLAGS` for hybrid targets.
The glibc hybrid also replaces `freopen` through glibc's separate `freopen64`
entry point to avoid recursing into the interposed symbol. The musl hybrid
routes `freopen` through `__wrap_freopen`, which delegates to the linker's
`__real_freopen` symbol; `MUSL_STDIO_WRAP_FLAGS` includes that interposition.
`src/stdio.nelua` provides standalone `getchar`, `putchar`, `puts`, and `perror`
through direct syscalls. It also provides buffered `fopen`, `fclose`, `fflush`,
`fseek`, `ftell`, `fgetpos`/`fsetpos`, `rewind`, `setvbuf`/`setbuf`, `fgetc`/`getc`, `fgets`, `ungetc`, `fread`/`fwrite`, stream indicators, and recursive
`flockfile` operations. POSIX `getdelim` and `getline` grow caller buffers as
needed. POSIX `fdopen` and `fileno` wrap and inspect existing descriptors.
The standalone `freopen` closes a stream's current association and reuses the
same `FILE` object for the replacement, including standard stream objects; a
failed replacement leaves the old stream closed. `fmemopen` provides read/write
streams over caller-owned or internally allocated memory. `open_memstream` grows
a caller-owned output buffer and publishes its current contents and length on
flush and close.
`tmpfile` creates an exclusive close-on-exec file and unlinks it
immediately. `remove` and `rename` use the standalone `unlinkat` and `renameat`
syscall wrappers. `printf`, `vprintf`, `fprintf`, `vfprintf`, `dprintf`,
`vdprintf`, `sprintf`, `vsprintf`, `snprintf`, `vsnprintf`, `asprintf`, and
`vasprintf` support
integer, floating-point `%f`/`%F`/`%e`/`%E`/`%g`/`%G`/`%a`/`%A`, pointer, string,
character, and percent conversions with flags, width, and precision. Floating
precision is supported through fifteen decimal places. Each standalone `FILE` embeds a 4 KiB buffer and can
also use a caller-provided buffer or switch to unbuffered and line-buffered
mode. A small
futex shim supplies per-stream locks, and `_start` initializes the standard
streams before calling user `main`. `include/nelibc_stdio_standalone.h` mirrors
the standalone `FILE` layout. `include/stdio.h` selects this definition for
`NELIBC_STANDALONE` builds and forwards to the host header for hybrid builds.
Standalone archives also export unlocked string/character I/O,
`fseeko`/`ftello`, `setbuffer`, and `setlinebuf`; syscall smoke exercises them
through the standalone `FILE` implementation.
The static musl hybrid replaces `fread` and `fwrite` through targeted linker
wrapping because each unlocked entry point aliases the same strong libc
definition. It leaves `ungetc` with musl because there is no separate
pushback helper. The glibc hybrid replaces all three through Nelua.
`include/nelibc_stdio.h` declares the extension surface without shadowing the
host `stdio.h`. Smoke builds redirect stdout to a pipe; hybrid builds check
buffered output order, complete-element and partial-element `fread` behavior,
and verify that concurrent `puts` calls remain line-atomic under the host
stream lock.

`src/errno/host.nelua` keeps hybrid builds interoperable by sharing the host
errno slot. `src/errno/standalone.nelua` implements `__errno_location` over
Nelua thread-local storage. It links and runs in the glibc shootout target;
the musl target compiles this object only because a static musl link already
defines `__errno_location` and therefore must be replaced as part of a complete
standalone runtime. `make smoke-syscall-tls-gcc` also runs a two-thread errno
isolation check against the standalone TLS provider.
There is no custom `include/errno.h`: the toolchain header supplies
target-specific error values and maps `errno` to `__errno_location()`. Hybrid
builds share the host slot; standalone builds provide that ABI symbol from
Nelua.
`strtok` stores its continuation pointer in a dedicated per-thread pointer.
The `make strtok-thread-matrix` check interleaves two tokenizers under Zig/glibc,
GCC/glibc, Zig/musl, and TinyCC/glibc.
