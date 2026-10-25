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

## Pinned musl API comparison

`docs/musl-1.2.5-api-coverage.csv` is a checked-in comparison against musl
1.2.5, built from the upstream release archive
`https://musl.libc.org/releases/musl-1.2.5.tar.gz` (SHA-256
`a9a118bbe84d8764da0ea0d28b3ab3fae8477fc7e4085d90102b8596fc7c75e4`). The
CSV keeps one row for every function declaration recognized in the release
headers and present as a global text/weak symbol in its `libc.a` or `libm.a`.
The audit intersects declarations and exports so function-like header macros
and parser false positives do not become supposed libc functions.

Regenerate it by building/installing that pinned release, then running:

```sh
python3 tools/musl_api_audit.py \
  --musl-include /path/to/musl-install/include \
  --musl-archive /path/to/musl-build/lib/libc.a \
  --musl-archive /path/to/musl-build/lib/libm.a \
  --nelibc-profile zig_glibc=build/libnelibc-zig.a \
  --nelibc-profile gcc_glibc=build/libnelibc-gcc.a \
  --nelibc-profile zig_musl=build/libnelibc-zig-musl.a \
  --nelibc-profile tcc_glibc=build/libnelibc-tcc.a \
  --output docs/musl-1.2.5-api-coverage.csv
```

The current snapshot contains 1,471 APIs: 656 exported by Nelibc in at least
one tested hybrid profile, 126 intentionally delegated to the host in hybrid
profiles, and 689 not declared by Nelibc. The audit removes preprocessor
directives and C++ linkage wrappers before parsing prototypes, while retaining
inline-function closing braces. No baseline API is currently marked as an
intentional omission; the 689 entries remain visible as unimplemented
scope candidates until an explicit policy records why an API is excluded.
No real-valued `math.h` APIs in this pinned musl baseline remain undeclared.
The largest remaining groups are complex math (51), wide characters (65), unistd (42), stdlib (41), and name service/network database
APIs (32). The pthread rows without Nelibc declarations are two private musl
cleanup helper symbols. This is a breadth comparison, not a behavioral
conformance score.

`scalb` and `scalbf` scale by integral exponents and report invalid fractional
exponents; `significand` and `significandf` normalize finite nonzero inputs so
the absolute result lies in [1, 2), preserving the input sign.
`lgammal` shifts positive inputs to a Stirling expansion with exact rational
correction coefficients; negative inputs use the reflection formula, with a
small-argument path that avoids subnormal sine loss. `tgammal` exponentiates the
logarithmic result. Differential checks use a relative bound of 4,096 binary80
epsilons for this composed calculation; correct rounding is not claimed.
`erfl` uses a power series near zero and `erfcl` uses a continued fraction in
the positive tail; the long-double functions are differentially checked but are
not claimed to be correctly rounded.
`fmal` is implemented with an exact binary80 significand accumulator and a
single final rounding step. Its accumulator uses 1,026 64-bit limbs to cover
the full binary80 product/addend exponent range, so this correctness-focused
software path is not optimized for speed. The x86-64 implementation also
updates x87 and SSE exception flags.
`exp10l` and its GNU-compatible `pow10l` alias reuse the existing `powl`
implementation for special values, range handling, and exponentiation. They
inherit its accuracy limits for extreme subnormal results; that range needs a
dedicated long-double base-10 implementation.
The `__fpclassify*` and `__signbit*` compatibility symbols classify the
binary32, binary64, and binary80 layouts directly from their representations.

The report uses the same bounded declaration scanner as the project API
inventory and requires the headers and archives from the same musl release.
Review scanner changes against representative prototypes before refreshing the
snapshot.
Standalone pthread implementations, including barriers and read-write locks,
are classified as host-delegated in hybrid profiles because
`include/pthread.h` forwards to the selected host header there.
