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
#include "warp.h"
#include "command.h"
#include "srscan.h"
#include "phasers.h"
#include "enemy.h"


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
            uchar d = distance(kp, *epp);
            if (!best || d < dist)
            {
                dist = d;
                best = *epp;
            }
        }        
    }  
    return best;
}

uchar findAdjacent(uchar* ep, uchar type)
{
    uchar sx, sy;
    char i, j;
    
    ENT_SXY(ep, sx, sy);
    for (i = -1; i <= 1; ++i)
    {
        for (j = -1; j <= 1; ++j)
        {
            char c = setSector(ep, sx + i, sy + j);
            if (c > 0) 
            {
                // hit something not boundary
                if (c-1 == type)
                {
                    // that will do
                    return 1;
                }
            }

            // restore
            setSector(ep, sx, sy);
        }
    }
    return 0;
}

static void klingonFire(uchar* kp, uchar dist)
{
    // consider firing
    unsigned int ke = ENT_ENERGY(kp);
    if (ke >= 200)
    {
        uchar pow = 1;
        while (dist > 0)
        {
            pow <<= 1;
            dist >>= 1;
        }
        if (!(rand16() & (pow-1)))
        {
            unsigned int e = ke - 100;
            phasers(kp, e, ENT_TYPE_FEDERATION);
        }
    }
}

static uchar klingonMove(uchar* kp)
{
    uchar* target = findClosest(kp, ENT_TYPE_FEDERATION);
    if (target)
    {
        uchar sx, sy;
        char i, j;
        char dx, dy;
        uchar dbest = -1;
        uchar flee = ENT_ENERGY(kp) < 200;

        if (flee)
        {
            // flee!
            dbest = 0;
        }
        
        ENT_SXY(kp, sx, sy);
        for (i = -1; i <= 1; ++i)
        {
            for (j = -1; j <= 1; ++j)
            {
                if (!setSector(kp, sx + i, sy + j))
                {
                    uchar d = distance(kp, target);
                    if (flee && d > dbest || !flee && d < dbest)
                    {
                        dbest = d;
                        dx = i;
                        dy = j;
                    }
                }
            }
        }

        // put back in original place
        setSector(kp, sx, sy);

        // then move delta (if non-zero)
        // NB: can expire here and be deleted
        if (!moveEnt(kp, dx, dy)) return 0;
        
        // fire?
        klingonFire(kp, dbest);

    }
    return 1;
}

void enemyMove()
{
    uchar** epp;
    for (epp = quadrant; *epp; ++epp)
    {
        if (ENT_TYPE(*epp) == ENT_TYPE_KLINGON)
        {
            // if destroyed, back up one as list is regenerated
            if (!klingonMove(*epp)) --epp;
        }
    }
}

void removeEnt(uchar *ep)
{
    undrawEnt(ep);
    memmove(ep, ep + ENT_SIZE, galaxyEnd - ep - ENT_SIZE);
    galaxyEnd -= ENT_SIZE;

    // rebuild quadrant content
    updateQuadrant();
}

uchar takeEnergy(uchar* ep, unsigned int d)
{
    // return 0 if `ep' expires.

    int e = ENT_ENERGY(ep) - d;
    if (e >= 0)
    {
        ENT_SET_ENERGY(ep, e);
    }
    else
    {
        // blow up!
        if (ep != galaxy) 
        {
            message("Destroyed");
            removeEnt(ep);
        }
        
        // else you're out of energy & game over
    }
    return (e >= 0);
}

