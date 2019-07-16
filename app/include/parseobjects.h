#ifndef INCLUDE_PARSEOBJECTS_H_
#define INCLUDE_PARSEOBJECTS_H_

#include "common.h"

typedef enum
{
    eDatetimeParsed    = 0x01,
    eTemperatureParsed = 0x02,
    eDescriptionParsed = 0x04,
    eIconParsed        = 0x08,
    eRainsnowParsed    = 0x10,
    eWeatherAllParsed  = eDatetimeParsed     |
                         eTemperatureParsed  |
                         eDescriptionParsed  |
                         eIconParsed,
    eForecastParsed    = eDatetimeParsed     |
                         eTemperatureParsed  |
                         eIconParsed
}ParsedFields;

ParsedFields parseWeather(const char *json, int jsonLen, CurWeather *curWeather, char *cityId, int cityIdSize, int *statusCode);

int parseForecast(const char *json, int jsonLen, Forecast *forecast, int forecastSize, int *statusCode);


#endif /* INCLUDE_PARSEOBJECTS_H_ */
