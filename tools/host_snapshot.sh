#!/bin/sh
# Emit a generated-C snapshot of the host profile: every Nelua module of the
# host archive lowered to C with the pinned Nelua, beside the C and assembly
# sources it links with and a plain Makefile. A consumer builds
# libnelibc-host.a from the snapshot with a C compiler, ar, and objcopy; no
# Nelua is needed.
#
# usage: tools/host_snapshot.sh "<nelua command>" <output directory>
set -eu
NELUA=$1
OUT=$2
case "$OUT" in
  ''|/|.|..) echo "host_snapshot: refusing output directory '$OUT'" >&2; exit 2 ;;
esac
if [ -e "$OUT" ] && [ ! -f "$OUT/SNAPSHOT" ]; then
  echo "host_snapshot: $OUT exists and is not a previous snapshot" >&2
  exit 2
fi
rm -rf "$OUT/src" "$OUT/include" "$OUT/obj-"* "$OUT/Makefile" "$OUT/SNAPSHOT"
mkdir -p "$OUT/src"
cp -R include "$OUT/include"

# name  source  (the extra per-object cflags live in the generated Makefile)
MODULES='
string_locale src/string_locale_standalone.nelua
string src/string.nelua
inet src/inet.nelua
memory src/memory.nelua
ctype src/ctype.nelua
stdlib src/stdlib.nelua
locale src/locale_standalone.nelua
stdlib_exit src/stdlib_exit_standalone.nelua
search src/search_standalone.nelua
regex src/regex_standalone.nelua
pwd src/pwd_standalone.nelua
termios src/termios_standalone.nelua
stdlib_float src/stdlib_float.nelua
sys_linux_x86_64 src/sys/linux_x86_64.nelua
errno src/errno/standalone.nelua
thread src/thread.nelua
alloc src/alloc.nelua
alloc_exports src/alloc/standalone_exports.nelua
spawn src/spawn_standalone.nelua
unistd src/unistd.nelua
stdio src/stdio.nelua
math_core src/math_core.nelua
fenv src/fenv_standalone.nelua
'
echo "$MODULES" | while read -r name source; do
  [ -n "$name" ] || continue
  $NELUA -c -r -o "$OUT/src/$name.c" "$source"
done

for c in stdio_varargs fcntl_adapter credential_adapter stdio_lock_standalone \
    stdio_startup_stub environment_adapter semaphore_adapter mqueue_adapter \
    complex_abi; do
  cp "src/$c.c" "$OUT/src/$c.c"
done
cp src/host/syscall_host.c "$OUT/src/syscall_host.c"
cp src/host/boot_host.c "$OUT/src/boot_host.c"
cp src/sys/linux_x86_64.S "$OUT/src/linux_x86_64.S"
cp src/memory_x86_64.S "$OUT/src/memory_x86_64.S"
cp src/math_core_x86_64.S "$OUT/src/math_core_x86_64.S"

{
  echo "nelibc $(git rev-parse HEAD 2>/dev/null || echo unknown)"
  echo "nelua $($NELUA --version 2>/dev/null | head -1)"
  echo "generated $(date -u +%Y-%m-%dT%H:%M:%SZ)"
} > "$OUT/SNAPSHOT"

cat > "$OUT/Makefile" <<'MK'
# Nelibc host profile, generated-C snapshot. Builds libnelibc-host.a with a C
# compiler, ar, and objcopy; see docs/host-profile.md in the Nelibc tree.
#   make CC='zig cc -mcpu=x86_64'      or      make CC=gcc
# O selects the object/archive directory so several compilers can coexist.
CC ?= cc
AR ?= ar
OBJCOPY ?= objcopy
O ?= .

CFLAGS_BASE := -DNELIBC_STANDALONE -ffreestanding -fno-builtin -fno-stack-protector -fno-sanitize=all -Iinclude
# GCC turns byte loops back into memcpy/memset calls; this library is them.
GCC_ONLY := $(shell $(CC) --version 2>/dev/null | head -1 | grep -q -i 'gcc' && echo -fno-tree-loop-distribute-patterns)
CFLAGS_stdlib := -ftls-model=local-exec
CFLAGS_errno := -ftls-model=local-exec
CFLAGS_stdio_varargs := -std=c11
CFLAGS_syscall_host := -std=c11
CFLAGS_boot_host := -std=c11
# Clang-only: no address-significance table, so objcopy can drop a section.
CLANG_ONLY := $(shell $(CC) --version 2>/dev/null | head -1 | grep -q -i 'clang' && echo -fno-addrsig)
CFLAGS_environment_adapter := -g0 -ffunction-sections -fno-asynchronous-unwind-tables -fno-unwind-tables $(CLANG_ONLY)

C_OBJECTS := string_locale string inet memory ctype stdlib locale stdlib_exit search regex pwd termios \
  stdlib_float sys_linux_x86_64 errno thread alloc alloc_exports spawn unistd \
  stdio math_core fenv stdio_varargs fcntl_adapter credential_adapter \
  stdio_lock_standalone stdio_startup_stub \
  semaphore_adapter mqueue_adapter complex_abi syscall_host boot_host
OBJECTS := $(addprefix $(O)/,$(addsuffix .o,$(C_OBJECTS))) \
  $(O)/environment_adapter_host.o \
  $(O)/linux_x86_64_host.o $(O)/memory_x86_64.o $(O)/math_core_x86_64.o

all: $(O)/libnelibc-host.a

$(O):
	mkdir -p $@

$(O)/libnelibc-host.a: $(OBJECTS)
	rm -f $@
	$(AR) rcs $@ $(OBJECTS)

$(O)/%.o: src/%.c | $(O)
	$(CC) $(CFLAGS_BASE) $(GCC_ONLY) $(CFLAGS_$*) -c $< -o $@

# The Linux syscall helper object keeps its lock, registry, and sigreturn
# helpers but loses the raw syscall section and the weak aliases to it, so
# the host dispatcher is the only definition the linker can pick.
$(O)/linux_x86_64_host.o: src/linux_x86_64.S | $(O)
	$(CC) -g0 -c $< -o $(O)/linux_x86_64.o
	$(OBJCOPY) --remove-section=.text.nelibc_raw_syscall --strip-symbol=nelibc_raw_syscall3 --strip-symbol=nelibc_raw_syscall6 --strip-symbol=nelibc_raw_syscall3_real --strip-symbol=nelibc_raw_syscall6_real $(O)/linux_x86_64.o $@

# The environment adapter's nelibc_thread_detached_exit unmaps its own stack
# and leaves through a `syscall` instruction; the dispatcher's lane version
# replaces it, so the section goes.
$(O)/environment_adapter_host.o: $(O)/environment_adapter.o
	$(OBJCOPY) --remove-section=.text.nelibc_thread_detached_exit $< $@

$(O)/memory_x86_64.o: src/memory_x86_64.S | $(O)
	$(CC) -c $< -o $@
$(O)/math_core_x86_64.o: src/math_core_x86_64.S | $(O)
	$(CC) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(O)/linux_x86_64.o $(O)/environment_adapter.o $(O)/libnelibc-host.a

.PHONY: all clean
MK
echo "host snapshot written to $OUT"
