#!/bin/sh
# Build and run unit tests on Linux (gcc/clang)
# Usage: sh doc/test/run.sh
# Requires: build-essential, libcap-dev (for core/system/priv.c)
set -eu

ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
OUT=${TMPDIR:-/tmp}/xebec_unit_test
CC=${CC:-gcc}
CFLAGS="-std=c11 -D_POSIX_C_SOURCE=200809L -I$ROOT"

mkdir -p "$OUT"

run_test() {
    name=$1
    shift
    "$CC" $CFLAGS -o "$OUT/$name" "$@" -lm
    "$OUT/$name"
}

# format
run_test test_format \
    "$ROOT/doc/test/unit/format/test_format.c" \
    "$ROOT/core/format.c" \
    "$ROOT/core/mem.c"

# struct: exclude realm.c
STRUCT_SRC=""
for f in "$ROOT"/core/struct/*.c; do
    case "$f" in
        */realm.c) ;;
        *) STRUCT_SRC="$STRUCT_SRC $f" ;;
    esac
done
run_test test_struct \
    "$ROOT/doc/test/unit/struct/test_struct.c" \
    $STRUCT_SRC \
    "$ROOT/core/mem.c" \
    "$ROOT/core/algo/sort.c"

# system
set -- "$ROOT"/core/system/*.c
"$CC" $CFLAGS -o "$OUT/test_system" \
    "$ROOT/doc/test/unit/system/test_system.c" \
    "$@" \
    -pthread -ldl -lm -lcap
"$OUT/test_system"

echo "all unit tests passed (linux)"
