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
static const unsigned char div3tab[] = {
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

    // have two loops, for set and unset
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

void plotHLine(int x1, int y, int x2, uchar c)
{
    if (x2 > x1)
        plotSpan(x1, x2-x1, y, c);
}

void plotVLine(int x, int y1, int y2, uchar c)
{
    if (y2 > y1)
    {
        do
        {
            plot(x,y1,c);
        } while (++y1 < y2);
    }
}

void drawRLE(char x, char y, uchar* dp, uchar c)
{
    // plot run-line encoded (RLE) sprite, colour c
    
    // sprite format is:
    // sequence of line alternations 
    // <byte number of alternations> <on-off pair> ... <byte flyback-x>
    // 
    // an on-off pair is a nibble, any part can be zero.

    for (;;)
    {
        uchar n = *dp++;
        if (!n) break; // empty line means done

        do 
        {
            uchar pair, m;
            pair = *dp++; // get on off pair
            m = pair >> 4;
            plotSpan(x, y, m, c); // on
            x += m + (pair & 0xf);
        } while (--n);

        // flyback
        x -= *dp++;
        ++y;
    }
}

void moveRLERight(char x, char y, uchar* dp)
{
    // moves RLE sprite from (x,y) to (x+1, y)
    for (;;)
    {
        uchar n = *dp++;
        if (!n) break; // empty line means done
        
        do 
        {
            uchar pair, m;

            pair = *dp++; // get on off pair
            
            m = pair >> 4;
            if (m > 0)
            {
                plot(x,y,0);
                x += m;
                plot(x,y,1);
            }
            
            x += pair & 0xf;
            
        } while (--n);

        // flyback
        x -= *dp++;
        ++y;
    }    
}
