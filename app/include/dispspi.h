#ifndef USER_DISPSPI_H_
#define USER_DISPSPI_H_

#include "typedefs.h"

#define CS_GPIO			15
#define CS_GPIO_MUX		PERIPHS_IO_MUX_MTDO_U
#define CS_GPIO_FUNC	FUNC_GPIO15

#define BUSY_GPIO		4
#define BUSY_GPIO_MUX	PERIPHS_IO_MUX_GPIO4_U
#define BUSY_GPIO_FUNC	FUNC_GPIO4

#define EN_GPIO			5
#define EN_GPIO_MUX		PERIPHS_IO_MUX_GPIO4_U
#define EN_GPIO_FUNC	FUNC_GPIO5

#define DISP_REFRESH_TIME_MS	2500

void dispBeginUpload(void);
void dispFinalizeUpload(void);
void dispUploadLine(uchar *line);
void dispPrintDeviceInfo(void);
void dispReadTemp(char *tempstr);
void dispSpiInit(void);
void dispOff(void);



#endif /* USER_DISPSPI_H_ */
