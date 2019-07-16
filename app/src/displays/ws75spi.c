#include <ets_sys.h>
#include <osapi.h>
#include <os_type.h>
#include <user_interface.h>
#include <gpio.h>
#include <mem.h>
#include "drivers/spi.h"
#include "displays/ws75spi.h"
#include "display.h"
#include "graphics.h"
#include "config.h"
#include "conv.h"
#include "debug.h"


// IO pins used for display control
#define CS_GPIO			15
#define CS_GPIO_MUX		PERIPHS_IO_MUX_MTDO_U
#define CS_GPIO_FUNC	FUNC_GPIO15

#define DATA_GPIO		13
#define DATA_GPIO_MUX	PERIPHS_IO_MUX_MTCK_U
#define DATA_GPIO_FUNC	FUNC_GPIO13

#define SCLK_GPIO		14
#define SCLK_GPIO_MUX	PERIPHS_IO_MUX_MTMS_U
#define SCLK_GPIO_FUNC	FUNC_GPIO14

#define BUSY_GPIO		12
#define BUSY_GPIO_MUX	PERIPHS_IO_MUX_MTDI_U
#define BUSY_GPIO_FUNC	FUNC_GPIO12

#define RST_GPIO		4
#define RST_GPIO_MUX	PERIPHS_IO_MUX_GPIO4_U
#define RST_GPIO_FUNC	FUNC_GPIO4

#define PWR_GPIO		5
#define PWR_GPIO_MUX	PERIPHS_IO_MUX_GPIO5_U
#define PWR_GPIO_FUNC	FUNC_GPIO5

#define RESET_ASSERT()		GPIO_OUTPUT_SET(RST_GPIO, 0)
#define RESET_DEASSERT()	GPIO_OUTPUT_SET(RST_GPIO, 1)

#define POWER_ON()		GPIO_OUTPUT_SET(PWR_GPIO, 0)
#define POWER_OFF()		GPIO_OUTPUT_SET(PWR_GPIO, 1)


// ---------- External functions ----------
extern void dispUpdateDoneCb(int rv);


// ---------- Internal SPI types and functions ----------
#define BITBANG_DELAY()		//os_delay_us(1)

typedef enum{
	spiHardware,
	spiBitBang
}SpiPinsConf;

typedef enum{
	eCmd = 0,
	eData = 1
}DataType;

LOCAL void spiPinsConfig(SpiPinsConf mode);
LOCAL void spiWrite(uchar low_8bit, uchar high_bit);
LOCAL uint16_t spiReadBitBang(void);


// ---------- Command register offsets ----------
typedef enum{
	panelSetting = 0x00,
	powerSetting = 0x01,
	powerOff = 0x02,
	powerOffSequence = 0x03,
	powerOn = 0x04,
	boosterSoftStart = 0x06,
	deepSleep = 0x07,
	dataStart = 0x10,
	displayRefresh = 0x12,
	imageProcessCmd = 0x13,
	pllControl = 0x30,
	tempSenSelect = 0x41,
	tempSenWrite = 0x42,
	vcomDataIntSetting = 0x50,
	tconSetting = 0x60,
	resolution = 0x61,
	spiFlashControl = 0x65,
	autoMeasureVcom = 0x80,
	vcomDcSetting = 0x82,
	flashModeCmd = 0xe5
}RegOffsetWr;

typedef enum{
	tempSenValue = 0x40,	// name in the datasheet: Temperature Sensor Calibration(TSC)
	tempSenRead = 0x43,
	lowPowerDetection = 0x51,
	revision = 0x70,
	status = 0x71,
	vcomValue = 0x81
}RegOffsetRd;

typedef enum{
	lutVcom = 0x20,
	lutBlack = 0x21,
	lutWhite = 0x22,
	lutGray1 = 0x23,
	lutGray2 = 0x24,
	lutRed0 = 0x25,
	lutRed1 = 0x26,
	lutRed2 = 0x27,
	lutRed3 = 0x28,
	lutXon = 0x29
}LutOffset;


// ---------- Command definitions ----------
typedef union
{
	struct
	{
		// byte 0
		unsigned rst_n:   1;
		unsigned shd_n:   1;
		unsigned shl:     1;
		unsigned ud:      1;
		unsigned unused1: 1;
		unsigned lut_en:  1;
		unsigned res:     2;

		// byte 1
		unsigned unused2: 3;
		unsigned vcm_hz:  1;
		unsigned unused3: 4;
	};
	uchar data[2];
}PanelSetting;

typedef union
{
	struct
	{
		// byte 0
		unsigned vgate_en:      1;
		unsigned vsource_en:    1;
		unsigned vsource_lv_en: 1;
		unsigned unused1:       1;
		unsigned edata_set:     1;
		unsigned edata_sel:     1;
		unsigned unused2:       2;

		// byte 1
		unsigned vghl_lvl:      2;
		unsigned unused3:       6;

		// byte 2
		unsigned vdsp_lv:       6;
		unsigned unused4:       2;

		// byte 3
		unsigned vdns_lv:       6;
		unsigned unused5:       2;
	};
	uchar data[4];
}PowerSetting;

typedef union
{
	struct
	{
		unsigned unused1:   4;
		unsigned t_vds_off: 2;
		unsigned unused2:   2;
	};
	uchar data[1];
}PowerOffSequence;

typedef union
{
	struct
	{
		unsigned min_off_time_a: 3;
		unsigned drv_strength_a: 3;
		unsigned ss_phase_per_a: 2;

		unsigned min_off_time_b: 3;
		unsigned drv_strength_b: 3;
		unsigned ss_phase_per_b: 2;

		unsigned min_off_time_c: 3;
		unsigned drv_strength_c: 3;
		unsigned unused:         2;
	};
	uchar data[3];
}BoosterSoftStart;

typedef union
{
	struct
	{
		unsigned ip_sel:  3;
		unsigned unused1: 1;
		unsigned ip_en:   1;
		unsigned unused2: 3;
	};
	uchar data[1];
}ImageProcessCmd;

typedef union
{
	struct
	{
		unsigned N: 3;
		unsigned M: 3;
		unsigned unused: 2;
	};
	uchar data[1];
}PllControl;

typedef union
{
	struct	// internal sensor
	{
		unsigned unused1: 7;
		signed value:     8;
		unsigned unused2: 1;
	}internal;
	struct	// external sensor
	{
		unsigned unused:  5;
		unsigned value:   11;
	}external;
	uchar data[2];
}TempSenValue;

typedef union
{
	struct
	{
		unsigned unused: 7;
		unsigned useExtSen: 1;
	};
	uchar data[1];
}TempSenSelect;

typedef union
{
	struct
	{
		unsigned cdi: 4;
		unsigned ddx: 1;
		unsigned vbd: 3;
	};
	uchar data[1];
}VcomDataIntSetting;

typedef union
{
	struct
	{
		unsigned g2s: 4;
		unsigned s2g: 4;
	};
	uchar data[1];
}TconSetting;

typedef union
{
	struct
	{
		// bytes 0, 1
		unsigned hres:    10;
		unsigned unused1: 6;

		// bytes 2, 3
		unsigned vres:    9;
		unsigned unused2: 7;
	};
	uchar data[4];
}Resolution;

typedef union
{
	struct
	{
		unsigned dam:    1;
		unsigned unused: 7;
	};
	uchar data[1];
}SpiFlashControl;

typedef union
{
	struct
	{
		unsigned amve:   1;
		unsigned amv:    1;
		unsigned amvs:   1;
		unsigned amvx:   1;
		unsigned amvt:   2;
		unsigned unused: 2;
	};
	uchar data[1];
}AutoMeasureVcom;

typedef union
{
	struct
	{
		unsigned vdcs:   7;
		unsigned unused: 1;
	};
	uchar data[1];
}VcomDcSetting;

typedef union
{
	struct
	{
		// bytes 0, 1
		unsigned lutver: 16;

		// byte 2
		unsigned chrev:  4;
		unsigned unused: 4;
	};
	uchar data[3];
}Revision;


// ---------- Internal functions ----------
LOCAL void ws75writeCmd(RegOffsetWr cmd, const uchar *data, int dataLen);
LOCAL int ws75readCmd(RegOffsetRd cmd, uchar *data, int dataLen);
LOCAL void ws75writeLut(LutOffset lut, const uint8_t *data, int dataLen, int repeat);
LOCAL void checkIfBusy(void);


// ---------- Public functions ----------
void ICACHE_FLASH_ATTR ws75Init(void)
{
	// Configure pins as a GPIO
	PIN_FUNC_SELECT(BUSY_GPIO_MUX, BUSY_GPIO_FUNC);
	PIN_FUNC_SELECT(RST_GPIO_MUX, RST_GPIO_FUNC);
	PIN_FUNC_SELECT(PWR_GPIO_MUX, PWR_GPIO_FUNC);

	RESET_ASSERT();
	POWER_ON();

	spi_hw_cs_disable();	// keep CS high while SPI is not yet configured

	HSpiPins pins = {.miso = FALSE, .mosi = TRUE, .cs = TRUE};
	//spi_init(HSPI, 10, 3, pins);	// 2.66 MHz
	spi_init(HSPI, 4, 2, pins);		// 10 MHz

	// Data is valid on clock leading edge
	// Clock is low when inactive
	spi_mode(HSPI, 1, 0);

	os_delay_us(1000);
	RESET_DEASSERT();
	os_delay_us(1000);
}

void ICACHE_FLASH_ATTR ws75RegSetup(void)
{
	// sequence and parameters adapted from example code by Waveshare
	{
		//debug("PowerSetting\n");
		PowerSetting s = {.data = {0}};
		s.vgate_en = 1;
		s.vsource_en = 1;
		s.vsource_lv_en = 1;
		s.edata_set = 1;
		s.edata_sel = 1;

		s.vghl_lvl = 0;

		s.vdsp_lv = 5;	// 4.0 V

		s.vdns_lv = 5;	// -4.0 V

		ws75writeCmd(powerSetting, s.data, sizeof(s.data));
	}
	{
		//debug("PanelSetting\n");
		PanelSetting s = {.data = {0}};
		s.rst_n = 1;
		s.shd_n = 1;
		s.shl = 1;		// shift right
		s.ud = 1;		// scan up
		s.lut_en = 0;
		s.res = 3;

		s.vcm_hz = 1;

		ws75writeCmd(panelSetting, s.data, sizeof(s.data));
	}
	{
		//debug("BoosterSoftStart\n");
		BoosterSoftStart s = {.data = {0}};
		s.min_off_time_a = 7;	// 6.77 us
		s.drv_strength_a = 0;
		s.ss_phase_per_a = 3;	// 40 ms

		s.min_off_time_b = 4;	// 0.77 us
		s.drv_strength_b = 1;
		s.ss_phase_per_b = 3;	// 40 ms

		s.min_off_time_c = 0;	// 0.26 us
		s.drv_strength_c = 5;

		ws75writeCmd(boosterSoftStart, s.data, sizeof(s.data));
	}
	{
		//debug("powerOn\n");
		ws75writeCmd(powerOn, 0,0);
	}
	{
		//debug("PllControl\n");
		PllControl s = {.data = {0}};
		s.N = 4;	// 50 Hz
		s.M = 7;

		ws75writeCmd(pllControl, s.data, sizeof(s.data));
	}
	{
		//debug("TempSenSelect\n");
		TempSenSelect s = {.data = {0}};
		s.useExtSen = 0;

		ws75writeCmd(tempSenSelect, s.data, sizeof(s.data));
	}
	{
		//debug("VcomDataIntSetting\n");
		VcomDataIntSetting s = {.data = {0}};
		s.cdi = 7;	// 10 hsync
		s.ddx = 0;
		s.vbd = 0;	// white border

		ws75writeCmd(vcomDataIntSetting, s.data, sizeof(s.data));
	}
	{
		//debug("TconSetting\n");
		TconSetting s = {.data = {0}};
		s.g2s = 2;
		s.s2g = 2;	// 7920 ns

		ws75writeCmd(tconSetting, s.data, sizeof(s.data));
	}
	{
		//debug("Resolution\n");
		Resolution s = {.data = {0}};
		s.hres = WS75_DISP_HEIGHT;
		s.vres = WS75_DISP_WIDTH;

		swap(&s.data[0], &s.data[1]);
		swap(&s.data[2], &s.data[3]);
		ws75writeCmd(resolution, s.data, sizeof(s.data));
	}
	{
		//debug("VcomDcSetting\n");
		VcomDcSetting s = {.data = {0}};
		s.vdcs = 0x1E;	// from LUT file

		ws75writeCmd(vcomDcSetting, s.data, sizeof(s.data));
	}
	{
		//debug("flashModeCmd\n");
		const uchar FlashModeCmdVal = 3;
		ws75writeCmd(flashModeCmd, &FlashModeCmdVal, sizeof(FlashModeCmdVal));
	}

	debug("ws75RegSetup done\n");
}

void ICACHE_FLASH_ATTR ws75UploadPerform(void)
{
	debug("ws75UploadPerform\n");

	int x, y, i;
	uchar *buf = os_malloc(GFXMEM_HEIGHT);
	if (!buf)
	{
		debug("!buf\n");
		return;
	}

	while (!GPIO_INPUT_GET(BUSY_GPIO));
	spiWrite(dataStart, eCmd);

	// display assumes data in landscape mode
	// however, the frame buffer is in portrait mode
	// so we need to transform it on fly
	for (x = (GFXMEM_BYTEWIDTH-1); x >= 0; x--)	// starting from the right side, moving left
	{
		// buffer 8 vertical lines of pixels
		for (y = 0, i = x; y < GFXMEM_HEIGHT; y++, i += GFXMEM_BYTEWIDTH)
		{
			buf[y] = gfxMem[i];
		}

		uint mask;
		for (mask = 1; mask <= 0x80; mask <<= 1)	// iterate through 8 vertical lines
		{
			for (y = 0; y < GFXMEM_HEIGHT; )			// iterate through pixels in the line
			{
				uchar p1 = (buf[y++] & mask) != 0;
				uchar p2 = (buf[y++] & mask) != 0;
				uchar data = ((p1 * 3) << 4) | (p2 * 3);	// pack two pixels in one byte
				spiWrite(data, eData);
			}
		}
		system_soft_wdt_feed();
	}

	os_free(buf);

	ws75writeCmd(displayRefresh, 0,0);

	os_timer_setfn(&busyCheckTmr, (os_timer_func_t *)checkIfBusy, NULL);
	os_timer_arm(&busyCheckTmr, 1000, 0);
}

int ICACHE_FLASH_ATTR ws75ReadTemp(float *temp)
{
	TempSenValue senVal = {.data = {0}};
	int readBytes = ws75readCmd(tempSenValue, senVal.data, sizeof(senVal.data));
	if (readBytes != sizeof(senVal.data))
	{
		return ERROR;
	}
	swap(&senVal.data[0], &senVal.data[1]);
	*temp = (float)senVal.internal.value / 2.0;
	return OK;
}

void ICACHE_FLASH_ATTR ws75PrintDeviceInfo(void)
{
	Revision rev = {.data = {0}};
	int readBytes = ws75readCmd(revision, rev.data, sizeof(rev.data));
	if (readBytes != sizeof(rev.data))
	{
		return;
	}
	swap(&rev.data[0], &rev.data[1]);
	debug("chrev 0x%X\n", rev.chrev);
	debug("lutver 0x%X\n", rev.lutver);
}

void ICACHE_FLASH_ATTR ws75DispOff(void)
{
	const uchar DeepSleepCode = 0xA5;
	ws75writeCmd(powerOff, 0,0);
	ws75writeCmd(deepSleep, (uchar*)&DeepSleepCode, sizeof(DeepSleepCode));
	while (!GPIO_INPUT_GET(BUSY_GPIO));
	POWER_OFF();
}


// ---------- Internal functions ----------
LOCAL void ws75writeCmd(RegOffsetWr cmd, const uchar *data, int dataLen)
{
	while (!GPIO_INPUT_GET(BUSY_GPIO));

	spiWrite(cmd, eCmd);
	while (dataLen > 0)
	{
		spiWrite(*data, eData);
		data++;
		dataLen--;
	}
}

LOCAL int ws75readCmd(RegOffsetRd cmd, uchar *data, int dataLen)
{
	while (!GPIO_INPUT_GET(BUSY_GPIO));
	spiWrite(cmd, eCmd);

	spiPinsConfig(spiBitBang);

	while (!GPIO_INPUT_GET(BUSY_GPIO));

	uint16_t temp;
	int bytesRead = 0;
	for (bytesRead = 0; bytesRead < dataLen; )
	{
		temp = spiReadBitBang();
		if (temp & 0x100)
		{
			*data = (uchar)temp;
			data++;
			bytesRead++;
		}
		else
		{
			debug("temp 0x%02X\n", temp);
			break;
		}
	}

	spiPinsConfig(spiHardware);

	return bytesRead;
}

LOCAL void ws75writeLut(LutOffset lut, const uint8_t *data, int dataLen, int repeat)
{
	int i, j;
	const uint8_t *pData;

	while (!GPIO_INPUT_GET(BUSY_GPIO));

	spiWrite(lut, eCmd);
	for (i = 0; i < repeat; i++)
	{
		pData = data;
		for (j = 0; j < dataLen; j++)
		{
			spiWrite(*pData, eData);
			pData++;
		}
	}
}

LOCAL void ICACHE_FLASH_ATTR checkIfBusy(void)
{
	if (!GPIO_INPUT_GET(BUSY_GPIO))
	{
		//debug("busy\n");
		os_timer_arm(&busyCheckTmr, 100, 0);
		return;
	}
	//debug("done\n");
	dispUpdateDoneCb(OK);
}


// ---------- Internal SPI functions ----------
LOCAL void ICACHE_FLASH_ATTR spiPinsConfig(SpiPinsConf conf)
{
	if (conf == spiHardware)
	{
		PIN_FUNC_SELECT(DATA_GPIO_MUX, 2);
		PIN_FUNC_SELECT(SCLK_GPIO_MUX, 2);
		spi_hw_cs_enable();
	}
	else
	{
		spi_hw_cs_disable();			// cs as gpio and set high

		PIN_FUNC_SELECT(DATA_GPIO_MUX, DATA_GPIO_FUNC);
		PIN_FUNC_SELECT(SCLK_GPIO_MUX, SCLK_GPIO_FUNC);

		GPIO_DIS_OUTPUT(DATA_GPIO);		// data as input
		GPIO_OUTPUT_SET(SCLK_GPIO, 0);	// clk idle low
	}
}

// Adapted from Espressif example:
// http://bbs.espressif.com/viewtopic.php?f=31&t=1346
LOCAL void spiWrite(uchar low_8bit, uchar high_bit)
{
	uint regvalue;
	uchar bytetemp;

	if (high_bit)
	{
		bytetemp = (low_8bit>>1) | 0x80;
	}
	else
	{
		bytetemp = (low_8bit>>1) & 0x7f;
	}

    // configure transmission variable, 9bit transmission length and first 8 command bit
	regvalue = ((8&SPI_USR_COMMAND_BITLEN)<<SPI_USR_COMMAND_BITLEN_S)|((uint)bytetemp);

	if(low_8bit&0x01)
	{
		regvalue |= BIT15;        //write the 9th bit
	}

	while (READ_PERI_REG(SPI_CMD(HSPI)) & SPI_USR);
	WRITE_PERI_REG(SPI_USER2(HSPI), regvalue);	// write command and command length into spi reg
	SET_PERI_REG_MASK(SPI_CMD(HSPI), SPI_USR);	// transmission start
	while (READ_PERI_REG(SPI_CMD(HSPI)) & SPI_USR);
}

LOCAL uint16_t spiReadBitBang(void)
{
	GPIO_OUTPUT_SET(CS_GPIO, 0);
	BITBANG_DELAY();

		uint16_t data = 0;
		int bit;
		for (bit = 0; bit < 9; bit++)
		{
			BITBANG_DELAY();
			data <<= 1;
			data |= GPIO_INPUT_GET(DATA_GPIO);

			GPIO_OUTPUT_SET(SCLK_GPIO, 1);
			BITBANG_DELAY();

			GPIO_OUTPUT_SET(SCLK_GPIO, 0);
		}

	GPIO_OUTPUT_SET(CS_GPIO, 1);
	BITBANG_DELAY();

	return data;
}
