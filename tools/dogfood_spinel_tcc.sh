#!/bin/sh
set -eu

if [ "$#" -ne 1 ] || [ ! -d "$1" ]; then
  echo "usage: $0 /path/to/spinel-checkout" >&2
  exit 2
fi

ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
SPINEL_DIR=$(CDPATH= cd -- "$1" && pwd)
ZIG=${ZIG:-zig}
TCC=${TCC:-tcc}
TCC_LIBTCC1=${TCC_LIBTCC1:-}
NM=${NM:-nm}
if [ -z "$TCC_LIBTCC1" ]; then
  TCC_INSTALL_DIR=$($TCC -print-search-dirs 2>/dev/null | sed -n 's/^install: //p')
  TCC_LIBTCC1="$TCC_INSTALL_DIR/libtcc1.a"
fi
if [ ! -x "$TCC" ] || [ ! -f "$TCC_LIBTCC1" ]; then
  echo "set TCC to an executable TinyCC and TCC_LIBTCC1 to its runtime archive" >&2
  exit 2
fi
if [ ! -f "$SPINEL_DIR/src/main.c" ] || [ ! -f "$SPINEL_DIR/Makefile" ]; then
  echo "not a Spinel source checkout: $SPINEL_DIR" >&2
  exit 2
fi

make -C "$ROOT" lib-zig ZIG="$ZIG"
make -C "$ROOT" lib-tcc ZIG="$ZIG" TCC="$TCC" \
  TCC_LIBTCC1="$TCC_LIBTCC1"
NO_CCACHE=1 make -B -C "$SPINEL_DIR" bin/spinel CC="$ZIG cc" \
  LDFLAGS="-Wl,-u,memcpy $ROOT/build/libnelibc-zig.a -lcrypt"
make -C "$SPINEL_DIR" regexp CC="$ZIG cc"

CLIENT_TMP=$(mktemp -d "${TMPDIR:-/tmp}/nelibc-spinel-tcc.XXXXXX")
trap 'rm -rf "$CLIENT_TMP"' EXIT HUP INT TERM
"$SPINEL_DIR/bin/spinel" -c \
  -e 'values = [3, 1, 4, 1, 5]; puts values.join(","); puts values.sort.join(",")' \
  -o "$CLIENT_TMP/client.c"
cat > "$CLIENT_TMP/memcpy_anchor.c" <<'EOF'
#include <stddef.h>
#include <string.h>
void *nelibc_tcc_force_memcpy(void *dst, const void *src, size_t n) {
  return memcpy(dst, src, n);
}
EOF

"$TCC" -fno-builtin -I"$SPINEL_DIR/lib" \
  -I"$SPINEL_DIR/lib/regexp" -c "$CLIENT_TMP/client.c" \
  -o "$CLIENT_TMP/client.o"
"$TCC" -fno-builtin -c "$CLIENT_TMP/memcpy_anchor.c" \
  -o "$CLIENT_TMP/memcpy_anchor.o"
"$ZIG" cc -Wl,-u,memcpy -fsanitize=undefined -fno-builtin \
  "$CLIENT_TMP/client.o" "$CLIENT_TMP/memcpy_anchor.o" \
  "$ROOT/build/libnelibc-tcc.a" "$TCC_LIBTCC1" \
  "$SPINEL_DIR/lib/libspinel_rt.a" -lm -lcrypt \
  -o "$CLIENT_TMP/client"

actual=$("$CLIENT_TMP/client")
expected=$(printf '3,1,4,1,5\n1,1,3,4,5')
if [ "$actual" != "$expected" ]; then
  printf 'unexpected Spinel client output:\n%s\n' "$actual" >&2
  exit 1
fi
printf '%s\n' "$actual"
file "$CLIENT_TMP/client"
for symbol in memcpy strlen strtol; do
  if ! "$NM" --defined-only "$CLIENT_TMP/client" |
      awk '{print $3}' | grep -Fxq "$symbol"; then
    echo "TinyCC client did not link Nelibc $symbol" >&2
    exit 1
  fi
done
echo "TinyCC-compiled Spinel client contains Nelibc memcpy, strlen, and strtol"
