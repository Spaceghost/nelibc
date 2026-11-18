# Full integration gate — 2026-11-17 Pacific / 2026-11-18 UTC

All 14 required and additional profile gate targets below exited zero on
Bak, inside Incus `nelibc-writ`, as the unprivileged `build` user.
The tested implementation is commit `d55e30050d987bc2dbbe1230d4f83f36043465e2`.
All 880 source-file hashes match that commit. The source was frozen throughout
the run; no other agent build ran during its shootout matrix.

A passing gate accepts the checked-in expected-failure lists. It does not
mean every libc-test case passes or establish complete C/POSIX conformance.

## Gate results

| Target | Exit | Seconds |
| --- | ---: | ---: |
| `smoke-matrix` | 0 | 153.40 |
| `dogfood-matrix` | 0 | 22.01 |
| `host-matrix` | 0 | 12.47 |
| `host-bend-hello-zig` | 0 | 11.61 |
| `host-bend-parallel-zig` | 0 | 6.79 |
| `host-snapshot-matrix` | 0 | 14.69 |
| `sys-ops-check` | 0 | 0.08 |
| `libc-test-gcc` | 0 | 35.29 |
| `libc-test-zig` | 0 | 75.81 |
| `libc-test-standalone-gcc` | 0 | 27.53 |
| `libc-test-standalone-zig` | 0 | 21.15 |
| `smoke-cosmo` | 0 | 3.07 |
| `purity-check` | 0 | 0.50 |
| `shootout-matrix` | 0 | 608.55 |

The RESETIDS spawn probe was **skipped** because user-namespace mapping is
unavailable in this container. Isolation and privileges were not changed.
Expected wordexp syntax-error and perror diagnostics remain in the logs.

## Conformance gates

The hybrid GCC gate built 344 test binaries and recorded 30 failure entries,
all 30 expected. The hybrid Zig gate built 344 binaries and recorded 34
failure entries, all 34 expected. Failure entries can include API build
failures, so subtracting them from the binary count is not a pass count.

Standalone GCC: 338/343 pass, four build failures and one runtime failure.
Standalone Zig: 339/343 pass and four build failures. There were no new
failures outside any of the four existing allowlists; none were expanded.
These gates use Nelibc's headers and differ from the separate runtime-header
corpus comparing each libc with its Nelibc-less version.

## Repairs needed to run the complete gate

- Declare TinyCC runtime-object prerequisites so its clean stdio build does
  not rely on objects produced by a later target.
- Preserve TinyCC constructor/destructor attributes after glibc headers
  erase them; otherwise the syscall smoke exits before exercising syscalls.
- Replace TinyCC's arithmetic classification macros with the existing
  bit classifiers and single-evaluation dispatch. Tests cover floating-point
  exceptions, negative zero, NaNs, subnormals and side-effecting arguments.
- Keep Zig sanitizer runtime references out of the TinyCC adapter archive;
  reuse the Zig-built atomic adapter in its integer shootout.
- Keep legacy benchmark calls observable through volatile function pointers.
  Read-only query purity had exposed loops that reported invalid zero timings.
  The published matched comparison already used observable wrappers.

TinyCC mob commit `32d1c0a5614b9979768b3711726db3d601750ed3` also needs the
[pinned x87 argument-pop patch and regression](../tools/toolchains/README.md).
The original compiler returns 2 in the regression; the patched compiler
returns 0 and passes its own complete test suite. Applying the retained
patch reproduces the compiled compiler source byte for byte. All eight
previously failing TinyCC shootouts passed in isolation before this full run.

## Toolchains and evidence

The separate `memcmp-shootout-matrix`, `memscan-shootout-matrix` and
`memscan-threshold-shootout` targets also exited zero. These additionally
exercise the changed comparison/search benchmark tools outside the main gate.

Nelua `dabbd9ef`, GCC 12.2, Zig 0.17, Bend 2.0.39 and Cosmopolitan 4.0.2;
TinyCC uses the pinned patched build above. The run uses `make -k -j1` to
avoid concurrent writes to Nelua's shared generated-C cache, with libc-test
`JOBS=2`. Exact commands and compiler versions are recorded in the manifest.

Retained in the container at
`/srv/ai/artifacts/nelibc-cosmo/integration-20261117-complete/`:
target logs, source hashes, archive hashes, actual tested archives, corpus
results and fresh benchmark CSVs. Earlier failed/interrupted sweeps remain
separate; their invalid zero timings are not used as performance evidence.

The [easy-first paired comparison](easy-first-20261117.md) remains the
performance comparison, with its exact historical source and archives.
On Bak's i5-10400 with GCC 12.2/glibc 2.36, median paired speedups include
1.14x for decimal strtol and 1.11x for 128-integer qsort; qsort varies across
rounds. The mixed parser is still 0.75x glibc's speed. Short strlen and
substring search remain valuable gaps. Fresh gate shootouts are additional
diagnostics and are not substituted for that paired experiment. This is
not evidence that Nelibc is globally the fastest libc.
