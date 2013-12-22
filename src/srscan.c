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

// quadrant view

#include "defs.h"
#include "os.h"
#include "libc.h"
#include "utils.h"
#include "ent.h"
#include "plot.h"
#include "srscan.h"

extern uchar fedship[];

void srScan()
{
    uchar* ep;
    int i;

    cls();
    printf("Short Range Scan, Quadrant %d %d %d\n", (int)QX, (int)QY, (int)QZ);

    for (i = 64*7; i >= 0; --i) { outchar('.'); outchar(' '); }
    

    ep = galaxy;
    while (ep != galaxyEnd)
    {
        if (ENT_QX(ep) == QX && ENT_QY(ep) == QY && ENT_QZ(ep) == QZ)
        {
            uchar t = ENT_TYPE(ep);
            uchar sx, sy;
            const EntObj* obj = objTable + t;

            ENT_SXY(ep, sx, sy);

            drawRLE(sx<<1,sy*3, obj->_data, 1);
        }
        ep += ENT_SIZE;
    }    

    conn();
}
