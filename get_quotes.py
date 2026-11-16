#!/usr/bin/env python3
"""Download adjusted daily stock prices and export the C++ input format."""
import argparse
import csv
import math
from pathlib import Path


def normalize_frame(frame):
    if frame is None or frame.empty:
        raise ValueError("No prices returned by Yahoo Finance")
    if getattr(frame.columns, "nlevels", 1) > 1:
        frame = frame.droplevel(-1, axis=1)
    if "Close" not in frame or "Volume" not in frame:
        raise ValueError("Missing Close or Volume columns")
    result = []
    for date, row in frame.sort_index().iterrows():
        try:
            price = float(row["Close"])
            volume = float(row["Volume"])
            if not (math.isfinite(price) and price > 0 and math.isfinite(volume) and volume >= 0):
                continue
            result.append((date.date().isoformat(), format(price, ".12g"), str(int(volume))))
        except (ValueError, TypeError, OverflowError):
            continue
    if not result:
        raise ValueError("No valid trading observations")
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("ticker")
    parser.add_argument("--start", default="2000-01-01")
    parser.add_argument("--end", help="Exclusive ending date")
    parser.add_argument("--output")
    args = parser.parse_args()

    import yfinance as yf
    kwargs = dict(tickers=args.ticker, start=args.start, auto_adjust=True,
                  interval="1d", progress=False, threads=False,
                  multi_level_index=False)
    if args.end:
        kwargs["end"] = args.end
    rows = normalize_frame(yf.download(**kwargs))
    target = Path(args.output or (args.ticker.upper() + ".csv"))
    target.parent.mkdir(parents=True, exist_ok=True)
    with target.open("w", newline="", encoding="utf-8") as handle:
        writer = csv.writer(handle, lineterminator="\n")
        writer.writerow(("Date", "Close", "Volume"))
        writer.writerows(rows)
    print(f"Wrote {len(rows)} adjusted observations to {target}")


if __name__ == "__main__":
    main()
