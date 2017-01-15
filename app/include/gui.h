#ifndef INCLUDE_GUI_H_
#define INCLUDE_GUI_H_

#include "common.h"
#include "datetime.h"

void drawCurrentWeather(CurWeather *curWeather);
void drawCurrentWeatherSmall(CurWeather *curWeather);

void drawHourlyForecastChart(Forecast *forecast);
void drawDailyForecastChart(Forecast *forecast);
void drawForecast(Forecast *forecast);

void drawIndoorTemp(const char *temp);
void drawIndoorTempSmall(const char *temp);

void drawMetaInfo(struct tm *curTime);

void printTime(struct tm *tm, char *str);
void epochToWeekday(uint epoch, char *weekday);
const uint* iconIdToImage(char *iconId, int size);

#endif /* INCLUDE_GUI_H_ */
