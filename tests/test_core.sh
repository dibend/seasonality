#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
bash comp.sh
tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT
{
  echo 'Date,Close,Volume'
  for year in 2020 2021 2022; do
    echo "$year-01-02,100,1000"
    echo "$year-02-03,110,1000"
    echo "$year-03-04,90,1000"
  done
} > "$tmp/TEST.csv"
./season "$tmp/TEST.csv" 3 > "$tmp/result"
grep -q '^long,TEST,1/2,2/3,10.0000,3,100.0000$' "$tmp/result"
grep -q '^worst_period,TEST,2/3,3/4,-18.1818,3,0.0000$' "$tmp/result"
if ./season "$tmp/TEST.csv" 4 >/dev/null 2>&1; then
  echo 'FAIL: minimum-years rejection missing' >&2
  exit 1
fi
echo 'PASS: date ordering, averages, win rates, minimum-years threshold'
