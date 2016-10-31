#include "season.h"

struct Day_List yh_csv_file_to_Day_List(char *filename)
{
	char *f, *element;
	int num_days, day_index, pos, column, row, timestamp, mkt_open;
	struct Day_List My_Day_List;    
	struct stat s;
	size_t size;
	int fd = open (filename, O_RDONLY);
	int start = 42; //start of file (excluding titles)

	/* Get the size of the file. */
	fstat (fd, & s);
	size = s.st_size;

	f = (char *) mmap (0, size, PROT_READ, MAP_PRIVATE, fd, 0);
	close(fd);

	num_days = 0;
	for(int i = start; i < size; i++) {
		if(f[i] == '\n') {
			num_days ++; 
		}
	}
	My_Day_List.Days = (struct Day *) malloc(num_days * sizeof(struct Day)); 

	pos = start;
	column = 0;
	row = 0;
	mkt_open = 0;
	day_index = 0;
	for(int i = start; i < size; i++) {
		if(f[i] == ',' || f[i] == '\n') {
			if(column == 0) {
				int el_size = i-pos+1;
				char year[5]; //yyyy
				char month[3]; //mm
				char day[3]; //dd
				element = (char *) malloc(el_size);
				memcpy(element, &f[pos], el_size);
				element[el_size-1] = 0; //null terminate
				//hardcoded date locations
				memcpy(year, &element[0], 4);
				year[4] = 0; //null terminate
				memcpy(month, &element[5], 2);
				month[2] = 0; //null terminate
				memcpy(day, &element[8], 2);
				month[2] = 0; //null terminate
				My_Day_List.Days[day_index].year = atoi(year);
				My_Day_List.Days[day_index].month = atoi(month);
				My_Day_List.Days[day_index].day = atoi(day);
				free(element);
			} else if(column == 6) {
				int el_size = i-pos+1;
				element = (char *) malloc(el_size);
				memcpy(element, &f[pos], el_size);
				element[el_size-1] = 0; //null terminate
				My_Day_List.Days[day_index].price = atof(element);
				free(element);
			}
			pos = i+1;
			column ++;
		} 
		if(f[i] == '\n') {
			day_index ++;
			pos = i+1;
			row ++;
			column = 0;
		}
	}
	munmap(f, size);
	My_Day_List.size = day_index;
	return My_Day_List;
}

