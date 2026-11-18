#ifndef NELIBC_ATTRIBUTES_H
#define NELIBC_ATTRIBUTES_H

/* Pointer-reading queries may depend on mutable memory: pure, never const.
 * Keep these declarations usable by non-GNU C compilers as well. */
#if defined(__GNUC__) || defined(__clang__)
#define NELIBC_PURE __attribute__((__pure__))
#define NELIBC_CONST __attribute__((__const__))
#else
#define NELIBC_PURE
#define NELIBC_CONST
#endif

#endif
