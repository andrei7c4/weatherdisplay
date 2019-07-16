#include <ets_sys.h>
#include <string.h>
#include <osapi.h>
#include <mem.h>
#include "graphics.h"
#include "typedefs.h"
#include "conv.h"
#include "debug.h"

uchar *gfxMem = NULL;
const uint gfxMemSize = (GFXMEM_HEIGHT*GFXMEM_BYTEWIDTH);


int ICACHE_FLASH_ATTR gfxMemAlloc(void)
{
	gfxMem = os_zalloc(gfxMemSize);
	if (!gfxMem)
	{
		debug("gfxmem malloc failed\n");
		return ERROR;
	}
	return OK;
}

void ICACHE_FLASH_ATTR gfxMemFill(uchar data)
{
	memset(gfxMem, data, gfxMemSize);
}


LOCAL void ICACHE_FLASH_ATTR gfxDrawPixel(int x, int y, int color)
{
    if (x >= GFXMEM_WIDTH || y >= GFXMEM_HEIGHT)
    {
        return;
    }
    uchar *pBuf = &gfxMem[y*GFXMEM_BYTEWIDTH + x/8];
    uchar mask = (0x80 >> (x & 7));
    *pBuf = color ? *pBuf | mask : *pBuf & ~mask;
}

LOCAL int ICACHE_FLASH_ATTR gfxGetPixel(int x, int y)
{
	uchar *pBuf = &gfxMem[y*GFXMEM_BYTEWIDTH + x/8];
	uchar mask = (0x80 >> (x & 7));
	return (*pBuf & mask) != 0;
}

// for big bitmaps only -> 'x' is aligned to left 8 pixel boundary
LOCAL void ICACHE_FLASH_ATTR drawBitmapAlign8(int x, int y, int bmWidth, int bmHeight, const uint *bitmap, int bitmapSize)
{
    int maxBmHeight = GFXMEM_HEIGHT-y;
    if (bmHeight > maxBmHeight)
    {
        bmHeight = maxBmHeight;
    }
    if (bmHeight <= 0)
        return;

    bmWidth /= 8;
    int memX = x/8;
    int bmWidthCpy = bmWidth;
    int maxBmWidth = (GFXMEM_BYTEWIDTH-memX);
    if (bmWidthCpy > maxBmWidth)
    {
        bmWidthCpy = maxBmWidth;
    }
    if (bmWidthCpy <= 0)
        return;

    int i;
    if ((memX % 4) == 0 && (bmWidthCpy % 4) == 0)	// x and width are multiples of 4 -> can access flash directly
    {
    	const uchar *pBitmap = (uchar*)bitmap;
		for (i = 0; i < bmHeight; i++, y++)
		{
			os_memcpy(gfxMem + y*GFXMEM_BYTEWIDTH+memX, pBitmap, bmWidthCpy);
			pBitmap += bmWidth;
		}
    }
    else	// x or width are not multiples of 4 -> need a temp buffer to avoid alignment issues
    {
        uchar *temp = (uchar*)os_malloc(bitmapSize);
        uchar *pTemp = temp;
        if (!temp)
        	return;
        os_memcpy(pTemp, bitmap, bitmapSize);
    	for (i = 0; i < bmHeight; i++, y++)
    	{
    		os_memcpy(gfxMem + y*GFXMEM_BYTEWIDTH+memX, pTemp, bmWidthCpy);
    		pTemp += bmWidth;
    	}
        os_free(temp);
    }
}

// multiple must be power of 2
LOCAL int ICACHE_FLASH_ATTR roundUp(int numToRound, int multiple)
{
	return (numToRound + multiple - 1) & -multiple;
}

LOCAL void ICACHE_FLASH_ATTR drawBitmap(int x0, int y0, int bmWidth, int bmHeight, const uint *bitmap, int bitmapSize)
{
    if (x0 < 0) x0 = 0;
    if (y0 < 0) y0 = 0;

	int maxBmHeight = GFXMEM_HEIGHT - y0;
    if (bmHeight > maxBmHeight)
    {
        bmHeight = maxBmHeight;
    }
    if (bmHeight <= 0)
        return;

    int bmByteWidth = roundUp(bmWidth,8)/8;

    int maxBmWidth = GFXMEM_WIDTH - x0;
    if (bmWidth > maxBmWidth)
    {
        bmWidth = maxBmWidth;
    }
    if (bmWidth <= 0)
        return;

    int x1 = x0 + bmWidth - 1;
    const int y1 = y0 + bmHeight - 1;
    const int idxFirst = x0 / 8;
    const int idxLast = x1 / 8;
    x0 &= 7;
    x1 &= 7;

    int y, i, j;
    uchar *pBuf;
    uchar *pLine;

	uchar *pBitmap = (uchar*)os_malloc(bitmapSize*4);
	if (!pBitmap)
		return;
	os_memcpy(pBitmap, bitmap, bitmapSize*4);

    if (idxFirst == idxLast)    // single column -> special case
    {
        uchar mask = 0;
        for (i = x0; i <= x1; i++)
        {
            mask |= (0x80 >> i);
        }

        for (y = y0, pBuf = &gfxMem[y0 * GFXMEM_BYTEWIDTH + idxFirst], pLine = pBitmap;
             y <= y1;
             y++, pBuf += GFXMEM_BYTEWIDTH, pLine += bmByteWidth)
        {
            // set/clear bits of single column
            *pBuf = (*pBuf & ~mask) | ((*pLine >> x0) & mask);
        }
    }
    else
    {
        const uchar maskFirst = (0xFF >> x0);
        const uchar maskLast = (0xFF << (7 - x1));
        const int cntFirst = 8 - x0;    // nr. of pixels from first (leftmost) column
        const int cntLast = x1 + 1;     // nr. of pixels from last (rightmost) column

        for (y = y0, pLine = pBitmap; y <= y1; y++, pLine += bmByteWidth)
        {
            // set/clear LSBits of fist (leftmost) column
            pBuf = &gfxMem[y * GFXMEM_BYTEWIDTH + idxFirst];
            *pBuf = (*pBuf & ~maskFirst) | (pLine[0] >> x0);
            pBuf++;

            // set/clear all bits of middle columns
            for (i = idxFirst + 1, j = 0; i < idxLast; i++, pBuf++)
            {
                uchar temp = pLine[j] << cntFirst;
                *pBuf = temp | (pLine[++j] >> x0);
            }

            // set/clear MSBits of last (rightmost) column
            uchar temp = pLine[j] << cntFirst;
            if(cntLast > x0)
            {
                temp |= (pLine[++j] >> x0);
            }
            *pBuf = (*pBuf & ~maskLast) | temp;
        }
    }

	os_free(pBitmap);
}


void ICACHE_FLASH_ATTR gfxDrawImage(int x, int y, const uint *image)
{
    int imgWidth = image[0];
    int imgHeight = image[1];
    int bitmapSize = image[2];
    drawBitmapAlign8(x, y, imgWidth, imgHeight, image+4, bitmapSize);
    //drawBitmap(x, y, imgWidth, imgHeight, image+4, bitmapSize);
}


int ICACHE_FLASH_ATTR gfxDrawCharBig(const uint **font, int x, int y, uchar ch)
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
    	return 0;
    }
    int chWidth = chHeader[0];
    int chHeight = chHeader[1];
    int bitmapSize = chHeader[2];
    int yoffset = chHeader[3];
    drawBitmapAlign8(x, y+yoffset, chWidth, chHeight, font[ch]+4, bitmapSize);
    //drawBitmap(x, y+yoffset, chWidth, chHeight, font[ch]+4, bitmapSize);
    return chWidth;
}

int ICACHE_FLASH_ATTR gfxDrawStrBig(const uint **font, int x, int y, const char *str)
{
	int strWidth = 0;
    while (*str)
    {
        char ch = *str;
        int chWidth = gfxDrawCharBig(font, x, y, ch);
        x += chWidth;
        strWidth += chWidth;
        str++;
    }
    return strWidth;
}

LOCAL int ICACHE_FLASH_ATTR gfxCharWidthBig(const uint **font, uchar ch)
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
    	return 0;
    }
    return chHeader[0];
}

int ICACHE_FLASH_ATTR gfxStrWidthBig(const uint **font, const char *str)
{
	int strWidth = 0;
    while (*str)
    {
        char ch = *str;
        int chWidth = gfxCharWidthBig(font, ch);
        strWidth += chWidth;
        str++;
    }
    return strWidth;
}


int gfxDrawChar(const uint *font, int x, int y, uchar ch)
{
	int first = font[0];
	int last = font[1];
	if (ch < first || ch > last)
	{
		return 0;
	}
	int chIdx = ch-(first-2);
	int chOffset = font[chIdx];
	if (!chOffset)	// char not found
	{
		return 0;
	}
	volatile uint header = font[chOffset];
    uchar chWidth = header>>24;
    if (ch == ' ')  // skip space
    {
        return chWidth;
    }
    uchar chHeight = header>>16;
    uchar bitmapSize = header>>8;
    uchar yoffset = header;
	//debug("0x%08X, %u, %u, %u, %u\n", header, chWidth, chHeight, bitmapSize, yoffset);

    drawBitmap(x, y+yoffset, chWidth, chHeight, &font[chOffset+1], bitmapSize);
    return chWidth;
}

int gfxDrawStr(const uint *font, int x, int y, const char *str)
{
	int strWidth = 0;
    while (*str)
    {
        char ch = *str;
        int chWidth = gfxDrawChar(font, x, y, ch);
        x += chWidth;
        strWidth += chWidth;
        str++;
    }
    return strWidth;
}

LOCAL int ICACHE_FLASH_ATTR gfxCharWidth(const uint *font, uchar ch)
{
	int first = font[0];
	int last = font[1];
	if (ch < first || ch > last)
	{
		return 0;
	}
	int chIdx = ch-(first-2);
	int chOffset = font[chIdx];
	if (!chOffset)	// char not found
	{
		return 0;
	}
	volatile uint header = font[chOffset];
	uchar chWidth = header>>24;
	return chWidth;
}

int gfxStrWidth(const uint *font, const char *str)
{
	int strWidth = 0;
    while (*str)
    {
        char ch = *str;
        int chWidth = gfxCharWidth(font, ch);
        strWidth += chWidth;
        str++;
    }
    return strWidth;
}


int ICACHE_FLASH_ATTR gfxDrawStrCentredBig(const uint **font, int centre, int y, const char *str)
{
	int strWidth = gfxStrWidthBig(font, str);
	int x = centre-(strWidth/2);
	x = alignTo8(x);	// align to nearest 8 pixel boundary
	return gfxDrawStrBig(font, x, y, str);
}

int ICACHE_FLASH_ATTR gfxDrawStrAlignRightBig(const uint **font, int right, int y, const char *str)
{
	int strWidth = gfxStrWidthBig(font, str);
	int x = right-strWidth;
	return gfxDrawStrBig(font, x, y, str);
}

int ICACHE_FLASH_ATTR gfxDrawStrCentred(const uint *font, int centre, int y, const char *str)
{
	int strWidth = gfxStrWidth(font, str);
	int x = centre-(strWidth/2);
	return gfxDrawStr(font, x, y, str);
}

int ICACHE_FLASH_ATTR gfxDrawStrAlignRight(const uint *font, int right, int y, const char *str)
{
	int strWidth = gfxStrWidth(font, str);
	int x = right-strWidth;
	return gfxDrawStr(font, x, y, str);
}


void ICACHE_FLASH_ATTR gfxDrawLine(int x0, int y0, int x1, int y1, char color)
{
    int dx = abs(x1-x0), sx = x0<x1 ? 1 : -1;
    int dy = abs(y1-y0), sy = y0<y1 ? 1 : -1;
    int err = (dx>dy ? dx : -dy)/2, e2;

    for(;;)
    {
        gfxDrawPixel(x0,y0,color);
        if (x0==x1 && y0==y1) break;
        e2 = err;
        if (e2 >-dx) { err -= dy; x0 += sx; }
        if (e2 < dy) { err += dx; y0 += sy; }
    }
}

void ICACHE_FLASH_ATTR gfxDrawcircle(int x0, int y0, int radius, char color, char fill)
{
    int x = radius;
    int y = 0;
    int err = 0;

    while (x >= y)
    {
    	if (fill)
    	{
    		gfxDrawLine(x0 - x, y0 + y, x0 + x, y0 + y, color);
    		gfxDrawLine(x0 - y, y0 + x, x0 + y, y0 + x, color);
    		gfxDrawLine(x0 - x, y0 - y, x0 + x, y0 - y, color);
    		gfxDrawLine(x0 - y, y0 - x, x0 + y, y0 - x, color);
    	}
    	else
    	{
            gfxDrawPixel(x0 + x, y0 + y, color);
            gfxDrawPixel(x0 + y, y0 + x, color);
            gfxDrawPixel(x0 - y, y0 + x, color);
            gfxDrawPixel(x0 - x, y0 + y, color);
            gfxDrawPixel(x0 - x, y0 - y, color);
            gfxDrawPixel(x0 - y, y0 - x, color);
            gfxDrawPixel(x0 + y, y0 - x, color);
            gfxDrawPixel(x0 + x, y0 - y, color);
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

void ICACHE_FLASH_ATTR gfxDrawLineBold(int x0, int y0, int x1, int y1, char color, char boldX, char boldY)
{
	gfxDrawLine(x0, y0, x1, y1, color);
	if (boldX)
	{
		gfxDrawLine(x0-1, y0, x1-1, y1, color);
		gfxDrawLine(x0+1, y0, x1+1, y1, color);
	}
	if (boldY)
	{
		gfxDrawLine(x0, y0-1, x1, y1-1, color);
		gfxDrawLine(x0, y0+1, x1, y1+1, color);
	}
}

void ICACHE_FLASH_ATTR gfxDrawLineDotted(int x0, int y0, int x1, int y1, int space, char color)
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
        		gfxDrawPixel(x0,y0,color);
        	}
    		i++;
    	}

        if (x0==x1 && y0==y1) break;
        e2 = err;
        if (e2 >-dx) { err -= dy; x0 += sx; }
        if (e2 < dy) { err += dx; y0 += sy; }
    }
}

void ICACHE_FLASH_ATTR gfxDrawRectDotted(int x0, int y0, int x1, int y1, int space, char color)
{
	int x;
	for (x = x0; x <= x1; x+=(space+1))
	{
		gfxDrawLineDotted(x, y0, x, y1, space, color);
	}
}

void ICACHE_FLASH_ATTR gfxDrawRectFill(int x0, int y0, int x1, int y1, char color)
{
    if (x0 > x1) swapInt(&x0, &x1);
    if (y0 > y1) swapInt(&y0, &y1);

    const int idxFirst = x0 / 8;
    const int idxLast = x1 / 8;
    x0 &= 7;
    x1 &= 7;
    int y, yOffset;
    int i;
    uchar *pBuf;

    if (idxFirst == idxLast)    // single column -> special case
    {
        uchar mask = 0;
        for (i = x0; i <= x1; i++)
        {
            mask |= (0x80 >> i);
        }

        for (y = y0; y <= y1; y++)
        {
            // set/clear bits of single column
            pBuf = &gfxMem[y * GFXMEM_BYTEWIDTH + idxFirst];
            *pBuf = color ? *pBuf | mask : *pBuf & ~mask;
        }
    }
    else
    {
        const uchar maskFirst = (0xFF >> x0);
        const uchar maskLast = (0xFF << (7 - x1));
        const uchar maskMid = color * 0xFF;

        for (y = y0; y <= y1; y++)
        {
            yOffset = y * GFXMEM_BYTEWIDTH;

            // set/clear LSBits of fist column
            pBuf = &gfxMem[yOffset + idxFirst];
            *pBuf = color ? *pBuf | maskFirst : *pBuf & ~maskFirst;

            // set/clear all bits of middle columns
            memset(&gfxMem[yOffset + idxFirst + 1], maskMid, idxLast - (idxFirst + 1));

            // set/clear MSBits of last column
            pBuf = &gfxMem[yOffset + idxLast];
            *pBuf = color ? *pBuf | maskLast : *pBuf & ~maskLast;
        }
    }
}
