#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")"
echo '[1/3] Building C++ analyzer'
bash comp.sh
command -v python3 >/dev/null || { echo 'python3 is required' >&2; exit 1; }
echo '[2/3] Creating isolated Python environment'
if [[ ! -x .venv/bin/python ]]; then python3 -m venv .venv; fi
echo '[3/3] Installing Python dependencies'
.venv/bin/python -m pip install -r requirements.txt -r requirements-viz.txt
echo 'Ready: bash start.sh IBM ; bash visualize.sh IBM'
