#include <ets_sys.h>
#include <osapi.h>
#include <os_type.h>
#include <user_interface.h>
#include <gpio.h>
#include <mem.h>
#include "drivers/spi.h"
#include "displays/pd74spi.h"
#include "display.h"
#include "graphics.h"
#include "config.h"
#include "debug.h"


// IO pins used for display control (in addition to MOSI/MISO)
#define CS_GPIO			15
#define CS_GPIO_MUX		PERIPHS_IO_MUX_MTDO_U
#define CS_GPIO_FUNC	FUNC_GPIO15

#define BUSY_GPIO		4
#define BUSY_GPIO_MUX	PERIPHS_IO_MUX_GPIO4_U
#define BUSY_GPIO_FUNC	FUNC_GPIO4

#define EN_GPIO			5
#define EN_GPIO_MUX		PERIPHS_IO_MUX_GPIO5_U
#define EN_GPIO_FUNC	FUNC_GPIO5

// display controller registers
#define CMD_UPLOAD_IMG_INS	0x20
#define CMD_UPLOAD_IMG_P1	0x01
#define CMD_UPLOAD_IMG_P2	0x00

#define CMD_RESET_DP_INS	0x20
#define CMD_RESET_DP_P1		0x0D
#define CMD_RESET_DP_P2		0x00

#define CMD_DISP_UPDATE_INS	0x24
#define CMD_DISP_UPDATE_P1	0x01
#define CMD_DISP_UPDATE_P2	0x00

#define CMD_DEV_INFO_INS	0x30
#define CMD_DEV_INFO_P1		0x01
#define CMD_DEV_INFO_P2		0x01

#define CMD_READ_SENSOR_INS	0xE5
#define CMD_READ_SENSOR_P1	0x01
#define CMD_READ_SENSOR_P2	0x00


static const uchar pd74Header[16] = {0x3A,0x01,0xE0,0x03,0x20,0x01,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
static const uint16 pd74cmdOkRv = 0x9000;
static uint curLine = 0;

extern void dispUpdateDoneCb(int rv);

static void pd74UploadLine(uchar *line);
static void pd74ConverLine(uchar *in, uchar *out);


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

static void ICACHE_FLASH_ATTR checkIfBusy(void)
{
	if (!GPIO_INPUT_GET(BUSY_GPIO))
	{
		os_timer_arm(&busyCheckTmr, 100, 0);
		return;
	}
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

	dispUpdateDoneCb(rv == pd74cmdOkRv ? OK : ERROR);
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



void ICACHE_FLASH_ATTR pd74SpiInit(void)
{
	// Configure pins as a GPIO
	PIN_FUNC_SELECT(CS_GPIO_MUX, CS_GPIO_FUNC);
	PIN_FUNC_SELECT(BUSY_GPIO_MUX, BUSY_GPIO_FUNC);
	PIN_FUNC_SELECT(EN_GPIO_MUX, EN_GPIO_FUNC);

	GPIO_OUTPUT_SET(CS_GPIO, 1);
	GPIO_OUTPUT_SET(EN_GPIO, 1);

	HSpiPins pins = {.miso = TRUE, .mosi = TRUE, .cs = FALSE};
	spi_init(HSPI, 10, 3, pins);

	// Data is valid on clock trailing edge
	// Clock is high when inactive
	spi_mode(HSPI, 1, 1);

	spi_tx_byte_order(HSPI, SPI_BYTE_ORDER_LOW_TO_HIGH);

	GPIO_OUTPUT_SET(EN_GPIO, 0);
}

void ICACHE_FLASH_ATTR pd74UploadBegin(void)
{
	curLine = 0;

	// send ResetDataPointer command
	uint16 rv = pd74SendCmd(CMD_RESET_DP_INS, 
							CMD_RESET_DP_P1, 
							CMD_RESET_DP_P2, 
							NULL, -1, NULL, -1);
	if (rv != pd74cmdOkRv)
	{
		debug("ResetDataPointer: %x\n", rv);
		return;
	}

	// send UploadImageData command with EPD header
	rv = pd74SendCmd(CMD_UPLOAD_IMG_INS, 
					CMD_UPLOAD_IMG_P1, 
					CMD_UPLOAD_IMG_P2, 
					(uchar*)pd74Header, sizeof(pd74Header), NULL, -1);
	if (rv != pd74cmdOkRv)
	{
		debug("UploadImageData header: %x\n", rv);
	}
}

void ICACHE_FLASH_ATTR pd74UploadPerform(void)
{
	uint i;
	uchar line[GFXMEM_BYTEWIDTH];
	for (i = 0; i < GFXMEM_HEIGHT; i++)
	{
		pd74ConverLine(gfxMem + GFXMEM_BYTEWIDTH*i, line);
		pd74UploadLine(line);
	}
}

void ICACHE_FLASH_ATTR pd74UploadFinalize(void)
{
	// send DisplayUpdate command
	pd74SendCmdBegin(CMD_DISP_UPDATE_INS,
					CMD_DISP_UPDATE_P1,
					CMD_DISP_UPDATE_P2, -1);
	// execute command
	os_delay_us(15);	// Te
	GPIO_OUTPUT_SET(CS_GPIO, 1);
	os_delay_us(10);	// Ta

	debug("DisplayUpdate\n");
	os_timer_setfn(&busyCheckTmr, (os_timer_func_t *)checkIfBusy, NULL);
	os_timer_arm(&busyCheckTmr, 1000, 0);
}

void ICACHE_FLASH_ATTR pd74PrintDeviceInfo(void)
{
	char infostr[50] = "";
	uint16 rv = pd74SendCmd(CMD_DEV_INFO_INS,
							CMD_DEV_INFO_P1,
							CMD_DEV_INFO_P2,
							NULL, -1, infostr, 0);
	if (rv == pd74cmdOkRv)
	{
		debug("devinfo: %s\n", infostr);
	}
	else
	{
		debug("devinfo rv: %x\n", rv);
	}
}

int ICACHE_FLASH_ATTR pd74ReadTemp(float *temp)
{
	const float scale[4] = {0.66, 0.52, 0.43, 0.39};
	const float offset[4] = {-19.69, -13.95, -8.55, -4.75};
	uint16 sensor;
	uint16 rv = pd74SendCmd(CMD_READ_SENSOR_INS,
							CMD_READ_SENSOR_P1,
							CMD_READ_SENSOR_P2,
							NULL, 0, (uchar*)&sensor, sizeof(sensor));
	if (rv != pd74cmdOkRv)
	{
		debug("dispReadTemp rv %x\n", rv);
		return ERROR;
	}
	sensor >>= 8;
	debug("dispReadTemp sensor %u\n", sensor);

	int range;
	if (sensor < 30)
	{
		return ERROR;
	}
	else if (sensor >= 30 && sensor <= 41) range = 0;
	else if (sensor >= 42 && sensor <= 61) range = 1;
	else if (sensor >= 62 && sensor <= 96) range = 2;
	else if (sensor >= 97 && sensor <= 160) range = 3;
	else
	{
		return ERROR;
	}

	*temp = (float)sensor * scale[range] + offset[range] + config.tempoffset + 0.05;
	return OK;
}

void ICACHE_FLASH_ATTR pd74DispOff(void)
{
	GPIO_OUTPUT_SET(EN_GPIO, 1);
}


static void ICACHE_FLASH_ATTR pd74UploadLine(uchar *line)
{
	uint16 rv;
	switch (curLine)	// we can upload up to 4 lines (240 B) in one command
	{
	case 0:
		// initiate UploadImageData command
		pd74SendCmdBegin(CMD_UPLOAD_IMG_INS,
						CMD_UPLOAD_IMG_P1,
						CMD_UPLOAD_IMG_P2,
						GFXMEM_BYTEWIDTH*4);
		spiSendData32((uint*)line, GFXMEM_BYTEWIDTH/4);
		curLine++;
		break;
	case 1:
	case 2:
		spiSendData32((uint*)line, GFXMEM_BYTEWIDTH/4);
		curLine++;
		break;
	case 3:
		spiSendData32((uint*)line, GFXMEM_BYTEWIDTH/4);
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

// Converts one line of pixels to Pervasive Displays Pixel Data Format Type 4 (TC-P74-230)
#define TEST_BIT(var, bit)  ((var >> bit) & 1)
static void ICACHE_FLASH_ATTR pd74ConverLine(uchar *in, uchar *out)
{
    uint temp1pos = GFXMEM_BYTEWIDTH/2-1;
    uint temp2pos = GFXMEM_BYTEWIDTH-1;
    uint i;
    for (i = 0; i < GFXMEM_BYTEWIDTH; i+=2)
    {
        uchar in1 = in[i];
        uchar in2 = in[i+1];
        uchar temp1 = 0;
        uchar temp2 = 0;

        temp1 |=  TEST_BIT(in2, 7);
        temp1 |= (TEST_BIT(in1, 7) << 1);
        temp1 |= (TEST_BIT(in2, 5) << 2);
        temp1 |= (TEST_BIT(in1, 5) << 3);
        temp1 |= (TEST_BIT(in2, 3) << 4);
        temp1 |= (TEST_BIT(in1, 3) << 5);
        temp1 |= (TEST_BIT(in2, 1) << 6);
        temp1 |= (TEST_BIT(in1, 1) << 7);

        temp2 |=  TEST_BIT(in2, 0);
        temp2 |= (TEST_BIT(in1, 0) << 1);
        temp2 |= (TEST_BIT(in2, 2) << 2);
        temp2 |= (TEST_BIT(in1, 2) << 3);
        temp2 |= (TEST_BIT(in2, 4) << 4);
        temp2 |= (TEST_BIT(in1, 4) << 5);
        temp2 |= (TEST_BIT(in2, 6) << 6);
        temp2 |= (TEST_BIT(in1, 6) << 7);

        out[temp1pos--] = temp1;
        out[temp2pos--] = temp2;
    }
}
