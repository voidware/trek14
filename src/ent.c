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
    { CW(12), 3, base },
    { CW(16), 3, fedshipRLE }, 
    { CW(6), 3, star },
    { CW(7), 3, planet },
    { CW(11), 3, klingon },
    { CW(0), 0, romulan },
    { 1, 1, romulan }, // additional entry used for torpedo
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

char collision(uchar* ep1, uchar* ep2)
{
    // if object `ep1' overlaps `ep2'
    // or if `ep1' does not fit properly in the quadrant

    uchar w1, x1, y1;
    uchar w2, x2, y2;

    w1 = objTable[ENT_TYPE(ep1)]._w;
    ENT_SXY(ep1, x1, y1);

    x1 -= (w1>>1); 

    // overlap quadrant edge
    if (!y1 || y1 > 14 || x1 >= 64 || x1 + w1 > 64) return -1;

    ENT_SXY(ep2, x2, y2);

    // for now assume everyone is the same height, so to collide they
    // must be on the same line
    if (y1 == y2)
    {
        uchar t = ENT_TYPE(ep2);
        w2 = objTable[t]._w;
        x2 -= (w2>>1);

        // collision!
        if (x2 + w2 > x1 && x2 < x1 + w1) 
            return t+1;
    }
    return 0;
}

uchar setSector(uchar* ep, uchar x, uchar y)
{
    // attempt to set the sector location of `ep' to (x,y)
    // if this results in collision or outside quadrant, restore
    // original location and return != 0 value.
    
    uchar** qp;

    // get old values
    int oldxy = *((int*)(ep + 1));

    if (y > 14) return -1; // hit edge
    
    // set new values
    ENT_SET_SXY(ep, x, y);

    // check for collision
    for (qp = quadrant; *qp; ++qp)
    {
        uchar c;
        if (ep != *qp && (c = collision(ep, *qp)))
        {
            // restore old values
            *(int*)(ep+1) = oldxy;
            return c;
        }
    }
    return 0; // ok
}

static void genEntLocation(uchar* ep, uchar type, uchar tmax)
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
        
    } while (quadCounts[type] >= tmax);

    ENT_SET_TYPE(ep, type);
    ENT_SET_QZ(ep, z);
    ENT_SET_QX(ep, x);
    ENT_SET_QY(ep, y);

    do
    {
        // put at a random location within the quadrant
        x = rand16() & 63;
        y = rand16() & 15;

    } while (setSector(ep, x, y));
}


void genGalaxy()
{
    uchar i;
        
    // XX should be zero anyway once we clear BSS
    memzero(galaxy, sizeof(galaxy));

    stardate = 20130; // 2014.0 - 10 (adjust for bogus warp to start)
    galaxyEnd = galaxy;

    // we are the first entity in the table
    genEntLocation(galaxyEnd, ENT_TYPE_FEDERATION, 1);

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
        genEntLocation(galaxyEnd, ENT_TYPE_BASE, 1);
        galaxyEnd += ENT_SIZE; 
    }

    // generate starfleet HQ
    genEntLocation(galaxyEnd, ENT_TYPE_BASE, 1);
    
    // change location to be our quadrant
    ENT_SET_QX(galaxyEnd, QX);
    ENT_SET_QY(galaxyEnd, QY);
    ENT_SET_QZ(galaxyEnd, QZ);

    galaxyEnd += ENT_SIZE; 

    // populate klingons
    for (i = 0; i < 30; ++i)
    {
        genEntLocation(galaxyEnd, ENT_TYPE_KLINGON, 3);
        
        // enemy has at least half its allowed energy
        ENT_SET_DAT(galaxyEnd, (rand16() & (ENT_ENERGYK_LIMIT/4-1)) + ENT_ENERGYK_LIMIT/2);
        galaxyEnd += ENT_SIZE;
    }

    // populate planets & stars
    for (i = 0; i < 100; ++i)
    {
        genEntLocation(galaxyEnd, ENT_TYPE_STAR, 4);
        galaxyEnd += ENT_SIZE;
        genEntLocation(galaxyEnd, ENT_TYPE_PLANET, 4);
        galaxyEnd += ENT_SIZE;
    }

    warp(QX, QY, QZ);
}




