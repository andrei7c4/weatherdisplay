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
};

int epochToTm(long long t, struct tm *tm);



#endif /* INCLUDE_DATETIME_H_ */
