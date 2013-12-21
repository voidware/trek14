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
#include "os.h"
#include "libc.h"
#include "ent.h"
#include "plot.h"

void printfat(uchar x, uchar y, const char* fmt, ...)
{
    va_list args;

    setcursor(x, y);    
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    
    flush();
}

void lrScanQuad(uchar x, uchar y, uchar z, uchar* quad)
{
    uchar* ep = galaxy;

    memzero(quad, ENT_TYPE_COUNT);
    
    while (ep != galaxyEnd)
    {
        if (ENT_QX(ep) == x && ENT_QY(ep) == y && ENT_QZ(ep) == z)
            ++quad[ENT_TYPE(ep)];

        ep += ENT_SIZE;
    }
}

void lrScan()
{
    // long range scan

    char i, j;
    uchar quad[ENT_TYPE_COUNT];
    char x, y, z;
    uchar cx, cy;

    cls();
    printf("Long Range Scan, Quadrant %d %d %d\n", (int)QX, (int)QY, (int)QZ);

    y = 7; // 2*cy+1
    x = 0;
    for (i = 0; i < 4; ++i)
    {
        plotHLine(0, y, 125, 1);
        plotVLine(x, 3, 43, 1);
        plotVLine(x+1, 3, 43, 1);
        y += 4*3; // 4 lines
        x += (1 + 18)*2; // self + 18 chars
    }

    cy = 3;
    y = QY - 1;
    for (i = 0; i <= 2; ++i, ++y)
    {
        cx = 2; 
        x = QX - 1;
        for (j = 0; j <= 2; ++j, ++x)
        {
            char k;
            
            if (!i) printfat(cx + 8, 1, "%d", (int)x);
            z = QZ - 1;

            for (k = 0; k <= 2; ++k, ++z)
            {
                char buf[ENT_TYPE_COUNT*2+1];
                char* bp;
                if (x >= 0 && x < 8 && y >= 0 && y < 8 && z >= 0 && z < 3)
                {
                    const char* tc = entTypeChar;
                    uchar* cp = quad;
                    lrScanQuad(x, y, z, cp);
                    
                    bp = buf;
                    while (*bp = *tc++)
                    {
                        if (*cp)
                        {
                            bp[1] = '0' + *cp;
                        }
                        else
                        {
                            *bp = ' ';
                            bp[1] = ' ';
                        }
                        bp += 2;
                        ++cp;
                    }
                    bp = buf;
                }
                else
                {
                    bp = (char*)"Unknown Quadrant";
                }
                printfat(cx, cy + k, bp);
            }
            cx += 19;
        }
        --cx;
        printfat(cx, cy, "%2d", (int)(QZ-1));
        printfat(cx, cy + 1, "%2d %d", (int)QZ, (int)y);
        printfat(cx, cy + 2, "%2d", (int)(QZ+1));
        cy += 4;
    }
    printfat(0, 15, "Now What?");
}


