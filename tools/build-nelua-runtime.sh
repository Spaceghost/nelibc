#!/bin/sh
# Build the bundled Lua interpreter and Nelua's builtin modules.
set -eu

if [ "$#" -ne 6 ]; then
    echo 'usage: build-nelua-runtime.sh ZIG ZIG_LIB_DIR NELUA_SRC NELIBC_SRC ARCHIVE OUTPUT' >&2
    exit 2
fi

zig=$1
zig_libdir=$2
nelua=$3
nelibc=$4
archive=$5
output=$6

# LUA_USE_POSIX enables process and file APIs without the dynamic loader.
# Use -nostdlib and only compiler headers, Linux UAPI, and Nelibc headers.
"$zig" cc -target x86_64-linux-musl -mcpu=x86_64 \
    -std=gnu99 -O2 -DNDEBUG -DLUA_USE_POSIX -DMAXRECLEVEL=400 \
    -DNELIBC_STANDALONE -nostdlib -static -no-pie -fno-pie \
    -fno-stack-protector -ffreestanding -fno-builtin -fno-sanitize=all \
    -Wl,-e,_start -Wl,--build-id=none \
    -I"$nelibc/include" -I"$nelua/src/lua" \
    -isystem "$zig_libdir/libc/include/x86-linux-any" \
    -isystem "$zig_libdir/libc/include/any-linux-any" \
    "$nelua/src/onelua.c" "$nelua/src/luainit.c" "$nelua/src/sys.c" \
    "$nelua/src/lfs.c" "$nelua/src/hasher.c" \
    "$nelua/src/lpeglabel/lplcap.c" "$nelua/src/lpeglabel/lplcode.c" \
    "$nelua/src/lpeglabel/lplprint.c" "$nelua/src/lpeglabel/lpltree.c" \
    "$nelua/src/lpeglabel/lplvm.c" \
    -Wl,--whole-archive "$archive" -Wl,--no-whole-archive \
    -o "$output"
