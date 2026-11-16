#!/usr/bin/env python3
"""Offline integration tests for CSV conversion and result parsing."""
import csv
import io
import tempfile
import unittest
from pathlib import Path

from visualize import annual_returns, load_prices, parse_results, seasonal_curve


class PythonIntegrationTests(unittest.TestCase):
    def test_parse_and_annual_returns(self):
        text = ("signal,ticker,buy_date,sell_date,avg_return_pct,years,win_rate_pct\n"
                "long,DEMO,1/2,2/3,10.0000,3,100.0000\n"
                "worst_period,DEMO,2/3,3/4,-18.1818,3,0.0000\n")
        rows = parse_results(text)
        self.assertEqual(len(rows), 2)
        prices = load_prices(Path(__file__).resolve().parents[1] / "samples" / "DEMO.csv")
        yearly = annual_returns(prices, rows[0]["buy_date_pair"], rows[0]["sell_date_pair"])
        self.assertEqual(len(yearly), 3)
        self.assertAlmostEqual(yearly[0][1], 10)
        self.assertEqual(len(seasonal_curve(prices)[0]), 3)


if __name__ == "__main__":
    unittest.main()
