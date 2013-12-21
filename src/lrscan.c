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

void lrScan()
{
    // long range scan

    char i, j, k;
    uchar x, y;
    uchar cx, cy;

    char qx = 7;
    char qy = 7;
    char qz = 0;

    cls();
    printf("Long Range Scan, Quadrant %d %d %d\n", (int)qx, (int)qy, (int)qz);

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
    --qy;
    --qx;
    --qz;
    for (i = 0; i <= 2; ++i)
    {
        cx = 2; 
        for (j = 0; j <= 2; ++j)
        {
            if (!i) printfat(cx + 8, 1, "%d", (int)(qx+j));
            for (k = 0; k <= 2; ++k)
            {
                printfat(cx, cy + k, "unknown quadrant");
            }
            cx += 19;
        }
        --cx;
        printfat(cx, cy, "%2d", (int)(qz-1));
        printfat(cx, cy + 1, "%2d %d", (int)qz, (int)(qy+i));
        printfat(cx, cy + 2, "%2d", (int)(qz+1));
        cy += 4;
    }
    printfat(0, 15, "Now What?");
}


