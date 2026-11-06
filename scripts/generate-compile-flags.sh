#!/bin/sh
set -e

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
OUT="$ROOT/compile_flags.txt"
PATHS_SCRIPT="$ROOT/scripts/get-libpg-path.sh"

echo "$PATHS_SCRIPT"

if [[ ! -x "$PATHS_SCRIPT" ]]; then
    echo "Error: $PATHS_SCRIPT not found or not executable." >&2
    exit 1
fi

LIBPQ_OUT="$("$PATHS_SCRIPT")"
LIBPQ_INC="$(echo "$LIBPQ_OUT" | grep '^INCLUDEDIR=' | cut -d= -f2)"

if [[ -z "$LIBPQ_INC" ]]; then
    echo "Error: failed to get libpq include dir from $PATHS_SCRIPT" >&2
    exit 1
fi

{
    echo "-I$ROOT/include"
    echo "-I$LIBPQ_INC"
    echo "-Wall"
    echo "-Wextra"
} > "$OUT"

echo "Wrote $OUT:"
cat "$OUT"
