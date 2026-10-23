# Full shootout matrix — 2026-10-22

Captured from `make shootout-matrix` on this host after rebuilding the
benchmark libraries. The matrices compare Nelibc with host libc under
Zig/glibc, GCC/glibc, Zig/musl, and TinyCC/glibc where each benchmark supports
those profiles. Timings are machine-specific; each CSV retains the benchmark's
sample and iteration counts.

The full target completed successfully. The float benchmark harness now
provides its C-locale radix callback directly, so its Nelibc parser link no
longer depends on the unrelated environment adapter object.

This is the pre-radix-cache baseline. The subsequent float benchmark uses a
`localeconv()` callback matching the hybrid adapter and is recorded separately
in `../2026-10-22-float-radix-cache/`.

CSV files: `mem`, `strstr`, `strcasecmp`, `strverscmp`, `sysconf`, `confstr`,
`pathconf`, `float`, `int`, `rand-r`, `qsort`, `stdio`, and `io`.
