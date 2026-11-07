#!/bin/sh
# Build and run musl's libc-test (https://git.musl-libc.org/cgit/libc-test)
# against a Nelibc hybrid archive, then compare the failing set with the
# recorded expectations in tools/libc_test_expected_<profile>.txt.
#
# usage: tools/libc_test.sh PROFILE LIBC_TEST_CHECKOUT [OUTPUT_DIR]
#   PROFILE  gcc | zig      hybrid archives; the host libc supplies the rest
#
# Every test is compiled with Nelibc's headers first on the include path and
# linked with the archive ahead of the host libc, which is how a consumer
# uses the hybrid profile. Only dynamic test binaries are built: a static
# glibc beside a second archive defining the same symbols is a different
# experiment. A test that fails to compile counts as a failure ("[build]").
#
# Exit status: 0 when the failing set is a subset of the expected list, 1
# when a test outside the list failed, 2 on usage or setup errors. Tests in
# the list that now pass are printed so the list can be shortened.
set -eu

usage() {
  echo "usage: $0 gcc|zig LIBC_TEST_CHECKOUT [OUTPUT_DIR]" >&2
  exit 2
}

[ "$#" -ge 2 ] || usage
PROFILE=$1
SRC=$2
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
OUT=${3:-$ROOT/build/libc-test-$PROFILE}
EXPECTED=$ROOT/tools/libc_test_expected_$PROFILE.txt

case $PROFILE in
  gcc) TEST_CC=${CC:-gcc}; ARCHIVE=$ROOT/build/libnelibc-gcc.a ;;
  zig) TEST_CC="${ZIG:-zig} cc ${ZIGFLAGS:-}"; ARCHIVE=$ROOT/build/libnelibc-zig.a ;;
  *) usage ;;
esac
[ -d "$SRC/src/functional" ] || { echo "$SRC is not a libc-test checkout" >&2; exit 2; }
[ -f "$ARCHIVE" ] || { echo "missing $ARCHIVE; run make lib-$PROFILE first" >&2; exit 2; }

rm -rf "$OUT"
mkdir -p "$OUT"
# Copy the sources only: a checkout that was built before would otherwise
# hand its old test binaries to make as up to date.
(cd "$SRC" && tar cf - --exclude=.git --exclude='*.o' --exclude='*.lo' \
  --exclude='*.so' --exclude='*.a' --exclude='*.exe' --exclude='*.err' \
  --exclude=REPORT --exclude=config.mak --exclude=build.log .) | (cd "$OUT" && tar xf -)
cd "$OUT"
make -s cleanall >/dev/null 2>&1 || true
sed -i 's/^\$(1)\.BINS_TEMPL:=bin\.exe bin-static\.exe$/$(1).BINS_TEMPL:=bin.exe/' Makefile
grep -q '^\$(1)\.BINS_TEMPL:=bin\.exe$' Makefile || { echo "libc-test Makefile layout changed; adjust the BINS_TEMPL edit" >&2; exit 2; }
# The hybrid wrappers reach the host's headers through #include_next, a GNU
# extension that -pedantic-errors rejects before any declaration is checked;
# the API tests keep -Werror for everything else.
sed -i 's/^\(\$(api\.OBJS):CFLAGS+=\)-pedantic-errors /\1/' Makefile
grep -q '^\$(api\.OBJS):CFLAGS+=-Werror' Makefile || { echo "libc-test Makefile layout changed; adjust the api CFLAGS edit" >&2; exit 2; }
cp config.mak.def config.mak
cat >> config.mak <<CONFIG
CC := $TEST_CC
CFLAGS += -I$ROOT/include
LDLIBS := $ARCHIVE \$(LDLIBS)
CONFIG

make -j"${JOBS:-$(nproc)}" -k > build.log 2>&1 || true
[ -f src/REPORT ] || { echo "libc-test produced no REPORT; see $OUT/build.log" >&2; exit 2; }

{
  # A test that fails after printing a line with no newline leaves the FAIL
  # marker glued to its output, so match it anywhere on the line.
  grep -o 'FAIL src/[^ ]*\.exe \[' src/REPORT | sed 's#^FAIL src/##; s#\.exe .*##'
  grep '^BUILDERROR ' build.log | sed 's#^BUILDERROR src/##; s#\.o$# [build]#; s#\.exe$# [build]#'
} | sort -u > failures.txt

if [ -f "$EXPECTED" ]; then
  sort -u "$EXPECTED" > expected.sorted
else
  : > expected.sorted
fi
comm -13 expected.sorted failures.txt > new-failures.txt
comm -23 expected.sorted failures.txt > now-passing.txt

total=$(find src -name '*.exe' | wc -l)
echo "libc-test $PROFILE: $total test binaries, $(wc -l < failures.txt) failing, $(wc -l < expected.sorted) expected"
if [ -s now-passing.txt ]; then
  echo "now passing (remove from $EXPECTED):"
  sed 's/^/  /' now-passing.txt
fi
if [ -s new-failures.txt ]; then
  echo "new failures:"
  sed 's/^/  /' new-failures.txt
  echo "details: $OUT/src/<name>.err"
  exit 1
fi
