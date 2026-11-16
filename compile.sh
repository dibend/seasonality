#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")"

usage() {
    echo "Usage: bash compile.sh [--cpp-only]"
    echo "  --cpp-only  Build the C++ analyzer only (no Python setup)"
}

if [[ $# -gt 1 || ( $# -eq 1 && "${1:-}" != "--cpp-only" ) ]]; then
    usage >&2
    exit 2
fi

echo '[1/3] Building C++ analyzer'
g++ -std=c++17 -O2 -Wall -Wextra -Wpedantic -Wconversion \
    -o season main.cpp season.cpp

if [[ "${1:-}" == "--cpp-only" ]]; then
    echo 'Built ./season (C++ only)'
    exit 0
fi

command -v python3 >/dev/null || { echo 'python3 is required' >&2; exit 1; }
echo '[2/3] Preparing isolated Python environment'
if [[ ! -x .venv/bin/python ]]; then python3 -m venv .venv; fi
echo '[3/3] Installing Python dependencies'
.venv/bin/python -m pip install -r requirements.txt -r requirements-viz.txt
echo 'Ready: bash start.sh IBM ; bash visualize.sh IBM'
