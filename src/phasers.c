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
#include "utils.h"
#include "ent.h"
#include "plot.h"
#include "srscan.h"
#include "phasers.h"

static void setPixel(char x, char y)
{
    plot(x, y, 1);
}

void phasers(uchar* ep, int e, uchar type)
{
    // entity `ep' fire phasers at `type' energy `e'
    
    uchar sx, sy;
    int en;
    uchar** qp;

    en = ENT_ENERGY(ep);

    ENT_SXY(ep, sx, sy);

    // shoot from the front
    sx += objTable[ENT_TYPE(ep)]._w>>1;
    
    for (qp = quadrant; *qp; ++qp)
    {
        if (ENT_TYPE(*qp) == type)
        {
            if (en >= e)
            {
                uchar ex, ey;
            
                en -= e;
                ENT_SET_ENERGY(ep, en);

                ENT_SXY(*qp, ex, ey);

                plotLine(sx<<1, sy*3 + 1, ex<<1, ey*3 + 1, setPixel);
                plotLine(sx, sy, ex, ey, fillbg);
            }
        }
    }
}

