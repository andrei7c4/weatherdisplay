#include <os_type.h>
#include <osapi.h>
#include <user_interface.h>
#include <math.h>	// for fabs
#include "retain.h"
#include "conv.h"
#include "common.h"
#include "debug.h"


Retain retain;

#define VALID_MAGIC_NUMBER			0xAABBCCDD
#define RTC_USER_DATA_ADDR			64
#define RTC_WEATHER_ADDR			(RTC_USER_DATA_ADDR + sizeof(Retain)/4)

LOCAL int weatherEqual(uint *rtcMemBlock, CurWeather *weather);
LOCAL int forecastsEqual(uint *rtcMemBlock, Forecast *forecast, ForecastType type, int count);


/**
 * dst: destination block number
 * src: pointer to data to be written (must be aligned to 4 bytes)
 * size: data size in bytes (must be multiple of 4)
 * returns next block number after written data
 */
LOCAL uint systemRtcMemWrite(uint dst, uint *src, uint size)
{
	system_rtc_mem_write(dst, src, size);
	return dst + size/4;
}

/**
 * src: source block number
 * dst: pointer to data buffer (must be aligned to 4 bytes)
 * size: data size in bytes (must be multiple of 4)
 * returns next block number after read data
 */
LOCAL uint systemRtcMemRead(uint src, uint *dst, uint size)
{
	system_rtc_mem_read(src, dst, size);
	return src + size/4;
}


void retainInit(Retain *retain)
{
	os_memset(retain, 0, sizeof(Retain));
	retain->magic = VALID_MAGIC_NUMBER;
}

void retainRead(Retain *retain)
{
	systemRtcMemRead(RTC_USER_DATA_ADDR, (uint*)retain, sizeof(Retain));
	if (retain->magic != VALID_MAGIC_NUMBER)
	{
		os_printf("no valid retain\n");
		retainInit(retain);
		retainWrite(retain);
	}
	else
	{
		os_printf("valid retain found\n");
	}
}

void retainWrite(Retain *retain)
{
	systemRtcMemWrite(RTC_USER_DATA_ADDR, (uint*)retain, sizeof(Retain));
}


/**
 * Saves weather, forecast and indoor temperature to RTC memory.
 * Data is preserved during deep sleep, but not when battery is unplugged.
 */
void ICACHE_FLASH_ATTR retainWeather(CurWeather *curWeather,
		Forecast *hourly, int hourlyCount,
		Forecast *daily, int dailyCount,
		float indoorTemp)
{
	debug("retainWeather hourlyCount %d, dailyCount %d\n", hourlyCount, dailyCount);
	if (hourlyCount < 0) hourlyCount = 0;
	if (dailyCount < 0) dailyCount = 0;

	// save current weather
	uint dst = RTC_WEATHER_ADDR;
	dst = systemRtcMemWrite(dst, (uint*)curWeather, sizeof(CurWeather));

	// save hourly forecast
	dst = systemRtcMemWrite(dst, &hourlyCount, sizeof(hourlyCount));
	dst = systemRtcMemWrite(dst, (uint*)hourly, sizeof(Forecast)*hourlyCount);

	// save daily forecast
	dst = systemRtcMemWrite(dst, &dailyCount, sizeof(dailyCount));
	dst = systemRtcMemWrite(dst, (uint*)daily, sizeof(Forecast)*dailyCount);

	// save indoor temp
	systemRtcMemWrite(dst, (uint*)&indoorTemp, sizeof(indoorTemp));
}

void ICACHE_FLASH_ATTR retainWeatherRead(CurWeather *curWeather,
		Forecast *hourly, int *hourlyCount,
		Forecast *daily, int *dailyCount)
{
	uint src = RTC_WEATHER_ADDR;
	src = systemRtcMemRead(src, (uint*)curWeather, sizeof(CurWeather));
	src = systemRtcMemRead(src, (uint*)hourlyCount, sizeof(int));
	if (*hourlyCount > 0)
	{
		src = systemRtcMemRead(src, (uint*)hourly, *hourlyCount * sizeof(Forecast));
	}
	src = systemRtcMemRead(src, (uint*)dailyCount, sizeof(int));
	if (*dailyCount > 0)
	{
		systemRtcMemRead(src, (uint*)daily, *dailyCount * sizeof(Forecast));
	}
}

/**
 * Returns true if weather, forecast and indoor temperature
 * are equal or close enough to the data saved in RTC memory.
 */
int ICACHE_FLASH_ATTR retainedWeatherEqual(CurWeather *curWeather,
		Forecast *hourly, int hourlyCount,
		Forecast *daily, int dailyCount,
		float indoorTemp)
{
	if (hourlyCount < 0) hourlyCount = 0;
	if (dailyCount < 0) dailyCount = 0;

	// compare current weather
	uint rtcMemBlock = RTC_WEATHER_ADDR;
	if (!weatherEqual(&rtcMemBlock, curWeather))
	{
		return FALSE;
	}

	// compare hourly forecast
	if (!forecastsEqual(&rtcMemBlock, hourly, eHourlyChart, hourlyCount))
	{
		return FALSE;
	}

	// compare daily forecast
	if (!forecastsEqual(&rtcMemBlock, daily, eDailyChart, dailyCount))
	{
		return FALSE;
	}

	// compare indoor temp
	float temp;
	systemRtcMemRead(rtcMemBlock, (uint*)&temp, sizeof(float));
	if (fabs(temp - indoorTemp) > 0.5)
	{
		return FALSE;
	}

	return TRUE;
}


LOCAL int ICACHE_FLASH_ATTR tempsEqual(float t1, float t2)
{
	return floatToIntRound(kelvinToTemp(t1)) == floatToIntRound(kelvinToTemp(t2));
}

LOCAL int ICACHE_FLASH_ATTR weatherEqual(uint *rtcMemBlock, CurWeather *weather)
{
	CurWeather retainedWeather;

	*rtcMemBlock = systemRtcMemRead(*rtcMemBlock, (uint*)&retainedWeather, sizeof(CurWeather));
	if (!tempsEqual(weather->temperature, retainedWeather.temperature))
	{
		return FALSE;
	}
	if (weather->icon.val != retainedWeather.icon.val)
	{
		return FALSE;
	}
	if ((config.chart == eNoChart || config.chart == eDailyChart) &&
		os_strcmp(weather->description, retainedWeather.description))
	{
		return FALSE;
	}
	return TRUE;
}

LOCAL int ICACHE_FLASH_ATTR forecastEqual(Forecast *f1, Forecast *f2, ForecastType type)
{
	if (f1->datetime != f2->datetime ||
		f1->icon.val != f2->icon.val)
	{
		return FALSE;
	}
	switch (type)
	{
	case eHourlyChart:
	    if (!tempsEqual(f1->value.temp / FLOAT_SCALE, f2->value.temp / FLOAT_SCALE) ||
	       fabs((f1->value.rainsnow / FLOAT_SCALE) - (f2->value.rainsnow / FLOAT_SCALE)) > 0.2)
	    {
	        return FALSE;
	    }
	    break;
	case eDailyChart:
        if (!tempsEqual(f1->value.tempMin / FLOAT_SCALE, f2->value.tempMin / FLOAT_SCALE) ||
            !tempsEqual(f1->value.tempMax / FLOAT_SCALE, f2->value.tempMax / FLOAT_SCALE))
        {
            return FALSE;
        }
        break;
	}
	return TRUE;
}

LOCAL int ICACHE_FLASH_ATTR forecastsEqual(uint *rtcMemBlock, Forecast *forecast, ForecastType type, int count)
{
	Forecast retainedForecast;
	uint retainedCount;
	int i;

	*rtcMemBlock = systemRtcMemRead(*rtcMemBlock, (uint*)&retainedCount, sizeof(int));
	if (retainedCount < count)
			return FALSE;

	for (i = 0; i < count; i++)
	{
		*rtcMemBlock = systemRtcMemRead(*rtcMemBlock, (uint*)&retainedForecast, sizeof(Forecast));
		if (!forecastEqual(&forecast[i], &retainedForecast, type))
			return FALSE;
	}
	*rtcMemBlock += (retainedCount-i) * sizeof(Forecast)/4;
	return TRUE;
}
