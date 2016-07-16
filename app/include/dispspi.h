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
#define EN_GPIO_MUX		PERIPHS_IO_MUX_GPIO5_U
#define EN_GPIO_FUNC	FUNC_GPIO5


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


void dispBeginUpload(void);
void dispFinalizeUpload(void);
void dispUploadLine(uchar *line);
void dispPrintDeviceInfo(void);
void dispReadTemp(char *tempstr);
void dispSpiInit(void);
void dispOff(void);



#endif /* USER_DISPSPI_H_ */
