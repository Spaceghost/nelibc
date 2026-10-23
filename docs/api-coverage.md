# API coverage manifest

Run `make api-coverage` to refresh `build/api-coverage.csv` from the public
headers and current archive exports. It compares Zig/glibc, GCC/glibc,
Zig/musl, TinyCC/glibc, and standalone Zig, GCC, and TinyCC archives. When
TinyCC is unavailable, `make api-coverage-available` refreshes
`build/api-coverage-available.csv` for Zig/glibc, GCC/glibc, and Zig/musl.
When all four hybrid toolchains are available, `make api-coverage-hybrid`
also includes TinyCC/glibc in `build/api-coverage-hybrid.csv`.

The manifest records declarations, source locations, per-profile exports, and
smoke or differential files that mention each API name. Smoke references are
only a locator for review; they do not prove complete behavioral coverage.
`missing-standalone-export` is an implementation gap. A function omitted by a
hybrid archive is marked `implemented;host-delegated:<profile>` only when the
build intentionally relies on the selected libc for that symbol. Other
unexplained hybrid omissions are marked `standalone-export;unexported:<profile>`
so they remain visible as gaps. `malloc` is marked `implemented` in all seven
archive profiles. Static musl links Nelua's public `malloc`; its `free` and
`realloc` calls remain routed through ownership-aware wrappers so host-owned
allocations from libc paths remain valid.

The static musl hybrid routes `ungetc`, `freopen`, `fread`, `fwrite`, `fflush`,
`fileno`, `feof`, `ferror`, `clearerr`, `fgetpos`, `fsetpos`, `fseek`, and
`ftell`, `fseeko`, `ftello`, `setvbuf`, `flockfile`, `ftrylockfile`,
`funlockfile`, `fclose`, `localeconv`, `setlocale`, `atexit`, `exit`, `free`, and
`realloc` through exported
`__wrap_*` entry points. The coverage scanner counts those as the standard APIs for that profile;
consumers must link with the matching `-Wl,--wrap` flags (provided by the
Makefile variable `MUSL_STDIO_WRAP_FLAGS`). The musl archive exports Nelua
`malloc` and uses the ownership-aware `free`/`realloc` wrappers to support
allocations from both Nelibc and musl paths.

The declaration scanner handles the C declaration forms currently used by
these headers. Review new or unusual declaration syntax when extending the
header set. The implementation scanner also recognizes public function names
passed as the first argument to `DEFINE_*` wrapper macros in C adapters.
The standalone-only pthread declarations are classified as host-delegated in
hybrid profiles because `include/pthread.h` forwards to the selected host
header there.
