#ifndef INCLUDE_GUI_H_
#define INCLUDE_GUI_H_

#include "common.h"
#include "graphics.h"
#include "display.h"

#if DISP_WIDTH >= 384 && DISP_WIDTH < 480 && DISP_HEIGHT >= 640 && DISP_HEIGHT < 800
#define GUI_SCALE 1
#elif DISP_WIDTH >= 480 && DISP_HEIGHT >= 800
#define GUI_SCALE 2
#else
#error "Unsupported display resolution"
#endif

#if GUI_SCALE == 1
#define FORECAST_HOURLY_CHART_CNT   7
#define FORECAST_DAILY_CNT          3
#elif GUI_SCALE == 2
#define FORECAST_HOURLY_CHART_CNT   9
#define FORECAST_DAILY_CNT          3
#endif


void drawCurrentWeather(CurWeather *curWeather);
void drawCurrentWeatherSmall(CurWeather *curWeather);

void drawHourlyForecastChart(Forecast *forecast, int count);
void drawDailyForecastChart(Forecast *hourly, int hourlyCnt, Forecast *daily, int dailyCnt);
void drawForecast(Forecast *forecast, int count);

void drawIndoorTemp(const char *temp);
void drawIndoorTempSmall(const char *temp);

void drawMetaInfo(uint fails, uint updates, uint attempts);

struct tm *tm;
void printTime(struct tm *tm, char *str);
const char* epochToWeekdayStr(uint epoch);
const char* weekdayToString(int tm_wday);
const uint* iconIdToImage(IconId id, int size);

#endif /* INCLUDE_GUI_H_ */
