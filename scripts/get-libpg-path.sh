#!/bin/sh
set -e

get_libpq_paths() {
    OS="$(uname -s)"

    if [[ "$OS" == "Linux" ]]; then
        if ! command -v pg_config >/dev/null 2>&1; then
            echo "Error: pg_config not found. Install libpq-dev first." >&2
            exit 1
        fi
        INCLUDEDIR="$(pg_config --includedir)"
        LIBDIR="$(pg_config --libdir)"
    elif [[ "$OS" == "Darwin" ]]; then
        if ! command -v brew >/dev/null 2>&1; then
            echo "Error: Homebrew not found." >&2
            exit 1
        fi
        if ! brew list libpq >/dev/null 2>&1; then
            echo "Error: libpq not installed via Homebrew. Run: brew install libpq" >&2
            exit 1
        fi
        PREFIX="$(brew --prefix libpq)"
        INCLUDEDIR="$PREFIX/include"
        LIBDIR="$PREFIX/lib"
    else
        echo "Error: unsupported OS: $OS" >&2
        exit 1
    fi

    echo "INCLUDEDIR=$INCLUDEDIR"
    echo "LIBDIR=$LIBDIR"
}

get_libpq_paths
