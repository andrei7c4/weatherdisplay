#include <ets_sys.h>
#include <string.h>
#include <osapi.h>
#include <mem.h>
#include "typedefs.h"
#include "display.h"
#include "conv.h"


static uchar mem[DISP_HEIGHT][DISP_MEMWIDTH];


void ICACHE_FLASH_ATTR dispFillMem(uchar data)
{
	uint i;
    for (i = 0; i < DISP_HEIGHT; i++)
    {
        memset(mem[i], data, DISP_MEMWIDTH);
    }
}

LOCAL void ICACHE_FLASH_ATTR dispDrawBitmap(int x, int y, int bmWidth, int bmHeight, const uint *bitmap, int bitmapSize)
{
    int maxBmHeight = DISP_HEIGHT-y;
    if (bmHeight > maxBmHeight)
    {
        bmHeight = maxBmHeight;
    }
    if (bmHeight <= 0)
        return;

    bmWidth /= 8;
    int memX = x/8;
    int bmWidthCpy = bmWidth;
    int maxBmWidth = (DISP_MEMWIDTH-memX);
    if (bmWidthCpy > maxBmWidth)
    {
        bmWidthCpy = maxBmWidth;
    }
    if (bmWidthCpy <= 0)
        return;

    int i;
    if ((memX%4) == 0 && (bmWidthCpy%4) == 0)	// x and width dividable by 4 -> can access flash directly
    {
    	const uchar *pBitmap = (uchar*)bitmap;
		for (i = 0; i < bmHeight; i++, y++)
		{
			memcpy(mem[y]+memX, pBitmap, bmWidthCpy);
			pBitmap += bmWidth;
		}
    }
    else	// x or width not dividable by 4 -> need a temp buffer to avoid alignment issues
    {
        uchar *temp = (uchar*)os_malloc(bitmapSize);
        uchar *pTemp = temp;
        if (!temp)
        	return;
        memcpy(pTemp, bitmap, bitmapSize);
    	for (i = 0; i < bmHeight; i++, y++)
    	{
    		memcpy(mem[y]+memX, pTemp, bmWidthCpy);
    		pTemp += bmWidth;
    	}
        os_free(temp);
    }
}

void ICACHE_FLASH_ATTR dispDrawImage(int x, int y, const uint *image)
{
    int imgWidth = image[0];
    int imgHeight = image[1];
    int bitmapSize = image[2];
    dispDrawBitmap(x, y, imgWidth, imgHeight, image+4, bitmapSize);
}

int ICACHE_FLASH_ATTR dispDrawChar(const uint **font, int x, int y, uchar ch)
{
    int first = (int)font[0];
    int last = (int)font[1];
    if (ch < first || ch > last)
    {
        return 0;
    }
    ch -= (first-2);
    const uint *chHeader = font[ch];
    if (!chHeader)	// char not found
    {
        ch = 2;		// print space
        chHeader = font[ch];
        if (!chHeader)
        {
            return 0;
        }
        return chHeader[0];
    }

    int chWidth = chHeader[0];
    int chHeight = chHeader[1];
    int bitmapSize = chHeader[2];
    int yoffset = chHeader[3];
    dispDrawBitmap(x, y+yoffset, chWidth, chHeight, font[ch]+4, bitmapSize);
    return chWidth;
}

int ICACHE_FLASH_ATTR dispDrawStr(const uint **font, int x, int y, const char *str)
{
	int strWidth = 0;
    while (*str)
    {
        char ch = *str;
        int chWidth = dispDrawChar(font, x, y, ch);
        x += chWidth;
        strWidth += chWidth;
        str++;
    }
    return strWidth;
}

LOCAL int ICACHE_FLASH_ATTR dispCharWidth(const uint **font, uchar ch)
{
    int first = (int)font[0];
    int last = (int)font[1];
    if (ch < first || ch > last)
    {
        return 0;
    }
    ch -= (first-2);
    const uint *chHeader = font[ch];
    if (!chHeader)
    {
        ch = 2;
        chHeader = font[ch];
        if (!chHeader)
        {
            return 0;
        }
    }
    return chHeader[0];
}

int ICACHE_FLASH_ATTR dispStrWidth(const uint **font, const char *str)
{
	int strWidth = 0;
    while (*str)
    {
        char ch = *str;
        int chWidth = dispCharWidth(font, ch);
        strWidth += chWidth;
        str++;
    }
    return strWidth;
}

int ICACHE_FLASH_ATTR dispDrawStrCentred(const uint **font, int centre, int y, const char *str)
{
	int strWidth = dispStrWidth(font, str);
	int x = centre-(strWidth/2);
	x = alignTo8(x);	// align to nearest 8 pixel boundary
	return dispDrawStr(font, x, y, str);
}

int ICACHE_FLASH_ATTR dispDrawStrAlignRight(const uint **font, int right, int y, const char *str)
{
	int strWidth = dispStrWidth(font, str);
	int x = right-strWidth;
	return dispDrawStr(font, x, y, str);
}


void ICACHE_FLASH_ATTR dispDrawPixel(int x, int y, char color)
{
    if (x >= DISP_WIDTH || y >= DISP_HEIGHT)
    {
        return;
    }

    int xByte = x/8;
    int bitMask = 1<<(7-(x%8));
    if (color)
    {
        mem[y][xByte] |= bitMask;
    }
    else
    {
        mem[y][xByte] &= ~bitMask;
    }
}

void ICACHE_FLASH_ATTR dispDrawLine(int x0, int y0, int x1, int y1, char color)
{
    int dx = abs(x1-x0), sx = x0<x1 ? 1 : -1;
    int dy = abs(y1-y0), sy = y0<y1 ? 1 : -1;
    int err = (dx>dy ? dx : -dy)/2, e2;

    for(;;)
    {
        dispDrawPixel(x0,y0,color);
        if (x0==x1 && y0==y1) break;
        e2 = err;
        if (e2 >-dx) { err -= dy; x0 += sx; }
        if (e2 < dy) { err += dx; y0 += sy; }
    }
}

void ICACHE_FLASH_ATTR dispDrawcircle(int x0, int y0, int radius, char color, char fill)
{
    int x = radius;
    int y = 0;
    int err = 0;

    while (x >= y)
    {
    	if (fill)
    	{
    		dispDrawLine(x0 - x, y0 + y, x0 + x, y0 + y, color);
    		dispDrawLine(x0 - y, y0 + x, x0 + y, y0 + x, color);
    		dispDrawLine(x0 - x, y0 - y, x0 + x, y0 - y, color);
    		dispDrawLine(x0 - y, y0 - x, x0 + y, y0 - x, color);
    	}
    	else
    	{
            dispDrawPixel(x0 + x, y0 + y, color);
            dispDrawPixel(x0 + y, y0 + x, color);
            dispDrawPixel(x0 - y, y0 + x, color);
            dispDrawPixel(x0 - x, y0 + y, color);
            dispDrawPixel(x0 - x, y0 - y, color);
            dispDrawPixel(x0 - y, y0 - x, color);
            dispDrawPixel(x0 + y, y0 - x, color);
            dispDrawPixel(x0 + x, y0 - y, color);
    	}

        if (err <= 0)
        {
            y += 1;
            err += 2*y + 1;
        }
        if (err > 0)
        {
            x -= 1;
            err -= 2*x + 1;
        }
    }
}

void ICACHE_FLASH_ATTR dispDrawLineBold(int x0, int y0, int x1, int y1, char color, char boldX, char boldY)
{
	dispDrawLine(x0, y0, x1, y1, color);
	if (boldX)
	{
		dispDrawLine(x0-1, y0, x1-1, y1, color);
		dispDrawLine(x0+1, y0, x1+1, y1, color);
	}
	if (boldY)
	{
		dispDrawLine(x0, y0-1, x1, y1-1, color);
		dispDrawLine(x0, y0+1, x1, y1+1, color);
	}
}

void ICACHE_FLASH_ATTR dispDrawLineDotted(int x0, int y0, int x1, int y1, int space, char color)
{
    int dx = abs(x1-x0), sx = x0<x1 ? 1 : -1;
    int dy = abs(y1-y0), sy = y0<y1 ? 1 : -1;
    int err = (dx>dy ? dx : -dy)/2, e2;
    int i = 0;
    if (space < 1)
    {
    	space = 1;
    }

    for(;;)
    {
    	if (i == space)
    	{
    		i = 0;
    	}
    	else
    	{
        	if (i == 0)
        	{
        		dispDrawPixel(x0,y0,color);
        	}
    		i++;
    	}

        if (x0==x1 && y0==y1) break;
        e2 = err;
        if (e2 >-dx) { err -= dy; x0 += sx; }
        if (e2 < dy) { err += dx; y0 += sy; }
    }
}

void ICACHE_FLASH_ATTR dispDrawRectDotted(int x0, int y0, int x1, int y1, int space, char color)
{
	int x;
	for (x = x0; x <= x1; x+=(space+1))
	{
		dispDrawLineDotted(x, y0, x, y1, space, color);
	}
}

//-------------------------------------------------------------------------------------------
// Converts one line of pixels to Pervasive Displays Pixel Data Format Type 4 (TC-P74-230)
#define TEST_BIT(var, bit)  ((var >> bit) & 1)
static void ICACHE_FLASH_ATTR dispConverLine(uchar *in, uchar *out)
{
    uint temp1pos = DISP_MEMWIDTH/2-1;
    uint temp2pos = DISP_MEMWIDTH-1;
    uint i;
    for (i = 0; i < DISP_MEMWIDTH; i+=2)
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

void ICACHE_FLASH_ATTR dispUpdate(DispPart part)
{
	uint i;
	uchar line[DISP_MEMWIDTH];

	if (part == eDispTopPart)
	{
		//dispSpiInit();
		dispBeginUpload();
	}
    for (i = 0; i < DISP_HEIGHT; i++)
    {
    	dispConverLine(mem[i], line);
    	dispUploadLine(line);
    }
    if (part == eDispBottomPart)
    {
    	dispFinalizeUpload();
    }
}
