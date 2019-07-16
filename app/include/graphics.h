#ifndef INCLUDE_GRAPHICS_H_
#define INCLUDE_GRAPHICS_H_

#include "typedefs.h"
#include "display.h"

#define GFXMEM_WIDTH        DISP_WIDTH
#define GFXMEM_BYTEWIDTH    (GFXMEM_WIDTH/8)

#if (GFXMEM_BYTEWIDTH*DISP_HEIGHT) > (32*1024)  // if resolution too big
#define GFXMEM_HEIGHT       (DISP_HEIGHT/2)     // buffer only half of display at a time
#else
#define GFXMEM_HEIGHT       DISP_HEIGHT
#endif

extern uchar *gfxMem;
extern const uint gfxMemSize;

int gfxMemAlloc(void);
void gfxMemFill(uchar data);

void gfxDrawImage(int x, int y, const uint *image);

int gfxDrawCharBig(const uint **font, int x, int y, uchar ch);
int gfxDrawStrBig(const uint **font, int x, int y, const char *str);
int gfxStrWidthBig(const uint **font, const char *str);

int gfxDrawChar(const uint *font, int x, int y, uchar ch);
int gfxDrawStr(const uint *font, int x, int y, const char *str);
int gfxStrWidth(const uint *font, const char *str);

int gfxDrawStrCentredBig(const uint **font, int centre, int y, const char *str);
int gfxDrawStrAlignRightBig(const uint **font, int right, int y, const char *str);

int gfxDrawStrCentred(const uint *font, int centre, int y, const char *str);
int gfxDrawStrAlignRight(const uint *font, int right, int y, const char *str);

void gfxDrawLine(int x0, int y0, int x1, int y1, char color);
void gfxDrawcircle(int x0, int y0, int radius, char color, char fill);
void gfxDrawLineBold(int x0, int y0, int x1, int y1, char color, char boldX, char boldY);
void gfxDrawLineDotted(int x0, int y0, int x1, int y1, int space, char color);
void gfxDrawRectDotted(int x0, int y0, int x1, int y1, int space, char color);
void gfxDrawRectFill(int x0, int y0, int x1, int y1, char color);


#endif /* INCLUDE_GRAPHICS_H_ */
