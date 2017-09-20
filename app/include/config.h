#ifndef INCLUDE_CONFIG_H_
#define INCLUDE_CONFIG_H_

#include <spi_flash.h>
#include <ip_addr.h>
#include "typedefs.h"


#define DEFAULT_SSID	""
#define DEFAULT_PASS	""
#define DEFAULT_CITY	""
#define DEFAULT_APPID	""
#define DEFAULT_TS_KEY	""

#define CONFIG_SAVE_FLASH_SECTOR	0x3C
#define CONFIG_SAVE_FLASH_ADDR		(CONFIG_SAVE_FLASH_SECTOR * SPI_FLASH_SEC_SIZE)
#define VALID_MAGIC_NUMBER			0xAABBCCDD
#define RTC_USER_DATA_ADDR			64

typedef struct{
	uint magic;
	char ssid[36];
	char pass[68];
	char city[24];
	char cityDisplayed[24];
	char appid[36];
	char tsApiKey[24];
	int utcoffset;
	float tempoffset;
	uint interval;
	uint chart;
	uint fahrenheit;
	uint clock24;
	uint debugEn;
}Config;
extern Config config;

typedef struct{
	uint magic;
	struct ip_info ipConfig;
	ip_addr_t dns1, dns2;
	uint attempts;
	uint fails;
	uint retry;
	uint longSleepCnt;
	char cityId[8];
}Retain;
extern Retain retain;

void configInit(Config *config);
void configRead(Config *config);
void configWrite(Config *config);

void retainInit(Retain *retain);
void retainRead(Retain *retain);
void retainWrite(Retain *retain);


#endif /* INCLUDE_CONFIG_H_ */
