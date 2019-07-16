#ifndef PD74SPI_H
#define PD74SPI_H

#include "typedefs.h"

#define PD74_DISP_HEIGHT	800
#define PD74_DISP_WIDTH		480


void pd74UploadBegin(void);
void pd74UploadPerform(void);
void pd74UploadFinalize(void);

void pd74PrintDeviceInfo(void);
int pd74ReadTemp(float *temp);
void pd74SpiInit(void);
void pd74DispOff(void);




#endif /* PD74SPI_H */
