#include <ets_sys.h>
#include <osapi.h>
#include <os_type.h>
#include <user_interface.h>
#include <gpio.h>
#include <mem.h>
#include "drivers/spi.h"
#include "displays/pd74spi.h"
#include "displays/ws75spi.h"
#include "display.h"
#include "graphics.h"
#include "config.h"
#include "debug.h"


extern int guiYoffset;

os_timer_t busyCheckTmr;

//#define PRINT_GFXMEM
#ifdef PRINT_GFXMEM
LOCAL void ICACHE_FLASH_ATTR sendInt(int data)
{
	char *pData = (char*)&data;
	uart_tx_one_char(*pData);
	uart_tx_one_char(*(pData+1));
	uart_tx_one_char(*(pData+2));
	uart_tx_one_char(*(pData+3));
}

LOCAL void ICACHE_FLASH_ATTR sendPreamble(void)
{
	const unsigned char data[] = {
			0x55, 0x55, 0x55, 0x55, 0x55,
			0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
			0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
			0x01, 0x23, 0x45, 0x67, 0x89};

	uart_tx_one_char('\n');
	int i;
	for (i = 0; i < sizeof(data); i++)
	{
		uart_tx_one_char(data[i]);
	}
}

LOCAL void ICACHE_FLASH_ATTR printGfxMem(void)
{
	uint i;

	sendPreamble();

	// complite display size
	sendInt(DISP_HEIGHT);
	sendInt(GFXMEM_BYTEWIDTH);

	// size of this part
	sendInt(GFXMEM_HEIGHT);
	sendInt(GFXMEM_BYTEWIDTH);

	// start coordinates of this part
	sendInt(0);	// x
	sendInt(0);	// y

    for (i = 0; i < gfxMemSize; i++)
    {
    	uart_tx_one_char(gfxMem[i]);
    }
}

LOCAL void ICACHE_FLASH_ATTR printGfxMemPart(DispPart part)
{
    uint i;

    sendPreamble();

    // complite display size
    sendInt(DISP_HEIGHT);
    sendInt(GFXMEM_BYTEWIDTH);

    // size of this part
    sendInt(GFXMEM_HEIGHT);
    sendInt(GFXMEM_BYTEWIDTH);

    // start coordinates of this part
    switch (part)
    {
    case eDispTopPart:
        sendInt(0); // x
        sendInt(0); // y
        break;
    case eDispBottomPart:
        sendInt(0); // x
        sendInt(GFXMEM_HEIGHT); // y
        break;
    }

    for (i = 0; i < gfxMemSize; i++)
    {
        uart_tx_one_char(gfxMem[i]);
    }
}
#endif

void ICACHE_FLASH_ATTR dispUpdate(DispPart part)
{
#if DISP_MODEL == WAVESHARE
    switch (part)
    {
    case eDispTopPart:
        guiYoffset = GFXMEM_HEIGHT/2;
        break;
    case eDispBottomPart:
        ws75UploadPerform();
        break;
    }
#ifdef PRINT_GFXMEM
    printGfxMem();
#endif
#elif DISP_MODEL == PERVASIVE
#ifdef PRINT_GFXMEM
	printGfxMemPart(part);
#endif
	switch (part)
	{
	case eDispTopPart:
		pd74UploadBegin();
		pd74UploadPerform();
		gfxMemFill(0x00);
		break;
	case eDispBottomPart:
		pd74UploadPerform();
		pd74UploadFinalize();
		break;
	}
#endif


}
