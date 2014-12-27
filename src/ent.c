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
#include "ent.h"
#include "libc.h"
#include "utils.h"
#include "warp.h"

uchar galaxy[ENT_COUNT_MAX*ENT_SIZE];
uchar* galaxyEnd;
unsigned int stardate;
int score;
const char entTypeChar[] = { 'B', 'F', 'S', 'P', 'K', 'R', 0 };

// current location
uchar QX, QY, QZ;

// entities in current quadrant
uchar quadCounts[ENT_TYPE_COUNT];
uchar* quadrant[ENT_QUAD_MAX];


// RLE sprites
static const uchar base[] = { 0x33, 0x00, 0x01,
                              0x01, 0x00, 0x06,
                              0x0c, 0x00,
                              0x00 };

const uchar fedshipRLE[] = { 0x06, 0, 0x02,
                             0x01, 0x47, 0, 0x0d,
                             0x0b, 0x00,
                             0x00 };

static const uchar star[] = { 0x02, 0x22, 0, 0x05,
                              0x04, 0, 0x05,
                              0x02, 0x22, 0x00,
                              0x00 };

static const uchar planet[] = { 0x15, 0, 0x06,
                                0x07, 0, 0x06,
                                0x05, 0,
                                0x00  };

static const uchar klingon[] = { 0x02, 0x72, 0, 0x0b,
                                 0x0b, 0, 0x07,
                                 0x03, 0,
                                 0x00 };

static const uchar romulan[] = { 0x02, 0x27, 0x20, 0x0b,
                                 0x01, 0xb0, 0x07,
                                 0x01, 0x30, 0x00,
                                 0x00 };


// pixel to char width
#define CW(_v) (((_v)+1)>>1)
                              
const EntObj objTable[] =
{
    { CW(12), 3, -1000, base },
    { CW(16), 3, -1000, fedshipRLE }, 
    { CW(6), 3, 0, star },
    { CW(7), 3, 0, planet },
    { CW(11), 3, SCORE_KLINGON, klingon },
    { CW(0), 0, SCORE_KLINGON, romulan },
    { 1, 1, 0, romulan }, // additional entry used for torpedo
};

unsigned int rand16()
{
    static unsigned short seed = 0;
    unsigned short v;
    uchar a;

    v = (seed + 1)*75;
    a = v;
    a -= (v >> 8);
    seed = ((v & 0xff00) | a) - 1;
    return seed;
}

void getQuad(uchar x, uchar y, uchar z, uchar* quadCounts, uchar** eplist)
{
    // get information about quadrant (x,y,z)
    // quadCounts is an array of counts indexed by ent type
    // eplist (optional) is a list of ent pointers, objects in this quadrant
    
    uchar* ep = galaxy;
    memzero(quadCounts, ENT_TYPE_COUNT);
    
    while (ep != galaxyEnd)
    {
        if (ENT_QX(ep) == x && ENT_QY(ep) == y && ENT_QZ(ep) == z)
        {
            // now seen if not already, increase explore score
            if (!ENT_MARKED(ep))
            {
                ENT_SET_MARK(ep);
                ++score;
            }

            if (eplist)
            {
                *eplist++ = ep;
                *eplist = 0;
            }
            ++quadCounts[ENT_TYPE(ep)];
        }
        ep += ENT_SIZE;
    }
}

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

uchar getWidth(uchar* ep)
{
    return objTable[ENT_TYPE(ep)]._w;
}

char collisionBorder(uchar* ep)
{
    // do we collide with the border, if so return the mask
    // otherwise 0
    
    uchar sx, sy;
    uchar w = getWidth(ep);

    ENT_SXY(ep, sx, sy);

    // logical draw pos
    sx -= (w>>1); 

    // overlap quadrant edge
    if ((char)sy < 1) return MASK_TOP;
    if ((char)sy > 14) return MASK_BOT;
    if ((char)sx < 0) return MASK_LEFT;
    if ((char)(sx + w) > 64) return MASK_RIGHT;
    return 0;
}

char collision(uchar* ep1, uchar* ep2)
{
    // if object `ep1' overlaps `ep2'

    uchar w1, x1, y1;
    uchar w2, x2, y2;

    w1 = getWidth(ep1);
    ENT_SXY(ep1, x1, y1);

    // logical draw pos
    x1 -= (w1>>1); 

    ENT_SXY(ep2, x2, y2);

    // for now assume everyone is the same height, so to collide they
    // must be on the same line
    if (y1 == y2)
    {
        w2 = getWidth(ep2);
        x2 -= w2>>1;

        // collision!
        if (x2 + w2 > x1 && x2 < x1 + w1) 
            return ENT_TYPE(ep2) + 1;
    }
    return 0;
}

void updateQuadrant()
{
    // update list of things in this quadrant
    getQuad(QX, QY, QZ, quadCounts, quadrant);
}


uchar setQuadrant(uchar* ep, uchar x, uchar y, uchar z)
{
    if (x < 8 && y < 8 && z < 3)
    {
        ENT_SET_QX(ep, x);
        ENT_SET_QY(ep, y);
        ENT_SET_QZ(ep, z);

        // setting our quadrant?
        if (ep == galaxy)
        {
            // update current location variables
            QX = x;
            QY = y;
            QZ = z;

            // refresh content of quadrant
            updateQuadrant();
        }
        return 1;
    }
    return 0;
}

uchar setSector(uchar* ep, uchar x, uchar y, uchar cancross)
{
    // attempt to set the sector location of `ep' to (x,y)
    // if this results in collision or outside quadrant, restore
    // original location.
    // return 0 => ok
    // return >0 => collision (or border) fail
    // return <0 => border crossing
    
    uchar** qp;
    char c;

    // get old values (quad & sector)
    uchar oldx, oldy;
    oldx = ep[1];
    oldy = ep[2];

    // set new values
    ENT_SET_SXY(ep, x, y);

    c = collisionBorder(ep);
    if (c)
    {
        // have a border collision, but can we cross?
        if (cancross)
        {
            uchar qx, qy, qz;
            char dx, dy;
            uchar w2 = getWidth(ep)>>1;

            qx = ENT_QX(ep);
            qy = ENT_QY(ep);
            qz = ENT_QZ(ep);
            
            dx = dy = 0;
            if (c == MASK_RIGHT)
            { 
                dx = 1;
                x = w2;
            }
            if (c == MASK_LEFT)
            {
                dx = -1;
                x = 64 - w2;
            }
            if (c == MASK_TOP) 
            {
                dy = -1;
                y = 14; // assume all heights 1
            }
            if (c == MASK_BOT)
            {
                dy = 1;
                y = 1;
            }
            
            if (setQuadrant(ep, qx + dx, qy + dy, qz))
            {
                // check for collision across border
                if (!setSector(ep, x, y, 0))
                    return -1; // <0 => border crossed
            }

            // failed to cross, restore original quadrant
            setQuadrant(ep, qx, qy, qz);
        }
    }
    else
    {
        // check for collision with other entities
        for (qp = quadrant; *qp; ++qp)
        {
            if (ep != *qp && (c = collision(ep, *qp)))
                break;
        }
    }

    if (c)
    {
        // restore old values
        ep[1] = oldx;
        ep[2] = oldy;
    }

    return c;
}

void genSector(uchar* ep)
{
    uchar x, y;
    do
    {
        // put at a random location within the quadrant avoiding collisions
        x = rand16() & 63;
        y = rand16() & 15;
    } while (setSector(ep, x, y, 0));
}

static void genEntLocation(uchar* ep, uchar type, uchar tmax, uchar tmin)
{
    // pick a random location, ensuring type' does not
    // exceed `tmax'
    // avoiding quadrant QX, QY, QZ

    uchar x, y, z;
    
    do
    {
        // generate quadrant
        // call rand separately for each part because the lower bits
        // are more random.
        
        do {
            x = rand16() & 0x7;
            y = rand16() & 0x7;
            z = rand16() & 0x3;
        } while (z == 3 || (x == QX && y == QY && z == QZ));

        getQuad(x, y, z, quadCounts, quadrant);
        
    } while ((uchar)(quadCounts[type] + tmin) > tmax);

    while (tmin)
    {
        --tmin;
        
        ENT_SET_TYPE(ep, type);
        ENT_SET_QX(ep, x);
        ENT_SET_QZ(ep, z);
        ENT_SET_QY(ep, y);
        genSector(ep);
        ep += ENT_SIZE;
    } 
}


void clearMarks()
{
    // except the first entry
    uchar* ep = galaxy;
    for (;;)
    {
        ep += ENT_SIZE;
        if (ep == galaxyEnd) break;
        ENT_CLEAR_MARK(ep);
    } 
}

uchar enoughEnergy(uchar* ep, unsigned int d)
{
    // reduce energy by `d' if possible
    int e = ENT_ENERGY(ep) - d;
    if (e >= 0)
    {
        ENT_SET_ENERGY(ep, e);
        return 1;
    }
    return 0;
}

void genGalaxy()
{
    uchar i;
        
    // XX should be zero anyway once we clear BSS
    memzero(galaxy, sizeof(galaxy));

    // (adjust for bogus warp to start)
    stardate = STARDATE_START - STARDATE_WARP; 

    galaxyEnd = galaxy;

    // we are the first entity in the table
    genEntLocation(galaxyEnd, ENT_TYPE_FEDERATION, 1, 1);

    // full energy & photons
    ENT_SET_DAT(galaxyEnd, ENT_REFUEL_DATA);

    galaxyEnd += ENT_SIZE; 

    // our current location. setting this with prevent anything else from 
    // being put in this quadrant
    QX = 7;
    QY = 7;
    QZ = 2;

    // populate bases
    for (i = 0; i < 5; ++i)
    {
        genEntLocation(galaxyEnd, ENT_TYPE_BASE, 1, 1);
        galaxyEnd += ENT_SIZE; 
    }

    // generate starfleet HQ
    genEntLocation(galaxyEnd, ENT_TYPE_BASE, 1, 1);
    
    // change location to be our quadrant
    setQuadrant(galaxyEnd, QX, QY, QZ);
    galaxyEnd += ENT_SIZE; 

    // populate klingons
    i = 30;
    while (i > 0)
    {
        // between 1 & 3 klingons
        uchar n = rand16() & 3;

        if (!n) continue;
        if (n > i) n = i;
        
        genEntLocation(galaxyEnd, ENT_TYPE_KLINGON, 3, n);
        
        while (n)
        {
            --n;
            --i;

            // enemy has at least half its allowed energy
            ENT_SET_DAT(galaxyEnd, (rand16() & (ENT_ENERGYK_LIMIT/4-1)) + ENT_ENERGYK_LIMIT/2);
            galaxyEnd += ENT_SIZE;
        }
    }

    // populate planets & stars
    for (i = 0; i < 100; ++i)
    {
        genEntLocation(galaxyEnd, ENT_TYPE_STAR, 1, 1);

        // give stars a random energy between 128 and 256
        // this energy can be drawn by enemies to recharge
        ENT_SET_DAT(galaxyEnd, (rand16() & 127) + 128);
        galaxyEnd += ENT_SIZE;

        genEntLocation(galaxyEnd, ENT_TYPE_PLANET, 3, 1);
        galaxyEnd += ENT_SIZE;
    }

    clearMarks();

    // +1 for starfleet HQ, not to count
    score = -1;

    // warp to QX, QY, QZ
    warp(); 
}




