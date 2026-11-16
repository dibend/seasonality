#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")"
if [[ ! -x ./season ]]; then echo 'Run bash compile.sh first' >&2; exit 1; fi
if [[ ${1:-} == --csv ]]; then
  [[ $# -ge 2 ]] || { echo 'Usage: bash start.sh --csv FILE.csv [MIN_YEARS]' >&2; exit 2; }
  exec ./season "$2" "${3:-3}"
fi
ticker=${1:-IBM}
if [[ $# -gt 0 ]]; then shift; fi
min_years=3
if [[ ${1:-} =~ ^[0-9]+$ ]]; then min_years=$1; shift; fi
[[ $ticker =~ ^[A-Za-z0-9.^_-]+$ ]] || { echo 'Invalid ticker' >&2; exit 2; }
start='2000-01-01'
end=''
refresh=0
while [[ $# -gt 0 ]]; do
  case "$1" in
    --start) [[ $# -gt 1 ]] || exit 2; start=$2; shift 2;;
    --end) [[ $# -gt 1 ]] || exit 2; end=$2; shift 2;;
    --refresh) refresh=1; shift;;
    *) echo "Unknown argument: $1" >&2; exit 2;;
  esac
done
[[ -x .venv/bin/python ]] || { echo 'Run bash compile.sh first' >&2; exit 1; }
mkdir -p data
key=${start}_${end:-latest}
key=${key//[^A-Za-z0-9_.-]/_}
csv="data/${ticker^^}_${key}.csv"
if [[ $refresh == 1 || ! -s $csv ]]; then
  cmd=(.venv/bin/python get_quotes.py "$ticker" --start "$start" --output "$csv")
  [[ -z $end ]] || cmd+=(--end "$end")
  "${cmd[@]}"
else
  echo "Using cached data: $csv (pass --refresh to redownload)"
fi
exec ./season "$csv" "$min_years"
