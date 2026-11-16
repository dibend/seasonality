#!/usr/bin/env python3
"""Turn C++ seasonality output and the matching price CSV into an offline Plotly HTML report."""
import argparse
import csv
import io
import math
import subprocess
import webbrowser
from collections import defaultdict
from datetime import datetime
from html import escape
from pathlib import Path


HEADER = "signal,ticker,buy_date,sell_date,avg_return_pct,years,win_rate_pct"


def parse_results(output):
    lines = output.splitlines()
    starts = [i for i, line in enumerate(lines) if line.strip() == HEADER]
    if not starts:
        raise ValueError("C++ result header was not found")
    records = []
    for row in csv.DictReader(io.StringIO("\n".join(lines[starts[-1]:]))):
        if row.get("signal") not in ("long", "worst_period"):
            continue
        for key in ("buy_date", "sell_date", "avg_return_pct", "years", "win_rate_pct"):
            if not row.get(key):
                raise ValueError("Incomplete C++ result: " + key)
        for key in ("buy_date", "sell_date"):
            month, day = [int(v) for v in row[key].split("/")]
            datetime(2000, month, day)
            row[key + "_pair"] = (month, day)
        for key in ("avg_return_pct", "win_rate_pct"):
            row[key] = float(row[key])
            if not math.isfinite(row[key]):
                raise ValueError("Invalid result: " + key)
        row["years"] = int(row["years"])
        records.append(row)
    if len(records) != 2:
        raise ValueError("Expected exactly two C++ result rows")
    return records


def load_prices(path):
    with Path(path).open(newline="", encoding="utf-8-sig") as handle:
        reader = csv.DictReader(handle)
        if reader.fieldnames != ["Date", "Close", "Volume"]:
            raise ValueError("Expected Date,Close,Volume header")
        rows = []
        for row in reader:
            day = datetime.strptime(row["Date"], "%Y-%m-%d").date()
            price = float(row["Close"])
            if not math.isfinite(price) or price <= 0.0:
                raise ValueError("Invalid price on " + str(day))
            rows.append((day, price))
    if not rows:
        raise ValueError("No price records")
    rows.sort()
    return rows


def annual_returns(prices, start_pair, end_pair):
    years = defaultdict(dict)
    for day, price in prices:
        years[day.year][(day.month, day.day)] = price
    result = []
    for year, values in sorted(years.items()):
        if start_pair in values and end_pair in values:
            result.append((year, 100 * (values[end_pair] / values[start_pair] - 1)))
    return result


def seasonal_curve(prices):
    by_year = defaultdict(list)
    for day, price in prices:
        by_year[day.year].append((day, price))
    values = defaultdict(list)
    for year_values in by_year.values():
        year_values.sort()
        baseline = year_values[0][1]
        for day, price in year_values:
            values[(day.month, day.day)].append(100 * (price / baseline - 1))
    pairs = sorted(values)
    return ([datetime(2000, *pair) for pair in pairs],
            [sum(values[pair]) / len(values[pair]) for pair in pairs],
            [len(values[pair]) for pair in pairs])


def render(prices, results, filename):
    import plotly.graph_objects as go
    from plotly.subplots import make_subplots
    from plotly.offline import plot

    figure = make_subplots(rows=3, cols=2, specs=[
        [{"colspan": 2}, None], [{}, {}], [{"colspan": 2}, None]
    ], subplot_titles=("Adjusted close", "Average seasonal year",
                       "Selected periods", "Annual returns by selected period"),
       vertical_spacing=0.14)
    figure.add_trace(go.Scatter(
        x=[day for day, price in prices], y=[price for day, price in prices],
        name="Adjusted close", mode="lines"), row=1, col=1)
    dates, average, counts = seasonal_curve(prices)
    figure.add_trace(go.Scatter(
        x=dates, y=average, customdata=counts, mode="lines",
        name="Average seasonal change",
        hovertemplate="%{x|%b %d}: %{y:.2f}% · %{customdata} years<extra></extra>"),
        row=2, col=1)
    figure.update_xaxes(tickformat="%b", row=2, col=1)
    for item in results:
        name = "Best long" if item["signal"] == "long" else "Worst holding period"
        figure.add_trace(go.Bar(x=[name], y=[item["avg_return_pct"]], name=name,
                                text=[f'{item["avg_return_pct"]:.2f}%'],
                                textposition="outside"), row=2, col=2)
        yearly = annual_returns(prices, item["buy_date_pair"], item["sell_date_pair"])
        figure.add_trace(go.Bar(x=[str(year) for year, ret in yearly],
                                y=[ret for year, ret in yearly],
                                name=name + " by year"), row=3, col=1)
    figure.update_layout(template="plotly_dark", height=1150, barmode="group",
                         title="Stock seasonality · historical in-sample exploration",
                         legend={"orientation": "h", "y": -0.1},
                         margin={"t": 100, "b": 95})
    panel = plot(figure, include_plotlyjs=True, output_type="div",
                 config={"responsive": True, "displaylogo": False})
    cards = "".join(
        f'<div class="card"><h2>{escape(item["signal"])}</h2>'
        f'<b>{escape(item["buy_date"])} → {escape(item["sell_date"])}</b>'
        f'<p>Mean return: {item["avg_return_pct"]:.2f}% · '
        f'Observations: {item["years"]} · Win rate: {item["win_rate_pct"]:.1f}%</p></div>'
        for item in results)
    html = (
        '<!doctype html><html lang="en"><head><meta charset="utf-8">'
        '<meta name="viewport" content="width=device-width, initial-scale=1">'
        '<title>Seasonality dashboard</title><style>'
        'body{background:#101723;color:#edf4fd;font:16px system-ui;margin:0}'
        'main{max-width:1500px;padding:24px;margin:auto}'
        '.cards{display:flex;flex-wrap:wrap;gap:16px}'
        '.card{background:#1b293a;border:1px solid #445;border-radius:14px;padding:18px}'
        '</style></head><body><main><h1>Stock Seasonality Explorer</h1>'
        '<div class="cards">' + cards + '</div>' + panel +
        '<p>In-sample research only. Results may be overfit. Worst period is '
        'not a short-sale strategy; trading costs and risks are excluded.</p>'
        '</main></body></html>')
    filename = Path(filename)
    filename.parent.mkdir(parents=True, exist_ok=True)
    filename.write_text(html, encoding="utf-8")
    return filename


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("ticker", nargs="?", default="IBM")
    parser.add_argument("--csv", help="Use an existing normalized CSV")
    parser.add_argument("--results", help="Read saved C++ stdout instead of running C++")
    parser.add_argument("--min-years", type=int, default=3)
    parser.add_argument("--start", default="2000-01-01")
    parser.add_argument("--end")
    parser.add_argument("--refresh", action="store_true")
    parser.add_argument("--output")
    parser.add_argument("--no-open", action="store_true")
    args = parser.parse_args()
    root = Path(__file__).resolve().parent
    if args.min_years < 1:
        parser.error("--min-years must be positive")
    if args.csv:
        price_csv = Path(args.csv).resolve()
        ticker = price_csv.stem
        command = [str(root / "season"), str(price_csv), str(args.min_years)]
    else:
        ticker = args.ticker.upper()
        key = args.start + "_" + (args.end or "latest")
        key = "".join(c if c.isalnum() or c in "_.-" else "_" for c in key)
        price_csv = root / "data" / f"{ticker}_{key}.csv"
        command = ["bash", str(root / "start.sh"), ticker, str(args.min_years),
                   "--start", args.start]
        if args.end:
            command.extend(["--end", args.end])
        if args.refresh:
            command.append("--refresh")
    if args.results:
        stdout = Path(args.results).read_text(encoding="utf-8")
    else:
        completed = subprocess.run(command, cwd=root, capture_output=True, text=True,
                                   check=False)
        if completed.returncode:
            raise RuntimeError((completed.stderr + "\n" + completed.stdout).strip())
        stdout = completed.stdout
    results = parse_results(stdout)
    prices = load_prices(price_csv)
    outfile = Path(args.output).resolve() if args.output else root / "reports" / (ticker + "-seasonality.html")
    report = render(prices, results, outfile)
    print("Dashboard created:", report)
    if not args.no_open:
        webbrowser.open(report.resolve().as_uri())


if __name__ == "__main__":
    main()
