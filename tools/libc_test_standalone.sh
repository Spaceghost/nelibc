#!/bin/sh
# Build and run musl's libc-test (https://git.musl-libc.org/cgit/libc-test)
# against a Nelibc standalone archive, then compare the failing set with
# tools/libc_test_expected_standalone_<cc>.txt.
#
# usage: tools/libc_test_standalone.sh CC LIBC_TEST_CHECKOUT [OUTPUT_DIR]
#   CC  gcc | zig     build/libnelibc-standalone-<cc>.a
#
# libc-test's own Makefile assumes a hosted toolchain, so this driver compiles
# each functional/, regression/ and math/ test itself (docs/libc-test.md,
# "Standalone profile"):
#
# - Headers: Nelibc's include/ only. -nostdinc drops the host libc headers;
#   the compiler's freestanding headers (stddef.h, stdint.h, stdarg.h,
#   limits.h, float.h) stay, and the Linux UAPI directories (linux/, asm/,
#   asm-generic/) that Nelibc's standalone headers include are reached
#   through a directory of symlinks to them, never through /usr/include.
#   Zig is given -target x86_64-linux-none because it adds the native libc
#   include directories even under -nostdinc.
# - Link: -nostdlib -static -no-pie with Nelibc's _start and the archive
#   linked whole, as the standalone smokes link; no libgcc, no host libc.
#   A binary with an undefined symbol left (`nm -u`) counts as a link failure.
# - Run: libc-test's own src/common/runtest.c, built the same way, so a test
#   gets libc-test's semantics: forked child, 100 KiB stack limit, 5 s
#   timeout, FAIL on nonzero status, signal or timeout.
#
# Every test is classified as pass, build (a header or declaration is
# missing, or the test does not compile), link (undefined symbols listed),
# fail (nonzero exit), timeout, or crash (killed by a signal). The per-test
# table is OUTPUT_DIR/results.tsv, the summary OUTPUT_DIR/summary.txt.
#
# Exit status: 0 when every failing test is in the expected list, 1 when a
# test outside the list failed, 2 on usage or setup errors. Listed tests that
# now pass are printed so the list can shrink. The comparison is by test name;
# a listed test whose category changed (link -> fail, say) is reported but
# does not fail the run.
set -eu

usage() {
  echo "usage: $0 gcc|zig LIBC_TEST_CHECKOUT [OUTPUT_DIR]" >&2
  exit 2
}

# One test, run by xargs: compile, link, run, print one results.tsv row.
if [ "${1:-}" = "--one" ]; then
  name=$2
  cd "$SA_OUT"
  o=src/$name.o
  exe=src/$name.exe
  extra=
  case $name in
    functional/tls_align) extra=src/functional/tls_align_dso.o ;;
  esac
  # shellcheck disable=SC2086
  if ! $SA_CC $SA_CFLAGS -c -o "$o" "src/$name.c" > "$o.err" 2>&1; then
    detail=$(sed -n \
      -e "s/.*fatal error: \([^:]*\): No such file or directory.*/header:\1/p" \
      -e "s/.*fatal error: '\([^']*\)' file not found.*/header:\1/p" \
      -e "s/.*implicit declaration of function '\([^']*\)'.*/decl:\1/p" \
      -e "s/.*call to undeclared function '\([^']*\)'.*/decl:\1/p" \
      -e "s/.*error: '\([^']*\)' undeclared.*/ident:\1/p" \
      -e "s/.*use of undeclared identifier '\([^']*\)'.*/ident:\1/p" \
      -e "s/.*unknown type name '\([^']*\)'.*/type:\1/p" \
      "$o.err" | sort -u | tr '\n' ' ')
    [ -n "$detail" ] || detail="other:$(grep -m1 'error' "$o.err" | sed 's/^[^ ]* //' | tr '\t' ' ')"
    printf '%s\tbuild\t%s\n' "$name" "$detail"
    exit 0
  fi
  # shellcheck disable=SC2086
  if ! $SA_CC $SA_LDFLAGS -o "$exe" "$o" $extra src/common/libtest.a \
      -Wl,--whole-archive "$SA_ARCHIVE" -Wl,--no-whole-archive > "$exe.ld.err" 2>&1; then
    detail=$(sed -n \
      -e "s/.*undefined reference to \`\([^']*\)'.*/\1/p" \
      -e "s/.*undefined symbol: \([^ ]*\).*/\1/p" \
      -e "s/.*multiple definition of \`\([^']*\)'.*/dup:\1/p" \
      -e "s/.*duplicate symbol: \([^ ]*\).*/dup:\1/p" \
      "$exe.ld.err" | sort -u | tr '\n' ' ')
    [ -n "$detail" ] || detail="other:$(grep -m1 . "$exe.ld.err" | tr '\t' ' ')"
    printf '%s\tlink\t%s\n' "$name" "$detail"
    rm -f "$o"
    exit 0
  fi
  undef=$($SA_NM -u "$exe" | awk '{print $NF}' | sort -u | tr '\n' ' ')
  if [ -n "$undef" ]; then
    printf '%s\tlink\tnm-u:%s\n' "$name" "$undef"
    rm -f "$o" "$exe"
    exit 0
  fi
  src/common/runtest.exe -w '' "$exe" > "src/$name.err" 2>&1 || true
  [ -n "${KEEP_BINARIES:-}" ] || rm -f "$o" "$exe"
  # A test that fails after printing a line with no newline leaves the FAIL
  # marker glued to its output, so match it anywhere on the line.
  verdict=$(grep -o "FAIL src/$name\\.exe \\[[^]]*\\]" "src/$name.err" | head -1 | sed 's/.*\[\(.*\)\]/\1/')
  case $verdict in
    '') printf '%s\tpass\t\n' "$name" ;;
    status*) printf '%s\tfail\t%s\n' "$name" "$verdict" ;;
    'timed out') printf '%s\ttimeout\t\n' "$name" ;;
    signal*) printf '%s\tcrash\t%s\n' "$name" "$verdict" ;;
    *) printf '%s\tcrash\t%s\n' "$name" "$verdict" ;;
  esac
  exit 0
fi

[ "$#" -ge 2 ] || usage
CCNAME=$1
SRC=$2
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
OUT=${3:-$ROOT/build/libc-test-standalone-$CCNAME}
EXPECTED=$ROOT/tools/libc_test_expected_standalone_$CCNAME.txt
ARCHIVE=$ROOT/build/libnelibc-standalone-$CCNAME.a
LIBC_TEST_FLAGS="-std=c99 -D_POSIX_C_SOURCE=200809L -Wall -Wno-unused-function \
-Wno-missing-braces -Wno-unused -Wno-overflow -Wno-unknown-pragmas -fno-builtin \
-frounding-math -Werror=implicit-function-declaration -Werror=implicit-int \
-Werror=pointer-sign -Werror=pointer-arith -Isrc/common"

case $CCNAME in
  gcc)
    CC=${CC:-gcc}
    CCINC="-isystem $($CC -print-file-name=include)"
    TARGET_FLAGS="-fno-tree-loop-distribute-patterns"
    ;;
  zig)
    CC="${ZIG:-zig} cc"
    CCINC=
    TARGET_FLAGS="-target x86_64-linux-none ${ZIGFLAGS:-}"
    ;;
  *) usage ;;
esac
NM=${NM:-nm}
[ -d "$SRC/src/functional" ] || { echo "$SRC is not a libc-test checkout" >&2; exit 2; }
[ -f "$ARCHIVE" ] || { echo "missing $ARCHIVE; run make $ARCHIVE first" >&2; exit 2; }
UAPI_SRC=${UAPI_INCLUDE:-/usr/include}
for d in linux asm-generic; do
  [ -d "$UAPI_SRC/$d" ] || { echo "no Linux UAPI headers at $UAPI_SRC/$d (set UAPI_INCLUDE)" >&2; exit 2; }
done
UAPI_ASM=$UAPI_SRC/asm
[ -d "$UAPI_ASM" ] || UAPI_ASM=$UAPI_SRC/x86_64-linux-gnu/asm
[ -d "$UAPI_ASM" ] || { echo "no Linux UAPI asm/ headers under $UAPI_SRC" >&2; exit 2; }

rm -rf "$OUT"
mkdir -p "$OUT/src" "$OUT/uapi"
OUT=$(CDPATH= cd -- "$OUT" && pwd)
ln -s "$UAPI_SRC/linux" "$OUT/uapi/linux"
ln -s "$UAPI_SRC/asm-generic" "$OUT/uapi/asm-generic"
ln -s "$UAPI_ASM" "$OUT/uapi/asm"
# Copy the sources only: a checkout that was built before holds stale objects.
(cd "$SRC/src" && tar cf - --exclude='*.o' --exclude='*.lo' --exclude='*.so' \
  --exclude='*.a' --exclude='*.exe' --exclude='*.err' --exclude=REPORT \
  common functional regression math) | (cd "$OUT/src" && tar xf -)
cd "$OUT"

SA_CFLAGS="$LIBC_TEST_FLAGS -DNELIBC_STANDALONE -nostdinc $CCINC -I$ROOT/include \
-idirafter $OUT/uapi -ffreestanding -fno-stack-protector -fno-pie -fno-sanitize=all $TARGET_FLAGS"
SA_LDFLAGS="-nostdlib -static -no-pie -fno-pie -fno-sanitize=all -Wl,-e,_start \
-Wl,--build-id=none $TARGET_FLAGS"
LC_ALL=C
export LC_ALL

# libc-test's support library: every source except the runner. A support
# file that does not compile is left out and reported; a test that needs it
# then fails to link on its t_* symbol.
: > common.log
objs=
for f in src/common/*.c; do
  b=${f%.c}
  [ "$b" = src/common/runtest ] && continue
  # shellcheck disable=SC2086
  if $CC $SA_CFLAGS -c -o "$b.o" "$f" > "$b.o.err" 2>&1; then
    objs="$objs $b.o"
  else
    echo "common: ${b#src/} does not compile: $(grep -m1 error "$b.o.err")" >> common.log
  fi
done
# shellcheck disable=SC2086
ar rc src/common/libtest.a $objs
# shellcheck disable=SC2086
$CC $SA_CFLAGS -c -o src/common/runtest.o src/common/runtest.c > src/common/runtest.o.err 2>&1 &&
  $CC $SA_LDFLAGS -o src/common/runtest.exe src/common/runtest.o src/common/libtest.a \
    -Wl,--whole-archive "$ARCHIVE" -Wl,--no-whole-archive > src/common/runtest.ld.err 2>&1 ||
  { echo "the runner did not build; see $OUT/src/common/runtest.*err" >&2; exit 2; }
[ -z "$($NM -u src/common/runtest.exe)" ] || { echo "the runner has undefined symbols" >&2; exit 2; }
# shellcheck disable=SC2086
$CC $SA_CFLAGS -c -o src/functional/tls_align_dso.o src/functional/tls_align_dso.c \
  > src/functional/tls_align_dso.o.err 2>&1 || true

# Tests: every .c in the three directories except the shared-object helpers.
(cd src && ls functional/*.c regression/*.c math/*.c) | sed 's/\.c$//' |
  grep -v '_dso$' > tests.txt

SA_OUT=$OUT SA_CC=$CC SA_CFLAGS=$SA_CFLAGS SA_LDFLAGS=$SA_LDFLAGS SA_ARCHIVE=$ARCHIVE SA_NM=$NM
export SA_OUT SA_CC SA_CFLAGS SA_LDFLAGS SA_ARCHIVE SA_NM
xargs -P "${JOBS:-$(nproc)}" -n 1 sh "$ROOT/tools/libc_test_standalone.sh" --one \
  < tests.txt | sort > results.tsv
[ "$(wc -l < results.tsv)" -eq "$(wc -l < tests.txt)" ] ||
  { echo "results.tsv has $(wc -l < results.tsv) rows for $(wc -l < tests.txt) tests" >&2; exit 2; }

{
  echo "libc-test standalone $CCNAME: $(wc -l < tests.txt) tests"
  cut -f2 results.tsv | sort | uniq -c | sort -rn
  cat common.log
  echo
  echo "missing headers (tests blocked):"
  awk -F'\t' '$2 == "build" { n = split($3, a, " "); for (i = 1; i <= n; i++) if (a[i] ~ /^header:/) c[substr(a[i], 8)]++ }
    END { for (k in c) printf "%5d %s\n", c[k], k }' results.tsv | sort -k1,1nr -k2
  echo
  echo "missing declarations, identifiers and types (tests blocked):"
  awk -F'\t' '$2 == "build" { n = split($3, a, " "); for (i = 1; i <= n; i++) if (a[i] ~ /^(decl|ident|type):/) c[a[i]]++ }
    END { for (k in c) printf "%5d %s\n", c[k], k }' results.tsv | sort -k1,1nr -k2
  echo
  echo "undefined symbols at link (tests blocked; t_* are libc-test's own support code):"
  awk -F'\t' '$2 == "link" && $3 !~ /^other:/ { sub(/^nm-u:/, "", $3); n = split($3, a, " "); for (i = 1; i <= n; i++) if (a[i] !~ /^dup:/) c[a[i]]++ }
    END { for (k in c) printf "%5d %s\n", c[k], k }' results.tsv | sort -k1,1nr -k2
  echo
  echo "symbols the test defines itself (static whole-archive link; tests blocked):"
  awk -F'\t' '$2 == "link" { n = split($3, a, " "); for (i = 1; i <= n; i++) if (a[i] ~ /^dup:/) c[substr(a[i], 5)]++ }
    END { for (k in c) printf "%5d %s\n", c[k], k }' results.tsv | sort -k1,1nr -k2
  awk -F'\t' '$2 == "link" && $3 ~ /^other:/ { print "other link failure: " $1 ": " $3 }' results.tsv
} > summary.txt

awk -F'\t' '$2 != "pass" { printf "%s [%s]\n", $1, $2 }' results.tsv > failures.txt
cut -d' ' -f1 failures.txt | sort -u > failures.names
if [ -f "$EXPECTED" ]; then
  sed 's/ .*//' "$EXPECTED" | sort -u > expected.names
else
  : > expected.names
fi
comm -13 expected.names failures.names > new-failures.txt
comm -23 expected.names failures.names > now-passing.txt
# Listed tests that still fail but under another category than recorded.
awk 'NR == FNR { e[$0]; next } !($0 in e) { print $1 }' "$([ -f "$EXPECTED" ] && echo "$EXPECTED" || echo /dev/null)" failures.txt |
  sort -u | comm -12 - expected.names > changed-category.txt
pick() { awk 'NR == FNR { n[$1]; next } ($1 in n)' "$1" failures.txt | sed 's/^/  /'; }

echo "libc-test standalone $CCNAME: $(wc -l < tests.txt) tests, $(wc -l < failures.names) failing, $(wc -l < expected.names) expected"
cut -f2 results.tsv | sort | uniq -c | sort -rn | sed 's/^ */  /'
[ ! -s common.log ] || sed 's/^/  /' common.log
if [ -s changed-category.txt ]; then
  echo "still failing, different category (update $EXPECTED):"
  pick changed-category.txt
fi
if [ -s now-passing.txt ]; then
  echo "now passing (remove from $EXPECTED):"
  sed 's/^/  /' now-passing.txt
fi
if [ -s new-failures.txt ]; then
  echo "new failures:"
  pick new-failures.txt
  echo "details: $OUT/results.tsv, $OUT/src/<name>.err, .o.err, .exe.ld.err"
  exit 1
fi
