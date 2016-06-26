#include <os_type.h>
#include <osapi.h>
#include "gui.h"
#include "common.h"
#include "fonts.h"
#include "icons.h"
#include "display.h"
#include "config.h"
#include "debug.h"


LOCAL float kelvinToTemp(float kelvin);
LOCAL int floatToIntRound(float f);
LOCAL void epochToWeekday(uint epoch, char *weekday);

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

	dispFillMem(0x00);

	if (config.cityDisplayed[0] != '\0')
	{
		int cityWidth = dispStrWidth(arial56, config.cityDisplayed);
		if ((cityX+cityWidth) > (iconX+20))	// small overlap allowed
		{
			iconY = 80;
			descY = 355;
		}
		dispDrawImage(iconX, iconY, curWeather->icon);
		dispDrawStr(arial56, cityX, cityY, config.cityDisplayed);
	}
	else
	{
		dispDrawImage(iconX, iconY, curWeather->icon);
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
	dispDrawLine(0, DISP_HEIGHT-1, DISP_WIDTH-1, DISP_HEIGHT-1, 1);
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

	dispDrawStrCentred(arial32, textCentre[0], weekdayY, "Today");
	dispDrawStrCentred(arial32, textCentre[1], weekdayY, "Tomorrow");

	epochToWeekday(forecast[2].datetime, strbuf);
	dispDrawStrCentred(arial32, textCentre[2], weekdayY, strbuf);

	int i;
	for (i = 0; i < 3; i++)
	{
		dispDrawImage(iconX[i], iconY, forecast[i].icon);

		int tempMin = floatToIntRound(kelvinToTemp(forecast[i].minTemp));
		int tempMax = floatToIntRound(kelvinToTemp(forecast[i].maxTemp));
		if (tempMin > -100 && tempMin < 100 &&
			tempMax > -100 && tempMax < 100)	// some sense
		{
			os_sprintf(strbuf, "%d*...%d*", tempMin, tempMax);
			dispDrawStrCentred(arial32b, textCentre[i], tempY, strbuf);
		}
	}

	dispDrawLine(0, 308, DISP_WIDTH-1, 308, 1);
}

void ICACHE_FLASH_ATTR drawIndoorTemp(const char *temp)
{
	dispDrawImage(16, DISP_HEIGHT-74, indoorTempIcon);
	int strWidth = dispDrawStr(segment58, 96, DISP_HEIGHT-51, temp);
	dispDrawChar(segment58, 96+strWidth, DISP_HEIGHT-51, '*');
}

void ICACHE_FLASH_ATTR drawMetaInfo(struct tm *curTime)
{
	char strbuf[20];
	int strWidth;
	if (curTime)
	{
		os_sprintf(strbuf, "@%02d:%02d", curTime->tm_hour, curTime->tm_min);
		strWidth = dispStrWidth(arial16, strbuf);
		dispDrawStr(arial16, DISP_WIDTH-strWidth, DISP_HEIGHT-30, strbuf);
	}

	os_sprintf(strbuf, "fails %u/%u", retain.fails, retain.attempts);
	strWidth = dispStrWidth(arial16, strbuf);
	dispDrawStr(arial16, DISP_WIDTH-strWidth, DISP_HEIGHT-15, strbuf);
}


//#define USE_FAHRENHEIT
LOCAL float ICACHE_FLASH_ATTR kelvinToTemp(float kelvin)
{
	const float zeroKelvin = 273.15;
#ifdef USE_FAHRENHEIT
        return (kelvin-zeroKelvin) * 1.8 + 32.0;
#else
        return kelvin-zeroKelvin;
#endif
}

LOCAL int ICACHE_FLASH_ATTR floatToIntRound(float f)
{
	return f >= 0 ? (int)(f+0.5) : (int)(f-0.5);
}

LOCAL void ICACHE_FLASH_ATTR epochToWeekday(uint epoch, char *weekday)
{
	struct tm tm;
	if (epochToTm(epoch, &tm) != OK)
		return;

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
