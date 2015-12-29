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
#include "command.h"
#include "srscan.h"
#include "phasers.h"
#include "enemy.h"
#include "damage.h"
#include "sound.h"

uchar* findClosest(uchar* kp, uchar type)
{
    // find the closest entity of `type' to `kp'
    uchar** epp;
    uchar dist = 0xff;
    uchar* best;

    for (epp = quadrant; *epp; ++epp)
    {
        if (ENT_TYPE(*epp) == type)
        {
            uchar d = distance(kp, *epp);
            if (d <= dist)
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
    // is entity `ep' adjacent to one of `type'
    uchar sx, sy;
    char i, j;
    
    ENT_SXY(ep, sx, sy);
    for (i = -1; i <= 1; ++i)
    {
        for (j = -1; j <= 1; ++j)
        {
            char c = setSector(ep, sx + i, sy + j, 0);
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
            setSector(ep, sx, sy, 0);
        }
    }
    return 0;
}

static void klingonFire(uchar* kp, uchar dist)
{
    // consider firing
    unsigned int ke = ENT_DAT(kp);

    // fire if have at least half max energy
    if (ke >= ENT_ENERGYK_LIMIT/2)
    {
        // more likely to fire the closer the distance.
        uchar pow = 1;
        while (pow < dist) pow <<= 1; // 2^k >= dist
        
        if (!(rand16() & (pow-1)))
        {
            // fire all energy, but keep 256
            phasers(kp, ke - 256, ENT_TYPE_FEDERATION);
        }
    }
}

static int klingonRecharge(uchar* kp)
{
    uchar* star;
    int e = ENT_DAT(kp);

    // small recharge each move
    e += 64;

    star = findClosest(kp, ENT_TYPE_STAR);
    if (star)
    {
        // recharge from the star
        e += ENT_DAT(star);
    }
    
    e &= ENT_ENERGYK_LIMIT-1; // power of 2
    ENT_SET_DAT(kp, e);

    return e;
}

#if 0
typedef uchar (*maxfn)(uchar*,uchar*);

static uchar klingonTryMove(uchar* kp, uchar* target, maxfn fn)
{
    uchar sx, sy;
    char i, j;
    uchar best = 0;
    char dx, dy;

    ENT_SXY(kp, sx, sy);
    for (i = -1; i <= 1; ++i)
    {
        for (j = -1; j <= 1; ++j)
        {
            if (!setSector(kp, sx + i, sy + j, 0))
            {
                uchar d = (*fn)(kp, target);
                if (d >= best)
                {
                    dbest = d;
                    dx = i;
                    dy = j;
                }
            }
        }
    }

    // put back in original place
    setSector(kp, sx, sy, 0);

    // then move delta (if non-zero)
    // NB: can expire here and be deleted
    if (moveEnt(kp, dx, dy)) return 0;
    return 1;
}
#endif

static uchar klingonMove(uchar* kp)
{
    uchar* target = findClosest(kp, ENT_TYPE_FEDERATION);
    if (target)
    {
        uchar sx, sy;
        char i, j;
        char dx, dy;
        uchar dbest = 0xff;
        uchar flee;

        // if weak, keep away and recharge
        flee = klingonRecharge(kp) < ENT_ENERGYK_LIMIT/2;
        
        if (flee)
        {
            // flee!
            dbest = 0;
        }
     
        // consider adjacent sectors that we could move into.
        // either we want to reduce the distance to the target 
        // or increase it (fleeing).
        ENT_SXY(kp, sx, sy);
        for (i = -1; i <= 1; ++i)
        {
            for (j = -1; j <= 1; ++j)
            {
                if (!setSector(kp, sx + i, sy + j, 0))
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
        setSector(kp, sx, sy, 0);

        // then move delta (if non-zero)
        // NB: can expire here and be deleted
        if (moveEnt(kp, dx, dy)) return 0;
        
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


uchar hitEnergy(uchar* ep, unsigned int d)
{
    // hit with `d' units of energy
    // return 0 if `ep' expires.

    uchar you = ep == galaxy;
    uchar u = enoughEnergy(ep, d);

    if (u)
    {
        if (you)
            takeDamage(d);
    }
    else
    {
        if (you)
        {
            // you've been killed
            endgame(MSG_CODE_ENDGAME_KILLED);
        }
        else
        {
            // destroyed an enemy
            messageCode(MSG_CODE_DESTROYED);

            // XX need explosion here.
            undrawEnt(ep);
            removeEnt(ep);
            
            explosionSound();
        }
    }
    return u;
}


