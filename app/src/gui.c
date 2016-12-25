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


void ICACHE_FLASH_ATTR drawForecast(Forecast *forecast)
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

	dispDrawStrCentred(arial32, textCentre[0], weekdayY, "Today");
	dispDrawStrCentred(arial32, textCentre[1], weekdayY, "Tomorrow");

	epochToWeekday(forecast[2].datetime, strbuf);
	dispDrawStrCentred(arial32, textCentre[2], weekdayY, strbuf);

	int i;
	for (i = 0; i < 3; i++)
	{
		dispDrawImage(iconX[i], iconY, iconIdToImage(forecast[i].icon, 128));

		int tempMin = floatToIntRound(kelvinToTemp(forecast[i].minTemp));
		int tempMax = floatToIntRound(kelvinToTemp(forecast[i].maxTemp));
		if (tempMin > -100 && tempMin < 100 &&
			tempMax > -100 && tempMax < 100)	// some sense
		{
			os_sprintf(strbuf, "%d*...%d*", tempMin, tempMax);
			dispDrawStrCentred(arial32b, textCentre[i], tempY, strbuf);
		}
	}
}

#define CHART_HEIGHT	220
void ICACHE_FLASH_ATTR drawHourlyForecastChart(Forecast *forecast)
{
	drawForecastChart(forecast, eForecastHourly, FORECAST_HOURLY_CNT,
						0, 80, DISP_WIDTH, CHART_HEIGHT);
}

void ICACHE_FLASH_ATTR drawDailyForecastChart(Forecast *forecast)
{
	dispFillMem(0x00);
	drawForecastChart(forecast+1, eForecastDaily, FORECAST_DAILY_CNT-1,
						0, 20, DISP_WIDTH, CHART_HEIGHT);
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
		os_sprintf(strbuf, "@%02d:%02d", curTime->tm_hour, curTime->tm_min);
		dispDrawStrAlignRight(arial16, DISP_WIDTH, DISP_HEIGHT-30, strbuf);
	}
	os_sprintf(strbuf, "fails %u/%u", retain.fails, retain.attempts);
	dispDrawStrAlignRight(arial16, DISP_WIDTH, DISP_HEIGHT-15, strbuf);
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

const uint* ICACHE_FLASH_ATTR iconIdToImage(char *iconId, int size)
{
	const IconIdPair icons64[NR_ICONS] = {
			{"01d",icon64_01d},
			{"01n",icon64_01n},
			{"02d",icon64_02d},
			{"02n",icon64_02n},
			{"03d",icon64_03d},
			{"03n",icon64_03d},
			{"04d",icon64_04d},
			{"04n",icon64_04d},
			{"09d",icon64_09d},
			{"09n",icon64_09d},
			{"10d",icon64_10d},
			{"10n",icon64_10d},
			{"11d",icon64_11d},
			{"11n",icon64_11d},
			{"13d",icon64_13d},
			{"13n",icon64_13d},
			{"50d",icon64_50d},
			{"50n",icon64_50n}
	};
	const IconIdPair icons128[NR_ICONS] = {
			{"01d",icon128_01d},
			{"01n",icon128_01n},
			{"02d",icon128_02d},
			{"02n",icon128_02n},
			{"03d",icon128_03d},
			{"03n",icon128_03d},
			{"04d",icon128_04d},
			{"04n",icon128_04d},
			{"09d",icon128_09d},
			{"09n",icon128_09d},
			{"10d",icon128_10d},
			{"10n",icon128_10d},
			{"11d",icon128_11d},
			{"11n",icon128_11d},
			{"13d",icon128_13d},
			{"13n",icon128_13d},
			{"50d",icon128_50d},
			{"50n",icon128_50n}
	};
	const IconIdPair icons256[NR_ICONS] = {
			{"01d",icon256_01d},
			{"01n",icon256_01n},
			{"02d",icon256_02d},
			{"02n",icon256_02n},
			{"03d",icon256_03d},
			{"03n",icon256_03d},
			{"04d",icon256_04d},
			{"04n",icon256_04d},
			{"09d",icon256_09d},
			{"09n",icon256_09d},
			{"10d",icon256_10d},
			{"10n",icon256_10d},
			{"11d",icon256_11d},
			{"11n",icon256_11d},
			{"13d",icon256_13d},
			{"13n",icon256_13d},
			{"50d",icon256_50d},
			{"50n",icon256_50n}
	};

	const IconIdPair *icons;
	switch (size)
	{
	case 64:
		icons = icons64;
		break;
	case 128:
		icons = icons128;
		break;
	case 256:
		icons = icons256;
		break;
	default:
		return NULL;
	}

	int i;
	for (i = 0; i < NR_ICONS; i++)
	{
		if (!os_strcmp(iconId, icons[i].id))
		{
			return icons[i].icon;
		}
	}

	return NULL;
}
