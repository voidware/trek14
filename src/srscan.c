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
#include "command.h"

static void fillbg(uchar x, uchar y)
{
    outcharat(x, y, (x & 1) ? ' ' : '.');
}

char srScan()
{
    uchar** epp;
    int i;
    char c;
    uchar sx, sy;
    
    cls();
    printf("Short Range Scan, Quadrant %d %d %d\n", (int)QX, (int)QY, (int)QZ);

    for (i = 64*7; i > 0; --i) { outchar('.'); outchar(' '); }

    // use the current quadrant info

    epp = quadrant;
    while (*epp)
    {
        const EntObj* eo = objTable + ENT_TYPE(*epp);
        ENT_SXY(*epp, sx, sy);

        // convert sector position to pixel
        drawRLE((sx - (eo->_w>>1))<<1, sy*3, eo->_data, 1);
        ++epp;
    }    

    for (;;)
    {
        uchar* ep;
        uchar x, y;
        
        c = inkey();
        if (!c) continue;

        // our entity
        ep = galaxyEnd - ENT_SIZE;
        ENT_SXY(ep, sx, sy);        

        // convert to pixels        
        x = (sx - ENTOBJ_FEDSHIP_W/2)<<1;
        y = sy * 3;
        
        if (c == KEY_ARROW_LEFT)
        {
            if (!setSector(ep, sx-1, sy))
            {
                moveRLE(x, y, fedshipRLE, 1);
                moveRLE(x-1, y, fedshipRLE, 1);
                fillbg(sx + ENTOBJ_FEDSHIP_W/2, sy);
            }
        }
        else if (c == KEY_ARROW_RIGHT)
        {
            if (!setSector(ep, sx+1, sy))
            {
                moveRLE(x, y, fedshipRLE, 0);
                moveRLE(x+1, y, fedshipRLE, 0);
                fillbg(sx - ENTOBJ_FEDSHIP_W/2, sy);
            }
        }
        else if (c == KEY_ARROW_UP)
        {
            if (!setSector(ep, sx, sy-1))
            {
                drawRLE(x, y-3, fedshipRLE, 1);
                for (x = sx - ENTOBJ_FEDSHIP_W/2; x < sx + ENTOBJ_FEDSHIP_W; ++x) fillbg(x, sy);

            }
        }
        else if (c == KEY_ARROW_DOWN)
        {
            if (!setSector(ep, sx, sy+1))
            {
                drawRLE(x, y+3, fedshipRLE, 1);
                for (x = sx - ENTOBJ_FEDSHIP_W/2; x < sx + ENTOBJ_FEDSHIP_W; ++x) fillbg(x, sy);
            }
        }
        else break;
    }

    return c;
}
