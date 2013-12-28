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

// enemy AI

#include "defs.h"
#include "os.h"
#include "libc.h"
#include "utils.h"
#include "ent.h"
#include "enemy.h"
#include "warp.h"
#include "command.h"
#include "srscan.h"

uchar distance(uchar* ep1, uchar* ep2)
{
    // distance between two entities
    uchar x1, y1;
    uchar x2, y2;
    ENT_SXY(ep1, x1, y1);
    ENT_SXY(ep2, x2, y2);

    // use hypot approx |a| + |b| - min(|a|,|b|)/2 
    if ((char)(x2 -= x1) < 0) x2 = -x2;
    if ((char)(y2 -= y1) < 0) y2 = -y2;
    return x2 + y2 - (((x2>y2) ? y2 : x2) >> 1);
}

uchar* findClosest(uchar* kp, uchar type)
{
    uchar** epp;
    uchar dist;
    uchar* best = 0;

    for (epp = quadrant; *epp; ++epp)
    {
        if (ENT_TYPE(*epp) == type)
        {
            uchar d;
            if (!best || (d = distance(kp, *epp) < dist))
            {
                dist = d;
                best = *epp;
            }
        }        
    }  
    return best;
}

void klingonMove(uchar* kp)
{
    uchar* target = findClosest(kp, ENT_TYPE_FEDERATION);
    if (target)
    {
        uchar sx, sy;
        char i, j;
        uchar dmin = -1;
        char dx, dy;
        
        ENT_SXY(kp, sx, sy);
        for (i = -1; i <= 1; ++i)
        {
            for (j = -1; j <= 1; ++j)
            {
                if (!setSector(kp, sx + i, sy + j))
                {
                    uchar d = distance(kp, target);
                    if (d < dmin)
                    {
                        dmin = d;
                        dx = i;
                        dy = j;
                    }
                }
            }
        }

        // put back in original place
        setSector(kp, sx, sy);

        // then move delta (if non-zero)
        moveEnt(kp, dx, dy);
    }
}

void enemyMove()
{
    uchar** epp;
    for (epp = quadrant; *epp; ++epp)
    {
        if (ENT_TYPE(*epp) == ENT_TYPE_KLINGON)
            klingonMove(*epp);
    }
}

void removeEnt(uchar *ep)
{
    undrawEnt(ep);
    memmove(ep, ep + ENT_SIZE, galaxyEnd - ep - ENT_SIZE);
    galaxyEnd -= ENT_SIZE;

    // regenerate tables for this quadrant
    warp(QX, QY, QZ);
}

char takeDamage(uchar* ep, unsigned int d)
{
    // return < 0, means destroyed
    
    int e;
    e = ENT_ENERGY(ep) - d;
    
    //printfat(40,1, "%u %u = %d", (int)ENT_ENERGY(ep), d, e); flush();

    if (e >= 0)
    {
        ENT_SET_ENERGY(ep, e);
    }
    else
    {
        // blow up!
        message("Destroyed");
        removeEnt(ep);
    }
    return (char)e;
}

