# Stock Seasonality

A **C-style C++17** scanner for historical stock-price seasonality, with a Python
`yfinance` downloader and a standalone interactive **Plotly** HTML dashboard.

The C++ engine compares every earlier/later trading-date pair **within a calendar
year**, groups identical month/day pairs across years, and reports the best and
worst mean adjusted-price returns. These are *in-sample* comparisons and are
not forecasts or investment advice.

## Quick start (Fedora)

```bash
sudo dnf install gcc-c++ python3 python3-pip
git clone https://github.com/dibend/seasonality.git
cd seasonality
bash compile.sh
bash start.sh IBM
bash visualize.sh IBM
```

`compile.sh` builds the C++ binary (`season`), creates an isolated `.venv`,
and installs Python dependencies. `comp.sh` only builds the C++ executable;
use `bash comp.sh` after C++-only changes.

`start.sh` downloads and caches adjusted daily closes in `data/`, then runs
the compiled analyzer. `visualize.sh` additionally builds an interactive
standalone HTML dashboard in `reports/` and opens it in your browser.

Scripts are invoked with `bash` so they work even without executable file bits.

## Examples

```bash
bash start.sh SPY 5 --start 2005-01-01
bash start.sh AAPL 3 --refresh
bash visualize.sh SPY --min-years 5 --start 2005-01-01
bash visualize.sh IBM --no-open
bash visualize.sh --csv samples/DEMO.csv --no-open
bash start.sh --csv samples/DEMO.csv 3
```

The final two commands use bundled offline sample data and require no Yahoo
download. If the browser fails to launch, open an HTML file from `reports/`
manually in Firefox.

Run deterministic checks:

```bash
bash tests/test_core.sh
python3 -m unittest discover -s tests -p 'test_python.py'
```

GitHub Actions runs C++ and offline Python tests on pushes and pull requests.

## Files

| File | Purpose |
| --- | --- |
| `main.cpp`, `season.cpp`, `season.h` | C++ date-pair scanner and strict CSV reader |
| `comp.sh` | Compile C++ only |
| `compile.sh` | Build C++ and prepare Python environment |
| `start.sh` | Fetch/cache data and run C++ analysis |
| `get_quotes.py`, `get_quotes.sh` | Download and normalize yfinance data |
| `visualize.py`, `visualize.sh` | Parse C++ output and generate Plotly HTML |
| `requirements*.txt` | Python dependencies |
| `samples/DEMO.csv` | Offline demonstration fixture |
| `tests/` | C++ and Python regression checks |
| `STYLE.md` | NASA/JPL-inspired C-style C++ conventions |

## Data format

Python fetches daily data using `yfinance.download(..., auto_adjust=True)`
and exports this exact, intentionally simple schema:

```csv
Date,Close,Volume
2020-01-02,100.0,1000
2020-02-03,110.0,1200
```

Date uses `YYYY-MM-DD`; Close is a finite positive adjusted price; Volume
is a nonnegative integer. The C++ reader checks the schema, dates, values,
duplicate dates and input errors. CSV records are sorted chronologically.

## C++ result format

```csv
signal,ticker,buy_date,sell_date,avg_return_pct,years,win_rate_pct
```

- `long`: largest historical mean holding-period return.
- `worst_period`: smallest historical mean *long* holding-period return.
  This is **not** a short-selling return.
- `years`: number of observed years containing both trading dates.
- `win_rate_pct`: proportion of observations with positive long returns.
- `min_years`: optional minimum observations per date pair (default 3).

Only exact trading dates are matched. Dates absent due to weekends/holidays
do not contribute an observation for that year and pair.

## Dashboard

The Plotly HTML includes historical adjusted closes, a normalized average
seasonal-year curve, selected best/worst periods and their annual returns.
The generated HTML embeds Plotly and works offline once generated.

To use an existing input file and previously captured result output:

```bash
bash start.sh --csv samples/DEMO.csv > results.txt
bash visualize.sh --csv samples/DEMO.csv --results results.txt --no-open
```

## Limitations and research ethics

Exhaustively choosing the best date pair introduces **multiple-testing bias**.
Different pairs may have different sample sizes. Dividends/corporate-action
adjustments depend on the data provider. Neither the scanner nor charts model
slippage, transaction costs, taxes, borrow availability or short-sale expenses.

Validate any proposed strategy on unseen historical periods and realistic
execution assumptions before drawing predictive conclusions.

The C++ style is **NASA/JPL-inspired**, not officially certified. Licensed
under GPL-3.0; see [LICENSE](LICENSE).
