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
#include "warp.h"
#include "damage.h"
#include "lrscan.h"
#include "sound.h"
#include "alert.h"

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
    {
        undrawEnt(ep); // disappear!
        return 1; // run out of energy, expired
    }

    ENT_SXY(ep, sx, sy);

    // allow border crossing
    c = setSector(ep, sx+dx, sy+dy, 1);

    // if ok, but not crossed border
    if (!c && opCheckSR()) 
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

            // move two pixels, either right of left
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

    if (quadCounts[ENT_TYPE_KLINGON])
    {
        if (!alertLevel)
        {
            alertLevel = 1;
            alert("Red Alert", 3);
        }
    }
    else
    {
        if (alertLevel)
            alert("Condition Green", 1);
        
        alertLevel = 0;
    }
        
    d = stardate/10;
    printfat(31, 0, "E:%-4d T:%d S:%d D:%d.%d %s",
             ENT_ENERGY(galaxy),
             ENT_TORPS(galaxy),
             GET_SHIELD_ENERGY,
             d, (stardate - d*10),
             (alertLevel ? "RED " : "Green"));
}

static void animateOut()
{
    // draw warp out animation with sound
    
    uchar x, y;
    const EntObj* obj = objTable + ENT_TYPE_FEDERATION;
    uchar w2 = obj->_w;
    int f = 1000;
    uchar df = 10;
    uchar n;

    // current sector
    ENT_SXY(galaxy, x, y);
    x -= w2>>1; // left position

    n = 63 - x - w2; // -1 for first movement
    
    // convert to pixels
    x <<= 1;
    y *= 3;
    n <<= 1;

    for (;;)
    {
        if (n > 0)
        {
            --n;
            ++x;
            drawRLE(x, y, obj->_data, 1);
        }

        // make the sound increase in pitch until it reaches a final point
        // if we've finished moving, then we're done
        bit_sound(4, f);
        f -= df;
        if (f < 100)
        {
            if (!n) break;
            f = 100;
        }
    }
}

char srScan(char k)
{
    // short range scan.
    // draw quadrant screen & content.

    uchar** epp;
    int i;
    char c;

    mline = 15;

 again:

    tick();
    showState();
    
    printfat(0,0, "Short Range Scan, Quadrant %d%d%d", 
             (int)QX, (int)QY, (int)QZ);

    // we are redrawing, so clear this
    redrawsr = false;

    setcursor(0,1);
    for (i = 64*7; i > 0; --i) { outchar('.'); outchar(' '); }

    if (opCheckSR())
    {
        // draw items in this quadrant. Assume here, `quadrant' is correctly
        // setup
        epp = quadrant;
        while (*epp)
            drawEnt(*epp++);
    }
    
    // enter interactive loop on SR view
    for (;;)
    {
        char dx, dy;
        uchar moved = 0;

        // operate command passed in, if any
        if (k) { c = k; k = 0; }
        else
        {
            c = getkey();
            if (islower(c)) c = _toupper(c);
        }
        
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
            // NB: enemy move called here early to allow
            // enemy to (maybe) dodge torpedo
            moved = torpCommand();
        }
        else if ((c & 0x7f) == 'W')
        {
            if (c != 'W' || warpCommand())
            {
                animateOut();
                warp();
            }
            goto again; // new quadrant
        }
        else break;

        if ((dx || dy) && opCheck(L_IMPULSE))
        {
            if (moveEnt(galaxy, dx, dy) < 0)
            {
                // crossed border
                goto again;
            }
        }

        if (!moved)
        {
            // any enemies here get a turn
            enemyMove();
        }

        // redraw blank SR
        if (!opCheckSR()) goto again;

        showState();

        if (redrawsr) goto again;
        
        // update the state
        tick();
    }

    return c;
}
