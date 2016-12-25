#ifndef INCLUDE_COMMON_H_
#define INCLUDE_COMMON_H_

#include "typedefs.h"

typedef struct CurWeather_t
{
	uint datetime;
	float temperature;
	char description[32];
	//const uint *icon;
	char icon[4];
}CurWeather;

typedef struct Forecast_t
{
	uint datetime;
	float temp;
	float minTemp;
	float maxTemp;
	float rainsnow;
	//const uint *icon;
	char icon[4];
}Forecast;

typedef enum {eForecastHourly=1, eForecastDaily} ForecastType;

#define FORECAST_HOURLY_CNT	9
#define FORECAST_DAILY_CNT	6

#endif /* INCLUDE_COMMON_H_ */
