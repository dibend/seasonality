#include "season.h"

using namespace std;

struct Date {
	uint32_t day;
	uint32_t month;
};

struct Trades {
	double average_return;
	list<double> returns;
};

int main(int argc, char *argv[]) {
	struct Day_List dList;
	dList = yh_csv_file_to_Day_List(argv[1]);

	string best, worst;
	map<string, struct Trades> pairs;
	int num_pairs = 0;
	for(int i = dList.size-1; i > 0; i--) {
		int year = dList.Days[i].year;
		int j = i-1;
		double change;


		while(dList.Days[j].year == year && j > 0) {
			string dp = to_string(dList.Days[i].month) + "/" + to_string(dList.Days[i].day) + "," + to_string(dList.Days[j].month) + "/" + to_string(dList.Days[j].day);
			change = (dList.Days[j].price - dList.Days[i].price) / dList.Days[i].price;
			pairs[dp].returns.push_back(change);
			j--;			
		}
	}

	map<string, struct Trades>::iterator it = pairs.begin();
	while(it != pairs.end())
	{
		string dp = it->first;
		double av = 0;
		list<double>::const_iterator iterator;
		for (iterator = pairs[dp].returns.begin(); iterator != pairs[dp].returns.end(); ++iterator) {
			av += *iterator;
		}
		av /= pairs[dp].returns.size();
		pairs[dp].average_return = av;

		if(pairs[dp].average_return > pairs[best].average_return) {
			best = dp;
		} else if(pairs[dp].average_return < pairs[worst].average_return) {
			worst = dp;
		}
		it++;
	}

	int ticker_start;
	for(ticker_start = strlen(argv[1]); ticker_start > 0; ticker_start--) {
		if(argv[1][ticker_start] == '/') {
			ticker_start++;
			break;
		}
	}

	char *ticker = &argv[1][ticker_start];
	ticker[strlen(ticker)-4] = 0;
	printf("long,%s,%s,%f\n", ticker, best.c_str(), pairs[best].average_return);
	printf("short,%s,%s,%f\n", ticker, worst.c_str(), pairs[worst].average_return * -1);
}
