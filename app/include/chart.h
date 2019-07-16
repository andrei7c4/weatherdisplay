#ifndef CHART_H_
#define CHART_H_

#include "common.h"

void drawForecastChart(Forecast *hourly, int hourlyCnt, Forecast *daily, int dailyCnt, ForecastType type, int x, int y, int width, int height);


#endif /* CHART_H_ */
