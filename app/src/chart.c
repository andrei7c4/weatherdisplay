#include <os_type.h>
#include <osapi.h>
#include <mem.h>
#include <float.h>
#include <stddef.h>

#include "display.h"
#include "graphics.h"
#include "gui.h"
#include "common.h"
#include "conv.h"
#include "datetime.h"
#include "fonts.h"
#include "icons.h"
#include "config.h"
#include "debug.h"


const uint *scaleFont = arial21;
#define SCALE_WIDTH			30
#define SCALE_MAX_STEPS		5
#define SCALE_STEP_MULT		5

const uint *hoursFont = arial21;
const uint *daysFont = arial21;

#define ICON_SIZE		64


LOCAL void ICACHE_FLASH_ATTR findMinMaxTemp(Forecast *forecast, int cnt, float *min, float *max)
{
	*min = FLT_MAX;
	*max = FLT_MIN;
	int i;
	float temp;
    for (i = 0; i < cnt; i++)
    {
        temp = forecast[i].value.temp / FLOAT_SCALE;
        if (temp < *min)
        {
            *min = temp;
        }
        if (temp > *max)
        {
            *max = temp;
        }
    }
}

LOCAL float ICACHE_FLASH_ATTR findMaxRain(Forecast *forecast, int cnt)
{
	float max = 0;
	int i;
	float rain;
	for (i = 0; i < cnt; i++)
	{
		rain = forecast[i].value.rainsnow / FLOAT_SCALE;
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
	int strWidth = gfxStrWidth(scaleFont, title);
	if (alignRight)
	{
		gfxDrawStrAlignRight(scaleFont, x+strWidth, yPos-6, title);
	}
	else
	{
		gfxDrawStr(scaleFont, x-strWidth, yPos-6, title);
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
				gfxDrawStrAlignRight(scaleFont, x+24, yPos-6, str);
			}
			else
			{
				gfxDrawStr(scaleFont, x-24, yPos-6, str);
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
			gfxDrawLineDotted(x, yPos, x+width-1, yPos, 1, 1);
		}
		else
		{
			gfxDrawLine(x, yPos, x+width-1, yPos, 1);
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

LOCAL void ICACHE_FLASH_ATTR drawVerticalLinesHourly(int x, int y, int width, int height, int cnt)
{
	int xStep = xStep = (float)width/cnt + 0.5;
	int xPos = x + xStep/2;
	int halfStep = xStep/2;

	gfxDrawLine(x, y, x, y+height-1, 1);
	int i;
	for (i = 0; i < (cnt-1); i++)
	{
		gfxDrawLine(xPos+halfStep, y, xPos+halfStep, y+height-1, 1);
		xPos += xStep;
	}
	int lastPos = x+width-1;
	gfxDrawLine(lastPos, y, lastPos, y+height-1, 1);
}

LOCAL void ICACHE_FLASH_ATTR drawVerticalLinesDaily(int *dayBoundaries, int dayBoundariesCnt, int x, int y, int width, int height)
{
	int i;
	for (i = 0; i < dayBoundariesCnt; i++)
	{
	    gfxDrawLine(dayBoundaries[i], y, dayBoundaries[i], y+height-1, 1);
	}
}

LOCAL void ICACHE_FLASH_ATTR drawPlotLine(float *values, int valMin, int valMax, int cnt,
		int x, int y, int width, int height, int radius)
{
	int valDiff = valMax - valMin;
	float pixelsPerStep = (float)height / valDiff;

	int xStep = (float)width/cnt + 0.5;
	int xPos0 = x + xStep/2;
	int xPos1, yPos1;
	int yPos0 = (((float)valMax - values[0]) * pixelsPerStep) + y + 0.5;
	if(radius) gfxDrawcircle(xPos0, yPos0, radius, 1, 1);

	int i;
	for (i = 1; i < cnt; i++)
	{
		yPos1 = (((float)valMax - values[i]) * pixelsPerStep) + y + 0.5;
		xPos1 = xPos0 + xStep;
		gfxDrawLineBold(xPos0, yPos0, xPos1, yPos1, 1, 1, 1);
		if(radius) gfxDrawcircle(xPos0, yPos0, radius, 1, 1);
		yPos0 = yPos1;
		xPos0 = xPos1;
	}
	if(radius) gfxDrawcircle(xPos0, yPos0, radius, 1, 1);
}

LOCAL void ICACHE_FLASH_ATTR drawPlotBars(float *values, int valMin, int valMax, int cnt,
		int x, int y, int width, int height)
{
	int valDiff = valMax - valMin;
	float pixelsPerStep = (float)height / valDiff;

	int xStep = (float)width/cnt + 0.5;
	int xCenter = x + xStep/2;
    int xMax = x + width - 1;
    int yPos0 = y + height - 2;
	int yPos1, xPos0, xPos1;
	int barWidth = (float)xStep*0.7 + 0.5;
	int halfBarWidth = barWidth/2;
	int i;
	float rain;
	for (i = 0; i < cnt; i++)
	{
		rain = values[i];
		if (rain > 0.001)
		{
			yPos1 = (((float)valMax - rain) * pixelsPerStep) + y + 0.5;
			if (yPos0 > yPos1)
			{
	            xPos0 = xCenter-halfBarWidth;
	            xPos1 = xCenter+halfBarWidth;
	            if (xPos0 < x)
	            {
	                xPos0 = x;
	            }
	            if (xPos1 > xMax)
	            {
	                xPos1 = xMax;
	            }

	            gfxDrawRectFill(xPos0 > x ? xPos0-1 : xPos0,
                                yPos0,
                                xPos1 < xMax ? xPos1+1 : xPos1,
                                yPos1, 0);
                gfxDrawRectDotted(xPos0, yPos0, xPos1, yPos1, 1, 1);
			}
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
		if (epochToTm(forecast[i].datetime, &tm) == OK)
		{
			printTime(&tm, strbuf);
			if (config.clock24)
			{
				gfxDrawStrCentred(hoursFont, xPos, y, strbuf);
			}
			else
			{
				// am/pm part is drawn with a smaller font
				char ampm[3];
				int timeStrLen = os_strlen(strbuf);
				os_strcpy(ampm, strbuf+timeStrLen-2);
				strbuf[timeStrLen-2] = '\0';

				int timeWidth = gfxStrWidth(hoursFont, strbuf);
				int fullWidth = timeWidth + gfxStrWidth(arial16, ampm);
				int strX = xPos-(fullWidth/2);
				gfxDrawStr(hoursFont, strX, y, strbuf);
				gfxDrawStr(arial16, strX+timeWidth, y+4, ampm);
			}
			xPos += xStep;
		}
	}
}

LOCAL void ICACHE_FLASH_ATTR drawDays(Forecast *forecast, int *dayBoundaries, int cnt, int x, int y, int width)
{
    int i;
	for (i = 0; i < cnt; i++)
	{
		const char *weekday = epochToWeekdayStr(forecast[i].datetime);
		int space = dayBoundaries[i+1] - dayBoundaries[i];
		int strWidth = gfxStrWidth(daysFont, weekday);
		if(space >= strWidth)
		{
	        int center = space / 2 + dayBoundaries[i];
		    int strX = center - (strWidth/2);
		    gfxDrawStr(daysFont, strX, y, weekday);
		}
	}
}

LOCAL void ICACHE_FLASH_ATTR drawIconsDaily(Forecast *forecast, int *dayBoundaries, int cnt, int x, int y, int width)
{
    int i;
	for (i = 0; i < cnt; i++)
	{
		const uint* image = iconIdToImage(forecast[i].icon, ICON_SIZE);
		if (image)
		{
	        int space = dayBoundaries[i+1] - dayBoundaries[i];
	        if(space >= (ICON_SIZE-12))  // allow small overlap
	        {
	            int center = space / 2 + dayBoundaries[i];
	            int iconX = alignTo8(center - (ICON_SIZE/2));
	            gfxDrawImage(iconX, y, image);
	        }
		}
	}
}

LOCAL void ICACHE_FLASH_ATTR drawIconsHourly(Forecast *forecast, int cnt, int x, int y, int width, int stretchFactor, int inc)
{
	int xStep = calcXstep(width, cnt, stretchFactor, inc);
	int xPos = x + xStep/stretchFactor/2;
	int i;
	for (i = 0; i < cnt; i+=inc)
	{
		const uint* image = iconIdToImage(forecast[i].icon, ICON_SIZE);
		if (image)
		{
			gfxDrawImage(xPos-(ICON_SIZE/2), y, image);
		}
		xPos += xStep;
	}
}

void ICACHE_FLASH_ATTR drawForecastChart(Forecast *hourly, int hourlyCnt, Forecast *daily, int dailyCnt, ForecastType type, int x, int y, int width, int height)
{
	float min, max;
	findMinMaxTemp(hourly, hourlyCnt, &min, &max);
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

	char *degree = config.fahrenheit ? "°F" : "°C";
	drawScaleValus(values, valYpos, valCnt, x, y, degree, 1);


	int rainMax = findMaxRain(hourly, hourlyCnt);
	calcRainScale(rainMax, values, valCnt);
	rainMax = values[0];
	drawScaleValus(values, valYpos, valCnt, x+width, y, "mm", 0);


	int plotX = x+SCALE_WIDTH;
	int plotWidth = width-(SCALE_WIDTH*2);
	drawHorizontalLines(valYpos, valCnt, plotX, y, plotWidth);

	float *temps = (float*)os_zalloc(hourlyCnt*sizeof(float));
	float *rains = (float*)os_zalloc(hourlyCnt*sizeof(float));
	switch (type)
	{
	case eHourlyChart:
        for (i = 0; i < hourlyCnt; i++)
        {
            rains[i] = hourly[i].value.rainsnow / FLOAT_SCALE;
        }
        drawPlotBars(rains, 0, rainMax, hourlyCnt, plotX, y, plotWidth, height);

		for (i = 0; i < hourlyCnt; i++)
		{
			temps[i] = kelvinToTemp(hourly[i].value.temp / FLOAT_SCALE);
		}
		drawPlotLine(temps, tempMin, tempMax, hourlyCnt, plotX, y, plotWidth, height, 5);

		drawVerticalLinesHourly(plotX, y+height, plotWidth, 6, hourlyCnt);
		drawHours(hourly, hourlyCnt, plotX, y+height+10, plotWidth, 2, 2);
		drawIconsHourly(hourly, hourlyCnt, plotX, y+height+30, plotWidth, 2, 2);
		break;
	case eDailyChart:
	{
        for (i = 0; i < hourlyCnt; i++)
        {
            rains[i] = hourly[i].value.rainsnow / FLOAT_SCALE;
        }
        drawPlotBars(rains, 0, rainMax, hourlyCnt, plotX, y, plotWidth, height);

        for (i = 0; i < hourlyCnt; i++)
        {
            temps[i] = kelvinToTemp(hourly[i].value.temp / FLOAT_SCALE);
        }
        drawPlotLine(temps, tempMin, tempMax, hourlyCnt, plotX, y, plotWidth, height, 0);


        // calculate boundaries between days
        int dayBoundariesCnt = dailyCnt + 1;
        int *dayBoundaries = os_zalloc(dayBoundariesCnt * sizeof(int));
        dayBoundaries[0] = plotX;
        dayBoundaries[dayBoundariesCnt-1] = plotX + plotWidth-1;

        int xStep = (float)plotWidth/hourlyCnt + 0.5;
        int xPos = plotX + xStep/2;
        int prevHour = 0;
        int j;
        for (i = 0, j = 1; i < hourlyCnt && j < (dayBoundariesCnt-1); i++)
        {
            int curHour = epochToHours(hourly[i].datetime);
            if (curHour < prevHour)     // detect day change
            {
                dayBoundaries[j] = xPos;
                j++;
            }
            xPos += xStep;
            prevHour = curHour;
        }


        drawVerticalLinesDaily(dayBoundaries, dayBoundariesCnt, plotX, y+height, plotWidth, 6);
        drawDays(daily, dayBoundaries, dailyCnt, plotX, y+height+10, plotWidth);
        drawIconsDaily(daily, dayBoundaries, dailyCnt, plotX, y+height+30, plotWidth);

        os_free(dayBoundaries);
	}
		break;
	}
	os_free(temps);
	os_free(rains);
}
