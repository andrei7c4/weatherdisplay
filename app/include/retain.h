#ifndef INCLUDE_RETAIN_H_
#define INCLUDE_RETAIN_H_

#include <ip_addr.h>
#include "typedefs.h"

typedef struct{
	uint magic;
	struct ip_info ipConfig;
	ip_addr_t dns1, dns2;
	uint attempts;
	uint fails;
	uint updates;
	uint retry;
	uint longSleepCnt;
	char cityId[8];
}Retain;
extern Retain retain;

void retainInit(Retain *retain);
void retainRead(Retain *retain);
void retainWrite(Retain *retain);

typedef struct CurWeather_t CurWeather;
typedef struct Forecast_t Forecast;
void retainWeather(CurWeather *curWeather,
		Forecast *hourly, int hourlyCount,
		Forecast *daily, int dailyCount,
		float indoorTemp);
void retainWeatherRead(CurWeather *curWeather,
		Forecast *hourly, int *hourlyCount,
		Forecast *daily, int *dailyCount);
int retainedWeatherEqual(CurWeather *curWeather,
		Forecast *hourly, int hourlyCount,
		Forecast *daily, int dailyCount,
		float indoorTemp);


#endif /* INCLUDE_RETAIN_H_ */
