#ifndef WS75SPI_H
#define WS75SPI_H

#include "typedefs.h"

#define WS75_DISP_HEIGHT	640
#define WS75_DISP_WIDTH		384


void ws75Init(void);
void ws75RegSetup(void);
void ws75UploadPerform(void);
void ws75PrintDeviceInfo(void);
int ws75ReadTemp(float *temp);
void ws75DispOff(void);


#endif /* WS75SPI_H */
