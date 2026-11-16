#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")"
[[ -x .venv/bin/python ]] || { echo 'Run bash compile.sh first' >&2; exit 1; }
exec .venv/bin/python visualize.py "$@"
