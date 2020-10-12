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
#include "utils.h"
#include "ent.h"
#include "command.h"
#include "srscan.h"
#include "phasers.h"
#include "enemy.h"
#include "damage.h"
#include "sound.h"
#include "plot.h"

static uchar setMap(uchar* map, uchar x, uchar y)
{
    // `map' is 128 bytes of memory representing one bit for every
    // sector location x in [0,63] y in [0,15]

    // set location but return original value.
    
    uchar q = (y<<3) + (x>>3);
    uchar m = 1<<(x & 7);
    uchar r = map[q] & m;
    map[q] |= m;
    return r;
}

static uchar getMap(uchar* map, uchar x, uchar y)
{
    return map[(y<<3) + (x>>3)] & (1<<(x & 7));
}

typedef struct
{
    uchar*      abuf;
    uchar*      map;
    uchar       sp;
    uchar       xt;
    uchar       yt;
    char        dx;
    char        dy;
    uchar       x1;
    uchar       y1;
    uchar*      visit;
    uchar*      target;
} routing;

// originally a parameter to routing functions, but cheaper to make
// single static version.
static routing rt;

static void prepSectorBuf(uchar* src, uchar type)
{
    // prepare a bit buffer with the sector barrier map
    // also identify the target
    
    uchar** epp;
    uchar sw = getWidth(src);

    memset(rt.map, 0, 128);
    for (epp = quadrant; *epp; ++epp)
    {
        uchar x, y;
        
        // we are not a barrier!
        if (*epp == src) continue;
        
        ENT_SXY(*epp, x, y);
        
        if (ENT_TYPE(*epp) == type)
        {
            // set target, also not a barrier
            rt.xt = x;
            rt.yt = y;
            rt.target = *epp;
        }
        else
        {
            uchar w = getWidth(*epp);

            // barrier left = left dest + right source
            x -= (w>>1) + (sw - (sw>>1) - 1);
            
            // void region size = source right + barrier w + source left
            w += sw-1;
            
            // otherwise set as barrier width w
            // ASSUME barriers are only 1 high.
            while (w)
            {
                --w;
                setMap(rt.map, x++, y);
            }
        }
    }
}

#define ABUF_SIZE  1024

static uchar push(uchar x, uchar y, uchar d, uchar src)
{
    uchar* a = rt.abuf + (((int)rt.sp)<<2);
    if (a > rt.abuf + ABUF_SIZE - 4) return 0;
    ++rt.sp;
    
    *a++ = x;
    *a++ = y;
    *a++ = d;
    *a = src;
    return 1;
}

//  XY displacements for the 8 surrounding locations
static const char xytable[] =
{
    -1, -1, 0, -1, 1, -1,
    -1, 0, 1, 0,
    -1, 1, 0, 1, 1, 1,
};

static void router()
{
    uchar d = 0;
    uchar d2 = 0;
    
    for (;;)
    {
        uchar k;
        uchar ks;
        uchar ke;
        uint j;

        if (d == d2)
        {
            ks = 0;
            ke = rt.sp;
            d = 127; // max
            j = 0;

            // find overall min
            for (k = 0; k < ke; ++k, j+=4)
            {
                // min node not yet expanded
                if (rt.abuf[j+2] < d && !getMap(rt.map, rt.abuf[j], rt.abuf[j+1]))
                    d = rt.abuf[j+2];
                
            }
            d2 = d;
            if (d == 127)
            {
                //printfat(15,15, "CAN'T ROUTE!\n");
                return; // did not find any new routes
            }
        }
        else
        {
            // min is in new range
            ks = ke;
            ke = rt.sp;
            d = d2;
        }
        
        j = ((int)ks)<<2;        
        for (k = ks; k < ke; ++k, j += 4)
        {
            if (rt.abuf[j+2] == d)
            {
                char xb = rt.abuf[j];
                char yb = rt.abuf[j+1];
                uchar i;

                // mark expanded
                if (setMap(rt.map, xb, yb)) continue;
                
                for (i = 0; i < sizeof(xytable); i += 2)
                {
                    char x = xb + xytable[i];
                    char y = yb + xytable[i+1];
                    
                    if (x >= 0 && x < 64 && y >= 0 && y < 16)
                    {
                        uchar di = distm(x, y, rt.xt, rt.yt);
                        if (!di)
                        {
                            // unwind 
                            for (;;)
                            {
                                x = rt.abuf[j];
                                y = rt.abuf[j+1];
                                //printfat(x, y, "x");
                                j = ((int)rt.abuf[j+3])<<2;
                                if (!j)
                                {
                                    rt.dx = x - rt.x1;
                                    rt.dy = y - rt.y1;    
                                    break;
                                }
                            }

                            return;
                        }

                        // if not already visited, expand 
                        if (!setMap(rt.visit, x, y))
                        {
                            if (di < d2) d2 = di; // find next min
                            if (!push(x, y, di, k))
                            {
                                // failed, out of working buffer
                                //printfat(15,15, "FAILED!\n");
                                return; 
                            }
                        }
                    }
                }
            }
        }
    }
}

static void klingonFire(uchar* kp)
{
    // consider firing
    unsigned int ke = ENT_DAT(kp);

    // fire if have at least half max energy
    if (ke >= objTable[ENT_TYPE(kp)]._emax>>1)
    {
        uchar dist = distance(kp, galaxy)>>1;
        
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

static int enemyRecharge(uchar* kp, uint de)
{
    // add energy

    uint e = ENT_DAT(kp) + de;
    uint emax = objTable[ENT_TYPE(kp)]._emax;
    
    if (e > emax) e = emax;
    
    ENT_SET_DAT(kp, e);

    return e;
}

void moveAway(uchar* kp, char x, char y, char* dx, char* dy)
{
    // best direction to move kp away from (x,y)
    char sx, sy;
    uchar i;
    uchar dbest = 0;
    
    ENT_SXY(kp, sx, sy);
    *dx = 0;
    *dy = 0;

    for (i = 0; i < sizeof(xytable); i += 2)
    {
        char x1 = sx + xytable[i];
        char y1 = sy + xytable[i+1];
        if (!setSector(kp, x1, y1, 0))
        {
            uchar d = distm(x1, y1, x, y);
            if (d > dbest)
            {
                dbest = d;
                *dx = xytable[i];
                *dy = xytable[i+1];
            }
        }
    }
    
    // put back in original place
    setSector(kp, sx, sy, 0);
}

static uchar klingonMove(uchar* kp)
{
    // AI for Klingon movement
    
    char ttype = ENT_TYPE_FEDERATION;

    rt.dx = 0;
    rt.dy = 0;
    
    // if weak, keep away and recharge
    if (enemyRecharge(kp, 32) < KLINGON_ENERGY/2)
    {
        ttype = -1; // no target
        
        // if we have a star, head for that
        // klingons recharge from stars
        if (quadCounts[ENT_TYPE_STAR]) ttype = ENT_TYPE_STAR;
    }

    ENT_SXY(kp, rt.x1, rt.y1);
    
    if (ttype >= 0)
    {
        // barrier and expanded node map.
        // This two maps are bitmaps of the sector locations

        // note 1.5K of stack here!!
        uchar map[128];
        uchar visit[128];
        uchar abuf[ABUF_SIZE];
        
        rt.abuf = abuf;
        rt.map = map;
        rt.sp = 0;
        rt.visit = visit;

        // head for target
        prepSectorBuf(kp, ttype);

        // visit map starts same as barrier
        memmove(visit, map, 128);

        // push initial position and mark visited
        push(rt.x1, rt.y1, 100, 0);
        setMap(visit, rt.x1, rt.y1);

        // route to target
        router();
    }
    else
    {
        // run away!
        moveAway(kp, rt.x1, rt.y1, &rt.dx, &rt.dy);
    }

    if (rt.dx || rt.dy)
    {
        // NB: can expire here and be deleted
        char c = moveEnt(kp, rt.dx, rt.dy);
        
        if (c == 127) return 0;  // expired

        // bip sound as K moves
        bit_soundi(10, 1000);

        if (ttype == ENT_TYPE_STAR)
        {
            // near enough to recharge?
            if (collision(kp, rt.target) != 0)
            {
                int f = useSVC ? 1000 : 500; // XX M4 adjust

                // klingons recharge from stars, collect energy
                enemyRecharge(kp, ENT_DAT(rt.target));

                disableInterrupts();
                do 
                {
                    bit_sound(4, f);
                    f -= 10;
                } while (f > 100);
                enableInterrupts();
            }
        }
    }
 
    // consider firing
    klingonFire(kp);
    
    return 1;
}

    
void enemyMove()
{
    uchar** epp;
    for (epp = quadrant; *epp; ++epp)
    {
        if (mainType(*epp) == ENT_TYPE_KLINGON)
        {
            // if destroyed, back up one as list is regenerated
            if (!klingonMove(*epp)) --epp;
        }
    }
}

static char rand3(char* p)
{
    // random number -1,0,1,2
    char c = rand16();
    c = (c & 3) - 1;
    if (*p < 0) c = -c;
    return *p += c;
}

void explode(uchar* ep)
{
    uchar sx, sy;
    uchar n;
    uchar i;
    char* p;
    uchar h2, w2;
    int sd;

    const EntObj* eo = objTable + ENT_TYPE(ep);

    // XX must be enough space to convert entity to array of pixel pairs
    char pix[100*2];

    // get sector position
    ENT_SXY(ep, sx, sy);

    // half width and height in pixels
    h2 = (eo->_h*3)>>1;
    w2 = eo->_w;

    // convert to centre pixels
    sx = sx*2;
    sy = sy*3 + h2;

    // convert to array of pixels offset from (sx, sy)
    n = pixelsRLE(eo->_sprite, pix);

    // adjust for centre 
    p = pix;
    for (i = n; i > 0; --i)
    {
        *p++ -= w2;
        *p++ -= h2;
    }

    sd = 0;
    while (n && sd < 512)
    {
        char x, y;
        p = pix;
        for (i = 0; i < n; ++i)
        {
            x = sx + rand3(p++);
            y = sy + rand3(p++);

            if (x < 0 || y < 3 || y > 44)
            {
                // pixel died off screen
                memmove(p - 2, p, (--n - i)<<1);
            }
            else
                plot(x, y, 1);

            explode_sound(++sd);
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
        else
            messageCode(MSG_CODE_SURVIVED_HIT);
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

            // remove from screen
            undrawEnt(ep);

            // explosion with sound
            explode(ep);
            
            // remove entity from table
            removeEnt(ep);

            if (!gameover) playVictory();
                
            // indicate screen redraw
            redrawsr = TRUE;
        }
    }
    return u;
}


