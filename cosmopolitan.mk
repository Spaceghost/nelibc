# x86-64 Cosmopolitan fallback. Never reuse glibc/musl-built objects.
COSMOCC ?= x86_64-unknown-cosmo-cc
COSMOAR ?= ar
READELF ?= readelf
COSMOFLAGS ?= -O2 -fno-builtin -fno-stack-protector -fno-tree-loop-distribute-patterns
COSMO_NELUA_CACHE ?= $(OUT)/cosmo/nelua-cache
COSMO_MODULES := string memory ctype stdlib_float inet math_core
COSMO_OBJECTS := $(addprefix $(OUT)/cosmo/,$(addsuffix .o,$(COSMO_MODULES))) \
 $(OUT)/cosmo/memory_x86_64.o $(OUT)/cosmo/math_core_x86_64.o $(OUT)/cosmo/cosmopolitan_adapter.o $(OUT)/cosmo/complex_abi.o

$(OUT)/cosmo:
	mkdir -p $@

$(COSMO_NELUA_CACHE):
	mkdir -p $@

$(OUT)/cosmo/%.o: src/%.nelua cosmopolitan.mk | $(OUT)/cosmo $(COSMO_NELUA_CACHE)
	$(NELUA) --cache-dir '$(COSMO_NELUA_CACHE)' -B -r --cc '$(COSMOCC)' --cflags='$(COSMOFLAGS)' -o $@ $<

$(OUT)/cosmo/%.o: src/%.S cosmopolitan.mk | $(OUT)/cosmo
	$(COSMOCC) -c $< -o $@

$(OUT)/cosmo/%.o: src/%.c cosmopolitan.mk | $(OUT)/cosmo
	$(COSMOCC) $(COSMOFLAGS) -c $< -o $@

$(OUT)/libnelibc-cosmo.a: $(COSMO_OBJECTS)
	rm -f $@
	$(COSMOAR) rcs $@ $^

lib-cosmo: $(OUT)/libnelibc-cosmo.a

$(OUT)/cosmo/smoke: tools/cosmopolitan_smoke.c $(OUT)/libnelibc-cosmo.a
	$(COSMOCC) $(COSMOFLAGS) $< $(OUT)/libnelibc-cosmo.a -Wl,-Map,$@.map -o $@

cosmo-link-check: $(OUT)/cosmo/smoke
	$<
	$(NM) -u $< > $<.undefined
	@test ! -s $<.undefined
	$(READELF) -l $< > $<.segments
	@! grep -q INTERP $<.segments
	$(READELF) -d $< > $<.dynamic
	@! grep -q NEEDED $<.dynamic
	$(NM) $< > $<.symbols
	@! grep -q GLIBC_ $<.symbols
	@grep -q 'libnelibc-cosmo.a(memory.o)' $<.map
	@grep -q 'libnelibc-cosmo.a(string.o)' $<.map
	@grep -q 'libnelibc-cosmo.a(math_core.o)' $<.map
	@grep -q 'libnelibc-cosmo.a(ctype.o)' $<.map
	@grep -q 'libnelibc-cosmo.a(stdlib_float.o)' $<.map
	@echo 'Cosmopolitan fallback: static link, Nelibc core selected, no glibc dependency'

.PHONY: lib-cosmo smoke-cosmo dogfood-cosmo cosmo-link-check
smoke-cosmo dogfood-cosmo: cosmo-link-check
smoke-cosmo: memory-diff-cosmo complex-math-smoke-cosmo

$(OUT)/cosmo/memory-diff: tools/memory_diff.c $(OUT)/libnelibc-cosmo.a
	$(COSMOCC) $(COSMOFLAGS) $< $(OUT)/libnelibc-cosmo.a -o $@

memory-diff-cosmo: $(OUT)/cosmo/memory-diff
	$<

$(OUT)/cosmo/complex-math-smoke: tools/complex_math_smoke.c $(OUT)/libnelibc-cosmo.a
	$(COSMOCC) $(COSMOFLAGS) $< $(OUT)/libnelibc-cosmo.a -o $@

complex-math-smoke-cosmo: $(OUT)/cosmo/complex-math-smoke
	$<

.PHONY: memory-diff-cosmo complex-math-smoke-cosmo

# Force every archive member into one consumer to check the complete fallback
# closure, including modules the ordinary consumer would not extract.
$(OUT)/cosmo/whole-smoke: tools/cosmopolitan_smoke.c $(OUT)/libnelibc-cosmo.a
	$(COSMOCC) $(COSMOFLAGS) $< -Wl,--whole-archive $(OUT)/libnelibc-cosmo.a -Wl,--no-whole-archive -o $@

whole-smoke-cosmo: $(OUT)/cosmo/whole-smoke
	$<

smoke-cosmo: whole-smoke-cosmo
.PHONY: whole-smoke-cosmo
