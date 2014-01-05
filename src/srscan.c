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
#include "enemy.h"
#include "command.h"

void fillbg(char x, char y)
{
    outcharat(x, y, (x & 1) ? ' ' : '.');
}

void drawEnt(uchar* ep)
{
    uchar sx, sy;
    const EntObj* eo = objTable + ENT_TYPE(ep);
    ENT_SXY(ep, sx, sy);

    // convert sector position to pixel
    drawRLE((sx - (eo->_w>>1))<<1, sy*3, eo->_data, 1);
}

void undrawEnt(uchar* ep)
{
    uchar sx, sy;
    const EntObj* eo = objTable + ENT_TYPE(ep);
    uchar w2 = eo->_w;

    ENT_SXY(ep, sx, sy);

    // adjust to left pos
    sx -= w2>>1;
            
    while (w2)
    {
        --w2;
        fillbg(sx++, sy);
    }
}

char moveEnt(uchar* ep, char dx, char dy)
{
    // move entity `ep' by `dx' `dy' & redraw
    // return >0, expired
    // return <0 border crossed
    // return 0, ok

    uchar sx, sy;
    char c;

    if (!takeEnergy(ep, ABS(dx) + ABS(dy)))
        return 1; // run out of energy, expired

    ENT_SXY(ep, sx, sy);

    // allow border crossing
    c = setSector(ep, sx+dx, sy+dy, 1);

    // if ok, but not crossed border
    if (!c) 
    {
        uchar x, y;
        const EntObj* obj = objTable + ENT_TYPE(ep);
        uchar w2 = obj->_w;

        sx -= w2>>1;
            
        if (dy)
        {
            drawEnt(ep);
            while (w2)
            {
                --w2;
                fillbg(sx++, sy);
            }
        }
        else if (dx)
        {
            x = sx<<1;
            y = sy*3;
            moveRLE(x, y, obj->_data, (dx < 0));            
            moveRLE(x+dx, y, obj->_data, (dx < 0));            
            if (dx < 0)
                fillbg(sx + w2 - 1, sy);                
            else
                fillbg(sx, sy);
        }
    }

    return c;
}

void showState()
{
    unsigned int d;

    // advance time `here, since we always refresh the status line each move
    ++stardate;

    d = stardate/10;
    printfat(34, 0, "E:%-4d T:%d D:%d.%d",
             ENT_ENERGY(galaxy), ENT_TORPS(galaxy),
             d, (stardate - d*10));
}

char srScan(char k)
{
    // short range scan.
    // draw quadrant screen & content.

    uchar** epp;
    int i;
    char c;

 again:
    
    printfat(0,0, "Short Range Scan, Quadrant %d %d %d", 
             (int)QX, (int)QY, (int)QZ);
    showState();

    setcursor(0,1);
    for (i = 64*7; i > 0; --i) { outchar('.'); outchar(' '); }

    // draw items in this quadrant
    epp = quadrant;
    while (*epp)
        drawEnt(*epp++);
    
    for (;;)
    {
        char dx, dy;

        if (k) { c = k; k = 0; }
        else c = inkey();
        if (!c) continue;

        dx = 0;
        dy = 0;
        if (c == KEY_ARROW_LEFT)
            dx = -1;
        else if (c == KEY_ARROW_RIGHT)
            dx = 1;
        else if (c == KEY_ARROW_UP)
            dy = -1;
        else if (c == KEY_ARROW_DOWN)
            dy = 1;
        else if (c == ' ')
        {
            // dummy move!
        }
        else if (c == 'P')
        {
            phaserCommand();
        }
        else if (c == 'D')
        {
            docCommand();
        }
        else if (c == 'T')
        {
            torpCommand();
        }
        else break;

        if (moveEnt(galaxy, dx, dy) < 0)
        {
            // crossed border
            goto again;
        }

        enemyMove();
        showState();
    }

    return c;
}
