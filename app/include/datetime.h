#ifndef INCLUDE_DATETIME_H_
#define INCLUDE_DATETIME_H_

struct tm
{
	int tm_sec;
	int tm_min;
	int tm_hour;
	int tm_mday;
	int tm_mon;
	int tm_year;
	int tm_wday;
	int tm_yday;
	int tm_isdst;
	int valid;
};

extern struct tm curTime;

int epochToTm(long long t, struct tm *tm);
int epochToWeekday(long long t);
int epochToHours(long long t);


#endif /* INCLUDE_DATETIME_H_ */
