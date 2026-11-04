# nelibc agent agreement

## Branch

Work on `main` (Jack calls it master). It is the integration branch and the
only long-lived one. The `host-profile` branch was merged into `main` on
2026-11-04 and is retired; do not revive it. Short-lived topic branches are
fine, but land them on `main` with ordinary direct pushes. No PRs unless
asked, never force-push.

## Where to build

Never compile or run tests on the workstation. Every build and every smoke,
diff, and shootout runs inside the Incus container `nelibc-writ` on bak
(Debian 12; Nelua `3c29419` at `/opt/nelua/nelua`, Zig 0.17.0 at
`/opt/zig/zig`, GCC 12.2, TinyCC 0.9.27, musl-gcc, Bend 2.0.39 at
`/opt/bend-2.0.39/bend/bin/bend`). The checkout is `/work/nelibc`, a clone of
`origin/main`; push, then `git pull` there. Run the matrix as the
unprivileged `build` user, not root: several syscall smokes assert the
unprivileged error path, and root in the container is privileged enough to
make them succeed instead.

```sh
ssh bak incus exec nelibc-writ -- su build -c 'cd /work/nelibc && git pull -q && \
  TMPDIR=/work/tmp ZIG_GLOBAL_CACHE_DIR=/work/zig-cache \
  make smoke-matrix "NELUA=/opt/nelua/nelua --cache-dir /work/nelua-cache" ZIG=/opt/zig/zig'
```

Debian's packaged TinyCC 0.9.27 cannot build the TinyCC profile (see the
quickstart); `TCC=` must point at a mob-branch build. Until one is installed
in the container, `*-tcc` targets are unrun.

CI (`.github/workflows/host.yml`) runs on the self-hosted bak runner and
covers the host profile plus the standalone and freestanding Zig smokes only.
The full matrix is a manual gate.

## The gate

Before pushing a behavior change, run and record the result of:

| Target | Covers |
| --- | --- |
| `make smoke-matrix` | hybrid (zig/glibc, gcc/glibc, zig/musl, tcc/glibc), standalone (zig, gcc, tcc), freestanding (zig, gcc), consumers, diffs |
| `make dogfood-matrix` | link a C program against each hybrid archive |
| `make host-matrix` + `make host-bend-hello-zig host-bend-parallel-zig` | host profile (zig, gcc), the Bend runtime over it, and its worker pool as lanes |
| `make host-snapshot-matrix` | the generated-C snapshot rebuilt without Nelua (zig, gcc) |
| `make shootout-matrix` | performance versus the host libc per profile |

A shootout result is a number from a specific profile on a specific machine;
report it with both. A smoke that was not run is "not run", not "passes".

## Evidence standard

Do not report a build, test, or shootout result that was not observed on the
exact artifact and profile claimed. Static checks prove only what they run.
When something is unimplemented, say so in the docs rather than writing
around it. Keep secrets out of history.
