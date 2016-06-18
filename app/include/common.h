#ifndef INCLUDE_COMMON_H_
#define INCLUDE_COMMON_H_

#include "typedefs.h"

typedef struct CurWeather_t
{
	uint datetime;
	float temperature;
	char description[32];
	const uint *icon;
}CurWeather;

typedef struct Forecast_t
{
	uint datetime;
	float minTemp;
	float maxTemp;
	const uint *icon;
}Forecast;

#define FORECAST_DAYS	5


#endif /* INCLUDE_COMMON_H_ */
