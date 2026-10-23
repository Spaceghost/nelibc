NELUA ?= $(shell command -v nelua 2>/dev/null || printf '%s\n' "$$HOME/.local/nelua/bin/nelua")
ZIG ?= zig
TCC ?= tcc
TCC_LIBTCC1 ?= $(shell $(TCC) -print-search-dirs 2>/dev/null | sed -n 's/^install: //p')/libtcc1.a
AR ?= ar
OBJCOPY ?= objcopy
NM ?= nm
OUT ?= build
PATHCONF_PATHS ?= . /proc /dev/shm /boot
SOURCE := src/string.nelua
COLLATION_LOCALE ?= C
MATH_CORE_RENAMES := --redefine-sym fabs=nelibc_fabs --redefine-sym fabsf=nelibc_fabsf \
	--redefine-sym copysign=nelibc_copysign --redefine-sym copysignf=nelibc_copysignf \
	--redefine-sym trunc=nelibc_trunc --redefine-sym truncf=nelibc_truncf \
	--redefine-sym floor=nelibc_floor --redefine-sym floorf=nelibc_floorf \
	--redefine-sym ceil=nelibc_ceil --redefine-sym ceilf=nelibc_ceilf \
	--redefine-sym round=nelibc_round --redefine-sym roundf=nelibc_roundf \
	--redefine-sym rint=nelibc_rint --redefine-sym rintf=nelibc_rintf \
	--redefine-sym nearbyint=nelibc_nearbyint --redefine-sym nearbyintf=nelibc_nearbyintf \
	--redefine-sym exp=nelibc_exp --redefine-sym expf=nelibc_expf \
	--redefine-sym exp2=nelibc_exp2 --redefine-sym exp2f=nelibc_exp2f \
	--redefine-sym expm1=nelibc_expm1 --redefine-sym expm1f=nelibc_expm1f \
	--redefine-sym exp10=nelibc_exp10 --redefine-sym exp10f=nelibc_exp10f \
	--redefine-sym cbrt=nelibc_cbrt --redefine-sym cbrtf=nelibc_cbrtf \
	--redefine-sym pow=nelibc_pow --redefine-sym powf=nelibc_powf \
	--redefine-sym sinh=nelibc_sinh --redefine-sym sinhf=nelibc_sinhf \
	--redefine-sym cosh=nelibc_cosh --redefine-sym coshf=nelibc_coshf \
	--redefine-sym tanh=nelibc_tanh --redefine-sym tanhf=nelibc_tanhf \
	--redefine-sym asinh=nelibc_asinh --redefine-sym asinhf=nelibc_asinhf \
	--redefine-sym acosh=nelibc_acosh --redefine-sym acoshf=nelibc_acoshf \
	--redefine-sym atanh=nelibc_atanh --redefine-sym atanhf=nelibc_atanhf \
	--redefine-sym atan=nelibc_atan --redefine-sym atanf=nelibc_atanf \
	--redefine-sym atan2=nelibc_atan2 --redefine-sym atan2f=nelibc_atan2f \
	--redefine-sym log=nelibc_log --redefine-sym logf=nelibc_logf \
	--redefine-sym log2=nelibc_log2 --redefine-sym log2f=nelibc_log2f \
	--redefine-sym log1p=nelibc_log1p --redefine-sym log1pf=nelibc_log1pf \
	--redefine-sym log10=nelibc_log10 --redefine-sym log10f=nelibc_log10f \
	--redefine-sym sin=nelibc_sin --redefine-sym sinf=nelibc_sinf \
	--redefine-sym cos=nelibc_cos --redefine-sym cosf=nelibc_cosf \
	--redefine-sym tan=nelibc_tan --redefine-sym tanf=nelibc_tanf \
	--redefine-sym hypot=nelibc_hypot --redefine-sym hypotf=nelibc_hypotf \
	--redefine-sym sqrt=nelibc_sqrt --redefine-sym sqrtf=nelibc_sqrtf \
	--redefine-sym fmod=nelibc_fmod --redefine-sym fmodf=nelibc_fmodf \
	--redefine-sym remainder=nelibc_remainder --redefine-sym remainderf=nelibc_remainderf \
	--redefine-sym fmin=nelibc_fmin --redefine-sym fminf=nelibc_fminf \
	--redefine-sym fmax=nelibc_fmax --redefine-sym fmaxf=nelibc_fmaxf \
	--redefine-sym fdim=nelibc_fdim --redefine-sym fdimf=nelibc_fdimf \
	--redefine-sym frexp=nelibc_frexp --redefine-sym frexpf=nelibc_frexpf \
	--redefine-sym modf=nelibc_modf --redefine-sym modff=nelibc_modff \
	--redefine-sym ldexp=nelibc_ldexp --redefine-sym ldexpf=nelibc_ldexpf \
	--redefine-sym scalbn=nelibc_scalbn --redefine-sym scalbnf=nelibc_scalbnf \
	--redefine-sym ilogb=nelibc_ilogb --redefine-sym ilogbf=nelibc_ilogbf \
	--redefine-sym logb=nelibc_logb --redefine-sym logbf=nelibc_logbf \
	--redefine-sym nextafter=nelibc_nextafter --redefine-sym nextafterf=nelibc_nextafterf \
	--redefine-sym scalbln=nelibc_scalbln --redefine-sym scalblnf=nelibc_scalblnf \
	--redefine-sym nexttoward=nelibc_nexttoward --redefine-sym nexttowardf=nelibc_nexttowardf
MUSL_STDIO_WRAP_FLAGS ?= -Wl,-u,nelibc_hybrid_free -Wl,--wrap=fread -Wl,--wrap=fwrite -Wl,--wrap=fflush \
	-Wl,--wrap=fileno -Wl,--wrap=feof -Wl,--wrap=ferror -Wl,--wrap=clearerr \
	-Wl,--wrap=ungetc -Wl,--wrap=freopen -Wl,--wrap=fgetpos -Wl,--wrap=fsetpos \
	-Wl,--wrap=fseek -Wl,--wrap=ftell -Wl,--wrap=fseeko -Wl,--wrap=ftello \
	-Wl,--wrap=setvbuf -Wl,--wrap=flockfile -Wl,--wrap=ftrylockfile \
	-Wl,--wrap=funlockfile -Wl,--wrap=fclose -Wl,--wrap=localeconv \
	-Wl,--wrap=setlocale -Wl,--wrap=atexit -Wl,--wrap=exit -Wl,--wrap=free -Wl,--wrap=realloc

.PHONY: $(shell awk -F: '/^[[:alnum:]_.-]+:/ { print $$1 }' $(MAKEFILE_LIST))

all: smoke-zig

$(OUT):
	mkdir -p $@

$(OUT)/math-core-zig.o: src/math_core.nelua | $(OUT)
	$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-fno-builtin -o $@ $<

$(OUT)/math-core-zig-musl.o: src/math_core.nelua | $(OUT)
	$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-target=x86_64-linux-musl --cflags=-fno-builtin -o $@ $<

$(OUT)/math-core-gcc.o: src/math_core.nelua | $(OUT)
	$(NELUA) -B -r --cc gcc --cflags=-fno-builtin -o $@ $<

$(OUT)/math-core-tcc.o: src/math_core.nelua | $(OUT)
	$(NELUA) -B -r --cc '$(TCC)' --cflags=-fno-builtin -o $@ $<

$(OUT)/math-core-x86_64.o: src/math_core_x86_64.S | $(OUT)
	$(ZIG) cc -c $< -o $@

$(OUT)/libnelibc-zig.a: $(OUT)/math-core-zig.o $(OUT)/math-core-x86_64.o
$(OUT)/libnelibc-gcc.a: $(OUT)/math-core-gcc.o $(OUT)/math-core-x86_64.o
$(OUT)/libnelibc-zig-musl.a: $(OUT)/math-core-zig-musl.o $(OUT)/math-core-x86_64.o
$(OUT)/libnelibc-tcc.a: $(OUT)/math-core-tcc.o $(OUT)/math-core-x86_64.o
$(OUT)/libnelibc-standalone-zig.a: $(OUT)/math-core-zig.o $(OUT)/math-core-x86_64.o
$(OUT)/libnelibc-standalone-gcc.a: $(OUT)/math-core-gcc.o $(OUT)/math-core-x86_64.o
$(OUT)/libnelibc-standalone-tcc.a: $(OUT)/math-core-tcc.o $(OUT)/math-core-x86_64.o

math-core-diff-zig: $(OUT)/math-core-zig.o $(OUT)/math-core-x86_64.o tools/math_core_diff.c
	cp $(OUT)/math-core-zig.o $(OUT)/math-core-zig-renamed.o
	$(OBJCOPY) $(MATH_CORE_RENAMES) $(OUT)/math-core-zig-renamed.o
	$(ZIG) cc -Iinclude -fno-builtin tools/math_core_diff.c $(OUT)/math-core-zig-renamed.o $(OUT)/math-core-x86_64.o -lm -o $(OUT)/math-core-diff-zig
	$(OUT)/math-core-diff-zig

math-core-diff-gcc: $(OUT)/math-core-gcc.o $(OUT)/math-core-x86_64.o tools/math_core_diff.c
	cp $(OUT)/math-core-gcc.o $(OUT)/math-core-gcc-renamed.o
	$(OBJCOPY) $(MATH_CORE_RENAMES) $(OUT)/math-core-gcc-renamed.o
	gcc -Iinclude -fno-builtin tools/math_core_diff.c $(OUT)/math-core-gcc-renamed.o $(OUT)/math-core-x86_64.o -lm -o $(OUT)/math-core-diff-gcc
	$(OUT)/math-core-diff-gcc

math-core-diff-zig-musl: $(OUT)/math-core-zig-musl.o $(OUT)/math-core-x86_64.o tools/math_core_diff.c
	cp $(OUT)/math-core-zig-musl.o $(OUT)/math-core-zig-musl-renamed.o
	$(OBJCOPY) $(MATH_CORE_RENAMES) $(OUT)/math-core-zig-musl-renamed.o
	$(ZIG) cc -target x86_64-linux-musl -static -Iinclude -fno-builtin tools/math_core_diff.c $(OUT)/math-core-zig-musl-renamed.o $(OUT)/math-core-x86_64.o -lm -o $(OUT)/math-core-diff-zig-musl
	$(OUT)/math-core-diff-zig-musl

math-core-diff-tcc: $(OUT)/math-core-tcc.o $(OUT)/math-core-x86_64.o tools/math_core_diff.c
	$(TCC) -Iinclude -fno-builtin -c tools/math_core_diff.c -o $(OUT)/math-core-diff-tcc.o
	cp $(OUT)/math-core-tcc.o $(OUT)/math-core-tcc-renamed.o
	$(OBJCOPY) $(MATH_CORE_RENAMES) $(OUT)/math-core-tcc-renamed.o
	$(ZIG) cc -Iinclude -fno-builtin $(OUT)/math-core-diff-tcc.o $(OUT)/math-core-tcc-renamed.o $(OUT)/math-core-x86_64.o $(TCC_LIBTCC1) -lm -o $(OUT)/math-core-diff-tcc
	$(OUT)/math-core-diff-tcc

math-core-diff-matrix: math-core-diff-zig math-core-diff-gcc math-core-diff-zig-musl math-core-diff-tcc

math-core-standalone-zig: $(OUT)/libnelibc-standalone-zig.a tools/math_core_standalone_smoke.c
	$(ZIG) cc -DNELIBC_STANDALONE -nostdlib -static -no-pie -fno-pie -fno-stack-protector -ffreestanding -fno-builtin -fno-sanitize=all -Wl,-e,_start -Wl,--build-id=none -Wl,--whole-archive $(OUT)/libnelibc-standalone-zig.a -Wl,--no-whole-archive -Iinclude tools/math_core_standalone_smoke.c -o $(OUT)/math-core-standalone-zig
	@test -z "$$($(NM) -u $(OUT)/math-core-standalone-zig)"
	$(OUT)/math-core-standalone-zig

math-core-standalone-gcc: $(OUT)/libnelibc-standalone-gcc.a tools/math_core_standalone_smoke.c
	gcc -DNELIBC_STANDALONE -nostdlib -static -no-pie -fno-pie -fno-stack-protector -ffreestanding -fno-builtin -fno-sanitize=all -fno-tree-loop-distribute-patterns -Wl,-e,_start -Wl,--build-id=none -Wl,--whole-archive $(OUT)/libnelibc-standalone-gcc.a -Wl,--no-whole-archive -Iinclude tools/math_core_standalone_smoke.c -o $(OUT)/math-core-standalone-gcc
	@test -z "$$($(NM) -u $(OUT)/math-core-standalone-gcc)"
	$(OUT)/math-core-standalone-gcc

math-core-standalone-tcc: $(OUT)/libnelibc-standalone-tcc.a tools/math_core_standalone_smoke.c
	$(TCC) -DNELIBC_STANDALONE -Iinclude -ffreestanding -fno-builtin -c tools/math_core_standalone_smoke.c -o $(OUT)/math-core-standalone-tcc.o
	$(ZIG) cc -nostdlib -static -no-pie -fno-pie -fno-stack-protector -ffreestanding -fno-builtin -fno-sanitize=all -Wl,-e,_start -Wl,--build-id=none -Wl,--whole-archive $(OUT)/libnelibc-standalone-tcc.a -Wl,--no-whole-archive $(OUT)/math-core-standalone-tcc.o -o $(OUT)/math-core-standalone-tcc
	@test -z "$$($(NM) -u $(OUT)/math-core-standalone-tcc)"
	$(OUT)/math-core-standalone-tcc

math-core-standalone-matrix: math-core-standalone-zig math-core-standalone-gcc math-core-standalone-tcc

.PHONY: math-core-diff-zig math-core-diff-gcc math-core-diff-zig-musl math-core-diff-tcc math-core-diff-matrix math-core-standalone-zig math-core-standalone-gcc math-core-standalone-tcc math-core-standalone-matrix

unistd-header-smoke: $(OUT)
	$(ZIG) cc -Iinclude -c tools/unistd_header_smoke.c -o $(OUT)/unistd-header-zig.o
	$(ZIG) cc -target x86_64-linux-musl -Iinclude -c tools/unistd_header_smoke.c -o $(OUT)/unistd-header-zig-musl.o
	gcc -Iinclude -c tools/unistd_header_smoke.c -o $(OUT)/unistd-header-gcc.o
	$(TCC) -Iinclude -c tools/unistd_header_smoke.c -o $(OUT)/unistd-header-tcc.o
	$(ZIG) cc -DNELIBC_STANDALONE -Iinclude -ffreestanding -c tools/unistd_header_smoke.c -o $(OUT)/unistd-header-standalone-zig.o
	gcc -DNELIBC_STANDALONE -Iinclude -ffreestanding -c tools/unistd_header_smoke.c -o $(OUT)/unistd-header-standalone-gcc.o

zig: $(OUT) $(OUT)/memory-x86_64-simd.o $(OUT)/string-locale-zig.o $(OUT)/string-locale-lock-zig.o $(OUT)/math-core-zig.o
	$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-fno-builtin -o $(OUT)/string-zig.o $(SOURCE)
	$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-fno-builtin -o $(OUT)/memory-zig.o src/memory.nelua
	$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-fno-builtin -o $(OUT)/ctype-zig.o src/ctype.nelua
	$(NELUA) -B -r --cc '$(ZIG) cc' --cflags='-fno-builtin -DNELIBC_QSORT_HOLE_SIFT' -o $(OUT)/stdlib-zig.o src/stdlib.nelua
	$(OBJCOPY) --localize-symbol=nelibc_test_qsort_depth_fallbacks $(OUT)/stdlib-zig.o
	$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-fno-builtin -o $(OUT)/stdlib-float-zig.o src/stdlib_float.nelua
	$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-fno-builtin -o $(OUT)/errno-host-zig.o src/errno/host.nelua
	$(ZIG) cc -Iinclude -fno-builtin tools/abi_smoke.c $(OUT)/string-locale-zig.o $(OUT)/string-locale-lock-zig.o $(OUT)/string-zig.o $(OUT)/memory-zig.o $(OUT)/memory-x86_64-simd.o $(OUT)/ctype-zig.o $(OUT)/stdlib-zig.o $(OUT)/stdlib-float-zig.o $(OUT)/errno-host-zig.o src/environment_adapter.c -o $(OUT)/abi-smoke-zig

smoke-zig: zig
	$(ZIG) cc -Iinclude -fno-builtin tools/memory_diff.c $(OUT)/memory-zig.o $(OUT)/memory-x86_64-simd.o -o $(OUT)/memory-diff-zig
	$(OUT)/memory-diff-zig
	$(OUT)/abi-smoke-zig

zig-musl: $(OUT) $(OUT)/memory-x86_64-simd.o $(OUT)/string-locale-zig-musl.o $(OUT)/string-locale-lock-zig-musl.o $(OUT)/math-core-zig-musl.o
	$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-target=x86_64-linux-musl --cflags=-fno-builtin -o $(OUT)/string-zig-musl.o $(SOURCE)
	$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-target=x86_64-linux-musl --cflags=-fno-builtin -o $(OUT)/memory-zig-musl.o src/memory.nelua
	$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-target=x86_64-linux-musl --cflags=-fno-builtin -o $(OUT)/ctype-zig-musl.o src/ctype.nelua
	$(NELUA) -B -r --cc '$(ZIG) cc' --cflags='-target x86_64-linux-musl -fno-builtin -DNELIBC_QSORT_HOLE_SIFT' -o $(OUT)/stdlib-zig-musl.o src/stdlib.nelua
	$(OBJCOPY) --localize-symbol=nelibc_test_qsort_depth_fallbacks $(OUT)/stdlib-zig-musl.o
	$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-target=x86_64-linux-musl --cflags=-fno-builtin -o $(OUT)/stdlib-float-zig-musl.o src/stdlib_float.nelua
	$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-target=x86_64-linux-musl --cflags=-fno-builtin -o $(OUT)/errno-host-zig-musl.o src/errno/host.nelua
	$(ZIG) cc -target x86_64-linux-musl -static -Iinclude -fno-builtin tools/abi_smoke.c $(OUT)/string-locale-zig-musl.o $(OUT)/string-locale-lock-zig-musl.o $(OUT)/string-zig-musl.o $(OUT)/memory-zig-musl.o $(OUT)/memory-x86_64-simd.o $(OUT)/ctype-zig-musl.o $(OUT)/stdlib-zig-musl.o $(OUT)/stdlib-float-zig-musl.o $(OUT)/errno-host-zig-musl.o src/environment_adapter.c -o $(OUT)/abi-smoke-zig-musl

smoke-zig-musl: zig-musl
	$(ZIG) cc -target x86_64-linux-musl -static -Iinclude -fno-builtin tools/memory_diff.c $(OUT)/memory-zig-musl.o $(OUT)/memory-x86_64-simd.o -o $(OUT)/memory-diff-zig-musl
	$(OUT)/memory-diff-zig-musl
	$(OUT)/abi-smoke-zig-musl

$(OUT)/memory-x86_64-simd.o: src/memory_x86_64.S | $(OUT)
	$(ZIG) cc -c $< -o $@

$(OUT)/memory-tcc-simd.o: src/memory_x86_64.S | $(OUT)
	$(ZIG) cc -c -DNELIBC_TCC_PUBLIC_MEMCPY $< -o $@

$(OUT)/string-locale-zig.o: src/string_locale.nelua | $(OUT)
	$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-fno-builtin -o $@ $<

$(OUT)/string-locale-zig-musl.o: src/string_locale.nelua | $(OUT)
	$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-target=x86_64-linux-musl --cflags=-fno-builtin -o $@ $<

$(OUT)/string-locale-gcc.o: src/string_locale.nelua | $(OUT)
	$(NELUA) -B -r --cc gcc --cflags=-fno-builtin -o $@ $<

$(OUT)/string-locale-musl.o: src/string_locale.nelua | $(OUT)
	$(NELUA) -B -r --cc musl-gcc --cflags=-fno-builtin -o $@ $<

$(OUT)/string-locale-tcc.o: src/string_locale.nelua | $(OUT)
	$(NELUA) -B -r --cc '$(TCC)' --cflags=-fno-builtin -o $@ $<

$(OUT)/string-locale-lock-zig.o: src/string_locale_lock.c | $(OUT)
	$(ZIG) cc -pthread -fno-builtin -fno-sanitize=all -c $< -o $@

$(OUT)/string-locale-lock-zig-musl.o: src/string_locale_lock.c | $(OUT)
	$(ZIG) cc -target x86_64-linux-musl -pthread -fno-builtin -fno-sanitize=all -c $< -o $@

$(OUT)/string-locale-lock-gcc.o: src/string_locale_lock.c | $(OUT)
	gcc -pthread -fno-builtin -c $< -o $@

$(OUT)/string-locale-lock-musl.o: src/string_locale_lock.c | $(OUT)
	musl-gcc -pthread -fno-builtin -c $< -o $@

$(OUT)/string-locale-lock-tcc.o: src/string_locale_lock.c | $(OUT)
	$(TCC) -pthread -fno-builtin -c $< -o $@

$(OUT)/memory-x86_64-sse2-baseline.o: src/memory_x86_64.S | $(OUT)
	$(ZIG) cc -c -DNELIBC_DISABLE_AVX2 $< -o $@

gcc: $(OUT) $(OUT)/memory-x86_64-simd.o $(OUT)/string-locale-gcc.o $(OUT)/string-locale-lock-gcc.o $(OUT)/math-core-gcc.o
	$(NELUA) -B -r --cc gcc --cflags=-fno-builtin -o $(OUT)/string-gcc.o $(SOURCE)
	$(NELUA) -B -r --cc gcc --cflags=-fno-builtin -o $(OUT)/memory-gcc.o src/memory.nelua
	$(NELUA) -B -r --cc gcc --cflags=-fno-builtin -o $(OUT)/ctype-gcc.o src/ctype.nelua
	$(NELUA) -B -r --cc gcc --cflags='-fno-builtin -DNELIBC_QSORT_GCC_INTROSORT' -o $(OUT)/stdlib-gcc.o src/stdlib.nelua
	$(OBJCOPY) --localize-symbol=nelibc_test_qsort_depth_fallbacks $(OUT)/stdlib-gcc.o
	$(NELUA) -B -r --cc gcc --cflags=-fno-builtin -o $(OUT)/stdlib-float-gcc.o src/stdlib_float.nelua
	$(NELUA) -B -r --cc gcc --cflags=-fno-builtin -o $(OUT)/errno-host-gcc.o src/errno/host.nelua
	gcc -Iinclude -fno-builtin tools/abi_smoke.c $(OUT)/string-locale-gcc.o $(OUT)/string-locale-lock-gcc.o $(OUT)/string-gcc.o $(OUT)/memory-gcc.o $(OUT)/memory-x86_64-simd.o $(OUT)/ctype-gcc.o $(OUT)/stdlib-gcc.o $(OUT)/stdlib-float-gcc.o $(OUT)/errno-host-gcc.o src/environment_adapter.c -o $(OUT)/abi-smoke-gcc

smoke-gcc: gcc
	gcc -Iinclude -fno-builtin tools/memory_diff.c $(OUT)/memory-gcc.o $(OUT)/memory-x86_64-simd.o -o $(OUT)/memory-diff-gcc
	$(OUT)/memory-diff-gcc
	$(OUT)/abi-smoke-gcc

float-diff-zig: zig
	$(ZIG) cc -Iinclude -fno-builtin tools/float_diff.c $(OUT)/memory-zig.o $(OUT)/memory-x86_64-simd.o $(OUT)/stdlib-float-zig.o $(OUT)/errno-host-zig.o -o $(OUT)/float-diff-zig
	$(OUT)/float-diff-zig

float-diff-gcc: gcc
	gcc -Iinclude -fno-builtin tools/float_diff.c $(OUT)/memory-gcc.o $(OUT)/memory-x86_64-simd.o $(OUT)/stdlib-float-gcc.o $(OUT)/errno-host-gcc.o -o $(OUT)/float-diff-gcc
	$(OUT)/float-diff-gcc

float-diff-zig-musl: zig-musl
	cp $(OUT)/stdlib-float-zig-musl.o $(OUT)/stdlib-float-zig-musl-diff.o
	$(OBJCOPY) --redefine-sym strtof=nelibc_strtof --redefine-sym strtod=nelibc_strtod --redefine-sym strtold=nelibc_strtold $(OUT)/stdlib-float-zig-musl-diff.o
	$(ZIG) cc -target x86_64-linux-musl -static -Iinclude -fno-builtin -DNELIBC_RENAMED_FLOAT tools/float_diff.c $(OUT)/memory-zig-musl.o $(OUT)/memory-x86_64-simd.o $(OUT)/stdlib-float-zig-musl-diff.o $(OUT)/errno-host-zig-musl.o -o $(OUT)/float-diff-zig-musl
	$(OUT)/float-diff-zig-musl

float-diff-tcc: tcc
	$(TCC) -Iinclude -fno-builtin tools/float_diff.c -o $(OUT)/float-diff-host-tcc
	$(TCC) -Iinclude -fno-builtin tools/float_diff.c $(OUT)/memory-tcc.o $(OUT)/memory-tcc-helper.o $(OUT)/memory-tcc-simd.o $(OUT)/stdlib-float-tcc.o $(OUT)/errno-host-tcc.o -o $(OUT)/float-diff-nelibc-tcc
	$(OUT)/float-diff-host-tcc > $(OUT)/float-diff-host-tcc.csv
	$(OUT)/float-diff-nelibc-tcc > $(OUT)/float-diff-nelibc-tcc.csv
	cmp $(OUT)/float-diff-host-tcc.csv $(OUT)/float-diff-nelibc-tcc.csv && echo 'floating-point conversion cases passed (TinyCC/glibc)'

.PHONY: float-diff-tcc
float-diff-matrix: float-diff-zig float-diff-gcc float-diff-zig-musl float-diff-tcc float-divide-precheck-matrix float-radix-smoke-matrix

float-radix-smoke-zig: zig
	$(ZIG) cc -Iinclude -fno-builtin tools/float_radix_smoke.c $(OUT)/memory-zig.o $(OUT)/memory-x86_64-simd.o $(OUT)/stdlib-float-zig.o $(OUT)/errno-host-zig.o -o $(OUT)/float-radix-smoke-zig
	$(OUT)/float-radix-smoke-zig

float-radix-smoke-gcc: gcc
	gcc -Iinclude -fno-builtin tools/float_radix_smoke.c $(OUT)/memory-gcc.o $(OUT)/memory-x86_64-simd.o $(OUT)/stdlib-float-gcc.o $(OUT)/errno-host-gcc.o -o $(OUT)/float-radix-smoke-gcc
	$(OUT)/float-radix-smoke-gcc

float-radix-smoke-zig-musl: zig-musl
	$(ZIG) cc -target x86_64-linux-musl -static -Iinclude -fno-builtin tools/float_radix_smoke.c $(OUT)/memory-zig-musl.o $(OUT)/memory-x86_64-simd.o $(OUT)/stdlib-float-zig-musl.o $(OUT)/errno-host-zig-musl.o -o $(OUT)/float-radix-smoke-zig-musl
	$(OUT)/float-radix-smoke-zig-musl

float-radix-smoke-tcc: tcc
	$(TCC) -Iinclude -fno-builtin tools/float_radix_smoke.c $(OUT)/memory-tcc.o $(OUT)/memory-tcc-helper.o $(OUT)/memory-tcc-simd.o $(OUT)/stdlib-float-tcc.o $(OUT)/errno-host-tcc.o -o $(OUT)/float-radix-smoke-tcc
	$(OUT)/float-radix-smoke-tcc

.PHONY: float-radix-smoke-zig float-radix-smoke-gcc float-radix-smoke-zig-musl float-radix-smoke-tcc float-radix-smoke-matrix
float-radix-smoke-matrix: float-radix-smoke-zig float-radix-smoke-gcc float-radix-smoke-zig-musl float-radix-smoke-tcc

$(OUT)/stdlib_float_divide_test.nelua: src/stdlib_float.nelua tools/gen_float_divide_precheck_test.py | $(OUT)
	python3 tools/gen_float_divide_precheck_test.py src/stdlib_float.nelua $@

float-divide-precheck-zig: zig $(OUT)/stdlib_float_divide_test.nelua
	$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-fno-builtin -o $(OUT)/stdlib-float-divide-test-zig.o $(OUT)/stdlib_float_divide_test.nelua
	$(ZIG) cc -Iinclude -fno-builtin tools/float_divide_precheck_test.c $(OUT)/stdlib-float-divide-test-zig.o $(OUT)/memory-zig.o $(OUT)/memory-x86_64-simd.o $(OUT)/errno-host-zig.o -o $(OUT)/float-divide-precheck-zig
	$(OUT)/float-divide-precheck-zig

float-divide-precheck-gcc: gcc $(OUT)/stdlib_float_divide_test.nelua
	$(NELUA) -B -r --cc gcc --cflags=-fno-builtin -o $(OUT)/stdlib-float-divide-test-gcc.o $(OUT)/stdlib_float_divide_test.nelua
	gcc -Iinclude -fno-builtin tools/float_divide_precheck_test.c $(OUT)/stdlib-float-divide-test-gcc.o $(OUT)/memory-gcc.o $(OUT)/memory-x86_64-simd.o $(OUT)/errno-host-gcc.o -o $(OUT)/float-divide-precheck-gcc
	$(OUT)/float-divide-precheck-gcc

float-divide-precheck-zig-musl: zig-musl $(OUT)/stdlib_float_divide_test.nelua
	$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-target=x86_64-linux-musl --cflags=-fno-builtin -o $(OUT)/stdlib-float-divide-test-zig-musl.o $(OUT)/stdlib_float_divide_test.nelua
	$(ZIG) cc -target x86_64-linux-musl -static -Iinclude -fno-builtin tools/float_divide_precheck_test.c $(OUT)/stdlib-float-divide-test-zig-musl.o $(OUT)/memory-zig-musl.o $(OUT)/memory-x86_64-simd.o $(OUT)/errno-host-zig-musl.o -o $(OUT)/float-divide-precheck-zig-musl
	$(OUT)/float-divide-precheck-zig-musl

float-divide-precheck-tcc: tcc $(OUT)/stdlib_float_divide_test.nelua
	$(NELUA) -B -r --cc '$(TCC)' --cflags=-fno-builtin -o $(OUT)/stdlib-float-divide-test-tcc.o $(OUT)/stdlib_float_divide_test.nelua
	$(TCC) -Iinclude -fno-builtin tools/float_divide_precheck_test.c $(OUT)/stdlib-float-divide-test-tcc.o $(OUT)/memory-tcc.o $(OUT)/memory-tcc-helper.o $(OUT)/memory-tcc-simd.o $(OUT)/errno-host-tcc.o -o $(OUT)/float-divide-precheck-tcc
	$(OUT)/float-divide-precheck-tcc

float-divide-precheck-matrix: float-divide-precheck-zig float-divide-precheck-gcc float-divide-precheck-zig-musl float-divide-precheck-tcc

.PHONY: float-divide-precheck-zig float-divide-precheck-gcc float-divide-precheck-zig-musl float-divide-precheck-tcc float-divide-precheck-matrix

$(OUT)/stdio-format-zig.o: Makefile src/stdio.nelua src/stdio_varargs.c src/stdio_lock_standalone.c src/stdio_format_host_adapter.c
	$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-fno-builtin -o $(OUT)/stdio-format-zig-raw.o src/stdio.nelua
	$(NM) -g --defined-only $(OUT)/stdio-format-zig-raw.o | awk '$$3 != "snprintf" && $$3 != "vsnprintf" && $$3 != "sprintf" && $$3 != "vsprintf" && $$3 != "dprintf" && $$3 != "vdprintf" && $$3 != "nelibc_vfprintf_host" { print $$3 }' > $(OUT)/stdio-format-zig-localize.txt
	$(OBJCOPY) --localize-symbols=$(OUT)/stdio-format-zig-localize.txt $(OUT)/stdio-format-zig-raw.o $@
	$(ZIG) cc -Iinclude -fno-builtin -fno-sanitize=all -c src/stdio_varargs.c -o $(OUT)/stdio-varargs-hybrid-zig.o
	$(ZIG) cc -Iinclude -fno-builtin -fno-sanitize=all -c src/stdio_lock_standalone.c -o $(OUT)/stdio-lock-hybrid-zig.o
	$(ZIG) cc -Iinclude -fno-builtin -fno-sanitize=all -c src/stdio_format_host_adapter.c -o $(OUT)/stdio-format-host-adapter-zig.o
	$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-fno-builtin -o $(OUT)/stdio-asprintf-hybrid-zig.o src/stdio_asprintf_hybrid.nelua

$(OUT)/strerror-r-hybrid-zig.o: src/strerror_r_hybrid_gnu.nelua
	$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-fno-builtin -o $@ $<

$(OUT)/libnelibc-zig.a: Makefile zig $(OUT)/stdio-format-zig.o src/environment_adapter.c src/stdio_host_adapter.c src/alloc_hybrid_adapter.c src/calloc_hybrid.nelua src/fcntl_adapter.c src/credential_adapter.c src/sem_clockwait_glibc.c src/stdio_hybrid.nelua src/alloc.nelua src/unistd.nelua src/sys/linux_x86_64.nelua src/sys/linux_x86_64.S src/stdio_asprintf_hybrid.nelua src/aligned_alloc_hybrid.nelua src/posix_memalign_hybrid.nelua src/atexit_hybrid.nelua src/rewind_hybrid.nelua src/fclose_hybrid.nelua src/fdopen_hybrid.nelua src/fopen_hybrid.nelua src/tmpfile_hybrid.nelua src/stdio_file_lock_hybrid.nelua src/stdio_seek_hybrid.nelua src/stdio_fpos_hybrid.nelua src/stdio_setvbuf_hybrid.nelua src/stdio_buffer_extras_hybrid.nelua src/open_memstream_hybrid.nelua src/fmemopen_hybrid.nelua src/strerror_r_hybrid_gnu.nelua $(OUT)/strerror-r-hybrid-zig.o
	$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-fno-builtin -o $(OUT)/alloc-zig.o src/alloc.nelua
	$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-fno-builtin -o $(OUT)/alloc-hybrid-exports-zig.o src/alloc_hybrid_exports.nelua
	$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-fno-builtin -o $(OUT)/calloc-hybrid-zig.o src/calloc_hybrid.nelua
	$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-fno-builtin -o $(OUT)/aligned-alloc-hybrid-zig.o src/aligned_alloc_hybrid.nelua
	$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-fno-builtin -o $(OUT)/posix-memalign-hybrid-zig.o src/posix_memalign_hybrid.nelua
	$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-fno-builtin -o $(OUT)/atexit-hybrid-zig.o src/atexit_hybrid.nelua
	$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-fno-builtin -o $(OUT)/rewind-hybrid-zig.o src/rewind_hybrid.nelua
	$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-fno-builtin -o $(OUT)/fdopen-hybrid-zig.o src/fdopen_hybrid.nelua
	$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-fno-builtin -o $(OUT)/tmpfile-hybrid-zig.o src/tmpfile_hybrid.nelua
	$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-fno-builtin -o $(OUT)/stdio-seek-hybrid-zig.o src/stdio_seek_hybrid.nelua
	$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-fno-builtin -o $(OUT)/stdio-file-lock-hybrid-zig.o src/stdio_file_lock_hybrid.nelua
	$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-fno-builtin -o $(OUT)/stdio-fpos-hybrid-zig.o src/stdio_fpos_hybrid.nelua
	$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-fno-builtin -o $(OUT)/stdio-setvbuf-hybrid-zig.o src/stdio_setvbuf_hybrid.nelua
	$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-fno-builtin -o $(OUT)/stdio-buffer-extras-hybrid-zig.o src/stdio_buffer_extras_hybrid.nelua
	$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-fno-builtin -o $(OUT)/open-memstream-hybrid-zig.o src/open_memstream_hybrid.nelua
	$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-fno-builtin -o $(OUT)/fmemopen-hybrid-zig.o src/fmemopen_hybrid.nelua
	$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-fno-builtin -o $(OUT)/fopen-hybrid-zig.o src/fopen_hybrid.nelua
	$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-fno-builtin -o $(OUT)/unistd-zig.o src/unistd.nelua
	$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-fno-builtin -o $(OUT)/stdio-hybrid-zig.o src/stdio_hybrid.nelua
	$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-fno-builtin -o $(OUT)/fclose-hybrid-zig.o src/fclose_hybrid.nelua
	$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-fno-builtin -o $(OUT)/sys-linux-x86_64-zig.o src/sys/linux_x86_64.nelua
	$(ZIG) cc -c src/sys/linux_x86_64.S -o $(OUT)/sys-linux-x86_64-zig-asm.o
	$(ZIG) cc -Iinclude -fno-builtin -fno-sanitize=all -DNELIBC_WITH_UNISTD -DNELIBC_USE_GLIBC_CANCEL_WRAPPERS -DNELIBC_ALLOCATOR_FASTPATH_SAFE -c src/environment_adapter.c -o $(OUT)/environment-adapter-zig.o
	$(ZIG) cc -Iinclude -fno-builtin -fno-sanitize=all -c src/stdio_host_adapter.c -o $(OUT)/stdio-host-adapter-zig.o
	$(ZIG) cc -Iinclude -fno-builtin -c src/alloc_hybrid_adapter.c -o $(OUT)/alloc-hybrid-adapter-zig.o
	$(ZIG) cc -Iinclude -fno-builtin -c src/fcntl_adapter.c -o $(OUT)/fcntl-adapter-zig.o
	$(ZIG) cc -Iinclude -fno-builtin -c src/credential_adapter.c -o $(OUT)/credential-adapter-zig.o
	$(ZIG) cc -Iinclude -fno-builtin -c src/sem_clockwait_glibc.c -o $(OUT)/sem-clockwait-glibc-zig.o
	rm -f $@
	$(AR) rcs $@ $(OUT)/stdio-format-zig.o $(OUT)/stdio-varargs-hybrid-zig.o $(OUT)/stdio-lock-hybrid-zig.o $(OUT)/stdio-format-host-adapter-zig.o $(OUT)/stdio-asprintf-hybrid-zig.o $(OUT)/string-locale-zig.o $(OUT)/string-locale-lock-zig.o $(OUT)/string-zig.o $(OUT)/strerror-r-hybrid-zig.o $(OUT)/memory-zig.o $(OUT)/memory-x86_64-simd.o $(OUT)/ctype-zig.o $(OUT)/stdlib-zig.o $(OUT)/stdlib-float-zig.o $(OUT)/errno-host-zig.o $(OUT)/environment-adapter-zig.o $(OUT)/stdio-host-adapter-zig.o $(OUT)/alloc-hybrid-adapter-zig.o $(OUT)/alloc-hybrid-exports-zig.o $(OUT)/calloc-hybrid-zig.o $(OUT)/aligned-alloc-hybrid-zig.o $(OUT)/posix-memalign-hybrid-zig.o $(OUT)/atexit-hybrid-zig.o $(OUT)/rewind-hybrid-zig.o $(OUT)/fdopen-hybrid-zig.o $(OUT)/tmpfile-hybrid-zig.o $(OUT)/stdio-seek-hybrid-zig.o $(OUT)/stdio-file-lock-hybrid-zig.o $(OUT)/stdio-fpos-hybrid-zig.o $(OUT)/stdio-setvbuf-hybrid-zig.o $(OUT)/stdio-buffer-extras-hybrid-zig.o $(OUT)/open-memstream-hybrid-zig.o $(OUT)/fmemopen-hybrid-zig.o $(OUT)/fopen-hybrid-zig.o $(OUT)/fcntl-adapter-zig.o $(OUT)/credential-adapter-zig.o $(OUT)/sem-clockwait-glibc-zig.o $(OUT)/stdio-hybrid-zig.o $(OUT)/fclose-hybrid-zig.o $(OUT)/alloc-zig.o $(OUT)/unistd-zig.o $(OUT)/sys-linux-x86_64-zig.o $(OUT)/sys-linux-x86_64-zig-asm.o
	$(AR) rcs $@ $(OUT)/math-core-zig.o
	$(AR) rcs $@ $(OUT)/math-core-x86_64.o

$(OUT)/stdio-format-gcc.o: Makefile src/stdio.nelua src/stdio_varargs.c src/stdio_lock_standalone.c src/stdio_format_host_adapter.c
	$(NELUA) -B -r --cc gcc --cflags=-fno-builtin -o $(OUT)/stdio-format-gcc-raw.o src/stdio.nelua
	$(NM) -g --defined-only $(OUT)/stdio-format-gcc-raw.o | awk '$$3 != "snprintf" && $$3 != "vsnprintf" && $$3 != "sprintf" && $$3 != "vsprintf" && $$3 != "dprintf" && $$3 != "vdprintf" && $$3 != "nelibc_vfprintf_host" { print $$3 }' > $(OUT)/stdio-format-gcc-localize.txt
	$(OBJCOPY) --localize-symbols=$(OUT)/stdio-format-gcc-localize.txt $(OUT)/stdio-format-gcc-raw.o $@
	gcc -Iinclude -fno-builtin -fno-sanitize=all -c src/stdio_varargs.c -o $(OUT)/stdio-varargs-hybrid-gcc.o
	gcc -Iinclude -fno-builtin -fno-sanitize=all -c src/stdio_lock_standalone.c -o $(OUT)/stdio-lock-hybrid-gcc.o
	gcc -Iinclude -fno-builtin -fno-sanitize=all -c src/stdio_format_host_adapter.c -o $(OUT)/stdio-format-host-adapter-gcc.o
	$(NELUA) -B -r --cc 'gcc' --cflags=-fno-builtin -o $(OUT)/stdio-asprintf-hybrid-gcc.o src/stdio_asprintf_hybrid.nelua

$(OUT)/strerror-r-hybrid-gcc.o: src/strerror_r_hybrid_gnu.nelua
	$(NELUA) -B -r --cc gcc --cflags=-fno-builtin -o $@ $<

$(OUT)/libnelibc-gcc.a: Makefile gcc $(OUT)/stdio-format-gcc.o src/environment_adapter.c src/stdio_host_adapter.c src/alloc_hybrid_adapter.c src/calloc_hybrid.nelua src/fcntl_adapter.c src/credential_adapter.c src/sem_clockwait_glibc.c src/stdio_hybrid.nelua src/alloc.nelua src/unistd.nelua src/sys/linux_x86_64.nelua src/sys/linux_x86_64.S src/stdio_asprintf_hybrid.nelua src/aligned_alloc_hybrid.nelua src/posix_memalign_hybrid.nelua src/atexit_hybrid.nelua src/rewind_hybrid.nelua src/fclose_hybrid.nelua src/fdopen_hybrid.nelua src/fopen_hybrid.nelua src/tmpfile_hybrid.nelua src/stdio_file_lock_hybrid.nelua src/stdio_seek_hybrid.nelua src/stdio_fpos_hybrid.nelua src/stdio_setvbuf_hybrid.nelua src/stdio_buffer_extras_hybrid.nelua src/open_memstream_hybrid.nelua src/fmemopen_hybrid.nelua src/strerror_r_hybrid_gnu.nelua $(OUT)/strerror-r-hybrid-gcc.o
	$(NELUA) -B -r --cc gcc --cflags=-fno-builtin -o $(OUT)/alloc-gcc.o src/alloc.nelua
	$(NELUA) -B -r --cc gcc --cflags=-fno-builtin -o $(OUT)/alloc-hybrid-exports-gcc.o src/alloc_hybrid_exports.nelua
	$(NELUA) -B -r --cc gcc --cflags=-fno-builtin -o $(OUT)/calloc-hybrid-gcc.o src/calloc_hybrid.nelua
	$(NELUA) -B -r --cc gcc --cflags=-fno-builtin -o $(OUT)/aligned-alloc-hybrid-gcc.o src/aligned_alloc_hybrid.nelua
	$(NELUA) -B -r --cc gcc --cflags=-fno-builtin -o $(OUT)/posix-memalign-hybrid-gcc.o src/posix_memalign_hybrid.nelua
	$(NELUA) -B -r --cc gcc --cflags=-fno-builtin -o $(OUT)/atexit-hybrid-gcc.o src/atexit_hybrid.nelua
	$(NELUA) -B -r --cc gcc --cflags=-fno-builtin -o $(OUT)/rewind-hybrid-gcc.o src/rewind_hybrid.nelua
	$(NELUA) -B -r --cc 'gcc' --cflags=-fno-builtin -o $(OUT)/fdopen-hybrid-gcc.o src/fdopen_hybrid.nelua
	$(NELUA) -B -r --cc 'gcc' --cflags=-fno-builtin -o $(OUT)/tmpfile-hybrid-gcc.o src/tmpfile_hybrid.nelua
	$(NELUA) -B -r --cc 'gcc' --cflags=-fno-builtin -o $(OUT)/stdio-seek-hybrid-gcc.o src/stdio_seek_hybrid.nelua
	$(NELUA) -B -r --cc 'gcc' --cflags=-fno-builtin -o $(OUT)/stdio-file-lock-hybrid-gcc.o src/stdio_file_lock_hybrid.nelua
	$(NELUA) -B -r --cc 'gcc' --cflags=-fno-builtin -o $(OUT)/stdio-fpos-hybrid-gcc.o src/stdio_fpos_hybrid.nelua
	$(NELUA) -B -r --cc 'gcc' --cflags=-fno-builtin -o $(OUT)/stdio-setvbuf-hybrid-gcc.o src/stdio_setvbuf_hybrid.nelua
	$(NELUA) -B -r --cc 'gcc' --cflags=-fno-builtin -o $(OUT)/stdio-buffer-extras-hybrid-gcc.o src/stdio_buffer_extras_hybrid.nelua
	$(NELUA) -B -r --cc 'gcc' --cflags=-fno-builtin -o $(OUT)/open-memstream-hybrid-gcc.o src/open_memstream_hybrid.nelua
	$(NELUA) -B -r --cc 'gcc' --cflags=-fno-builtin -o $(OUT)/fmemopen-hybrid-gcc.o src/fmemopen_hybrid.nelua
	$(NELUA) -B -r --cc 'gcc' --cflags=-fno-builtin -o $(OUT)/fopen-hybrid-gcc.o src/fopen_hybrid.nelua
	$(NELUA) -B -r --cc gcc --cflags=-fno-builtin -o $(OUT)/unistd-gcc.o src/unistd.nelua
	$(NELUA) -B -r --cc gcc --cflags=-fno-builtin -o $(OUT)/stdio-hybrid-gcc.o src/stdio_hybrid.nelua
	$(NELUA) -B -r --cc gcc --cflags=-fno-builtin -o $(OUT)/fclose-hybrid-gcc.o src/fclose_hybrid.nelua
	$(NELUA) -B -r --cc gcc --cflags=-fno-builtin -o $(OUT)/sys-linux-x86_64-gcc.o src/sys/linux_x86_64.nelua
	gcc -c src/sys/linux_x86_64.S -o $(OUT)/sys-linux-x86_64-gcc-asm.o
	gcc -Iinclude -fno-builtin -fno-sanitize=all -DNELIBC_WITH_UNISTD -DNELIBC_USE_GLIBC_CANCEL_WRAPPERS -DNELIBC_ALLOCATOR_FASTPATH_SAFE -c src/environment_adapter.c -o $(OUT)/environment-adapter-gcc.o
	gcc -Iinclude -fno-builtin -fno-sanitize=all -c src/stdio_host_adapter.c -o $(OUT)/stdio-host-adapter-gcc.o
	gcc -Iinclude -fno-builtin -c src/alloc_hybrid_adapter.c -o $(OUT)/alloc-hybrid-adapter-gcc.o
	gcc -Iinclude -fno-builtin -c src/fcntl_adapter.c -o $(OUT)/fcntl-adapter-gcc.o
	gcc -Iinclude -fno-builtin -c src/credential_adapter.c -o $(OUT)/credential-adapter-gcc.o
	gcc -Iinclude -fno-builtin -c src/sem_clockwait_glibc.c -o $(OUT)/sem-clockwait-glibc-gcc.o
	rm -f $@
	$(AR) rcs $@ $(OUT)/stdio-format-gcc.o $(OUT)/stdio-varargs-hybrid-gcc.o $(OUT)/stdio-lock-hybrid-gcc.o $(OUT)/stdio-format-host-adapter-gcc.o $(OUT)/stdio-asprintf-hybrid-gcc.o $(OUT)/string-locale-gcc.o $(OUT)/string-locale-lock-gcc.o $(OUT)/string-gcc.o $(OUT)/strerror-r-hybrid-gcc.o $(OUT)/memory-gcc.o $(OUT)/memory-x86_64-simd.o $(OUT)/ctype-gcc.o $(OUT)/stdlib-gcc.o $(OUT)/stdlib-float-gcc.o $(OUT)/errno-host-gcc.o $(OUT)/environment-adapter-gcc.o $(OUT)/stdio-host-adapter-gcc.o $(OUT)/alloc-hybrid-adapter-gcc.o $(OUT)/alloc-hybrid-exports-gcc.o $(OUT)/calloc-hybrid-gcc.o $(OUT)/aligned-alloc-hybrid-gcc.o $(OUT)/posix-memalign-hybrid-gcc.o $(OUT)/atexit-hybrid-gcc.o $(OUT)/rewind-hybrid-gcc.o $(OUT)/fdopen-hybrid-gcc.o $(OUT)/tmpfile-hybrid-gcc.o $(OUT)/stdio-seek-hybrid-gcc.o $(OUT)/stdio-file-lock-hybrid-gcc.o $(OUT)/stdio-fpos-hybrid-gcc.o $(OUT)/stdio-setvbuf-hybrid-gcc.o $(OUT)/stdio-buffer-extras-hybrid-gcc.o $(OUT)/open-memstream-hybrid-gcc.o $(OUT)/fmemopen-hybrid-gcc.o $(OUT)/fopen-hybrid-gcc.o $(OUT)/fcntl-adapter-gcc.o $(OUT)/credential-adapter-gcc.o $(OUT)/sem-clockwait-glibc-gcc.o $(OUT)/stdio-hybrid-gcc.o $(OUT)/fclose-hybrid-gcc.o $(OUT)/alloc-gcc.o $(OUT)/unistd-gcc.o $(OUT)/sys-linux-x86_64-gcc.o $(OUT)/sys-linux-x86_64-gcc-asm.o
	$(AR) rcs $@ $(OUT)/math-core-gcc.o
	$(AR) rcs $@ $(OUT)/math-core-x86_64.o

$(OUT)/stdio-format-zig-musl.o: Makefile src/stdio.nelua src/stdio_varargs.c src/stdio_lock_standalone.c src/stdio_format_host_adapter.c
	$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-target=x86_64-linux-musl --cflags=-fno-builtin -o $(OUT)/stdio-format-zig-musl-raw.o src/stdio.nelua
	$(NM) -g --defined-only $(OUT)/stdio-format-zig-musl-raw.o | awk '$$3 != "snprintf" && $$3 != "vsnprintf" && $$3 != "sprintf" && $$3 != "vsprintf" && $$3 != "dprintf" && $$3 != "vdprintf" && $$3 != "nelibc_vfprintf_host" { print $$3 }' > $(OUT)/stdio-format-zig-musl-localize.txt
	$(OBJCOPY) --localize-symbols=$(OUT)/stdio-format-zig-musl-localize.txt $(OUT)/stdio-format-zig-musl-raw.o $@
	$(ZIG) cc -target x86_64-linux-musl -Iinclude -fno-builtin -fno-sanitize=all -c src/stdio_varargs.c -o $(OUT)/stdio-varargs-hybrid-zig-musl.o
	$(ZIG) cc -target x86_64-linux-musl -Iinclude -fno-builtin -fno-sanitize=all -c src/stdio_lock_standalone.c -o $(OUT)/stdio-lock-hybrid-zig-musl.o
	$(ZIG) cc -target x86_64-linux-musl -Iinclude -fno-builtin -fno-sanitize=all -c src/stdio_format_host_adapter.c -o $(OUT)/stdio-format-host-adapter-zig-musl.o
	$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-target=x86_64-linux-musl --cflags=-fno-builtin -o $(OUT)/stdio-asprintf-hybrid-zig-musl.o src/stdio_asprintf_hybrid.nelua

$(OUT)/strerror-r-hybrid-zig-musl.o: src/strerror_r_hybrid_posix.nelua
	$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-target=x86_64-linux-musl --cflags=-fno-builtin -o $@ $<

$(OUT)/malloc-hybrid-zig-musl.o: src/malloc_hybrid_export.nelua
	$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-target=x86_64-linux-musl --cflags=-fno-builtin -o $@ $<

$(OUT)/libnelibc-zig-musl.a: Makefile zig-musl $(OUT)/malloc-hybrid-zig-musl.o $(OUT)/stdio-format-zig-musl.o src/environment_adapter.c src/stdio_host_adapter.c src/alloc_hybrid_adapter.c src/calloc_hybrid.nelua src/fcntl_adapter.c src/credential_adapter.c src/sem_clockwait_musl.c src/stdio_hybrid_musl.nelua src/atexit_musl_hybrid.nelua src/alloc.nelua src/unistd.nelua src/sys/linux_x86_64.nelua src/sys/linux_x86_64.S src/stdio_asprintf_hybrid.nelua src/aligned_alloc_hybrid.nelua src/posix_memalign_hybrid.nelua src/rewind_hybrid.nelua src/fdopen_hybrid.nelua src/stdio_setbuf_musl_hybrid.nelua src/stdio_buffer_extras_hybrid.nelua src/open_memstream_hybrid.nelua src/fmemopen_hybrid.nelua src/strerror_r_hybrid_posix.nelua $(OUT)/strerror-r-hybrid-zig-musl.o
	$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-target=x86_64-linux-musl --cflags=-fno-builtin -o $(OUT)/alloc-zig-musl.o src/alloc.nelua
	$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-target=x86_64-linux-musl --cflags=-fno-builtin -o $(OUT)/calloc-hybrid-zig-musl.o src/calloc_hybrid.nelua
	$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-target=x86_64-linux-musl --cflags=-fno-builtin -o $(OUT)/aligned-alloc-hybrid-zig-musl.o src/aligned_alloc_hybrid.nelua
	$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-target=x86_64-linux-musl --cflags=-fno-builtin -o $(OUT)/posix-memalign-hybrid-zig-musl.o src/posix_memalign_hybrid.nelua
	$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-target=x86_64-linux-musl --cflags=-fno-builtin -o $(OUT)/rewind-hybrid-zig-musl.o src/rewind_hybrid.nelua
	$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-target=x86_64-linux-musl --cflags=-fno-builtin -o $(OUT)/fdopen-hybrid-zig-musl.o src/fdopen_hybrid.nelua
	$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-target=x86_64-linux-musl --cflags=-fno-builtin -o $(OUT)/tmpfile-hybrid-zig-musl.o src/tmpfile_hybrid.nelua
	$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-target=x86_64-linux-musl --cflags=-fno-builtin -o $(OUT)/stdio-setbuf-musl-hybrid-zig-musl.o src/stdio_setbuf_musl_hybrid.nelua
	$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-target=x86_64-linux-musl --cflags=-fno-builtin -o $(OUT)/stdio-buffer-extras-hybrid-zig-musl.o src/stdio_buffer_extras_hybrid.nelua
	$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-target=x86_64-linux-musl --cflags=-fno-builtin -o $(OUT)/open-memstream-hybrid-zig-musl.o src/open_memstream_hybrid.nelua
	$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-target=x86_64-linux-musl --cflags=-fno-builtin -o $(OUT)/fmemopen-hybrid-zig-musl.o src/fmemopen_hybrid.nelua
	$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-target=x86_64-linux-musl --cflags=-fno-builtin -o $(OUT)/fopen-hybrid-zig-musl.o src/fopen_hybrid.nelua
	$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-target=x86_64-linux-musl --cflags=-fno-builtin -o $(OUT)/unistd-zig-musl.o src/unistd.nelua
	$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-target=x86_64-linux-musl --cflags=-fno-builtin -o $(OUT)/stdio-hybrid-zig-musl.o src/stdio_hybrid_musl.nelua
	$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-target=x86_64-linux-musl --cflags=-fno-builtin -o $(OUT)/atexit-musl-hybrid-zig-musl.o src/atexit_musl_hybrid.nelua
	$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-target=x86_64-linux-musl --cflags=-fno-builtin -o $(OUT)/sys-linux-x86_64-zig-musl.o src/sys/linux_x86_64.nelua
	$(ZIG) cc -target x86_64-linux-musl -c src/sys/linux_x86_64.S -o $(OUT)/sys-linux-x86_64-zig-musl-asm.o
	$(ZIG) cc -target x86_64-linux-musl -Iinclude -fno-builtin -fno-sanitize=all -DNELIBC_WITH_UNISTD -DNELIBC_USE_MUSL_SYSCALL_CP -DNELIBC_ALLOCATOR_FASTPATH_SAFE -c src/environment_adapter.c -o $(OUT)/environment-adapter-zig-musl.o
	$(ZIG) cc -target x86_64-linux-musl -Iinclude -fno-builtin -fno-sanitize=all -c src/stdio_host_adapter.c -o $(OUT)/stdio-host-adapter-zig-musl.o
	$(ZIG) cc -target x86_64-linux-musl -Iinclude -fno-builtin -DNELIBC_STATIC_MUSL -c src/alloc_hybrid_adapter.c -o $(OUT)/alloc-hybrid-adapter-zig-musl.o
	$(ZIG) cc -target x86_64-linux-musl -Iinclude -fno-builtin -c src/fcntl_adapter.c -o $(OUT)/fcntl-adapter-zig-musl.o
	$(ZIG) cc -target x86_64-linux-musl -Iinclude -fno-builtin -DNELIBC_STATIC_MUSL -c src/credential_adapter.c -o $(OUT)/credential-adapter-zig-musl.o
	$(ZIG) cc -target x86_64-linux-musl -Iinclude -fno-builtin -c src/sem_clockwait_musl.c -o $(OUT)/sem-clockwait-musl-zig-musl.o
	rm -f $@
	$(AR) rcs $@ $(OUT)/malloc-hybrid-zig-musl.o $(OUT)/stdio-format-zig-musl.o $(OUT)/stdio-varargs-hybrid-zig-musl.o $(OUT)/stdio-lock-hybrid-zig-musl.o $(OUT)/stdio-format-host-adapter-zig-musl.o $(OUT)/stdio-asprintf-hybrid-zig-musl.o $(OUT)/string-locale-zig-musl.o $(OUT)/string-locale-lock-zig-musl.o $(OUT)/string-zig-musl.o $(OUT)/strerror-r-hybrid-zig-musl.o $(OUT)/memory-zig-musl.o $(OUT)/memory-x86_64-simd.o $(OUT)/ctype-zig-musl.o $(OUT)/stdlib-zig-musl.o $(OUT)/stdlib-float-zig-musl.o $(OUT)/errno-host-zig-musl.o $(OUT)/environment-adapter-zig-musl.o $(OUT)/stdio-host-adapter-zig-musl.o $(OUT)/alloc-hybrid-adapter-zig-musl.o $(OUT)/calloc-hybrid-zig-musl.o $(OUT)/aligned-alloc-hybrid-zig-musl.o $(OUT)/posix-memalign-hybrid-zig-musl.o $(OUT)/rewind-hybrid-zig-musl.o $(OUT)/fdopen-hybrid-zig-musl.o $(OUT)/tmpfile-hybrid-zig-musl.o $(OUT)/stdio-setbuf-musl-hybrid-zig-musl.o $(OUT)/stdio-buffer-extras-hybrid-zig-musl.o $(OUT)/open-memstream-hybrid-zig-musl.o $(OUT)/fmemopen-hybrid-zig-musl.o $(OUT)/fopen-hybrid-zig-musl.o $(OUT)/fcntl-adapter-zig-musl.o $(OUT)/credential-adapter-zig-musl.o $(OUT)/sem-clockwait-musl-zig-musl.o $(OUT)/stdio-hybrid-zig-musl.o $(OUT)/atexit-musl-hybrid-zig-musl.o $(OUT)/alloc-zig-musl.o $(OUT)/unistd-zig-musl.o $(OUT)/sys-linux-x86_64-zig-musl.o $(OUT)/sys-linux-x86_64-zig-musl-asm.o
	$(AR) rcs $@ $(OUT)/math-core-zig-musl.o
	$(AR) rcs $@ $(OUT)/math-core-x86_64.o

$(OUT)/stdio-format-tcc.o: Makefile src/stdio.nelua src/stdio_varargs.c src/stdio_lock_tcc.c src/stdio_format_host_adapter.c
	$(NELUA) --print-code -r src/stdio.nelua > $(OUT)/stdio-format-tcc-generated.c
	$(ZIG) cc -O2 -fno-builtin -Iinclude -fno-sanitize=all -c $(OUT)/stdio-format-tcc-generated.c -o $(OUT)/stdio-format-tcc-raw.o
	$(NM) -g --defined-only $(OUT)/stdio-format-tcc-raw.o | awk '$$3 != "snprintf" && $$3 != "vsnprintf" && $$3 != "sprintf" && $$3 != "vsprintf" && $$3 != "dprintf" && $$3 != "vdprintf" && $$3 != "nelibc_vfprintf_host" { print $$3 }' > $(OUT)/stdio-format-tcc-localize.txt
	$(OBJCOPY) --localize-symbols=$(OUT)/stdio-format-tcc-localize.txt $(OUT)/stdio-format-tcc-raw.o $@
	$(TCC) -Iinclude -fno-builtin -fno-sanitize=all -c src/stdio_varargs.c -o $(OUT)/stdio-varargs-hybrid-tcc.o
	$(TCC) -pthread -Iinclude -fno-builtin -fno-sanitize=all -c src/stdio_lock_tcc.c -o $(OUT)/stdio-lock-hybrid-tcc.o
	$(TCC) -Iinclude -fno-builtin -fno-sanitize=all -c src/stdio_format_host_adapter.c -o $(OUT)/stdio-format-host-adapter-tcc.o
	$(NELUA) -B -r --cc '$(TCC)' --cflags=-fno-builtin -o $(OUT)/stdio-asprintf-hybrid-tcc.o src/stdio_asprintf_hybrid.nelua

$(OUT)/strerror-r-hybrid-tcc.o: src/strerror_r_hybrid_gnu.nelua
	$(NELUA) -B -r --cc '$(TCC)' --cflags=-fno-builtin -o $@ $<

$(OUT)/libnelibc-tcc.a: Makefile tcc $(OUT)/stdio-format-tcc.o src/environment_adapter.c src/stdio_host_adapter.c src/alloc_hybrid_adapter.c src/calloc_hybrid.nelua src/fcntl_adapter.c src/credential_adapter.c src/sem_clockwait_glibc.c src/stdio_hybrid.nelua src/alloc.nelua src/unistd.nelua src/sys/linux_x86_64.nelua src/sys/linux_x86_64.S src/stdio_asprintf_hybrid.nelua src/aligned_alloc_hybrid.nelua src/posix_memalign_hybrid.nelua src/atexit_hybrid.nelua src/rewind_hybrid.nelua src/fclose_hybrid.nelua src/fdopen_hybrid.nelua src/fopen_hybrid.nelua src/tmpfile_hybrid.nelua src/stdio_file_lock_hybrid.nelua src/stdio_seek_hybrid.nelua src/stdio_fpos_hybrid.nelua src/stdio_setvbuf_hybrid.nelua src/stdio_buffer_extras_hybrid.nelua src/open_memstream_hybrid.nelua src/fmemopen_hybrid.nelua src/strerror_r_hybrid_gnu.nelua $(OUT)/strerror-r-hybrid-tcc.o
	$(NELUA) -B -r --cc '$(TCC)' --cflags=-fno-builtin -o $(OUT)/alloc-tcc.o src/alloc.nelua
	$(NELUA) -B -r --cc '$(TCC)' --cflags=-fno-builtin -o $(OUT)/alloc-hybrid-exports-tcc.o src/alloc_hybrid_exports.nelua
	$(NELUA) -B -r --cc '$(TCC)' --cflags=-fno-builtin -o $(OUT)/calloc-hybrid-tcc.o src/calloc_hybrid.nelua
	$(NELUA) -B -r --cc '$(TCC)' --cflags=-fno-builtin -o $(OUT)/aligned-alloc-hybrid-tcc.o src/aligned_alloc_hybrid.nelua
	$(NELUA) -B -r --cc '$(TCC)' --cflags=-fno-builtin -o $(OUT)/posix-memalign-hybrid-tcc.o src/posix_memalign_hybrid.nelua
	$(NELUA) -B -r --cc '$(TCC)' --cflags=-fno-builtin -o $(OUT)/atexit-hybrid-tcc.o src/atexit_hybrid.nelua
	$(NELUA) -B -r --cc '$(TCC)' --cflags=-fno-builtin -o $(OUT)/rewind-hybrid-tcc.o src/rewind_hybrid.nelua
	$(NELUA) -B -r --cc '$(TCC)' --cflags=-fno-builtin -o $(OUT)/fdopen-hybrid-tcc.o src/fdopen_hybrid.nelua
	$(NELUA) -B -r --cc '$(TCC)' --cflags=-fno-builtin -o $(OUT)/tmpfile-hybrid-tcc.o src/tmpfile_hybrid.nelua
	$(NELUA) -B -r --cc '$(TCC)' --cflags=-fno-builtin -o $(OUT)/stdio-seek-hybrid-tcc.o src/stdio_seek_hybrid.nelua
	$(NELUA) -B -r --cc '$(TCC)' --cflags=-fno-builtin -o $(OUT)/stdio-file-lock-hybrid-tcc.o src/stdio_file_lock_hybrid.nelua
	$(NELUA) -B -r --cc '$(TCC)' --cflags=-fno-builtin -o $(OUT)/stdio-fpos-hybrid-tcc.o src/stdio_fpos_hybrid.nelua
	$(NELUA) -B -r --cc '$(TCC)' --cflags=-fno-builtin -o $(OUT)/stdio-setvbuf-hybrid-tcc.o src/stdio_setvbuf_hybrid.nelua
	$(NELUA) -B -r --cc '$(TCC)' --cflags=-fno-builtin -o $(OUT)/stdio-buffer-extras-hybrid-tcc.o src/stdio_buffer_extras_hybrid.nelua
	$(NELUA) -B -r --cc '$(TCC)' --cflags=-fno-builtin -o $(OUT)/open-memstream-hybrid-tcc.o src/open_memstream_hybrid.nelua
	$(NELUA) -B -r --cc '$(TCC)' --cflags=-fno-builtin -o $(OUT)/fmemopen-hybrid-tcc.o src/fmemopen_hybrid.nelua
	$(NELUA) -B -r --cc '$(TCC)' --cflags=-fno-builtin -o $(OUT)/fopen-hybrid-tcc.o src/fopen_hybrid.nelua
	$(NELUA) -B -r --cc '$(TCC)' --cflags=-fno-builtin -o $(OUT)/unistd-tcc.o src/unistd.nelua
	$(NELUA) -B -r --cc '$(TCC)' --cflags=-fno-builtin -o $(OUT)/stdio-hybrid-tcc.o src/stdio_hybrid.nelua
	$(NELUA) -B -r --cc '$(TCC)' --cflags=-fno-builtin -o $(OUT)/fclose-hybrid-tcc.o src/fclose_hybrid.nelua
	$(NELUA) -B -r --cc '$(TCC)' --cflags=-fno-builtin -o $(OUT)/sys-linux-x86_64-tcc.o src/sys/linux_x86_64.nelua
	$(ZIG) cc -c src/sys/linux_x86_64.S -o $(OUT)/sys-linux-x86_64-tcc-asm.o
	# TinyCC lacks C TLS; compile this TLS-bearing adapter with Zig cc.
	$(ZIG) cc -pthread -Iinclude -fno-builtin -DNELIBC_WITH_UNISTD -DNELIBC_USE_GLIBC_CANCEL_WRAPPERS -DNELIBC_ALLOCATOR_FASTPATH_SAFE -c src/environment_adapter.c -o $(OUT)/environment-adapter-tcc.o
	$(TCC) -Iinclude -fno-builtin -c src/stdio_host_adapter.c -o $(OUT)/stdio-host-adapter-tcc.o
	$(TCC) -Iinclude -fno-builtin -c src/alloc_hybrid_adapter.c -o $(OUT)/alloc-hybrid-adapter-tcc.o
	$(TCC) -Iinclude -fno-builtin -c src/fcntl_adapter.c -o $(OUT)/fcntl-adapter-tcc.o
	$(TCC) -Iinclude -fno-builtin -c src/credential_adapter.c -o $(OUT)/credential-adapter-tcc.o
	$(TCC) -Iinclude -fno-builtin -c src/sem_clockwait_glibc.c -o $(OUT)/sem-clockwait-glibc-tcc.o
	rm -f $@
	$(AR) rcs $@ $(OUT)/stdio-format-tcc.o $(OUT)/stdio-varargs-hybrid-tcc.o $(OUT)/stdio-lock-hybrid-tcc.o $(OUT)/stdio-format-host-adapter-tcc.o $(OUT)/stdio-asprintf-hybrid-tcc.o $(OUT)/string-locale-tcc.o $(OUT)/string-locale-lock-tcc.o $(OUT)/string-tcc.o $(OUT)/strerror-r-hybrid-tcc.o $(OUT)/memory-tcc.o $(OUT)/memory-tcc-helper.o $(OUT)/memory-tcc-simd.o $(OUT)/ctype-tcc.o $(OUT)/stdlib-tcc.o $(OUT)/stdlib-float-tcc.o $(OUT)/errno-host-tcc.o $(OUT)/environment-adapter-tcc.o $(OUT)/stdio-host-adapter-tcc.o $(OUT)/alloc-hybrid-adapter-tcc.o $(OUT)/alloc-hybrid-exports-tcc.o $(OUT)/calloc-hybrid-tcc.o $(OUT)/aligned-alloc-hybrid-tcc.o $(OUT)/posix-memalign-hybrid-tcc.o $(OUT)/atexit-hybrid-tcc.o $(OUT)/rewind-hybrid-tcc.o $(OUT)/fdopen-hybrid-tcc.o $(OUT)/tmpfile-hybrid-tcc.o $(OUT)/stdio-seek-hybrid-tcc.o $(OUT)/stdio-file-lock-hybrid-tcc.o $(OUT)/stdio-fpos-hybrid-tcc.o $(OUT)/stdio-setvbuf-hybrid-tcc.o $(OUT)/stdio-buffer-extras-hybrid-tcc.o $(OUT)/open-memstream-hybrid-tcc.o $(OUT)/fmemopen-hybrid-tcc.o $(OUT)/fopen-hybrid-tcc.o $(OUT)/fcntl-adapter-tcc.o $(OUT)/credential-adapter-tcc.o $(OUT)/sem-clockwait-glibc-tcc.o $(OUT)/stdio-hybrid-tcc.o $(OUT)/fclose-hybrid-tcc.o $(OUT)/alloc-tcc.o $(OUT)/unistd-tcc.o $(OUT)/sys-linux-x86_64-tcc.o $(OUT)/sys-linux-x86_64-tcc-asm.o
	$(AR) rcs $@ $(OUT)/math-core-tcc.o
	$(AR) rcs $@ $(OUT)/math-core-x86_64.o

lib-zig: $(OUT)/libnelibc-zig.a
lib-gcc: $(OUT)/libnelibc-gcc.a
lib-zig-musl: $(OUT)/libnelibc-zig-musl.a
lib-tcc: $(OUT)/libnelibc-tcc.a

api-coverage: $(OUT)/libnelibc-zig.a $(OUT)/libnelibc-gcc.a \
	$(OUT)/libnelibc-zig-musl.a $(OUT)/libnelibc-tcc.a \
	$(OUT)/libnelibc-standalone-zig.a $(OUT)/libnelibc-standalone-gcc.a \
	$(OUT)/libnelibc-standalone-tcc.a lib-private-test-symbols \
	lib-private-test-symbols-tcc lib-private-test-symbols-standalone
	python3 tools/api_coverage.py --nm '$(NM)' --output $(OUT)/api-coverage.csv \
		--profile zig_glibc=$(OUT)/libnelibc-zig.a \
		--profile gcc_glibc=$(OUT)/libnelibc-gcc.a \
		--profile zig_musl=$(OUT)/libnelibc-zig-musl.a \
		--profile tcc_glibc=$(OUT)/libnelibc-tcc.a \
		--profile zig_standalone=$(OUT)/libnelibc-standalone-zig.a \
		--profile gcc_standalone=$(OUT)/libnelibc-standalone-gcc.a \
		--profile tcc_standalone=$(OUT)/libnelibc-standalone-tcc.a

.PHONY: lib-private-test-symbols
lib-private-test-symbols: $(OUT)/libnelibc-zig.a $(OUT)/libnelibc-gcc.a $(OUT)/libnelibc-zig-musl.a
	@for archive in $^; do \
	  for symbol in __nelibc_tz_file_signature __nelibc_set_localtime_path; do \
	    if $(NM) -g --defined-only "$$archive" | awk '{print $$NF}' | grep -Fxq "$$symbol"; then \
	      echo "$$archive exports test-only symbol $$symbol" >&2; exit 1; \
	    fi; \
	  done; \
	done

api-coverage-available: $(OUT)/libnelibc-zig.a $(OUT)/libnelibc-gcc.a $(OUT)/libnelibc-zig-musl.a lib-private-test-symbols
	python3 tools/api_coverage.py --nm '$(NM)' --output $(OUT)/api-coverage-available.csv \
		--profile zig_glibc=$(OUT)/libnelibc-zig.a \
		--profile gcc_glibc=$(OUT)/libnelibc-gcc.a \
		--profile zig_musl=$(OUT)/libnelibc-zig-musl.a

api-coverage-hybrid: $(OUT)/libnelibc-zig.a $(OUT)/libnelibc-gcc.a \
	$(OUT)/libnelibc-zig-musl.a $(OUT)/libnelibc-tcc.a lib-private-test-symbols-tcc
	python3 tools/api_coverage.py --nm '$(NM)' --output $(OUT)/api-coverage-hybrid.csv \
		--profile zig_glibc=$(OUT)/libnelibc-zig.a \
		--profile gcc_glibc=$(OUT)/libnelibc-gcc.a \
		--profile zig_musl=$(OUT)/libnelibc-zig-musl.a \
		--profile tcc_glibc=$(OUT)/libnelibc-tcc.a

.PHONY: lib-private-test-symbols-tcc
lib-private-test-symbols-tcc: $(OUT)/libnelibc-tcc.a
	@for symbol in __nelibc_tz_file_signature __nelibc_set_localtime_path; do \
	  if $(NM) -g --defined-only "$<" | awk '{print $$NF}' | grep -Fxq "$$symbol"; then \
	    echo "$< exports test-only symbol $$symbol" >&2; exit 1; \
	  fi; \
	done

.PHONY: lib-private-test-symbols-standalone
lib-private-test-symbols-standalone: $(OUT)/libnelibc-standalone-zig.a $(OUT)/libnelibc-standalone-gcc.a $(OUT)/libnelibc-standalone-tcc.a
	@for archive in $^; do \
	  for symbol in __nelibc_tz_file_signature __nelibc_set_localtime_path; do \
	    if $(NM) -g --defined-only "$$archive" | awk '{print $$NF}' | grep -Fxq "$$symbol"; then \
	      echo "$$archive exports test-only symbol $$symbol" >&2; exit 1; \
	    fi; \
	  done; \
	done

.PHONY: lib-adapter-symbols
lib-adapter-symbols: $(OUT)/libnelibc-zig.a $(OUT)/libnelibc-gcc.a $(OUT)/libnelibc-zig-musl.a $(OUT)/libnelibc-tcc.a
	@for archive in $^; do \
	  for symbol in fcntl mremap _Exit reallocarray nelibc_allocator_owns nelibc_hybrid_free nelibc_hybrid_realloc setuid seteuid setgid setegid setreuid setregid setresuid setresgid setgroups; do \
	    nm -g --defined-only "$$archive" | awk '{print $$3}' | grep -Fxq "$$symbol" || { echo "$$archive is missing $$symbol" >&2; exit 1; }; \
	  done; \
	done

lib-smoke-zig: $(OUT)/libnelibc-zig.a
	$(ZIG) cc -pthread -Iinclude -fno-builtin tools/abi_smoke.c $(OUT)/libnelibc-zig.a -o $(OUT)/lib-smoke-zig
	NELIBC_GETENV_PROBE=visible $(OUT)/lib-smoke-zig

lib-smoke-gcc: $(OUT)/libnelibc-gcc.a
	gcc -pthread -Iinclude -fno-builtin tools/abi_smoke.c $(OUT)/libnelibc-gcc.a -o $(OUT)/lib-smoke-gcc
	NELIBC_GETENV_PROBE=visible $(OUT)/lib-smoke-gcc

lib-smoke-zig-musl: $(OUT)/libnelibc-zig-musl.a
	$(ZIG) cc -target x86_64-linux-musl -static -pthread -Iinclude -fno-builtin $(MUSL_STDIO_WRAP_FLAGS) tools/abi_smoke.c $(OUT)/libnelibc-zig-musl.a -o $(OUT)/lib-smoke-zig-musl
	NELIBC_GETENV_PROBE=visible $(OUT)/lib-smoke-zig-musl

lib-smoke-tcc: $(OUT)/libnelibc-tcc.a
	$(TCC) -pthread -Iinclude -fno-builtin -c tools/abi_smoke.c -o $(OUT)/lib-smoke-tcc.o
	$(ZIG) cc -pthread -Iinclude -fno-builtin $(OUT)/lib-smoke-tcc.o $(OUT)/libnelibc-tcc.a $(TCC_LIBTCC1) -o $(OUT)/lib-smoke-tcc
	NELIBC_GETENV_PROBE=visible $(OUT)/lib-smoke-tcc

lib-smoke-matrix: lib-smoke-zig lib-smoke-gcc lib-smoke-zig-musl lib-smoke-tcc

lib-consumer-zig: $(OUT)/libnelibc-zig.a
	$(ZIG) cc -pthread -ldl -Iinclude -fno-builtin tools/lib_consumer_smoke.c $(OUT)/libnelibc-zig.a -o $(OUT)/lib-consumer-zig
	$(OUT)/lib-consumer-zig

lib-consumer-gcc: $(OUT)/libnelibc-gcc.a
	gcc -pthread -ldl -Iinclude -fno-builtin tools/lib_consumer_smoke.c $(OUT)/libnelibc-gcc.a -o $(OUT)/lib-consumer-gcc
	$(OUT)/lib-consumer-gcc

lib-consumer-zig-musl: $(OUT)/libnelibc-zig-musl.a
	$(ZIG) cc -target x86_64-linux-musl -static -pthread -Iinclude -fno-builtin $(MUSL_STDIO_WRAP_FLAGS) -DNELIBC_STATIC_MUSL tools/lib_consumer_smoke.c $(OUT)/libnelibc-zig-musl.a -o $(OUT)/lib-consumer-zig-musl
	$(OUT)/lib-consumer-zig-musl

musl-malloc-smoke-zig-musl: $(OUT)/libnelibc-zig-musl.a tools/musl_malloc_smoke.c
	$(ZIG) cc -target x86_64-linux-musl -static -pthread -Iinclude -fno-builtin $(MUSL_STDIO_WRAP_FLAGS) tools/musl_malloc_smoke.c $(OUT)/libnelibc-zig-musl.a -o $(OUT)/musl-malloc-smoke-zig-musl
	$(OUT)/musl-malloc-smoke-zig-musl

lib-consumer-tcc: $(OUT)/libnelibc-tcc.a
	$(TCC) -pthread -Iinclude -fno-builtin -c tools/lib_consumer_smoke.c -o $(OUT)/lib-consumer-tcc.o
	$(ZIG) cc -pthread -ldl -Iinclude -fno-builtin $(OUT)/lib-consumer-tcc.o $(OUT)/libnelibc-tcc.a $(TCC_LIBTCC1) -o $(OUT)/lib-consumer-tcc
	$(OUT)/lib-consumer-tcc

dogfood-zig: $(OUT)/libnelibc-zig.a examples/incremental.c
	$(ZIG) cc -Wl,-u,memcpy -Iinclude -fno-builtin examples/incremental.c $(OUT)/libnelibc-zig.a -o $(OUT)/dogfood-zig
	$(OUT)/dogfood-zig
	@for symbol in memcpy strlen strchr strtol sqrt cbrt cbrtf fmod remainder fmin fmax fdim fdimf tan tanf sinh sinhf cosh coshf tanh tanhf asinh asinhf acosh acoshf atanh atanhf atan atanf atan2 atan2f pow powf modf frexp ldexp ldexpf scalbn scalbnf scalbln scalblnf rint rintf nearbyint nearbyintf exp expf exp2 exp2f expm1 expm1f exp10 exp10f log logf log2 log2f log1p log1pf log10 log10f ilogb logb nextafter nextafterf nexttoward nexttowardf; do nm --defined-only $(OUT)/dogfood-zig | awk '{print $$3}' | grep -Fxq "$$symbol" || { echo "dogfood binary did not link Nelibc $$symbol" >&2; exit 1; }; done

dogfood-gcc: $(OUT)/libnelibc-gcc.a examples/incremental.c
	gcc -Wl,-u,memcpy -Iinclude -fno-builtin examples/incremental.c $(OUT)/libnelibc-gcc.a -o $(OUT)/dogfood-gcc
	$(OUT)/dogfood-gcc
	@for symbol in memcpy strlen strchr strtol sqrt cbrt cbrtf fmod remainder fmin fmax fdim fdimf tan tanf sinh sinhf cosh coshf tanh tanhf asinh asinhf acosh acoshf atanh atanhf atan atanf atan2 atan2f pow powf modf frexp ldexp ldexpf scalbn scalbnf scalbln scalblnf rint rintf nearbyint nearbyintf exp expf exp2 exp2f expm1 expm1f exp10 exp10f log logf log2 log2f log1p log1pf log10 log10f ilogb logb nextafter nextafterf nexttoward nexttowardf; do nm --defined-only $(OUT)/dogfood-gcc | awk '{print $$3}' | grep -Fxq "$$symbol" || { echo "dogfood binary did not link Nelibc $$symbol" >&2; exit 1; }; done

dogfood-zig-musl: $(OUT)/libnelibc-zig-musl.a examples/incremental.c
	$(ZIG) cc -target x86_64-linux-musl -static -Wl,-u,memcpy -Iinclude -fno-builtin $(MUSL_STDIO_WRAP_FLAGS) examples/incremental.c $(OUT)/libnelibc-zig-musl.a -o $(OUT)/dogfood-zig-musl
	$(OUT)/dogfood-zig-musl
	@for symbol in memcpy strlen strchr strtol sqrt cbrt cbrtf fmod remainder fmin fmax fdim fdimf tan tanf sinh sinhf cosh coshf tanh tanhf asinh asinhf acosh acoshf atanh atanhf atan atanf atan2 atan2f pow powf modf frexp ldexp ldexpf scalbn scalbnf scalbln scalblnf rint rintf nearbyint nearbyintf exp expf exp2 exp2f expm1 expm1f exp10 exp10f log logf log2 log2f log1p log1pf log10 log10f ilogb logb nextafter nextafterf nexttoward nexttowardf; do nm --defined-only $(OUT)/dogfood-zig-musl | awk '{print $$3}' | grep -Fxq "$$symbol" || { echo "dogfood binary did not link Nelibc $$symbol" >&2; exit 1; }; done

dogfood-tcc: $(OUT)/libnelibc-tcc.a examples/incremental.c
	$(TCC) -Iinclude -fno-builtin -c examples/incremental.c -o $(OUT)/dogfood-tcc.o
	$(ZIG) cc -fsanitize=undefined -Wl,-u,memcpy -Iinclude -fno-builtin $(OUT)/dogfood-tcc.o $(OUT)/libnelibc-tcc.a $(TCC_LIBTCC1) -o $(OUT)/dogfood-tcc
	$(OUT)/dogfood-tcc
	@for symbol in memcpy strlen strchr strtol sqrt cbrt cbrtf fmod remainder fmin fmax fdim fdimf tan tanf sinh sinhf cosh coshf tanh tanhf asinh asinhf acosh acoshf atanh atanhf atan atanf atan2 atan2f pow powf modf frexp ldexp ldexpf scalbn scalbnf scalbln scalblnf rint rintf nearbyint nearbyintf exp expf exp2 exp2f expm1 expm1f exp10 exp10f log logf log2 log2f log1p log1pf log10 log10f ilogb logb nextafter nextafterf nexttoward nexttowardf; do nm --defined-only $(OUT)/dogfood-tcc | awk '{print $$3}' | grep -Fxq "$$symbol" || { echo "dogfood binary did not link Nelibc $$symbol" >&2; exit 1; }; done

dogfood-matrix: dogfood-zig dogfood-gcc dogfood-zig-musl dogfood-tcc

.PHONY: dogfood-zig dogfood-gcc dogfood-zig-musl dogfood-tcc dogfood-matrix

getopt-diff-zig: $(OUT)/libnelibc-zig.a
	$(ZIG) cc -Iinclude tools/getopt_diff.c -o $(OUT)/getopt-diff-host
	$(ZIG) cc -Iinclude -fno-builtin tools/getopt_diff.c $(OUT)/libnelibc-zig.a -o $(OUT)/getopt-diff-nelibc
	set -e; for mode in mixed terminator missing optional cluster unknown plus minus; do $(OUT)/getopt-diff-host $$mode > $(OUT)/getopt-host.out; $(OUT)/getopt-diff-nelibc $$mode > $(OUT)/getopt-nelibc.out; cmp $(OUT)/getopt-host.out $(OUT)/getopt-nelibc.out; done
	POSIXLY_CORRECT=1 $(OUT)/getopt-diff-host mixed > $(OUT)/getopt-host.out
	POSIXLY_CORRECT=1 $(OUT)/getopt-diff-nelibc mixed > $(OUT)/getopt-nelibc.out
	cmp $(OUT)/getopt-host.out $(OUT)/getopt-nelibc.out

getopt-long-diff-zig: $(OUT)/libnelibc-zig.a
	$(ZIG) cc -Iinclude tools/getopt_long_diff.c -o $(OUT)/getopt-long-diff-host
	$(ZIG) cc -Iinclude -fno-builtin tools/getopt_long_diff.c $(OUT)/libnelibc-zig.a -o $(OUT)/getopt-long-diff-nelibc
	set -e; for mode in exact abbrev required optional flag ambiguous unknown missing single-long single-short single-short-arg; do $(OUT)/getopt-long-diff-host $$mode > $(OUT)/getopt-long-host.out; $(OUT)/getopt-long-diff-nelibc $$mode > $(OUT)/getopt-long-nelibc.out; cmp $(OUT)/getopt-long-host.out $(OUT)/getopt-long-nelibc.out; done

getsubopt-diff-zig: $(OUT)/libnelibc-zig.a
	$(ZIG) cc -Iinclude tools/getsubopt_diff.c -o $(OUT)/getsubopt-diff-host
	$(ZIG) cc -Iinclude -fno-builtin tools/getsubopt_diff.c $(OUT)/libnelibc-zig.a -o $(OUT)/getsubopt-diff-nelibc
	$(OUT)/getsubopt-diff-host > $(OUT)/getsubopt-host.out
	$(OUT)/getsubopt-diff-nelibc > $(OUT)/getsubopt-nelibc.out
	cmp $(OUT)/getsubopt-host.out $(OUT)/getsubopt-nelibc.out

strerror-r-diff-zig: $(OUT)/libnelibc-zig.a
	$(ZIG) cc -DNELIBC_STRERROR_R_HOST -Iinclude tools/strerror_r_diff.c -o $(OUT)/strerror-r-diff-host
	$(ZIG) cc -Iinclude -fno-builtin tools/strerror_r_diff.c $(OUT)/libnelibc-zig.a -o $(OUT)/strerror-r-diff-nelibc
	$(OUT)/strerror-r-diff-host > $(OUT)/strerror-r-host.out
	$(OUT)/strerror-r-diff-nelibc > $(OUT)/strerror-r-nelibc.out
	cmp $(OUT)/strerror-r-host.out $(OUT)/strerror-r-nelibc.out

multibyte-diff-zig: $(OUT)/libnelibc-zig.a
	$(ZIG) cc -Iinclude tools/multibyte_diff.c -o $(OUT)/multibyte-diff-host-zig
	$(ZIG) cc -Iinclude -fno-builtin tools/multibyte_diff.c $(OUT)/libnelibc-zig.a -o $(OUT)/multibyte-diff-nelibc-zig
	$(OUT)/multibyte-diff-host-zig > $(OUT)/multibyte-host-zig.out
	$(OUT)/multibyte-diff-nelibc-zig > $(OUT)/multibyte-nelibc-zig.out
	cmp $(OUT)/multibyte-host-zig.out $(OUT)/multibyte-nelibc-zig.out

multibyte-diff-gcc: $(OUT)/libnelibc-gcc.a
	gcc -Iinclude tools/multibyte_diff.c -o $(OUT)/multibyte-diff-host-gcc
	gcc -Iinclude -fno-builtin tools/multibyte_diff.c $(OUT)/libnelibc-gcc.a -o $(OUT)/multibyte-diff-nelibc-gcc
	$(OUT)/multibyte-diff-host-gcc > $(OUT)/multibyte-host-gcc.out
	$(OUT)/multibyte-diff-nelibc-gcc > $(OUT)/multibyte-nelibc-gcc.out
	cmp $(OUT)/multibyte-host-gcc.out $(OUT)/multibyte-nelibc-gcc.out

multibyte-diff-zig-musl: $(OUT)/libnelibc-zig-musl.a
	$(ZIG) cc -target x86_64-linux-musl -static -Iinclude tools/multibyte_diff.c -o $(OUT)/multibyte-diff-host-zig-musl
	$(ZIG) cc -target x86_64-linux-musl -static -Iinclude -fno-builtin $(MUSL_STDIO_WRAP_FLAGS) tools/multibyte_diff.c $(OUT)/libnelibc-zig-musl.a -o $(OUT)/multibyte-diff-nelibc-zig-musl
	$(OUT)/multibyte-diff-host-zig-musl > $(OUT)/multibyte-host-zig-musl.out
	$(OUT)/multibyte-diff-nelibc-zig-musl > $(OUT)/multibyte-nelibc-zig-musl.out
	cmp $(OUT)/multibyte-host-zig-musl.out $(OUT)/multibyte-nelibc-zig-musl.out

multibyte-diff-tcc: $(OUT)/libnelibc-tcc.a
	$(TCC) -Iinclude tools/multibyte_diff.c -o $(OUT)/multibyte-diff-host-tcc
	$(TCC) -Iinclude -fno-builtin -c tools/multibyte_diff.c -o $(OUT)/multibyte-diff-nelibc-tcc.o
	$(ZIG) cc -fsanitize=undefined -Iinclude -fno-builtin $(OUT)/multibyte-diff-nelibc-tcc.o $(OUT)/libnelibc-tcc.a $(TCC_LIBTCC1) -o $(OUT)/multibyte-diff-nelibc-tcc
	$(OUT)/multibyte-diff-host-tcc > $(OUT)/multibyte-host-tcc.out
	$(OUT)/multibyte-diff-nelibc-tcc > $(OUT)/multibyte-nelibc-tcc.out
	cmp $(OUT)/multibyte-host-tcc.out $(OUT)/multibyte-nelibc-tcc.out

multibyte-diff-matrix: multibyte-diff-zig multibyte-diff-gcc multibyte-diff-zig-musl multibyte-diff-tcc

lib-consumer-matrix: lib-adapter-symbols lib-consumer-zig lib-consumer-gcc lib-consumer-zig-musl musl-malloc-smoke-zig-musl lib-consumer-tcc getopt-diff-zig getopt-long-diff-zig getsubopt-diff-zig strerror-r-diff-zig multibyte-diff-matrix wide-int-diff-matrix wide-string-diff-matrix rand-r-diff-matrix strtok-thread-matrix strtok-diff-matrix strverscmp-diff-matrix

strtok-thread-zig: $(OUT)/libnelibc-zig.a
	$(ZIG) cc -pthread -Iinclude -fno-builtin tools/strtok_thread_smoke.c $(OUT)/libnelibc-zig.a -o $(OUT)/strtok-thread-zig
	$(OUT)/strtok-thread-zig

strtok-thread-gcc: $(OUT)/libnelibc-gcc.a
	gcc -pthread -Iinclude -fno-builtin tools/strtok_thread_smoke.c $(OUT)/libnelibc-gcc.a -o $(OUT)/strtok-thread-gcc
	$(OUT)/strtok-thread-gcc

strtok-thread-zig-musl: $(OUT)/libnelibc-zig-musl.a
	$(ZIG) cc -target x86_64-linux-musl -static -pthread -Iinclude -fno-builtin $(MUSL_STDIO_WRAP_FLAGS) tools/strtok_thread_smoke.c $(OUT)/libnelibc-zig-musl.a -o $(OUT)/strtok-thread-zig-musl
	$(OUT)/strtok-thread-zig-musl

strtok-thread-tcc: $(OUT)/libnelibc-tcc.a
	$(TCC) -pthread -Iinclude -fno-builtin -c tools/strtok_thread_smoke.c -o $(OUT)/strtok-thread-tcc.o
	$(ZIG) cc -fsanitize=undefined -pthread -Iinclude -fno-builtin $(OUT)/strtok-thread-tcc.o $(OUT)/libnelibc-tcc.a $(TCC_LIBTCC1) -o $(OUT)/strtok-thread-tcc
	$(OUT)/strtok-thread-tcc

strtok-thread-matrix: strtok-thread-zig strtok-thread-gcc strtok-thread-zig-musl strtok-thread-tcc

strtok-diff-zig: $(OUT)/libnelibc-zig.a
	$(ZIG) cc -Iinclude -fno-builtin tools/strtok_diff.c -o $(OUT)/strtok-diff-host-zig
	$(ZIG) cc -Iinclude -fno-builtin tools/strtok_diff.c $(OUT)/libnelibc-zig.a -o $(OUT)/strtok-diff-nelibc-zig
	$(OUT)/strtok-diff-host-zig > $(OUT)/strtok-diff-host-zig.out
	$(OUT)/strtok-diff-nelibc-zig > $(OUT)/strtok-diff-nelibc-zig.out
	cmp $(OUT)/strtok-diff-host-zig.out $(OUT)/strtok-diff-nelibc-zig.out && echo '18434 tokenizer traces matched (Zig/glibc)'

strtok-diff-gcc: $(OUT)/libnelibc-gcc.a
	gcc -Iinclude -fno-builtin tools/strtok_diff.c -o $(OUT)/strtok-diff-host-gcc
	gcc -Iinclude -fno-builtin tools/strtok_diff.c $(OUT)/libnelibc-gcc.a -o $(OUT)/strtok-diff-nelibc-gcc
	$(OUT)/strtok-diff-host-gcc > $(OUT)/strtok-diff-host-gcc.out
	$(OUT)/strtok-diff-nelibc-gcc > $(OUT)/strtok-diff-nelibc-gcc.out
	cmp $(OUT)/strtok-diff-host-gcc.out $(OUT)/strtok-diff-nelibc-gcc.out && echo '18434 tokenizer traces matched (GCC/glibc)'

strtok-diff-zig-musl: $(OUT)/libnelibc-zig-musl.a
	$(ZIG) cc -target x86_64-linux-musl -static -Iinclude -fno-builtin tools/strtok_diff.c -o $(OUT)/strtok-diff-host-zig-musl
	$(ZIG) cc -target x86_64-linux-musl -static -Iinclude -fno-builtin $(MUSL_STDIO_WRAP_FLAGS) tools/strtok_diff.c $(OUT)/libnelibc-zig-musl.a -o $(OUT)/strtok-diff-nelibc-zig-musl
	$(OUT)/strtok-diff-host-zig-musl > $(OUT)/strtok-diff-host-zig-musl.out
	$(OUT)/strtok-diff-nelibc-zig-musl > $(OUT)/strtok-diff-nelibc-zig-musl.out
	cmp $(OUT)/strtok-diff-host-zig-musl.out $(OUT)/strtok-diff-nelibc-zig-musl.out && echo '18434 tokenizer traces matched (Zig/musl)'

strtok-diff-tcc: $(OUT)/libnelibc-tcc.a
	$(TCC) -Iinclude -fno-builtin tools/strtok_diff.c -o $(OUT)/strtok-diff-host-tcc
	$(TCC) -Iinclude -fno-builtin -c tools/strtok_diff.c -o $(OUT)/strtok-diff-nelibc-tcc.o
	$(ZIG) cc -fsanitize=undefined -Iinclude -fno-builtin $(OUT)/strtok-diff-nelibc-tcc.o $(OUT)/libnelibc-tcc.a $(TCC_LIBTCC1) -o $(OUT)/strtok-diff-nelibc-tcc
	$(OUT)/strtok-diff-host-tcc > $(OUT)/strtok-diff-host-tcc.out
	$(OUT)/strtok-diff-nelibc-tcc > $(OUT)/strtok-diff-nelibc-tcc.out
	cmp $(OUT)/strtok-diff-host-tcc.out $(OUT)/strtok-diff-nelibc-tcc.out && echo '18434 tokenizer traces matched (TinyCC/glibc)'

strtok-diff-matrix: strtok-diff-zig strtok-diff-gcc strtok-diff-zig-musl strtok-diff-tcc

.PHONY: strtok-diff-zig strtok-diff-gcc strtok-diff-zig-musl strtok-diff-tcc strtok-diff-matrix

strverscmp-diff-zig: $(OUT)/libnelibc-zig.a
	$(ZIG) cc -Iinclude -fno-builtin tools/strverscmp_diff.c -o $(OUT)/strverscmp-diff-host-zig
	$(ZIG) cc -Iinclude -fno-builtin tools/strverscmp_diff.c $(OUT)/libnelibc-zig.a -o $(OUT)/strverscmp-diff-nelibc-zig
	$(OUT)/strverscmp-diff-host-zig > $(OUT)/strverscmp-diff-host-zig.out
	$(OUT)/strverscmp-diff-nelibc-zig > $(OUT)/strverscmp-diff-nelibc-zig.out
	cmp $(OUT)/strverscmp-diff-host-zig.out $(OUT)/strverscmp-diff-nelibc-zig.out && echo '16384 strverscmp cases matched (Zig/glibc)'

strverscmp-diff-gcc: $(OUT)/libnelibc-gcc.a
	gcc -Iinclude -fno-builtin tools/strverscmp_diff.c -o $(OUT)/strverscmp-diff-host-gcc
	gcc -Iinclude -fno-builtin tools/strverscmp_diff.c $(OUT)/libnelibc-gcc.a -o $(OUT)/strverscmp-diff-nelibc-gcc
	$(OUT)/strverscmp-diff-host-gcc > $(OUT)/strverscmp-diff-host-gcc.out
	$(OUT)/strverscmp-diff-nelibc-gcc > $(OUT)/strverscmp-diff-nelibc-gcc.out
	cmp $(OUT)/strverscmp-diff-host-gcc.out $(OUT)/strverscmp-diff-nelibc-gcc.out && echo '16384 strverscmp cases matched (GCC/glibc)'

strverscmp-diff-zig-musl: $(OUT)/libnelibc-zig-musl.a
	$(ZIG) cc -target x86_64-linux-musl -static -Iinclude -fno-builtin tools/strverscmp_diff.c -o $(OUT)/strverscmp-diff-host-zig-musl
	$(ZIG) cc -target x86_64-linux-musl -static -Iinclude -fno-builtin $(MUSL_STDIO_WRAP_FLAGS) tools/strverscmp_diff.c $(OUT)/libnelibc-zig-musl.a -o $(OUT)/strverscmp-diff-nelibc-zig-musl
	$(OUT)/strverscmp-diff-host-zig-musl > $(OUT)/strverscmp-diff-host-zig-musl.out
	$(OUT)/strverscmp-diff-nelibc-zig-musl > $(OUT)/strverscmp-diff-nelibc-zig-musl.out
	cmp $(OUT)/strverscmp-diff-host-zig-musl.out $(OUT)/strverscmp-diff-nelibc-zig-musl.out && echo '16384 strverscmp cases matched (Zig/musl)'

strverscmp-diff-tcc: $(OUT)/libnelibc-tcc.a
	$(TCC) -Iinclude -fno-builtin tools/strverscmp_diff.c -o $(OUT)/strverscmp-diff-host-tcc
	$(TCC) -Iinclude -fno-builtin -c tools/strverscmp_diff.c -o $(OUT)/strverscmp-diff-nelibc-tcc.o
	$(ZIG) cc -fsanitize=undefined -Iinclude -fno-builtin $(OUT)/strverscmp-diff-nelibc-tcc.o $(OUT)/libnelibc-tcc.a $(TCC_LIBTCC1) -o $(OUT)/strverscmp-diff-nelibc-tcc
	$(OUT)/strverscmp-diff-host-tcc > $(OUT)/strverscmp-diff-host-tcc.out
	$(OUT)/strverscmp-diff-nelibc-tcc > $(OUT)/strverscmp-diff-nelibc-tcc.out
	cmp $(OUT)/strverscmp-diff-host-tcc.out $(OUT)/strverscmp-diff-nelibc-tcc.out && echo '16384 strverscmp cases matched (TinyCC/glibc)'

strverscmp-diff-matrix: strverscmp-diff-zig strverscmp-diff-gcc strverscmp-diff-zig-musl strverscmp-diff-tcc

.PHONY: strverscmp-diff-zig strverscmp-diff-gcc strverscmp-diff-zig-musl strverscmp-diff-tcc strverscmp-diff-matrix

strtok-shootout-zig: $(OUT)/libnelibc-zig.a
	$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-O2 --cflags=-fno-builtin -o $(OUT)/string-strtok-zig.o src/string.nelua
	$(ZIG) cc -O2 -pthread -Iinclude -fno-builtin -DBENCH_COMPILER='"Zig cc"' -DBENCH_LIBC='"glibc"' -DBENCH_IMPLEMENTATION='"host-libc"' tools/strtok_bench.c -o $(OUT)/strtok-zig-host
	$(ZIG) cc -O2 -pthread -Iinclude -fno-builtin -DBENCH_COMPILER='"Zig cc"' -DBENCH_LIBC='"glibc"' -DBENCH_IMPLEMENTATION='"nelibc"' -DBENCH_NELIBC_STRTOK_R tools/strtok_bench.c $(OUT)/string-strtok-zig.o $(OUT)/libnelibc-zig.a -o $(OUT)/strtok-zig-nelibc
	$(OUT)/strtok-zig-host > $(OUT)/strtok-shootout-zig-host.csv
	$(OUT)/strtok-zig-nelibc > $(OUT)/strtok-shootout-zig-nelibc.csv
	{ cat $(OUT)/strtok-shootout-zig-host.csv; tail -n +2 $(OUT)/strtok-shootout-zig-nelibc.csv; } > $(OUT)/strtok-shootout-zig.csv
	cat $(OUT)/strtok-shootout-zig.csv

strtok-shootout-gcc: $(OUT)/libnelibc-gcc.a
	$(NELUA) -B -r --cc gcc --cflags=-O2 --cflags=-fno-builtin -o $(OUT)/string-strtok-gcc.o src/string.nelua
	gcc -O2 -pthread -Iinclude -fno-builtin -DBENCH_COMPILER='"GCC"' -DBENCH_LIBC='"glibc"' -DBENCH_IMPLEMENTATION='"host-libc"' tools/strtok_bench.c -o $(OUT)/strtok-gcc-host
	gcc -O2 -pthread -Iinclude -fno-builtin -DBENCH_COMPILER='"GCC"' -DBENCH_LIBC='"glibc"' -DBENCH_IMPLEMENTATION='"nelibc"' -DBENCH_NELIBC_STRTOK_R tools/strtok_bench.c $(OUT)/string-strtok-gcc.o $(OUT)/libnelibc-gcc.a -o $(OUT)/strtok-gcc-nelibc
	$(OUT)/strtok-gcc-host > $(OUT)/strtok-shootout-gcc-host.csv
	$(OUT)/strtok-gcc-nelibc > $(OUT)/strtok-shootout-gcc-nelibc.csv
	{ cat $(OUT)/strtok-shootout-gcc-host.csv; tail -n +2 $(OUT)/strtok-shootout-gcc-nelibc.csv; } > $(OUT)/strtok-shootout-gcc.csv
	cat $(OUT)/strtok-shootout-gcc.csv

strtok-shootout-zig-musl: $(OUT)/libnelibc-zig-musl.a
	$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-target=x86_64-linux-musl --cflags=-O2 --cflags=-fno-builtin -o $(OUT)/string-strtok-zig-musl.o src/string.nelua
	$(ZIG) cc -target x86_64-linux-musl -static -O2 -pthread -Iinclude -fno-builtin -DBENCH_COMPILER='"Zig cc"' -DBENCH_LIBC='"musl"' -DBENCH_IMPLEMENTATION='"host-libc"' tools/strtok_bench.c -o $(OUT)/strtok-zig-musl-host
	$(ZIG) cc -target x86_64-linux-musl -static -O2 -pthread -Iinclude -fno-builtin $(MUSL_STDIO_WRAP_FLAGS) -DBENCH_COMPILER='"Zig cc"' -DBENCH_LIBC='"musl"' -DBENCH_IMPLEMENTATION='"nelibc"' -DBENCH_NELIBC_STRTOK_R tools/strtok_bench.c $(OUT)/string-strtok-zig-musl.o $(OUT)/libnelibc-zig-musl.a -o $(OUT)/strtok-zig-musl-nelibc
	$(OUT)/strtok-zig-musl-host > $(OUT)/strtok-shootout-zig-musl-host.csv
	$(OUT)/strtok-zig-musl-nelibc > $(OUT)/strtok-shootout-zig-musl-nelibc.csv
	{ cat $(OUT)/strtok-shootout-zig-musl-host.csv; tail -n +2 $(OUT)/strtok-shootout-zig-musl-nelibc.csv; } > $(OUT)/strtok-shootout-zig-musl.csv
	cat $(OUT)/strtok-shootout-zig-musl.csv

strtok-shootout-tcc: $(OUT)/libnelibc-tcc.a
	$(NELUA) -B -r --cc '$(TCC)' --cflags=-O2 --cflags=-fno-builtin -o $(OUT)/string-strtok-tcc.o src/string.nelua
	$(TCC) -O2 -pthread -Iinclude -fno-builtin -DBENCH_COMPILER='"TinyCC"' -DBENCH_LIBC='"glibc"' -DBENCH_IMPLEMENTATION='"host-libc"' tools/strtok_bench.c -o $(OUT)/strtok-tcc-host
	$(TCC) -O2 -pthread -Iinclude -fno-builtin -DBENCH_COMPILER='"TinyCC"' -DBENCH_LIBC='"glibc"' -DBENCH_IMPLEMENTATION='"nelibc"' -DBENCH_NELIBC_STRTOK_R tools/strtok_bench.c $(OUT)/string-strtok-tcc.o $(OUT)/libnelibc-tcc.a -o $(OUT)/strtok-tcc-nelibc
	$(OUT)/strtok-tcc-host > $(OUT)/strtok-shootout-tcc-host.csv
	$(OUT)/strtok-tcc-nelibc > $(OUT)/strtok-shootout-tcc-nelibc.csv
	{ cat $(OUT)/strtok-shootout-tcc-host.csv; tail -n +2 $(OUT)/strtok-shootout-tcc-nelibc.csv; } > $(OUT)/strtok-shootout-tcc.csv
	cat $(OUT)/strtok-shootout-tcc.csv

strtok-shootout-matrix: strtok-shootout-zig strtok-shootout-gcc strtok-shootout-zig-musl strtok-shootout-tcc
	awk 'FNR == 1 && NR > 1 { next } { print }' $(OUT)/strtok-shootout-zig.csv $(OUT)/strtok-shootout-gcc.csv $(OUT)/strtok-shootout-zig-musl.csv $(OUT)/strtok-shootout-tcc.csv > $(OUT)/strtok-shootout-matrix.csv
	cat $(OUT)/strtok-shootout-matrix.csv

lib-stdio-smoke-zig: $(OUT)/libnelibc-zig.a
	$(ZIG) cc -pthread -Iinclude -fno-builtin tools/lib_stdio_smoke.c $(OUT)/libnelibc-zig.a -o $(OUT)/lib-stdio-smoke-zig
	$(OUT)/lib-stdio-smoke-zig

lib-stdio-smoke-gcc: $(OUT)/libnelibc-gcc.a
	gcc -pthread -Iinclude -fno-builtin tools/lib_stdio_smoke.c $(OUT)/libnelibc-gcc.a -o $(OUT)/lib-stdio-smoke-gcc
	$(OUT)/lib-stdio-smoke-gcc

lib-stdio-smoke-zig-musl: $(OUT)/libnelibc-zig-musl.a
	$(ZIG) cc -target x86_64-linux-musl -static -pthread -Iinclude -fno-builtin $(MUSL_STDIO_WRAP_FLAGS) -DNELIBC_STATIC_MUSL tools/lib_stdio_smoke.c $(OUT)/libnelibc-zig-musl.a -o $(OUT)/lib-stdio-smoke-zig-musl
	$(OUT)/lib-stdio-smoke-zig-musl

lib-stdio-smoke-tcc: $(OUT)/libnelibc-tcc.a
	$(TCC) -pthread -Iinclude -fno-builtin -c tools/lib_stdio_smoke.c -o $(OUT)/lib-stdio-smoke-tcc.o
	$(ZIG) cc -fsanitize=undefined -pthread -Iinclude -fno-builtin $(OUT)/lib-stdio-smoke-tcc.o $(OUT)/libnelibc-tcc.a $(TCC_LIBTCC1) -o $(OUT)/lib-stdio-smoke-tcc
	$(OUT)/lib-stdio-smoke-tcc

lib-stdio-smoke-matrix: lib-stdio-smoke-zig lib-stdio-smoke-gcc lib-stdio-smoke-zig-musl lib-stdio-smoke-tcc fmemopen-diff-matrix

lib-stdio-smoke-available: lib-stdio-smoke-zig lib-stdio-smoke-gcc lib-stdio-smoke-zig-musl fmemopen-diff-matrix

atexit-hybrid-smoke-zig: $(OUT)/libnelibc-zig.a tools/atexit_hybrid_smoke.c
	$(ZIG) cc -Iinclude -fno-builtin tools/atexit_hybrid_smoke.c $(OUT)/libnelibc-zig.a -o $(OUT)/atexit-hybrid-smoke-zig
	$(OUT)/atexit-hybrid-smoke-zig

atexit-hybrid-smoke-gcc: $(OUT)/libnelibc-gcc.a tools/atexit_hybrid_smoke.c
	gcc -Iinclude -fno-builtin tools/atexit_hybrid_smoke.c $(OUT)/libnelibc-gcc.a -o $(OUT)/atexit-hybrid-smoke-gcc
	$(OUT)/atexit-hybrid-smoke-gcc

atexit-hybrid-smoke-zig-musl: $(OUT)/libnelibc-zig-musl.a tools/atexit_hybrid_smoke.c
	$(ZIG) cc -target x86_64-linux-musl -static -Iinclude -fno-builtin $(MUSL_STDIO_WRAP_FLAGS) -Wl,--wrap=atexit -Wl,--wrap=exit tools/atexit_hybrid_smoke.c $(OUT)/libnelibc-zig-musl.a -o $(OUT)/atexit-hybrid-smoke-zig-musl
	$(OUT)/atexit-hybrid-smoke-zig-musl

atexit-hybrid-smoke-tcc: $(OUT)/libnelibc-tcc.a tools/atexit_hybrid_smoke.c
	$(TCC) -Iinclude -fno-builtin -c tools/atexit_hybrid_smoke.c -o $(OUT)/atexit-hybrid-smoke-tcc.o
	$(ZIG) cc -fsanitize=undefined -Iinclude -fno-builtin $(OUT)/atexit-hybrid-smoke-tcc.o $(OUT)/libnelibc-tcc.a $(TCC_LIBTCC1) -o $(OUT)/atexit-hybrid-smoke-tcc
	$(OUT)/atexit-hybrid-smoke-tcc

atexit-hybrid-smoke-matrix: atexit-hybrid-smoke-zig atexit-hybrid-smoke-gcc atexit-hybrid-smoke-zig-musl atexit-hybrid-smoke-tcc

stdio-format-diff-zig: $(OUT)/libnelibc-zig.a
	$(ZIG) cc -Iinclude -fno-builtin tools/stdio_format_diff.c -o $(OUT)/stdio-format-host-zig
	$(ZIG) cc -Iinclude -fno-builtin tools/stdio_format_diff.c $(OUT)/libnelibc-zig.a -o $(OUT)/stdio-format-nelibc-zig
	$(OUT)/stdio-format-host-zig > $(OUT)/stdio-format-host-zig.txt
	$(OUT)/stdio-format-nelibc-zig > $(OUT)/stdio-format-nelibc-zig.txt
	cmp $(OUT)/stdio-format-host-zig.txt $(OUT)/stdio-format-nelibc-zig.txt

stdio-format-diff-gcc: $(OUT)/libnelibc-gcc.a
	gcc -Iinclude -fno-builtin tools/stdio_format_diff.c -o $(OUT)/stdio-format-host-gcc
	gcc -Iinclude -fno-builtin tools/stdio_format_diff.c $(OUT)/libnelibc-gcc.a -o $(OUT)/stdio-format-nelibc-gcc
	$(OUT)/stdio-format-host-gcc > $(OUT)/stdio-format-host-gcc.txt
	$(OUT)/stdio-format-nelibc-gcc > $(OUT)/stdio-format-nelibc-gcc.txt
	cmp $(OUT)/stdio-format-host-gcc.txt $(OUT)/stdio-format-nelibc-gcc.txt

stdio-format-diff-zig-musl: $(OUT)/libnelibc-zig-musl.a
	$(ZIG) cc -target x86_64-linux-musl -static -Iinclude -fno-builtin tools/stdio_format_diff.c -o $(OUT)/stdio-format-host-zig-musl
	$(ZIG) cc -target x86_64-linux-musl -static -Iinclude -fno-builtin $(MUSL_STDIO_WRAP_FLAGS) tools/stdio_format_diff.c $(OUT)/libnelibc-zig-musl.a -o $(OUT)/stdio-format-nelibc-zig-musl
	$(OUT)/stdio-format-host-zig-musl > $(OUT)/stdio-format-host-zig-musl.txt
	$(OUT)/stdio-format-nelibc-zig-musl > $(OUT)/stdio-format-nelibc-zig-musl.txt
	cmp $(OUT)/stdio-format-host-zig-musl.txt $(OUT)/stdio-format-nelibc-zig-musl.txt

stdio-format-diff-tcc: $(OUT)/libnelibc-tcc.a
	$(TCC) -Iinclude -fno-builtin tools/stdio_format_diff.c -o $(OUT)/stdio-format-host-tcc
	$(TCC) -Iinclude -fno-builtin -c tools/stdio_format_diff.c -o $(OUT)/stdio-format-nelibc-tcc.o
	$(ZIG) cc -fsanitize=undefined -Iinclude -fno-builtin $(OUT)/stdio-format-nelibc-tcc.o $(OUT)/libnelibc-tcc.a $(TCC_LIBTCC1) -o $(OUT)/stdio-format-nelibc-tcc
	$(OUT)/stdio-format-host-tcc > $(OUT)/stdio-format-host-tcc.txt
	$(OUT)/stdio-format-nelibc-tcc > $(OUT)/stdio-format-nelibc-tcc.txt
	cmp $(OUT)/stdio-format-host-tcc.txt $(OUT)/stdio-format-nelibc-tcc.txt

stdio-format-diff-matrix: stdio-format-diff-zig stdio-format-diff-gcc stdio-format-diff-zig-musl stdio-format-diff-tcc

fmemopen-diff-zig: $(OUT)/libnelibc-zig.a
	$(ZIG) cc -Iinclude -fno-builtin tools/fmemopen_diff.c -o $(OUT)/fmemopen-host-zig
	$(ZIG) cc -Iinclude -fno-builtin tools/fmemopen_diff.c $(OUT)/libnelibc-zig.a -o $(OUT)/fmemopen-nelibc-zig
	$(OUT)/fmemopen-host-zig > $(OUT)/fmemopen-host-zig.txt
	$(OUT)/fmemopen-nelibc-zig > $(OUT)/fmemopen-nelibc-zig.txt
	cmp $(OUT)/fmemopen-host-zig.txt $(OUT)/fmemopen-nelibc-zig.txt

fmemopen-diff-gcc: $(OUT)/libnelibc-gcc.a
	gcc -Iinclude -fno-builtin tools/fmemopen_diff.c -o $(OUT)/fmemopen-host-gcc
	gcc -Iinclude -fno-builtin tools/fmemopen_diff.c $(OUT)/libnelibc-gcc.a -o $(OUT)/fmemopen-nelibc-gcc
	$(OUT)/fmemopen-host-gcc > $(OUT)/fmemopen-host-gcc.txt
	$(OUT)/fmemopen-nelibc-gcc > $(OUT)/fmemopen-nelibc-gcc.txt
	cmp $(OUT)/fmemopen-host-gcc.txt $(OUT)/fmemopen-nelibc-gcc.txt

fmemopen-diff-zig-musl: $(OUT)/libnelibc-zig-musl.a
	$(ZIG) cc -target x86_64-linux-musl -static -Iinclude -fno-builtin tools/fmemopen_diff.c -o $(OUT)/fmemopen-host-zig-musl
	$(ZIG) cc -target x86_64-linux-musl -static -Iinclude -fno-builtin $(MUSL_STDIO_WRAP_FLAGS) tools/fmemopen_diff.c $(OUT)/libnelibc-zig-musl.a -o $(OUT)/fmemopen-nelibc-zig-musl
	$(OUT)/fmemopen-host-zig-musl > $(OUT)/fmemopen-host-zig-musl.txt
	$(OUT)/fmemopen-nelibc-zig-musl > $(OUT)/fmemopen-nelibc-zig-musl.txt
	cmp $(OUT)/fmemopen-host-zig-musl.txt $(OUT)/fmemopen-nelibc-zig-musl.txt

fmemopen-diff-matrix: fmemopen-diff-zig fmemopen-diff-gcc fmemopen-diff-zig-musl

stdio-shootout-zig: $(OUT)/libnelibc-zig.a
	$(ZIG) cc -O2 -fno-builtin -DBENCH_COMPILER='"Zig cc"' -DBENCH_LIBC='"glibc"' -DBENCH_IMPLEMENTATION='"host-libc"' tools/stdio_bench.c -o $(OUT)/stdio-host-zig
	$(ZIG) cc -O2 -fno-builtin -DBENCH_COMPILER='"Zig cc"' -DBENCH_LIBC='"glibc"' -DBENCH_IMPLEMENTATION='"nelibc"' tools/stdio_bench.c $(OUT)/libnelibc-zig.a -o $(OUT)/stdio-nelibc-zig
	@printf 'compiler,libc,implementation,function,iterations,median_ns_per_call\n' > $(OUT)/stdio-shootout-zig.csv
	$(OUT)/stdio-host-zig >> $(OUT)/stdio-shootout-zig.csv
	$(OUT)/stdio-nelibc-zig >> $(OUT)/stdio-shootout-zig.csv
	cat $(OUT)/stdio-shootout-zig.csv

stdio-shootout-gcc: $(OUT)/libnelibc-gcc.a
	gcc -O2 -fno-builtin -DBENCH_COMPILER='"GCC"' -DBENCH_LIBC='"glibc"' -DBENCH_IMPLEMENTATION='"host-libc"' tools/stdio_bench.c -o $(OUT)/stdio-host-gcc
	gcc -O2 -fno-builtin -DBENCH_COMPILER='"GCC"' -DBENCH_LIBC='"glibc"' -DBENCH_IMPLEMENTATION='"nelibc"' tools/stdio_bench.c $(OUT)/libnelibc-gcc.a -o $(OUT)/stdio-nelibc-gcc
	@printf 'compiler,libc,implementation,function,iterations,median_ns_per_call\n' > $(OUT)/stdio-shootout-gcc.csv
	$(OUT)/stdio-host-gcc >> $(OUT)/stdio-shootout-gcc.csv
	$(OUT)/stdio-nelibc-gcc >> $(OUT)/stdio-shootout-gcc.csv
	cat $(OUT)/stdio-shootout-gcc.csv

stdio-shootout-zig-musl: $(OUT)/libnelibc-zig-musl.a
	$(ZIG) cc -target x86_64-linux-musl -static -O2 -fno-builtin -DBENCH_COMPILER='"Zig cc"' -DBENCH_LIBC='"musl"' -DBENCH_IMPLEMENTATION='"host-libc"' tools/stdio_bench.c -o $(OUT)/stdio-host-zig-musl
	$(ZIG) cc -target x86_64-linux-musl -static -O2 -fno-builtin $(MUSL_STDIO_WRAP_FLAGS) -DBENCH_COMPILER='"Zig cc"' -DBENCH_LIBC='"musl"' -DBENCH_IMPLEMENTATION='"nelibc"' tools/stdio_bench.c $(OUT)/libnelibc-zig-musl.a -o $(OUT)/stdio-nelibc-zig-musl
	@printf 'compiler,libc,implementation,function,iterations,median_ns_per_call\n' > $(OUT)/stdio-shootout-zig-musl.csv
	$(OUT)/stdio-host-zig-musl >> $(OUT)/stdio-shootout-zig-musl.csv
	$(OUT)/stdio-nelibc-zig-musl >> $(OUT)/stdio-shootout-zig-musl.csv
	cat $(OUT)/stdio-shootout-zig-musl.csv

stdio-shootout-tcc: $(OUT)/libnelibc-tcc.a
	$(TCC) -O2 -fno-builtin -DBENCH_COMPILER='"TinyCC"' -DBENCH_LIBC='"glibc"' -DBENCH_IMPLEMENTATION='"host-libc"' tools/stdio_bench.c -o $(OUT)/stdio-host-tcc
	$(TCC) -O2 -fno-builtin -DBENCH_COMPILER='"TinyCC"' -DBENCH_LIBC='"glibc"' -DBENCH_IMPLEMENTATION='"nelibc"' tools/stdio_bench.c $(OUT)/libnelibc-tcc.a -o $(OUT)/stdio-nelibc-tcc
	@printf 'compiler,libc,implementation,function,iterations,median_ns_per_call\n' > $(OUT)/stdio-shootout-tcc.csv
	$(OUT)/stdio-host-tcc >> $(OUT)/stdio-shootout-tcc.csv
	$(OUT)/stdio-nelibc-tcc >> $(OUT)/stdio-shootout-tcc.csv
	cat $(OUT)/stdio-shootout-tcc.csv

stdio-shootout-matrix: stdio-shootout-zig stdio-shootout-gcc stdio-shootout-zig-musl stdio-shootout-tcc
	@awk 'FNR == 1 && NR > 1 { next } { print }' $(OUT)/stdio-shootout-zig.csv $(OUT)/stdio-shootout-gcc.csv $(OUT)/stdio-shootout-zig-musl.csv $(OUT)/stdio-shootout-tcc.csv > $(OUT)/stdio-shootout-matrix.csv
	@cat $(OUT)/stdio-shootout-matrix.csv

stdio-shootout-available: stdio-shootout-zig stdio-shootout-gcc stdio-shootout-zig-musl
	@awk 'FNR == 1 && NR > 1 { next } { print }' $(OUT)/stdio-shootout-zig.csv $(OUT)/stdio-shootout-gcc.csv $(OUT)/stdio-shootout-zig-musl.csv > $(OUT)/stdio-shootout-available.csv
	@cat $(OUT)/stdio-shootout-available.csv

.PHONY: lib-stdio-smoke-available fmemopen-diff-zig fmemopen-diff-gcc fmemopen-diff-zig-musl fmemopen-diff-matrix lib-zig lib-gcc lib-zig-musl lib-tcc lib-smoke-zig lib-smoke-gcc lib-smoke-zig-musl lib-smoke-tcc lib-smoke-matrix lib-consumer-zig lib-consumer-gcc lib-consumer-zig-musl lib-consumer-tcc lib-consumer-matrix lib-stdio-smoke-zig lib-stdio-smoke-gcc lib-stdio-smoke-zig-musl lib-stdio-smoke-tcc lib-stdio-smoke-matrix atexit-hybrid-smoke-zig atexit-hybrid-smoke-gcc atexit-hybrid-smoke-zig-musl atexit-hybrid-smoke-tcc atexit-hybrid-smoke-matrix stdio-shootout-zig stdio-shootout-gcc stdio-shootout-zig-musl stdio-shootout-tcc stdio-shootout-matrix getopt-diff-zig getopt-long-diff-zig getsubopt-diff-zig strerror-r-diff-zig multibyte-diff-zig multibyte-diff-gcc multibyte-diff-zig-musl multibyte-diff-tcc multibyte-diff-matrix wide-int-diff-zig wide-int-diff-gcc wide-int-diff-zig-musl wide-int-diff-tcc wide-int-diff-matrix wide-string-diff-zig wide-string-diff-gcc wide-string-diff-zig-musl wide-string-diff-tcc wide-string-diff-matrix

int-diff-zig: zig
	$(ZIG) cc -Iinclude -fno-builtin tools/int_diff.c -o $(OUT)/int-diff-host-zig
	$(ZIG) cc -Iinclude -fno-builtin tools/int_diff.c $(OUT)/memory-zig.o $(OUT)/memory-x86_64-simd.o $(OUT)/stdlib-zig.o $(OUT)/errno-host-zig.o src/environment_adapter.c -o $(OUT)/int-diff-nelibc-zig
	$(OUT)/int-diff-host-zig > $(OUT)/int-diff-host-zig.csv
	$(OUT)/int-diff-nelibc-zig > $(OUT)/int-diff-nelibc-zig.csv
	cmp $(OUT)/int-diff-host-zig.csv $(OUT)/int-diff-nelibc-zig.csv && echo '1209 integer conversion cases passed (Zig/glibc)'

int-diff-gcc: gcc
	gcc -Iinclude -fno-builtin tools/int_diff.c -o $(OUT)/int-diff-host-gcc
	gcc -Iinclude -fno-builtin tools/int_diff.c $(OUT)/memory-gcc.o $(OUT)/memory-x86_64-simd.o $(OUT)/stdlib-gcc.o $(OUT)/errno-host-gcc.o src/environment_adapter.c -o $(OUT)/int-diff-nelibc-gcc
	$(OUT)/int-diff-host-gcc > $(OUT)/int-diff-host-gcc.csv
	$(OUT)/int-diff-nelibc-gcc > $(OUT)/int-diff-nelibc-gcc.csv
	cmp $(OUT)/int-diff-host-gcc.csv $(OUT)/int-diff-nelibc-gcc.csv && echo '1209 integer conversion cases passed (GCC/glibc)'

int-diff-zig-musl: zig-musl
	$(ZIG) cc -target x86_64-linux-musl -static -Iinclude -fno-builtin tools/int_diff.c -o $(OUT)/int-diff-host-zig-musl
	$(ZIG) cc -target x86_64-linux-musl -static -Iinclude -fno-builtin tools/int_diff.c $(OUT)/memory-zig-musl.o $(OUT)/memory-x86_64-simd.o $(OUT)/stdlib-zig-musl.o $(OUT)/errno-host-zig-musl.o src/environment_adapter.c -o $(OUT)/int-diff-nelibc-zig-musl
	$(OUT)/int-diff-host-zig-musl > $(OUT)/int-diff-host-zig-musl.csv
	$(OUT)/int-diff-nelibc-zig-musl > $(OUT)/int-diff-nelibc-zig-musl.csv
	cmp $(OUT)/int-diff-host-zig-musl.csv $(OUT)/int-diff-nelibc-zig-musl.csv && echo '1209 integer conversion cases passed (Zig/musl)'

int-diff-tcc: tcc
	$(TCC) -Iinclude -fno-builtin tools/int_diff.c -o $(OUT)/int-diff-host-tcc
	$(TCC) -Iinclude -fno-builtin -c tools/int_diff.c -o $(OUT)/int-diff-nelibc-tcc.o
	$(ZIG) cc -Iinclude -fno-builtin -c src/environment_adapter.c -o $(OUT)/environment-adapter-int-diff-tcc.o
	$(ZIG) cc -Iinclude -fno-builtin $(OUT)/int-diff-nelibc-tcc.o $(OUT)/memory-tcc.o $(OUT)/memory-tcc-helper.o $(OUT)/memory-tcc-simd.o $(OUT)/stdlib-tcc.o $(OUT)/errno-host-tcc.o $(OUT)/environment-adapter-int-diff-tcc.o $(TCC_LIBTCC1) -o $(OUT)/int-diff-nelibc-tcc
	$(OUT)/int-diff-host-tcc > $(OUT)/int-diff-host-tcc.csv
	$(OUT)/int-diff-nelibc-tcc > $(OUT)/int-diff-nelibc-tcc.csv
	cmp $(OUT)/int-diff-host-tcc.csv $(OUT)/int-diff-nelibc-tcc.csv && echo '1209 integer conversion cases passed (TinyCC/glibc)'

.PHONY: int-diff-tcc
int-diff-matrix: int-diff-zig int-diff-gcc int-diff-zig-musl int-diff-tcc

qsort-diff-zig: $(OUT)/libnelibc-zig.a
	$(ZIG) cc -Iinclude -fno-builtin tools/qsort_diff.c -o $(OUT)/qsort-diff-host-zig
	$(NELUA) --print-code -r src/stdlib.nelua > $(OUT)/stdlib-zig-qsort-test-generated.c
	$(ZIG) cc -DNELIBC_QSORT_DIAGNOSTICS -DNELIBC_QSORT_HOLE_SIFT -fno-builtin -Iinclude -c $(OUT)/stdlib-zig-qsort-test-generated.c -o $(OUT)/stdlib-zig-qsort-test.o
	$(ZIG) cc -DNELIBC_QSORT_DIAGNOSTICS -Iinclude -fno-builtin tools/qsort_diff.c $(OUT)/stdlib-zig-qsort-test.o $(OUT)/libnelibc-zig.a -o $(OUT)/qsort-diff-nelibc-zig
	$(OUT)/qsort-diff-host-zig > $(OUT)/qsort-diff-host-zig.csv
	$(OUT)/qsort-diff-nelibc-zig > $(OUT)/qsort-diff-nelibc-zig.csv
	cmp $(OUT)/qsort-diff-host-zig.csv $(OUT)/qsort-diff-nelibc-zig.csv && echo 'qsort/bsearch: boundary, patterned, and randomized arrays match (Zig/glibc)'

qsort-diff-gcc: $(OUT)/libnelibc-gcc.a
	gcc -Iinclude -fno-builtin tools/qsort_diff.c -o $(OUT)/qsort-diff-host-gcc
	$(NELUA) --print-code -r src/stdlib.nelua > $(OUT)/stdlib-gcc-qsort-test-generated.c
	gcc -DNELIBC_QSORT_DIAGNOSTICS -DNELIBC_QSORT_GCC_INTROSORT -fno-builtin -Iinclude -c $(OUT)/stdlib-gcc-qsort-test-generated.c -o $(OUT)/stdlib-gcc-qsort-test.o
	gcc -DNELIBC_QSORT_DIAGNOSTICS -Iinclude -fno-builtin tools/qsort_diff.c $(OUT)/stdlib-gcc-qsort-test.o $(OUT)/libnelibc-gcc.a -o $(OUT)/qsort-diff-nelibc-gcc
	$(OUT)/qsort-diff-host-gcc > $(OUT)/qsort-diff-host-gcc.csv
	$(OUT)/qsort-diff-nelibc-gcc > $(OUT)/qsort-diff-nelibc-gcc.csv
	cmp $(OUT)/qsort-diff-host-gcc.csv $(OUT)/qsort-diff-nelibc-gcc.csv && echo 'qsort/bsearch: boundary, patterned, and randomized arrays match (GCC/glibc)'

qsort-diff-zig-musl: $(OUT)/libnelibc-zig-musl.a
	$(ZIG) cc -target x86_64-linux-musl -static -Iinclude -fno-builtin tools/qsort_diff.c -o $(OUT)/qsort-diff-host-zig-musl
	$(NELUA) --print-code -r src/stdlib.nelua > $(OUT)/stdlib-zig-musl-qsort-test-generated.c
	$(ZIG) cc -target x86_64-linux-musl -DNELIBC_QSORT_DIAGNOSTICS -DNELIBC_QSORT_HOLE_SIFT -fno-builtin -Iinclude -c $(OUT)/stdlib-zig-musl-qsort-test-generated.c -o $(OUT)/stdlib-zig-musl-qsort-test.o
	$(ZIG) cc -target x86_64-linux-musl -static -DNELIBC_QSORT_DIAGNOSTICS -Iinclude -fno-builtin $(MUSL_STDIO_WRAP_FLAGS) tools/qsort_diff.c $(OUT)/stdlib-zig-musl-qsort-test.o $(OUT)/libnelibc-zig-musl.a -o $(OUT)/qsort-diff-nelibc-zig-musl
	$(OUT)/qsort-diff-host-zig-musl > $(OUT)/qsort-diff-host-zig-musl.csv
	$(OUT)/qsort-diff-nelibc-zig-musl > $(OUT)/qsort-diff-nelibc-zig-musl.csv
	cmp $(OUT)/qsort-diff-host-zig-musl.csv $(OUT)/qsort-diff-nelibc-zig-musl.csv && echo 'qsort/bsearch: boundary, patterned, and randomized arrays match (Zig/musl)'

qsort-diff-tcc: $(OUT)/libnelibc-tcc.a
	$(TCC) -Iinclude -fno-builtin tools/qsort_diff.c -o $(OUT)/qsort-diff-host-tcc
	$(NELUA) --print-code -r src/stdlib.nelua > $(OUT)/stdlib-tcc-qsort-test-generated.c
	$(ZIG) cc -O2 -DNELIBC_QSORT_DIAGNOSTICS -DNELIBC_QSORT_HOLE_SIFT -fno-builtin -Iinclude -fno-sanitize=all -c $(OUT)/stdlib-tcc-qsort-test-generated.c -o $(OUT)/stdlib-tcc-qsort-test.o
	$(TCC) -DNELIBC_QSORT_DIAGNOSTICS -Iinclude -fno-builtin tools/qsort_diff.c $(OUT)/stdlib-tcc-qsort-test.o $(OUT)/libnelibc-tcc.a -o $(OUT)/qsort-diff-nelibc-tcc
	$(OUT)/qsort-diff-host-tcc > $(OUT)/qsort-diff-host-tcc.csv
	$(OUT)/qsort-diff-nelibc-tcc > $(OUT)/qsort-diff-nelibc-tcc.csv
	cmp $(OUT)/qsort-diff-host-tcc.csv $(OUT)/qsort-diff-nelibc-tcc.csv && echo 'qsort/bsearch: boundary, patterned, and randomized arrays match (TinyCC/glibc)'

qsort-diff-matrix: qsort-diff-zig qsort-diff-gcc qsort-diff-zig-musl qsort-diff-tcc
.PHONY: qsort-diff-zig qsort-diff-gcc qsort-diff-zig-musl qsort-diff-tcc qsort-diff-matrix

qsort-shootout-zig: $(OUT)/libnelibc-zig.a
	$(ZIG) cc -O2 -fno-builtin -DBENCH_COMPILER='"Zig cc"' -DBENCH_LIBC='"glibc"' -DBENCH_IMPLEMENTATION='"host-libc"' tools/qsort_bench.c -o $(OUT)/qsort-host-zig
	$(ZIG) cc -O2 -fno-builtin -DBENCH_COMPILER='"Zig cc"' -DBENCH_LIBC='"glibc"' -DBENCH_IMPLEMENTATION='"nelibc"' tools/qsort_bench.c $(OUT)/libnelibc-zig.a -o $(OUT)/qsort-nelibc-zig
	$(OUT)/qsort-host-zig > $(OUT)/qsort-shootout-zig-host.csv
	$(OUT)/qsort-nelibc-zig > $(OUT)/qsort-shootout-zig-nelibc.csv
	{ cat $(OUT)/qsort-shootout-zig-host.csv; tail -n +2 $(OUT)/qsort-shootout-zig-nelibc.csv; } > $(OUT)/qsort-shootout-zig.csv
	cat $(OUT)/qsort-shootout-zig.csv

qsort-shootout-gcc: $(OUT)/libnelibc-gcc.a
	gcc -O2 -fno-builtin -DBENCH_COMPILER='"GCC"' -DBENCH_LIBC='"glibc"' -DBENCH_IMPLEMENTATION='"host-libc"' tools/qsort_bench.c -o $(OUT)/qsort-host-gcc
	gcc -O2 -fno-builtin -DBENCH_COMPILER='"GCC"' -DBENCH_LIBC='"glibc"' -DBENCH_IMPLEMENTATION='"nelibc"' tools/qsort_bench.c $(OUT)/libnelibc-gcc.a -o $(OUT)/qsort-nelibc-gcc
	$(OUT)/qsort-host-gcc > $(OUT)/qsort-shootout-gcc-host.csv
	$(OUT)/qsort-nelibc-gcc > $(OUT)/qsort-shootout-gcc-nelibc.csv
	{ cat $(OUT)/qsort-shootout-gcc-host.csv; tail -n +2 $(OUT)/qsort-shootout-gcc-nelibc.csv; } > $(OUT)/qsort-shootout-gcc.csv
	cat $(OUT)/qsort-shootout-gcc.csv

qsort-shootout-zig-musl: $(OUT)/libnelibc-zig-musl.a
	$(ZIG) cc -target x86_64-linux-musl -static -O2 -fno-builtin -DBENCH_COMPILER='"Zig cc"' -DBENCH_LIBC='"musl"' -DBENCH_IMPLEMENTATION='"host-libc"' tools/qsort_bench.c -o $(OUT)/qsort-host-zig-musl
	$(ZIG) cc -target x86_64-linux-musl -static -O2 -fno-builtin $(MUSL_STDIO_WRAP_FLAGS) -DBENCH_COMPILER='"Zig cc"' -DBENCH_LIBC='"musl"' -DBENCH_IMPLEMENTATION='"nelibc"' tools/qsort_bench.c $(OUT)/libnelibc-zig-musl.a -o $(OUT)/qsort-nelibc-zig-musl
	$(OUT)/qsort-host-zig-musl > $(OUT)/qsort-shootout-zig-musl-host.csv
	$(OUT)/qsort-nelibc-zig-musl > $(OUT)/qsort-shootout-zig-musl-nelibc.csv
	{ cat $(OUT)/qsort-shootout-zig-musl-host.csv; tail -n +2 $(OUT)/qsort-shootout-zig-musl-nelibc.csv; } > $(OUT)/qsort-shootout-zig-musl.csv
	cat $(OUT)/qsort-shootout-zig-musl.csv

qsort-shootout-tcc: $(OUT)/libnelibc-tcc.a
	$(TCC) -O2 -fno-builtin -DBENCH_COMPILER='"TinyCC"' -DBENCH_LIBC='"glibc"' -DBENCH_IMPLEMENTATION='"host-libc"' tools/qsort_bench.c -o $(OUT)/qsort-host-tcc
	$(TCC) -O2 -fno-builtin -DBENCH_COMPILER='"TinyCC"' -DBENCH_LIBC='"glibc"' -DBENCH_IMPLEMENTATION='"nelibc"' tools/qsort_bench.c $(OUT)/libnelibc-tcc.a -o $(OUT)/qsort-nelibc-tcc
	$(OUT)/qsort-host-tcc > $(OUT)/qsort-shootout-tcc-host.csv
	$(OUT)/qsort-nelibc-tcc > $(OUT)/qsort-shootout-tcc-nelibc.csv
	{ cat $(OUT)/qsort-shootout-tcc-host.csv; tail -n +2 $(OUT)/qsort-shootout-tcc-nelibc.csv; } > $(OUT)/qsort-shootout-tcc.csv
	cat $(OUT)/qsort-shootout-tcc.csv

qsort-shootout-matrix: qsort-shootout-zig qsort-shootout-gcc qsort-shootout-zig-musl qsort-shootout-tcc
	awk 'FNR == 1 && NR > 1 { next } { print }' $(OUT)/qsort-shootout-zig.csv $(OUT)/qsort-shootout-gcc.csv $(OUT)/qsort-shootout-zig-musl.csv $(OUT)/qsort-shootout-tcc.csv > $(OUT)/qsort-shootout-matrix.csv
	cat $(OUT)/qsort-shootout-matrix.csv
.PHONY: qsort-shootout-zig qsort-shootout-gcc qsort-shootout-zig-musl qsort-shootout-tcc qsort-shootout-matrix

wide-int-diff-zig: zig
	$(ZIG) cc -Iinclude -fno-builtin tools/wide_int_diff.c -o $(OUT)/wide-int-diff-host-zig
	$(ZIG) cc -Iinclude -fno-builtin tools/wide_int_diff.c $(OUT)/memory-zig.o $(OUT)/memory-x86_64-simd.o $(OUT)/stdlib-zig.o $(OUT)/errno-host-zig.o src/environment_adapter.c -o $(OUT)/wide-int-diff-nelibc-zig
	$(OUT)/wide-int-diff-host-zig > $(OUT)/wide-int-diff-host-zig.csv
	$(OUT)/wide-int-diff-nelibc-zig > $(OUT)/wide-int-diff-nelibc-zig.csv
	cmp $(OUT)/wide-int-diff-host-zig.csv $(OUT)/wide-int-diff-nelibc-zig.csv && echo '1210 wide integer conversion cases passed (Zig/glibc)'

wide-int-diff-gcc: gcc
	gcc -Iinclude -fno-builtin tools/wide_int_diff.c -o $(OUT)/wide-int-diff-host-gcc
	gcc -Iinclude -fno-builtin tools/wide_int_diff.c $(OUT)/memory-gcc.o $(OUT)/memory-x86_64-simd.o $(OUT)/stdlib-gcc.o $(OUT)/errno-host-gcc.o src/environment_adapter.c -o $(OUT)/wide-int-diff-nelibc-gcc
	$(OUT)/wide-int-diff-host-gcc > $(OUT)/wide-int-diff-host-gcc.csv
	$(OUT)/wide-int-diff-nelibc-gcc > $(OUT)/wide-int-diff-nelibc-gcc.csv
	cmp $(OUT)/wide-int-diff-host-gcc.csv $(OUT)/wide-int-diff-nelibc-gcc.csv && echo '1210 wide integer conversion cases passed (GCC/glibc)'

wide-int-diff-zig-musl: zig-musl
	$(ZIG) cc -target x86_64-linux-musl -static -Iinclude -fno-builtin tools/wide_int_diff.c -o $(OUT)/wide-int-diff-host-zig-musl
	$(ZIG) cc -target x86_64-linux-musl -static -Iinclude -fno-builtin tools/wide_int_diff.c $(OUT)/memory-zig-musl.o $(OUT)/memory-x86_64-simd.o $(OUT)/stdlib-zig-musl.o $(OUT)/errno-host-zig-musl.o src/environment_adapter.c -o $(OUT)/wide-int-diff-nelibc-zig-musl
	$(OUT)/wide-int-diff-host-zig-musl > $(OUT)/wide-int-diff-host-zig-musl.csv
	$(OUT)/wide-int-diff-nelibc-zig-musl > $(OUT)/wide-int-diff-nelibc-zig-musl.csv
	cmp $(OUT)/wide-int-diff-host-zig-musl.csv $(OUT)/wide-int-diff-nelibc-zig-musl.csv && echo '1210 wide integer conversion cases passed (Zig/musl)'

wide-int-diff-tcc: tcc
	$(TCC) -Iinclude -fno-builtin tools/wide_int_diff.c -o $(OUT)/wide-int-diff-host-tcc
	$(TCC) -Iinclude -fno-builtin -c tools/wide_int_diff.c -o $(OUT)/wide-int-diff-nelibc-tcc.o
	$(ZIG) cc -Iinclude -fno-builtin -c src/environment_adapter.c -o $(OUT)/environment-adapter-wide-int-tcc.o
	$(ZIG) cc -Iinclude -fno-builtin $(OUT)/wide-int-diff-nelibc-tcc.o $(OUT)/memory-tcc.o $(OUT)/memory-tcc-helper.o $(OUT)/memory-tcc-simd.o $(OUT)/stdlib-tcc.o $(OUT)/errno-host-tcc.o $(OUT)/environment-adapter-wide-int-tcc.o $(TCC_LIBTCC1) -o $(OUT)/wide-int-diff-nelibc-tcc
	$(OUT)/wide-int-diff-host-tcc > $(OUT)/wide-int-diff-host-tcc.csv
	$(OUT)/wide-int-diff-nelibc-tcc > $(OUT)/wide-int-diff-nelibc-tcc.csv
	cmp $(OUT)/wide-int-diff-host-tcc.csv $(OUT)/wide-int-diff-nelibc-tcc.csv && echo '1210 wide integer conversion cases passed (TinyCC/glibc)'

wide-int-diff-matrix: wide-int-diff-zig wide-int-diff-gcc wide-int-diff-zig-musl wide-int-diff-tcc

wide-string-diff-zig: zig
	$(ZIG) cc -Iinclude -fno-builtin tools/wide_string_diff.c -o $(OUT)/wide-string-diff-host-zig
	$(ZIG) cc -Iinclude -fno-builtin tools/wide_string_diff.c $(OUT)/memory-zig.o $(OUT)/memory-x86_64-simd.o $(OUT)/stdlib-zig.o $(OUT)/errno-host-zig.o src/environment_adapter.c -o $(OUT)/wide-string-diff-nelibc-zig
	$(OUT)/wide-string-diff-host-zig > $(OUT)/wide-string-diff-host-zig.csv
	$(OUT)/wide-string-diff-nelibc-zig > $(OUT)/wide-string-diff-nelibc-zig.csv
	cmp $(OUT)/wide-string-diff-host-zig.csv $(OUT)/wide-string-diff-nelibc-zig.csv && echo '256 wide string and memory cases passed (Zig/glibc)'

wide-string-diff-gcc: gcc
	gcc -Iinclude -fno-builtin tools/wide_string_diff.c -o $(OUT)/wide-string-diff-host-gcc
	gcc -Iinclude -fno-builtin tools/wide_string_diff.c $(OUT)/memory-gcc.o $(OUT)/memory-x86_64-simd.o $(OUT)/stdlib-gcc.o $(OUT)/errno-host-gcc.o src/environment_adapter.c -o $(OUT)/wide-string-diff-nelibc-gcc
	$(OUT)/wide-string-diff-host-gcc > $(OUT)/wide-string-diff-host-gcc.csv
	$(OUT)/wide-string-diff-nelibc-gcc > $(OUT)/wide-string-diff-nelibc-gcc.csv
	cmp $(OUT)/wide-string-diff-host-gcc.csv $(OUT)/wide-string-diff-nelibc-gcc.csv && echo '256 wide string and memory cases passed (GCC/glibc)'

wide-string-diff-zig-musl: zig-musl
	$(ZIG) cc -target x86_64-linux-musl -static -Iinclude -fno-builtin tools/wide_string_diff.c -o $(OUT)/wide-string-diff-host-zig-musl
	$(ZIG) cc -target x86_64-linux-musl -static -Iinclude -fno-builtin tools/wide_string_diff.c $(OUT)/memory-zig-musl.o $(OUT)/memory-x86_64-simd.o $(OUT)/stdlib-zig-musl.o $(OUT)/errno-host-zig-musl.o src/environment_adapter.c -o $(OUT)/wide-string-diff-nelibc-zig-musl
	$(OUT)/wide-string-diff-host-zig-musl > $(OUT)/wide-string-diff-host-zig-musl.csv
	$(OUT)/wide-string-diff-nelibc-zig-musl > $(OUT)/wide-string-diff-nelibc-zig-musl.csv
	cmp $(OUT)/wide-string-diff-host-zig-musl.csv $(OUT)/wide-string-diff-nelibc-zig-musl.csv && echo '256 wide string and memory cases passed (Zig/musl)'

wide-string-diff-tcc: tcc
	$(TCC) -Iinclude -fno-builtin tools/wide_string_diff.c -o $(OUT)/wide-string-diff-host-tcc
	$(TCC) -Iinclude -fno-builtin -c tools/wide_string_diff.c -o $(OUT)/wide-string-diff-nelibc-tcc.o
	$(ZIG) cc -Iinclude -fno-builtin -c src/environment_adapter.c -o $(OUT)/environment-adapter-wide-string-tcc.o
	$(ZIG) cc -Iinclude -fno-builtin $(OUT)/wide-string-diff-nelibc-tcc.o $(OUT)/memory-tcc.o $(OUT)/memory-tcc-helper.o $(OUT)/memory-tcc-simd.o $(OUT)/stdlib-tcc.o $(OUT)/errno-host-tcc.o $(OUT)/environment-adapter-wide-string-tcc.o $(TCC_LIBTCC1) -o $(OUT)/wide-string-diff-nelibc-tcc
	$(OUT)/wide-string-diff-host-tcc > $(OUT)/wide-string-diff-host-tcc.csv
	$(OUT)/wide-string-diff-nelibc-tcc > $(OUT)/wide-string-diff-nelibc-tcc.csv
	cmp $(OUT)/wide-string-diff-host-tcc.csv $(OUT)/wide-string-diff-nelibc-tcc.csv && echo '256 wide string and memory cases passed (TinyCC/glibc)'

wide-string-diff-matrix: wide-string-diff-zig wide-string-diff-gcc wide-string-diff-zig-musl wide-string-diff-tcc

collation-diff-zig: zig
	$(ZIG) cc -Iinclude -fno-builtin tools/collation_diff.c $(OUT)/string-locale-zig.o $(OUT)/string-locale-lock-zig.o $(OUT)/string-zig.o $(OUT)/memory-zig.o $(OUT)/memory-x86_64-simd.o -o $(OUT)/collation-diff-zig
	$(OUT)/collation-diff-zig --global C

collation-diff-gcc: gcc
	gcc -Iinclude -fno-builtin tools/collation_diff.c $(OUT)/string-locale-gcc.o $(OUT)/string-locale-lock-gcc.o $(OUT)/string-gcc.o $(OUT)/memory-gcc.o $(OUT)/memory-x86_64-simd.o -o $(OUT)/collation-diff-gcc
	$(OUT)/collation-diff-gcc --global C

collation-diff-zig-musl: zig-musl
	$(ZIG) cc -target x86_64-linux-musl -static -Iinclude -fno-builtin tools/collation_diff.c $(OUT)/string-locale-zig-musl.o $(OUT)/string-locale-lock-zig-musl.o $(OUT)/string-zig-musl.o $(OUT)/memory-zig-musl.o $(OUT)/memory-x86_64-simd.o -o $(OUT)/collation-diff-zig-musl
	$(OUT)/collation-diff-zig-musl --global C

collation-diff-tcc: tcc
	$(TCC) -pthread -Iinclude -fno-builtin -c tools/collation_diff.c -o $(OUT)/collation-diff-tcc.o
	$(ZIG) cc -pthread -fno-builtin -c src/string_locale_lock.c -o $(OUT)/string-locale-lock-collation-tcc.o
	$(ZIG) cc -pthread -Iinclude -fno-builtin $(OUT)/collation-diff-tcc.o $(OUT)/string-locale-tcc.o $(OUT)/string-locale-lock-collation-tcc.o $(OUT)/string-tcc.o $(OUT)/memory-tcc.o $(OUT)/memory-tcc-helper.o $(OUT)/memory-tcc-simd.o $(TCC_LIBTCC1) -o $(OUT)/collation-diff-tcc
	$(OUT)/collation-diff-tcc --global C

collation-diff-matrix: collation-diff-zig collation-diff-gcc collation-diff-zig-musl collation-diff-tcc

collation-shootout-zig: zig
	$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-O2 --cflags=-fno-builtin -o $(OUT)/string-collation-zig.o src/string.nelua
	$(ZIG) cc -O2 -Iinclude -fno-builtin -DBENCH_COMPILER='"Zig cc"' -DBENCH_LIBC='"glibc"' tools/collation_bench.c $(OUT)/string-locale-zig.o $(OUT)/string-locale-lock-zig.o $(OUT)/string-collation-zig.o $(OUT)/memory-zig.o $(OUT)/memory-x86_64-simd.o -o $(OUT)/collation-shootout-zig
	$(OUT)/collation-shootout-zig $(COLLATION_LOCALE) > $(OUT)/collation-shootout-zig.csv
	cat $(OUT)/collation-shootout-zig.csv

collation-shootout-gcc: gcc
	$(NELUA) -B -r --cc gcc --cflags=-O2 --cflags=-fno-builtin -o $(OUT)/string-collation-gcc.o src/string.nelua
	gcc -O2 -Iinclude -fno-builtin -DBENCH_COMPILER='"GCC"' -DBENCH_LIBC='"glibc"' tools/collation_bench.c $(OUT)/string-locale-gcc.o $(OUT)/string-locale-lock-gcc.o $(OUT)/string-collation-gcc.o $(OUT)/memory-gcc.o $(OUT)/memory-x86_64-simd.o -o $(OUT)/collation-shootout-gcc
	$(OUT)/collation-shootout-gcc $(COLLATION_LOCALE) > $(OUT)/collation-shootout-gcc.csv
	cat $(OUT)/collation-shootout-gcc.csv

collation-shootout-zig-musl: zig-musl
	$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-target=x86_64-linux-musl --cflags=-O2 --cflags=-fno-builtin -o $(OUT)/string-collation-zig-musl.o src/string.nelua
	$(ZIG) cc -target x86_64-linux-musl -static -O2 -Iinclude -fno-builtin -DNELIBC_STATIC_MUSL -Wl,--wrap=free -Wl,--wrap=realloc -DBENCH_COMPILER='"Zig cc"' -DBENCH_LIBC='"musl"' tools/collation_bench.c $(OUT)/string-locale-zig-musl.o $(OUT)/string-locale-lock-zig-musl.o $(OUT)/string-collation-zig-musl.o $(OUT)/memory-zig-musl.o $(OUT)/memory-x86_64-simd.o -o $(OUT)/collation-shootout-zig-musl
	$(OUT)/collation-shootout-zig-musl $(COLLATION_LOCALE) > $(OUT)/collation-shootout-zig-musl.csv
	cat $(OUT)/collation-shootout-zig-musl.csv

collation-shootout-tcc: tcc
	$(NELUA) -B -r --cc '$(TCC)' --cflags=-O2 --cflags=-fno-builtin -o $(OUT)/string-collation-tcc.o src/string.nelua
	$(TCC) -pthread -O2 -Iinclude -fno-builtin -DNELIBC_ALLOCATOR_FASTPATH_SAFE -DBENCH_COMPILER='"TinyCC"' -DBENCH_LIBC='"glibc"' tools/collation_bench.c $(OUT)/string-locale-tcc.o $(OUT)/string-locale-lock-tcc.o $(OUT)/string-collation-tcc.o $(OUT)/memory-tcc.o $(OUT)/memory-tcc-helper.o $(OUT)/memory-tcc-simd.o -o $(OUT)/collation-shootout-tcc
	$(OUT)/collation-shootout-tcc $(COLLATION_LOCALE) > $(OUT)/collation-shootout-tcc.csv
	cat $(OUT)/collation-shootout-tcc.csv

collation-shootout-matrix: collation-shootout-zig collation-shootout-gcc collation-shootout-zig-musl collation-shootout-tcc
	@awk 'FNR == 1 && NR > 1 { next } { print }' $(OUT)/collation-shootout-zig.csv $(OUT)/collation-shootout-gcc.csv $(OUT)/collation-shootout-zig-musl.csv $(OUT)/collation-shootout-tcc.csv > $(OUT)/collation-shootout-matrix.csv
	@cat $(OUT)/collation-shootout-matrix.csv

float-shootout-zig: zig
	$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-O2 --cflags=-fno-builtin -o $(OUT)/stdlib-float-bench-zig.o src/stdlib_float.nelua
	$(ZIG) cc -O2 -Iinclude -fno-builtin -fno-sanitize=undefined -DBENCH_COMPILER='"Zig cc"' -DBENCH_LIBC='"glibc"' tools/float_bench.c $(OUT)/memory-zig.o $(OUT)/memory-x86_64-simd.o $(OUT)/stdlib-float-bench-zig.o $(OUT)/errno-host-zig.o -o $(OUT)/float-shootout-zig
	$(OUT)/float-shootout-zig > $(OUT)/float-shootout-zig.csv
	cat $(OUT)/float-shootout-zig.csv

float-shootout-gcc: gcc
	$(NELUA) -B -r --cc gcc --cflags=-O2 --cflags=-fno-builtin -o $(OUT)/stdlib-float-bench-gcc.o src/stdlib_float.nelua
	gcc -O2 -Iinclude -fno-builtin -DBENCH_COMPILER='"GCC"' -DBENCH_LIBC='"glibc"' tools/float_bench.c $(OUT)/memory-gcc.o $(OUT)/memory-x86_64-simd.o $(OUT)/stdlib-float-bench-gcc.o $(OUT)/errno-host-gcc.o -o $(OUT)/float-shootout-gcc
	$(OUT)/float-shootout-gcc > $(OUT)/float-shootout-gcc.csv
	cat $(OUT)/float-shootout-gcc.csv

float-shootout-zig-musl: zig-musl
	$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-target=x86_64-linux-musl --cflags=-O2 --cflags=-fno-builtin -o $(OUT)/stdlib-float-bench-zig-musl.o src/stdlib_float.nelua
	$(ZIG) cc -target x86_64-linux-musl -static -O2 -Iinclude -fno-builtin -DBENCH_HOST_ONLY -DBENCH_COMPILER='"Zig cc"' -DBENCH_LIBC='"musl"' tools/float_bench.c -o $(OUT)/float-shootout-zig-musl-host
	$(ZIG) cc -target x86_64-linux-musl -static -O2 -Iinclude -fno-builtin -fno-sanitize=undefined -DBENCH_NELIBC_ONLY -DBENCH_COMPILER='"Zig cc"' -DBENCH_LIBC='"musl"' tools/float_bench.c $(OUT)/memory-zig-musl.o $(OUT)/memory-x86_64-simd.o $(OUT)/stdlib-float-bench-zig-musl.o $(OUT)/errno-host-zig-musl.o -o $(OUT)/float-shootout-zig-musl-nelibc
	$(OUT)/float-shootout-zig-musl-host > $(OUT)/float-shootout-zig-musl-host.csv
	$(OUT)/float-shootout-zig-musl-nelibc > $(OUT)/float-shootout-zig-musl-nelibc.csv
	@awk 'FNR == 1 && NR > 1 { next } { print }' $(OUT)/float-shootout-zig-musl-host.csv $(OUT)/float-shootout-zig-musl-nelibc.csv > $(OUT)/float-shootout-zig-musl.csv
	cat $(OUT)/float-shootout-zig-musl.csv

float-shootout-tcc: tcc
	$(NELUA) -B -r --cc '$(TCC)' --cflags=-O2 --cflags=-fno-builtin -o $(OUT)/stdlib-float-bench-tcc.o src/stdlib_float.nelua
	$(TCC) -O2 -Iinclude -fno-builtin -DBENCH_COMPILER='"TinyCC"' -DBENCH_LIBC='"glibc"' tools/float_bench.c $(OUT)/memory-tcc.o $(OUT)/memory-tcc-helper.o $(OUT)/memory-tcc-simd.o $(OUT)/stdlib-float-bench-tcc.o $(OUT)/errno-host-tcc.o -o $(OUT)/float-shootout-tcc
	$(OUT)/float-shootout-tcc > $(OUT)/float-shootout-tcc.csv
	cat $(OUT)/float-shootout-tcc.csv

.PHONY: float-shootout-tcc
float-shootout-matrix: float-shootout-zig float-shootout-gcc float-shootout-zig-musl float-shootout-tcc
	@awk 'FNR == 1 && NR > 1 { next } { print }' $(OUT)/float-shootout-zig.csv $(OUT)/float-shootout-gcc.csv $(OUT)/float-shootout-zig-musl.csv $(OUT)/float-shootout-tcc.csv > $(OUT)/float-shootout-matrix.csv
	@cat $(OUT)/float-shootout-matrix.csv

float-long-shootout-zig: zig
	$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-O2 --cflags=-fno-builtin -o $(OUT)/stdlib-float-bench-zig.o src/stdlib_float.nelua
	$(ZIG) cc -O2 -Iinclude -fno-builtin -fno-sanitize=undefined -DBENCH_LONG_DECIMALS -DBENCH_COMPILER='"Zig cc"' -DBENCH_LIBC='"glibc"' tools/float_bench.c $(OUT)/memory-zig.o $(OUT)/memory-x86_64-simd.o $(OUT)/stdlib-float-bench-zig.o $(OUT)/errno-host-zig.o -o $(OUT)/float-long-shootout-zig
	$(OUT)/float-long-shootout-zig > $(OUT)/float-long-shootout-zig.csv
	cat $(OUT)/float-long-shootout-zig.csv

float-long-shootout-gcc: gcc
	$(NELUA) -B -r --cc gcc --cflags=-O2 --cflags=-fno-builtin -o $(OUT)/stdlib-float-bench-gcc.o src/stdlib_float.nelua
	gcc -O2 -Iinclude -fno-builtin -DBENCH_LONG_DECIMALS -DBENCH_COMPILER='"GCC"' -DBENCH_LIBC='"glibc"' tools/float_bench.c $(OUT)/memory-gcc.o $(OUT)/memory-x86_64-simd.o $(OUT)/stdlib-float-bench-gcc.o $(OUT)/errno-host-gcc.o -o $(OUT)/float-long-shootout-gcc
	$(OUT)/float-long-shootout-gcc > $(OUT)/float-long-shootout-gcc.csv
	cat $(OUT)/float-long-shootout-gcc.csv

float-long-shootout-zig-musl: zig-musl
	$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-target=x86_64-linux-musl --cflags=-O2 --cflags=-fno-builtin -o $(OUT)/stdlib-float-bench-zig-musl.o src/stdlib_float.nelua
	$(ZIG) cc -target x86_64-linux-musl -static -O2 -Iinclude -fno-builtin -DBENCH_LONG_DECIMALS -DBENCH_HOST_ONLY -DBENCH_COMPILER='"Zig cc"' -DBENCH_LIBC='"musl"' tools/float_bench.c -o $(OUT)/float-long-shootout-zig-musl-host
	$(ZIG) cc -target x86_64-linux-musl -static -O2 -Iinclude -fno-builtin -fno-sanitize=undefined -DBENCH_LONG_DECIMALS -DBENCH_NELIBC_ONLY -DBENCH_COMPILER='"Zig cc"' -DBENCH_LIBC='"musl"' tools/float_bench.c $(OUT)/memory-zig-musl.o $(OUT)/memory-x86_64-simd.o $(OUT)/stdlib-float-bench-zig-musl.o $(OUT)/errno-host-zig-musl.o -o $(OUT)/float-long-shootout-zig-musl-nelibc
	$(OUT)/float-long-shootout-zig-musl-host > $(OUT)/float-long-shootout-zig-musl-host.csv
	$(OUT)/float-long-shootout-zig-musl-nelibc > $(OUT)/float-long-shootout-zig-musl-nelibc.csv
	@awk 'FNR == 1 && NR > 1 { next } { print }' $(OUT)/float-long-shootout-zig-musl-host.csv $(OUT)/float-long-shootout-zig-musl-nelibc.csv > $(OUT)/float-long-shootout-zig-musl.csv
	cat $(OUT)/float-long-shootout-zig-musl.csv

float-long-shootout-tcc: tcc
	$(NELUA) -B -r --cc '$(TCC)' --cflags=-O2 --cflags=-fno-builtin -o $(OUT)/stdlib-float-bench-tcc.o src/stdlib_float.nelua
	$(TCC) -O2 -Iinclude -fno-builtin -DBENCH_LONG_DECIMALS -DBENCH_COMPILER='"TinyCC"' -DBENCH_LIBC='"glibc"' tools/float_bench.c $(OUT)/memory-tcc.o $(OUT)/memory-tcc-helper.o $(OUT)/memory-tcc-simd.o $(OUT)/stdlib-float-bench-tcc.o $(OUT)/errno-host-tcc.o -o $(OUT)/float-long-shootout-tcc
	$(OUT)/float-long-shootout-tcc > $(OUT)/float-long-shootout-tcc.csv
	cat $(OUT)/float-long-shootout-tcc.csv

float-long-shootout-matrix: float-long-shootout-zig float-long-shootout-gcc float-long-shootout-zig-musl float-long-shootout-tcc
	@awk 'FNR == 1 && NR > 1 { next } { print }' $(OUT)/float-long-shootout-zig.csv $(OUT)/float-long-shootout-gcc.csv $(OUT)/float-long-shootout-zig-musl.csv $(OUT)/float-long-shootout-tcc.csv > $(OUT)/float-long-shootout-matrix.csv
	@cat $(OUT)/float-long-shootout-matrix.csv

float-shootout-tcc-opt: tcc
	@for opt in 0 1 2 3; do \
		$(NELUA) -B -r --cc '$(TCC)' --cflags=-O$$opt --cflags=-fno-builtin -o $(OUT)/stdlib-float-opt-O$$opt.o src/stdlib_float.nelua || exit; \
		$(TCC) -O$$opt -Iinclude -fno-builtin -DBENCH_COMPILER='"TinyCC -O'$$opt'"' -DBENCH_LIBC='"glibc"' tools/float_bench.c $(OUT)/memory-tcc.o $(OUT)/memory-tcc-helper.o $(OUT)/memory-tcc-simd.o $(OUT)/stdlib-float-opt-O$$opt.o $(OUT)/errno-host-tcc.o -o $(OUT)/float-shootout-opt-O$$opt || exit; \
		$(OUT)/float-shootout-opt-O$$opt > $(OUT)/float-shootout-opt-O$$opt.csv || exit; \
	done
	@awk 'FNR == 1 && NR > 1 { next } { print }' $(OUT)/float-shootout-opt-O0.csv $(OUT)/float-shootout-opt-O1.csv $(OUT)/float-shootout-opt-O2.csv $(OUT)/float-shootout-opt-O3.csv > $(OUT)/float-shootout-tcc-opt.csv
	@cat $(OUT)/float-shootout-tcc-opt.csv

.PHONY: float-shootout-tcc-opt

int-shootout-zig: zig
	$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-O2 --cflags=-fno-builtin -o $(OUT)/stdlib-int-bench-zig.o src/stdlib.nelua
	$(ZIG) cc -O2 -Iinclude -fno-builtin -DBENCH_COMPILER='"Zig cc"' -DBENCH_LIBC='"glibc"' -DBENCH_IMPLEMENTATION='"host-libc"' tools/int_bench.c -o $(OUT)/int-shootout-host-zig
	$(ZIG) cc -O2 -Iinclude -fno-builtin -DBENCH_COMPILER='"Zig cc"' -DBENCH_LIBC='"glibc"' -DBENCH_IMPLEMENTATION='"nelibc"' tools/int_bench.c $(OUT)/memory-zig.o $(OUT)/memory-x86_64-simd.o $(OUT)/stdlib-int-bench-zig.o $(OUT)/errno-host-zig.o src/environment_adapter.c -o $(OUT)/int-shootout-nelibc-zig
	$(OUT)/int-shootout-host-zig > $(OUT)/int-shootout-host-zig.csv
	$(OUT)/int-shootout-nelibc-zig > $(OUT)/int-shootout-nelibc-zig.csv
	{ cat $(OUT)/int-shootout-host-zig.csv; tail -n +2 $(OUT)/int-shootout-nelibc-zig.csv; } > $(OUT)/int-shootout-zig.csv
	cat $(OUT)/int-shootout-zig.csv

int-shootout-gcc: gcc
	$(NELUA) -B -r --cc gcc --cflags=-O2 --cflags=-fno-builtin -o $(OUT)/stdlib-int-bench-gcc.o src/stdlib.nelua
	gcc -O2 -Iinclude -fno-builtin -DBENCH_COMPILER='"GCC"' -DBENCH_LIBC='"glibc"' -DBENCH_IMPLEMENTATION='"host-libc"' tools/int_bench.c -o $(OUT)/int-shootout-host-gcc
	gcc -O2 -Iinclude -fno-builtin -DBENCH_COMPILER='"GCC"' -DBENCH_LIBC='"glibc"' -DBENCH_IMPLEMENTATION='"nelibc"' tools/int_bench.c $(OUT)/memory-gcc.o $(OUT)/memory-x86_64-simd.o $(OUT)/stdlib-int-bench-gcc.o $(OUT)/errno-host-gcc.o src/environment_adapter.c -o $(OUT)/int-shootout-nelibc-gcc
	$(OUT)/int-shootout-host-gcc > $(OUT)/int-shootout-host-gcc.csv
	$(OUT)/int-shootout-nelibc-gcc > $(OUT)/int-shootout-nelibc-gcc.csv
	{ cat $(OUT)/int-shootout-host-gcc.csv; tail -n +2 $(OUT)/int-shootout-nelibc-gcc.csv; } > $(OUT)/int-shootout-gcc.csv
	cat $(OUT)/int-shootout-gcc.csv

int-shootout-zig-musl: zig-musl
	$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-target=x86_64-linux-musl --cflags=-O2 --cflags=-fno-builtin -o $(OUT)/stdlib-int-bench-zig-musl.o src/stdlib.nelua
	$(ZIG) cc -target x86_64-linux-musl -static -O2 -Iinclude -fno-builtin -DNELIBC_STATIC_MUSL -Wl,--wrap=free -Wl,--wrap=realloc -DBENCH_COMPILER='"Zig cc"' -DBENCH_LIBC='"musl"' -DBENCH_IMPLEMENTATION='"host-libc"' tools/int_bench.c -o $(OUT)/int-shootout-host-zig-musl
	$(ZIG) cc -target x86_64-linux-musl -static -O2 -Iinclude -fno-builtin -DNELIBC_STATIC_MUSL -Wl,--wrap=free -Wl,--wrap=realloc -DBENCH_COMPILER='"Zig cc"' -DBENCH_LIBC='"musl"' -DBENCH_IMPLEMENTATION='"nelibc"' tools/int_bench.c $(OUT)/memory-zig-musl.o $(OUT)/memory-x86_64-simd.o $(OUT)/stdlib-int-bench-zig-musl.o $(OUT)/errno-host-zig-musl.o src/environment_adapter.c -o $(OUT)/int-shootout-nelibc-zig-musl
	$(OUT)/int-shootout-host-zig-musl > $(OUT)/int-shootout-host-zig-musl.csv
	$(OUT)/int-shootout-nelibc-zig-musl > $(OUT)/int-shootout-nelibc-zig-musl.csv
	{ cat $(OUT)/int-shootout-host-zig-musl.csv; tail -n +2 $(OUT)/int-shootout-nelibc-zig-musl.csv; } > $(OUT)/int-shootout-zig-musl.csv
	cat $(OUT)/int-shootout-zig-musl.csv

int-shootout-tcc: tcc
	$(NELUA) -B -r --cc '$(TCC)' --cflags=-O2 --cflags=-fno-builtin -o $(OUT)/stdlib-int-bench-tcc.o src/stdlib.nelua
	$(TCC) -O2 -Iinclude -fno-builtin -DBENCH_COMPILER='"TinyCC"' -DBENCH_LIBC='"glibc"' -DBENCH_IMPLEMENTATION='"host-libc"' tools/int_bench.c -o $(OUT)/int-shootout-host-tcc
	$(TCC) -O2 -Iinclude -fno-builtin -DBENCH_COMPILER='"TinyCC"' -DBENCH_LIBC='"glibc"' -DBENCH_IMPLEMENTATION='"nelibc"' tools/int_bench.c $(OUT)/memory-tcc.o $(OUT)/memory-tcc-helper.o $(OUT)/memory-tcc-simd.o $(OUT)/stdlib-int-bench-tcc.o $(OUT)/errno-host-tcc.o src/environment_adapter.c -o $(OUT)/int-shootout-nelibc-tcc
	$(OUT)/int-shootout-host-tcc > $(OUT)/int-shootout-host-tcc.csv
	$(OUT)/int-shootout-nelibc-tcc > $(OUT)/int-shootout-nelibc-tcc.csv
	{ cat $(OUT)/int-shootout-host-tcc.csv; tail -n +2 $(OUT)/int-shootout-nelibc-tcc.csv; } > $(OUT)/int-shootout-tcc.csv
	cat $(OUT)/int-shootout-tcc.csv

int-shootout-matrix: int-shootout-zig int-shootout-gcc int-shootout-zig-musl int-shootout-tcc
	@awk 'FNR == 1 && NR > 1 { next } { print }' $(OUT)/int-shootout-zig.csv $(OUT)/int-shootout-gcc.csv $(OUT)/int-shootout-zig-musl.csv $(OUT)/int-shootout-tcc.csv > $(OUT)/int-shootout-matrix.csv
	@cat $(OUT)/int-shootout-matrix.csv

rand-r-shootout-zig: $(OUT)/libnelibc-zig.a
	$(ZIG) cc -O2 -Iinclude -fno-builtin -DBENCH_COMPILER='"Zig cc"' -DBENCH_LIBC='"glibc"' -DBENCH_IMPLEMENTATION='"host-libc"' tools/rand_r_bench.c -o $(OUT)/rand-r-shootout-host-zig
	$(ZIG) cc -fsanitize=undefined -O2 -Iinclude -fno-builtin -DBENCH_COMPILER='"Zig cc"' -DBENCH_LIBC='"glibc"' -DBENCH_IMPLEMENTATION='"nelibc"' tools/rand_r_bench.c $(OUT)/libnelibc-zig.a -o $(OUT)/rand-r-shootout-nelibc-zig
	$(OUT)/rand-r-shootout-host-zig > $(OUT)/rand-r-shootout-host-zig.csv
	$(OUT)/rand-r-shootout-nelibc-zig > $(OUT)/rand-r-shootout-nelibc-zig.csv
	{ cat $(OUT)/rand-r-shootout-host-zig.csv; tail -n +2 $(OUT)/rand-r-shootout-nelibc-zig.csv; } > $(OUT)/rand-r-shootout-zig.csv
	cat $(OUT)/rand-r-shootout-zig.csv

rand-r-shootout-gcc: $(OUT)/libnelibc-gcc.a
	gcc -O2 -Iinclude -fno-builtin -DBENCH_COMPILER='"GCC"' -DBENCH_LIBC='"glibc"' -DBENCH_IMPLEMENTATION='"host-libc"' tools/rand_r_bench.c -o $(OUT)/rand-r-shootout-host-gcc
	gcc -O2 -Iinclude -fno-builtin -DBENCH_COMPILER='"GCC"' -DBENCH_LIBC='"glibc"' -DBENCH_IMPLEMENTATION='"nelibc"' tools/rand_r_bench.c $(OUT)/libnelibc-gcc.a -o $(OUT)/rand-r-shootout-nelibc-gcc
	$(OUT)/rand-r-shootout-host-gcc > $(OUT)/rand-r-shootout-host-gcc.csv
	$(OUT)/rand-r-shootout-nelibc-gcc > $(OUT)/rand-r-shootout-nelibc-gcc.csv
	{ cat $(OUT)/rand-r-shootout-host-gcc.csv; tail -n +2 $(OUT)/rand-r-shootout-nelibc-gcc.csv; } > $(OUT)/rand-r-shootout-gcc.csv
	cat $(OUT)/rand-r-shootout-gcc.csv

rand-r-shootout-zig-musl: $(OUT)/libnelibc-zig-musl.a
	$(ZIG) cc -target x86_64-linux-musl -static -O2 -Iinclude -fno-builtin -DNELIBC_STATIC_MUSL -Wl,--wrap=free -Wl,--wrap=realloc -DBENCH_COMPILER='"Zig cc"' -DBENCH_LIBC='"musl"' -DBENCH_IMPLEMENTATION='"host-libc"' tools/rand_r_bench.c -o $(OUT)/rand-r-shootout-host-zig-musl
	$(ZIG) cc -target x86_64-linux-musl -static -fsanitize=undefined -O2 -Iinclude -fno-builtin $(MUSL_STDIO_WRAP_FLAGS) -DBENCH_COMPILER='"Zig cc"' -DBENCH_LIBC='"musl"' -DBENCH_IMPLEMENTATION='"nelibc"' tools/rand_r_bench.c $(OUT)/libnelibc-zig-musl.a -o $(OUT)/rand-r-shootout-nelibc-zig-musl
	$(OUT)/rand-r-shootout-host-zig-musl > $(OUT)/rand-r-shootout-host-zig-musl.csv
	$(OUT)/rand-r-shootout-nelibc-zig-musl > $(OUT)/rand-r-shootout-nelibc-zig-musl.csv
	{ cat $(OUT)/rand-r-shootout-host-zig-musl.csv; tail -n +2 $(OUT)/rand-r-shootout-nelibc-zig-musl.csv; } > $(OUT)/rand-r-shootout-zig-musl.csv
	cat $(OUT)/rand-r-shootout-zig-musl.csv

rand-r-shootout-tcc: $(OUT)/libnelibc-tcc.a
	$(TCC) -O2 -Iinclude -fno-builtin -DBENCH_COMPILER='"TinyCC"' -DBENCH_LIBC='"glibc"' -DBENCH_IMPLEMENTATION='"host-libc"' tools/rand_r_bench.c -o $(OUT)/rand-r-shootout-host-tcc
	$(TCC) -O2 -Iinclude -fno-builtin -DBENCH_COMPILER='"TinyCC"' -DBENCH_LIBC='"glibc"' -DBENCH_IMPLEMENTATION='"nelibc"' tools/rand_r_bench.c $(OUT)/libnelibc-tcc.a -o $(OUT)/rand-r-shootout-nelibc-tcc
	$(OUT)/rand-r-shootout-host-tcc > $(OUT)/rand-r-shootout-host-tcc.csv
	$(OUT)/rand-r-shootout-nelibc-tcc > $(OUT)/rand-r-shootout-nelibc-tcc.csv
	{ cat $(OUT)/rand-r-shootout-host-tcc.csv; tail -n +2 $(OUT)/rand-r-shootout-nelibc-tcc.csv; } > $(OUT)/rand-r-shootout-tcc.csv
	cat $(OUT)/rand-r-shootout-tcc.csv

rand-r-shootout-matrix: rand-r-shootout-zig rand-r-shootout-gcc rand-r-shootout-zig-musl rand-r-shootout-tcc
	@awk 'FNR == 1 && NR > 1 { next } { print }' $(OUT)/rand-r-shootout-zig.csv $(OUT)/rand-r-shootout-gcc.csv $(OUT)/rand-r-shootout-zig-musl.csv $(OUT)/rand-r-shootout-tcc.csv > $(OUT)/rand-r-shootout-matrix.csv
	@cat $(OUT)/rand-r-shootout-matrix.csv

rand-r-diff-zig: $(OUT)/libnelibc-zig.a
	$(ZIG) cc -pthread -Iinclude -fno-builtin tools/rand_r_diff.c $(OUT)/libnelibc-zig.a -o $(OUT)/rand-r-diff-zig
	$(OUT)/rand-r-diff-zig

rand-r-diff-gcc: $(OUT)/libnelibc-gcc.a
	gcc -pthread -Iinclude -fno-builtin tools/rand_r_diff.c $(OUT)/libnelibc-gcc.a -o $(OUT)/rand-r-diff-gcc
	$(OUT)/rand-r-diff-gcc

rand-r-diff-zig-musl: $(OUT)/libnelibc-zig-musl.a
	$(ZIG) cc -target x86_64-linux-musl -static -Iinclude -fno-builtin $(MUSL_STDIO_WRAP_FLAGS) tools/rand_r_diff.c $(OUT)/libnelibc-zig-musl.a -o $(OUT)/rand-r-diff-zig-musl
	$(OUT)/rand-r-diff-zig-musl

rand-r-diff-tcc: $(OUT)/libnelibc-tcc.a
	$(TCC) -pthread -Iinclude -fno-builtin -c tools/rand_r_diff.c -o $(OUT)/rand-r-diff-tcc.o
	$(ZIG) cc -fsanitize=undefined -pthread -Iinclude -fno-builtin $(OUT)/rand-r-diff-tcc.o $(OUT)/libnelibc-tcc.a $(TCC_LIBTCC1) -o $(OUT)/rand-r-diff-tcc
	$(OUT)/rand-r-diff-tcc

rand-r-diff-matrix: rand-r-diff-zig rand-r-diff-gcc rand-r-diff-zig-musl rand-r-diff-tcc

.PHONY: rand-r-shootout-zig rand-r-shootout-gcc rand-r-shootout-zig-musl rand-r-shootout-tcc rand-r-shootout-matrix rand-r-diff-zig rand-r-diff-gcc rand-r-diff-zig-musl rand-r-diff-tcc rand-r-diff-matrix

.PHONY: int-shootout-tcc

alloc-shootout-zig: $(OUT) $(OUT)/memory-x86_64-simd.o $(OUT)/libnelibc-zig.a
	$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-O2 --cflags=-fno-builtin -o $(OUT)/memory-bench-zig.o src/memory.nelua
	$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-O2 --cflags=-fno-builtin -o $(OUT)/alloc-bench-zig.o src/alloc.nelua
	$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-O2 --cflags=-fno-builtin -o $(OUT)/sys-linux-x86_64-bench-zig.o src/sys/linux_x86_64.nelua
	$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-O2 --cflags=-fno-builtin -o $(OUT)/errno-host-bench-zig.o src/errno/host.nelua
	$(ZIG) cc -c src/sys/linux_x86_64.S -o $(OUT)/sys-linux-x86_64-bench-zig-asm.o
	$(ZIG) cc -O2 -Iinclude -fno-builtin -DNELIBC_ALLOCATOR_FASTPATH_SAFE -DBENCH_COMPILER='"Zig cc"' -DBENCH_LIBC='"glibc"' -DNELIBC_BENCH_GLIBC tools/alloc_bench.c $(OUT)/alloc-bench-zig.o $(OUT)/memory-bench-zig.o $(OUT)/memory-x86_64-simd.o $(OUT)/sys-linux-x86_64-bench-zig.o $(OUT)/sys-linux-x86_64-bench-zig-asm.o $(OUT)/errno-host-bench-zig.o $(OUT)/libnelibc-zig.a -o $(OUT)/alloc-shootout-zig
	$(OUT)/alloc-shootout-zig > $(OUT)/alloc-shootout-zig.csv
	cat $(OUT)/alloc-shootout-zig.csv

alloc-shootout-gcc: $(OUT) $(OUT)/memory-x86_64-simd.o $(OUT)/libnelibc-gcc.a
	$(NELUA) -B -r --cc gcc --cflags=-O2 --cflags=-fno-builtin -o $(OUT)/memory-bench-gcc.o src/memory.nelua
	$(NELUA) -B -r --cc gcc --cflags=-O2 --cflags=-fno-builtin -o $(OUT)/alloc-bench-gcc.o src/alloc.nelua
	$(NELUA) -B -r --cc gcc --cflags=-O2 --cflags=-fno-builtin -o $(OUT)/sys-linux-x86_64-bench-gcc.o src/sys/linux_x86_64.nelua
	$(NELUA) -B -r --cc gcc --cflags=-O2 --cflags=-fno-builtin -o $(OUT)/errno-host-bench-gcc.o src/errno/host.nelua
	gcc -c src/sys/linux_x86_64.S -o $(OUT)/sys-linux-x86_64-bench-gcc-asm.o
	gcc -O2 -Iinclude -fno-builtin -DNELIBC_ALLOCATOR_FASTPATH_SAFE -DBENCH_COMPILER='"GCC"' -DBENCH_LIBC='"glibc"' -DNELIBC_BENCH_GLIBC tools/alloc_bench.c $(OUT)/alloc-bench-gcc.o $(OUT)/memory-bench-gcc.o $(OUT)/memory-x86_64-simd.o $(OUT)/sys-linux-x86_64-bench-gcc.o $(OUT)/sys-linux-x86_64-bench-gcc-asm.o $(OUT)/errno-host-bench-gcc.o $(OUT)/libnelibc-gcc.a -o $(OUT)/alloc-shootout-gcc
	$(OUT)/alloc-shootout-gcc > $(OUT)/alloc-shootout-gcc.csv
	cat $(OUT)/alloc-shootout-gcc.csv

alloc-shootout-zig-musl: $(OUT) $(OUT)/memory-x86_64-simd.o
	$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-target=x86_64-linux-musl --cflags=-O2 --cflags=-fno-builtin -o $(OUT)/memory-bench-zig-musl.o src/memory.nelua
	$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-target=x86_64-linux-musl --cflags=-O2 --cflags=-fno-builtin -o $(OUT)/alloc-bench-zig-musl.o src/alloc.nelua
	$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-target=x86_64-linux-musl --cflags=-O2 --cflags=-fno-builtin -o $(OUT)/sys-linux-x86_64-bench-zig-musl.o src/sys/linux_x86_64.nelua
	$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-target=x86_64-linux-musl --cflags=-O2 --cflags=-fno-builtin -o $(OUT)/errno-host-bench-zig-musl.o src/errno/host.nelua
	$(ZIG) cc -target x86_64-linux-musl -c src/sys/linux_x86_64.S -o $(OUT)/sys-linux-x86_64-bench-zig-musl-asm.o
	$(ZIG) cc -target x86_64-linux-musl -Iinclude -fno-builtin -fno-sanitize=all -DNELIBC_STATIC_MUSL -c src/alloc_hybrid_adapter.c -o $(OUT)/alloc-hybrid-adapter-zig-musl-bench.o
	$(ZIG) cc -target x86_64-linux-musl -Iinclude -fno-builtin -fno-sanitize=all -DNELIBC_ALLOCATOR_FASTPATH_SAFE -c src/environment_adapter.c -o $(OUT)/environment-adapter-zig-musl-bench.o
	$(ZIG) cc -target x86_64-linux-musl -static -O2 -Iinclude -fno-builtin -Wl,--wrap=free -Wl,--wrap=realloc -DBENCH_COMPILER='"Zig cc"' -DBENCH_LIBC='"musl"' tools/alloc_bench.c $(OUT)/alloc-hybrid-adapter-zig-musl-bench.o $(OUT)/alloc-bench-zig-musl.o $(OUT)/memory-bench-zig-musl.o $(OUT)/memory-x86_64-simd.o $(OUT)/sys-linux-x86_64-bench-zig-musl.o $(OUT)/sys-linux-x86_64-bench-zig-musl-asm.o $(OUT)/errno-host-bench-zig-musl.o $(OUT)/environment-adapter-zig-musl-bench.o -o $(OUT)/alloc-shootout-zig-musl
	$(OUT)/alloc-shootout-zig-musl > $(OUT)/alloc-shootout-zig-musl.csv
	cat $(OUT)/alloc-shootout-zig-musl.csv

alloc-shootout-tcc: $(OUT) $(OUT)/memory-x86_64-simd.o $(OUT)/libnelibc-tcc.a
	$(NELUA) -B -r --cc '$(TCC)' --cflags=-O2 --cflags=-fno-builtin -o $(OUT)/memory-bench-tcc.o src/memory.nelua
	$(TCC) -c -fno-builtin src/memory_tcc_x86_64.c -o $(OUT)/memory-tcc-helper-bench.o
	$(ZIG) cc -c -DNELIBC_TCC_PUBLIC_MEMCPY src/memory_x86_64.S -o $(OUT)/memory-tcc-simd-bench.o
	$(NELUA) -B -r --cc '$(TCC)' --cflags=-O2 --cflags=-fno-builtin -o $(OUT)/alloc-bench-tcc.o src/alloc.nelua
	$(NELUA) -B -r --cc '$(TCC)' --cflags=-O2 --cflags=-fno-builtin -o $(OUT)/sys-linux-x86_64-bench-tcc.o src/sys/linux_x86_64.nelua
	$(NELUA) -B -r --cc '$(TCC)' --cflags=-O2 --cflags=-fno-builtin -o $(OUT)/errno-host-bench-tcc.o src/errno/host.nelua
	$(ZIG) cc -c src/sys/linux_x86_64.S -o $(OUT)/sys-linux-x86_64-bench-tcc-asm.o
	$(TCC) -O2 -Iinclude -fno-builtin -DNELIBC_ALLOCATOR_FASTPATH_SAFE -DBENCH_COMPILER='"TinyCC"' -DBENCH_LIBC='"glibc"' -DNELIBC_BENCH_GLIBC tools/alloc_bench.c $(OUT)/alloc-bench-tcc.o $(OUT)/memory-bench-tcc.o $(OUT)/memory-tcc-helper-bench.o $(OUT)/memory-tcc-simd-bench.o $(OUT)/sys-linux-x86_64-bench-tcc.o $(OUT)/sys-linux-x86_64-bench-tcc-asm.o $(OUT)/errno-host-bench-tcc.o $(OUT)/libnelibc-tcc.a -o $(OUT)/alloc-shootout-tcc
	$(OUT)/alloc-shootout-tcc > $(OUT)/alloc-shootout-tcc.csv
	cat $(OUT)/alloc-shootout-tcc.csv

alloc-shootout-matrix: alloc-shootout-zig alloc-shootout-gcc alloc-shootout-zig-musl alloc-shootout-tcc
	@awk 'FNR == 1 && NR > 1 { next } { print }' $(OUT)/alloc-shootout-zig.csv $(OUT)/alloc-shootout-gcc.csv $(OUT)/alloc-shootout-zig-musl.csv $(OUT)/alloc-shootout-tcc.csv > $(OUT)/alloc-shootout-matrix.csv
	@cat $(OUT)/alloc-shootout-matrix.csv

alloc-public-zig-musl-shootout: $(OUT)/libnelibc-zig-musl.a tools/alloc_public_musl_bench.c tools/alloc_public_musl_shootout.py
	$(ZIG) cc -target x86_64-linux-musl -static -O2 -Iinclude -fno-builtin tools/alloc_public_musl_bench.c -o $(OUT)/alloc-public-musl-host
	$(ZIG) cc -target x86_64-linux-musl -static -pthread -O2 -Iinclude -fno-builtin $(MUSL_STDIO_WRAP_FLAGS) -DBENCH_NELIBC_PUBLIC tools/alloc_public_musl_bench.c $(OUT)/libnelibc-zig-musl.a -o $(OUT)/alloc-public-musl-nelibc
	python3 tools/alloc_public_musl_shootout.py --host $(OUT)/alloc-public-musl-host --nelibc $(OUT)/alloc-public-musl-nelibc --output $(OUT)/alloc-public-zig-musl-shootout.csv --pairs 24

alloc-thread-shootout-zig: alloc-shootout-zig
	$(ZIG) cc -pthread -O2 -Iinclude -fno-builtin -DNELIBC_ALLOCATOR_FASTPATH_SAFE -DBENCH_COMPILER='"Zig cc"' -DBENCH_LIBC='"glibc"' tools/alloc_thread_bench.c $(OUT)/alloc-bench-zig.o $(OUT)/memory-bench-zig.o $(OUT)/memory-x86_64-simd.o $(OUT)/sys-linux-x86_64-bench-zig.o $(OUT)/sys-linux-x86_64-bench-zig-asm.o $(OUT)/errno-host-bench-zig.o src/environment_adapter.c -o $(OUT)/alloc-thread-shootout-zig
	$(OUT)/alloc-thread-shootout-zig > $(OUT)/alloc-thread-shootout-zig.csv
	cat $(OUT)/alloc-thread-shootout-zig.csv

alloc-thread-shootout-gcc: alloc-shootout-gcc
	gcc -pthread -O2 -Iinclude -fno-builtin -DNELIBC_ALLOCATOR_FASTPATH_SAFE -DBENCH_COMPILER='"GCC"' -DBENCH_LIBC='"glibc"' tools/alloc_thread_bench.c $(OUT)/alloc-bench-gcc.o $(OUT)/memory-bench-gcc.o $(OUT)/memory-x86_64-simd.o $(OUT)/sys-linux-x86_64-bench-gcc.o $(OUT)/sys-linux-x86_64-bench-gcc-asm.o $(OUT)/errno-host-bench-gcc.o src/environment_adapter.c -o $(OUT)/alloc-thread-shootout-gcc
	$(OUT)/alloc-thread-shootout-gcc > $(OUT)/alloc-thread-shootout-gcc.csv
	cat $(OUT)/alloc-thread-shootout-gcc.csv

alloc-thread-shootout-zig-musl: alloc-shootout-zig-musl
	$(ZIG) cc -pthread -target x86_64-linux-musl -static -O2 -Iinclude -fno-builtin -DNELIBC_ALLOCATOR_FASTPATH_SAFE -DBENCH_COMPILER='"Zig cc"' -DBENCH_LIBC='"musl"' tools/alloc_thread_bench.c $(OUT)/alloc-bench-zig-musl.o $(OUT)/memory-bench-zig-musl.o $(OUT)/memory-x86_64-simd.o $(OUT)/sys-linux-x86_64-bench-zig-musl.o $(OUT)/sys-linux-x86_64-bench-zig-musl-asm.o $(OUT)/errno-host-bench-zig-musl.o src/environment_adapter.c -o $(OUT)/alloc-thread-shootout-zig-musl
	$(OUT)/alloc-thread-shootout-zig-musl > $(OUT)/alloc-thread-shootout-zig-musl.csv
	cat $(OUT)/alloc-thread-shootout-zig-musl.csv

alloc-thread-shootout-tcc: alloc-shootout-tcc
	$(TCC) -pthread -O2 -Iinclude -fno-builtin -DNELIBC_ALLOCATOR_FASTPATH_SAFE -DBENCH_COMPILER='"TinyCC"' -DBENCH_LIBC='"glibc"' tools/alloc_thread_bench.c $(OUT)/alloc-bench-tcc.o $(OUT)/memory-bench-tcc.o $(OUT)/memory-tcc-helper-bench.o $(OUT)/memory-tcc-simd-bench.o $(OUT)/sys-linux-x86_64-bench-tcc.o $(OUT)/sys-linux-x86_64-bench-tcc-asm.o $(OUT)/errno-host-bench-tcc.o src/environment_adapter.c -o $(OUT)/alloc-thread-shootout-tcc
	$(OUT)/alloc-thread-shootout-tcc > $(OUT)/alloc-thread-shootout-tcc.csv
	cat $(OUT)/alloc-thread-shootout-tcc.csv

alloc-thread-shootout-matrix: alloc-thread-shootout-zig alloc-thread-shootout-gcc alloc-thread-shootout-zig-musl alloc-thread-shootout-tcc
	@awk 'FNR == 1 && NR > 1 { next } { print }' $(OUT)/alloc-thread-shootout-zig.csv $(OUT)/alloc-thread-shootout-gcc.csv $(OUT)/alloc-thread-shootout-zig-musl.csv $(OUT)/alloc-thread-shootout-tcc.csv > $(OUT)/alloc-thread-shootout-matrix.csv
	@cat $(OUT)/alloc-thread-shootout-matrix.csv

.PHONY: alloc-thread-shootout-zig alloc-thread-shootout-gcc alloc-thread-shootout-zig-musl alloc-thread-shootout-tcc alloc-thread-shootout-matrix

getentropy-shootout-zig: $(OUT)/libnelibc-zig.a
	$(ZIG) cc -O2 -fno-builtin -DBENCH_COMPILER='"Zig cc"' -DBENCH_LIBC='"glibc"' -DBENCH_IMPLEMENTATION='"host-libc"' tools/getentropy_bench.c -o $(OUT)/getentropy-host-zig
	$(ZIG) cc -O2 -fno-builtin -DBENCH_COMPILER='"Zig cc"' -DBENCH_LIBC='"glibc"' -DBENCH_IMPLEMENTATION='"nelibc"' tools/getentropy_bench.c $(OUT)/libnelibc-zig.a -o $(OUT)/getentropy-nelibc-zig
	@printf 'compiler,libc,implementation,function,iterations,median_ns_per_call\n' > $(OUT)/getentropy-shootout-zig.csv
	$(OUT)/getentropy-host-zig >> $(OUT)/getentropy-shootout-zig.csv
	$(OUT)/getentropy-nelibc-zig >> $(OUT)/getentropy-shootout-zig.csv
	cat $(OUT)/getentropy-shootout-zig.csv

getentropy-shootout-gcc: $(OUT)/libnelibc-gcc.a
	gcc -O2 -fno-builtin -DBENCH_COMPILER='"GCC"' -DBENCH_LIBC='"glibc"' -DBENCH_IMPLEMENTATION='"host-libc"' tools/getentropy_bench.c -o $(OUT)/getentropy-host-gcc
	gcc -O2 -fno-builtin -DBENCH_COMPILER='"GCC"' -DBENCH_LIBC='"glibc"' -DBENCH_IMPLEMENTATION='"nelibc"' tools/getentropy_bench.c $(OUT)/libnelibc-gcc.a -o $(OUT)/getentropy-nelibc-gcc
	@printf 'compiler,libc,implementation,function,iterations,median_ns_per_call\n' > $(OUT)/getentropy-shootout-gcc.csv
	$(OUT)/getentropy-host-gcc >> $(OUT)/getentropy-shootout-gcc.csv
	$(OUT)/getentropy-nelibc-gcc >> $(OUT)/getentropy-shootout-gcc.csv
	cat $(OUT)/getentropy-shootout-gcc.csv

getentropy-shootout-zig-musl: $(OUT)/libnelibc-zig-musl.a
	$(ZIG) cc -target x86_64-linux-musl -static -O2 -fno-builtin -DBENCH_COMPILER='"Zig cc"' -DBENCH_LIBC='"musl"' -DBENCH_IMPLEMENTATION='"host-libc"' tools/getentropy_bench.c -o $(OUT)/getentropy-host-zig-musl
	$(ZIG) cc -target x86_64-linux-musl -static -O2 -fno-builtin $(MUSL_STDIO_WRAP_FLAGS) -DBENCH_COMPILER='"Zig cc"' -DBENCH_LIBC='"musl"' -DBENCH_IMPLEMENTATION='"nelibc"' tools/getentropy_bench.c $(OUT)/libnelibc-zig-musl.a -o $(OUT)/getentropy-nelibc-zig-musl
	@printf 'compiler,libc,implementation,function,iterations,median_ns_per_call\n' > $(OUT)/getentropy-shootout-zig-musl.csv
	$(OUT)/getentropy-host-zig-musl >> $(OUT)/getentropy-shootout-zig-musl.csv
	$(OUT)/getentropy-nelibc-zig-musl >> $(OUT)/getentropy-shootout-zig-musl.csv
	cat $(OUT)/getentropy-shootout-zig-musl.csv

getentropy-shootout-tcc: $(OUT)/libnelibc-tcc.a
	$(TCC) -O2 -fno-builtin -DBENCH_COMPILER='"TinyCC"' -DBENCH_LIBC='"glibc"' -DBENCH_IMPLEMENTATION='"host-libc"' tools/getentropy_bench.c -o $(OUT)/getentropy-host-tcc
	$(TCC) -O2 -fno-builtin -DBENCH_COMPILER='"TinyCC"' -DBENCH_LIBC='"glibc"' -DBENCH_IMPLEMENTATION='"nelibc"' tools/getentropy_bench.c $(OUT)/libnelibc-tcc.a -o $(OUT)/getentropy-nelibc-tcc
	@printf 'compiler,libc,implementation,function,iterations,median_ns_per_call\n' > $(OUT)/getentropy-shootout-tcc.csv
	$(OUT)/getentropy-host-tcc >> $(OUT)/getentropy-shootout-tcc.csv
	$(OUT)/getentropy-nelibc-tcc >> $(OUT)/getentropy-shootout-tcc.csv
	cat $(OUT)/getentropy-shootout-tcc.csv

getentropy-shootout-matrix: getentropy-shootout-zig getentropy-shootout-gcc getentropy-shootout-zig-musl getentropy-shootout-tcc
	@awk 'FNR == 1 && NR > 1 { next } { print }' $(OUT)/getentropy-shootout-zig.csv $(OUT)/getentropy-shootout-gcc.csv $(OUT)/getentropy-shootout-zig-musl.csv $(OUT)/getentropy-shootout-tcc.csv > $(OUT)/getentropy-shootout-matrix.csv
	@cat $(OUT)/getentropy-shootout-matrix.csv

.PHONY: getentropy-shootout-zig getentropy-shootout-gcc getentropy-shootout-zig-musl getentropy-shootout-tcc getentropy-shootout-matrix

process-id-shootout-zig: $(OUT)/libnelibc-zig.a
	$(ZIG) cc -O2 -fno-builtin -DBENCH_COMPILER='"Zig cc"' -DBENCH_LIBC='"glibc"' -DBENCH_IMPLEMENTATION='"host-libc"' tools/process_id_bench.c -o $(OUT)/process-id-host-zig
	$(ZIG) cc -O2 -fno-builtin -DBENCH_COMPILER='"Zig cc"' -DBENCH_LIBC='"glibc"' -DBENCH_IMPLEMENTATION='"nelibc"' tools/process_id_bench.c $(OUT)/libnelibc-zig.a -o $(OUT)/process-id-nelibc-zig
	@printf 'compiler,libc,implementation,function,iterations,median_ns_per_call\n' > $(OUT)/process-id-shootout-zig.csv
	$(OUT)/process-id-host-zig >> $(OUT)/process-id-shootout-zig.csv
	$(OUT)/process-id-nelibc-zig >> $(OUT)/process-id-shootout-zig.csv
	cat $(OUT)/process-id-shootout-zig.csv

process-id-shootout-gcc: $(OUT)/libnelibc-gcc.a
	gcc -O2 -fno-builtin -DBENCH_COMPILER='"GCC"' -DBENCH_LIBC='"glibc"' -DBENCH_IMPLEMENTATION='"host-libc"' tools/process_id_bench.c -o $(OUT)/process-id-host-gcc
	gcc -O2 -fno-builtin -DBENCH_COMPILER='"GCC"' -DBENCH_LIBC='"glibc"' -DBENCH_IMPLEMENTATION='"nelibc"' tools/process_id_bench.c $(OUT)/libnelibc-gcc.a -o $(OUT)/process-id-nelibc-gcc
	@printf 'compiler,libc,implementation,function,iterations,median_ns_per_call\n' > $(OUT)/process-id-shootout-gcc.csv
	$(OUT)/process-id-host-gcc >> $(OUT)/process-id-shootout-gcc.csv
	$(OUT)/process-id-nelibc-gcc >> $(OUT)/process-id-shootout-gcc.csv
	cat $(OUT)/process-id-shootout-gcc.csv

process-id-shootout-zig-musl: $(OUT)/libnelibc-zig-musl.a
	$(ZIG) cc -target x86_64-linux-musl -static -O2 -fno-builtin -DBENCH_COMPILER='"Zig cc"' -DBENCH_LIBC='"musl"' -DBENCH_IMPLEMENTATION='"host-libc"' tools/process_id_bench.c -o $(OUT)/process-id-host-zig-musl
	$(ZIG) cc -target x86_64-linux-musl -static -O2 -fno-builtin $(MUSL_STDIO_WRAP_FLAGS) -DBENCH_COMPILER='"Zig cc"' -DBENCH_LIBC='"musl"' -DBENCH_IMPLEMENTATION='"nelibc"' tools/process_id_bench.c $(OUT)/libnelibc-zig-musl.a -o $(OUT)/process-id-nelibc-zig-musl
	@printf 'compiler,libc,implementation,function,iterations,median_ns_per_call\n' > $(OUT)/process-id-shootout-zig-musl.csv
	$(OUT)/process-id-host-zig-musl >> $(OUT)/process-id-shootout-zig-musl.csv
	$(OUT)/process-id-nelibc-zig-musl >> $(OUT)/process-id-shootout-zig-musl.csv
	cat $(OUT)/process-id-shootout-zig-musl.csv

process-id-shootout-tcc: $(OUT)/libnelibc-tcc.a
	$(TCC) -O2 -fno-builtin -DBENCH_COMPILER='"TinyCC"' -DBENCH_LIBC='"glibc"' -DBENCH_IMPLEMENTATION='"host-libc"' tools/process_id_bench.c -o $(OUT)/process-id-host-tcc
	$(TCC) -O2 -fno-builtin -DBENCH_COMPILER='"TinyCC"' -DBENCH_LIBC='"glibc"' -DBENCH_IMPLEMENTATION='"nelibc"' tools/process_id_bench.c $(OUT)/libnelibc-tcc.a -o $(OUT)/process-id-nelibc-tcc
	@printf 'compiler,libc,implementation,function,iterations,median_ns_per_call\n' > $(OUT)/process-id-shootout-tcc.csv
	$(OUT)/process-id-host-tcc >> $(OUT)/process-id-shootout-tcc.csv
	$(OUT)/process-id-nelibc-tcc >> $(OUT)/process-id-shootout-tcc.csv
	cat $(OUT)/process-id-shootout-tcc.csv

process-id-shootout-matrix: process-id-shootout-zig process-id-shootout-gcc process-id-shootout-zig-musl process-id-shootout-tcc
	@awk 'FNR == 1 && NR > 1 { next } { print }' $(OUT)/process-id-shootout-zig.csv $(OUT)/process-id-shootout-gcc.csv $(OUT)/process-id-shootout-zig-musl.csv $(OUT)/process-id-shootout-tcc.csv > $(OUT)/process-id-shootout-matrix.csv
	@cat $(OUT)/process-id-shootout-matrix.csv

.PHONY: process-id-shootout-zig process-id-shootout-gcc process-id-shootout-zig-musl process-id-shootout-tcc process-id-shootout-matrix

eventfd-shootout-zig: $(OUT)/libnelibc-zig.a
	$(ZIG) cc -O2 -fno-builtin -DBENCH_COMPILER='"Zig cc"' -DBENCH_LIBC='"glibc"' -DBENCH_IMPLEMENTATION='"host-libc"' tools/eventfd_bench.c -o $(OUT)/eventfd-host-zig
	$(ZIG) cc -O2 -fno-builtin -DBENCH_COMPILER='"Zig cc"' -DBENCH_LIBC='"glibc"' -DBENCH_IMPLEMENTATION='"nelibc"' tools/eventfd_bench.c $(OUT)/libnelibc-zig.a -o $(OUT)/eventfd-nelibc-zig
	@printf 'compiler,libc,implementation,function,iterations,median_ns_per_call\n' > $(OUT)/eventfd-shootout-zig.csv
	$(OUT)/eventfd-host-zig >> $(OUT)/eventfd-shootout-zig.csv
	$(OUT)/eventfd-nelibc-zig >> $(OUT)/eventfd-shootout-zig.csv
	cat $(OUT)/eventfd-shootout-zig.csv

eventfd-shootout-gcc: $(OUT)/libnelibc-gcc.a
	gcc -O2 -fno-builtin -DBENCH_COMPILER='"GCC"' -DBENCH_LIBC='"glibc"' -DBENCH_IMPLEMENTATION='"host-libc"' tools/eventfd_bench.c -o $(OUT)/eventfd-host-gcc
	gcc -O2 -fno-builtin -DBENCH_COMPILER='"GCC"' -DBENCH_LIBC='"glibc"' -DBENCH_IMPLEMENTATION='"nelibc"' tools/eventfd_bench.c $(OUT)/libnelibc-gcc.a -o $(OUT)/eventfd-nelibc-gcc
	@printf 'compiler,libc,implementation,function,iterations,median_ns_per_call\n' > $(OUT)/eventfd-shootout-gcc.csv
	$(OUT)/eventfd-host-gcc >> $(OUT)/eventfd-shootout-gcc.csv
	$(OUT)/eventfd-nelibc-gcc >> $(OUT)/eventfd-shootout-gcc.csv
	cat $(OUT)/eventfd-shootout-gcc.csv

eventfd-shootout-zig-musl: $(OUT)/libnelibc-zig-musl.a
	$(ZIG) cc -target x86_64-linux-musl -static -O2 -fno-builtin -DBENCH_COMPILER='"Zig cc"' -DBENCH_LIBC='"musl"' -DBENCH_IMPLEMENTATION='"host-libc"' tools/eventfd_bench.c -o $(OUT)/eventfd-host-zig-musl
	$(ZIG) cc -target x86_64-linux-musl -static -O2 -fno-builtin $(MUSL_STDIO_WRAP_FLAGS) -DBENCH_COMPILER='"Zig cc"' -DBENCH_LIBC='"musl"' -DBENCH_IMPLEMENTATION='"nelibc"' tools/eventfd_bench.c $(OUT)/libnelibc-zig-musl.a -o $(OUT)/eventfd-nelibc-zig-musl
	@printf 'compiler,libc,implementation,function,iterations,median_ns_per_call\n' > $(OUT)/eventfd-shootout-zig-musl.csv
	$(OUT)/eventfd-host-zig-musl >> $(OUT)/eventfd-shootout-zig-musl.csv
	$(OUT)/eventfd-nelibc-zig-musl >> $(OUT)/eventfd-shootout-zig-musl.csv
	cat $(OUT)/eventfd-shootout-zig-musl.csv

eventfd-shootout-tcc: $(OUT)/libnelibc-tcc.a
	$(TCC) -O2 -fno-builtin -DBENCH_COMPILER='"TinyCC"' -DBENCH_LIBC='"glibc"' -DBENCH_IMPLEMENTATION='"host-libc"' tools/eventfd_bench.c -o $(OUT)/eventfd-host-tcc
	$(TCC) -O2 -fno-builtin -DBENCH_COMPILER='"TinyCC"' -DBENCH_LIBC='"glibc"' -DBENCH_IMPLEMENTATION='"nelibc"' tools/eventfd_bench.c $(OUT)/libnelibc-tcc.a -o $(OUT)/eventfd-nelibc-tcc
	@printf 'compiler,libc,implementation,function,iterations,median_ns_per_call\n' > $(OUT)/eventfd-shootout-tcc.csv
	$(OUT)/eventfd-host-tcc >> $(OUT)/eventfd-shootout-tcc.csv
	$(OUT)/eventfd-nelibc-tcc >> $(OUT)/eventfd-shootout-tcc.csv
	cat $(OUT)/eventfd-shootout-tcc.csv

eventfd-shootout-matrix: eventfd-shootout-zig eventfd-shootout-gcc eventfd-shootout-zig-musl eventfd-shootout-tcc
	@awk 'FNR == 1 && NR > 1 { next } { print }' $(OUT)/eventfd-shootout-zig.csv $(OUT)/eventfd-shootout-gcc.csv $(OUT)/eventfd-shootout-zig-musl.csv $(OUT)/eventfd-shootout-tcc.csv > $(OUT)/eventfd-shootout-matrix.csv
	@cat $(OUT)/eventfd-shootout-matrix.csv

.PHONY: eventfd-shootout-zig eventfd-shootout-gcc eventfd-shootout-zig-musl eventfd-shootout-tcc eventfd-shootout-matrix

dirent-shootout-zig: $(OUT)/libnelibc-zig.a
	$(ZIG) cc -O2 -fno-builtin -DBENCH_COMPILER='"Zig cc"' -DBENCH_LIBC='"glibc"' -DBENCH_IMPLEMENTATION='"host-libc"' tools/dirent_bench.c -o $(OUT)/dirent-host-zig
	$(ZIG) cc -O2 -fno-builtin -DBENCH_COMPILER='"Zig cc"' -DBENCH_LIBC='"glibc"' -DBENCH_IMPLEMENTATION='"nelibc"' tools/dirent_bench.c $(OUT)/libnelibc-zig.a -o $(OUT)/dirent-nelibc-zig
	@printf 'compiler,libc,implementation,function,iterations,median_ns_per_call\n' > $(OUT)/dirent-shootout-zig.csv
	$(OUT)/dirent-host-zig >> $(OUT)/dirent-shootout-zig.csv
	$(OUT)/dirent-nelibc-zig >> $(OUT)/dirent-shootout-zig.csv
	cat $(OUT)/dirent-shootout-zig.csv

dirent-shootout-gcc: $(OUT)/libnelibc-gcc.a
	gcc -O2 -fno-builtin -DBENCH_COMPILER='"GCC"' -DBENCH_LIBC='"glibc"' -DBENCH_IMPLEMENTATION='"host-libc"' tools/dirent_bench.c -o $(OUT)/dirent-host-gcc
	gcc -O2 -fno-builtin -DBENCH_COMPILER='"GCC"' -DBENCH_LIBC='"glibc"' -DBENCH_IMPLEMENTATION='"nelibc"' tools/dirent_bench.c $(OUT)/libnelibc-gcc.a -o $(OUT)/dirent-nelibc-gcc
	@printf 'compiler,libc,implementation,function,iterations,median_ns_per_call\n' > $(OUT)/dirent-shootout-gcc.csv
	$(OUT)/dirent-host-gcc >> $(OUT)/dirent-shootout-gcc.csv
	$(OUT)/dirent-nelibc-gcc >> $(OUT)/dirent-shootout-gcc.csv
	cat $(OUT)/dirent-shootout-gcc.csv

dirent-shootout-zig-musl: $(OUT)/libnelibc-zig-musl.a
	$(ZIG) cc -target x86_64-linux-musl -static -O2 -fno-builtin -DBENCH_COMPILER='"Zig cc"' -DBENCH_LIBC='"musl"' -DBENCH_IMPLEMENTATION='"host-libc"' tools/dirent_bench.c -o $(OUT)/dirent-host-zig-musl
	$(ZIG) cc -target x86_64-linux-musl -static -O2 -fno-builtin $(MUSL_STDIO_WRAP_FLAGS) -DBENCH_COMPILER='"Zig cc"' -DBENCH_LIBC='"musl"' -DBENCH_IMPLEMENTATION='"nelibc"' tools/dirent_bench.c $(OUT)/libnelibc-zig-musl.a -o $(OUT)/dirent-nelibc-zig-musl
	@printf 'compiler,libc,implementation,function,iterations,median_ns_per_call\n' > $(OUT)/dirent-shootout-zig-musl.csv
	$(OUT)/dirent-host-zig-musl >> $(OUT)/dirent-shootout-zig-musl.csv
	$(OUT)/dirent-nelibc-zig-musl >> $(OUT)/dirent-shootout-zig-musl.csv
	cat $(OUT)/dirent-shootout-zig-musl.csv

dirent-shootout-tcc: $(OUT)/libnelibc-tcc.a
	$(TCC) -O2 -fno-builtin -DBENCH_COMPILER='"TinyCC"' -DBENCH_LIBC='"glibc"' -DBENCH_IMPLEMENTATION='"host-libc"' tools/dirent_bench.c -o $(OUT)/dirent-host-tcc
	$(TCC) -O2 -fno-builtin -DBENCH_COMPILER='"TinyCC"' -DBENCH_LIBC='"glibc"' -DBENCH_IMPLEMENTATION='"nelibc"' tools/dirent_bench.c $(OUT)/libnelibc-tcc.a -o $(OUT)/dirent-nelibc-tcc
	@printf 'compiler,libc,implementation,function,iterations,median_ns_per_call\n' > $(OUT)/dirent-shootout-tcc.csv
	$(OUT)/dirent-host-tcc >> $(OUT)/dirent-shootout-tcc.csv
	$(OUT)/dirent-nelibc-tcc >> $(OUT)/dirent-shootout-tcc.csv
	cat $(OUT)/dirent-shootout-tcc.csv

dirent-shootout-matrix: dirent-shootout-zig dirent-shootout-gcc dirent-shootout-zig-musl dirent-shootout-tcc
	@awk 'FNR == 1 && NR > 1 { next } { print }' $(OUT)/dirent-shootout-zig.csv $(OUT)/dirent-shootout-gcc.csv $(OUT)/dirent-shootout-zig-musl.csv $(OUT)/dirent-shootout-tcc.csv > $(OUT)/dirent-shootout-matrix.csv
	@cat $(OUT)/dirent-shootout-matrix.csv

.PHONY: dirent-shootout-zig dirent-shootout-gcc dirent-shootout-zig-musl dirent-shootout-tcc dirent-shootout-matrix

waitpid-shootout-zig: $(OUT)/libnelibc-zig.a
	$(ZIG) cc -pthread -O2 -fno-builtin -DBENCH_COMPILER='"Zig cc"' -DBENCH_LIBC='"glibc"' tools/waitpid_bench.c -DBENCH_IMPLEMENTATION='"host-libc"' -o $(OUT)/waitpid-host-zig
	$(ZIG) cc -pthread -O2 -fno-builtin -DBENCH_COMPILER='"Zig cc"' -DBENCH_LIBC='"glibc"' -DBENCH_IMPLEMENTATION='"nelibc"' tools/waitpid_bench.c $(OUT)/libnelibc-zig.a -o $(OUT)/waitpid-nelibc-zig
	@printf 'compiler,libc,implementation,function,iterations,median_ns_per_call\n' > $(OUT)/waitpid-shootout-zig.csv
	$(OUT)/waitpid-host-zig >> $(OUT)/waitpid-shootout-zig.csv
	$(OUT)/waitpid-nelibc-zig >> $(OUT)/waitpid-shootout-zig.csv
	cat $(OUT)/waitpid-shootout-zig.csv

waitpid-shootout-gcc: $(OUT)/libnelibc-gcc.a
	gcc -pthread -O2 -fno-builtin -DBENCH_COMPILER='"GCC"' -DBENCH_LIBC='"glibc"' tools/waitpid_bench.c -DBENCH_IMPLEMENTATION='"host-libc"' -o $(OUT)/waitpid-host-gcc
	gcc -pthread -O2 -fno-builtin -DBENCH_COMPILER='"GCC"' -DBENCH_LIBC='"glibc"' -DBENCH_IMPLEMENTATION='"nelibc"' tools/waitpid_bench.c $(OUT)/libnelibc-gcc.a -o $(OUT)/waitpid-nelibc-gcc
	@printf 'compiler,libc,implementation,function,iterations,median_ns_per_call\n' > $(OUT)/waitpid-shootout-gcc.csv
	$(OUT)/waitpid-host-gcc >> $(OUT)/waitpid-shootout-gcc.csv
	$(OUT)/waitpid-nelibc-gcc >> $(OUT)/waitpid-shootout-gcc.csv
	cat $(OUT)/waitpid-shootout-gcc.csv

waitpid-shootout-zig-musl: $(OUT)/libnelibc-zig-musl.a
	$(ZIG) cc -target x86_64-linux-musl -static -pthread -O2 -fno-builtin -DBENCH_COMPILER='"Zig cc"' -DBENCH_LIBC='"musl"' tools/waitpid_bench.c -DBENCH_IMPLEMENTATION='"host-libc"' -o $(OUT)/waitpid-host-zig-musl
	$(ZIG) cc -target x86_64-linux-musl -static -pthread -O2 -fno-builtin $(MUSL_STDIO_WRAP_FLAGS) -DBENCH_COMPILER='"Zig cc"' -DBENCH_LIBC='"musl"' -DBENCH_IMPLEMENTATION='"nelibc"' tools/waitpid_bench.c $(OUT)/libnelibc-zig-musl.a -o $(OUT)/waitpid-nelibc-zig-musl
	@printf 'compiler,libc,implementation,function,iterations,median_ns_per_call\n' > $(OUT)/waitpid-shootout-zig-musl.csv
	$(OUT)/waitpid-host-zig-musl >> $(OUT)/waitpid-shootout-zig-musl.csv
	$(OUT)/waitpid-nelibc-zig-musl >> $(OUT)/waitpid-shootout-zig-musl.csv
	cat $(OUT)/waitpid-shootout-zig-musl.csv

waitpid-shootout-tcc: $(OUT)/libnelibc-tcc.a
	$(TCC) -pthread -O2 -fno-builtin -DBENCH_COMPILER='"TinyCC"' -DBENCH_LIBC='"glibc"' tools/waitpid_bench.c -DBENCH_IMPLEMENTATION='"host-libc"' -o $(OUT)/waitpid-host-tcc
	$(TCC) -pthread -O2 -fno-builtin -DBENCH_COMPILER='"TinyCC"' -DBENCH_LIBC='"glibc"' -DBENCH_IMPLEMENTATION='"nelibc"' tools/waitpid_bench.c $(OUT)/libnelibc-tcc.a -o $(OUT)/waitpid-nelibc-tcc
	@printf 'compiler,libc,implementation,function,iterations,median_ns_per_call\n' > $(OUT)/waitpid-shootout-tcc.csv
	$(OUT)/waitpid-host-tcc >> $(OUT)/waitpid-shootout-tcc.csv
	$(OUT)/waitpid-nelibc-tcc >> $(OUT)/waitpid-shootout-tcc.csv
	cat $(OUT)/waitpid-shootout-tcc.csv

waitpid-shootout-matrix: waitpid-shootout-zig waitpid-shootout-gcc waitpid-shootout-zig-musl waitpid-shootout-tcc
	@awk 'FNR == 1 && NR > 1 { next } { print }' $(OUT)/waitpid-shootout-zig.csv $(OUT)/waitpid-shootout-gcc.csv $(OUT)/waitpid-shootout-zig-musl.csv $(OUT)/waitpid-shootout-tcc.csv > $(OUT)/waitpid-shootout-matrix.csv
	@cat $(OUT)/waitpid-shootout-matrix.csv

.PHONY: waitpid-shootout-zig waitpid-shootout-gcc waitpid-shootout-zig-musl waitpid-shootout-tcc waitpid-shootout-matrix

io-shootout-zig: $(OUT)/libnelibc-zig.a
	$(ZIG) cc -O2 -fno-builtin -DBENCH_COMPILER='"Zig cc"' -DBENCH_LIBC='"glibc"' -DBENCH_IMPLEMENTATION='"host-libc"' tools/io_bench.c -o $(OUT)/io-host-zig
	$(ZIG) cc -O2 -fno-builtin -DBENCH_COMPILER='"Zig cc"' -DBENCH_LIBC='"glibc"' -DBENCH_IMPLEMENTATION='"nelibc"' tools/io_bench.c $(OUT)/libnelibc-zig.a -o $(OUT)/io-nelibc-zig
	@printf 'compiler,libc,implementation,function,iterations,median_ns_per_call\n' > $(OUT)/io-shootout-zig.csv
	$(OUT)/io-host-zig >> $(OUT)/io-shootout-zig.csv
	$(OUT)/io-nelibc-zig >> $(OUT)/io-shootout-zig.csv
	cat $(OUT)/io-shootout-zig.csv

io-shootout-gcc: $(OUT)/libnelibc-gcc.a
	gcc -O2 -fno-builtin -DBENCH_COMPILER='"GCC"' -DBENCH_LIBC='"glibc"' -DBENCH_IMPLEMENTATION='"host-libc"' tools/io_bench.c -o $(OUT)/io-host-gcc
	gcc -O2 -fno-builtin -DBENCH_COMPILER='"GCC"' -DBENCH_LIBC='"glibc"' -DBENCH_IMPLEMENTATION='"nelibc"' tools/io_bench.c $(OUT)/libnelibc-gcc.a -o $(OUT)/io-nelibc-gcc
	@printf 'compiler,libc,implementation,function,iterations,median_ns_per_call\n' > $(OUT)/io-shootout-gcc.csv
	$(OUT)/io-host-gcc >> $(OUT)/io-shootout-gcc.csv
	$(OUT)/io-nelibc-gcc >> $(OUT)/io-shootout-gcc.csv
	cat $(OUT)/io-shootout-gcc.csv

io-shootout-zig-musl: $(OUT)/libnelibc-zig-musl.a
	$(ZIG) cc -target x86_64-linux-musl -static -O2 -fno-builtin -DBENCH_COMPILER='"Zig cc"' -DBENCH_LIBC='"musl"' -DBENCH_IMPLEMENTATION='"host-libc"' tools/io_bench.c -o $(OUT)/io-host-zig-musl
	$(ZIG) cc -target x86_64-linux-musl -static -O2 -fno-builtin $(MUSL_STDIO_WRAP_FLAGS) -DBENCH_COMPILER='"Zig cc"' -DBENCH_LIBC='"musl"' -DBENCH_IMPLEMENTATION='"nelibc"' tools/io_bench.c $(OUT)/libnelibc-zig-musl.a -o $(OUT)/io-nelibc-zig-musl
	@printf 'compiler,libc,implementation,function,iterations,median_ns_per_call\n' > $(OUT)/io-shootout-zig-musl.csv
	$(OUT)/io-host-zig-musl >> $(OUT)/io-shootout-zig-musl.csv
	$(OUT)/io-nelibc-zig-musl >> $(OUT)/io-shootout-zig-musl.csv
	cat $(OUT)/io-shootout-zig-musl.csv

io-shootout-tcc: $(OUT)/libnelibc-tcc.a
	$(TCC) -O2 -fno-builtin -DBENCH_COMPILER='"TinyCC"' -DBENCH_LIBC='"glibc"' -DBENCH_IMPLEMENTATION='"host-libc"' tools/io_bench.c -o $(OUT)/io-host-tcc
	$(TCC) -O2 -fno-builtin -DBENCH_COMPILER='"TinyCC"' -DBENCH_LIBC='"glibc"' -DBENCH_IMPLEMENTATION='"nelibc"' tools/io_bench.c $(OUT)/libnelibc-tcc.a -o $(OUT)/io-nelibc-tcc
	@printf 'compiler,libc,implementation,function,iterations,median_ns_per_call\n' > $(OUT)/io-shootout-tcc.csv
	$(OUT)/io-host-tcc >> $(OUT)/io-shootout-tcc.csv
	$(OUT)/io-nelibc-tcc >> $(OUT)/io-shootout-tcc.csv
	cat $(OUT)/io-shootout-tcc.csv

io-shootout-matrix: io-shootout-zig io-shootout-gcc io-shootout-zig-musl io-shootout-tcc
	@awk 'FNR == 1 && NR > 1 { next } { print }' $(OUT)/io-shootout-zig.csv $(OUT)/io-shootout-gcc.csv $(OUT)/io-shootout-zig-musl.csv $(OUT)/io-shootout-tcc.csv > $(OUT)/io-shootout-matrix.csv
	@cat $(OUT)/io-shootout-matrix.csv

.PHONY: io-shootout-zig io-shootout-gcc io-shootout-zig-musl io-shootout-tcc io-shootout-matrix

unistd-cancel-zig: $(OUT)/libnelibc-zig.a tools/unistd_cancel_smoke.c
	$(ZIG) cc -pthread -Iinclude -fno-builtin tools/unistd_cancel_smoke.c $(OUT)/libnelibc-zig.a -o $(OUT)/unistd-cancel-zig
	$(OUT)/unistd-cancel-zig

unistd-cancel-gcc: $(OUT)/libnelibc-gcc.a tools/unistd_cancel_smoke.c
	gcc -pthread -Iinclude -fno-builtin tools/unistd_cancel_smoke.c $(OUT)/libnelibc-gcc.a -o $(OUT)/unistd-cancel-gcc
	$(OUT)/unistd-cancel-gcc

unistd-cancel-zig-musl: $(OUT)/libnelibc-zig-musl.a tools/unistd_cancel_smoke.c
	$(ZIG) cc -target x86_64-linux-musl -static -pthread -Iinclude -fno-builtin $(MUSL_STDIO_WRAP_FLAGS) tools/unistd_cancel_smoke.c $(OUT)/libnelibc-zig-musl.a -o $(OUT)/unistd-cancel-zig-musl
	$(OUT)/unistd-cancel-zig-musl

unistd-cancel-tcc: $(OUT)/libnelibc-tcc.a tools/unistd_cancel_smoke.c
	$(TCC) -pthread -Iinclude -fno-builtin -c tools/unistd_cancel_smoke.c -o $(OUT)/unistd-cancel-tcc.o
	$(ZIG) cc -fsanitize=undefined -pthread -Iinclude -fno-builtin $(OUT)/unistd-cancel-tcc.o $(OUT)/libnelibc-tcc.a $(TCC_LIBTCC1) -o $(OUT)/unistd-cancel-tcc
	$(OUT)/unistd-cancel-tcc

unistd-cancel-matrix: unistd-cancel-zig unistd-cancel-gcc unistd-cancel-zig-musl unistd-cancel-tcc

.PHONY: unistd-cancel-zig unistd-cancel-gcc unistd-cancel-zig-musl unistd-cancel-tcc unistd-cancel-matrix

musl: $(OUT) $(OUT)/memory-x86_64-simd.o $(OUT)/string-locale-musl.o $(OUT)/string-locale-lock-musl.o
	$(NELUA) -B -r --cc musl-gcc --cflags=-fno-builtin -o $(OUT)/string-musl.o $(SOURCE)
	$(NELUA) -B -r --cc musl-gcc --cflags=-fno-builtin -o $(OUT)/memory-musl.o src/memory.nelua
	$(NELUA) -B -r --cc musl-gcc --cflags=-fno-builtin -o $(OUT)/ctype-musl.o src/ctype.nelua
	$(NELUA) -B -r --cc musl-gcc --cflags=-fno-builtin -o $(OUT)/stdlib-musl.o src/stdlib.nelua
	$(OBJCOPY) --localize-symbol=nelibc_test_qsort_depth_fallbacks $(OUT)/stdlib-musl.o
	$(NELUA) -B -r --cc musl-gcc --cflags=-fno-builtin -o $(OUT)/stdlib-float-musl.o src/stdlib_float.nelua
	$(NELUA) -B -r --cc musl-gcc --cflags=-fno-builtin -o $(OUT)/errno-host-musl.o src/errno/host.nelua
	musl-gcc -Iinclude -fno-builtin tools/abi_smoke.c $(OUT)/string-locale-musl.o $(OUT)/string-locale-lock-musl.o $(OUT)/string-musl.o $(OUT)/memory-musl.o $(OUT)/memory-x86_64-simd.o $(OUT)/ctype-musl.o $(OUT)/stdlib-musl.o $(OUT)/stdlib-float-musl.o $(OUT)/errno-host-musl.o src/environment_adapter.c -o $(OUT)/abi-smoke-musl

smoke-musl: musl
	$(OUT)/abi-smoke-musl

tcc: $(OUT) $(OUT)/memory-tcc-simd.o $(OUT)/string-locale-tcc.o $(OUT)/string-locale-lock-tcc.o
	$(NELUA) -B -r --cc '$(TCC)' --cflags=-fno-builtin -o $(OUT)/string-tcc.o $(SOURCE)
	$(NELUA) -B -r --cc '$(TCC)' --cflags=-fno-builtin -o $(OUT)/memory-tcc.o src/memory.nelua
	$(TCC) -c -fno-builtin src/memory_tcc_x86_64.c -o $(OUT)/memory-tcc-helper.o
	$(NELUA) -B -r --cc '$(TCC)' --cflags=-fno-builtin -o $(OUT)/ctype-tcc.o src/ctype.nelua
	$(NELUA) --print-code -r src/stdlib.nelua > $(OUT)/stdlib-tcc-generated.c
	$(ZIG) cc -O2 -DNELIBC_QSORT_HOLE_SIFT -fno-builtin -Iinclude -fno-sanitize=all -c $(OUT)/stdlib-tcc-generated.c -o $(OUT)/stdlib-tcc.o
	$(OBJCOPY) --localize-symbol=nelibc_test_qsort_depth_fallbacks $(OUT)/stdlib-tcc.o
	$(NELUA) -B -r --cc '$(TCC)' --cflags=-fno-builtin -o $(OUT)/stdlib-float-tcc.o src/stdlib_float.nelua
	$(NELUA) -B -r --cc '$(TCC)' --cflags=-fno-builtin -o $(OUT)/errno-host-tcc.o src/errno/host.nelua
	$(NELUA) -B -r --cc '$(TCC)' --cflags=-fno-builtin -o $(OUT)/stdio-hybrid-tcc.o src/stdio_hybrid.nelua
	$(NELUA) -B -r --cc '$(TCC)' --cflags=-fno-builtin -o $(OUT)/open-memstream-hybrid-tcc.o src/open_memstream_hybrid.nelua
	$(NELUA) -B -r --cc '$(TCC)' --cflags=-fno-builtin -o $(OUT)/fmemopen-hybrid-tcc.o src/fmemopen_hybrid.nelua
	# TinyCC lacks C TLS; compile the adapter used by TCC smoke programs with Zig cc.
	$(ZIG) cc -pthread -Iinclude -fno-builtin -fno-sanitize=all -c src/environment_adapter.c -o $(OUT)/environment-adapter-smoke-tcc.o
	$(TCC) -fno-builtin tools/memory_tcc_diff.c $(OUT)/memory-tcc-simd.o -o $(OUT)/memory-tcc-diff
	$(TCC) -Iinclude -fno-builtin tools/memory_diff.c $(OUT)/memory-tcc.o $(OUT)/memory-tcc-helper.o $(OUT)/memory-tcc-simd.o -o $(OUT)/memory-diff-tcc
	$(TCC) -pthread -Iinclude -fno-builtin -c tools/abi_smoke.c -o $(OUT)/abi-smoke-tcc.o
	$(ZIG) cc -pthread -Iinclude -fno-builtin $(OUT)/abi-smoke-tcc.o $(OUT)/string-locale-tcc.o $(OUT)/string-locale-lock-tcc.o $(OUT)/string-tcc.o $(OUT)/memory-tcc.o $(OUT)/memory-tcc-helper.o $(OUT)/memory-tcc-simd.o $(OUT)/ctype-tcc.o $(OUT)/stdlib-tcc.o $(OUT)/stdlib-float-tcc.o $(OUT)/errno-host-tcc.o $(OUT)/environment-adapter-smoke-tcc.o -o $(OUT)/abi-smoke-tcc
	$(TCC) -Iinclude -fno-builtin -c tools/lib_stdio_smoke.c -o $(OUT)/lib-stdio-smoke-tcc.o
	$(TCC) -Iinclude -fno-builtin -c src/stdio_host_adapter.c -o $(OUT)/stdio-host-smoke-tcc.o
	$(TCC) -Iinclude -fno-builtin -c src/alloc_hybrid_adapter.c -o $(OUT)/alloc-hybrid-smoke-tcc.o
	$(ZIG) cc -pthread -Iinclude -fno-builtin $(OUT)/lib-stdio-smoke-tcc.o $(OUT)/stdio-host-smoke-tcc.o $(OUT)/alloc-hybrid-smoke-tcc.o $(OUT)/environment-adapter-smoke-tcc.o $(OUT)/stdio-hybrid-tcc.o $(OUT)/open-memstream-hybrid-tcc.o $(OUT)/fmemopen-hybrid-tcc.o $(OUT)/alloc-tcc.o $(OUT)/memory-tcc.o $(OUT)/memory-tcc-helper.o $(OUT)/memory-tcc-simd.o $(OUT)/errno-host-tcc.o $(OUT)/sys-linux-x86_64-tcc.o $(OUT)/sys-linux-x86_64-tcc-asm.o -o $(OUT)/stdio-smoke-tcc

smoke-tcc: tcc
	$(OUT)/memory-tcc-diff
	$(OUT)/memory-diff-tcc
	$(OUT)/abi-smoke-tcc
	$(OUT)/stdio-smoke-tcc

smoke-stdio-tcc: tcc
	$(OUT)/stdio-smoke-tcc

bench-zig: $(OUT) $(OUT)/memory-x86_64-simd.o
	@$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-O2 --cflags=-fno-builtin -o $(OUT)/memory-bench-zig.o src/memory.nelua
	@$(ZIG) cc -O2 -Iinclude -fno-builtin -DBENCH_COMPILER='"Zig cc"' -DBENCH_LIBC='"glibc"' -DBENCH_IMPLEMENTATION='"libc"' tools/mem_bench.c -o $(OUT)/mem-bench-zig-libc
	@$(ZIG) cc -O2 -Iinclude -fno-builtin -DBENCH_COMPILER='"Zig cc"' -DBENCH_LIBC='"glibc"' -DBENCH_IMPLEMENTATION='"nelibc"' tools/mem_bench.c $(OUT)/memory-bench-zig.o $(OUT)/memory-x86_64-simd.o -o $(OUT)/mem-bench-zig-nelibc

shootout-zig: bench-zig
	@$(OUT)/mem-bench-zig-libc > $(OUT)/mem-shootout-zig-libc.csv
	@$(OUT)/mem-bench-zig-nelibc > $(OUT)/mem-shootout-zig-nelibc.csv
	@{ cat $(OUT)/mem-shootout-zig-libc.csv; tail -n +2 $(OUT)/mem-shootout-zig-nelibc.csv; } > $(OUT)/mem-shootout-zig.csv
	@cat $(OUT)/mem-shootout-zig.csv

bench-gcc: $(OUT) $(OUT)/memory-x86_64-simd.o
	@$(NELUA) -B -r --cc gcc --cflags=-O2 --cflags=-fno-builtin -o $(OUT)/memory-bench-gcc.o src/memory.nelua
	@gcc -O2 -Iinclude -fno-builtin -DBENCH_COMPILER='"GCC"' -DBENCH_LIBC='"glibc"' -DBENCH_IMPLEMENTATION='"libc"' tools/mem_bench.c -o $(OUT)/mem-bench-gcc-libc
	@gcc -O2 -Iinclude -fno-builtin -DBENCH_COMPILER='"GCC"' -DBENCH_LIBC='"glibc"' -DBENCH_IMPLEMENTATION='"nelibc"' tools/mem_bench.c $(OUT)/memory-bench-gcc.o $(OUT)/memory-x86_64-simd.o -o $(OUT)/mem-bench-gcc-nelibc

shootout-gcc: bench-gcc
	@$(OUT)/mem-bench-gcc-libc > $(OUT)/mem-shootout-gcc-libc.csv
	@$(OUT)/mem-bench-gcc-nelibc > $(OUT)/mem-shootout-gcc-nelibc.csv
	@{ cat $(OUT)/mem-shootout-gcc-libc.csv; tail -n +2 $(OUT)/mem-shootout-gcc-nelibc.csv; } > $(OUT)/mem-shootout-gcc.csv
	@cat $(OUT)/mem-shootout-gcc.csv

bench-zig-musl: $(OUT) $(OUT)/memory-x86_64-simd.o
	@$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-target=x86_64-linux-musl --cflags=-O2 --cflags=-fno-builtin -o $(OUT)/memory-bench-zig-musl.o src/memory.nelua
	@$(ZIG) cc -target x86_64-linux-musl -O2 -static -Iinclude -fno-builtin -DBENCH_COMPILER='"Zig cc"' -DBENCH_LIBC='"musl"' -DBENCH_IMPLEMENTATION='"libc"' tools/mem_bench.c -o $(OUT)/mem-bench-zig-musl-libc
	@$(ZIG) cc -target x86_64-linux-musl -O2 -static -Iinclude -fno-builtin -DBENCH_COMPILER='"Zig cc"' -DBENCH_LIBC='"musl"' -DBENCH_IMPLEMENTATION='"nelibc"' tools/mem_bench.c $(OUT)/memory-bench-zig-musl.o $(OUT)/memory-x86_64-simd.o -o $(OUT)/mem-bench-zig-musl-nelibc

shootout-zig-musl: bench-zig-musl
	@$(OUT)/mem-bench-zig-musl-libc > $(OUT)/mem-shootout-zig-musl-libc.csv
	@$(OUT)/mem-bench-zig-musl-nelibc > $(OUT)/mem-shootout-zig-musl-nelibc.csv
	@{ cat $(OUT)/mem-shootout-zig-musl-libc.csv; tail -n +2 $(OUT)/mem-shootout-zig-musl-nelibc.csv; } > $(OUT)/mem-shootout-zig-musl.csv
	@cat $(OUT)/mem-shootout-zig-musl.csv

bench-musl-gcc: $(OUT) $(OUT)/memory-x86_64-simd.o
	@$(NELUA) -B -r --cc musl-gcc --cflags=-O2 --cflags=-fno-builtin -o $(OUT)/memory-bench-musl-gcc.o src/memory.nelua
	@musl-gcc -O2 -Iinclude -fno-builtin -DBENCH_COMPILER='"GCC via musl-gcc"' -DBENCH_LIBC='"musl"' -DBENCH_IMPLEMENTATION='"libc"' tools/mem_bench.c -o $(OUT)/mem-bench-musl-gcc-libc
	@musl-gcc -O2 -Iinclude -fno-builtin -DBENCH_COMPILER='"GCC via musl-gcc"' -DBENCH_LIBC='"musl"' -DBENCH_IMPLEMENTATION='"nelibc"' tools/mem_bench.c $(OUT)/memory-bench-musl-gcc.o $(OUT)/memory-x86_64-simd.o -o $(OUT)/mem-bench-musl-gcc-nelibc

shootout-musl-gcc: bench-musl-gcc
	@$(OUT)/mem-bench-musl-gcc-libc
	@$(OUT)/mem-bench-musl-gcc-nelibc | tail -n +2

bench-tcc: $(OUT)
	@$(NELUA) -B -r --cc '$(TCC)' --cflags=-O2 --cflags=-fno-builtin -o $(OUT)/memory-bench-tcc.o src/memory.nelua
	@$(TCC) -O2 -c -fno-builtin src/memory_tcc_x86_64.c -o $(OUT)/memory-bench-tcc-helper.o
	@$(ZIG) cc -c -DNELIBC_TCC_PUBLIC_MEMCPY src/memory_x86_64.S -o $(OUT)/memory-bench-tcc-simd.o
	@$(TCC) -O2 -Iinclude -fno-builtin -DBENCH_COMPILER='"TinyCC"' -DBENCH_LIBC='"glibc"' -DBENCH_IMPLEMENTATION='"libc"' tools/mem_bench.c -o $(OUT)/mem-bench-tcc-libc
	@$(TCC) -O2 -Iinclude -fno-builtin -DBENCH_COMPILER='"TinyCC"' -DBENCH_LIBC='"glibc"' -DBENCH_IMPLEMENTATION='"nelibc"' tools/mem_bench.c $(OUT)/memory-bench-tcc.o $(OUT)/memory-bench-tcc-helper.o $(OUT)/memory-bench-tcc-simd.o -o $(OUT)/mem-bench-tcc-nelibc

shootout-tcc: bench-tcc
	@$(OUT)/mem-bench-tcc-libc > $(OUT)/mem-shootout-tcc-libc.csv
	@$(OUT)/mem-bench-tcc-nelibc > $(OUT)/mem-shootout-tcc-nelibc.csv
	@{ cat $(OUT)/mem-shootout-tcc-libc.csv; tail -n +2 $(OUT)/mem-shootout-tcc-nelibc.csv; } > $(OUT)/mem-shootout-tcc.csv
	@cat $(OUT)/mem-shootout-tcc.csv

shootout-tcc-memcpy-variants: bench-tcc
	@$(ZIG) cc -c -DNELIBC_TCC_PUBLIC_MEMCPY -DNELIBC_TCC_MEMCPY_REP_ONLY src/memory_x86_64.S -o $(OUT)/memory-tcc-repmovsb-baseline.o
	@$(TCC) -O2 -Iinclude -fno-builtin -DBENCH_COMPILER='"TinyCC"' -DBENCH_LIBC='"glibc"' -DBENCH_IMPLEMENTATION='"nelibc-repmovsb-baseline"' tools/mem_bench.c $(OUT)/memory-bench-tcc.o $(OUT)/memory-bench-tcc-helper.o $(OUT)/memory-tcc-repmovsb-baseline.o -o $(OUT)/mem-bench-tcc-repmovsb-baseline
	@$(OUT)/mem-bench-tcc-libc > $(OUT)/mem-shootout-tcc-memcpy-libc.csv
	@$(OUT)/mem-bench-tcc-nelibc > $(OUT)/mem-shootout-tcc-memcpy-sse64.csv
	@$(OUT)/mem-bench-tcc-repmovsb-baseline > $(OUT)/mem-shootout-tcc-memcpy-repmovsb.csv
	@cat $(OUT)/mem-shootout-tcc-memcpy-libc.csv $(OUT)/mem-shootout-tcc-memcpy-sse64.csv $(OUT)/mem-shootout-tcc-memcpy-repmovsb.csv | awk -F, 'NR == 1 || $$4 == "memcpy"'

memcmp-shootout-zig: $(OUT) $(OUT)/memory-x86_64-simd.o $(OUT)/memory-x86_64-sse2-baseline.o
	@$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-O2 --cflags=-fno-builtin -o $(OUT)/memory-memcmp-zig.o src/memory.nelua
	@$(ZIG) cc -O2 -Iinclude -fno-builtin -DBENCH_COMPILER='"Zig cc"' -DBENCH_LIBC='"glibc"' -DBENCH_IMPLEMENTATION='"libc"' tools/memcmp_bench.c -o $(OUT)/memcmp-bench-zig-libc
	@$(ZIG) cc -O2 -Iinclude -fno-builtin -DBENCH_COMPILER='"Zig cc"' -DBENCH_LIBC='"glibc"' -DBENCH_IMPLEMENTATION='"nelibc-sse2"' tools/memcmp_bench.c $(OUT)/memory-memcmp-zig.o $(OUT)/memory-x86_64-sse2-baseline.o -o $(OUT)/memcmp-bench-zig-sse2
	@$(ZIG) cc -O2 -Iinclude -fno-builtin -DBENCH_COMPILER='"Zig cc"' -DBENCH_LIBC='"glibc"' -DBENCH_IMPLEMENTATION='"nelibc"' tools/memcmp_bench.c $(OUT)/memory-memcmp-zig.o $(OUT)/memory-x86_64-simd.o -o $(OUT)/memcmp-bench-zig-nelibc
	@$(OUT)/memcmp-bench-zig-libc > $(OUT)/memcmp-shootout-zig-libc.csv
	@$(OUT)/memcmp-bench-zig-sse2 > $(OUT)/memcmp-shootout-zig-sse2.csv
	@$(OUT)/memcmp-bench-zig-nelibc > $(OUT)/memcmp-shootout-zig-nelibc.csv
	@{ cat $(OUT)/memcmp-shootout-zig-libc.csv; tail -n +2 $(OUT)/memcmp-shootout-zig-sse2.csv; tail -n +2 $(OUT)/memcmp-shootout-zig-nelibc.csv; } > $(OUT)/memcmp-shootout-zig.csv
	@cat $(OUT)/memcmp-shootout-zig.csv

memcmp-shootout-gcc: $(OUT) $(OUT)/memory-x86_64-simd.o $(OUT)/memory-x86_64-sse2-baseline.o
	@$(NELUA) -B -r --cc gcc --cflags=-O2 --cflags=-fno-builtin -o $(OUT)/memory-memcmp-gcc.o src/memory.nelua
	@gcc -O2 -Iinclude -fno-builtin -DBENCH_COMPILER='"GCC"' -DBENCH_LIBC='"glibc"' -DBENCH_IMPLEMENTATION='"libc"' tools/memcmp_bench.c -o $(OUT)/memcmp-bench-gcc-libc
	@gcc -O2 -Iinclude -fno-builtin -DBENCH_COMPILER='"GCC"' -DBENCH_LIBC='"glibc"' -DBENCH_IMPLEMENTATION='"nelibc-sse2"' tools/memcmp_bench.c $(OUT)/memory-memcmp-gcc.o $(OUT)/memory-x86_64-sse2-baseline.o -o $(OUT)/memcmp-bench-gcc-sse2
	@gcc -O2 -Iinclude -fno-builtin -DBENCH_COMPILER='"GCC"' -DBENCH_LIBC='"glibc"' -DBENCH_IMPLEMENTATION='"nelibc"' tools/memcmp_bench.c $(OUT)/memory-memcmp-gcc.o $(OUT)/memory-x86_64-simd.o -o $(OUT)/memcmp-bench-gcc-nelibc
	@$(OUT)/memcmp-bench-gcc-libc > $(OUT)/memcmp-shootout-gcc-libc.csv
	@$(OUT)/memcmp-bench-gcc-sse2 > $(OUT)/memcmp-shootout-gcc-sse2.csv
	@$(OUT)/memcmp-bench-gcc-nelibc > $(OUT)/memcmp-shootout-gcc-nelibc.csv
	@{ cat $(OUT)/memcmp-shootout-gcc-libc.csv; tail -n +2 $(OUT)/memcmp-shootout-gcc-sse2.csv; tail -n +2 $(OUT)/memcmp-shootout-gcc-nelibc.csv; } > $(OUT)/memcmp-shootout-gcc.csv
	@cat $(OUT)/memcmp-shootout-gcc.csv

memcmp-shootout-zig-musl: $(OUT) $(OUT)/memory-x86_64-simd.o $(OUT)/memory-x86_64-sse2-baseline.o
	@$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-target=x86_64-linux-musl --cflags=-O2 --cflags=-fno-builtin -o $(OUT)/memory-memcmp-zig-musl.o src/memory.nelua
	@$(ZIG) cc -target x86_64-linux-musl -static -O2 -Iinclude -fno-builtin -DNELIBC_STATIC_MUSL -Wl,--wrap=free -Wl,--wrap=realloc -DBENCH_COMPILER='"Zig cc"' -DBENCH_LIBC='"musl"' -DBENCH_IMPLEMENTATION='"libc"' tools/memcmp_bench.c -o $(OUT)/memcmp-bench-zig-musl-libc
	@$(ZIG) cc -target x86_64-linux-musl -static -O2 -Iinclude -fno-builtin -DNELIBC_STATIC_MUSL -Wl,--wrap=free -Wl,--wrap=realloc -DBENCH_COMPILER='"Zig cc"' -DBENCH_LIBC='"musl"' -DBENCH_IMPLEMENTATION='"nelibc-sse2"' tools/memcmp_bench.c $(OUT)/memory-memcmp-zig-musl.o $(OUT)/memory-x86_64-sse2-baseline.o -o $(OUT)/memcmp-bench-zig-musl-sse2
	@$(ZIG) cc -target x86_64-linux-musl -static -O2 -Iinclude -fno-builtin -DNELIBC_STATIC_MUSL -Wl,--wrap=free -Wl,--wrap=realloc -DBENCH_COMPILER='"Zig cc"' -DBENCH_LIBC='"musl"' -DBENCH_IMPLEMENTATION='"nelibc"' tools/memcmp_bench.c $(OUT)/memory-memcmp-zig-musl.o $(OUT)/memory-x86_64-simd.o -o $(OUT)/memcmp-bench-zig-musl-nelibc
	@$(OUT)/memcmp-bench-zig-musl-libc > $(OUT)/memcmp-shootout-zig-musl-libc.csv
	@$(OUT)/memcmp-bench-zig-musl-sse2 > $(OUT)/memcmp-shootout-zig-musl-sse2.csv
	@$(OUT)/memcmp-bench-zig-musl-nelibc > $(OUT)/memcmp-shootout-zig-musl-nelibc.csv
	@{ cat $(OUT)/memcmp-shootout-zig-musl-libc.csv; tail -n +2 $(OUT)/memcmp-shootout-zig-musl-sse2.csv; tail -n +2 $(OUT)/memcmp-shootout-zig-musl-nelibc.csv; } > $(OUT)/memcmp-shootout-zig-musl.csv
	@cat $(OUT)/memcmp-shootout-zig-musl.csv

memcmp-shootout-tcc: $(OUT) $(OUT)/memory-x86_64-simd.o $(OUT)/memory-x86_64-sse2-baseline.o
	@$(NELUA) -B -r --cc '$(TCC)' --cflags=-O2 --cflags=-fno-builtin -o $(OUT)/memory-memcmp-tcc.o src/memory.nelua
	@$(TCC) -O2 -c -fno-builtin src/memory_tcc_x86_64.c -o $(OUT)/memory-memcmp-tcc-helper.o
	@$(TCC) -O2 -Iinclude -fno-builtin -DBENCH_COMPILER='"TinyCC"' -DBENCH_LIBC='"glibc"' -DBENCH_IMPLEMENTATION='"libc"' tools/memcmp_bench.c -o $(OUT)/memcmp-bench-tcc-libc
	@$(TCC) -O2 -Iinclude -fno-builtin -DBENCH_COMPILER='"TinyCC"' -DBENCH_LIBC='"glibc"' -DBENCH_IMPLEMENTATION='"nelibc-sse2"' tools/memcmp_bench.c $(OUT)/memory-memcmp-tcc.o $(OUT)/memory-memcmp-tcc-helper.o $(OUT)/memory-x86_64-sse2-baseline.o -o $(OUT)/memcmp-bench-tcc-sse2
	@$(TCC) -O2 -Iinclude -fno-builtin -DBENCH_COMPILER='"TinyCC"' -DBENCH_LIBC='"glibc"' -DBENCH_IMPLEMENTATION='"nelibc"' tools/memcmp_bench.c $(OUT)/memory-memcmp-tcc.o $(OUT)/memory-memcmp-tcc-helper.o $(OUT)/memory-x86_64-simd.o -o $(OUT)/memcmp-bench-tcc-nelibc
	@$(OUT)/memcmp-bench-tcc-libc > $(OUT)/memcmp-shootout-tcc-libc.csv
	@$(OUT)/memcmp-bench-tcc-sse2 > $(OUT)/memcmp-shootout-tcc-sse2.csv
	@$(OUT)/memcmp-bench-tcc-nelibc > $(OUT)/memcmp-shootout-tcc-nelibc.csv
	@{ cat $(OUT)/memcmp-shootout-tcc-libc.csv; tail -n +2 $(OUT)/memcmp-shootout-tcc-sse2.csv; tail -n +2 $(OUT)/memcmp-shootout-tcc-nelibc.csv; } > $(OUT)/memcmp-shootout-tcc.csv
	@cat $(OUT)/memcmp-shootout-tcc.csv

memcmp-shootout-matrix: memcmp-shootout-zig memcmp-shootout-gcc memcmp-shootout-zig-musl memcmp-shootout-tcc
	@awk 'FNR == 1 && NR > 1 { next } { print }' $(OUT)/memcmp-shootout-zig.csv $(OUT)/memcmp-shootout-gcc.csv $(OUT)/memcmp-shootout-zig-musl.csv $(OUT)/memcmp-shootout-tcc.csv > $(OUT)/memcmp-shootout-matrix.csv
	@cat $(OUT)/memcmp-shootout-matrix.csv

strstr-shootout-zig: $(OUT) $(OUT)/memory-x86_64-simd.o
	@$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-O2 --cflags=-fno-builtin -o $(OUT)/string-strstr-zig.o src/string.nelua
	@$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-O2 --cflags=-fno-builtin -o $(OUT)/memory-strstr-zig.o src/memory.nelua
	@$(ZIG) cc -O2 -Iinclude -fno-builtin -DBENCH_COMPILER='"Zig cc"' -DBENCH_LIBC='"glibc"' -DBENCH_IMPLEMENTATION='"libc"' tools/strstr_bench.c -o $(OUT)/strstr-bench-zig-libc
	@$(ZIG) cc -O2 -Iinclude -fno-builtin -DBENCH_COMPILER='"Zig cc"' -DBENCH_LIBC='"glibc"' -DBENCH_IMPLEMENTATION='"nelibc"' tools/strstr_bench.c $(OUT)/string-locale-zig.o $(OUT)/string-locale-lock-zig.o $(OUT)/string-strstr-zig.o $(OUT)/memory-strstr-zig.o $(OUT)/memory-x86_64-simd.o -o $(OUT)/strstr-bench-zig-nelibc
	@$(OUT)/strstr-bench-zig-libc > $(OUT)/strstr-shootout-zig-libc.csv
	@$(OUT)/strstr-bench-zig-nelibc > $(OUT)/strstr-shootout-zig-nelibc.csv
	@{ cat $(OUT)/strstr-shootout-zig-libc.csv; tail -n +2 $(OUT)/strstr-shootout-zig-nelibc.csv; } > $(OUT)/strstr-shootout-zig.csv
	@cat $(OUT)/strstr-shootout-zig.csv

strstr-shootout-gcc: $(OUT) $(OUT)/memory-x86_64-simd.o
	@$(NELUA) -B -r --cc gcc --cflags=-O2 --cflags=-fno-builtin -o $(OUT)/string-strstr-gcc.o src/string.nelua
	@$(NELUA) -B -r --cc gcc --cflags=-O2 --cflags=-fno-builtin -o $(OUT)/memory-strstr-gcc.o src/memory.nelua
	@gcc -O2 -Iinclude -fno-builtin -DBENCH_COMPILER='"GCC"' -DBENCH_LIBC='"glibc"' -DBENCH_IMPLEMENTATION='"libc"' tools/strstr_bench.c -o $(OUT)/strstr-bench-gcc-libc
	@gcc -O2 -Iinclude -fno-builtin -DBENCH_COMPILER='"GCC"' -DBENCH_LIBC='"glibc"' -DBENCH_IMPLEMENTATION='"nelibc"' tools/strstr_bench.c $(OUT)/string-locale-gcc.o $(OUT)/string-locale-lock-gcc.o $(OUT)/string-strstr-gcc.o $(OUT)/memory-strstr-gcc.o $(OUT)/memory-x86_64-simd.o -o $(OUT)/strstr-bench-gcc-nelibc
	@$(OUT)/strstr-bench-gcc-libc > $(OUT)/strstr-shootout-gcc-libc.csv
	@$(OUT)/strstr-bench-gcc-nelibc > $(OUT)/strstr-shootout-gcc-nelibc.csv
	@{ cat $(OUT)/strstr-shootout-gcc-libc.csv; tail -n +2 $(OUT)/strstr-shootout-gcc-nelibc.csv; } > $(OUT)/strstr-shootout-gcc.csv
	@cat $(OUT)/strstr-shootout-gcc.csv

strstr-shootout-zig-musl: $(OUT) $(OUT)/memory-x86_64-simd.o
	@$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-target=x86_64-linux-musl --cflags=-O2 --cflags=-fno-builtin -o $(OUT)/string-strstr-zig-musl.o src/string.nelua
	@$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-target=x86_64-linux-musl --cflags=-O2 --cflags=-fno-builtin -o $(OUT)/memory-strstr-zig-musl.o src/memory.nelua
	@$(ZIG) cc -target x86_64-linux-musl -static -O2 -Iinclude -fno-builtin -DNELIBC_STATIC_MUSL -Wl,--wrap=free -Wl,--wrap=realloc -DBENCH_COMPILER='"Zig cc"' -DBENCH_LIBC='"musl"' -DBENCH_IMPLEMENTATION='"libc"' tools/strstr_bench.c -o $(OUT)/strstr-bench-zig-musl-libc
	@$(ZIG) cc -target x86_64-linux-musl -static -O2 -Iinclude -fno-builtin -DNELIBC_STATIC_MUSL -Wl,--wrap=free -Wl,--wrap=realloc -DBENCH_COMPILER='"Zig cc"' -DBENCH_LIBC='"musl"' -DBENCH_IMPLEMENTATION='"nelibc"' tools/strstr_bench.c $(OUT)/string-locale-zig-musl.o $(OUT)/string-locale-lock-zig-musl.o $(OUT)/string-strstr-zig-musl.o $(OUT)/memory-strstr-zig-musl.o $(OUT)/memory-x86_64-simd.o -o $(OUT)/strstr-bench-zig-musl-nelibc
	@$(OUT)/strstr-bench-zig-musl-libc > $(OUT)/strstr-shootout-zig-musl-libc.csv
	@$(OUT)/strstr-bench-zig-musl-nelibc > $(OUT)/strstr-shootout-zig-musl-nelibc.csv
	@{ cat $(OUT)/strstr-shootout-zig-musl-libc.csv; tail -n +2 $(OUT)/strstr-shootout-zig-musl-nelibc.csv; } > $(OUT)/strstr-shootout-zig-musl.csv
	@cat $(OUT)/strstr-shootout-zig-musl.csv

strstr-shootout-tcc: $(OUT) $(OUT)/memory-x86_64-simd.o
	@$(NELUA) -B -r --cc '$(TCC)' --cflags=-O2 --cflags=-fno-builtin -o $(OUT)/string-strstr-tcc.o src/string.nelua
	@$(NELUA) -B -r --cc '$(TCC)' --cflags=-O2 --cflags=-fno-builtin -o $(OUT)/memory-strstr-tcc.o src/memory.nelua
	@$(TCC) -O2 -c -fno-builtin src/memory_tcc_x86_64.c -o $(OUT)/memory-strstr-tcc-helper.o
	@$(TCC) -O2 -Iinclude -fno-builtin -DBENCH_COMPILER='"TinyCC"' -DBENCH_LIBC='"glibc"' -DBENCH_IMPLEMENTATION='"libc"' tools/strstr_bench.c -o $(OUT)/strstr-bench-tcc-libc
	@$(TCC) -O2 -Iinclude -fno-builtin -DBENCH_COMPILER='"TinyCC"' -DBENCH_LIBC='"glibc"' -DBENCH_IMPLEMENTATION='"nelibc"' tools/strstr_bench.c $(OUT)/string-locale-tcc.o $(OUT)/string-locale-lock-tcc.o $(OUT)/string-strstr-tcc.o $(OUT)/memory-strstr-tcc.o $(OUT)/memory-strstr-tcc-helper.o $(OUT)/memory-x86_64-simd.o -o $(OUT)/strstr-bench-tcc-nelibc
	@$(OUT)/strstr-bench-tcc-libc > $(OUT)/strstr-shootout-tcc-libc.csv
	@$(OUT)/strstr-bench-tcc-nelibc > $(OUT)/strstr-shootout-tcc-nelibc.csv
	@{ cat $(OUT)/strstr-shootout-tcc-libc.csv; tail -n +2 $(OUT)/strstr-shootout-tcc-nelibc.csv; } > $(OUT)/strstr-shootout-tcc.csv
	@cat $(OUT)/strstr-shootout-tcc.csv

strstr-shootout-matrix: strstr-shootout-zig strstr-shootout-gcc strstr-shootout-zig-musl strstr-shootout-tcc
	@awk 'FNR == 1 && NR > 1 { next } { print }' $(OUT)/strstr-shootout-zig.csv $(OUT)/strstr-shootout-gcc.csv $(OUT)/strstr-shootout-zig-musl.csv $(OUT)/strstr-shootout-tcc.csv > $(OUT)/strstr-shootout-matrix.csv
	@cat $(OUT)/strstr-shootout-matrix.csv

strcasecmp-shootout-zig: zig
	@$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-O2 --cflags=-fno-builtin -o $(OUT)/string-strcasecmp-zig.o src/string.nelua
	@$(ZIG) cc -O2 -Iinclude -fno-builtin -DBENCH_COMPILER='"Zig cc"' -DBENCH_LIBC='"glibc"' -DBENCH_IMPLEMENTATION='"host-libc"' tools/strcasecmp_bench.c -o $(OUT)/strcasecmp-bench-zig-libc
	@$(ZIG) cc -O2 -Iinclude -fno-builtin -DBENCH_COMPILER='"Zig cc"' -DBENCH_LIBC='"glibc"' -DBENCH_IMPLEMENTATION='"nelibc"' -DBENCH_NELIBC tools/strcasecmp_bench.c $(OUT)/string-locale-zig.o $(OUT)/string-locale-lock-zig.o $(OUT)/string-strcasecmp-zig.o $(OUT)/memory-zig.o $(OUT)/memory-x86_64-simd.o -o $(OUT)/strcasecmp-bench-zig-nelibc
	@$(OUT)/strcasecmp-bench-zig-libc --exhaustive > $(OUT)/strcasecmp-exhaustive-zig-libc.csv
	@$(OUT)/strcasecmp-bench-zig-nelibc --exhaustive > $(OUT)/strcasecmp-exhaustive-zig-nelibc.csv
	@cmp $(OUT)/strcasecmp-exhaustive-zig-libc.csv $(OUT)/strcasecmp-exhaustive-zig-nelibc.csv && echo '3,670,016 prefix-pattern byte cases matched (Zig/glibc)'
	@$(OUT)/strcasecmp-bench-zig-libc --random > $(OUT)/strcasecmp-random-zig-libc.csv
	@$(OUT)/strcasecmp-bench-zig-nelibc --random > $(OUT)/strcasecmp-random-zig-nelibc.csv
	@cmp $(OUT)/strcasecmp-random-zig-libc.csv $(OUT)/strcasecmp-random-zig-nelibc.csv && echo '100,000 randomized, page-edge, same-pointer, and overlap cases matched (Zig/glibc)'
	@$(OUT)/strcasecmp-bench-zig-libc > $(OUT)/strcasecmp-shootout-zig-libc.csv
	@$(OUT)/strcasecmp-bench-zig-nelibc > $(OUT)/strcasecmp-shootout-zig-nelibc.csv
	@{ cat $(OUT)/strcasecmp-shootout-zig-libc.csv; tail -n +2 $(OUT)/strcasecmp-shootout-zig-nelibc.csv; } > $(OUT)/strcasecmp-shootout-zig.csv
	@cat $(OUT)/strcasecmp-shootout-zig.csv

strcasecmp-shootout-gcc: gcc
	@$(NELUA) -B -r --cc gcc --cflags=-O2 --cflags=-fno-builtin -o $(OUT)/string-strcasecmp-gcc.o src/string.nelua
	@gcc -O2 -Iinclude -fno-builtin -DBENCH_COMPILER='"GCC"' -DBENCH_LIBC='"glibc"' -DBENCH_IMPLEMENTATION='"host-libc"' tools/strcasecmp_bench.c -o $(OUT)/strcasecmp-bench-gcc-libc
	@gcc -O2 -Iinclude -fno-builtin -DBENCH_COMPILER='"GCC"' -DBENCH_LIBC='"glibc"' -DBENCH_IMPLEMENTATION='"nelibc"' -DBENCH_NELIBC tools/strcasecmp_bench.c $(OUT)/string-locale-gcc.o $(OUT)/string-locale-lock-gcc.o $(OUT)/string-strcasecmp-gcc.o $(OUT)/memory-gcc.o $(OUT)/memory-x86_64-simd.o -o $(OUT)/strcasecmp-bench-gcc-nelibc
	@$(OUT)/strcasecmp-bench-gcc-libc --exhaustive > $(OUT)/strcasecmp-exhaustive-gcc-libc.csv
	@$(OUT)/strcasecmp-bench-gcc-nelibc --exhaustive > $(OUT)/strcasecmp-exhaustive-gcc-nelibc.csv
	@cmp $(OUT)/strcasecmp-exhaustive-gcc-libc.csv $(OUT)/strcasecmp-exhaustive-gcc-nelibc.csv && echo '3,670,016 prefix-pattern byte cases matched (GCC/glibc)'
	@$(OUT)/strcasecmp-bench-gcc-libc --random > $(OUT)/strcasecmp-random-gcc-libc.csv
	@$(OUT)/strcasecmp-bench-gcc-nelibc --random > $(OUT)/strcasecmp-random-gcc-nelibc.csv
	@cmp $(OUT)/strcasecmp-random-gcc-libc.csv $(OUT)/strcasecmp-random-gcc-nelibc.csv && echo '100,000 randomized, page-edge, same-pointer, and overlap cases matched (GCC/glibc)'
	@$(OUT)/strcasecmp-bench-gcc-libc > $(OUT)/strcasecmp-shootout-gcc-libc.csv
	@$(OUT)/strcasecmp-bench-gcc-nelibc > $(OUT)/strcasecmp-shootout-gcc-nelibc.csv
	@{ cat $(OUT)/strcasecmp-shootout-gcc-libc.csv; tail -n +2 $(OUT)/strcasecmp-shootout-gcc-nelibc.csv; } > $(OUT)/strcasecmp-shootout-gcc.csv
	@cat $(OUT)/strcasecmp-shootout-gcc.csv

strcasecmp-shootout-zig-musl: zig-musl
	@$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-target=x86_64-linux-musl --cflags=-O2 --cflags=-fno-builtin -o $(OUT)/string-strcasecmp-zig-musl.o src/string.nelua
	@$(ZIG) cc -target x86_64-linux-musl -static -O2 -Iinclude -fno-builtin -DNELIBC_STATIC_MUSL -Wl,--wrap=free -Wl,--wrap=realloc -DBENCH_COMPILER='"Zig cc"' -DBENCH_LIBC='"musl"' -DBENCH_IMPLEMENTATION='"host-libc"' tools/strcasecmp_bench.c -o $(OUT)/strcasecmp-bench-zig-musl-libc
	@$(ZIG) cc -target x86_64-linux-musl -static -O2 -Iinclude -fno-builtin -DNELIBC_STATIC_MUSL -Wl,--wrap=free -Wl,--wrap=realloc -DBENCH_COMPILER='"Zig cc"' -DBENCH_LIBC='"musl"' -DBENCH_IMPLEMENTATION='"nelibc"' -DBENCH_NELIBC tools/strcasecmp_bench.c $(OUT)/string-locale-zig-musl.o $(OUT)/string-locale-lock-zig-musl.o $(OUT)/string-strcasecmp-zig-musl.o $(OUT)/memory-zig-musl.o $(OUT)/memory-x86_64-simd.o -o $(OUT)/strcasecmp-bench-zig-musl-nelibc
	@$(OUT)/strcasecmp-bench-zig-musl-libc --exhaustive > $(OUT)/strcasecmp-exhaustive-zig-musl-libc.csv
	@$(OUT)/strcasecmp-bench-zig-musl-nelibc --exhaustive > $(OUT)/strcasecmp-exhaustive-zig-musl-nelibc.csv
	@cmp $(OUT)/strcasecmp-exhaustive-zig-musl-libc.csv $(OUT)/strcasecmp-exhaustive-zig-musl-nelibc.csv && echo '3,670,016 prefix-pattern byte cases matched (Zig/musl)'
	@$(OUT)/strcasecmp-bench-zig-musl-libc --random > $(OUT)/strcasecmp-random-zig-musl-libc.csv
	@$(OUT)/strcasecmp-bench-zig-musl-nelibc --random > $(OUT)/strcasecmp-random-zig-musl-nelibc.csv
	@cmp $(OUT)/strcasecmp-random-zig-musl-libc.csv $(OUT)/strcasecmp-random-zig-musl-nelibc.csv && echo '100,000 randomized, page-edge, same-pointer, and overlap cases matched (Zig/musl)'
	@$(OUT)/strcasecmp-bench-zig-musl-libc > $(OUT)/strcasecmp-shootout-zig-musl-libc.csv
	@$(OUT)/strcasecmp-bench-zig-musl-nelibc > $(OUT)/strcasecmp-shootout-zig-musl-nelibc.csv
	@{ cat $(OUT)/strcasecmp-shootout-zig-musl-libc.csv; tail -n +2 $(OUT)/strcasecmp-shootout-zig-musl-nelibc.csv; } > $(OUT)/strcasecmp-shootout-zig-musl.csv
	@cat $(OUT)/strcasecmp-shootout-zig-musl.csv

strcasecmp-shootout-tcc: tcc
	@$(NELUA) -B -r --cc '$(TCC)' --cflags=-O2 --cflags=-fno-builtin -o $(OUT)/string-strcasecmp-tcc.o src/string.nelua
	@$(TCC) -O2 -Iinclude -fno-builtin -DBENCH_COMPILER='"TinyCC"' -DBENCH_LIBC='"glibc"' -DBENCH_IMPLEMENTATION='"host-libc"' tools/strcasecmp_bench.c -o $(OUT)/strcasecmp-bench-tcc-libc
	@$(TCC) -O2 -Iinclude -fno-builtin -DBENCH_COMPILER='"TinyCC"' -DBENCH_LIBC='"glibc"' -DBENCH_IMPLEMENTATION='"nelibc"' -DBENCH_NELIBC tools/strcasecmp_bench.c $(OUT)/string-locale-tcc.o $(OUT)/string-locale-lock-tcc.o $(OUT)/string-strcasecmp-tcc.o $(OUT)/memory-tcc.o $(OUT)/memory-tcc-helper.o $(OUT)/memory-tcc-simd.o -o $(OUT)/strcasecmp-bench-tcc-nelibc
	@$(OUT)/strcasecmp-bench-tcc-libc --exhaustive > $(OUT)/strcasecmp-exhaustive-tcc-libc.csv
	@$(OUT)/strcasecmp-bench-tcc-nelibc --exhaustive > $(OUT)/strcasecmp-exhaustive-tcc-nelibc.csv
	@cmp $(OUT)/strcasecmp-exhaustive-tcc-libc.csv $(OUT)/strcasecmp-exhaustive-tcc-nelibc.csv && echo '3,670,016 prefix-pattern byte cases matched (TinyCC/glibc)'
	@$(OUT)/strcasecmp-bench-tcc-libc --random > $(OUT)/strcasecmp-random-tcc-libc.csv
	@$(OUT)/strcasecmp-bench-tcc-nelibc --random > $(OUT)/strcasecmp-random-tcc-nelibc.csv
	@cmp $(OUT)/strcasecmp-random-tcc-libc.csv $(OUT)/strcasecmp-random-tcc-nelibc.csv && echo '100,000 randomized, page-edge, same-pointer, and overlap cases matched (TinyCC/glibc)'
	@$(OUT)/strcasecmp-bench-tcc-libc > $(OUT)/strcasecmp-shootout-tcc-libc.csv
	@$(OUT)/strcasecmp-bench-tcc-nelibc > $(OUT)/strcasecmp-shootout-tcc-nelibc.csv
	@{ cat $(OUT)/strcasecmp-shootout-tcc-libc.csv; tail -n +2 $(OUT)/strcasecmp-shootout-tcc-nelibc.csv; } > $(OUT)/strcasecmp-shootout-tcc.csv
	@cat $(OUT)/strcasecmp-shootout-tcc.csv

strcasecmp-sse2-fallback: strcasecmp-shootout-zig $(OUT)/memory-x86_64-sse2-baseline.o
	@$(ZIG) cc -O2 -fno-builtin -Iinclude -DBENCH_COMPILER='"Zig cc SSE2-only"' -DBENCH_LIBC='"glibc"' -DBENCH_IMPLEMENTATION='"nelibc"' -DBENCH_NELIBC tools/strcasecmp_bench.c $(OUT)/string-locale-zig.o $(OUT)/string-locale-lock-zig.o $(OUT)/string-strcasecmp-zig.o $(OUT)/memory-zig.o $(OUT)/memory-x86_64-sse2-baseline.o -o $(OUT)/strcasecmp-bench-zig-sse2-only
	@$(OUT)/strcasecmp-bench-zig-libc --exhaustive > $(OUT)/strcasecmp-exhaustive-zig-sse2-host.csv
	@$(OUT)/strcasecmp-bench-zig-sse2-only --exhaustive > $(OUT)/strcasecmp-exhaustive-zig-sse2-only.csv
	@cmp $(OUT)/strcasecmp-exhaustive-zig-sse2-host.csv $(OUT)/strcasecmp-exhaustive-zig-sse2-only.csv && echo 'SSE2-only fallback passed 3,670,016 prefix-pattern byte cases (Zig/glibc)'
	@$(OUT)/strcasecmp-bench-zig-libc --random > $(OUT)/strcasecmp-random-zig-sse2-host.csv
	@$(OUT)/strcasecmp-bench-zig-sse2-only --random > $(OUT)/strcasecmp-random-zig-sse2-only.csv
	@cmp $(OUT)/strcasecmp-random-zig-sse2-host.csv $(OUT)/strcasecmp-random-zig-sse2-only.csv && echo 'SSE2-only fallback passed 100,000 randomized/page-edge cases (Zig/glibc)'
	@if objdump -d $(OUT)/memory-x86_64-sse2-baseline.o | grep -Eq 'ymm[0-9]+'; then echo 'SSE2-only object contains YMM instructions'; exit 1; fi

strcasecmp-shootout-matrix: strcasecmp-shootout-zig strcasecmp-shootout-gcc strcasecmp-shootout-zig-musl strcasecmp-shootout-tcc strcasecmp-sse2-fallback
	@awk 'FNR == 1 && NR > 1 { next } { print }' $(OUT)/strcasecmp-shootout-zig.csv $(OUT)/strcasecmp-shootout-gcc.csv $(OUT)/strcasecmp-shootout-zig-musl.csv $(OUT)/strcasecmp-shootout-tcc.csv > $(OUT)/strcasecmp-shootout-matrix.csv
	@cat $(OUT)/strcasecmp-shootout-matrix.csv

strcasecmp-perf-matrix: strcasecmp-shootout-matrix
	@$(ZIG) cc -O2 -fno-builtin -Iinclude -DBENCH_COMPILER='"Zig cc"' -DBENCH_LIBC='"glibc"' -DBENCH_IMPLEMENTATION='"host-libc"' tools/strcasecmp_perf.c -o $(OUT)/strcasecmp-perf-zig-libc
	@$(ZIG) cc -O2 -fno-builtin -Iinclude -DBENCH_COMPILER='"Zig cc"' -DBENCH_LIBC='"glibc"' -DBENCH_IMPLEMENTATION='"nelibc"' -DBENCH_NELIBC tools/strcasecmp_perf.c $(OUT)/string-locale-zig.o $(OUT)/string-locale-lock-zig.o $(OUT)/string-strcasecmp-zig.o $(OUT)/memory-zig.o $(OUT)/memory-x86_64-simd.o -o $(OUT)/strcasecmp-perf-zig-nelibc
	@gcc -O2 -fno-builtin -Iinclude -DBENCH_COMPILER='"GCC"' -DBENCH_LIBC='"glibc"' -DBENCH_IMPLEMENTATION='"host-libc"' tools/strcasecmp_perf.c -o $(OUT)/strcasecmp-perf-gcc-libc
	@gcc -O2 -fno-builtin -Iinclude -DBENCH_COMPILER='"GCC"' -DBENCH_LIBC='"glibc"' -DBENCH_IMPLEMENTATION='"nelibc"' -DBENCH_NELIBC tools/strcasecmp_perf.c $(OUT)/string-locale-gcc.o $(OUT)/string-locale-lock-gcc.o $(OUT)/string-strcasecmp-gcc.o $(OUT)/memory-gcc.o $(OUT)/memory-x86_64-simd.o -o $(OUT)/strcasecmp-perf-gcc-nelibc
	@$(ZIG) cc -target x86_64-linux-musl -static -O2 -fno-builtin -Iinclude -DBENCH_COMPILER='"Zig cc"' -DBENCH_LIBC='"musl"' -DBENCH_IMPLEMENTATION='"host-libc"' tools/strcasecmp_perf.c -o $(OUT)/strcasecmp-perf-zig-musl-libc
	@$(ZIG) cc -target x86_64-linux-musl -static -O2 -fno-builtin -Iinclude -DBENCH_COMPILER='"Zig cc"' -DBENCH_LIBC='"musl"' -DBENCH_IMPLEMENTATION='"nelibc"' -DBENCH_NELIBC tools/strcasecmp_perf.c $(OUT)/string-locale-zig-musl.o $(OUT)/string-locale-lock-zig-musl.o $(OUT)/string-strcasecmp-zig-musl.o $(OUT)/memory-zig-musl.o $(OUT)/memory-x86_64-simd.o -o $(OUT)/strcasecmp-perf-zig-musl-nelibc
	@$(TCC) -O2 -fno-builtin -Iinclude -DBENCH_COMPILER='"TinyCC"' -DBENCH_LIBC='"glibc"' -DBENCH_IMPLEMENTATION='"host-libc"' tools/strcasecmp_perf.c -o $(OUT)/strcasecmp-perf-tcc-libc
	@$(TCC) -O2 -fno-builtin -Iinclude -DBENCH_COMPILER='"TinyCC"' -DBENCH_LIBC='"glibc"' -DBENCH_IMPLEMENTATION='"nelibc"' -DBENCH_NELIBC tools/strcasecmp_perf.c $(OUT)/string-locale-tcc.o $(OUT)/string-locale-lock-tcc.o $(OUT)/string-strcasecmp-tcc.o $(OUT)/memory-tcc.o $(OUT)/memory-tcc-helper.o $(OUT)/memory-tcc-simd.o -o $(OUT)/strcasecmp-perf-tcc-nelibc
	@for row in zig-libc zig-nelibc gcc-libc gcc-nelibc zig-musl-libc zig-musl-nelibc tcc-libc tcc-nelibc; do $(OUT)/strcasecmp-perf-$$row > $(OUT)/strcasecmp-perf-$$row.csv || exit $$?; done
	@awk 'FNR == 1 && NR > 1 { next } { print }' $(OUT)/strcasecmp-perf-zig-libc.csv $(OUT)/strcasecmp-perf-zig-nelibc.csv $(OUT)/strcasecmp-perf-gcc-libc.csv $(OUT)/strcasecmp-perf-gcc-nelibc.csv $(OUT)/strcasecmp-perf-zig-musl-libc.csv $(OUT)/strcasecmp-perf-zig-musl-nelibc.csv $(OUT)/strcasecmp-perf-tcc-libc.csv $(OUT)/strcasecmp-perf-tcc-nelibc.csv > $(OUT)/strcasecmp-perf-matrix.csv
	@cat $(OUT)/strcasecmp-perf-matrix.csv

strverscmp-shootout-zig: zig
	@$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-O2 --cflags=-fno-builtin -o $(OUT)/string-strverscmp-zig.o src/string.nelua
	@$(ZIG) cc -O2 -Iinclude -fno-builtin -DBENCH_COMPILER='"Zig cc"' -DBENCH_LIBC='"glibc"' -DBENCH_IMPLEMENTATION='"host-libc"' tools/strverscmp_bench.c -o $(OUT)/strverscmp-zig-libc
	@$(ZIG) cc -O2 -Iinclude -fno-builtin -DBENCH_COMPILER='"Zig cc"' -DBENCH_LIBC='"glibc"' -DBENCH_IMPLEMENTATION='"nelibc"' tools/strverscmp_bench.c $(OUT)/string-locale-zig.o $(OUT)/string-locale-lock-zig.o $(OUT)/string-strverscmp-zig.o $(OUT)/memory-zig.o $(OUT)/memory-x86_64-simd.o -o $(OUT)/strverscmp-zig-nelibc
	@$(OUT)/strverscmp-zig-libc > $(OUT)/strverscmp-shootout-zig-libc.csv
	@$(OUT)/strverscmp-zig-nelibc > $(OUT)/strverscmp-shootout-zig-nelibc.csv
	@{ cat $(OUT)/strverscmp-shootout-zig-libc.csv; tail -n +2 $(OUT)/strverscmp-shootout-zig-nelibc.csv; } > $(OUT)/strverscmp-shootout-zig.csv
	@cat $(OUT)/strverscmp-shootout-zig.csv

strverscmp-shootout-gcc: gcc
	@$(NELUA) -B -r --cc gcc --cflags=-O2 --cflags=-fno-builtin -o $(OUT)/string-strverscmp-gcc.o src/string.nelua
	@gcc -O2 -Iinclude -fno-builtin -DBENCH_COMPILER='"GCC"' -DBENCH_LIBC='"glibc"' -DBENCH_IMPLEMENTATION='"host-libc"' tools/strverscmp_bench.c -o $(OUT)/strverscmp-gcc-libc
	@gcc -O2 -Iinclude -fno-builtin -DBENCH_COMPILER='"GCC"' -DBENCH_LIBC='"glibc"' -DBENCH_IMPLEMENTATION='"nelibc"' tools/strverscmp_bench.c $(OUT)/string-locale-gcc.o $(OUT)/string-locale-lock-gcc.o $(OUT)/string-strverscmp-gcc.o $(OUT)/memory-gcc.o $(OUT)/memory-x86_64-simd.o -o $(OUT)/strverscmp-gcc-nelibc
	@$(OUT)/strverscmp-gcc-libc > $(OUT)/strverscmp-shootout-gcc-libc.csv
	@$(OUT)/strverscmp-gcc-nelibc > $(OUT)/strverscmp-shootout-gcc-nelibc.csv
	@{ cat $(OUT)/strverscmp-shootout-gcc-libc.csv; tail -n +2 $(OUT)/strverscmp-shootout-gcc-nelibc.csv; } > $(OUT)/strverscmp-shootout-gcc.csv
	@cat $(OUT)/strverscmp-shootout-gcc.csv

strverscmp-shootout-zig-musl: zig-musl
	@$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-target=x86_64-linux-musl --cflags=-O2 --cflags=-fno-builtin -o $(OUT)/string-strverscmp-zig-musl.o src/string.nelua
	@$(ZIG) cc -target x86_64-linux-musl -static -O2 -Iinclude -fno-builtin -DNELIBC_STATIC_MUSL -Wl,--wrap=free -Wl,--wrap=realloc -DBENCH_COMPILER='"Zig cc"' -DBENCH_LIBC='"musl"' -DBENCH_IMPLEMENTATION='"host-libc"' tools/strverscmp_bench.c -o $(OUT)/strverscmp-zig-musl-libc
	@$(ZIG) cc -target x86_64-linux-musl -static -O2 -Iinclude -fno-builtin -DNELIBC_STATIC_MUSL -Wl,--wrap=free -Wl,--wrap=realloc -DBENCH_COMPILER='"Zig cc"' -DBENCH_LIBC='"musl"' -DBENCH_IMPLEMENTATION='"nelibc"' tools/strverscmp_bench.c $(OUT)/string-locale-zig-musl.o $(OUT)/string-locale-lock-zig-musl.o $(OUT)/string-strverscmp-zig-musl.o $(OUT)/memory-zig-musl.o $(OUT)/memory-x86_64-simd.o -o $(OUT)/strverscmp-zig-musl-nelibc
	@$(OUT)/strverscmp-zig-musl-libc > $(OUT)/strverscmp-shootout-zig-musl-libc.csv
	@$(OUT)/strverscmp-zig-musl-nelibc > $(OUT)/strverscmp-shootout-zig-musl-nelibc.csv
	@{ cat $(OUT)/strverscmp-shootout-zig-musl-libc.csv; tail -n +2 $(OUT)/strverscmp-shootout-zig-musl-nelibc.csv; } > $(OUT)/strverscmp-shootout-zig-musl.csv
	@cat $(OUT)/strverscmp-shootout-zig-musl.csv

strverscmp-shootout-tcc: tcc
	@$(NELUA) -B -r --cc '$(TCC)' --cflags=-O2 --cflags=-fno-builtin -o $(OUT)/string-strverscmp-tcc.o src/string.nelua
	@$(TCC) -O2 -Iinclude -fno-builtin -DBENCH_COMPILER='"TinyCC"' -DBENCH_LIBC='"glibc"' -DBENCH_IMPLEMENTATION='"host-libc"' tools/strverscmp_bench.c -o $(OUT)/strverscmp-tcc-libc
	@$(TCC) -O2 -Iinclude -fno-builtin -DBENCH_COMPILER='"TinyCC"' -DBENCH_LIBC='"glibc"' -DBENCH_IMPLEMENTATION='"nelibc"' tools/strverscmp_bench.c $(OUT)/string-locale-tcc.o $(OUT)/string-locale-lock-tcc.o $(OUT)/string-strverscmp-tcc.o $(OUT)/memory-tcc.o $(OUT)/memory-tcc-helper.o $(OUT)/memory-tcc-simd.o -o $(OUT)/strverscmp-tcc-nelibc
	@$(OUT)/strverscmp-tcc-libc > $(OUT)/strverscmp-shootout-tcc-libc.csv
	@$(OUT)/strverscmp-tcc-nelibc > $(OUT)/strverscmp-shootout-tcc-nelibc.csv
	@{ cat $(OUT)/strverscmp-shootout-tcc-libc.csv; tail -n +2 $(OUT)/strverscmp-shootout-tcc-nelibc.csv; } > $(OUT)/strverscmp-shootout-tcc.csv
	@cat $(OUT)/strverscmp-shootout-tcc.csv

strverscmp-shootout-matrix: strverscmp-shootout-zig strverscmp-shootout-gcc strverscmp-shootout-zig-musl strverscmp-shootout-tcc
	@awk 'FNR == 1 && NR > 1 { next } { print }' $(OUT)/strverscmp-shootout-zig.csv $(OUT)/strverscmp-shootout-gcc.csv $(OUT)/strverscmp-shootout-zig-musl.csv $(OUT)/strverscmp-shootout-tcc.csv > $(OUT)/strverscmp-shootout-matrix.csv
	@cat $(OUT)/strverscmp-shootout-matrix.csv

sysconf-shootout-zig: $(OUT)/libnelibc-zig.a
	@$(ZIG) cc -O2 -fno-builtin -DBENCH_COMPILER='"Zig cc"' -DBENCH_LIBC='"glibc"' -DBENCH_IMPLEMENTATION='"host-libc"' tools/sysconf_bench.c -o $(OUT)/sysconf-host-zig
	@$(ZIG) cc -O2 -fno-builtin -DBENCH_COMPILER='"Zig cc"' -DBENCH_LIBC='"glibc"' -DBENCH_IMPLEMENTATION='"nelibc"' tools/sysconf_bench.c $(OUT)/libnelibc-zig.a -o $(OUT)/sysconf-nelibc-zig
	@$(OUT)/sysconf-host-zig > $(OUT)/sysconf-shootout-zig-host.csv
	@$(OUT)/sysconf-nelibc-zig > $(OUT)/sysconf-shootout-zig-nelibc.csv
	@{ cat $(OUT)/sysconf-shootout-zig-host.csv; tail -n +2 $(OUT)/sysconf-shootout-zig-nelibc.csv; } > $(OUT)/sysconf-shootout-zig.csv
	@cat $(OUT)/sysconf-shootout-zig.csv

sysconf-shootout-gcc: $(OUT)/libnelibc-gcc.a
	@gcc -O2 -fno-builtin -DBENCH_COMPILER='"GCC"' -DBENCH_LIBC='"glibc"' -DBENCH_IMPLEMENTATION='"host-libc"' tools/sysconf_bench.c -o $(OUT)/sysconf-host-gcc
	@gcc -O2 -fno-builtin -DBENCH_COMPILER='"GCC"' -DBENCH_LIBC='"glibc"' -DBENCH_IMPLEMENTATION='"nelibc"' tools/sysconf_bench.c $(OUT)/libnelibc-gcc.a -o $(OUT)/sysconf-nelibc-gcc
	@$(OUT)/sysconf-host-gcc > $(OUT)/sysconf-shootout-gcc-host.csv
	@$(OUT)/sysconf-nelibc-gcc > $(OUT)/sysconf-shootout-gcc-nelibc.csv
	@{ cat $(OUT)/sysconf-shootout-gcc-host.csv; tail -n +2 $(OUT)/sysconf-shootout-gcc-nelibc.csv; } > $(OUT)/sysconf-shootout-gcc.csv
	@cat $(OUT)/sysconf-shootout-gcc.csv

sysconf-shootout-zig-musl: $(OUT)/libnelibc-zig-musl.a
	@$(ZIG) cc -target x86_64-linux-musl -static -O2 -fno-builtin -DBENCH_COMPILER='"Zig cc"' -DBENCH_LIBC='"musl"' -DBENCH_IMPLEMENTATION='"host-libc"' tools/sysconf_bench.c -o $(OUT)/sysconf-host-zig-musl
	@$(ZIG) cc -target x86_64-linux-musl -static -O2 -fno-builtin $(MUSL_STDIO_WRAP_FLAGS) -DBENCH_COMPILER='"Zig cc"' -DBENCH_LIBC='"musl"' -DBENCH_IMPLEMENTATION='"nelibc"' tools/sysconf_bench.c $(OUT)/libnelibc-zig-musl.a -o $(OUT)/sysconf-nelibc-zig-musl
	@$(OUT)/sysconf-host-zig-musl > $(OUT)/sysconf-shootout-zig-musl-host.csv
	@$(OUT)/sysconf-nelibc-zig-musl > $(OUT)/sysconf-shootout-zig-musl-nelibc.csv
	@{ cat $(OUT)/sysconf-shootout-zig-musl-host.csv; tail -n +2 $(OUT)/sysconf-shootout-zig-musl-nelibc.csv; } > $(OUT)/sysconf-shootout-zig-musl.csv
	@cat $(OUT)/sysconf-shootout-zig-musl.csv

sysconf-shootout-tcc: $(OUT)/libnelibc-tcc.a
	@$(TCC) -O2 -fno-builtin -DBENCH_COMPILER='"TinyCC"' -DBENCH_LIBC='"glibc"' -DBENCH_IMPLEMENTATION='"host-libc"' tools/sysconf_bench.c -o $(OUT)/sysconf-host-tcc
	@$(TCC) -O2 -fno-builtin -DBENCH_COMPILER='"TinyCC"' -DBENCH_LIBC='"glibc"' -DBENCH_IMPLEMENTATION='"nelibc"' tools/sysconf_bench.c $(OUT)/libnelibc-tcc.a -o $(OUT)/sysconf-nelibc-tcc
	@$(OUT)/sysconf-host-tcc > $(OUT)/sysconf-shootout-tcc-host.csv
	@$(OUT)/sysconf-nelibc-tcc > $(OUT)/sysconf-shootout-tcc-nelibc.csv
	@{ cat $(OUT)/sysconf-shootout-tcc-host.csv; tail -n +2 $(OUT)/sysconf-shootout-tcc-nelibc.csv; } > $(OUT)/sysconf-shootout-tcc.csv
	@cat $(OUT)/sysconf-shootout-tcc.csv

sysconf-shootout-matrix: sysconf-shootout-zig sysconf-shootout-gcc sysconf-shootout-zig-musl sysconf-shootout-tcc
	@awk 'FNR == 1 && NR > 1 { next } { print }' $(OUT)/sysconf-shootout-zig.csv $(OUT)/sysconf-shootout-gcc.csv $(OUT)/sysconf-shootout-zig-musl.csv $(OUT)/sysconf-shootout-tcc.csv > $(OUT)/sysconf-shootout-matrix.csv
	@cat $(OUT)/sysconf-shootout-matrix.csv

confstr-shootout-zig: $(OUT)/libnelibc-zig.a
	@$(ZIG) cc -O2 -fno-builtin -DBENCH_COMPILER='"Zig cc"' -DBENCH_LIBC='"glibc"' -DBENCH_IMPLEMENTATION='"host-libc"' tools/confstr_bench.c -o $(OUT)/confstr-host-zig
	@$(ZIG) cc -O2 -fno-builtin -DBENCH_COMPILER='"Zig cc"' -DBENCH_LIBC='"glibc"' -DBENCH_IMPLEMENTATION='"nelibc"' tools/confstr_bench.c $(OUT)/libnelibc-zig.a -o $(OUT)/confstr-nelibc-zig
	@$(OUT)/confstr-host-zig > $(OUT)/confstr-shootout-zig-host.csv
	@$(OUT)/confstr-nelibc-zig > $(OUT)/confstr-shootout-zig-nelibc.csv
	@{ cat $(OUT)/confstr-shootout-zig-host.csv; tail -n +2 $(OUT)/confstr-shootout-zig-nelibc.csv; } > $(OUT)/confstr-shootout-zig.csv
	@cat $(OUT)/confstr-shootout-zig.csv

confstr-shootout-gcc: $(OUT)/libnelibc-gcc.a
	@gcc -O2 -fno-builtin -DBENCH_COMPILER='"GCC"' -DBENCH_LIBC='"glibc"' -DBENCH_IMPLEMENTATION='"host-libc"' tools/confstr_bench.c -o $(OUT)/confstr-host-gcc
	@gcc -O2 -fno-builtin -DBENCH_COMPILER='"GCC"' -DBENCH_LIBC='"glibc"' -DBENCH_IMPLEMENTATION='"nelibc"' tools/confstr_bench.c $(OUT)/libnelibc-gcc.a -o $(OUT)/confstr-nelibc-gcc
	@$(OUT)/confstr-host-gcc > $(OUT)/confstr-shootout-gcc-host.csv
	@$(OUT)/confstr-nelibc-gcc > $(OUT)/confstr-shootout-gcc-nelibc.csv
	@{ cat $(OUT)/confstr-shootout-gcc-host.csv; tail -n +2 $(OUT)/confstr-shootout-gcc-nelibc.csv; } > $(OUT)/confstr-shootout-gcc.csv
	@cat $(OUT)/confstr-shootout-gcc.csv

confstr-shootout-zig-musl: $(OUT)/libnelibc-zig-musl.a
	@$(ZIG) cc -target x86_64-linux-musl -static -O2 -fno-builtin -DBENCH_COMPILER='"Zig cc"' -DBENCH_LIBC='"musl"' -DBENCH_IMPLEMENTATION='"host-libc"' tools/confstr_bench.c -o $(OUT)/confstr-host-zig-musl
	@$(ZIG) cc -target x86_64-linux-musl -static -O2 -fno-builtin $(MUSL_STDIO_WRAP_FLAGS) -DBENCH_COMPILER='"Zig cc"' -DBENCH_LIBC='"musl"' -DBENCH_IMPLEMENTATION='"nelibc"' tools/confstr_bench.c $(OUT)/libnelibc-zig-musl.a -o $(OUT)/confstr-nelibc-zig-musl
	@$(OUT)/confstr-host-zig-musl > $(OUT)/confstr-shootout-zig-musl-host.csv
	@$(OUT)/confstr-nelibc-zig-musl > $(OUT)/confstr-shootout-zig-musl-nelibc.csv
	@{ cat $(OUT)/confstr-shootout-zig-musl-host.csv; tail -n +2 $(OUT)/confstr-shootout-zig-musl-nelibc.csv; } > $(OUT)/confstr-shootout-zig-musl.csv
	@cat $(OUT)/confstr-shootout-zig-musl.csv

confstr-shootout-tcc: $(OUT)/libnelibc-tcc.a
	@$(TCC) -O2 -fno-builtin -DBENCH_COMPILER='"TinyCC"' -DBENCH_LIBC='"glibc"' -DBENCH_IMPLEMENTATION='"host-libc"' tools/confstr_bench.c -o $(OUT)/confstr-host-tcc
	@$(TCC) -O2 -fno-builtin -DBENCH_COMPILER='"TinyCC"' -DBENCH_LIBC='"glibc"' -DBENCH_IMPLEMENTATION='"nelibc"' tools/confstr_bench.c $(OUT)/libnelibc-tcc.a -o $(OUT)/confstr-nelibc-tcc
	@$(OUT)/confstr-host-tcc > $(OUT)/confstr-shootout-tcc-host.csv
	@$(OUT)/confstr-nelibc-tcc > $(OUT)/confstr-shootout-tcc-nelibc.csv
	@{ cat $(OUT)/confstr-shootout-tcc-host.csv; tail -n +2 $(OUT)/confstr-shootout-tcc-nelibc.csv; } > $(OUT)/confstr-shootout-tcc.csv
	@cat $(OUT)/confstr-shootout-tcc.csv

confstr-shootout-matrix: confstr-shootout-zig confstr-shootout-gcc confstr-shootout-zig-musl confstr-shootout-tcc
	@awk 'FNR == 1 && NR > 1 { next } { print }' $(OUT)/confstr-shootout-zig.csv $(OUT)/confstr-shootout-gcc.csv $(OUT)/confstr-shootout-zig-musl.csv $(OUT)/confstr-shootout-tcc.csv > $(OUT)/confstr-shootout-matrix.csv
	@cat $(OUT)/confstr-shootout-matrix.csv

pathconf-shootout-zig: $(OUT)/libnelibc-zig.a
	@$(ZIG) cc -O2 -fno-builtin -DBENCH_COMPILER='"Zig cc"' -DBENCH_LIBC='"glibc"' -DBENCH_IMPLEMENTATION='"host-libc"' tools/pathconf_bench.c -o $(OUT)/pathconf-host-zig
	@$(ZIG) cc -O2 -fno-builtin -DBENCH_NELIBC -DBENCH_COMPILER='"Zig cc"' -DBENCH_LIBC='"glibc"' -DBENCH_IMPLEMENTATION='"nelibc"' tools/pathconf_bench.c $(OUT)/libnelibc-zig.a -o $(OUT)/pathconf-nelibc-zig
	@sh tools/pathconf_shootout.sh $(OUT)/pathconf-host-zig $(OUT)/pathconf-nelibc-zig $(OUT)/pathconf-shootout-zig $(PATHCONF_PATHS)

pathconf-shootout-gcc: $(OUT)/libnelibc-gcc.a
	@gcc -O2 -fno-builtin -DBENCH_COMPILER='"GCC"' -DBENCH_LIBC='"glibc"' -DBENCH_IMPLEMENTATION='"host-libc"' tools/pathconf_bench.c -o $(OUT)/pathconf-host-gcc
	@gcc -O2 -fno-builtin -DBENCH_NELIBC -DBENCH_COMPILER='"GCC"' -DBENCH_LIBC='"glibc"' -DBENCH_IMPLEMENTATION='"nelibc"' tools/pathconf_bench.c $(OUT)/libnelibc-gcc.a -o $(OUT)/pathconf-nelibc-gcc
	@sh tools/pathconf_shootout.sh $(OUT)/pathconf-host-gcc $(OUT)/pathconf-nelibc-gcc $(OUT)/pathconf-shootout-gcc $(PATHCONF_PATHS)

pathconf-shootout-zig-musl: $(OUT)/libnelibc-zig-musl.a
	@$(ZIG) cc -target x86_64-linux-musl -static -O2 -fno-builtin -DBENCH_COMPILER='"Zig cc"' -DBENCH_LIBC='"musl"' -DBENCH_IMPLEMENTATION='"host-libc"' tools/pathconf_bench.c -o $(OUT)/pathconf-host-zig-musl
	@$(ZIG) cc -target x86_64-linux-musl -static -O2 -fno-builtin $(MUSL_STDIO_WRAP_FLAGS) -DBENCH_NELIBC -DBENCH_COMPILER='"Zig cc"' -DBENCH_LIBC='"musl"' -DBENCH_IMPLEMENTATION='"nelibc"' tools/pathconf_bench.c $(OUT)/libnelibc-zig-musl.a -o $(OUT)/pathconf-nelibc-zig-musl
	@sh tools/pathconf_shootout.sh $(OUT)/pathconf-host-zig-musl $(OUT)/pathconf-nelibc-zig-musl $(OUT)/pathconf-shootout-zig-musl $(PATHCONF_PATHS)

pathconf-shootout-tcc: $(OUT)/libnelibc-tcc.a
	@$(TCC) -O2 -fno-builtin -DBENCH_COMPILER='"TinyCC"' -DBENCH_LIBC='"glibc"' -DBENCH_IMPLEMENTATION='"host-libc"' tools/pathconf_bench.c -o $(OUT)/pathconf-host-tcc
	@$(TCC) -O2 -fno-builtin -DBENCH_NELIBC -DBENCH_COMPILER='"TinyCC"' -DBENCH_LIBC='"glibc"' -DBENCH_IMPLEMENTATION='"nelibc"' tools/pathconf_bench.c $(OUT)/libnelibc-tcc.a -o $(OUT)/pathconf-nelibc-tcc
	@sh tools/pathconf_shootout.sh $(OUT)/pathconf-host-tcc $(OUT)/pathconf-nelibc-tcc $(OUT)/pathconf-shootout-tcc $(PATHCONF_PATHS)

pathconf-shootout-matrix: pathconf-shootout-zig pathconf-shootout-gcc pathconf-shootout-zig-musl pathconf-shootout-tcc
	@awk 'FNR == 1 && NR > 1 { next } { print }' $(OUT)/pathconf-shootout-zig.csv $(OUT)/pathconf-shootout-gcc.csv $(OUT)/pathconf-shootout-zig-musl.csv $(OUT)/pathconf-shootout-tcc.csv > $(OUT)/pathconf-shootout-matrix.csv
	@cat $(OUT)/pathconf-shootout-matrix.csv

.PHONY: strcasecmp-shootout-zig strcasecmp-shootout-gcc strcasecmp-shootout-zig-musl strcasecmp-shootout-tcc strcasecmp-shootout-matrix strcasecmp-sse2-fallback strcasecmp-perf-matrix strverscmp-shootout-zig strverscmp-shootout-gcc strverscmp-shootout-zig-musl strverscmp-shootout-tcc strverscmp-shootout-matrix sysconf-shootout-zig sysconf-shootout-gcc sysconf-shootout-zig-musl sysconf-shootout-tcc sysconf-shootout-matrix confstr-shootout-zig confstr-shootout-gcc confstr-shootout-zig-musl confstr-shootout-tcc confstr-shootout-matrix pathconf-shootout-zig pathconf-shootout-gcc pathconf-shootout-zig-musl pathconf-shootout-tcc pathconf-shootout-matrix

shootout-matrix: shootout-zig shootout-gcc shootout-zig-musl shootout-tcc strstr-shootout-matrix strcasecmp-shootout-matrix strverscmp-shootout-matrix sysconf-shootout-matrix confstr-shootout-matrix pathconf-shootout-matrix float-shootout-matrix int-shootout-matrix rand-r-shootout-matrix qsort-shootout-matrix stdio-shootout-matrix io-shootout-matrix
	@awk 'FNR == 1 && NR > 1 { next } { print }' $(OUT)/mem-shootout-zig.csv $(OUT)/mem-shootout-gcc.csv $(OUT)/mem-shootout-zig-musl.csv $(OUT)/mem-shootout-tcc.csv > $(OUT)/mem-shootout-matrix.csv
	@cat $(OUT)/mem-shootout-matrix.csv

memscan-shootout-matrix: bench-zig bench-gcc bench-zig-musl bench-tcc
	@printf 'compiler,libc,implementation,function,size_bytes,iterations,median_ns_per_call\n' > $(OUT)/memscan-shootout-matrix.csv
	@for variant in zig gcc zig-musl tcc; do \
	  for implementation in libc nelibc; do \
	    $(OUT)/mem-bench-$$variant-$$implementation | awk -F, 'NR > 1 && ($$4 == "memchr" || $$4 == "memrchr") { print }' >> $(OUT)/memscan-shootout-matrix.csv; \
	  done; \
	done
	@cat $(OUT)/memscan-shootout-matrix.csv

memscan-threshold-shootout: $(OUT) $(OUT)/memory-x86_64-sse2-baseline.o
	@$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-O2 --cflags=-fno-builtin -o $(OUT)/memory-memscan-threshold-zig.o src/memory.nelua
	@$(ZIG) cc -O2 -Iinclude -fno-builtin -DBENCH_COMPILER='"Zig cc"' -DBENCH_LIBC='"glibc"' -DBENCH_IMPLEMENTATION='"libc"' tools/memscan_bench.c -o $(OUT)/memscan-threshold-libc
	@$(ZIG) cc -O2 -Iinclude -fno-builtin -DBENCH_COMPILER='"Zig cc"' -DBENCH_LIBC='"glibc"' -DBENCH_IMPLEMENTATION='"nelibc-sse2"' tools/memscan_bench.c $(OUT)/memory-memscan-threshold-zig.o $(OUT)/memory-x86_64-sse2-baseline.o -o $(OUT)/memscan-threshold-sse2
	@for threshold in 2048 4096 8192; do \
	  $(ZIG) cc -c -DNELIBC_MEMSCAN_AVX2_THRESHOLD=$$threshold -DNELIBC_MEMSCAN_AVX2_BLOCK_SIZE=128 src/memory_x86_64.S -o $(OUT)/memory-memscan-avx2-$$threshold.o; \
	  $(ZIG) cc -O2 -Iinclude -fno-builtin -DBENCH_COMPILER='"Zig cc"' -DBENCH_LIBC='"glibc"' -DBENCH_IMPLEMENTATION='"nelibc-avx2-'$$threshold'-128"' tools/memscan_bench.c $(OUT)/memory-memscan-threshold-zig.o $(OUT)/memory-memscan-avx2-$$threshold.o -o $(OUT)/memscan-avx2-$$threshold; \
	done
	@$(ZIG) cc -c -DNELIBC_MEMSCAN_AVX2_THRESHOLD=2048 -DNELIBC_MEMSCAN_AVX2_BLOCK_SIZE=256 src/memory_x86_64.S -o $(OUT)/memory-memscan-avx2-2048-256.o
	@$(ZIG) cc -O2 -Iinclude -fno-builtin -DBENCH_COMPILER='"Zig cc"' -DBENCH_LIBC='"glibc"' -DBENCH_IMPLEMENTATION='"nelibc-avx2-2048-256"' tools/memscan_bench.c $(OUT)/memory-memscan-threshold-zig.o $(OUT)/memory-memscan-avx2-2048-256.o -o $(OUT)/memscan-avx2-2048-256
	@$(OUT)/memscan-threshold-libc > $(OUT)/memscan-threshold-libc.csv
	@$(OUT)/memscan-threshold-sse2 > $(OUT)/memscan-threshold-sse2.csv
	@for threshold in 2048 4096 8192; do $(OUT)/memscan-avx2-$$threshold > $(OUT)/memscan-avx2-$$threshold.csv; done
	@$(OUT)/memscan-avx2-2048-256 > $(OUT)/memscan-avx2-2048-256.csv
	@{ cat $(OUT)/memscan-threshold-libc.csv; tail -n +2 $(OUT)/memscan-threshold-sse2.csv; tail -n +2 $(OUT)/memscan-avx2-2048.csv; tail -n +2 $(OUT)/memscan-avx2-4096.csv; tail -n +2 $(OUT)/memscan-avx2-8192.csv; tail -n +2 $(OUT)/memscan-avx2-2048-256.csv; } > $(OUT)/memscan-threshold-shootout.csv
	@cat $(OUT)/memscan-threshold-shootout.csv

syscall-zig: $(OUT) $(OUT)/memory-x86_64-simd.o $(OUT)/string-locale-zig.o $(OUT)/string-locale-lock-zig.o
	$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-fno-builtin -o $(OUT)/memory-syscall-zig.o src/memory.nelua
	$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-fno-builtin -o $(OUT)/string-syscall-zig.o src/string.nelua
	$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-fno-builtin -o $(OUT)/stdlib-syscall-zig.o src/stdlib.nelua
	$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-fno-builtin -o $(OUT)/sys-linux-x86_64-zig.o src/sys/linux_x86_64.nelua
	$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-fno-builtin -o $(OUT)/errno-host-zig.o src/errno/host.nelua
	$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-fno-builtin -o $(OUT)/alloc-zig.o src/alloc.nelua
	$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-fno-builtin -o $(OUT)/unistd-zig.o src/unistd.nelua
	$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-fno-builtin -D NELIBC_TESTING=true -o $(OUT)/unistd-smoke-zig.o src/unistd.nelua
	$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-fno-builtin -o $(OUT)/stdio-hybrid-zig.o src/stdio_hybrid.nelua
	$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-fno-builtin -o $(OUT)/open-memstream-hybrid-zig.o src/open_memstream_hybrid.nelua
	$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-fno-builtin -o $(OUT)/fmemopen-hybrid-zig.o src/fmemopen_hybrid.nelua
	$(ZIG) cc -c src/sys/linux_x86_64.S -o $(OUT)/sys-linux-x86_64-zig-asm.o
	$(ZIG) cc -pthread -Iinclude -fno-builtin -DNELIBC_TESTING tools/syscall_smoke.c $(OUT)/string-locale-zig.o $(OUT)/string-locale-lock-zig.o src/fcntl_adapter.c src/credential_adapter.c src/stdio_host_adapter.c src/alloc_hybrid_adapter.c tools/syscall_interpose_smoke.c $(OUT)/string-syscall-zig.o $(OUT)/stdlib-syscall-zig.o $(OUT)/memory-syscall-zig.o $(OUT)/memory-x86_64-simd.o $(OUT)/unistd-smoke-zig.o $(OUT)/stdio-hybrid-zig.o $(OUT)/open-memstream-hybrid-zig.o $(OUT)/fmemopen-hybrid-zig.o $(OUT)/alloc-zig.o $(OUT)/errno-host-zig.o $(OUT)/sys-linux-x86_64-zig.o $(OUT)/sys-linux-x86_64-zig-asm.o src/environment_adapter.c -o $(OUT)/syscall-smoke-zig

smoke-syscall-zig: syscall-zig
	NELIBC_GETENV_PROBE=visible $(OUT)/syscall-smoke-zig

syscall-gcc: $(OUT) $(OUT)/memory-x86_64-simd.o $(OUT)/string-locale-gcc.o $(OUT)/string-locale-lock-gcc.o
	$(NELUA) -B -r --cc gcc --cflags=-fno-builtin -o $(OUT)/memory-syscall-gcc.o src/memory.nelua
	$(NELUA) -B -r --cc gcc --cflags=-fno-builtin -o $(OUT)/string-syscall-gcc.o src/string.nelua
	$(NELUA) -B -r --cc gcc --cflags=-fno-builtin -o $(OUT)/stdlib-syscall-gcc.o src/stdlib.nelua
	$(NELUA) -B -r --cc gcc --cflags=-fno-builtin -o $(OUT)/sys-linux-x86_64-gcc.o src/sys/linux_x86_64.nelua
	$(NELUA) -B -r --cc gcc --cflags=-fno-builtin -o $(OUT)/errno-host-gcc.o src/errno/host.nelua
	$(NELUA) -B -r --cc gcc --cflags=-fno-builtin -o $(OUT)/alloc-gcc.o src/alloc.nelua
	$(NELUA) -B -r --cc gcc --cflags=-fno-builtin -o $(OUT)/unistd-gcc.o src/unistd.nelua
	$(NELUA) -B -r --cc gcc --cflags=-fno-builtin -D NELIBC_TESTING=true -o $(OUT)/unistd-smoke-gcc.o src/unistd.nelua
	$(NELUA) -B -r --cc gcc --cflags=-fno-builtin -o $(OUT)/stdio-hybrid-gcc.o src/stdio_hybrid.nelua
	$(NELUA) -B -r --cc gcc --cflags=-fno-builtin -o $(OUT)/open-memstream-hybrid-gcc.o src/open_memstream_hybrid.nelua
	$(NELUA) -B -r --cc gcc --cflags=-fno-builtin -o $(OUT)/fmemopen-hybrid-gcc.o src/fmemopen_hybrid.nelua
	gcc -c src/sys/linux_x86_64.S -o $(OUT)/sys-linux-x86_64-gcc-asm.o
	gcc -pthread -Iinclude -fno-builtin -DNELIBC_TESTING tools/syscall_smoke.c $(OUT)/string-locale-gcc.o $(OUT)/string-locale-lock-gcc.o src/fcntl_adapter.c src/credential_adapter.c src/stdio_host_adapter.c src/alloc_hybrid_adapter.c tools/syscall_interpose_smoke.c $(OUT)/string-syscall-gcc.o $(OUT)/stdlib-syscall-gcc.o $(OUT)/memory-syscall-gcc.o $(OUT)/memory-x86_64-simd.o $(OUT)/unistd-smoke-gcc.o $(OUT)/stdio-hybrid-gcc.o $(OUT)/open-memstream-hybrid-gcc.o $(OUT)/fmemopen-hybrid-gcc.o $(OUT)/alloc-gcc.o $(OUT)/errno-host-gcc.o $(OUT)/sys-linux-x86_64-gcc.o $(OUT)/sys-linux-x86_64-gcc-asm.o src/environment_adapter.c -o $(OUT)/syscall-smoke-gcc

smoke-syscall-gcc: syscall-gcc
	NELIBC_GETENV_PROBE=visible $(OUT)/syscall-smoke-gcc

syscall-zig-musl: $(OUT) $(OUT)/memory-x86_64-simd.o $(OUT)/string-locale-zig-musl.o $(OUT)/string-locale-lock-zig-musl.o
	$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-target=x86_64-linux-musl --cflags=-fno-builtin -o $(OUT)/memory-syscall-zig-musl.o src/memory.nelua
	$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-target=x86_64-linux-musl --cflags=-fno-builtin -o $(OUT)/string-syscall-zig-musl.o src/string.nelua
	$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-target=x86_64-linux-musl --cflags=-fno-builtin -o $(OUT)/stdlib-syscall-zig-musl.o src/stdlib.nelua
	$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-target=x86_64-linux-musl --cflags=-fno-builtin -o $(OUT)/sys-linux-x86_64-zig-musl.o src/sys/linux_x86_64.nelua
	$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-target=x86_64-linux-musl --cflags=-fno-builtin -o $(OUT)/errno-host-zig-musl.o src/errno/host.nelua
	$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-target=x86_64-linux-musl --cflags=-fno-builtin -o $(OUT)/alloc-zig-musl.o src/alloc.nelua
	$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-target=x86_64-linux-musl --cflags=-fno-builtin -o $(OUT)/unistd-zig-musl.o src/unistd.nelua
	$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-target=x86_64-linux-musl --cflags=-fno-builtin -D NELIBC_TESTING=true -o $(OUT)/unistd-smoke-zig-musl.o src/unistd.nelua
	$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-target=x86_64-linux-musl --cflags=-fno-builtin -o $(OUT)/stdio-hybrid-zig-musl.o src/stdio_hybrid_musl.nelua
	$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-target=x86_64-linux-musl --cflags=-fno-builtin -o $(OUT)/open-memstream-hybrid-zig-musl.o src/open_memstream_hybrid.nelua
	$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-target=x86_64-linux-musl --cflags=-fno-builtin -o $(OUT)/fmemopen-hybrid-zig-musl.o src/fmemopen_hybrid.nelua
	$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-target=x86_64-linux-musl --cflags=-fno-builtin -o $(OUT)/atexit-musl-hybrid-zig-musl.o src/atexit_musl_hybrid.nelua
	$(ZIG) cc -target x86_64-linux-musl -c src/sys/linux_x86_64.S -o $(OUT)/sys-linux-x86_64-zig-musl-asm.o
	$(ZIG) cc -pthread -target x86_64-linux-musl -static -Iinclude -fno-builtin -DNELIBC_STATIC_MUSL $(MUSL_STDIO_WRAP_FLAGS) -DNELIBC_TESTING tools/syscall_smoke.c $(OUT)/string-locale-zig-musl.o $(OUT)/string-locale-lock-zig-musl.o src/fcntl_adapter.c src/credential_adapter.c src/stdio_host_adapter.c src/alloc_hybrid_adapter.c tools/syscall_interpose_smoke.c $(OUT)/string-syscall-zig-musl.o $(OUT)/stdlib-syscall-zig-musl.o $(OUT)/memory-syscall-zig-musl.o $(OUT)/memory-x86_64-simd.o $(OUT)/unistd-smoke-zig-musl.o $(OUT)/stdio-hybrid-zig-musl.o $(OUT)/open-memstream-hybrid-zig-musl.o $(OUT)/fmemopen-hybrid-zig-musl.o $(OUT)/atexit-musl-hybrid-zig-musl.o $(OUT)/alloc-zig-musl.o $(OUT)/errno-host-zig-musl.o $(OUT)/sys-linux-x86_64-zig-musl.o $(OUT)/sys-linux-x86_64-zig-musl-asm.o src/environment_adapter.c -o $(OUT)/syscall-smoke-zig-musl

smoke-syscall-zig-musl: syscall-zig-musl
	NELIBC_GETENV_PROBE=visible $(OUT)/syscall-smoke-zig-musl

syscall-tcc: tcc $(OUT)/memory-x86_64-simd.o
	@$(NELUA) -B -r --cc '$(TCC)' --cflags=-fno-builtin -o $(OUT)/sys-linux-x86_64-tcc.o src/sys/linux_x86_64.nelua
	@$(NELUA) -B -r --cc '$(TCC)' --cflags=-fno-builtin -o $(OUT)/alloc-tcc.o src/alloc.nelua
	@$(NELUA) -B -r --cc '$(TCC)' --cflags=-fno-builtin -o $(OUT)/unistd-tcc.o src/unistd.nelua
	@$(NELUA) -B -r --cc '$(TCC)' --cflags=-fno-builtin -D NELIBC_TESTING=true -o $(OUT)/unistd-smoke-tcc.o src/unistd.nelua
	@$(NELUA) -B -r --cc '$(TCC)' --cflags=-fno-builtin -o $(OUT)/open-memstream-hybrid-tcc.o src/open_memstream_hybrid.nelua
	@$(NELUA) -B -r --cc '$(TCC)' --cflags=-fno-builtin -o $(OUT)/fmemopen-hybrid-tcc.o src/fmemopen_hybrid.nelua
	@$(ZIG) cc -c src/sys/linux_x86_64.S -o $(OUT)/sys-linux-x86_64-tcc-asm.o
	# TinyCC lacks max_align_t; long double supplies its x86-64 alignment here.
	@$(TCC) -pthread -Iinclude -fno-builtin -Dmax_align_t='long double' -DNELIBC_TESTING -c tools/syscall_smoke.c -o $(OUT)/syscall-smoke-tcc.o
	@$(TCC) -Iinclude -fno-builtin -Dmax_align_t='long double' -DNELIBC_TESTING -c src/fcntl_adapter.c -o $(OUT)/fcntl-adapter-syscall-tcc.o
	@$(TCC) -Iinclude -fno-builtin -Dmax_align_t='long double' -DNELIBC_TESTING -c src/credential_adapter.c -o $(OUT)/credential-adapter-syscall-tcc.o
	@$(TCC) -Iinclude -fno-builtin -Dmax_align_t='long double' -DNELIBC_TESTING -c src/stdio_host_adapter.c -o $(OUT)/stdio-host-adapter-syscall-tcc.o
	@$(TCC) -Iinclude -fno-builtin -Dmax_align_t='long double' -DNELIBC_TESTING -c src/alloc_hybrid_adapter.c -o $(OUT)/alloc-hybrid-adapter-syscall-tcc.o
	@$(TCC) -Iinclude -fno-builtin -Dmax_align_t='long double' -DNELIBC_TESTING -c tools/syscall_interpose_smoke.c -o $(OUT)/syscall-interpose-smoke-tcc.o
	$(ZIG) cc -pthread -Iinclude -fno-builtin $(OUT)/syscall-smoke-tcc.o $(OUT)/fcntl-adapter-syscall-tcc.o $(OUT)/credential-adapter-syscall-tcc.o $(OUT)/stdio-host-adapter-syscall-tcc.o $(OUT)/alloc-hybrid-adapter-syscall-tcc.o $(OUT)/syscall-interpose-smoke-tcc.o $(OUT)/string-locale-tcc.o $(OUT)/string-locale-lock-tcc.o $(OUT)/string-tcc.o $(OUT)/stdlib-tcc.o $(OUT)/memory-tcc.o $(OUT)/memory-tcc-helper.o $(OUT)/memory-tcc-simd.o $(OUT)/unistd-smoke-tcc.o $(OUT)/stdio-hybrid-tcc.o $(OUT)/open-memstream-hybrid-tcc.o $(OUT)/fmemopen-hybrid-tcc.o $(OUT)/alloc-tcc.o $(OUT)/errno-host-tcc.o $(OUT)/sys-linux-x86_64-tcc.o $(OUT)/sys-linux-x86_64-tcc-asm.o $(OUT)/environment-adapter-smoke-tcc.o $(TCC_LIBTCC1) -o $(OUT)/syscall-smoke-tcc

smoke-syscall-tcc: syscall-tcc
	NELIBC_GETENV_PROBE=visible $(OUT)/syscall-smoke-tcc

syscall-tls-gcc: $(OUT) $(OUT)/memory-x86_64-simd.o $(OUT)/string-locale-gcc.o $(OUT)/string-locale-lock-gcc.o
	$(NELUA) -B -r --cc gcc --cflags=-fno-builtin -o $(OUT)/memory-syscall-tls-gcc.o src/memory.nelua
	$(NELUA) -B -r --cc gcc --cflags=-fno-builtin -o $(OUT)/string-syscall-tls-gcc.o src/string.nelua
	$(NELUA) -B -r --cc gcc --cflags=-fno-builtin -o $(OUT)/stdlib-syscall-tls-gcc.o src/stdlib.nelua
	$(NELUA) -B -r --cc gcc --cflags=-fno-builtin -o $(OUT)/sys-linux-x86_64-tls-gcc.o src/sys/linux_x86_64.nelua
	$(NELUA) -B -r --cc gcc --cflags=-fno-builtin --cflags=-ftls-model=initial-exec -o $(OUT)/errno-tls-gcc.o src/errno/standalone.nelua
	$(NELUA) -B -r --cc gcc --cflags=-fno-builtin -o $(OUT)/alloc-tls-gcc.o src/alloc.nelua
	$(NELUA) -B -r --cc gcc --cflags=-fno-builtin -o $(OUT)/unistd-tls-gcc.o src/unistd.nelua
	$(NELUA) -B -r --cc gcc --cflags=-fno-builtin -o $(OUT)/stdio-hybrid-tls-gcc.o src/stdio_hybrid.nelua
	$(NELUA) -B -r --cc gcc --cflags=-fno-builtin -o $(OUT)/open-memstream-hybrid-tls-gcc.o src/open_memstream_hybrid.nelua
	$(NELUA) -B -r --cc gcc --cflags=-fno-builtin -o $(OUT)/fmemopen-hybrid-tls-gcc.o src/fmemopen_hybrid.nelua
	gcc -c src/sys/linux_x86_64.S -o $(OUT)/sys-linux-x86_64-tls-gcc-asm.o
	gcc -pthread -Iinclude -fno-builtin -DNELIBC_WITH_UNISTD tools/syscall_smoke.c $(OUT)/string-locale-gcc.o $(OUT)/string-locale-lock-gcc.o src/fcntl_adapter.c src/credential_adapter.c src/stdio_host_adapter.c tools/syscall_interpose_smoke.c $(OUT)/string-syscall-tls-gcc.o $(OUT)/stdlib-syscall-tls-gcc.o $(OUT)/memory-syscall-tls-gcc.o $(OUT)/memory-x86_64-simd.o $(OUT)/unistd-tls-gcc.o $(OUT)/stdio-hybrid-tls-gcc.o $(OUT)/open-memstream-hybrid-tls-gcc.o $(OUT)/fmemopen-hybrid-tls-gcc.o $(OUT)/alloc-tls-gcc.o $(OUT)/errno-tls-gcc.o $(OUT)/sys-linux-x86_64-tls-gcc.o $(OUT)/sys-linux-x86_64-tls-gcc-asm.o src/environment_adapter.c -o $(OUT)/syscall-smoke-tls-gcc
	gcc -pthread -fno-builtin tools/errno_tls_smoke.c $(OUT)/errno-tls-gcc.o -o $(OUT)/errno-tls-smoke-gcc

smoke-syscall-tls-gcc: syscall-tls-gcc
	NELIBC_GETENV_PROBE=visible $(OUT)/syscall-smoke-tls-gcc
	$(OUT)/errno-tls-smoke-gcc

smoke-matrix: unistd-header-smoke smoke-zig smoke-gcc smoke-zig-musl smoke-tcc float-diff-matrix int-diff-matrix collation-diff-matrix smoke-syscall-zig smoke-syscall-gcc smoke-syscall-zig-musl smoke-syscall-tcc smoke-syscall-tls-gcc smoke-freestanding-zig smoke-freestanding-gcc smoke-standalone-zig smoke-standalone-gcc smoke-standalone-tcc exit-smoke-standalone-matrix abort-smoke-matrix lib-smoke-matrix smoke-lib-standalone-matrix lib-consumer-matrix lib-stdio-smoke-matrix atexit-hybrid-smoke-matrix stdio-format-diff-matrix unistd-cancel-matrix lib-consumer-standalone-matrix math-core-diff-matrix math-core-standalone-matrix

errno-tls-zig-musl: $(OUT)
	$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-target=x86_64-linux-musl --cflags=-fno-builtin --cflags=-ftls-model=initial-exec -o $(OUT)/errno-tls-zig-musl.o src/errno/standalone.nelua

freestanding-zig: $(OUT) $(OUT)/memory-x86_64-simd.o
	$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-fno-builtin --cflags=-ffreestanding --cflags=-fno-stack-protector --cflags=-DNELIBC_STANDALONE -o $(OUT)/string-freestanding-zig.o src/string.nelua
	$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-fno-builtin --cflags=-ffreestanding --cflags=-fno-stack-protector -o $(OUT)/string-locale-freestanding-zig.o src/string_locale_standalone.nelua
	$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-fno-builtin --cflags=-ffreestanding --cflags=-fno-stack-protector -o $(OUT)/memory-freestanding-zig.o src/memory.nelua
	$(ZIG) cc -DNELIBC_FREESTANDING -ffreestanding -fno-builtin -fno-stack-protector -c src/string_locale_lock.c -o $(OUT)/string-tls-freestanding-zig.o
	$(ZIG) cc -c src/sys/start_linux_x86_64.S -o $(OUT)/start-linux-x86_64-zig.o
	$(ZIG) cc -DNELIBC_STANDALONE -nostdlib -static -no-pie -fno-pie -fno-stack-protector -ffreestanding -fno-builtin -fno-sanitize=all -Wl,-e,_start -Wl,--build-id=none -Iinclude tools/freestanding_smoke.c src/stdio_startup_stub.c $(OUT)/string-locale-freestanding-zig.o $(OUT)/string-tls-freestanding-zig.o $(OUT)/string-freestanding-zig.o $(OUT)/memory-freestanding-zig.o $(OUT)/memory-x86_64-simd.o $(OUT)/start-linux-x86_64-zig.o -o $(OUT)/freestanding-zig
	@test -z "$$(nm -u $(OUT)/freestanding-zig)"

smoke-freestanding-zig: freestanding-zig
	$(OUT)/freestanding-zig

freestanding-gcc: $(OUT) $(OUT)/memory-x86_64-simd.o
	$(NELUA) -B -r --cc gcc --cflags=-fno-builtin --cflags=-ffreestanding --cflags=-fno-stack-protector --cflags=-fno-tree-loop-distribute-patterns --cflags=-DNELIBC_STANDALONE -o $(OUT)/string-freestanding-gcc.o src/string.nelua
	$(NELUA) -B -r --cc gcc --cflags=-fno-builtin --cflags=-ffreestanding --cflags=-fno-stack-protector --cflags=-fno-tree-loop-distribute-patterns -o $(OUT)/string-locale-freestanding-gcc.o src/string_locale_standalone.nelua
	$(NELUA) -B -r --cc gcc --cflags=-fno-builtin --cflags=-ffreestanding --cflags=-fno-stack-protector --cflags=-fno-tree-loop-distribute-patterns -o $(OUT)/memory-freestanding-gcc.o src/memory.nelua
	gcc -DNELIBC_FREESTANDING -ffreestanding -fno-builtin -fno-stack-protector -fno-tree-loop-distribute-patterns -c src/string_locale_lock.c -o $(OUT)/string-tls-freestanding-gcc.o
	gcc -c src/sys/start_linux_x86_64.S -o $(OUT)/start-linux-x86_64-gcc.o
	gcc -DNELIBC_STANDALONE -nostdlib -static -no-pie -fno-pie -fno-stack-protector -ffreestanding -fno-builtin -fno-sanitize=all -fno-tree-loop-distribute-patterns -Wl,-e,_start -Wl,--build-id=none -Iinclude tools/freestanding_smoke.c src/stdio_startup_stub.c $(OUT)/string-locale-freestanding-gcc.o $(OUT)/string-tls-freestanding-gcc.o $(OUT)/string-freestanding-gcc.o $(OUT)/memory-freestanding-gcc.o $(OUT)/memory-x86_64-simd.o $(OUT)/start-linux-x86_64-gcc.o -o $(OUT)/freestanding-gcc
	@test -z "$$(nm -u $(OUT)/freestanding-gcc)"

smoke-freestanding-gcc: freestanding-gcc
	$(OUT)/freestanding-gcc

standalone-zig: $(OUT) $(OUT)/memory-x86_64-simd.o
	$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-fno-builtin --cflags=-ffreestanding --cflags=-fno-stack-protector --cflags=-DNELIBC_STANDALONE -o $(OUT)/string-standalone-zig.o src/string.nelua
	$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-fno-builtin --cflags=-ffreestanding --cflags=-fno-stack-protector -o $(OUT)/string-locale-standalone-zig.o src/string_locale_standalone.nelua
	$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-fno-builtin --cflags=-ffreestanding --cflags=-fno-stack-protector -o $(OUT)/memory-standalone-zig.o src/memory.nelua
	$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-fno-builtin --cflags=-ffreestanding --cflags=-fno-stack-protector -o $(OUT)/ctype-standalone-zig.o src/ctype.nelua
	$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-fno-builtin --cflags=-ffreestanding --cflags=-fno-stack-protector --cflags=-ftls-model=local-exec -o $(OUT)/stdlib-standalone-zig.o src/stdlib.nelua
	$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-fno-builtin --cflags=-ffreestanding --cflags=-fno-stack-protector -o $(OUT)/locale-standalone-zig.o src/locale_standalone.nelua
	$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-fno-builtin --cflags=-ffreestanding --cflags=-fno-stack-protector -o $(OUT)/stdlib-exit-standalone-zig.o src/stdlib_exit_standalone.nelua
	$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-fno-builtin --cflags=-ffreestanding --cflags=-fno-stack-protector -o $(OUT)/stdlib-float-standalone-zig.o src/stdlib_float.nelua
	$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-fno-builtin --cflags=-ffreestanding --cflags=-fno-stack-protector -o $(OUT)/sys-linux-x86_64-standalone-zig.o src/sys/linux_x86_64.nelua
	$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-fno-builtin --cflags=-ffreestanding --cflags=-fno-stack-protector --cflags=-ftls-model=local-exec -o $(OUT)/errno-standalone-zig.o src/errno/standalone.nelua
	$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-fno-builtin --cflags=-ffreestanding --cflags=-fno-stack-protector --cflags=-ftls-model=local-exec -o $(OUT)/tls-fixture-standalone-zig.o tools/tls_fixture.nelua
	$(ZIG) cc -c -std=c11 -ffreestanding -fno-stack-protector -fno-builtin -fno-sanitize=all -ftls-model=local-exec tools/tls_alignment_fixture.c -o $(OUT)/tls-alignment-fixture-standalone-zig.o
	$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-fno-builtin --cflags=-ffreestanding --cflags=-fno-stack-protector -o $(OUT)/thread-standalone-zig.o src/thread.nelua
	$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-fno-builtin --cflags=-ffreestanding --cflags=-fno-stack-protector -o $(OUT)/alloc-standalone-zig.o src/alloc.nelua
	$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-fno-builtin --cflags=-ffreestanding --cflags=-fno-stack-protector -o $(OUT)/alloc-exports-standalone-zig.o src/alloc/standalone_exports.nelua
	$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-fno-builtin --cflags=-ffreestanding --cflags=-fno-stack-protector -o $(OUT)/unistd-standalone-zig.o src/unistd.nelua
	$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-fno-builtin --cflags=-ffreestanding --cflags=-fno-stack-protector -D NELIBC_TESTING=true -o $(OUT)/unistd-standalone-smoke-zig.o src/unistd.nelua
	$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-fno-builtin --cflags=-ffreestanding --cflags=-fno-stack-protector -o $(OUT)/stdio-standalone-zig.o src/stdio.nelua
	$(ZIG) cc -c -std=c11 -ffreestanding -fno-builtin -fno-sanitize=all src/stdio_varargs.c -o $(OUT)/stdio-varargs-standalone-zig.o
	$(ZIG) cc -c src/sys/linux_x86_64.S -o $(OUT)/sys-linux-x86_64-standalone-zig-asm.o
	$(ZIG) cc -c src/sys/start_linux_x86_64_standalone.S -o $(OUT)/start-linux-x86_64-standalone-zig.o
	$(ZIG) cc -DNELIBC_STANDALONE -nostdlib -static -no-pie -fno-pie -fno-stack-protector -ffreestanding -fno-builtin -fno-sanitize=all -Wl,-e,_start -Wl,--build-id=none -Iinclude -DNELIBC_TESTING tools/syscall_smoke.c src/stdio_startup_stub.c src/fcntl_adapter.c src/credential_adapter.c src/stdio_lock_standalone.c tools/syscall_interpose_smoke.c $(OUT)/string-locale-standalone-zig.o $(OUT)/string-standalone-zig.o $(OUT)/memory-standalone-zig.o $(OUT)/memory-x86_64-simd.o $(OUT)/ctype-standalone-zig.o $(OUT)/stdlib-standalone-zig.o $(OUT)/locale-standalone-zig.o $(OUT)/stdlib-exit-standalone-zig.o $(OUT)/stdlib-float-standalone-zig.o $(OUT)/sys-linux-x86_64-standalone-zig.o $(OUT)/sys-linux-x86_64-standalone-zig-asm.o $(OUT)/start-linux-x86_64-standalone-zig.o $(OUT)/errno-standalone-zig.o $(OUT)/tls-fixture-standalone-zig.o $(OUT)/tls-alignment-fixture-standalone-zig.o $(OUT)/thread-standalone-zig.o $(OUT)/alloc-standalone-zig.o $(OUT)/alloc-exports-standalone-zig.o $(OUT)/unistd-standalone-smoke-zig.o $(OUT)/stdio-standalone-zig.o $(OUT)/stdio-varargs-standalone-zig.o src/environment_adapter.c -o $(OUT)/standalone-zig
	@test -z "$$(nm -u $(OUT)/standalone-zig)"

smoke-standalone-zig: standalone-zig
	NELIBC_GETENV_PROBE=visible $(OUT)/standalone-zig

standalone-gcc: $(OUT) $(OUT)/memory-x86_64-simd.o
	$(NELUA) -B -r --cc gcc --cflags=-fno-builtin --cflags=-ffreestanding --cflags=-fno-stack-protector --cflags=-fno-tree-loop-distribute-patterns --cflags=-DNELIBC_STANDALONE -o $(OUT)/string-standalone-gcc.o src/string.nelua
	$(NELUA) -B -r --cc gcc --cflags=-fno-builtin --cflags=-ffreestanding --cflags=-fno-stack-protector --cflags=-fno-tree-loop-distribute-patterns -o $(OUT)/string-locale-standalone-gcc.o src/string_locale_standalone.nelua
	$(NELUA) -B -r --cc gcc --cflags=-fno-builtin --cflags=-ffreestanding --cflags=-fno-stack-protector --cflags=-fno-tree-loop-distribute-patterns -o $(OUT)/memory-standalone-gcc.o src/memory.nelua
	$(NELUA) -B -r --cc gcc --cflags=-fno-builtin --cflags=-ffreestanding --cflags=-fno-stack-protector --cflags=-fno-tree-loop-distribute-patterns -o $(OUT)/ctype-standalone-gcc.o src/ctype.nelua
	$(NELUA) -B -r --cc gcc --cflags=-fno-builtin --cflags=-ffreestanding --cflags=-fno-stack-protector --cflags=-fno-tree-loop-distribute-patterns --cflags=-ftls-model=local-exec -o $(OUT)/stdlib-standalone-gcc.o src/stdlib.nelua
	$(NELUA) -B -r --cc gcc --cflags=-fno-builtin --cflags=-ffreestanding --cflags=-fno-stack-protector --cflags=-fno-tree-loop-distribute-patterns -o $(OUT)/locale-standalone-gcc.o src/locale_standalone.nelua
	$(NELUA) -B -r --cc gcc --cflags=-fno-builtin --cflags=-ffreestanding --cflags=-fno-stack-protector --cflags=-fno-tree-loop-distribute-patterns -o $(OUT)/stdlib-exit-standalone-gcc.o src/stdlib_exit_standalone.nelua
	$(NELUA) -B -r --cc gcc --cflags=-fno-builtin --cflags=-ffreestanding --cflags=-fno-stack-protector --cflags=-fno-tree-loop-distribute-patterns -o $(OUT)/stdlib-float-standalone-gcc.o src/stdlib_float.nelua
	$(NELUA) -B -r --cc gcc --cflags=-fno-builtin --cflags=-ffreestanding --cflags=-fno-stack-protector --cflags=-fno-tree-loop-distribute-patterns -o $(OUT)/sys-linux-x86_64-standalone-gcc.o src/sys/linux_x86_64.nelua
	$(NELUA) -B -r --cc gcc --cflags=-fno-builtin --cflags=-ffreestanding --cflags=-fno-stack-protector --cflags=-ftls-model=local-exec -o $(OUT)/errno-standalone-gcc.o src/errno/standalone.nelua
	$(NELUA) -B -r --cc gcc --cflags=-fno-builtin --cflags=-ffreestanding --cflags=-fno-stack-protector --cflags=-ftls-model=local-exec -o $(OUT)/tls-fixture-standalone-gcc.o tools/tls_fixture.nelua
	gcc -c -std=c11 -ffreestanding -fno-stack-protector -fno-builtin -fno-sanitize=all -ftls-model=local-exec tools/tls_alignment_fixture.c -o $(OUT)/tls-alignment-fixture-standalone-gcc.o
	$(NELUA) -B -r --cc gcc --cflags=-fno-builtin --cflags=-ffreestanding --cflags=-fno-stack-protector -o $(OUT)/thread-standalone-gcc.o src/thread.nelua
	$(NELUA) -B -r --cc gcc --cflags=-fno-builtin --cflags=-ffreestanding --cflags=-fno-stack-protector --cflags=-fno-tree-loop-distribute-patterns -o $(OUT)/alloc-standalone-gcc.o src/alloc.nelua
	$(NELUA) -B -r --cc gcc --cflags=-fno-builtin --cflags=-ffreestanding --cflags=-fno-stack-protector --cflags=-fno-tree-loop-distribute-patterns -o $(OUT)/alloc-exports-standalone-gcc.o src/alloc/standalone_exports.nelua
	$(NELUA) -B -r --cc gcc --cflags=-fno-builtin --cflags=-ffreestanding --cflags=-fno-stack-protector --cflags=-fno-tree-loop-distribute-patterns -o $(OUT)/unistd-standalone-gcc.o src/unistd.nelua
	$(NELUA) -B -r --cc gcc --cflags=-fno-builtin --cflags=-ffreestanding --cflags=-fno-stack-protector --cflags=-fno-tree-loop-distribute-patterns -D NELIBC_TESTING=true -o $(OUT)/unistd-standalone-smoke-gcc.o src/unistd.nelua
	$(NELUA) -B -r --cc gcc --cflags=-fno-builtin --cflags=-ffreestanding --cflags=-fno-stack-protector --cflags=-fno-tree-loop-distribute-patterns -o $(OUT)/stdio-standalone-gcc.o src/stdio.nelua
	gcc -c -std=c11 -ffreestanding -fno-builtin -fno-sanitize=all src/stdio_varargs.c -o $(OUT)/stdio-varargs-standalone-gcc.o
	gcc -c src/sys/linux_x86_64.S -o $(OUT)/sys-linux-x86_64-standalone-gcc-asm.o
	gcc -c src/sys/start_linux_x86_64_standalone.S -o $(OUT)/start-linux-x86_64-standalone-gcc.o
	gcc -DNELIBC_STANDALONE -nostdlib -static -no-pie -fno-pie -fno-stack-protector -ffreestanding -fno-builtin -fno-sanitize=all -fno-tree-loop-distribute-patterns -Wl,-e,_start -Wl,--build-id=none -Iinclude -DNELIBC_TESTING tools/syscall_smoke.c src/stdio_startup_stub.c src/fcntl_adapter.c src/credential_adapter.c src/stdio_lock_standalone.c tools/syscall_interpose_smoke.c $(OUT)/string-locale-standalone-gcc.o $(OUT)/string-standalone-gcc.o $(OUT)/memory-standalone-gcc.o $(OUT)/memory-x86_64-simd.o $(OUT)/ctype-standalone-gcc.o $(OUT)/stdlib-standalone-gcc.o $(OUT)/locale-standalone-gcc.o $(OUT)/stdlib-exit-standalone-gcc.o $(OUT)/stdlib-float-standalone-gcc.o $(OUT)/sys-linux-x86_64-standalone-gcc.o $(OUT)/sys-linux-x86_64-standalone-gcc-asm.o $(OUT)/start-linux-x86_64-standalone-gcc.o $(OUT)/errno-standalone-gcc.o $(OUT)/tls-fixture-standalone-gcc.o $(OUT)/tls-alignment-fixture-standalone-gcc.o $(OUT)/thread-standalone-gcc.o $(OUT)/alloc-standalone-gcc.o $(OUT)/alloc-exports-standalone-gcc.o $(OUT)/unistd-standalone-smoke-gcc.o $(OUT)/stdio-standalone-gcc.o $(OUT)/stdio-varargs-standalone-gcc.o src/environment_adapter.c -o $(OUT)/standalone-gcc
	@test -z "$$(nm -u $(OUT)/standalone-gcc)"

smoke-standalone-gcc: standalone-gcc
	NELIBC_GETENV_PROBE=visible $(OUT)/standalone-gcc

smoke-standalone-tcc: standalone-tcc
	NELIBC_GETENV_PROBE=visible $(OUT)/standalone-tcc

standalone-tcc: $(OUT) $(OUT)/memory-tcc-simd.o
	$(NELUA) -B -r --cc '$(TCC)' --cflags=-fno-builtin --cflags=-ffreestanding --cflags=-fno-stack-protector --cflags=-DNELIBC_STANDALONE -o $(OUT)/string-standalone-tcc.o src/string.nelua
	$(NELUA) -B -r --cc '$(TCC)' --cflags=-fno-builtin --cflags=-ffreestanding --cflags=-fno-stack-protector -o $(OUT)/string-locale-standalone-tcc.o src/string_locale_standalone.nelua
	$(NELUA) -B -r --cc '$(TCC)' --cflags=-fno-builtin --cflags=-ffreestanding --cflags=-fno-stack-protector -o $(OUT)/memory-standalone-tcc.o src/memory.nelua
	$(TCC) -c -ffreestanding -fno-builtin src/memory_tcc_x86_64.c -o $(OUT)/memory-tcc-standalone-helper.o
	$(NELUA) -B -r --cc '$(TCC)' --cflags=-fno-builtin --cflags=-ffreestanding --cflags=-fno-stack-protector -o $(OUT)/ctype-standalone-tcc.o src/ctype.nelua
	$(NELUA) -B -r --cc '$(TCC)' --cflags=-fno-builtin --cflags=-ffreestanding --cflags=-fno-stack-protector --cflags=-ftls-model=local-exec -o $(OUT)/stdlib-standalone-tcc.o src/stdlib.nelua
	$(NELUA) -B -r --cc '$(TCC)' --cflags=-fno-builtin --cflags=-ffreestanding --cflags=-fno-stack-protector -o $(OUT)/locale-standalone-tcc.o src/locale_standalone.nelua
	$(NELUA) -B -r --cc '$(TCC)' --cflags=-fno-builtin --cflags=-ffreestanding --cflags=-fno-stack-protector -o $(OUT)/stdlib-exit-standalone-tcc.o src/stdlib_exit_standalone.nelua
	$(NELUA) -B -r --cc '$(TCC)' --cflags=-fno-builtin --cflags=-ffreestanding --cflags=-fno-stack-protector -o $(OUT)/stdlib-float-standalone-tcc.o src/stdlib_float.nelua
	$(NELUA) -B -r --cc '$(TCC)' --cflags=-fno-builtin --cflags=-ffreestanding --cflags=-fno-stack-protector -o $(OUT)/sys-linux-x86_64-standalone-tcc.o src/sys/linux_x86_64.nelua
	# TinyCC emits ordinary RIP-relative references for Nelua TLS globals; use Zig cc for these TLS-bearing objects.
	$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-fno-builtin --cflags=-ffreestanding --cflags=-fno-stack-protector --cflags=-ftls-model=local-exec -o $(OUT)/errno-standalone-tcc.o src/errno/standalone.nelua
	$(NELUA) -B -r --cc '$(ZIG) cc' --cflags=-fno-builtin --cflags=-ffreestanding --cflags=-fno-stack-protector --cflags=-ftls-model=local-exec -o $(OUT)/tls-fixture-standalone-tcc.o tools/tls_fixture.nelua
	# TinyCC does not parse C TLS declarations; use Zig cc for the TLS alignment fixture.
	$(ZIG) cc -c -std=c11 -ffreestanding -fno-stack-protector -fno-builtin -fno-sanitize=all -ftls-model=local-exec tools/tls_alignment_fixture.c -o $(OUT)/tls-alignment-fixture-standalone-tcc.o
	$(NELUA) -B -r --cc '$(TCC)' --cflags=-fno-builtin --cflags=-ffreestanding --cflags=-fno-stack-protector -o $(OUT)/thread-standalone-tcc.o src/thread.nelua
	$(NELUA) -B -r --cc '$(TCC)' --cflags=-fno-builtin --cflags=-ffreestanding --cflags=-fno-stack-protector -o $(OUT)/alloc-standalone-tcc.o src/alloc.nelua
	$(NELUA) -B -r --cc '$(TCC)' --cflags=-fno-builtin --cflags=-ffreestanding --cflags=-fno-stack-protector -o $(OUT)/alloc-exports-standalone-tcc.o src/alloc/standalone_exports.nelua
	$(NELUA) -B -r --cc '$(TCC)' --cflags=-fno-builtin --cflags=-ffreestanding --cflags=-fno-stack-protector -o $(OUT)/unistd-standalone-tcc.o src/unistd.nelua
	$(NELUA) -B -r --cc '$(TCC)' --cflags=-fno-builtin --cflags=-ffreestanding --cflags=-fno-stack-protector -o $(OUT)/stdio-standalone-tcc.o src/stdio.nelua
	$(TCC) -c -std=c11 -ffreestanding -fno-builtin -fno-sanitize=all src/stdio_varargs.c -o $(OUT)/stdio-varargs-standalone-tcc.o
	$(ZIG) cc -c src/sys/linux_x86_64.S -o $(OUT)/sys-linux-x86_64-standalone-tcc-asm.o
	$(ZIG) cc -c src/sys/start_linux_x86_64_standalone.S -o $(OUT)/start-linux-x86_64-standalone-tcc.o
	$(TCC) -DNELIBC_STANDALONE -ffreestanding -fno-builtin -fno-stack-protector -fno-sanitize=all -c src/fcntl_adapter.c -o $(OUT)/fcntl-adapter-standalone-tcc.o
	$(ZIG) cc -DNELIBC_STANDALONE -ffreestanding -fno-builtin -fno-stack-protector -fno-sanitize=all -c src/credential_adapter.c -o $(OUT)/credential-adapter-standalone-tcc.o
	$(ZIG) cc -DNELIBC_STANDALONE -ffreestanding -fno-builtin -fno-stack-protector -fno-sanitize=all -c src/stdio_lock_standalone.c -o $(OUT)/stdio-lock-standalone-tcc.o
	$(TCC) -DNELIBC_STANDALONE -ffreestanding -fno-builtin -fno-stack-protector -fno-sanitize=all -c src/stdio_startup_stub.c -o $(OUT)/stdio-startup-stub-standalone-tcc.o
	$(ZIG) cc -DNELIBC_STANDALONE -ffreestanding -fno-builtin -fno-stack-protector -fno-sanitize=all -c src/environment_adapter.c -o $(OUT)/environment-adapter-standalone-tcc.o
	$(ZIG) cc -DNELIBC_STANDALONE -Iinclude -ffreestanding -fno-builtin -fno-stack-protector -fno-sanitize=all -c tools/syscall_smoke.c -o $(OUT)/syscall-smoke-standalone-tcc.o
	$(TCC) -DNELIBC_STANDALONE -Iinclude -ffreestanding -fno-builtin -fno-stack-protector -fno-sanitize=all -c tools/syscall_interpose_smoke.c -o $(OUT)/syscall-interpose-smoke-standalone-tcc.o
	$(ZIG) cc -DNELIBC_STANDALONE -nostdlib -static -no-pie -fno-pie -fno-stack-protector -ffreestanding -fno-builtin -fno-sanitize=all -Wl,-e,_start -Wl,--build-id=none -Iinclude $(OUT)/syscall-smoke-standalone-tcc.o $(OUT)/stdio-startup-stub-standalone-tcc.o $(OUT)/fcntl-adapter-standalone-tcc.o $(OUT)/credential-adapter-standalone-tcc.o $(OUT)/stdio-lock-standalone-tcc.o $(OUT)/syscall-interpose-smoke-standalone-tcc.o $(OUT)/string-locale-standalone-tcc.o $(OUT)/string-standalone-tcc.o $(OUT)/memory-standalone-tcc.o $(OUT)/memory-tcc-standalone-helper.o $(OUT)/memory-tcc-simd.o $(OUT)/ctype-standalone-tcc.o $(OUT)/stdlib-standalone-tcc.o $(OUT)/locale-standalone-tcc.o $(OUT)/stdlib-exit-standalone-tcc.o $(OUT)/stdlib-float-standalone-tcc.o $(OUT)/sys-linux-x86_64-standalone-tcc.o $(OUT)/sys-linux-x86_64-standalone-tcc-asm.o $(OUT)/start-linux-x86_64-standalone-tcc.o $(OUT)/errno-standalone-tcc.o $(OUT)/tls-fixture-standalone-tcc.o $(OUT)/tls-alignment-fixture-standalone-tcc.o $(OUT)/thread-standalone-tcc.o $(OUT)/alloc-standalone-tcc.o $(OUT)/alloc-exports-standalone-tcc.o $(OUT)/unistd-standalone-tcc.o $(OUT)/stdio-standalone-tcc.o $(OUT)/stdio-varargs-standalone-tcc.o $(OUT)/environment-adapter-standalone-tcc.o $(TCC_LIBTCC1) -o $(OUT)/standalone-tcc
	@test -z "$$(nm -u $(OUT)/standalone-tcc)"

exit-smoke-standalone-zig: $(OUT)/libnelibc-standalone-zig.a tools/exit_smoke.c
	$(ZIG) cc -DNELIBC_STANDALONE -nostdlib -static -no-pie -fno-pie -fno-stack-protector -ffreestanding -fno-builtin -fno-sanitize=all -Wl,-e,_start -Wl,--build-id=none -Wl,--whole-archive $(OUT)/libnelibc-standalone-zig.a -Wl,--no-whole-archive -Iinclude tools/exit_smoke.c -o $(OUT)/exit-smoke-standalone-zig
	@test -z "$$(nm -u $(OUT)/exit-smoke-standalone-zig)"
	@$(OUT)/exit-smoke-standalone-zig > $(OUT)/exit-smoke-standalone-zig.out
	@test "$$(cat $(OUT)/exit-smoke-standalone-zig.out)" = CBAF
	@$(OUT)/exit-smoke-standalone-zig --return > $(OUT)/exit-smoke-standalone-zig.out
	@test "$$(cat $(OUT)/exit-smoke-standalone-zig.out)" = CBAF
	@status=0; $(OUT)/exit-smoke-standalone-zig --_Exit > $(OUT)/exit-smoke-standalone-zig.out || status=$$?; test $$status -eq 42 && test ! -s $(OUT)/exit-smoke-standalone-zig.out
	@$(OUT)/exit-smoke-standalone-zig --capacity > $(OUT)/exit-smoke-standalone-zig.out
	@test ! -s $(OUT)/exit-smoke-standalone-zig.out
	@$(OUT)/exit-smoke-standalone-zig --abort

exit-smoke-standalone-gcc: $(OUT)/libnelibc-standalone-gcc.a tools/exit_smoke.c
	gcc -DNELIBC_STANDALONE -nostdlib -static -no-pie -fno-pie -fno-stack-protector -ffreestanding -fno-builtin -fno-sanitize=all -fno-tree-loop-distribute-patterns -Wl,-e,_start -Wl,--build-id=none -Wl,--whole-archive $(OUT)/libnelibc-standalone-gcc.a -Wl,--no-whole-archive -Iinclude tools/exit_smoke.c -o $(OUT)/exit-smoke-standalone-gcc
	@test -z "$$(nm -u $(OUT)/exit-smoke-standalone-gcc)"
	@$(OUT)/exit-smoke-standalone-gcc > $(OUT)/exit-smoke-standalone-gcc.out
	@test "$$(cat $(OUT)/exit-smoke-standalone-gcc.out)" = CBAF
	@$(OUT)/exit-smoke-standalone-gcc --return > $(OUT)/exit-smoke-standalone-gcc.out
	@test "$$(cat $(OUT)/exit-smoke-standalone-gcc.out)" = CBAF
	@status=0; $(OUT)/exit-smoke-standalone-gcc --_Exit > $(OUT)/exit-smoke-standalone-gcc.out || status=$$?; test $$status -eq 42 && test ! -s $(OUT)/exit-smoke-standalone-gcc.out
	@$(OUT)/exit-smoke-standalone-gcc --capacity > $(OUT)/exit-smoke-standalone-gcc.out
	@test ! -s $(OUT)/exit-smoke-standalone-gcc.out
	@$(OUT)/exit-smoke-standalone-gcc --abort

exit-smoke-standalone-matrix: exit-smoke-standalone-zig exit-smoke-standalone-gcc

abort-smoke-zig: $(OUT)/libnelibc-zig.a tools/abort_smoke.c
	$(ZIG) cc -pthread -Iinclude -fno-builtin tools/abort_smoke.c $(OUT)/libnelibc-zig.a -o $(OUT)/abort-smoke-zig
	$(OUT)/abort-smoke-zig

abort-smoke-gcc: $(OUT)/libnelibc-gcc.a tools/abort_smoke.c
	gcc -pthread -Iinclude -fno-builtin tools/abort_smoke.c $(OUT)/libnelibc-gcc.a -o $(OUT)/abort-smoke-gcc
	$(OUT)/abort-smoke-gcc

abort-smoke-zig-musl: $(OUT)/libnelibc-zig-musl.a tools/abort_smoke.c
	$(ZIG) cc -target x86_64-linux-musl -static -pthread -Iinclude -fno-builtin $(MUSL_STDIO_WRAP_FLAGS) tools/abort_smoke.c $(OUT)/libnelibc-zig-musl.a -o $(OUT)/abort-smoke-zig-musl
	$(OUT)/abort-smoke-zig-musl

abort-smoke-tcc: $(OUT)/libnelibc-tcc.a tools/abort_smoke.c
	$(TCC) -pthread -Iinclude -fno-builtin -c tools/abort_smoke.c -o $(OUT)/abort-smoke-tcc.o
	$(ZIG) cc -fsanitize=undefined -pthread -Iinclude -fno-builtin $(OUT)/abort-smoke-tcc.o $(OUT)/libnelibc-tcc.a $(TCC_LIBTCC1) -o $(OUT)/abort-smoke-tcc
	$(OUT)/abort-smoke-tcc

abort-smoke-standalone-zig: $(OUT)/libnelibc-standalone-zig.a tools/abort_smoke.c
	$(ZIG) cc -DNELIBC_STANDALONE -nostdlib -static -no-pie -fno-pie -fno-stack-protector -ffreestanding -fno-builtin -fno-sanitize=all -Wl,-e,_start -Wl,--build-id=none -Wl,--whole-archive $(OUT)/libnelibc-standalone-zig.a -Wl,--no-whole-archive -Iinclude tools/abort_smoke.c -o $(OUT)/abort-smoke-standalone-zig
	@test -z "$$($(NM) -u $(OUT)/abort-smoke-standalone-zig)"
	@ulimit -c 0; $(OUT)/abort-smoke-standalone-zig

abort-smoke-standalone-gcc: $(OUT)/libnelibc-standalone-gcc.a tools/abort_smoke.c
	gcc -DNELIBC_STANDALONE -nostdlib -static -no-pie -fno-pie -fno-stack-protector -ffreestanding -fno-builtin -fno-sanitize=all -fno-tree-loop-distribute-patterns -Wl,-e,_start -Wl,--build-id=none -Wl,--whole-archive $(OUT)/libnelibc-standalone-gcc.a -Wl,--no-whole-archive -Iinclude tools/abort_smoke.c -o $(OUT)/abort-smoke-standalone-gcc
	@test -z "$$($(NM) -u $(OUT)/abort-smoke-standalone-gcc)"
	@ulimit -c 0; $(OUT)/abort-smoke-standalone-gcc

abort-smoke-matrix: abort-smoke-zig abort-smoke-gcc abort-smoke-zig-musl abort-smoke-tcc abort-smoke-standalone-zig abort-smoke-standalone-gcc

$(OUT)/libnelibc-standalone-zig.a: standalone-zig src/fcntl_adapter.c src/credential_adapter.c src/stdio_lock_standalone.c src/stdio_startup_stub.c src/environment_adapter.c src/semaphore_adapter.c
	$(ZIG) cc -DNELIBC_STANDALONE -ffreestanding -fno-builtin -fno-stack-protector -fno-sanitize=all -c src/fcntl_adapter.c -o $(OUT)/fcntl-adapter-standalone-zig.o
	$(ZIG) cc -DNELIBC_STANDALONE -ffreestanding -fno-builtin -fno-stack-protector -fno-sanitize=all -c src/credential_adapter.c -o $(OUT)/credential-adapter-standalone-zig.o
	$(ZIG) cc -DNELIBC_STANDALONE -ffreestanding -fno-builtin -fno-stack-protector -fno-sanitize=all -c src/stdio_lock_standalone.c -o $(OUT)/stdio-lock-standalone-zig.o
	$(ZIG) cc -DNELIBC_STANDALONE -ffreestanding -fno-builtin -fno-stack-protector -fno-sanitize=all -c src/stdio_startup_stub.c -o $(OUT)/stdio-startup-stub-standalone-zig.o
	$(ZIG) cc -DNELIBC_STANDALONE -ffreestanding -fno-builtin -fno-stack-protector -fno-sanitize=all -c src/environment_adapter.c -o $(OUT)/environment-adapter-standalone-zig.o
	$(ZIG) cc -DNELIBC_STANDALONE -ffreestanding -fno-builtin -fno-stack-protector -fno-sanitize=all -Iinclude -c src/semaphore_adapter.c -o $(OUT)/semaphore-adapter-standalone-zig.o
	rm -f $@
	$(AR) rcs $@ $(OUT)/string-locale-standalone-zig.o $(OUT)/string-standalone-zig.o $(OUT)/memory-standalone-zig.o $(OUT)/memory-x86_64-simd.o $(OUT)/ctype-standalone-zig.o $(OUT)/stdlib-standalone-zig.o $(OUT)/locale-standalone-zig.o $(OUT)/stdlib-exit-standalone-zig.o $(OUT)/stdlib-float-standalone-zig.o $(OUT)/sys-linux-x86_64-standalone-zig.o $(OUT)/sys-linux-x86_64-standalone-zig-asm.o $(OUT)/start-linux-x86_64-standalone-zig.o $(OUT)/errno-standalone-zig.o $(OUT)/thread-standalone-zig.o $(OUT)/alloc-standalone-zig.o $(OUT)/alloc-exports-standalone-zig.o $(OUT)/unistd-standalone-zig.o $(OUT)/stdio-standalone-zig.o $(OUT)/stdio-varargs-standalone-zig.o $(OUT)/fcntl-adapter-standalone-zig.o $(OUT)/credential-adapter-standalone-zig.o $(OUT)/stdio-lock-standalone-zig.o $(OUT)/stdio-startup-stub-standalone-zig.o $(OUT)/environment-adapter-standalone-zig.o $(OUT)/semaphore-adapter-standalone-zig.o
	$(AR) rcs $@ $(OUT)/math-core-zig.o
	$(AR) rcs $@ $(OUT)/math-core-x86_64.o

$(OUT)/libnelibc-standalone-gcc.a: standalone-gcc src/fcntl_adapter.c src/credential_adapter.c src/stdio_lock_standalone.c src/stdio_startup_stub.c src/environment_adapter.c src/semaphore_adapter.c
	gcc -DNELIBC_STANDALONE -ffreestanding -fno-builtin -fno-stack-protector -fno-sanitize=all -fno-tree-loop-distribute-patterns -c src/fcntl_adapter.c -o $(OUT)/fcntl-adapter-standalone-gcc.o
	gcc -DNELIBC_STANDALONE -ffreestanding -fno-builtin -fno-stack-protector -fno-sanitize=all -fno-tree-loop-distribute-patterns -c src/credential_adapter.c -o $(OUT)/credential-adapter-standalone-gcc.o
	gcc -DNELIBC_STANDALONE -ffreestanding -fno-builtin -fno-stack-protector -fno-sanitize=all -fno-tree-loop-distribute-patterns -c src/stdio_lock_standalone.c -o $(OUT)/stdio-lock-standalone-gcc.o
	gcc -DNELIBC_STANDALONE -ffreestanding -fno-builtin -fno-stack-protector -fno-sanitize=all -fno-tree-loop-distribute-patterns -c src/stdio_startup_stub.c -o $(OUT)/stdio-startup-stub-standalone-gcc.o
	gcc -DNELIBC_STANDALONE -ffreestanding -fno-builtin -fno-stack-protector -fno-sanitize=all -fno-tree-loop-distribute-patterns -c src/environment_adapter.c -o $(OUT)/environment-adapter-standalone-gcc.o
	gcc -DNELIBC_STANDALONE -ffreestanding -fno-builtin -fno-stack-protector -fno-sanitize=all -fno-tree-loop-distribute-patterns -Iinclude -c src/semaphore_adapter.c -o $(OUT)/semaphore-adapter-standalone-gcc.o
	rm -f $@
	$(AR) rcs $@ $(OUT)/string-locale-standalone-gcc.o $(OUT)/string-standalone-gcc.o $(OUT)/memory-standalone-gcc.o $(OUT)/memory-x86_64-simd.o $(OUT)/ctype-standalone-gcc.o $(OUT)/stdlib-standalone-gcc.o $(OUT)/locale-standalone-gcc.o $(OUT)/stdlib-exit-standalone-gcc.o $(OUT)/stdlib-float-standalone-gcc.o $(OUT)/sys-linux-x86_64-standalone-gcc.o $(OUT)/sys-linux-x86_64-standalone-gcc-asm.o $(OUT)/start-linux-x86_64-standalone-gcc.o $(OUT)/errno-standalone-gcc.o $(OUT)/thread-standalone-gcc.o $(OUT)/alloc-standalone-gcc.o $(OUT)/alloc-exports-standalone-gcc.o $(OUT)/unistd-standalone-gcc.o $(OUT)/stdio-standalone-gcc.o $(OUT)/stdio-varargs-standalone-gcc.o $(OUT)/fcntl-adapter-standalone-gcc.o $(OUT)/credential-adapter-standalone-gcc.o $(OUT)/stdio-lock-standalone-gcc.o $(OUT)/stdio-startup-stub-standalone-gcc.o $(OUT)/environment-adapter-standalone-gcc.o $(OUT)/semaphore-adapter-standalone-gcc.o
	$(AR) rcs $@ $(OUT)/math-core-gcc.o
	$(AR) rcs $@ $(OUT)/math-core-x86_64.o

$(OUT)/tcc-runtime-standalone: $(TCC_LIBTCC1)
	mkdir -p $@
	cd $@ && $(AR) x $(abspath $(TCC_LIBTCC1)) libtcc1.o va_list.o

$(OUT)/libnelibc-standalone-tcc.a: standalone-tcc $(OUT)/tcc-runtime-standalone src/semaphore_adapter.c
	rm -f $@
	$(TCC) -DNELIBC_STANDALONE -ffreestanding -fno-builtin -fno-stack-protector -fno-sanitize=all -Iinclude -c src/semaphore_adapter.c -o $(OUT)/semaphore-adapter-standalone-tcc.o
	$(AR) rcs $@ $(OUT)/string-locale-standalone-tcc.o $(OUT)/string-standalone-tcc.o $(OUT)/memory-standalone-tcc.o $(OUT)/memory-tcc-standalone-helper.o $(OUT)/memory-tcc-simd.o $(OUT)/ctype-standalone-tcc.o $(OUT)/stdlib-standalone-tcc.o $(OUT)/locale-standalone-tcc.o $(OUT)/stdlib-exit-standalone-tcc.o $(OUT)/stdlib-float-standalone-tcc.o $(OUT)/sys-linux-x86_64-standalone-tcc.o $(OUT)/sys-linux-x86_64-standalone-tcc-asm.o $(OUT)/start-linux-x86_64-standalone-tcc.o $(OUT)/errno-standalone-tcc.o $(OUT)/thread-standalone-tcc.o $(OUT)/alloc-standalone-tcc.o $(OUT)/alloc-exports-standalone-tcc.o $(OUT)/unistd-standalone-tcc.o $(OUT)/stdio-standalone-tcc.o $(OUT)/stdio-varargs-standalone-tcc.o $(OUT)/fcntl-adapter-standalone-tcc.o $(OUT)/credential-adapter-standalone-tcc.o $(OUT)/stdio-lock-standalone-tcc.o $(OUT)/stdio-startup-stub-standalone-tcc.o $(OUT)/environment-adapter-standalone-tcc.o $(OUT)/semaphore-adapter-standalone-tcc.o $(OUT)/tcc-runtime-standalone/libtcc1.o $(OUT)/tcc-runtime-standalone/va_list.o
	$(AR) rcs $@ $(OUT)/math-core-tcc.o
	$(AR) rcs $@ $(OUT)/math-core-x86_64.o

lib-standalone-zig: $(OUT)/libnelibc-standalone-zig.a
lib-standalone-gcc: $(OUT)/libnelibc-standalone-gcc.a
lib-standalone-tcc: $(OUT)/libnelibc-standalone-tcc.a

smoke-lib-standalone-zig: $(OUT)/libnelibc-standalone-zig.a
	$(ZIG) cc -DNELIBC_STANDALONE -nostdlib -static -no-pie -fno-pie -fno-stack-protector -ffreestanding -fno-builtin -fno-sanitize=all -Wl,-e,_start -Wl,--build-id=none -Wl,--whole-archive $(OUT)/libnelibc-standalone-zig.a -Wl,--no-whole-archive -Iinclude tools/syscall_smoke.c tools/syscall_interpose_smoke.c $(OUT)/tls-fixture-standalone-zig.o $(OUT)/tls-alignment-fixture-standalone-zig.o -o $(OUT)/smoke-lib-standalone-zig
	@test -z "$$(nm -u $(OUT)/smoke-lib-standalone-zig)"
	NELIBC_GETENV_PROBE=visible $(OUT)/smoke-lib-standalone-zig

smoke-lib-standalone-gcc: $(OUT)/libnelibc-standalone-gcc.a
	gcc -DNELIBC_STANDALONE -nostdlib -static -no-pie -fno-pie -fno-stack-protector -ffreestanding -fno-builtin -fno-sanitize=all -fno-tree-loop-distribute-patterns -Wl,-e,_start -Wl,--build-id=none -Wl,--whole-archive $(OUT)/libnelibc-standalone-gcc.a -Wl,--no-whole-archive -Iinclude tools/syscall_smoke.c tools/syscall_interpose_smoke.c $(OUT)/tls-fixture-standalone-gcc.o $(OUT)/tls-alignment-fixture-standalone-gcc.o -o $(OUT)/smoke-lib-standalone-gcc
	@test -z "$$(nm -u $(OUT)/smoke-lib-standalone-gcc)"
	NELIBC_GETENV_PROBE=visible $(OUT)/smoke-lib-standalone-gcc

lib-consumer-standalone-zig: $(OUT)/libnelibc-standalone-zig.a
	$(ZIG) cc -DNELIBC_STANDALONE -nostdlib -static -no-pie -fno-pie -fno-stack-protector -ffreestanding -fno-builtin -fno-sanitize=all -Wl,-e,_start -Wl,--build-id=none -Wl,--whole-archive $(OUT)/libnelibc-standalone-zig.a -Wl,--no-whole-archive -Iinclude tools/lib_consumer_smoke.c -o $(OUT)/lib-consumer-standalone-zig
	@test -z "$$(nm -u $(OUT)/lib-consumer-standalone-zig)"
	$(OUT)/lib-consumer-standalone-zig

lib-consumer-standalone-gcc: $(OUT)/libnelibc-standalone-gcc.a
	gcc -DNELIBC_STANDALONE -nostdlib -static -no-pie -fno-pie -fno-stack-protector -ffreestanding -fno-builtin -fno-sanitize=all -fno-tree-loop-distribute-patterns -Wl,-e,_start -Wl,--build-id=none -Wl,--whole-archive $(OUT)/libnelibc-standalone-gcc.a -Wl,--no-whole-archive -Iinclude tools/lib_consumer_smoke.c -o $(OUT)/lib-consumer-standalone-gcc
	@test -z "$$(nm -u $(OUT)/lib-consumer-standalone-gcc)"
	$(OUT)/lib-consumer-standalone-gcc

lib-consumer-standalone-tcc: $(OUT)/libnelibc-standalone-tcc.a
	$(ZIG) cc -DNELIBC_STANDALONE -nostdlib -static -no-pie -fno-pie -fno-stack-protector -ffreestanding -fno-builtin -fno-sanitize=all -Wl,-e,_start -Wl,--build-id=none -Wl,--whole-archive $(OUT)/libnelibc-standalone-tcc.a -Wl,--no-whole-archive -Iinclude tools/lib_consumer_smoke.c -o $(OUT)/lib-consumer-standalone-tcc
	@test -z "$$(nm -u $(OUT)/lib-consumer-standalone-tcc)"
	$(OUT)/lib-consumer-standalone-tcc

lib-consumer-link-standalone-tcc: lib-consumer-standalone-tcc

pthread-standalone-zig: $(OUT)/libnelibc-standalone-zig.a tools/pthread_standalone_smoke.c
	$(ZIG) cc -DNELIBC_STANDALONE -nostdlib -static -no-pie -fno-pie -fno-stack-protector -ffreestanding -fno-builtin -fno-sanitize=all -Wl,-e,_start -Wl,--build-id=none -Wl,--whole-archive $(OUT)/libnelibc-standalone-zig.a -Wl,--no-whole-archive -Iinclude tools/pthread_standalone_smoke.c -o $(OUT)/pthread-smoke-standalone-zig
	@test -z "$$(nm -u $(OUT)/pthread-smoke-standalone-zig)"
	$(OUT)/pthread-smoke-standalone-zig

pthread-standalone-gcc: $(OUT)/libnelibc-standalone-gcc.a tools/pthread_standalone_smoke.c
	gcc -DNELIBC_STANDALONE -nostdlib -static -no-pie -fno-pie -fno-stack-protector -ffreestanding -fno-builtin -fno-sanitize=all -fno-tree-loop-distribute-patterns -Wl,-e,_start -Wl,--build-id=none -Wl,--whole-archive $(OUT)/libnelibc-standalone-gcc.a -Wl,--no-whole-archive -Iinclude tools/pthread_standalone_smoke.c -o $(OUT)/pthread-smoke-standalone-gcc
	@test -z "$$(nm -u $(OUT)/pthread-smoke-standalone-gcc)"
	$(OUT)/pthread-smoke-standalone-gcc

pthread-standalone-tcc: $(OUT)/libnelibc-standalone-tcc.a tools/pthread_standalone_smoke.c
	$(ZIG) cc -DNELIBC_STANDALONE -nostdlib -static -no-pie -fno-pie -fno-stack-protector -ffreestanding -fno-builtin -fno-sanitize=all -Wl,-e,_start -Wl,--build-id=none -Wl,--whole-archive $(OUT)/libnelibc-standalone-tcc.a -Wl,--no-whole-archive -Iinclude tools/pthread_standalone_smoke.c -o $(OUT)/pthread-smoke-standalone-tcc
	@test -z "$$(nm -u $(OUT)/pthread-smoke-standalone-tcc)"
	$(OUT)/pthread-smoke-standalone-tcc

pthread-standalone-matrix: pthread-standalone-zig pthread-standalone-gcc pthread-standalone-tcc
.PHONY: pthread-standalone-zig pthread-standalone-gcc pthread-standalone-tcc pthread-standalone-matrix

sem-clockwait-musl-smoke: $(OUT)/libnelibc-zig-musl.a tools/sem_clockwait_musl_smoke.c
	$(ZIG) cc -target x86_64-linux-musl -static -pthread -Iinclude -fno-builtin $(MUSL_STDIO_WRAP_FLAGS) tools/sem_clockwait_musl_smoke.c $(OUT)/libnelibc-zig-musl.a -o $(OUT)/sem-clockwait-musl-smoke
	$(OUT)/sem-clockwait-musl-smoke
.PHONY: sem-clockwait-musl-smoke

sem-clockwait-shootout-zig: $(OUT)/libnelibc-zig.a tools/sem_clockwait_shootout.c
	$(ZIG) cc -pthread -ldl -Iinclude -fno-builtin tools/sem_clockwait_shootout.c $(OUT)/libnelibc-zig.a -o $(OUT)/sem-clockwait-shootout-zig
	$(OUT)/sem-clockwait-shootout-zig | tee $(OUT)/sem-clockwait-shootout-zig.csv
.PHONY: sem-clockwait-shootout-zig

rand-r-diff-standalone-zig: $(OUT)/libnelibc-standalone-zig.a
	$(ZIG) cc -DNELIBC_STANDALONE -nostdlib -static -no-pie -fno-pie -fno-stack-protector -ffreestanding -fno-builtin -fno-sanitize=all -Wl,-e,_start -Wl,--build-id=none -Wl,--whole-archive $(OUT)/libnelibc-standalone-zig.a -Wl,--no-whole-archive -Iinclude tools/rand_r_diff.c -o $(OUT)/rand-r-diff-standalone-zig
	@test -z "$$(nm -u $(OUT)/rand-r-diff-standalone-zig)"
	$(OUT)/rand-r-diff-standalone-zig

rand-r-diff-standalone-gcc: $(OUT)/libnelibc-standalone-gcc.a
	gcc -DNELIBC_STANDALONE -nostdlib -static -no-pie -fno-pie -fno-stack-protector -ffreestanding -fno-builtin -fno-sanitize=all -fno-tree-loop-distribute-patterns -Wl,-e,_start -Wl,--build-id=none -Wl,--whole-archive $(OUT)/libnelibc-standalone-gcc.a -Wl,--no-whole-archive -Iinclude tools/rand_r_diff.c -o $(OUT)/rand-r-diff-standalone-gcc
	@test -z "$$(nm -u $(OUT)/rand-r-diff-standalone-gcc)"
	$(OUT)/rand-r-diff-standalone-gcc

rand-r-diff-standalone-matrix: rand-r-diff-standalone-zig rand-r-diff-standalone-gcc

lib-consumer-standalone-matrix: lib-consumer-standalone-zig lib-consumer-standalone-gcc lib-consumer-standalone-tcc rand-r-diff-standalone-matrix pthread-standalone-matrix

smoke-lib-standalone-matrix: smoke-lib-standalone-zig smoke-lib-standalone-gcc

locale-format-diff-zig: $(OUT)/libnelibc-standalone-zig.a
	$(ZIG) cc -Iinclude -fno-builtin tools/locale_format_diff.c -o $(OUT)/locale-format-host-zig
	$(ZIG) cc -DNELIBC_STANDALONE -nostdlib -static -no-pie -fno-pie -fno-stack-protector -ffreestanding -fno-builtin -fno-sanitize=all -Wl,-e,_start -Wl,--build-id=none -Wl,--whole-archive $(OUT)/libnelibc-standalone-zig.a -Wl,--no-whole-archive -Iinclude tools/locale_format_diff.c -o $(OUT)/locale-format-nelibc-zig
	$(OUT)/locale-format-host-zig > $(OUT)/locale-format-host-zig.txt
	$(OUT)/locale-format-nelibc-zig > $(OUT)/locale-format-nelibc-zig.txt
	diff -u $(OUT)/locale-format-host-zig.txt $(OUT)/locale-format-nelibc-zig.txt

locale-format-diff-gcc: $(OUT)/libnelibc-standalone-gcc.a
	gcc -Iinclude -fno-builtin tools/locale_format_diff.c -o $(OUT)/locale-format-host-gcc
	gcc -DNELIBC_STANDALONE -nostdlib -static -no-pie -fno-pie -fno-stack-protector -ffreestanding -fno-builtin -fno-sanitize=all -fno-tree-loop-distribute-patterns -Wl,-e,_start -Wl,--build-id=none -Wl,--whole-archive $(OUT)/libnelibc-standalone-gcc.a -Wl,--no-whole-archive -Iinclude tools/locale_format_diff.c -o $(OUT)/locale-format-nelibc-gcc
	$(OUT)/locale-format-host-gcc > $(OUT)/locale-format-host-gcc.txt
	$(OUT)/locale-format-nelibc-gcc > $(OUT)/locale-format-nelibc-gcc.txt
	diff -u $(OUT)/locale-format-host-gcc.txt $(OUT)/locale-format-nelibc-gcc.txt

locale-format-diff-tcc: $(OUT)/libnelibc-standalone-tcc.a
	$(TCC) -Iinclude -fno-builtin tools/locale_format_diff.c -o $(OUT)/locale-format-host-tcc
	$(ZIG) cc -DNELIBC_STANDALONE -nostdlib -static -no-pie -fno-pie -fno-stack-protector -ffreestanding -fno-builtin -fno-sanitize=all -Wl,-e,_start -Wl,--build-id=none -Wl,--whole-archive $(OUT)/libnelibc-standalone-tcc.a -Wl,--no-whole-archive -Iinclude tools/locale_format_diff.c -o $(OUT)/locale-format-nelibc-tcc
	$(OUT)/locale-format-host-tcc > $(OUT)/locale-format-host-tcc.txt
	$(OUT)/locale-format-nelibc-tcc > $(OUT)/locale-format-nelibc-tcc.txt
	diff -u $(OUT)/locale-format-host-tcc.txt $(OUT)/locale-format-nelibc-tcc.txt

locale-format-diff-matrix: locale-format-diff-zig locale-format-diff-gcc locale-format-diff-tcc

locale-format-hybrid-diff-zig: $(OUT)/libnelibc-zig.a
	$(ZIG) cc -Iinclude -fno-builtin tools/locale_format_diff.c -o $(OUT)/locale-format-hybrid-host-zig
	$(ZIG) cc -Iinclude -fno-builtin tools/locale_format_diff.c $(OUT)/libnelibc-zig.a -o $(OUT)/locale-format-hybrid-nelibc-zig
	$(OUT)/locale-format-hybrid-host-zig > $(OUT)/locale-format-hybrid-host-zig.txt
	$(OUT)/locale-format-hybrid-nelibc-zig > $(OUT)/locale-format-hybrid-nelibc-zig.txt
	diff -u $(OUT)/locale-format-hybrid-host-zig.txt $(OUT)/locale-format-hybrid-nelibc-zig.txt

locale-format-hybrid-diff-gcc: $(OUT)/libnelibc-gcc.a
	gcc -Iinclude -fno-builtin tools/locale_format_diff.c -o $(OUT)/locale-format-hybrid-host-gcc
	gcc -Iinclude -fno-builtin tools/locale_format_diff.c $(OUT)/libnelibc-gcc.a -o $(OUT)/locale-format-hybrid-nelibc-gcc
	$(OUT)/locale-format-hybrid-host-gcc > $(OUT)/locale-format-hybrid-host-gcc.txt
	$(OUT)/locale-format-hybrid-nelibc-gcc > $(OUT)/locale-format-hybrid-nelibc-gcc.txt
	diff -u $(OUT)/locale-format-hybrid-host-gcc.txt $(OUT)/locale-format-hybrid-nelibc-gcc.txt

locale-format-hybrid-diff-zig-musl: $(OUT)/libnelibc-zig-musl.a
	$(ZIG) cc -target x86_64-linux-musl -static -Iinclude -fno-builtin tools/locale_format_diff.c -o $(OUT)/locale-format-hybrid-host-zig-musl
	$(ZIG) cc -target x86_64-linux-musl -static -Iinclude -fno-builtin $(MUSL_STDIO_WRAP_FLAGS) tools/locale_format_diff.c $(OUT)/libnelibc-zig-musl.a -o $(OUT)/locale-format-hybrid-nelibc-zig-musl
	$(OUT)/locale-format-hybrid-host-zig-musl > $(OUT)/locale-format-hybrid-host-zig-musl.txt
	$(OUT)/locale-format-hybrid-nelibc-zig-musl > $(OUT)/locale-format-hybrid-nelibc-zig-musl.txt
	diff -u $(OUT)/locale-format-hybrid-host-zig-musl.txt $(OUT)/locale-format-hybrid-nelibc-zig-musl.txt

locale-format-hybrid-diff-tcc: $(OUT)/libnelibc-tcc.a
	$(TCC) -Iinclude -fno-builtin tools/locale_format_diff.c -o $(OUT)/locale-format-hybrid-host-tcc
	$(TCC) -Iinclude -fno-builtin tools/locale_format_diff.c $(OUT)/libnelibc-tcc.a -o $(OUT)/locale-format-hybrid-nelibc-tcc
	$(OUT)/locale-format-hybrid-host-tcc > $(OUT)/locale-format-hybrid-host-tcc.txt
	$(OUT)/locale-format-hybrid-nelibc-tcc > $(OUT)/locale-format-hybrid-nelibc-tcc.txt
	diff -u $(OUT)/locale-format-hybrid-host-tcc.txt $(OUT)/locale-format-hybrid-nelibc-tcc.txt

locale-format-hybrid-diff-matrix: locale-format-hybrid-diff-zig locale-format-hybrid-diff-gcc locale-format-hybrid-diff-zig-musl locale-format-hybrid-diff-tcc

.PHONY: lib-standalone-zig lib-standalone-gcc lib-standalone-tcc smoke-lib-standalone-zig smoke-lib-standalone-gcc smoke-lib-standalone-matrix lib-consumer-standalone-zig lib-consumer-standalone-gcc lib-consumer-link-standalone-tcc lib-consumer-standalone-matrix rand-r-diff-standalone-zig rand-r-diff-standalone-gcc rand-r-diff-standalone-matrix exit-smoke-standalone-zig exit-smoke-standalone-gcc exit-smoke-standalone-matrix abort-smoke-zig abort-smoke-gcc abort-smoke-zig-musl abort-smoke-tcc abort-smoke-standalone-zig abort-smoke-standalone-gcc abort-smoke-matrix locale-format-diff-zig locale-format-diff-gcc locale-format-diff-tcc locale-format-diff-matrix

clean:
	rm -rf $(OUT)
