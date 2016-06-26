#include <os_type.h>
#include <osapi.h>
#include "contikijson/jsonparse.h"
#include "contikijson/jsontree.h"
#include "common.h"
#include "debug.h"
#include "conv.h"
#include "icons.h"
#include "config.h"

LOCAL const uint* ICACHE_FLASH_ATTR iconIdToImage(char *iconId, int big)
{
	if (!os_strcmp(iconId,"01d")) return big ? big_01d : small_01d;
	else if (!os_strcmp(iconId,"01n")) return big ? big_01n : small_01n;
	else if (!os_strcmp(iconId,"02d")) return big ? big_02d : small_02d;
	else if (!os_strcmp(iconId,"02n")) return big ? big_02n : small_02n;
	else if (!os_strcmp(iconId,"03d")) return big ? big_03d : small_03d;
	else if (!os_strcmp(iconId,"03n")) return big ? big_03d : small_03d;
	else if (!os_strcmp(iconId,"04d")) return big ? big_04d : small_04d;
	else if (!os_strcmp(iconId,"04n")) return big ? big_04d : small_04d;
	else if (!os_strcmp(iconId,"09d")) return big ? big_09d : small_09d;
	else if (!os_strcmp(iconId,"09n")) return big ? big_09d : small_09d;
	else if (!os_strcmp(iconId,"10d")) return big ? big_10d : small_10d;
	else if (!os_strcmp(iconId,"10n")) return big ? big_10d : small_10d;
	else if (!os_strcmp(iconId,"11d")) return big ? big_11d : small_11d;
	else if (!os_strcmp(iconId,"11n")) return big ? big_11d : small_11d;
	else if (!os_strcmp(iconId,"13d")) return big ? big_13d : small_13d;
	else if (!os_strcmp(iconId,"13n")) return big ? big_13d : small_13d;
	else if (!os_strcmp(iconId,"50d")) return big ? big_50d : small_50d;
	else if (!os_strcmp(iconId,"50n")) return big ? big_50n : small_50n;
	return NULL;
}

LOCAL int ICACHE_FLASH_ATTR jumpToNextType(struct jsonparse_state *state, char *buf, int bufSize, int depth, int type, char *name)
{
	int json_type;
	while((json_type = jsonparse_next(state)) != 0)
	{
		if (depth == state->depth && json_type == type)
		{
			if (name)
			{
				jsonparse_copy_value(state, buf, bufSize);
				if (!os_strncmp(buf, name, bufSize))
				{
					return 1;
				}
			}
			else
			{
				return 1;
			}
		}
	}
	return 0;
}

int ICACHE_FLASH_ATTR parseWeather(char *data, CurWeather *curWeather)
{
	char buf[128];
	int json_type;
	struct jsonparse_state state;

	jsonparse_setup(&state, data, os_strlen(data));

	if (!jumpToNextType(&state, buf, sizeof(buf),
			1, JSON_TYPE_PAIR_NAME, "weather"))
		return ERROR;

	if (!jumpToNextType(&state, buf, sizeof(buf),
			2, JSON_TYPE_ARRAY, NULL))
		return ERROR;

	if (!jumpToNextType(&state, buf, sizeof(buf),
			3, JSON_TYPE_OBJECT, NULL))
		return ERROR;

	if (!jumpToNextType(&state, buf, sizeof(buf),
			3, JSON_TYPE_PAIR_NAME, "description"))
		return ERROR;

	if (jsonparse_next(&state) != JSON_TYPE_STRING)
		return ERROR;

	jsonparse_copy_value(&state, buf, sizeof(buf));
	debug("weather description: %s\n", buf);
	os_strncpy(curWeather->description, buf, sizeof(curWeather->description)-1);
	curWeather->description[sizeof(curWeather->description)-1] = '\0';

	if (!jumpToNextType(&state, buf, sizeof(buf),
			3, JSON_TYPE_PAIR_NAME, "icon"))
		return ERROR;

	if (jsonparse_next(&state) != JSON_TYPE_STRING)
		return ERROR;

	jsonparse_copy_value(&state, buf, sizeof(buf));
	debug("weather icon: %s\n", buf);
	curWeather->icon = iconIdToImage(buf, 1);

	if (!jumpToNextType(&state, buf, sizeof(buf),
			1, JSON_TYPE_PAIR_NAME, "main"))
		return ERROR;

	if (!jumpToNextType(&state, buf, sizeof(buf),
			2, JSON_TYPE_OBJECT, NULL))
		return ERROR;

	if (!jumpToNextType(&state, buf, sizeof(buf),
			2, JSON_TYPE_PAIR_NAME, "temp"))
		return ERROR;

	if (jsonparse_next(&state) != JSON_TYPE_NUMBER)
		return ERROR;

	jsonparse_copy_value(&state, buf, sizeof(buf));
	curWeather->temperature = strtofloat(buf);
	debug("temperature: %s\n", buf);

	if (!jumpToNextType(&state, buf, sizeof(buf),
			1, JSON_TYPE_PAIR_NAME, "dt"))
		return ERROR;

	if (jsonparse_next(&state) != JSON_TYPE_NUMBER)
		return ERROR;

	jsonparse_copy_value(&state, buf, sizeof(buf));
	curWeather->datetime = strtoint(buf);
	debug("datetime: %s\n", buf);

	if (!retain.cityId[0])
	{
		if (!jumpToNextType(&state, buf, sizeof(buf),
				1, JSON_TYPE_PAIR_NAME, "id"))
			return ERROR;

		if (jsonparse_next(&state) != JSON_TYPE_NUMBER)
			return ERROR;

		jsonparse_copy_value(&state, buf, sizeof(buf));
		if (os_strlen(buf) >= sizeof(retain.cityId))
			return ERROR;

		os_strcpy(retain.cityId, buf);
		debug("cityId: %s\n", retain.cityId);
	}
	return OK;
}

int ICACHE_FLASH_ATTR parseForecast(char *data, Forecast *forecast)
{
	char buf[128];
	int json_type;
	struct jsonparse_state state;

	jsonparse_setup(&state, data, os_strlen(data));

	if (!jumpToNextType(&state, buf, sizeof(buf),
			1, JSON_TYPE_PAIR_NAME, "cnt"))
		return ERROR;

	if (jsonparse_next(&state) != JSON_TYPE_NUMBER ||
		jsonparse_get_value_as_int(&state) != FORECAST_DAYS)
		return ERROR;

	if (!jumpToNextType(&state, buf, sizeof(buf),
			1, JSON_TYPE_PAIR_NAME, "list"))
		return ERROR;

	if (!jumpToNextType(&state, buf, sizeof(buf),
			2, JSON_TYPE_ARRAY, NULL))
		return ERROR;

	int i;
	for (i = 0; i < FORECAST_DAYS; i++)
	{
		if (!jumpToNextType(&state, buf, sizeof(buf),
				3, JSON_TYPE_OBJECT, NULL))
			return ERROR;

		if (!jumpToNextType(&state, buf, sizeof(buf),
				3, JSON_TYPE_PAIR_NAME, "dt"))
			return ERROR;

		if (jsonparse_next(&state) != JSON_TYPE_NUMBER)
			return ERROR;

		jsonparse_copy_value(&state, buf, sizeof(buf));
		forecast[i].datetime = strtoint(buf);
		debug("forecast[%d] dt: %s\n", i, buf);

		if (!jumpToNextType(&state, buf, sizeof(buf),
				3, JSON_TYPE_PAIR_NAME, "temp"))
			return ERROR;

		if (!jumpToNextType(&state, buf, sizeof(buf),
				4, JSON_TYPE_OBJECT, NULL))
			return ERROR;

		if (!jumpToNextType(&state, buf, sizeof(buf),
				4, JSON_TYPE_PAIR_NAME, "min"))
			return ERROR;

		if (jsonparse_next(&state) != JSON_TYPE_NUMBER)
			return ERROR;

		jsonparse_copy_value(&state, buf, sizeof(buf));
		forecast[i].minTemp = strtofloat(buf);
		debug("forecast[%d] min: %s\n", i, buf);

		if (!jumpToNextType(&state, buf, sizeof(buf),
				4, JSON_TYPE_PAIR_NAME, "max"))
			return ERROR;

		if (jsonparse_next(&state) != JSON_TYPE_NUMBER)
			return ERROR;

		jsonparse_copy_value(&state, buf, sizeof(buf));
		forecast[i].maxTemp = strtofloat(buf);
		debug("forecast[%d] max: %s\n", i, buf);

		if (!jumpToNextType(&state, buf, sizeof(buf),
				3, JSON_TYPE_PAIR_NAME, "weather"))
			return ERROR;

		if (!jumpToNextType(&state, buf, sizeof(buf),
				4, JSON_TYPE_ARRAY, NULL))
			return ERROR;

		if (!jumpToNextType(&state, buf, sizeof(buf),
				5, JSON_TYPE_OBJECT, NULL))
			return ERROR;

		if (!jumpToNextType(&state, buf, sizeof(buf),
				5, JSON_TYPE_PAIR_NAME, "icon"))
			return ERROR;

		if (jsonparse_next(&state) != JSON_TYPE_STRING)
			return ERROR;

		jsonparse_copy_value(&state, buf, sizeof(buf));
		forecast[i].icon = iconIdToImage(buf, 0);
		debug("forecast[%d] icon: %s\n", i, buf);
	}
	return OK;
}
