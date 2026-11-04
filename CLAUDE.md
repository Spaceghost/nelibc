# CLAUDE.md

Read `AGENTS.md` first; it is binding. Summary:

- Branch: `main` only (master). Direct pushes, no PRs, no force-push.
- Build host: Incus container `nelibc-writ` on bak, never the workstation.
  Checkout there is `/work/nelibc`; push, pull, then run `make` with
  `NELUA=/opt/nelua/nelua ZIG=/opt/zig/zig`.
- Gate: `smoke-matrix`, `dogfood-matrix`, `host-matrix`, `shootout-matrix`.
- Report only results observed on the named profile; unrun is unrun.

Project layout: Nelua sources in `src/`, C headers in `include/`, smokes,
diffs, and shootouts in `tools/`, build rules in `Makefile` and `host.mk`,
coverage and roadmap in `docs/`.
