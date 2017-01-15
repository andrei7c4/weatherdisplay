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


const uint **scaleFont = arial21;
#define SCALE_WIDTH			30
#define SCALE_MAX_STEPS		5
#define SCALE_STEP_MULT		5

const uint **hoursFont = arial21;
const uint **daysFont = arial21;

#define ICON_SIZE		64


LOCAL void ICACHE_FLASH_ATTR findMinMaxTemp(Forecast *forecast, ForecastType type, int cnt, float *min, float *max)
{
	*min = FLT_MAX;
	*max = FLT_MIN;
	int i;
	float temp;
	switch (type)
	{
	case eForecastHourly:
		for (i = 0; i < cnt; i++)
		{
			temp = forecast[i].temp;
			if (temp < *min)
			{
				*min = temp;
			}
			if (temp > *max)
			{
				*max = temp;
			}
		}
		break;
	case eForecastDaily:
		for (i = 0; i < cnt; i++)
		{
			temp = forecast[i].minTemp;
			if (temp < *min)
			{
				*min = temp;
			}
			temp = forecast[i].maxTemp;
			if (temp > *max)
			{
				*max = temp;
			}
		}
		break;
	}
}

LOCAL float ICACHE_FLASH_ATTR findMaxRain(Forecast *forecast, int cnt)
{
	float max = 0;
	int i;
	float rain;
	for (i = 0; i < cnt; i++)
	{
		rain = forecast[i].rainsnow;
		if (rain > max)
		{
			max = rain;
		}
	}
	return max;
}

LOCAL int ICACHE_FLASH_ATTR roundUp(int numToRound, int multiple)
{
    if (multiple == 0)
        return numToRound;

    int remainder = abs(numToRound) % multiple;
    if (remainder == 0)
        return numToRound+multiple;

    if (numToRound < 0)
        return -(abs(numToRound) - remainder);
    else
        return numToRound + multiple - remainder;
}

LOCAL int ICACHE_FLASH_ATTR roundDown(int numToRound, int multiple)
{
    if (multiple == 0)
        return numToRound;

    int remainder = abs(numToRound) % multiple;
    if (remainder == 0)
        return numToRound-multiple;

    if (numToRound < 0)
    	return numToRound - multiple + remainder;
    else
        return numToRound - remainder;
}

LOCAL void ICACHE_FLASH_ATTR calcTempScale(int min, int max, int multiple, int maxSteps, int *steps, int *nrSteps)
{
	int top = roundUp(max, multiple);
	int bottom = roundDown(min, multiple);
	int diff = top - bottom;
	if (diff < (multiple*2))
	{
		bottom -= multiple;
	}

	int i;
	int stepSize = multiple;

	diff = top - bottom;
	*nrSteps = (diff / stepSize)+1;
	if (*nrSteps > maxSteps)
	{
		int newDiff = 0;
		while (newDiff <= diff)
		{
			stepSize *= 2;
			newDiff = stepSize*4;
		}

		int topAdd = (newDiff - diff)/2;
		topAdd -= (topAdd%multiple);
		top += topAdd;

		diff = newDiff;
		*nrSteps = 5;
	}

	steps[0] = top;
	for (i = 1; i < *nrSteps; i++)
	{
		steps[i] = top-(stepSize*i);
	}
}

LOCAL void ICACHE_FLASH_ATTR calcRainScale(int max, int *steps, int nrSteps)
{
	max += 1;
	int lastStep = (nrSteps-1);
	int step;
	int diff = (lastStep - max);
	if (diff > 0)
	{
		step = 1;
	}
	else
	{
		if (diff == 0)
		{
			max = lastStep+1;
		}
		step = (float)max/lastStep+1;
	}

	int i;
	for (i = 0; i < nrSteps; i++)
	{
		steps[nrSteps-i-1] = (i*step);
	}
}

LOCAL void ICACHE_FLASH_ATTR drawScaleValus(int *values, int *valYpos, int cnt, int x, int y, char *title, int alignRight)
{
	int yPos = yPos = valYpos[0]+y;
	int strWidth = dispStrWidth(scaleFont, title);
	if (alignRight)
	{
		dispDrawStrAlignRight(scaleFont, x+strWidth, yPos-6, title);
	}
	else
	{
		dispDrawStr(scaleFont, x-strWidth, yPos-6, title);
	}

	char str[4];
	int value, i;
	for (i = 1; i < cnt; i++)
	{
		value = values[i];
		yPos = valYpos[i]+y;

		if (value > -100 && value < 100)	// some sense
		{
			os_sprintf(str, "%d", value);
			if (alignRight)
			{
				dispDrawStrAlignRight(scaleFont, x+24, yPos-6, str);
			}
			else
			{
				dispDrawStr(scaleFont, x-24, yPos-6, str);
			}
		}
	}
}

LOCAL void ICACHE_FLASH_ATTR drawHorizontalLines(int *stepsYpos, int nrSteps, int x, int y, int width)
{
	int yPos, i;
	for (i = 0; i < nrSteps; i++)
	{
		yPos = stepsYpos[i]+y;
		if (i < (nrSteps-1))
		{
			if (i > 0 && (yPos & 1)) // might interfere with dotted bars
			{
				yPos++;
			}
			dispDrawLineDotted(x, yPos, x+width-1, yPos, 1, 1);
		}
		else
		{
			dispDrawLine(x, yPos, x+width-1, yPos, 1);
		}
	}
}

LOCAL int ICACHE_FLASH_ATTR calcXstep(int width, int cnt, int stretchFactor, int inc)
{
	int xStep;
	if ((stretchFactor-inc) > 0)
	{
		xStep = (float)width/(cnt*stretchFactor-1) + 0.5;
	}
	else
	{
		xStep = (float)width/cnt + 0.5;
	}
	return (xStep*stretchFactor);
}

LOCAL void ICACHE_FLASH_ATTR drawVerticalLines(int x, int y, int width, int height, int cnt, int stretchFactor)
{
	int xStep = calcXstep(width, cnt, stretchFactor, 1);
	int xPos = x + xStep/stretchFactor/2;
	int halfStep = xStep/2;

	dispDrawLine(x, y, x, y+height-1, 1);
	int i;
	for (i = 0; i < (cnt-1); i++)
	{
		dispDrawLine(xPos+halfStep, y, xPos+halfStep, y+height-1, 1);
		xPos += xStep;
	}
	int lastPos = x+width-1;
	dispDrawLine(lastPos, y, lastPos, y+height-1, 1);
}

LOCAL void ICACHE_FLASH_ATTR drawPlotLine(float *values, int valMin, int valMax, int cnt,
		int x, int y, int width, int height, int stretchFactor)
{
	int valDiff = valMax - valMin;
	float pixelsPerStep = (float)height / valDiff;

	int xStep = calcXstep(width, cnt, stretchFactor, 1);
	int xPos0 = x + xStep/stretchFactor/2;
	int xPos1, yPos1;
	int yPos0 = (((float)valMax - values[0]) * pixelsPerStep) + y + 0.5;
	dispDrawcircle(xPos0, yPos0, 5, 1, 1);

	int i;
	for (i = 1; i < cnt; i++)
	{
		yPos1 = (((float)valMax - values[i]) * pixelsPerStep) + y + 0.5;
		xPos1 = xPos0 + xStep;
		dispDrawLineBold(xPos0, yPos0, xPos1, yPos1, 1, 1, 1);
		dispDrawcircle(xPos0, yPos0, 5, 1, 1);
		yPos0 = yPos1;
		xPos0 = xPos1;
	}
	dispDrawcircle(xPos0, yPos0, 5, 1, 1);
}

LOCAL void ICACHE_FLASH_ATTR drawPlotBars(float *values, int valMin, int valMax, int cnt,
		int x, int y, int width, int height, int stretchFactor)
{
	int valDiff = valMax - valMin;
	float pixelsPerStep = (float)height / valDiff;

	int xStep = calcXstep(width, cnt, stretchFactor, 1);
	int xCenter = x + xStep/stretchFactor/2;
	int yPos, xPos0, xPos1;
	int barWidth = (float)xStep*0.7 + 0.5;
	int halfBarWidth = barWidth/2;
	int i;
	float rain;
	for (i = 0; i < cnt; i++)
	{
		rain = values[i];
		if (rain > 0.001)
		{
			yPos = (((float)valMax - rain) * pixelsPerStep) + y + 0.5;
			xPos0 = xCenter-halfBarWidth;
			xPos1 = xCenter+halfBarWidth;
			if (xPos0 < x)
			{
				xPos0 = x;
			}
			if (xPos0 & 1) // might interfere with horisontal lines
			{
				xPos0++;
			}
			if (xPos1 > (x+width-1))
			{
				xPos1 = x+width-1;
			}
			dispDrawRectDotted(xPos0, y+height, xPos1, yPos, 1, 1);
		}
		xCenter += xStep;
	}
}

LOCAL void ICACHE_FLASH_ATTR drawHours(Forecast *forecast, int cnt, int x, int y, int width, int stretchFactor, int inc)
{
	int xStep = calcXstep(width, cnt, stretchFactor, inc);
	int xPos = x + xStep/stretchFactor/2;
	struct tm tm;
	char strbuf[8];
	int i;
	for (i = 0; i < cnt; i+=inc)
	{
		if (epochToTm(forecast[i].datetime+config.utcoffset, &tm) == OK)
		{
			printTime(&tm, strbuf);
			if (config.clock24)
			{
				dispDrawStrCentred(hoursFont, xPos, y, strbuf);
			}
			else
			{
				// am/pm part is drawn with a smaller font
				char ampm[3];
				int timeStrLen = os_strlen(strbuf);
				os_strcpy(ampm, strbuf+timeStrLen-2);
				strbuf[timeStrLen-2] = '\0';

				int timeWidth = dispStrWidth(hoursFont, strbuf);
				int fullWidth = timeWidth + dispStrWidth(arial16, ampm);
				int strX = xPos-(fullWidth/2);
				strX = alignTo8(strX);	// align to nearest 8 pixel boundary
				dispDrawStr(hoursFont, strX, y, strbuf);
				dispDrawStr(arial16, strX+timeWidth, y+4, ampm);
			}
			xPos += xStep;
		}
	}
}

LOCAL void ICACHE_FLASH_ATTR drawDays(Forecast *forecast, int cnt, int x, int y, int width, int stretchFactor, int inc)
{
	int xStep = calcXstep(width, cnt, stretchFactor, inc);
	int xPos = x + xStep/stretchFactor/2;

	char weekday[10] = "Tomorrow";
	dispDrawStrCentred(daysFont, xPos, y, weekday);
	xPos += xStep;

	int i;
	for (i = 1; i < cnt; i+=inc)
	{
		epochToWeekday(forecast[i].datetime+config.utcoffset, weekday);
		dispDrawStrCentred(daysFont, xPos, y, weekday);
		xPos += xStep;
	}
}

LOCAL void ICACHE_FLASH_ATTR drawIcons(Forecast *forecast, int cnt, int x, int y, int width, int stretchFactor, int inc)
{
	int xStep = calcXstep(width, cnt, stretchFactor, inc);
	int xPos = x + xStep/stretchFactor/2;
	int i;
	for (i = 0; i < cnt; i+=inc)
	{
		const uint* image = iconIdToImage(forecast[i].icon, ICON_SIZE);
		if (image)
		{
			dispDrawImage(xPos-(ICON_SIZE/2), y, image);
		}
		xPos += xStep;
	}
}

void ICACHE_FLASH_ATTR drawForecastChart(Forecast *forecast, ForecastType type, int cnt, int x, int y, int width, int height)
{
	float min, max;
	findMinMaxTemp(forecast, type, cnt, &min, &max);
	int tempMin = floatToIntRound(kelvinToTemp(min));
	int tempMax = floatToIntRound(kelvinToTemp(max));

	int values[SCALE_MAX_STEPS];
	int valYpos[SCALE_MAX_STEPS];
	int valCnt, i;

	calcTempScale(tempMin, tempMax, SCALE_STEP_MULT, SCALE_MAX_STEPS, values, &valCnt);
	tempMin = values[valCnt-1];
	tempMax = values[0];

	valYpos[0] = 0;
	valYpos[valCnt-1] = height;
	int stepPixels = (float)height / (valCnt-1) + 0.5;
	for (i = 1; i < (valCnt-1); i++)
	{
		valYpos[i] = stepPixels*i;
	}

	char *degree = config.fahrenheit ? "*F" : "*C";
	drawScaleValus(values, valYpos, valCnt, x, y, degree, 1);


	int rainMax = findMaxRain(forecast, cnt);
	calcRainScale(rainMax, values, valCnt);
	rainMax = values[0];
	drawScaleValus(values, valYpos, valCnt, x+width, y, "mm", 0);


	int plotX = x+SCALE_WIDTH;
	int plotWidth = width-(SCALE_WIDTH*2);
	drawHorizontalLines(valYpos, valCnt, plotX, y, plotWidth);

	float *temps = (float*)os_zalloc(cnt*sizeof(float));
	float *rains = (float*)os_zalloc(cnt*sizeof(float));
	switch (type)
	{
	case eForecastHourly:
		for (i = 0; i < cnt; i++)
		{
			temps[i] = kelvinToTemp(forecast[i].temp);
		}
		drawPlotLine(temps, tempMin, tempMax, cnt, plotX, y, plotWidth, height, 1);

		for (i = 0; i < cnt; i++)
		{
			rains[i] = forecast[i].rainsnow;
		}
		drawPlotBars(rains, 0, rainMax, cnt, plotX, y, plotWidth, height, 1);

		drawVerticalLines(plotX, y+height, plotWidth, 6, cnt, 1);
		drawHours(forecast, cnt, plotX, y+height+10, plotWidth, 2, 2);
		drawIcons(forecast, cnt, plotX, y+height+30, plotWidth, 2, 2);
		break;
	case eForecastDaily:
		for (i = 0; i < cnt; i++)
		{
			temps[i] = kelvinToTemp(forecast[i].minTemp);
		}
		drawPlotLine(temps, tempMin, tempMax, cnt, plotX, y, plotWidth, height, 2);

		for (i = 0; i < cnt; i++)
		{
			temps[i] = kelvinToTemp(forecast[i].maxTemp);
		}
		drawPlotLine(temps, tempMin, tempMax, cnt, plotX, y, plotWidth, height, 2);

		for (i = 0; i < cnt; i++)
		{
			rains[i] = forecast[i].rainsnow;
		}
		drawPlotBars(rains, 0, rainMax, cnt, plotX, y, plotWidth, height, 2);

		drawVerticalLines(plotX, y+height, plotWidth, 6, cnt, 2);
		drawDays(forecast, cnt, plotX, y+height+10, plotWidth, 2, 1);
		drawIcons(forecast, cnt, plotX, y+height+30, plotWidth, 2, 1);
		break;
	}
	os_free(temps);
	os_free(rains);
}
