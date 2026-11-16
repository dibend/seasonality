#ifndef SEASON_H_
#define SEASON_H_

#include <cstdint>
#include <string>
#include <vector>

struct Day
{
    unsigned year;
    unsigned month;
    unsigned day;
    double price;
    std::uint64_t volume;
};

struct Day_List
{
    std::vector<Day> days;
};

/* Returns false with an error message when input is invalid. */
bool load_price_csv(const std::string &filename, Day_List *result,
                    std::string *error);

#endif /* SEASON_H_ */
