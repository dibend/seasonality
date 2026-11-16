# Stock Seasonality — C-style C++17

An exploratory stock-seasonality scanner. Searches earlier/later trading-day pairs
within each calendar year and ranks their **average adjusted-price returns**
across years. This is historical analysis, **not** a trading recommendation.

## Compile

```sh
./comp.sh
```

## Prepare data

The program accepts normalized daily CSV data with an exact header and an ISO date:

```csv
Date,Close,Volume
2020-01-02,100.0,1000
2020-02-03,110.0,1200
```

Use corporate-action-adjusted closing prices if comparing returns over time.
The original `get_quotes.sh` is obsolete; use the yfinance converter from
the companion local project until the Python downloader is integrated.

## Run

```sh
./season IBM.csv 3
```

The optional number is the minimum number of years observed for a calendar pair.

CSV output columns:
`signal,ticker,buy_date,sell_date,avg_return_pct,years,win_rate_pct`.

`worst_period` denotes the lowest mean **long** return. It is *not* a
calculated short-sale profit. Transactions, dividend-accounting nuances, and
execution costs are not modeled. Selecting extreme historical averages
without out-of-sample validation causes substantial overfitting.

See [STYLE.md](STYLE.md) for code conventions. GPL-3.0 licensing retained.
