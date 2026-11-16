#include "season.h"

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <map>
#include <string>
#include <utility>

namespace
{
typedef std::pair<unsigned, unsigned> Month_Day;
typedef std::pair<Month_Day, Month_Day> Date_Pair;

struct Trade_Stats
{
    double sum = 0.0;
    unsigned count = 0U;
    unsigned wins = 0U;
};

bool parse_min_years(const char *text, unsigned *result)
{
    if (text == nullptr || text[0] == '\0' || text[0] == '-')
    {
        return false;
    }
    char *end = nullptr;
    errno = 0;
    const unsigned long value = std::strtoul(text, &end, 10);
    if (errno == ERANGE || *end != '\0' || value == 0UL ||
        value > std::numeric_limits<unsigned>::max())
    {
        return false;
    }
    *result = static_cast<unsigned>(value);
    return true;
}

void accumulate_pairs(const Day_List &list,
                      std::map<Date_Pair, Trade_Stats> *pairs)
{
    const std::vector<Day> &days = list.days;
    std::size_t first = 0U;
    while (first < days.size())
    {
        std::size_t end = first + 1U;
        while (end < days.size() && days[end].year == days[first].year)
        {
            ++end;
        }
        for (std::size_t buy = first; buy < end; ++buy)
        {
            for (std::size_t sell = buy + 1U; sell < end; ++sell)
            {
                const Date_Pair key = {
                    {days[buy].month, days[buy].day},
                    {days[sell].month, days[sell].day}
                };
                const double change = days[sell].price / days[buy].price - 1.0;
                Trade_Stats &stats = (*pairs)[key];
                stats.sum += change;
                ++stats.count;
                if (change > 0.0)
                {
                    ++stats.wins;
                }
            }
        }
        first = end;
    }
}

std::string ticker_from_path(const std::string &path)
{
    const std::size_t slash = path.find_last_of("/\\");
    std::string ticker = path.substr(
        slash == std::string::npos ? 0U : slash + 1U);
    if (ticker.size() > 4U &&
        ticker.substr(ticker.size() - 4U) == ".csv")
    {
        ticker.resize(ticker.size() - 4U);
    }
    return ticker;
}

void print_result(const char *signal, const std::string &ticker,
                  const Date_Pair &pair, const Trade_Stats &stats)
{
    const double average = 100.0 * stats.sum / stats.count;
    const double win_rate = 100.0 * stats.wins / stats.count;
    std::printf("%s,%s,%u/%u,%u/%u,%.4f,%u,%.4f\n",
                signal, ticker.c_str(),
                pair.first.first, pair.first.second,
                pair.second.first, pair.second.second,
                average, stats.count, win_rate);
}
} /* namespace */

int main(int argc, char *argv[])
{
    if (argc < 2 || argc > 3)
    {
        std::fprintf(stderr, "Usage: %s TICKER.csv [min_years=3]\n", argv[0]);
        return 2;
    }

    unsigned min_years = 3U;
    if (argc == 3 && !parse_min_years(argv[2], &min_years))
    {
        std::fprintf(stderr, "min_years must be a positive integer\n");
        return 2;
    }

    Day_List list;
    std::string error;
    if (!load_price_csv(argv[1], &list, &error))
    {
        std::fprintf(stderr, "Error: %s\n", error.c_str());
        return 1;
    }

    std::map<Date_Pair, Trade_Stats> pairs;
    accumulate_pairs(list, &pairs);

    bool found = false;
    Date_Pair best = {};
    Date_Pair worst = {};
    double best_average = -std::numeric_limits<double>::infinity();
    double worst_average = std::numeric_limits<double>::infinity();

    for (const auto &entry : pairs)
    {
        const Trade_Stats &stats = entry.second;
        if (stats.count < min_years)
        {
            continue;
        }
        const double average = stats.sum / stats.count;
        if (!found || average > best_average)
        {
            best = entry.first;
            best_average = average;
        }
        if (!found || average < worst_average)
        {
            worst = entry.first;
            worst_average = average;
        }
        found = true;
    }
    if (!found)
    {
        std::fprintf(stderr, "No date pairs with %u observations\n", min_years);
        return 1;
    }

    const std::string ticker = ticker_from_path(argv[1]);
    std::puts("signal,ticker,buy_date,sell_date,avg_return_pct,years,win_rate_pct");
    print_result("long", ticker, best, pairs.at(best));
    print_result("worst_period", ticker, worst, pairs.at(worst));
    return 0;
}
