#include <os_type.h>
#include <osapi.h>
#include <mem.h>
#include "parseobjects.h"
#include "parsejson.h"
#include "common.h"
#include "config.h"
#include "conv.h"
#include "debug.h"


typedef struct
{
    CurWeather *curWeather;
    char *cityId;
    int cityIdSize;
    int statusCode;
    ParsedFields parsed;
}WeatherReply;

typedef struct
{
    Forecast *forecast;
    int curItem;
    int maxItems;
    int statusCode;
    ParsedFields parsed;
}ForecastReply;

LOCAL void ICACHE_FLASH_ATTR printWeatherReply(const WeatherReply *reply)
{
    debug("WeatherReply:\n");
    if (reply->parsed & eDatetimeParsed) debug(" dt %u\n", reply->curWeather->datetime);
    if (reply->parsed & eTemperatureParsed) debug(" temp %d\n", floatToIntRound(kelvinToTemp(reply->curWeather->temperature)));
    if (reply->parsed & eDescriptionParsed) debug(" desc %s\n", reply->curWeather->description);
    if (reply->parsed & eIconParsed) debug(" icod %s\n", reply->curWeather->icon.str);
    if (reply->cityId[0]) debug(" cityId %s\n", reply->cityId);
    debug("  parsed 0x%02X\n", reply->parsed);
    debug("  status %d\n", reply->statusCode);
}

void printForecastReply(const ForecastReply *reply, int fullItems)
{
    debug("ForecastReply:\n");
    int i;
    for(i = 0; i < fullItems; i++)
    {
        debug("Item %d:\n", i);
        debug(" dt %u\n", reply->forecast[i].datetime);
        debug(" temp %d\n", floatToIntRound(kelvinToTemp(reply->forecast[i].value.temp / FLOAT_SCALE)));
        debug(" rainsnow %d\n", floatToIntRound(reply->forecast[i].value.rainsnow));
        debug(" icon %s\n", reply->forecast[i].icon.str);
    }
    debug("  parsed 0x%02X\n", reply->parsed);
    debug("  status %d\n", reply->statusCode);
}

LOCAL void onWeatherDatetime(const char *value, int length, int type, void *object);
LOCAL void onWeatherTemp(const char *value, int length, int type, void *object);
LOCAL void onWeatherDescription(const char *value, int length, int type, void *object);
LOCAL void onWeatherIcon(const char *value, int length, int type, void *object);
LOCAL void onWeatherCityId(const char *value, int length, int type, void *object);
LOCAL void onWeatherStatusCode(const char *value, int length, int type, void *object);

LOCAL void onForecastItem(const char *value, int length, int type, void *object);
LOCAL void onForecastDatetime(const char *value, int length, int type, void *object);
LOCAL void onForecastTemp(const char *value, int length, int type, void *object);
LOCAL void onForecastRainsnow(const char *value, int length, int type, void *object);
LOCAL void onForecastIcon(const char *value, int length, int type, void *object);
LOCAL void onForecastStatusCode(const char *value, int length, int type, void *object);

#define WEATHER_CALLBACKS_SIZE      6
LOCAL PathCbPair weatherCallbacks[WEATHER_CALLBACKS_SIZE];

#define FORECAST_CALLBACKS_SIZE     7
LOCAL PathCbPair forecastCallbacks[FORECAST_CALLBACKS_SIZE];

LOCAL int weatherPathsInit = FALSE;
LOCAL int forecastPathsInit = FALSE;

LOCAL void ICACHE_FLASH_ATTR initWeatherPaths(void)
{
    bindPathCb(weatherCallbacks[0], onWeatherDatetime,    "dt");
    bindPathCb(weatherCallbacks[1], onWeatherTemp,        "main", "temp");
    bindPathCb(weatherCallbacks[2], onWeatherDescription, "weather", "[", "description");
    bindPathCb(weatherCallbacks[3], onWeatherIcon,        "weather", "[", "icon");
    bindPathCb(weatherCallbacks[4], onWeatherCityId,      "id");
    bindPathCb(weatherCallbacks[5], onWeatherStatusCode,  "cod");
}

LOCAL void ICACHE_FLASH_ATTR initForecastPaths(void)
{
    bindPathCb(forecastCallbacks[0], onForecastItem,       "list", "[", "{");
    bindPathCb(forecastCallbacks[1], onForecastDatetime,   "list", "[", "dt");
    bindPathCb(forecastCallbacks[2], onForecastTemp,       "list", "[", "main", "temp");
    bindPathCb(forecastCallbacks[3], onForecastRainsnow,   "list", "[", "rain", "3h");
    bindPathCb(forecastCallbacks[4], onForecastRainsnow,   "list", "[", "snow", "3h");
    bindPathCb(forecastCallbacks[5], onForecastIcon,       "list", "[", "weather", "[", "icon");
    bindPathCb(forecastCallbacks[6], onForecastStatusCode, "cod");
}

ParsedFields ICACHE_FLASH_ATTR parseWeather(const char *json, int jsonLen, CurWeather *curWeather, char *cityId, int cityIdSize, int *statusCode)
{
    if (!weatherPathsInit)
    {
        initWeatherPaths();
        weatherPathsInit = TRUE;
    }
    WeatherReply reply = {.curWeather = curWeather, .cityId = cityId, .cityIdSize = cityIdSize, .statusCode = 0, .parsed = 0};
    parsejson(json, jsonLen, weatherCallbacks, NELEMENTS(weatherCallbacks), &reply);
    if (statusCode)
    {
        *statusCode = reply.statusCode;
    }
    printWeatherReply(&reply);
    return reply.parsed;
}

int ICACHE_FLASH_ATTR parseForecast(const char *json, int jsonLen, Forecast *forecast, int forecastSize, int *statusCode)
{
    if (!forecastPathsInit)
    {
        initForecastPaths();
        forecastPathsInit = TRUE;
    }
    ForecastReply reply = {.forecast = forecast, .curItem = -1, .maxItems = forecastSize, .statusCode = 0, .parsed = 0};
    parsejson(json, jsonLen, forecastCallbacks, NELEMENTS(forecastCallbacks), &reply);
    if (statusCode)
    {
        *statusCode = reply.statusCode;
    }
    int fullItems = (reply.parsed & eForecastParsed) == eForecastParsed ? (reply.curItem + 1) :
                                                         reply.curItem < 0 ? 0 : reply.curItem;
    printForecastReply(&reply, fullItems);
    return fullItems;
}


LOCAL void ICACHE_FLASH_ATTR onWeatherDatetime(const char *value, int length, int type, void *object)
{
    //debug("onWeatherDatetime: value %s, len %d, type %c\n", value, length, (char)type);
    if (type == JSON_TYPE_NUMBER)
    {
        WeatherReply *reply = object;
        reply->curWeather->datetime = strtoint(value) + config.utcoffset;
        reply->parsed |= eDatetimeParsed;
    }
}

LOCAL void ICACHE_FLASH_ATTR onWeatherTemp(const char *value, int length, int type, void *object)
{
    //debug("onWeatherTemp: value %s, len %d, type %c\n", value, length, (char)type);
    if (type == JSON_TYPE_NUMBER)
    {
        WeatherReply *reply = object;
        reply->curWeather->temperature = strtofloat(value);
        reply->parsed |= eTemperatureParsed;
    }
}

LOCAL void ICACHE_FLASH_ATTR onWeatherDescription(const char *value, int length, int type, void *object)
{
    //debug("onWeatherDescription: value %s, len %d, type %c\n", value, length, (char)type);
    if (type == JSON_TYPE_STRING)
    {
        WeatherReply *reply = object;
        size_t descSize = sizeof(reply->curWeather->description);
        os_strncpy(reply->curWeather->description, value, descSize-1);
        reply->curWeather->description[descSize-1] = '\0';
        reply->parsed |= eDescriptionParsed;
    }
}

LOCAL void ICACHE_FLASH_ATTR onWeatherIcon(const char *value, int length, int type, void *object)
{
    //debug("onWeatherIcon: value %s, len %d, type %c\n", value, length, (char)type);
    if (type == JSON_TYPE_STRING)
    {
        WeatherReply *reply = object;
        IconId *icon = &reply->curWeather->icon;
        if (length < sizeof(icon->str))
        {
            os_strcpy(icon->str, value);
            reply->parsed |= eIconParsed;
        }
        else
        {
            icon->val = 0;
        }
    }
}

LOCAL void ICACHE_FLASH_ATTR onWeatherCityId(const char *value, int length, int type, void *object)
{
    //debug("onWeatherCityId: value %s, len %d, type %c\n", value, length, (char)type);
    if (type == JSON_TYPE_NUMBER)
    {
        WeatherReply *reply = object;
        if (length < reply->cityIdSize)
        {
            os_strcpy(reply->cityId, value);
        }
        else
        {
            reply->cityId[0] = '\0';
        }
    }
}

LOCAL void ICACHE_FLASH_ATTR onWeatherStatusCode(const char *value, int length, int type, void *object)
{
    //debug("onWeatherStatusCode: value %s, len %d, type %c\n", value, length, (char)type);
    if (type == JSON_TYPE_NUMBER || type == JSON_TYPE_STRING)
    {
        WeatherReply *reply = object;
        reply->statusCode = strtoint(value);
    }
}


LOCAL void ICACHE_FLASH_ATTR onForecastItem(const char *value, int length, int type, void *object)
{
    //debug("\nonForecastItem: value %s, len %d, type %c\n", value, length, (char)type);
    ForecastReply *reply = object;
    reply->parsed = 0;
    if (reply->curItem < reply->maxItems)
    {
        reply->curItem++;
        if (reply->curItem >= 0 && reply->curItem < reply->maxItems)
        {
            memset(&reply->forecast[reply->curItem], 0, sizeof(Forecast));
        }
    }
}

LOCAL void ICACHE_FLASH_ATTR onForecastDatetime(const char *value, int length, int type, void *object)
{
    //debug("onForecastDatetime: value %s, len %d, type %c\n", value, length, (char)type);
    if (type == JSON_TYPE_NUMBER)
    {
        ForecastReply *reply = object;
        if (reply->curItem >= 0 && reply->curItem < reply->maxItems)
        {
            reply->forecast[reply->curItem].datetime = strtoint(value) + config.utcoffset;
            reply->parsed |= eDatetimeParsed;
        }
    }
}

LOCAL void ICACHE_FLASH_ATTR onForecastTemp(const char *value, int length, int type, void *object)
{
    //debug("onForecastTemp: value %s, len %d, type %c\n", value, length, (char)type);
    if (type == JSON_TYPE_NUMBER)
    {
        ForecastReply *reply = object;
        if (reply->curItem >= 0 && reply->curItem < reply->maxItems)
        {
            reply->forecast[reply->curItem].value.temp = strtofloat(value)  *FLOAT_SCALE;
            reply->parsed |= eTemperatureParsed;
        }
    }
}

LOCAL void ICACHE_FLASH_ATTR onForecastRainsnow(const char *value, int length, int type, void *object)
{
    //debug("onForecastRainsnow: value %s, len %d, type %c\n", value, length, (char)type);
    if (type == JSON_TYPE_NUMBER)
    {
        ForecastReply *reply = object;
        if (reply->curItem >= 0 && reply->curItem < reply->maxItems)
        {
            reply->forecast[reply->curItem].value.rainsnow += strtofloat(value)  *FLOAT_SCALE;
            reply->parsed |= eRainsnowParsed;
        }
    }
}

LOCAL void ICACHE_FLASH_ATTR onForecastIcon(const char *value, int length, int type, void *object)
{
    //debug("onForecastIcon: value %s, len %d, type %c\n", value, length, (char)type);
    if (type == JSON_TYPE_STRING)
    {
        ForecastReply *reply = object;
        if (reply->curItem >= 0 && reply->curItem < reply->maxItems)
        {
            IconId *icon = &reply->forecast[reply->curItem].icon;
            if (length < sizeof(icon->str))
            {
                os_strcpy(icon->str, value);
                reply->parsed |= eIconParsed;
            }
            else
            {
                icon->val = 0;
            }
        }
    }
}

LOCAL void ICACHE_FLASH_ATTR onForecastStatusCode(const char *value, int length, int type, void *object)
{
    //debug("onForecastStatusCode: value %s, len %d, type %c\n", value, length, (char)type);
    if (type == JSON_TYPE_NUMBER || type == JSON_TYPE_STRING)
    {
        ForecastReply *reply = object;
        reply->statusCode = strtoint(value);
    }
}
