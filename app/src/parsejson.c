#include <os_type.h>
#include <osapi.h>
#include "contikijson/jsonparse.h"
#include "contikijson/jsontree.h"
#include "parsejson.h"
#include "common.h"
#include "debug.h"
#include "conv.h"
#include "icons.h"
#include "config.h"


LOCAL int ICACHE_FLASH_ATTR jumpToNextType(struct jsonparse_state *state, char *buf, int bufSize, int depth, int type, char *name, int stopOnDepthChange)
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
		else if (state->depth < depth && stopOnDepthChange)
		{
			return 0;
		}
	}
	return 0;
}

LOCAL int ICACHE_FLASH_ATTR jumpToNextType2(struct jsonparse_state *state, char *buf, int bufSize, int depth, int type, char *name1, char *name2, int stopOnDepthChange)
{
	int json_type;
	while((json_type = jsonparse_next(state)) != 0)
	{
		if (depth == state->depth && json_type == type)
		{
			jsonparse_copy_value(state, buf, bufSize);
			if (!os_strncmp(buf, name1, bufSize) ||
				!os_strncmp(buf, name2, bufSize))
			{
				return 1;
			}
		}
		else if (state->depth < depth && stopOnDepthChange)
		{
			return 0;
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
			1, JSON_TYPE_PAIR_NAME, "weather", 0))
		return ERROR;

	if (!jumpToNextType(&state, buf, sizeof(buf),
			2, JSON_TYPE_ARRAY, NULL, 0))
		return ERROR;

//	if (!jumpToNextType(&state, buf, sizeof(buf),
//			3, JSON_TYPE_OBJECT, NULL, 0))
//		return ERROR;

	if (!jumpToNextType(&state, buf, sizeof(buf),
			3, JSON_TYPE_PAIR_NAME, "description", 0))
		return ERROR;

	if (jsonparse_next(&state) != JSON_TYPE_STRING)
		return ERROR;

	jsonparse_copy_value(&state, buf, sizeof(buf));
	debug("weather description: %s\n", buf);
	os_strncpy(curWeather->description, buf, sizeof(curWeather->description)-1);
	curWeather->description[sizeof(curWeather->description)-1] = '\0';

	if (!jumpToNextType(&state, buf, sizeof(buf),
			3, JSON_TYPE_PAIR_NAME, "icon", 0))
		return ERROR;

	if (jsonparse_next(&state) != JSON_TYPE_STRING)
		return ERROR;

	jsonparse_copy_value(&state, buf, sizeof(buf));
	debug("weather icon: %s\n", buf);
	if (os_strlen(buf) < sizeof(curWeather->icon))
	{
		os_strcpy(curWeather->icon, buf);
	}
	else
	{
		os_strcpy(curWeather->icon, "");
	}
	//curWeather->icon = iconIdToImage(buf, 1);

	if (!jumpToNextType(&state, buf, sizeof(buf),
			1, JSON_TYPE_PAIR_NAME, "main", 0))
		return ERROR;

//	if (!jumpToNextType(&state, buf, sizeof(buf),
//			2, JSON_TYPE_OBJECT, NULL, 0))
//		return ERROR;

	if (!jumpToNextType(&state, buf, sizeof(buf),
			2, JSON_TYPE_PAIR_NAME, "temp", 0))
		return ERROR;

	if (jsonparse_next(&state) != JSON_TYPE_NUMBER)
		return ERROR;

	jsonparse_copy_value(&state, buf, sizeof(buf));
	curWeather->temperature = strtofloat(buf);
	debug("temperature: %s\n", buf);

	if (!jumpToNextType(&state, buf, sizeof(buf),
			1, JSON_TYPE_PAIR_NAME, "dt", 0))
		return ERROR;

	if (jsonparse_next(&state) != JSON_TYPE_NUMBER)
		return ERROR;

	jsonparse_copy_value(&state, buf, sizeof(buf));
	curWeather->datetime = strtoint(buf);
	debug("datetime: %s\n", buf);

	if (!retain.cityId[0])
	{
		if (!jumpToNextType(&state, buf, sizeof(buf),
				1, JSON_TYPE_PAIR_NAME, "id", 0))
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

int ICACHE_FLASH_ATTR parseForecast(char *data, Forecast *forecast, ForecastType forecastType)
{
	char buf[128];
	int json_type;
	struct jsonparse_state state;
	int count;
	char *tempObj;
	switch (forecastType)
	{
	case eForecastHourly:
		count = FORECAST_HOURLY_CNT;
		tempObj = "main";
		break;
	case eForecastDaily:
		count = FORECAST_DAILY_CNT;
		tempObj = "temp";
		break;
	default:
		return ERROR;
	}


	jsonparse_setup(&state, data, os_strlen(data));

	if (!jumpToNextType(&state, buf, sizeof(buf),
			1, JSON_TYPE_PAIR_NAME, "cnt", 0))
		return ERROR;

	if (jsonparse_next(&state) != JSON_TYPE_NUMBER ||
		jsonparse_get_value_as_int(&state) != count)
		return ERROR;

	if (!jumpToNextType(&state, buf, sizeof(buf),
			1, JSON_TYPE_PAIR_NAME, "list", 0))
		return ERROR;

	if (!jumpToNextType(&state, buf, sizeof(buf),
			2, JSON_TYPE_ARRAY, NULL, 0))
		return ERROR;

	int i;
	for (i = 0; i < count; i++)
	{
//		if (!jumpToNextType(&state, buf, sizeof(buf),
//				3, JSON_TYPE_OBJECT, NULL), 0)
//			return ERROR;

		if (!jumpToNextType(&state, buf, sizeof(buf),
				3, JSON_TYPE_PAIR_NAME, "dt", 0))
			return ERROR;

		if (jsonparse_next(&state) != JSON_TYPE_NUMBER)
			return ERROR;

		jsonparse_copy_value(&state, buf, sizeof(buf));
		forecast[i].datetime = strtoint(buf);
		debug("forecast[%d] dt: %s\n", i, buf);

		if (!jumpToNextType(&state, buf, sizeof(buf),
				3, JSON_TYPE_PAIR_NAME, tempObj, 0))
			return ERROR;

//		if (!jumpToNextType(&state, buf, sizeof(buf),
//				4, JSON_TYPE_OBJECT, NULL, 0))
//			return ERROR;

		switch (forecastType)
		{
		case eForecastHourly:
			if (!jumpToNextType(&state, buf, sizeof(buf),
					4, JSON_TYPE_PAIR_NAME, "temp", 0))
				return ERROR;

			if (jsonparse_next(&state) != JSON_TYPE_NUMBER)
				return ERROR;

			jsonparse_copy_value(&state, buf, sizeof(buf));
			forecast[i].temp = strtofloat(buf);
			debug("forecast[%d] temp: %s\n", i, buf);
			break;
		case eForecastDaily:
			if (!jumpToNextType(&state, buf, sizeof(buf),
					4, JSON_TYPE_PAIR_NAME, "min", 0))
				return ERROR;

			if (jsonparse_next(&state) != JSON_TYPE_NUMBER)
				return ERROR;

			jsonparse_copy_value(&state, buf, sizeof(buf));
			forecast[i].minTemp = strtofloat(buf);
			debug("forecast[%d] min: %s\n", i, buf);

			if (!jumpToNextType(&state, buf, sizeof(buf),
					4, JSON_TYPE_PAIR_NAME, "max", 0))
				return ERROR;

			if (jsonparse_next(&state) != JSON_TYPE_NUMBER)
				return ERROR;

			jsonparse_copy_value(&state, buf, sizeof(buf));
			forecast[i].maxTemp = strtofloat(buf);
			debug("forecast[%d] max: %s\n", i, buf);
			break;
		}

		if (!jumpToNextType(&state, buf, sizeof(buf),
				3, JSON_TYPE_PAIR_NAME, "weather", 0))
			return ERROR;

		if (!jumpToNextType(&state, buf, sizeof(buf),
				4, JSON_TYPE_ARRAY, NULL, 0))
			return ERROR;

//		if (!jumpToNextType(&state, buf, sizeof(buf),
//				5, JSON_TYPE_OBJECT, NULL, 0))
//			return ERROR;

		if (!jumpToNextType(&state, buf, sizeof(buf),
				5, JSON_TYPE_PAIR_NAME, "icon", 0))
			return ERROR;

		if (jsonparse_next(&state) != JSON_TYPE_STRING)
			return ERROR;

		jsonparse_copy_value(&state, buf, sizeof(buf));
		debug("forecast[%d] icon: %s\n", i, buf);
		if (os_strlen(buf) < sizeof(forecast[i].icon))
		{
			os_strcpy(forecast[i].icon, buf);
		}
		else
		{
			os_strcpy(forecast[i].icon, "");
		}
		//forecast[i].icon = iconIdToImage(buf, 0);

		forecast[i].rainsnow = 0;
		switch (forecastType)
		{
		case eForecastHourly:
			while (jumpToNextType2(&state, buf, sizeof(buf),
					3, JSON_TYPE_PAIR_NAME, "rain", "snow", 1))
			{
//				if (jumpToNextType(&state, buf, sizeof(buf),
//						4, JSON_TYPE_OBJECT, NULL, 1))
//				{
					if (jumpToNextType(&state, buf, sizeof(buf),
							4, JSON_TYPE_PAIR_NAME, "3h", 1))
					{
						if (jsonparse_next(&state) == JSON_TYPE_NUMBER)
						{
							jsonparse_copy_value(&state, buf, sizeof(buf));
							forecast[i].rainsnow += strtofloat(buf);
							debug("forecast[%d] rain: %s\n", i, buf);
						}
					}
//				}
			}
			break;
		case eForecastDaily:
			while (jumpToNextType2(&state, buf, sizeof(buf),
					3, JSON_TYPE_PAIR_NAME, "rain", "snow", 1))
			{
				if (jsonparse_next(&state) == JSON_TYPE_NUMBER)
				{
					jsonparse_copy_value(&state, buf, sizeof(buf));
					forecast[i].rainsnow += strtofloat(buf);
					debug("forecast[%d] rain: %s\n", i, buf);
				}
			}
			break;
		}
	}
	return OK;
}
