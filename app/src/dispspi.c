#include <ets_sys.h>
#include <osapi.h>
#include <os_type.h>
#include <user_interface.h>
#include <gpio.h>
#include <mem.h>
#include "drivers/spi.h"
#include "dispspi.h"
#include "display.h"
#include "config.h"
#include "debug.h"

static const uchar pd74Header[16] = {0x3A,0x01,0xE0,0x03,0x20,0x01,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
static const uint16 pd74cmdOkRv = 0x9000;
static uint curLine = 0;

static volatile os_timer_t busyCheckTmr;
extern void dispUpdateDoneCb(uint16 rv);

static void ICACHE_FLASH_ATTR pd74SendCmdBegin(uchar ins, uchar p1, uchar p2, int lcle)
{
	while (!GPIO_INPUT_GET(BUSY_GPIO))
	{
		debug("busy!\n");
	}
	GPIO_OUTPUT_SET(CS_GPIO, 0);
	os_delay_us(10);	// Ts
	spi_tx8(HSPI, ins);
	spi_tx8(HSPI, p1);
	spi_tx8(HSPI, p2);
	if (lcle >= 0)
	{
		spi_tx8(HSPI, lcle);
	}
}

static uint16 ICACHE_FLASH_ATTR pd74SendCmdExec(void)
{
	// execute current command
	os_delay_us(15);	// Te
	GPIO_OUTPUT_SET(CS_GPIO, 1);
	os_delay_us(10);	// Ta
	while (!GPIO_INPUT_GET(BUSY_GPIO));
	os_delay_us(5);		// Tns

	// get respons
	GPIO_OUTPUT_SET(CS_GPIO, 0);
	os_delay_us(10);	// Ts

	uint16 rv = spi_rx16(HSPI);

	os_delay_us(15);	// Te
	GPIO_OUTPUT_SET(CS_GPIO, 1);
	os_delay_us(10);	// Ta
	while (!GPIO_INPUT_GET(BUSY_GPIO));
	os_delay_us(5);		// Tns

	return rv;
}

static uint16 ICACHE_FLASH_ATTR pd74SendCmd(uchar ins, uchar p1, uchar p2, uchar *data, int lc, uchar *resp, int le)
{
	while (!GPIO_INPUT_GET(BUSY_GPIO))
	{
		debug("busy!\n");
	}
	GPIO_OUTPUT_SET(CS_GPIO, 0);
	os_delay_us(10);	// Ts
	spi_tx8(HSPI, ins);
	spi_tx8(HSPI, p1);
	spi_tx8(HSPI, p2);

	if (data && lc >= 0)
	{
		spi_tx8(HSPI, lc);
		while (lc > 0)
		{
			spi_tx8(HSPI, *data);
			data++;
			lc--;
		}
	}

	if (le >= 0)
	{
		spi_tx8(HSPI, le);
	}

	os_delay_us(15);	// Te
	GPIO_OUTPUT_SET(CS_GPIO, 1);
	os_delay_us(10);	// Ta
	while (!GPIO_INPUT_GET(BUSY_GPIO));
	os_delay_us(5);		// Tns

	// get respons
	GPIO_OUTPUT_SET(CS_GPIO, 0);
	os_delay_us(10);	// Ts

	if (le == 0)
	{
		if (resp)
		{
			while ((*resp = spi_rx8(HSPI)) != 0)
			{
				resp++;
			}
		}
		else
		{
			while (spi_rx8(HSPI) != 0);
		}
	}
	else if (le > 0)
	{
		if (resp)
		{
			while (le > 0)
			{
				*resp = spi_rx8(HSPI);
				resp++;
				le--;
			}
		}
		else
		{
			while (le > 0)
			{
				spi_rx8(HSPI);
				le--;
			}
		}
	}

	uint16 rv = spi_rx16(HSPI);

	os_delay_us(15);	// Te
	GPIO_OUTPUT_SET(CS_GPIO, 1);
	os_delay_us(10);	// Ta
	while (!GPIO_INPUT_GET(BUSY_GPIO));
	os_delay_us(5);		// Tns

	return rv;
}

void ICACHE_FLASH_ATTR dispBeginUpload(void)
{
	curLine = 0;

	// send ResetDataPointer command
	uint16 rv = pd74SendCmd(0x20, 0x0D, 0x00, 0, -1, 0, -1);
	if (rv != pd74cmdOkRv)
	{
		debug("ResetDataPointer: %x\n", rv);
		return;
	}

	// send UploadImageData command with EPD header
	rv = pd74SendCmd(0x20, 0x01, 0x00, (uchar*)pd74Header, sizeof(pd74Header), 0, -1);
	if (rv != pd74cmdOkRv)
	{
		debug("UploadImageData header: %x\n", rv);
	}
}

static void ICACHE_FLASH_ATTR waitTillNotBusy(void)
{
	while (!GPIO_INPUT_GET(BUSY_GPIO));
	os_delay_us(5);		// Tns

	// get respons
	GPIO_OUTPUT_SET(CS_GPIO, 0);
	os_delay_us(10);	// Ts
	uint16 rv = spi_rx16(HSPI);
	os_delay_us(15);	// Te
	GPIO_OUTPUT_SET(CS_GPIO, 1);
	os_delay_us(10);	// Ta
	while (!GPIO_INPUT_GET(BUSY_GPIO));
	os_delay_us(5);		// Tns

	dispUpdateDoneCb(rv);
}

void ICACHE_FLASH_ATTR dispFinalizeUpload(void)
{
	// send DisplayUpdate command
	pd74SendCmdBegin(0x24, 0x01, 0x00, -1);
	// execute command
	os_delay_us(15);	// Te
	GPIO_OUTPUT_SET(CS_GPIO, 1);
	os_delay_us(10);	// Ta

	debug("DisplayUpdate\n");
	os_timer_setfn(&busyCheckTmr, (os_timer_func_t *)waitTillNotBusy, NULL);
	os_timer_arm(&busyCheckTmr, DISP_REFRESH_TIME_MS, 0);
}

static void ICACHE_FLASH_ATTR spiSendData32(uint *data, uint size)
{
	uint i;
	for (i = 0; i < size; i++)
	{
		spi_tx32(HSPI, *data);
		data++;
	}
}

void ICACHE_FLASH_ATTR dispUploadLine(uchar *line)
{
	uint16 rv;
	switch (curLine)	// we can upload up to 4 lines (240 B) in one command
	{
	case 0:
		// initiate UploadImageData command
		pd74SendCmdBegin(0x20, 1, 0, DISP_MEMWIDTH*4);
		spiSendData32((uint*)line, DISP_MEMWIDTH/4);
		curLine++;
		break;
	case 1:
	case 2:
		spiSendData32((uint*)line, DISP_MEMWIDTH/4);
		curLine++;
		break;
	case 3:
		spiSendData32((uint*)line, DISP_MEMWIDTH/4);
		// execute UploadImageData command
		rv = pd74SendCmdExec();
		if (rv != pd74cmdOkRv)
		{
			debug("UploadImageData: %x\n", rv);
		}
		curLine = 0;
		break;
	}
}

void ICACHE_FLASH_ATTR dispPrintDeviceInfo(void)
{
	char *infostr = (char*)os_zalloc(50);
	uint16 rv = pd74SendCmd(0x30, 0x01, 0x01, 0, -1, infostr, 0);
	debug("info: %s, rv: %x\n", infostr, rv);
	os_free(infostr);
}

void ICACHE_FLASH_ATTR dispReadTemp(char *tempstr)
{
	const float scale[4] = {0.66, 0.52, 0.43, 0.39};
	const float offset[4] = {-19.69, -13.95, -8.55, -4.75};
	uint16 sensor;
	uint16 rv = pd74SendCmd(0xE5, 0x01, 0x00, NULL, 0, (uchar*)&sensor, sizeof(sensor));
	if (rv != pd74cmdOkRv)
	{
		debug("dispReadTemp rv %x\n", rv);
		os_strcpy(tempstr, "");
		return;
	}
	sensor >>= 8;
	debug("dispReadTemp sensor %u\n", sensor);

	int range;
	if (sensor < 30)
	{
		os_strcpy(tempstr, "<0");
		return;
	}
	else if (sensor >= 30 && sensor <= 41) range = 0;
	else if (sensor >= 42 && sensor <= 61) range = 1;
	else if (sensor >= 62 && sensor <= 96) range = 2;
	else if (sensor >= 97 && sensor <= 160) range = 3;
	else
	{
		os_strcpy(tempstr, ">58");
		return;
	}

	float temp = (float)sensor * scale[range] + offset[range] + config.tempoffset + 0.05;
	int integer = (int)temp;
	int fract = (int)((temp-integer)*10.0);
	os_sprintf(tempstr, "%d.%d", integer, fract);
}

void ICACHE_FLASH_ATTR dispSpiInit(void)
{
	// Configure pins as a GPIO
	PIN_FUNC_SELECT(CS_GPIO_MUX, CS_GPIO_FUNC);
	PIN_FUNC_SELECT(BUSY_GPIO_MUX, BUSY_GPIO_FUNC);
	PIN_FUNC_SELECT(EN_GPIO_MUX, EN_GPIO_FUNC);

	GPIO_OUTPUT_SET(CS_GPIO, 1);
	GPIO_OUTPUT_SET(EN_GPIO, 1);

	spi_init(HSPI);

	// Data is valid on clock trailing edge
	// Clock is high when inactive
	spi_mode(HSPI, 1, 1);

	spi_tx_byte_order(HSPI, SPI_BYTE_ORDER_LOW_TO_HIGH);

	GPIO_OUTPUT_SET(EN_GPIO, 0);
}

void ICACHE_FLASH_ATTR dispOff(void)
{
	GPIO_OUTPUT_SET(EN_GPIO, 1);
}
