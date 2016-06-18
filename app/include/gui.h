#ifndef INCLUDE_GUI_H_
#define INCLUDE_GUI_H_

#include "common.h"
#include "datetime.h"

void drawCurrentWeather(CurWeather *curWeather);
void drawForecast(Forecast *forecast);
void drawIndoorTemp(const char *temp);
void drawMetaInfo(struct tm *curTime);

#endif /* INCLUDE_GUI_H_ */
