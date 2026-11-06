#!/bin/sh
set -e

install_libpq() {
    OS="$(uname -s)"

    if [[ "$OS" == "Darwin" ]]; then
        echo "macOS detected. Installing libpq via Homebrew..."

        if ! command -v brew >/dev/null 2>&1; then
            echo "Error: Homebrew is not installed. Install it first: https://brew.sh" >&2
            exit 1
        fi

        brew install libpq
        echo "Done."
        return
    fi

    if [[ "$OS" == "Linux" ]]; then
        if [[ ! -f /etc/os-release ]]; then
            echo "Error: cannot detect Linux distribution." >&2
            exit 1
        fi

        . /etc/os-release

        if [[ "$ID" != "ubuntu" ]]; then
            echo "Error: this script supports only Ubuntu (detected: ${ID:-unknown})." >&2
            exit 1
        fi

        echo "Ubuntu detected. Installing libpq-dev..."
        sudo apt update
        sudo apt install -y libpq-dev
        echo "Done."
        return
    fi

    echo "Error: unsupported OS: $OS. This script supports macOS and Ubuntu only." >&2
    exit 1
}

install_libpq
