#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")"
g++ -std=c++17 -O2 -Wall -Wextra -Wpedantic -Wconversion -o season main.cpp season.cpp
echo "Built ./season"
