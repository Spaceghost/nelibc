# Host profile: the standalone archive with its Linux entry point and raw
# `syscall` instructions removed, dispatching syscall numbers to a
# nelibc_host table (include/nelibc_host.h, docs/host-profile.md). The Nelua
# modules are the standalone objects unchanged. Included from the Makefile.
HOST_CFLAGS := -DNELIBC_STANDALONE -std=c11 -ffreestanding -fno-builtin -fno-stack-protector -fno-sanitize=all -Iinclude
HOST_LINK_FLAGS := -DNELIBC_STANDALONE -nostdlib -static -no-pie -fno-pie -fno-stack-protector -ffreestanding -fno-builtin -fno-sanitize=all -Wl,-e,_start -Wl,--build-id=none -Iinclude

$(OUT)/host-syscall-zig.o: src/host/syscall_host.c include/nelibc_host.h | $(OUT)
	$(ZIG) cc $(ZIGFLAGS) $(HOST_CFLAGS) -c $< -o $@
$(OUT)/host-boot-zig.o: src/host/boot_host.c include/nelibc_host.h | $(OUT)
	$(ZIG) cc $(ZIGFLAGS) $(HOST_CFLAGS) -c $< -o $@
$(OUT)/host-syscall-gcc.o: src/host/syscall_host.c include/nelibc_host.h | $(OUT)
	gcc $(HOST_CFLAGS) -fno-tree-loop-distribute-patterns -c $< -o $@
$(OUT)/host-boot-gcc.o: src/host/boot_host.c include/nelibc_host.h | $(OUT)
	gcc $(HOST_CFLAGS) -fno-tree-loop-distribute-patterns -c $< -o $@

# The host archive keeps the Linux syscall assembly object for its lock and
# registry helpers but strips the weak raw-syscall aliases, so the dispatcher
# is the only definition the linker can pick. The environment adapter loses
# nelibc_thread_detached_exit (its own section: it unmaps its stack and leaves
# through a `syscall` instruction); the dispatcher's lane version replaces it.
define host-archive
	cp $< $@
	$(AR) d $@ start-linux-x86_64-standalone-$(1).o sys-linux-x86_64-standalone-$(1)-asm.o environment-adapter-standalone-$(1).o
	$(OBJCOPY) --strip-symbol=nelibc_raw_syscall3 --strip-symbol=nelibc_raw_syscall6 $(OUT)/sys-linux-x86_64-standalone-$(1)-asm.o $(OUT)/sys-linux-x86_64-host-$(1)-asm.o
	$(OBJCOPY) --remove-section=.text.nelibc_thread_detached_exit --strip-symbol=nelibc_thread_detached_exit $(OUT)/environment-adapter-standalone-$(1).o $(OUT)/environment-adapter-host-$(1).o
	$(AR) rs $@ $(OUT)/sys-linux-x86_64-host-$(1)-asm.o $(OUT)/environment-adapter-host-$(1).o $(OUT)/host-syscall-$(1).o $(OUT)/host-boot-$(1).o
endef

$(OUT)/libnelibc-host-zig.a: $(OUT)/libnelibc-standalone-zig.a $(OUT)/host-syscall-zig.o $(OUT)/host-boot-zig.o
	$(call host-archive,zig)
$(OUT)/libnelibc-host-gcc.a: $(OUT)/libnelibc-standalone-gcc.a $(OUT)/host-syscall-gcc.o $(OUT)/host-boot-gcc.o
	$(call host-archive,gcc)

lib-host-zig: $(OUT)/libnelibc-host-zig.a
lib-host-gcc: $(OUT)/libnelibc-host-gcc.a

# Every symbol the host archive needs is inside it or in nelibc_host, and the
# only `syscall` instruction left is nelibc_rt_sigreturn's, the restorer the
# Linux profiles install with sigaction; the dispatcher never installs it.
HOST_SYSCALL_ALLOWED := nelibc_rt_sigreturn
define host-syscall-scan
	@found="$$($(OBJDUMP) -d $(1) | awk '/^[0-9a-f]+ <.*>:$$/ { fn = $$2 } /\tsyscall/ { print fn }' | sort -u | tr -d '<>:' | grep -vx '$(HOST_SYSCALL_ALLOWED)')"; test -z "$$found" || { echo "syscall instructions in $(1): $$found"; exit 1; }
endef
define host-check
	@test -z "$$($(NM) -u $(OUT)/host-linked-$(1).o | grep -v _GLOBAL_OFFSET_TABLE_)" || { $(NM) -u $(OUT)/host-linked-$(1).o; exit 1; }
	$(call host-syscall-scan,$(OUT)/host-linked-$(1).o)
	@echo 'host archive ($(1)): no undefined symbols, no syscall instruction outside $(HOST_SYSCALL_ALLOWED)'
endef
host-check-zig: $(OUT)/libnelibc-host-zig.a
	$(ZIG) cc $(ZIGFLAGS) -nostdlib -r -Wl,--whole-archive $< -Wl,--no-whole-archive -o $(OUT)/host-linked-zig.o
	$(call host-check,zig)
host-check-gcc: $(OUT)/libnelibc-host-gcc.a
	gcc -nostdlib -r -Wl,--whole-archive $< -Wl,--no-whole-archive -o $(OUT)/host-linked-gcc.o
	$(call host-check,gcc)

# Hosted consumer: tools/host_linux.c is the "kernel" (raw Linux syscalls for
# console, memory, clock, exit); tools/host_smoke.c is the program.
define host-smoke-run
	@test -z "$$($(NM) -u $(OUT)/host-smoke-$(1))"
	@$(OUT)/host-smoke-$(1) > $(OUT)/host-smoke-$(1).log 2>&1; status=$$?; cat $(OUT)/host-smoke-$(1).log; test $$status -eq 7 || { echo "host smoke ($(1)) exited $$status"; exit 1; }
	@grep -q '^host smoke: OK' $(OUT)/host-smoke-$(1).log && grep -q '^atexit handler ran' $(OUT)/host-smoke-$(1).log
endef

host-smoke-zig: $(OUT)/libnelibc-host-zig.a tools/host_smoke.c tools/host_linux.c
	$(ZIG) cc $(ZIGFLAGS) $(HOST_LINK_FLAGS) tools/host_smoke.c tools/host_linux.c $(OUT)/libnelibc-host-zig.a -o $(OUT)/host-smoke-zig
	$(call host-smoke-run,zig)
host-smoke-gcc: $(OUT)/libnelibc-host-gcc.a tools/host_smoke.c tools/host_linux.c
	gcc $(HOST_LINK_FLAGS) -fno-tree-loop-distribute-patterns tools/host_smoke.c tools/host_linux.c $(OUT)/libnelibc-host-gcc.a -o $(OUT)/host-smoke-gcc
	$(call host-smoke-run,gcc)

host-matrix: host-check-zig host-check-gcc host-smoke-zig host-smoke-gcc

# Link any Bend-emitted C file, unmodified, against the host profile and run
# it under the Linux host:
#   make host-bend-run-zig BEND_C=/path/program.c BEND_ARGS='+RTS --threads 1 --gpu off'
# The Zig (clang) driver is required: the generated file uses musttail.
BEND_C ?= $(OUT)/host-bend-hello.c
BEND_OUT ?= $(OUT)/host-bend-program
BEND_ARGS ?=
host-bend-run-zig: $(OUT)/libnelibc-host-zig.a tools/host_linux.c
	$(ZIG) cc $(ZIGFLAGS) $(HOST_LINK_FLAGS) -w -O2 $(BEND_C) tools/host_linux.c $(OUT)/libnelibc-host-zig.a -o $(BEND_OUT)
	@test -z "$$($(NM) -u $(BEND_OUT))"
	@$(BEND_OUT) $(BEND_ARGS) > $(BEND_OUT).log 2>&1; status=$$?; tail -20 $(BEND_OUT).log; test $$status -eq 0 || { echo "bend program exited $$status"; exit 1; }

# Bend runtime smoke: emit C for tools/host_bend_hello.bend with the pinned
# compiler (bend 2.0.39), run it through host-bend-run-zig, expect "hi".
# Skipped when BEND is not installed so the ordinary matrix stays light.
BEND ?= bend
host-bend-hello-zig: $(OUT)/libnelibc-host-zig.a tools/host_bend_hello.bend tools/host_linux.c
	@command -v $(BEND) >/dev/null 2>&1 || { echo 'host-bend-hello-zig: bend not found, skipped'; exit 0; }
	BEND_NO_TELEMETRY=1 $(BEND) tools/host_bend_hello.bend -o $(OUT)/host-bend-hello.c
	$(MAKE) host-bend-run-zig BEND_C=$(OUT)/host-bend-hello.c BEND_OUT=$(OUT)/host-bend-hello-zig
	@test "$$(cat $(OUT)/host-bend-hello-zig.log)" = hi || { echo 'expected hi'; exit 1; }
	@echo 'host-bend-hello-zig: Bend 2 IO.print ran through the nelibc host profile'

# Generated-C snapshot of the host archive (tools/host_snapshot.sh): the
# Nelua modules lowered to C by the pinned Nelua, the C and assembly sources,
# the headers, and a plain Makefile. A consumer needs a C compiler, ar, and
# objcopy only. host-snapshot-check-* rebuilds the archive from the snapshot
# with that compiler, requires no undefined symbols, and runs the hosted smoke.
HOST_SNAPSHOT ?= $(OUT)/host-snapshot
host-snapshot: tools/host_snapshot.sh | $(OUT)
	sh tools/host_snapshot.sh '$(NELUA)' $(HOST_SNAPSHOT)

define host-snapshot-check
	$(MAKE) -C $(HOST_SNAPSHOT) CC='$(2)' AR='$(AR)' OBJCOPY='$(OBJCOPY)' O=obj-$(1) all
	$(2) -nostdlib -r -Wl,--whole-archive $(HOST_SNAPSHOT)/obj-$(1)/libnelibc-host.a -Wl,--no-whole-archive -o $(OUT)/host-snapshot-linked-$(1).o
	@test -z "$$($(NM) -u $(OUT)/host-snapshot-linked-$(1).o | grep -v _GLOBAL_OFFSET_TABLE_)" || { $(NM) -u $(OUT)/host-snapshot-linked-$(1).o; exit 1; }
	$(call host-syscall-scan,$(OUT)/host-snapshot-linked-$(1).o)
	$(2) $(HOST_LINK_FLAGS) $(3) tools/host_smoke.c tools/host_linux.c $(HOST_SNAPSHOT)/obj-$(1)/libnelibc-host.a -o $(OUT)/host-smoke-snapshot-$(1)
	@test -z "$$($(NM) -u $(OUT)/host-smoke-snapshot-$(1))"
	@$(OUT)/host-smoke-snapshot-$(1) > $(OUT)/host-smoke-snapshot-$(1).log 2>&1; status=$$?; cat $(OUT)/host-smoke-snapshot-$(1).log; test $$status -eq 7 || { echo "host snapshot smoke ($(1)) exited $$status"; exit 1; }
	@grep -q '^host smoke: OK' $(OUT)/host-smoke-snapshot-$(1).log && grep -q '^atexit handler ran' $(OUT)/host-smoke-snapshot-$(1).log
	@echo 'host snapshot ($(1)): archive built from generated C, no undefined symbols, hosted smoke passed'
endef

host-snapshot-check-zig: host-snapshot tools/host_smoke.c tools/host_linux.c
	$(call host-snapshot-check,zig,$(ZIG) cc $(ZIGFLAGS),)
host-snapshot-check-gcc: host-snapshot tools/host_smoke.c tools/host_linux.c
	$(call host-snapshot-check,gcc,gcc,-fno-tree-loop-distribute-patterns)

host-snapshot-matrix: host-snapshot-check-zig host-snapshot-check-gcc
