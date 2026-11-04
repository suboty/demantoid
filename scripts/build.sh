#!/bin/sh
set -e

SOURCE=$1

if [ -z "$SOURCE" ] || [ ! -s "$SOURCE" ]; then
    exit 1
fi

BIN=${SOURCE%.*}

case "$SOURCE" in
    *.c)
        CC=${CC:-gcc}
        STD=${CSTD:--std=c11}
        ;;
    *.cpp|*.cc|*.cxx|*.c++)
        CC=${CXX:-g++}
        STD=${CXXSTD:--std=c++17}
        ;;
esac

$CC $STD -Wall -Wextra -O2 -o "$BIN" "$SOURCE"
