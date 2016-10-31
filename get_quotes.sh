#!/bin/bash
ticker=$1
curl -s "http://ichart.finance.yahoo.com/table.csv?s=${ticker}&d=0&e=28&f=2020&g=d&a=3&b=12&c=1800&ignore=.csv" > ${ticker}.csv
