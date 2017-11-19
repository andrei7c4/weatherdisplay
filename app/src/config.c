#include <os_type.h>
#include <osapi.h>
#include <user_interface.h>
#include "config.h"
#include "retain.h"
#include "conv.h"
#include "display.h"
#include "debug.h"


Config config;

#define VALID_MAGIC_NUMBER			0xAABBCCDD

void ICACHE_FLASH_ATTR configInit(Config *config)
{
	os_memset(config, 0, sizeof(Config));
	config->magic = VALID_MAGIC_NUMBER;
	os_strcpy(config->ssid, DEFAULT_SSID);
	os_strcpy(config->pass, DEFAULT_PASS);
	os_strcpy(config->city, DEFAULT_CITY);
	os_strcpy(config->cityDisplayed, DEFAULT_CITY);
	os_strcpy(config->appid, DEFAULT_APPID);
	os_strcpy(config->tsApiKey, DEFAULT_TS_KEY);
	config->utcoffset = 0;
	config->tempoffset = 0;
	config->interval = 30UL*60UL*1000000UL;
	config->chart = 0;
	config->fahrenheit = FALSE;
	config->clock24 = TRUE;
	config->debugEn = FALSE;
}

void ICACHE_FLASH_ATTR configRead(Config *config)
{
	spi_flash_read(CONFIG_SAVE_FLASH_ADDR, (uint*)config, sizeof(Config));
	if (config->magic != VALID_MAGIC_NUMBER)
	{
		os_printf("no valid config in flash\n");
		configInit(config);
		configWrite(config);
	}
	else
	{
		os_printf("valid config found\n");
	}
}

void ICACHE_FLASH_ATTR configWrite(Config *config)
{
	spi_flash_erase_sector(CONFIG_SAVE_FLASH_SECTOR);
	spi_flash_write(CONFIG_SAVE_FLASH_ADDR, (uint*)config, sizeof(Config));
}



LOCAL int ICACHE_FLASH_ATTR setParam(char *param, uint paramSize, const char *value, uint valueLen)
{
	if (!value || !*value || valueLen == 0 || valueLen > paramSize-1)
	{
		return ERROR;
	}
	os_memset(param, 0, paramSize);
	os_memcpy(param, value, valueLen);
	return OK;
}

LOCAL int ICACHE_FLASH_ATTR setSsid(const char *value, uint valueLen)
{
	// also renew ip on ssid change
	os_memset(&retain.ipConfig, 0, sizeof(retain.ipConfig));
	retain.dns1.addr = 0;
	retain.dns2.addr = 0;
	retainWrite(&retain);
	return setParam(config.ssid, sizeof(config.ssid), value, valueLen);
}

LOCAL int ICACHE_FLASH_ATTR setPass(const char *value, uint valueLen)
{
	if (valueLen == 0)
	{
		os_memset(config.pass, 0, sizeof(config.pass));
		return OK;
	}
	return setParam(config.pass, sizeof(config.pass), value, valueLen);
}

LOCAL int ICACHE_FLASH_ATTR setCity(const char *value, uint valueLen)
{
	if (setParam(config.city, sizeof(config.city), value, valueLen) == OK)
	{
		os_memset(config.cityDisplayed, 0, sizeof(config.cityDisplayed));
		int i;
		for (i = 0; i < valueLen; i++)
		{
			if (value[i] == '_')
			{
				config.cityDisplayed[i] = ' ';
			}
			else if (value[i] == ',')
			{
				config.cityDisplayed[i] = '\0';
				break;
			}
			else
			{
				config.cityDisplayed[i] = value[i];
			}
		}
		os_memset(retain.cityId, 0, sizeof(retain.cityId));
		retainWrite(&retain);
		return OK;
	}
	return ERROR;
}

LOCAL int ICACHE_FLASH_ATTR setCitydisp(const char *value, uint valueLen)
{
	if (!value || !*value || valueLen == 0)
	{
		config.cityDisplayed[0] = '\0';
		return OK;
	}
	return setParam(config.cityDisplayed, sizeof(config.cityDisplayed), value, valueLen);
}

LOCAL int ICACHE_FLASH_ATTR setAppid(const char *value, uint valueLen)
{
	return setParam(config.appid, sizeof(config.appid), value, valueLen);
}

LOCAL int ICACHE_FLASH_ATTR setThingSpeakKey(const char *value, uint valueLen)
{
	return setParam(config.tsApiKey, sizeof(config.tsApiKey), value, valueLen);
}

LOCAL int ICACHE_FLASH_ATTR setUtcoffset(const char *value, uint valueLen)
{
	if (!value || !*value || !valueLen)
		return ERROR;

	int offset = strtoint(value);
	if (offset < -720 || offset > 840)
		return ERROR;

	config.utcoffset = offset*60;
	return OK;
}

LOCAL int ICACHE_FLASH_ATTR setTempoffset(const char *value, uint valueLen)
{
	if (!value || !*value || !valueLen)
		return ERROR;

	config.tempoffset = strtofloat(value);
	return OK;
}

LOCAL int ICACHE_FLASH_ATTR setInterval(const char *value, uint valueLen)
{
	if (!value || !*value || !valueLen)
		return ERROR;

	uint interval = strtoint(value);
	if (interval < 1 || interval > 71)
		return ERROR;

	config.interval = interval*60UL*1000000UL;
	return OK;
}

LOCAL int ICACHE_FLASH_ATTR setChart(const char *value, uint valueLen)
{
	if (!value || !*value || !valueLen)
		return ERROR;

	uint chart = strtoint(value);
	if (chart < 0 || chart > 3)
		return ERROR;

	config.chart = chart;
	return OK;
}

LOCAL int ICACHE_FLASH_ATTR setFahrenheit(const char *value, uint valueLen)
{
	if (!value || !*value || !valueLen)
		return ERROR;

	switch (value[0])
	{
	case '0':
		config.fahrenheit = FALSE;
		return OK;
	case '1':
		config.fahrenheit = TRUE;
		return OK;
	default:
		return ERROR;
	}
}

LOCAL int ICACHE_FLASH_ATTR setClock24(const char *value, uint valueLen)
{
	if (!value || !*value || !valueLen)
		return ERROR;

	switch (value[0])
	{
	case '0':
		config.clock24 = FALSE;
		return OK;
	case '1':
		config.clock24 = TRUE;
		return OK;
	default:
		return ERROR;
	}
}

LOCAL int ICACHE_FLASH_ATTR setDebug(const char *value, uint valueLen)
{
	if (!value || !*value || !valueLen)
		return ERROR;

	switch (value[0])
	{
	case '0':
		config.debugEn = FALSE;
		return OK;
	case '1':
		config.debugEn = TRUE;
		return OK;
	default:
		return ERROR;
	}
}

LOCAL int ICACHE_FLASH_ATTR resetConfig(const char *value, uint valueLen)
{
	configInit(&config);
	retainInit(&retain);
	retainWrite(&retain);
	return OK;
}

LOCAL int ICACHE_FLASH_ATTR clearDisp(const char *value, uint valueLen)
{
	dispFillMem(0x00);
	dispUpdate(eDispTopPart);
	dispUpdate(eDispBottomPart);
	while (1);	// let wdt reset
	return OK;
}


typedef struct
{
    const char *cmd;
    int (*func)(const char *value, uint valueLen);
}CmdEntry;

CmdEntry commands[] = {
	{"ssid", setSsid},
	{"pass", setPass},
	{"city", setCity},
	{"citydisp", setCitydisp},
	{"appid", setAppid},
	{"thingspeak", setThingSpeakKey},
	{"utc", setUtcoffset},
	{"temp", setTempoffset},
	{"interval", setInterval},
	{"chart", setChart},
	{"fahrenheit", setFahrenheit},
	{"clock24", setClock24},
	{"debug", setDebug},
	{"reset", resetConfig},
	{"clear", clearDisp}
};

void ICACHE_FLASH_ATTR onUartCmdReceived(char* command, int length)
{
	if (length < 5)
		return;

	char *sep = os_strchr(command, ':');
	if (!sep)
		return;
	*sep = '\0';

	char *value = sep+1;
	int valueLen = os_strlen(value);

	uint i;
	uint nrCmds = sizeof(commands)/sizeof(commands[0]);
	for (i = 0; i < nrCmds; i++)
	{
		if (!os_strcmp(command, commands[i].cmd))
		{
			if (commands[i].func(value, valueLen) == OK)
			{
				os_printf("OK\n");
				configWrite(&config);
				system_restart();
			}
			else
			{
				os_printf("invalid parameter\n");
			}
			return;
		}
	}
	os_printf("command not supported\n");
}
