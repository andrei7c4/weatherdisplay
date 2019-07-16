#ifndef DISPLAY_H_
#define DISPLAY_H_

#include "typedefs.h"
#include "displays/pd74spi.h"
#include "displays/ws75spi.h"

#define WAVESHARE   1
#define PERVASIVE	2
#define DISP_MODEL	WAVESHARE

#if DISP_MODEL == WAVESHARE
#define DISP_HEIGHT         WS75_DISP_HEIGHT
#define DISP_WIDTH          WS75_DISP_WIDTH
#elif DISP_MODEL == PERVASIVE
#define DISP_HEIGHT         PD74_DISP_HEIGHT
#define DISP_WIDTH          PD74_DISP_WIDTH
#else
#error "Display model not defined"
#endif


typedef enum{
	eDispTopPart,
	eDispBottomPart
}DispPart;

extern os_timer_t busyCheckTmr;

void dispUpdate(DispPart part);

#if DISP_MODEL == WAVESHARE
#define dispIfaceInit(x)        ws75Init(x)
#define dispRegSetup(x)         ws75RegSetup(x)
#define dispPrintDeviceInfo(x)  ws75PrintDeviceInfo(x)
#define dispReadTemp(x)         ws75ReadTemp(x)
#define dispTurnOff(x)          ws75DispOff(x)
#elif DISP_MODEL == PERVASIVE
#define dispIfaceInit(x)		pd74SpiInit(x)
#define dispRegSetup(x)
#define dispPrintDeviceInfo(x)	pd74PrintDeviceInfo(x)
#define dispReadTemp(x)			pd74ReadTemp(x)
#define dispTurnOff(x)			pd74DispOff(x)
#endif


#endif /* DISPLAY_H_ */
