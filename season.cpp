#include "season.h"

#include <algorithm>
#include <cerrno>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <string>

namespace
{
bool is_valid_date(unsigned year, unsigned month, unsigned day)
{
    static const unsigned days_in_month[12] = {
        31U, 28U, 31U, 30U, 31U, 30U,
        31U, 31U, 30U, 31U, 30U, 31U
    };
    if (year < 1900U || month < 1U || month > 12U || day < 1U)
    {
        return false;
    }
    unsigned limit = days_in_month[month - 1U];
    if (month == 2U && (year % 4U == 0U) &&
        ((year % 100U != 0U) || (year % 400U == 0U)))
    {
        limit = 29U;
    }
    return day <= limit;
}

bool parse_price(const std::string &text, double *value)
{
    if (text.empty())
    {
        return false;
    }
    char *end = nullptr;
    errno = 0;
    const double parsed = std::strtod(text.c_str(), &end);
    if (errno == ERANGE || end == text.c_str() || *end != '\0' ||
        !std::isfinite(parsed) || parsed <= 0.0)
    {
        return false;
    }
    *value = parsed;
    return true;
}

bool parse_volume(const std::string &text, std::uint64_t *value)
{
    if (text.empty() || text[0] == '-')
    {
        return false;
    }
    char *end = nullptr;
    errno = 0;
    const unsigned long long parsed = std::strtoull(text.c_str(), &end, 10);
    if (errno == ERANGE || end == text.c_str() || *end != '\0')
    {
        return false;
    }
    *value = static_cast<std::uint64_t>(parsed);
    return true;
}

bool parse_row(const std::string &line, Day *result)
{
    const std::size_t first = line.find(',');
    if (first != 10U)
    {
        return false;
    }
    const std::size_t second = line.find(',', first + 1U);
    if (second == std::string::npos ||
        line.find(',', second + 1U) != std::string::npos)
    {
        return false;
    }
    const std::string date = line.substr(0U, first);
    if (date[4] != '-' || date[7] != '-')
    {
        return false;
    }

    unsigned year = 0U;
    unsigned month = 0U;
    unsigned day = 0U;
    int consumed = 0;
    if (std::sscanf(date.c_str(), "%4u-%2u-%2u%n",
                    &year, &month, &day, &consumed) != 3 ||
        consumed != 10 || !is_valid_date(year, month, day))
    {
        return false;
    }
    double price = 0.0;
    std::uint64_t volume = 0U;
    if (!parse_price(line.substr(first + 1U, second - first - 1U), &price) ||
        !parse_volume(line.substr(second + 1U), &volume))
    {
        return false;
    }
    result->year = year;
    result->month = month;
    result->day = day;
    result->price = price;
    result->volume = volume;
    return true;
}

bool date_less(const Day &left, const Day &right)
{
    if (left.year != right.year)
    {
        return left.year < right.year;
    }
    if (left.month != right.month)
    {
        return left.month < right.month;
    }
    return left.day < right.day;
}
} /* namespace */

bool load_price_csv(const std::string &filename, Day_List *result,
                    std::string *error)
{
    if (result == nullptr || error == nullptr)
    {
        return false;
    }
    result->days.clear();
    error->clear();

    std::ifstream input(filename);
    if (!input)
    {
        *error = "cannot open file: " + filename;
        return false;
    }
    std::string line;
    if (!std::getline(input, line))
    {
        *error = "empty CSV file";
        return false;
    }
    if (!line.empty() && line.back() == '\r')
    {
        line.pop_back();
    }
    if (line != "Date,Close,Volume")
    {
        *error = "expected CSV header Date,Close,Volume";
        return false;
    }

    unsigned line_number = 1U;
    while (std::getline(input, line))
    {
        ++line_number;
        if (!line.empty() && line.back() == '\r')
        {
            line.pop_back();
        }
        if (line.empty())
        {
            continue;
        }
        Day record = {};
        if (!parse_row(line, &record))
        {
            *error = "invalid CSV line " + std::to_string(line_number);
            return false;
        }
        result->days.push_back(record);
    }
    if (input.bad())
    {
        *error = "I/O error reading CSV";
        return false;
    }
    if (result->days.empty())
    {
        *error = "CSV has no data rows";
        return false;
    }

    std::sort(result->days.begin(), result->days.end(), date_less);
    for (std::size_t i = 1U; i < result->days.size(); ++i)
    {
        const Day &a = result->days[i - 1U];
        const Day &b = result->days[i];
        if (a.year == b.year && a.month == b.month && a.day == b.day)
        {
            *error = "duplicate trading date";
            return false;
        }
    }
    return true;
}
