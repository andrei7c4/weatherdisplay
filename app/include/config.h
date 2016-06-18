#ifndef INCLUDE_CONFIG_H_
#define INCLUDE_CONFIG_H_

#include <spi_flash.h>
#include "typedefs.h"


#define DEFAULT_SSID	""
#define DEFAULT_PASS	""
#define DEFAULT_CITY	""
#define DEFAULT_APPID	""
#define DEFAULT_SP_PUBLICKEY	""
#define DEFAULT_SP_PRIVATEKEY	""

#define CONFIG_SAVE_FLASH_SECTOR	0x3C
#define CONFIG_SAVE_FLASH_ADDR		(CONFIG_SAVE_FLASH_SECTOR * SPI_FLASH_SEC_SIZE)
#define CONFIG_VALID_KEY			0xAABBCCDD

typedef struct{
	uint key;
	uint attempts;
	uint fails;
	uint retry;
	char ssid[36];
	char pass[68];
	char city[24];
	char cityDisplayed[24];
	char cityId[8];
	char appid[36];
	char publickey[24];
	char privatekey[24];
	int utcoffset;
	float tempoffset;
	uint interval;
	uint longSleepCnt;
	uint debug;
}Config;
extern Config config;

void configInit(Config *config);
void configRead(Config *config);
void configWrite(Config *config);



#endif /* INCLUDE_CONFIG_H_ */
