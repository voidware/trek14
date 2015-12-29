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

// helper utility functions

#include "defs.h"
#include "os.h"
#include "libc.h"

void memzero(void* ptr, size_t n)
{
    memset(ptr, 0, n);
}

void printfat(uchar x, uchar y, const char* fmt, ...)
{
    // printf at (x,y) character position
    // NB: text is automatically flushed (without need for "\n")
    
    va_list args;
    setcursor(x, y);    
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    flush();
}

#if 0
unsigned char isqrt16(unsigned short a)
{
    // 16 bit version, valid up to 16383
    unsigned short rem = 0;
    unsigned char root = 0;
    unsigned char i;
    for (i = 0; i < 8; ++i)
    {
        root <<= 1;
        rem = (rem << 2) + (a >> 14);
        a <<= 2;
        if (root < rem)
        {
            ++root;
            rem -= root;
            ++root;
        }
    }
    return root >> 1;
}
#endif

static const unsigned int exptab[] =
{
        710, 256, 1,
        355, 16, 1,
        177, 4, 1,
        89, 2, 1,
        52, 3, 2,
        29, 5, 4,
        15, 9, 8,
        8, 17, 16,
        4, 33, 32,
        2, 65, 64,
        1, 129, 128,
};

unsigned int expfixed(unsigned int v)
{
    // fixed point, value * 128 without multiply and divide!
    unsigned int r = 128;
    unsigned char n = DIM(exptab)/3;
    unsigned char more;
    do
    {
        unsigned char i;
        const int* tp = exptab;
        more = 0;
        for (i = 0; i < n; ++i)
        {
            if (*tp <= v)
            {
                int r0;
                unsigned char d;

                // subtract value column
                v -= *tp;

                // if numerator is odd, then it is 2^k+1, so we rememeber
                // the odd `r' and add it later
                
                r0 = 0;
                d = tp[1];
                if (d & 1)
                {
                    --d;
                    r0 = r;
                }

                // then perform the 2^k * r for the numerator
                while (d > 1)
                {
                    d >>= 1;
                    r <<= 1;
                }
                
                // add back any odd `r'
                r += r0;

                // denominator is always power of 2
                d = tp[2];
                while (d > 1)
                {
                    d >>= 1;
                    r >>= 1;
                }

                more = 1;
                break;
            }
            tp += 3;
        }
    } while (more);

    return r;
}


#define cordic_1K 0x26DD

static const short cordicTab[] = {0x3243, 0x1DAC, 0x0FAD, 0x07F5, 0x03FE, 0x01FF, 0x00FF, 0x007F, 0x003F, 0x001F, 0x000F, 0x0007, 0x0003, 0x0001 };

static void cordic(short theta, short *s, short *c)
{
    short x=cordic_1K,y=0,z=theta;
    unsigned char k;

    for (k=0; k<14; ++k)
    {
        short tx, ty;

        tx = y>>k;
        ty = x>>k;
        if (z < 0)
        {
            x += tx;
            y -= ty;
            z += cordicTab[k];
        }
        else
        {
            x -= tx;
            y += ty;
            z -= cordicTab[k];
        }
    }  
    *c = x; *s = y;
}

void tanfxDeg(short v, short* s, short* c)
{
    // v in [-180, +180] degrees
    // return s = sin(v), c = cos(v), that we can use as ratio
    
    uchar neg = 0;
    
    if (v > 90)
    {
        v -= 180;
        neg = 1;
    }
    else if (v < -90)
    {
        v += 180;
        neg = 1;
    }

    // v now reduced to [-90,90]
    v *= 286; // pi/180 shifted
    
    cordic(v, s, c);

    if (neg) 
    {
        *s = -*s;
        *c = -*c;
    }
}

