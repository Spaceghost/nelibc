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
# is the only definition the linker can pick.
define host-archive
	cp $< $@
	$(AR) d $@ start-linux-x86_64-standalone-$(1).o sys-linux-x86_64-standalone-$(1)-asm.o
	$(OBJCOPY) --strip-symbol=nelibc_raw_syscall3 --strip-symbol=nelibc_raw_syscall6 $(OUT)/sys-linux-x86_64-standalone-$(1)-asm.o $(OUT)/sys-linux-x86_64-host-$(1)-asm.o
	$(AR) rs $@ $(OUT)/sys-linux-x86_64-host-$(1)-asm.o $(OUT)/host-syscall-$(1).o $(OUT)/host-boot-$(1).o
endef

$(OUT)/libnelibc-host-zig.a: $(OUT)/libnelibc-standalone-zig.a $(OUT)/host-syscall-zig.o $(OUT)/host-boot-zig.o
	$(call host-archive,zig)
$(OUT)/libnelibc-host-gcc.a: $(OUT)/libnelibc-standalone-gcc.a $(OUT)/host-syscall-gcc.o $(OUT)/host-boot-gcc.o
	$(call host-archive,gcc)

lib-host-zig: $(OUT)/libnelibc-host-zig.a
lib-host-gcc: $(OUT)/libnelibc-host-gcc.a

# Every symbol the host archive needs is inside it or in nelibc_host, and no
# `syscall` instruction remains reachable through the raw entry points.
host-check-zig: $(OUT)/libnelibc-host-zig.a
	$(ZIG) cc $(ZIGFLAGS) -nostdlib -r -Wl,--whole-archive $< -Wl,--no-whole-archive -o $(OUT)/host-linked-zig.o
	@test -z "$$($(NM) -u $(OUT)/host-linked-zig.o)" || { $(NM) -u $(OUT)/host-linked-zig.o; exit 1; }
	@echo 'host archive (zig): no undefined symbols'
host-check-gcc: $(OUT)/libnelibc-host-gcc.a
	gcc -nostdlib -r -Wl,--whole-archive $< -Wl,--no-whole-archive -o $(OUT)/host-linked-gcc.o
	@test -z "$$($(NM) -u $(OUT)/host-linked-gcc.o)" || { $(NM) -u $(OUT)/host-linked-gcc.o; exit 1; }
	@echo 'host archive (gcc): no undefined symbols'

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
