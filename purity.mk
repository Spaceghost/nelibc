# Purity is a compiler contract, so check optimized callers as well as results.
purity-check: | $(OUT)
	python3 tools/purity_check.py --cc gcc --out $(OUT)/purity-gcc.s
	python3 tools/purity_check.py --cc '$(ZIG) cc $(ZIGFLAGS)' --out $(OUT)/purity-zig.s
	$(NELUA) -b --cc gcc --cflags=-O2 -o $(OUT)/purity-order tools/purity_order.nelua
	$(OUT)/purity-order

strcmp-guard-gcc: $(OUT)/libnelibc-gcc.a
	gcc -O2 -fno-builtin -Iinclude tools/strcmp_guard_smoke.c $< -lm -pthread -o $(OUT)/strcmp-guard-gcc
	$(OUT)/strcmp-guard-gcc

strcmp-guard-zig-musl: $(OUT)/libnelibc-zig-musl.a
	$(ZIG) cc $(ZIGFLAGS) -target x86_64-linux-musl -static -O2 -fno-builtin -fno-sanitize=all $(MUSL_STDIO_WRAP_FLAGS) tools/strcmp_guard_smoke.c $< -lm -pthread -o $(OUT)/strcmp-guard-zig-musl
	$(OUT)/strcmp-guard-zig-musl

strcmp-guard-cosmo: $(OUT)/libnelibc-cosmo.a
	$(COSMOCC) $(COSMOFLAGS) tools/strcmp_guard_smoke.c $< -o $(OUT)/strcmp-guard-cosmo
	$(OUT)/strcmp-guard-cosmo

strlen-guard-gcc: $(OUT)/libnelibc-gcc.a
	gcc -O2 -fno-builtin tools/strlen_guard_smoke.c $< -lm -pthread -o $(OUT)/strlen-guard-gcc
	$(OUT)/strlen-guard-gcc

strlen-guard-zig-musl: $(OUT)/libnelibc-zig-musl.a
	$(ZIG) cc $(ZIGFLAGS) -target x86_64-linux-musl -static -O2 -fno-builtin -fno-sanitize=all $(MUSL_STDIO_WRAP_FLAGS) tools/strlen_guard_smoke.c $< -lm -pthread -o $(OUT)/strlen-guard-zig-musl
	$(OUT)/strlen-guard-zig-musl

strlen-guard-cosmo: $(OUT)/libnelibc-cosmo.a
	$(COSMOCC) $(COSMOFLAGS) tools/strlen_guard_smoke.c $< -o $(OUT)/strlen-guard-cosmo
	$(OUT)/strlen-guard-cosmo

.PHONY: purity-check strcmp-guard-gcc strcmp-guard-zig-musl strcmp-guard-cosmo
.PHONY: strlen-guard-gcc strlen-guard-zig-musl strlen-guard-cosmo
