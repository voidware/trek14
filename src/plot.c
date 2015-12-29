/**
 * Copyright (c) 2013 Voidware Ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS," WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include "defs.h"
#include "plot.h"


// divide by 3, 0 to 47
const unsigned char div3tab[] = {
    0,0,0,1,1,1,2,2,2,3,3,3,4,4,4,5,5,5,6,6,6,7,7,7,8,8,8,9,9,9,10,10,10,11,11,11,12,12,12,13,13,13,14,14,14,15,15,15};

static const unsigned char leftCol[] = { 0x01, 0x04, 0x10 };
static const unsigned char rightCol[] =  { 0x02, 0x08, 0x20 };
static const unsigned char bothCol[] =  { 0x03, 0x0C, 0x30 };

void plot(char x, char y, uchar c)
{
    // plot pixel (x,y) colour c

    uchar q, r, mask;
    char* m;
    char v;
    
    r = y;
    q = div3tab[r];

    // 64*(y/3) + x/2
    m = VIDRAM + ((((int)q)<<6) + (x>>1));

    if (m > VIDEND) return;

    // remainder
    r -= q;
    r -= q;
    r -= q;

    if (x&1)
        mask = rightCol[r];
    else
        mask = leftCol[r];

    v = *m;
    if (v >= 0) v = 0x80;

    if (c)
        *m = v | mask;
    else
        *m = v & ~mask;
}


void plotSpan(char x, char y, uchar n, uchar c)
{
    // plot (x,y) to (x+n, y) colour c
    
    uchar q, mask;
    char* m;
    uchar k;
    char v;

    if (!n) return;

    q = div3tab[y];

    // 64*(y/3) + x/2
    m = VIDRAM + ((((int)q)<<6) + (x>>1));

    if (m > VIDEND) return;

    // remainder
    q = y - ((q<<1) + q);
    
    if (x&1)
    {
        v = *m;
        if (v >= 0) v = 0x80;
        if (c)
            *m = v | rightCol[q];
        else
            *m = v & ~rightCol[q];
        ++m;
        --n;
    }

    k = n>>1;

    mask = bothCol[q];
    while (k)
    {
        --k;
        v = *m;
        if (v >= 0) v = 0x80;
        if (c)
            *m = v | mask;
        else
            *m = v & ~mask;

        ++m;
        
        if (!(((int)m) & 63))
            return; // ran off line
    }

    if (n&1)
    {
        v = *m;
        if (v >= 0) v = 0x80;
        if (c)
            *m = v | leftCol[q];
        else
            *m = v & ~leftCol[q];
    }
}

void plotHLine(uchar x1, uchar y, uchar x2, uchar c)
{
    // plot from x1 to x2 (inclusive)
    plotSpan(x1, y, x2-x1+1, c);
}

void plotVLine(uchar x, uchar y1, uchar y2, uchar c)
{
    // plot from y1 to y2 inclusive
    do
    {
        plot(x,y1,c);
    } while (++y1 <= y2);

}

#if 0
void plotLine(char x1, char y1, char x2, char y2, plotfn* fn)
{
    // draw line (x1, y1) to (x2, y2) inclusive
    uchar ax, ay;
    char sx, sy;
    char x, y;

    sx = 1;
    ax = x2-x1; 
    if ((char)ax < 0)
    {
        ax = -ax;
        sx = -1;
    }
    else if (!ax)
        sx = 0;
    
    ax <<= 1;

    sy = 1;    
    ay = y2-y1;
    if ((char)ay < 0)
    {
        ay = -ay;
        sy = -1;
    }
    else if (!ay)
        sy = 0;
    
    ay <<= 1;

    x = x1;
    y = y1;
    
    if (ax>ay) 	
    {	
	char d = ay-(ax>>1);
	for (;;) 
        {
            (*fn)(x, y);
	    if (x==x2) return;
	    if (d>=0) 
            {
		y += sy;
		d -= ax;
	    }
	    x += sx;
	    d += ay;
	}
    }
    else        
    {
	char d = ax-(ay>>1);
	for (;;) 
        {
            (*fn)(x, y);
	    if (y==y2) return;
	    if (d>=0) 
            {
		x += sx;
		d -= ay;
	    }
	    y += sy;
	    d += ax;
	}
    }
}
#endif

uchar pixelsRLE(const uchar* dp, char* pix)
{
    // convert a sprite into an array of pixel offsets
    // write a number of (x,y) pairs to `pix' as offsets relative to the
    // sprite root.
    // return the number of pairs in the `pix' array
    //
    // XX ASSUME `pix' is big enough

    uchar c = 0;
    char x = 0;
    char y = 0;

    uchar pair;
    for (;;)
    {
        while (pair = *dp++)
        {
            x += pair >> 4; // skip
            pair &= 0xf;
            while (pair--)
            {
                *pix++ = x++;
                *pix++ = y;
                ++c;
            }
        }
        
        // flyback
        if (!*dp) break;
        x -= *dp++;
        ++y;
    }
    return c;
}

void drawRLE(char x, char y, const uchar* dp, uchar c)
{
    // plot run-line encoded (RLE) sprite, colour c
    
    // sprite format is made up of a sequence of skip/draw nibbles:
    // <skip-draw> ... <0> <flyback>
    // <skip-draw> ... <0> <flyback>
    // ...
    // 0x00

    uchar pair;
    for (;;)
    {
        while (pair = *dp++)
        {
            x += pair >> 4; // skip
            plotSpan(x, y, pair & 0xf, c); // plot
            x += pair & 0xf;
        }
        
        // flyback
        if (!*dp) break;
        x -= *dp++;
        ++y;
    }
}

void moveRLE(char x, char y, const uchar* dp, uchar left)
{
    // moves RLE sprite from (x,y) to (x-1, y) if `left'
    // or from (x,y) to (x+1, y)

    uchar pair;

    x -= left;
    for (;;)
    {
        while (pair = *dp++)
        {
            x += pair >> 4; // skip
            if (pair & 0xf)
            {
                plot(x,y,left);
                x += pair & 0xf;
                plot(x,y,!left);;
            }
        }
        
        // flyback
        if (!*dp) break;
        x -= *dp++;
        ++y;
    }

}

