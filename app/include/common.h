#ifndef INCLUDE_COMMON_H_
#define INCLUDE_COMMON_H_

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
		float val;
		struct
		{
			float min;
			float max;
		};
	}temp;
	float rainsnow;
	IconId icon;
}Forecast;

typedef enum
{
	eNoChart=0,
	eHourlyChart,
	eDailyChart,
	eBothCharts
}ForecastType;

#define FORECAST_HOURLY_SIZE		40
#define FORECAST_HOURLY_CHART_CNT	9
#define FORECAST_DAILY_SIZE			6


#endif /* INCLUDE_COMMON_H_ */
