#include <os_type.h>
#include <osapi.h>
#include <mem.h>
#include <float.h>
#include <stddef.h>
#include "gui.h"
#include "common.h"
#include "conv.h"
#include "fonts.h"
#include "icons.h"
#include "display.h"
#include "config.h"
#include "debug.h"


void ICACHE_FLASH_ATTR drawCurrentWeather(CurWeather *curWeather)
{
	const int iconWidth = 256;
	const int iconHeight = 256;
	const int cityX = 16;
	const int cityY = 35;
	const int tempX = 16;
	const int descX = 16;
	const int iconX = DISP_WIDTH-iconWidth-32;
	int iconY = 40;
	int tempY = 100;
	int descY = 320;
	const uint* icon = iconIdToImage(curWeather->icon, 256);

	dispFillMem(0x00);

	if (config.cityDisplayed[0] != '\0')
	{
		int cityWidth = dispStrWidth(arial56, config.cityDisplayed);
		if ((cityX+cityWidth) > (iconX+20))	// small overlap allowed
		{
			iconY = 80;
			descY = 355;
		}
		dispDrawImage(iconX, iconY, icon);
		dispDrawStr(arial56, cityX, cityY, config.cityDisplayed);
	}
	else
	{
		dispDrawImage(iconX, iconY, icon);
		tempY = 35;
	}

	int temp = floatToIntRound(kelvinToTemp(curWeather->temperature));
	if (temp > -100 && temp < 100)	// some sense
	{
		char strbuf[10];
		os_sprintf(strbuf, "%d*", temp);
		dispDrawStr(arial100b, tempX, tempY, strbuf);
	}

	dispDrawStr(arial56, descX, descY, curWeather->description);
}

void ICACHE_FLASH_ATTR drawCurrentWeatherSmall(CurWeather *curWeather)
{
	const int iconWidth = 64;
	const int iconHeight = 64;
	const int iconX = DISP_WIDTH-iconWidth-16;
	const int iconY = 1;
	const int cityX = 16;
	const int cityY = 9;
	const int tempXright = iconX-16;
	const int tempY = 9;

	char tempStr[10] = "\0";
	int tempStrWidth = 0;
	int temp = floatToIntRound(kelvinToTemp(curWeather->temperature));
	if (temp > -100 && temp < 100)	// some sense
	{
		os_sprintf(tempStr, "%d*", temp);
		tempStrWidth = dispStrWidth(arial56b, tempStr);
	}

	dispFillMem(0x00);

	if (config.cityDisplayed[0] != '\0')
	{
		int cityWidth56 = dispStrWidth(arial56, config.cityDisplayed);
		int cityWidthMax = tempXright-tempStrWidth-16-cityX;

		const uint **font = (cityWidth56 <= cityWidthMax) ? arial56 : arial32b;
		dispDrawStr(font, cityX, cityY, config.cityDisplayed);
	}

	if (tempStr[0] != '\0')
	{
		dispDrawStrAlignRight(arial56b, tempXright, tempY, tempStr);
	}

	const uint *image = iconIdToImage(curWeather->icon, 64);
	if (image)
	{
		dispDrawImage(iconX, iconY, image);
	}
}


void ICACHE_FLASH_ATTR drawForecast(Forecast *forecast, int count)
{
	const int iconWidth = 128;
	const int iconX[3] = {16, 176, 336};
	const int textCentre[3] = {iconX[0]+(iconWidth/2),
							   iconX[1]+(iconWidth/2),
							   iconX[2]+(iconWidth/2)};
	const int weekdayY = 30;
	const int iconY = 96;
	const int tempY = 270;
	char strbuf[20] = "";

	dispFillMem(0x00);
	dispDrawLine(0, 0, DISP_WIDTH-1, 0, 1);

	int i, days = MIN(count, 3);
	for (i = 0; i < days; i++)
	{
		switch (i)
		{
		case 0:
			os_strcpy(strbuf, "Today");
			break;
		case 1:
			os_strcpy(strbuf, "Tomorrow");
			break;
		case 2:
			epochToWeekday(forecast[2].datetime, strbuf);
			break;
		}
		dispDrawStrCentred(arial32, textCentre[i], weekdayY, strbuf);
		dispDrawImage(iconX[i], iconY, iconIdToImage(forecast[i].icon, 128));

		int tempMin = floatToIntRound(kelvinToTemp(forecast[i].temp.min));
		int tempMax = floatToIntRound(kelvinToTemp(forecast[i].temp.max));
		if (tempMin > -100 && tempMin < 100 &&
			tempMax > -100 && tempMax < 100)	// some sense
		{
			os_sprintf(strbuf, "%d*...%d*", tempMin, tempMax);
			dispDrawStrCentred(arial32b, textCentre[i], tempY, strbuf);
		}
	}
}

#define CHART_HEIGHT	220
void ICACHE_FLASH_ATTR drawHourlyForecastChart(Forecast *forecast, int count)
{
	drawForecastChart(forecast, eHourlyChart, MIN(FORECAST_HOURLY_CHART_CNT, count),
						0, 80, DISP_WIDTH, CHART_HEIGHT);
}

void ICACHE_FLASH_ATTR drawDailyForecastChart(Forecast *forecast, int count)
{
	dispFillMem(0x00);
	if (count > 1)
	{
		drawForecastChart(forecast+1, eDailyChart, count-1,
							0, 20, DISP_WIDTH, CHART_HEIGHT);
	}
}


void ICACHE_FLASH_ATTR drawIndoorTemp(const char *temp)
{
	dispDrawLine(0, DISP_HEIGHT-92, DISP_WIDTH-1, DISP_HEIGHT-92, 1);
	dispDrawImage(16, DISP_HEIGHT-74, indoorTempIcon64);
	int strWidth = dispDrawStr(segment58, 96, DISP_HEIGHT-51, temp);
	dispDrawChar(segment58, 96+strWidth, DISP_HEIGHT-51, '*');
}

void ICACHE_FLASH_ATTR drawIndoorTempSmall(const char *temp)
{
	dispDrawLine(0, DISP_HEIGHT-45, DISP_WIDTH-1, DISP_HEIGHT-45, 1);
	dispDrawImage(16, DISP_HEIGHT-42, indoorTempIcon40);
	int strWidth = dispDrawStr(segment58, 72, DISP_HEIGHT-41, temp);
	dispDrawChar(segment58, 72+strWidth, DISP_HEIGHT-41, '*');
}

void ICACHE_FLASH_ATTR drawMetaInfo(struct tm *curTime)
{
	char strbuf[20];
	if (curTime)
	{
		strbuf[0] = '@';
		printTime(curTime, strbuf+1);
		dispDrawStrAlignRight(arial16, DISP_WIDTH, DISP_HEIGHT-30, strbuf);
	}
	os_sprintf(strbuf, "fails %u/%u", retain.fails, retain.attempts);
	dispDrawStrAlignRight(arial16, DISP_WIDTH, DISP_HEIGHT-15, strbuf);
}


void ICACHE_FLASH_ATTR printTime(struct tm *tm, char *str)
{
	int am = FALSE;
	if (config.clock24)
	{
		os_sprintf(str, "%2d:%02d", tm->tm_hour, tm->tm_min);
	}
	else
	{
		if (tm->tm_hour == 0)
		{
			tm->tm_hour = 12;
			am = TRUE;
		}
		else if (tm->tm_hour < 12)
		{
			am = TRUE;
		}
		else if (tm->tm_hour > 12)
		{
			tm->tm_hour -= 12;
		}
		os_sprintf(str, "%2d:%02d%s", tm->tm_hour, tm->tm_min, am ? "AM":"PM");
	}
}

void ICACHE_FLASH_ATTR epochToWeekday(uint epoch, char *weekday)
{
	struct tm tm;
	if (epochToTm(epoch, &tm) != OK)
	{
		os_strcpy(weekday,"");
		return;
	}

	switch (tm.tm_wday)
	{
	case 0: os_strcpy(weekday,"Sunday"); break;
	case 1: os_strcpy(weekday,"Monday"); break;
	case 2: os_strcpy(weekday,"Tuesday"); break;
	case 3: os_strcpy(weekday,"Wednesday"); break;
	case 4: os_strcpy(weekday,"Thursday"); break;
	case 5: os_strcpy(weekday,"Friday"); break;
	case 6: os_strcpy(weekday,"Saturday"); break;
	default: os_strcpy(weekday,""); break;
	}
}






typedef struct
{
	IconId id;
	const uint *icon64;
	const uint *icon128;
	const uint *icon256;
}IconIdPair;

LOCAL const IconIdPair IconIds[] = {
	{"01d", icon64_01d, icon128_01d, icon256_01d},
	{"01n", icon64_01n, icon128_01n, icon256_01n},
	{"02d", icon64_02d, icon128_02d, icon256_02d},
	{"02n", icon64_02n, icon128_02n, icon256_02n},
	{"03d", icon64_03d, icon128_03d, icon256_03d},
	{"03n", icon64_03d, icon128_03d, icon256_03d},
	{"04d", icon64_04d, icon128_04d, icon256_04d},
	{"04n", icon64_04d, icon128_04d, icon256_04d},
	{"09d", icon64_09d, icon128_09d, icon256_09d},
	{"09n", icon64_09d, icon128_09d, icon256_09d},
	{"10d", icon64_10d, icon128_10d, icon256_10d},
	{"10n", icon64_10d, icon128_10d, icon256_10d},
	{"11d", icon64_11d, icon128_11d, icon256_11d},
	{"11n", icon64_11d, icon128_11d, icon256_11d},
	{"13d", icon64_13d, icon128_13d, icon256_13d},
	{"13n", icon64_13d, icon128_13d, icon256_13d},
	{"50d", icon64_50d, icon128_50d, icon256_50d},
	{"50n", icon64_50n, icon128_50n, icon256_50n}
};


const uint* ICACHE_FLASH_ATTR iconIdToImage(IconId id, int size)
{
	int i;
	for (i = 0; i < NELEMENTS(IconIds); i++)
	{
		if (IconIds[i].id.val == id.val)
		{
			switch (size)
			{
			case 64: return IconIds[i].icon64;
			case 128: return IconIds[i].icon128;
			case 256: return IconIds[i].icon256;
			default: return NULL;
			}
		}
	}
	return NULL;
}
