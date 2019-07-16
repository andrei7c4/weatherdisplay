#include <os_type.h>
#include <osapi.h>
#include <mem.h>
#include <float.h>
#include <stddef.h>
#include "gui.h"
#include "common.h"
#include "conv.h"
#include "datetime.h"
#include "fonts.h"
#include "icons.h"
#include "config.h"
#include "debug.h"

int guiYoffset = 0;

void ICACHE_FLASH_ATTR drawCurrentWeather(CurWeather *curWeather)
{
#if GUI_SCALE == 1
    const int iconWidth = 192;
    const int iconHeight = 192;
    const int cityX = 16;
    const int cityY = guiYoffset + 28;
    const int tempX = 16;
    const int descX = 16;
    const int iconX = GFXMEM_WIDTH-iconWidth-16;
    int iconY = guiYoffset + 32;
    int tempY = guiYoffset + 80;
    int descY = guiYoffset + 256;
    const uint **tempFont = arial80b;
    const uint *descFont = arial41;
#elif GUI_SCALE == 2
	const int iconWidth = 256;
	const int iconHeight = 256;
	const int cityX = 16;
	const int cityY = guiYoffset + 35;
	const int tempX = 16;
	const int descX = 16;
	const int iconX = GFXMEM_WIDTH-iconWidth-32;
	int iconY = guiYoffset + 40;
	int tempY = guiYoffset + 100;
	int descY = guiYoffset + 320;
	const uint **tempFont = arial100b;
	const uint **descFont = arial56;
#endif
	const uint* icon = iconIdToImage(curWeather->icon, iconWidth);

	if (config.cityDisplayed[0] != '\0')
	{
		int cityWidth = gfxStrWidthBig(arial56, config.cityDisplayed);
		if ((cityX+cityWidth) > (iconX+20))	// small overlap allowed
		{
			iconY += 40;
#if GUI_SCALE == 1
            descY += 28;
#elif GUI_SCALE == 2
			descY += 35;
#endif
		}
		gfxDrawImage(iconX, iconY, icon);
		gfxDrawStrBig(arial56, cityX, cityY, config.cityDisplayed);
	}
	else
	{
		gfxDrawImage(iconX, iconY, icon);
#if GUI_SCALE == 1
        tempY = 28;
#elif GUI_SCALE == 2
		tempY = 35;
#endif
	}

	int temp = floatToIntRound(kelvinToTemp(curWeather->temperature));
	//int temp = -28;	// todo: test
	if (temp > -100 && temp < 100)	// some sense
	{
		char strbuf[10];
		os_sprintf(strbuf, "%d*", temp);
		gfxDrawStrBig(tempFont, tempX, tempY, strbuf);
	}

#if GUI_SCALE == 1
    gfxDrawStr(descFont, descX, descY, curWeather->description);
#elif GUI_SCALE == 2
	gfxDrawStrBig(descFont, descX, descY, curWeather->description);
#endif
}

void ICACHE_FLASH_ATTR drawCurrentWeatherSmall(CurWeather *curWeather)
{
	const int iconWidth = 64;
	const int iconHeight = 64;
	const int iconY = guiYoffset + 1;
	const int cityY = guiYoffset + 9;
	const int tempY = guiYoffset + 9;
#if GUI_SCALE == 1
    const int cityX = 8;
    const int iconX = GFXMEM_WIDTH-iconWidth-8;
    const int tempXright = iconX-8;
#elif GUI_SCALE == 2
	const int cityX = 16;
	const int iconX = GFXMEM_WIDTH-iconWidth-16;
	const int tempXright = iconX-16;
#endif

	char tempStr[10] = "\0";
	int tempStrWidth = 0;
	int temp = floatToIntRound(kelvinToTemp(curWeather->temperature));
	//int temp = -28;	// todo: test
	if (temp > -100 && temp < 100)	// some sense
	{
		os_sprintf(tempStr, "%d*", temp);
		tempStrWidth = gfxStrWidthBig(arial56b, tempStr);
	}

	if (config.cityDisplayed[0] != '\0')
	{
		int cityWidth56 = gfxStrWidthBig(arial56, config.cityDisplayed);
		int cityWidthMax = tempXright-tempStrWidth-16-cityX;
		if (cityWidth56 <= cityWidthMax)
		{
			gfxDrawStrBig(arial56, cityX, cityY, config.cityDisplayed);
		}
		else
		{
			gfxDrawStr(arial32b, cityX, cityY, config.cityDisplayed);
		}
	}

	if (tempStr[0] != '\0')
	{
		gfxDrawStrAlignRightBig(arial56b, tempXright, tempY, tempStr);
	}

	const uint *image = iconIdToImage(curWeather->icon, 64);
	if (image)
	{
		gfxDrawImage(iconX, iconY, image);
	}
}


void ICACHE_FLASH_ATTR drawForecast(Forecast *forecast, int count)
{
#if GUI_SCALE == 1
    const int iconWidth = 96;
    const int iconX[3] = {16, 144, 272};
    const int weekdayY = guiYoffset + 27;
    const int iconY = guiYoffset + 96;
    const int tempY = guiYoffset + 240;
    const uint *weekdayFont = arial27;
    const uint *tempFont = arial27b;
#elif GUI_SCALE == 2
	const int iconWidth = 128;
	const int iconX[3] = {16, 176, 336};
	const int weekdayY = guiYoffset + 30;
	const int iconY = guiYoffset + 96;
	const int tempY = guiYoffset + 270;
	const uint *weekdayFont = arial32;
	const uint *tempFont = arial32b;
#endif
	const int textCentre[3] = {iconX[0]+(iconWidth/2),
							   iconX[1]+(iconWidth/2),
							   iconX[2]+(iconWidth/2)};

	gfxDrawLine(0, guiYoffset, GFXMEM_WIDTH-1, guiYoffset, 1);

	if (count > FORECAST_DAILY_CNT)
	{
		count = FORECAST_DAILY_CNT;
	}

	int i;
	for (i = 0; i < count; i++)
	{
		const char *weekday = epochToWeekdayStr(forecast[i].datetime);
		gfxDrawStrCentred(weekdayFont, textCentre[i], weekdayY, weekday);

		gfxDrawImage(iconX[i], iconY, iconIdToImage(forecast[i].icon, iconWidth));

		int tempMin = floatToIntRound(kelvinToTemp(forecast[i].value.tempMin / FLOAT_SCALE));
		int tempMax = floatToIntRound(kelvinToTemp(forecast[i].value.tempMax / FLOAT_SCALE));
		if (tempMin > -100 && tempMin < 100 &&
			tempMax > -100 && tempMax < 100)	// some sense
		{
			char strbuf[20] = "";
			os_sprintf(strbuf, "%d°...%d°", tempMin, tempMax);
			gfxDrawStrCentred(tempFont, textCentre[i], tempY, strbuf);
		}
	}
}

#if GUI_SCALE == 1
#define CHART_HEIGHT    140
#elif GUI_SCALE == 2
#define CHART_HEIGHT	220
#endif
void ICACHE_FLASH_ATTR drawHourlyForecastChart(Forecast *forecast, int count)
{
	if (count > 0)
	{
		if (count > FORECAST_HOURLY_CHART_CNT)
		{
			count = FORECAST_HOURLY_CHART_CNT;
		}
		drawForecastChart(forecast, count, NULL, 0, eHourlyChart,
							0, guiYoffset+80, GFXMEM_WIDTH, CHART_HEIGHT);
	}
}

void ICACHE_FLASH_ATTR drawDailyForecastChart(Forecast *hourly, int hourlyCnt, Forecast *daily, int dailyCnt)
{
	if (hourlyCnt > 0 && dailyCnt > 0)
	{
	    drawForecastChart(hourly, hourlyCnt, daily, dailyCnt, eDailyChart,
	                            0, guiYoffset+20, GFXMEM_WIDTH, CHART_HEIGHT);
	}
}


void ICACHE_FLASH_ATTR drawIndoorTemp(const char *temp)
{
	if (temp[0])
	{
		gfxDrawLine(0, GFXMEM_HEIGHT-92, GFXMEM_WIDTH-1, GFXMEM_HEIGHT-92, 1);
		gfxDrawImage(16, GFXMEM_HEIGHT-74, indoorTempIcon64);
		int strWidth = gfxDrawStrBig(segment58, 96, GFXMEM_HEIGHT-51, temp);
		gfxDrawCharBig(segment58, 96+strWidth, GFXMEM_HEIGHT-51, '*');
	}
}

void ICACHE_FLASH_ATTR drawIndoorTempSmall(const char *temp)
{
	if (temp[0])
	{
		gfxDrawLine(0, GFXMEM_HEIGHT-45, GFXMEM_WIDTH-1, GFXMEM_HEIGHT-45, 1);
		gfxDrawImage(16, GFXMEM_HEIGHT-42, indoorTempIcon40);
		int strWidth = gfxDrawStrBig(segment58, 72, GFXMEM_HEIGHT-41, temp);
		gfxDrawCharBig(segment58, 72+strWidth, GFXMEM_HEIGHT-41, '*');
	}
}

void ICACHE_FLASH_ATTR drawMetaInfo(uint fails, uint updates, uint attempts)
{
	char strbuf[40];
	if (curTime.valid)
	{
		strbuf[0] = '@';
		printTime(&curTime, strbuf+1);
		gfxDrawStrAlignRight(arial16, GFXMEM_WIDTH, GFXMEM_HEIGHT-30, strbuf);
	}
	os_sprintf(strbuf, "%u | %u | %u", fails, updates, attempts);
	gfxDrawStrAlignRight(arial16, GFXMEM_WIDTH, GFXMEM_HEIGHT-15, strbuf);
}


void ICACHE_FLASH_ATTR printTime(struct tm *tm, char *str)
{
	int am = FALSE;
	if (config.clock24)
	{
		os_sprintf(str, "%d:%02d", tm->tm_hour, tm->tm_min);
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


const char* ICACHE_FLASH_ATTR epochToWeekdayStr(uint epoch)
{
	int weekday = epochToWeekday(epoch);
	if (weekday != ERROR)
	{
		if (curTime.valid)
		{
			if (weekday == curTime.tm_wday)
			{
				return "Today";
			}
//			else if (weekday == curTime.tm_wday+1 ||
//					(weekday == 0 && curTime.tm_wday == 6))
//			{
//				return "Tomorrow";
//			}
			else
			{
				return weekdayToString(weekday);
			}
		}
		else
		{
			return weekdayToString(weekday);
		}
	}
	else
	{
		return "";
	}
}

const char* ICACHE_FLASH_ATTR weekdayToString(int tm_wday)
{
	switch (tm_wday)
	{
//	case 0: return "Sunday";
//	case 1: return "Monday";
//	case 2: return "Tuesday";
//	case 3: return "Wednesday";
//	case 4: return "Thursday";
//	case 5: return "Friday";
//	case 6: return "Saturday";
	case 0: return "Sun";
	case 1: return "Mon";
	case 2: return "Tue";
	case 3: return "Wed";
	case 4: return "Thu";
	case 5: return "Fri";
	case 6: return "Sat";
	default: return "";
	}
}



typedef struct
{
	IconId id;
	const uint *icon64;
	const uint *icon96;
	const uint *icon128;
	const uint *icon192;
	const uint *icon256;
}IconIdPair;

LOCAL const IconIdPair IconIds[] = {
	{"01d", icon64_01d, icon96_01d, icon128_01d, icon192_01d, icon256_01d},
	{"01n", icon64_01n, icon96_01n, icon128_01n, icon192_01n, icon256_01n},
	{"02d", icon64_02d, icon96_02d, icon128_02d, icon192_02d, icon256_02d},
	{"02n", icon64_02n, icon96_02n, icon128_02n, icon192_02n, icon256_02n},
	{"03d", icon64_03d, icon96_03d, icon128_03d, icon192_03d, icon256_03d},
	{"03n", icon64_03d, icon96_03d, icon128_03d, icon192_03d, icon256_03d},
	{"04d", icon64_04d, icon96_04d, icon128_04d, icon192_04d, icon256_04d},
	{"04n", icon64_04d, icon96_04d, icon128_04d, icon192_04d, icon256_04d},
	{"09d", icon64_09d, icon96_09d, icon128_09d, icon192_09d, icon256_09d},
	{"09n", icon64_09d, icon96_09d, icon128_09d, icon192_09d, icon256_09d},
	{"10d", icon64_10d, icon96_10d, icon128_10d, icon192_10d, icon256_10d},
	{"10n", icon64_10d, icon96_10d, icon128_10d, icon192_10d, icon256_10d},
	{"11d", icon64_11d, icon96_11d, icon128_11d, icon192_11d, icon256_11d},
	{"11n", icon64_11d, icon96_11d, icon128_11d, icon192_11d, icon256_11d},
	{"13d", icon64_13d, icon96_13d, icon128_13d, icon192_13d, icon256_13d},
	{"13n", icon64_13d, icon96_13d, icon128_13d, icon192_13d, icon256_13d},
	{"50d", icon64_50d, icon96_50d, icon128_50d, icon192_50d, icon256_50d},
	{"50n", icon64_50n, icon96_50n, icon128_50n, icon192_50n, icon256_50n}
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
			case 96: return IconIds[i].icon96;
			case 128: return IconIds[i].icon128;
			case 192: return IconIds[i].icon192;
			case 256: return IconIds[i].icon256;
			default: return NULL;
			}
		}
	}
	return NULL;
}
