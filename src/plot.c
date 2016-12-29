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
#include "os.h"


// divide by 3, 0 to 47
const unsigned char div3tab[] = {
    0,0,0,1,1,1,2,2,2,3,3,3,4,4,4,5,5,5,6,6,6,7,7,7,8,8,8,9,9,9,10,10,10,11,11,11,12,12,12,13,13,13,14,14,14,15,15,15};

static const unsigned char leftCol[] = { 0x01, 0x04, 0x10 };
static const unsigned char rightCol[] =  { 0x02, 0x08, 0x20 };
static const unsigned char bothCol[] =  { 0x03, 0x0C, 0x30 };

void plot(uchar x, uchar y, uchar c)
{
    // plot pixel (x,y) colour c

    uchar q, r, mask;
    char* m;
    signed char v;
    
    r = y;
    q = div3tab[r];

    // 64*(y/3) + x/2
    m = vidaddr(x>>1, q);
    if (!m) return;  // not within screen 
	
    // remainder
    r -= q;
    r -= q;
    r -= q;

    mask = leftCol[r];
    if (x&1) mask += mask; // rightCol
	
    v = *m;
    if (v >= 0) v = 0x80;

    if (c)
        *m = v | mask;
    else
        *m = v & ~mask;
}

uchar plotSpan(uchar x, uchar y, uchar n0, uchar c)
{
    // plot (x,y) to (x+n, y) colour c
    
    uchar q, mask;
    char* m;
    signed char v;
    uchar n = n0;

    if (n)
    {
        q = div3tab[y];

        // cols*(y/3) + x/2
        m = vidaddr(x>>1, q);
        if (!m) return 0;

        // remainder
        q = y - q*3;
    
        if (x&1)
        {
            v = *m;
            if (v >= 0) v = 0x80;
            if (c)
                *m = v | rightCol[q];
            else
                *m = v & ~rightCol[q];
            ++m;
            ++x;
            --n;
        }

        mask = bothCol[q];
        while (n > 1)
        {
            if (x >= 160) return;
            if (x >= 128 && !cols80) return;
            n -= 2;
            v = *m;
            if (v >= 0) v = 0x80;
            if (c)
                *m = v | mask;
            else
                *m = v & ~mask;

            ++m;
            ++x;
        }

        if (n)
        {
            v = *m;
            if (v >= 0) v = 0x80;
            if (c)
                *m = v | leftCol[q];
            else
                *m = v & ~leftCol[q];
        }
    }
    return n0;
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

#if 0
void drawRLE(char x, char y, const uchar* dp, uchar c)
{
    // plot run-line encoded (RLE) sprite, colour c
    // sprite format is made up of a sequence of skip/draw nibbles:
    // <skip-draw> ... <0> <flyback>
    // <skip-draw> ... <0> <flyback>
    // ...
    // 0x00

    // note that this does not reset the `skip` regions of the
    // sprite.

    uchar pair;
    for (;;)
    {
        while (pair = *dp++)
        {
            x += pair >> 4; // skip
            x += plotSpan(x, y, pair & 0xf, c); // plot
        }
        
        // flyback
        if (!*dp) break;
        x -= *dp++;
        ++y;
    }
}
#endif

void moveRLE(char x, char y, const uchar* dp, signed char dx)
{
    // move left or right according to dx. dx = +/-1 or 0
    // moves RLE sprite from (x,y) to (x-1, y) if `dx=-1`
    // or from (x,y) to (x+1, y) if dx=1.

    // plot run-line encoded (RLE) sprite, colour c
    // sprite format is made up of a sequence of skip/draw nibbles:
    // <skip-draw> ... <0> <flyback>
    // <skip-draw> ... <0> <flyback>
    // ...
    // 0x00

    uchar pair;

    for (;;)
    {
        // if moving right, set first pixel of row
        if (dx > 0) plot(x, y, 0);

        // adjust start of line span
        x += dx;

        // plot each line span of sprite
        while (pair = *dp++)
        {
            x += plotSpan(x, y, pair >> 4, 0); // reset skip
            x += plotSpan(x, y, pair & 0xf, 1); // set region
        }

        // if moving left, reset last pixel of row
        if (dx < 0)
            plot(x, y, 0);
        
        // flyback with adjustment
        if (!*dp) break;
        x -= (*dp++ + dx);
        ++y;
    }
}

