# seasonality
Try to find some seasonality in stock prices from yahoo finance historical csv data.

This program looks at each pair of days every year and determines which would've had the best performance in attempt to find seasonal patterns. 

compile with `./comp.sh`

get a file for testing with `./get_quotes.sh IBM` or whichever ticker you choose

run with `./season IBM.csv`

/*--------------------------------------------------------------------*/
GITHUB C0PIL0T >

Yes — this program is a very simple stock-seasonality finder. I looked at the repo at https://github.com/dibend/seasonality.

In plain English, it does this:

- Reads a Yahoo Finance historical CSV file for a ticker like IBM
- Parses each row into:
  - year
  - month
  - day
  - price
- For each year, it looks at pairs of dates inside that same year
- For every pair, it computes a return:
  - return = (later_price - earlier_price) / earlier_price
- It stores all those returns for each date-pair pattern, such as:
  - 3/15 to 11/30
  - 1/2 to 5/18
- Then it averages those returns across many years
- It picks:
  - the best-performing pair as the “long” signal
  - the worst-performing pair as the “short” signal
- Prints results like:
  - long,IBM,4/15,12/20,0.123456
  - short,IBM,8/01,2/10,0.089012

What the C++ code is doing

- season.h defines the data structures:
  - Day contains date + volume + price
  - Day_List holds an array of Day records

- season.cpp does the CSV parsing:
  - opens the file
  - mmaps it into memory
  - scans the CSV line by line
  - extracts:
    - year from the date field
    - month and day from the date field
    - closing price from the right column
  - builds an array of Day records

- main.cpp is the actual strategy:
  - loops through all days in reverse order
  - for each year, compares each day to earlier days in the same year
  - creates a key like:
    - "3/15,11/30"
  - stores all returns for that pair
  - computes the average return for each pair
  - chooses the max and min average return
  - prints them as long/short trade signals

The core idea is:
- “Maybe some calendar dates consistently outperform others across years.”
- It’s testing calendar seasonality, not a sophisticated predictive model.

Example of the logic in main.cpp

It builds a map keyed by pair:
- month/day for the later date + month/day for the earlier date

Then it does:
- return = (price_earlier - price_later) / price_earlier
- actually the code uses:
  - (Days[j].price - Days[i].price) / Days[i].price
- and stores this in a list for that pair
- average return over all years for that pair

Then it selects the largest and smallest average return.

Important caveat

This is a toy/experimental project, not a production-quality trading system. It is:
- very simple
- based on a raw CSV parser
- uses a fixed column assumption
- not robust to all file variations
- not statistically validated

How to run it

From the repo:

- compile:
  - ./comp.sh
- download sample quote data:
  - ./get_quotes.sh IBM
- run:
  - ./season IBM.csv

If you want, I can also explain the code line by line in a simpler “how each loop works” format, or show the exact logic used to compute the “best seasonal pair.”
