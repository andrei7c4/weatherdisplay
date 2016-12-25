#ifndef INCLUDE_DISPLAY_H_
#define INCLUDE_DISPLAY_H_

#include "typedefs.h"

#define DISP_HEIGHT		400		// total display height is 800, but only half of it is buffered
#define DISP_WIDTH		480
#define DISP_MEMWIDTH	(DISP_WIDTH/8)

typedef enum{
	eDispTopPart,
	eDispBottomPart
}DispPart;

void dispFillMem(uchar data);
void dispDrawImage(int x, int y, const uint *image);
int dispDrawChar(const uint **font, int x, int y, uchar ch);
int dispDrawStr(const uint **font, int x, int y, const char *str);
int dispStrWidth(const uint **font, const char *str);
int dispDrawStrCentred(const uint **font, int centre, int y, const char *str);
int dispDrawStrAlignRight(const uint **font, int right, int y, const char *str);
void dispDrawPixel(int x, int y, char color);
void dispDrawLine(int x0, int y0, int x1, int y1, char color);
void dispDrawcircle(int x0, int y0, int radius, char color, char fill);
void dispDrawLineBold(int x0, int y0, int x1, int y1, char color, char boldX, char boldY);
void dispDrawLineDotted(int x0, int y0, int x1, int y1, int space, char color);
void dispDrawRectDotted(int x0, int y0, int x1, int y1, int space, char color);
void dispUpdate(DispPart part);



#endif /* INCLUDE_DISPLAY_H_ */
