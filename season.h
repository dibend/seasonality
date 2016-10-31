#ifndef SSN_H_
#define SSN_H_

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>
#include <stdint.h>

#include <map>
#include <list>
#include <string>

#ifdef __APPLE__
    #include <sys/uio.h>
#else
    #include <sys/io.h>
#endif

struct Day {
    uint32_t day;
    uint32_t month;
    uint32_t year;
    uint32_t volume;
    double price;
};

struct Day_List {
    uint32_t size;
    struct Day *Days;
};

struct Day_List yh_csv_file_to_Day_List(char *filename);

#endif // SSN_H_
