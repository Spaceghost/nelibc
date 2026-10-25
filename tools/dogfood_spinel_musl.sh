#!/bin/sh
set -eu

if [ "$#" -ne 1 ] || [ ! -d "$1" ]; then
  echo "usage: $0 /path/to/spinel-checkout" >&2
  exit 2
fi

ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
SPINEL_DIR=$(CDPATH= cd -- "$1" && pwd)
ZIG=${ZIG:-zig}
NM=${NM:-nm}
MUSL_STDIO_WRAP_FLAGS=${MUSL_STDIO_WRAP_FLAGS:-}
RUNTIME_TMP=$(mktemp -d "${TMPDIR:-/tmp}/nelibc-spinel-musl.XXXXXX")
trap 'rm -rf "$RUNTIME_TMP"' EXIT HUP INT TERM

if [ ! -f "$SPINEL_DIR/src/main.c" ] || [ ! -f "$SPINEL_DIR/Makefile" ]; then
  echo "not a Spinel source checkout: $SPINEL_DIR" >&2
  exit 2
fi
if [ ! -f "$SPINEL_DIR/vendor/prism/include/prism.h" ] &&
   ! command -v ruby >/dev/null 2>&1; then
  echo "Spinel needs its Prism dependency; run 'make deps' in $SPINEL_DIR" >&2
  exit 2
fi

make -C "$ROOT" lib-zig lib-zig-musl ZIG="$ZIG"
NO_CCACHE=1 make -B -C "$SPINEL_DIR" bin/spinel CC="$ZIG cc" \
  LDFLAGS="-Wl,-u,memcpy $ROOT/build/libnelibc-zig.a -lcrypt"

mkdir -p "$RUNTIME_TMP/objects" "$RUNTIME_TMP/include" \
  "$RUNTIME_TMP/stage/bin" "$RUNTIME_TMP/stage/lib"
cat > "$RUNTIME_TMP/include/execinfo.h" <<'EOF'
#ifndef NELIBC_SPINEL_NO_EXECINFO_H
#define NELIBC_SPINEL_NO_EXECINFO_H
static inline int backtrace(void **buffer, int size) {
  (void)buffer; (void)size; return 0;
}
static inline char **backtrace_symbols(void *const *buffer, int size) {
  (void)buffer; (void)size; return 0;
}
#endif
EOF

members='sp_bigint sp_crypto sp_pack sp_time sp_core sp_net sp_system sp_gc sp_alloc sp_dtoa sp_marshal sp_format sp_string sp_inspect sp_array sp_str sp_hash sp_proc sp_exc sp_re sp_random sp_fiber sp_sched sp_io sp_cold sp_process sp_process_status'
for member in $members; do
  source="$SPINEL_DIR/lib/$member.c"
  object="$RUNTIME_TMP/objects/$member.o"
  "$ZIG" cc -target x86_64-linux-musl -O2 -ffunction-sections -fdata-sections \
    -I"$RUNTIME_TMP/include" -I"$SPINEL_DIR/lib" \
    -I"$SPINEL_DIR/lib/regexp" -c "$source" -o "$object"
done
for member in re_compile re_exec re_utf8; do
  source="$SPINEL_DIR/lib/regexp/$member.c"
  object="$RUNTIME_TMP/objects/$member.o"
  "$ZIG" cc -target x86_64-linux-musl -O2 -ffunction-sections -fdata-sections \
    -I"$RUNTIME_TMP/include" -I"$SPINEL_DIR/lib" \
    -I"$SPINEL_DIR/lib/regexp" -c "$source" -o "$object"
done
"$ZIG" ar rcs "$RUNTIME_TMP/stage/lib/libspinel_rt.a" \
  "$RUNTIME_TMP"/objects/*.o

for header in "$SPINEL_DIR"/lib/*.h; do
  ln -s "$header" "$RUNTIME_TMP/stage/lib/$(basename "$header")"
done
ln -s "$SPINEL_DIR/lib/regexp" "$RUNTIME_TMP/stage/lib/regexp"
cp "$SPINEL_DIR/bin/spinel" "$RUNTIME_TMP/stage/bin/spinel"

cc_command="$ZIG cc -target x86_64-linux-musl -static -Wl,-u,memcpy $MUSL_STDIO_WRAP_FLAGS"
"$RUNTIME_TMP/stage/bin/spinel" --cc="$cc_command" \
  --link "$ROOT/build/libnelibc-zig-musl.a" \
  -e 'values = [3, 1, 4, 1, 5]; puts values.join(","); puts values.sort.join(",")' \
  -o "$RUNTIME_TMP/nelibc-spinel-client"

actual=$("$RUNTIME_TMP/nelibc-spinel-client")
expected=$(printf '3,1,4,1,5\n1,1,3,4,5')
if [ "$actual" != "$expected" ]; then
  printf 'unexpected Spinel client output:\n%s\n' "$actual" >&2
  exit 1
fi
printf '%s\n' "$actual"
file "$RUNTIME_TMP/nelibc-spinel-client"
for symbol in memcpy strlen strtol; do
  if ! "$NM" --defined-only "$RUNTIME_TMP/nelibc-spinel-client" |
      awk '{print $3}' | grep -Fxq "$symbol"; then
    echo "static musl client did not link Nelibc $symbol" >&2
    exit 1
  fi
done
echo "static musl Spinel client contains Nelibc memcpy, strlen, and strtol"
