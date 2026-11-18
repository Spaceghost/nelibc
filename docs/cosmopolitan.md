# Cosmopolitan fallback

The `cosmo` hybrid profile compiles Nelibc with Cosmopolitan headers and links
unimplemented services from Cosmopolitan libc, without GNU libc. It currently
targets **Linux x86-64**; it does not claim cross-platform runtime conformance.

## Build

Install the [official Cosmopolitan 4.0.2 toolchain](https://github.com/jart/cosmopolitan/releases/tag/4.0.2)
on the build host. Download `cosmocc-4.0.2.zip` from that release and extract it
with a ZIP tool that preserves Unix symlinks and executable modes (`unzip`).
The archive used for validation has SHA-256:

```
85b8c37a406d862e656ad4ec14be9f6ce474c1b436b9615e91a55208aced3f44
```

Use the compiler **wrapper**, not the bare `x86_64-linux-cosmo-gcc` driver:

```sh
export PATH=/path/to/cosmocc/bin:$PATH
make smoke-cosmo
make dogfood-cosmo
make lib-cosmo
x86_64-unknown-cosmo-cc -fno-builtin app.c build/libnelibc-cosmo.a -o app
```

Set `NELUA`, `COSMOCC`, `COSMOFLAGS`, `COSMO_NELUA_CACHE`, and `OUT` to override their defaults.
Follow `AGENTS.md` for this repository's build-host and storage requirements.
Use Cosmopolitan's headers for the consumer; do not add `-Iinclude`, which
would expose Nelibc's standalone Linux layouts instead of Cosmopolitan's ABI.
Objects are built separately under `$(OUT)/cosmo`; existing glibc/musl objects
are never reused.

## Ownership

Nelibc supplies memory, strings, narrow character classification, floating
conversions, IPv4/IPv6 text conversions, and real/complex math. Cosmopolitan
supplies the missing APIs, including process startup, allocation, stdio,
threads, filesystem calls, resolver, locale handles, and integer conversions.
The adapter shares Cosmopolitan's `errno` slot and uses thread-local buffers
for Nelibc's `strtok` and `strsignal`. Collation and the numeric radix follow
Cosmopolitan's C-locale behavior. This is a hybrid profile, not the standalone
or host-callback profile, and its symbol coverage differs from the old profiles.

`smoke-cosmo` runs the memory boundary/differential test, complex math smoke,
a link with every archive member forced in,
and a consumer exercising Nelibc core calls with Cosmopolitan stdio, allocator,
resolver, and threads. The consumer link map proves selection of Nelibc objects;
ELF checks reject unresolved symbols, an ELF interpreter, shared dependencies,
and GLIBC-versioned symbols. These checks demonstrate a static Cosmopolitan
fallback on the tested host, not correctness of every exported function.
