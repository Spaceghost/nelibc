# TinyCC integration compiler

The 2026-11-17 full gate uses TinyCC mob commit
`32d1c0a5614b9979768b3711726db3d601750ed3` with
`tinycc-x87-argument-pop.patch`. Debian's packaged 0.9.27 remains unsupported.

The pinned mob compiler emits `fstpt` when passing a long-double argument,
then lets `vpop()` emit another x87 pop. The second pop raises `FE_INVALID`
when the stack is empty. This breaks exact `powl(x, 1)` exception checks and
the standalone math classification smoke. The patch marks the consumed
argument as a constant before releasing its compiler value.

Apply the patch to an isolated checkout of the pinned commit, configure a
separate installation prefix, and run `make`, `make install`, and `make test`.
The compiler's full test suite passed on Bak. Retain the unpatched compiler
separately; do not silently replace a system compiler.

Compile `tinycc_x87_argument.c` with each compiler and link with `-lm`.
The unpatched compiler returns 2; the patched compiler returns 0. The full
Nelibc gate must then use the patched `TCC` and its matching `TCC_LIBTCC1`.
Passing this reproducer alone does not establish that Nelibc's matrix passes.
