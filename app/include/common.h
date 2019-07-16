#ifndef INCLUDE_COMMON_H_
#define INCLUDE_COMMON_H_

#include <stdint.h>
#include "typedefs.h"

#define NELEMENTS(array) (sizeof (array) / sizeof ((array) [0]))

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))


typedef	union
{
	char str[4];
	uint val;
}IconId;

typedef struct CurWeather_t
{
	uint datetime;
	float temperature;
	char description[32];
	IconId icon;
}CurWeather;

typedef struct Forecast_t
{
	uint datetime;
	union
	{
	    struct
	    {
	        uint16_t temp;
	        uint16_t rainsnow;
	    };
		struct
		{
	        uint16_t tempMin;
	        uint16_t tempMax;
		};
	}value;
	IconId icon;
}Forecast;

#define FLOAT_SCALE     100.0

typedef enum
{
	eNoChart=0,
	eHourlyChart,
	eDailyChart,
	eBothCharts
}ForecastType;

#define FORECAST_HOURLY_SIZE		32
#define FORECAST_DAILY_SIZE			((FORECAST_HOURLY_SIZE / 8) + 1)


#endif /* INCLUDE_COMMON_H_ */
