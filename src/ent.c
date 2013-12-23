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
const char entTypeChar[] = { 'B', 'F', 'S', 'P', 'K', 'R', 0 };

// current location
uchar QX, QY, QZ;

// entities in current quadrant
uchar* quadrant[ENT_QUAD_MAX];


// RLE sprites
const uchar fedship[] = { 0x01, 0x60, 0x02,
                                 0x02, 0x14, 0x70, 0x0d,
                                 0x01, 0xb0, 0x00,
                                 0x00 };

static const uchar base[] = { 0x01, 0x30, 0x01,
                              0x01, 0x10, 0x06,
                              0x01, 0xc0, 0x00,
                              0x00 };

static const uchar star[] = { 0x02, 0x22, 0x20, 0x05,
                              0x01, 0x40, 0x05,
                              0x02, 0x22, 0x20, 0x00,
                              0x00 };

static const uchar planet[] = { 0x01, 0x50, 0x06,
                                0x01, 0x70, 0x06,
                                0x01, 0x50, 0x00,
                                0x00  };

static const uchar klingon[] = { 0x02, 0x27, 0x20, 0x0b,
                                 0x01, 0xb0, 0x07,
                                 0x01, 0x30, 0x00,
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
    { CW(16), 3, fedship },
    { CW(6), 3, star },
    { CW(7), 3, planet },
    { CW(11), 3, klingon },
    { CW(0), 0, romulan },
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


static int collision(uchar* ep1, uchar* ep2)
{
    // if object `ep1' overlaps `ep2'
    // or if `ep1' does not git properly in the quadrant

    uchar w1, x1, y1;
    uchar w2, x2, y2;

    w1 = objTable[ENT_TYPE(ep1)]._w;
    ENT_SXY(ep1, x1, y1);

    x1 -= (w1>>1); 

    // overlap quadrant edge
    if (!y1 || y1 == 15 || x1 >= 64 || x1 + w1 > 64) return 1;

    ENT_SXY(ep2, x2, y2);

    // for now assume everyone is the same height, so to collide they
    // must be on the same line
    if (y1 == y2)
    {
        w2 = objTable[ENT_TYPE(ep2)]._w;
        x2 -= (w2>>1);

        // collision!
        if (x2 + w2 > x1 && x2 < x1 + w1) 
            return 2;

    }
    
    return 0;
}

static void genEntLocation(uchar* ep, uchar type, uchar tmax)
{
    // pick a random location, ensuring type' does not
    // exceed `tmax'
    // avoiding quadrant QX, QY, QZ

    uchar x, y, z;
    uchar quadCounts[ENT_TYPE_COUNT];
    
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

    for (;;)
    {
        uchar** qp;

    retry:

        qp = quadrant;
        
        // put at a random location within the quadrant
        x = rand16() & 63;
        y = rand16() & 15;
        ENT_SET_SXY(ep, x, y);

        // check for collision
        while (*qp)
        {
            if (collision(ep, *qp)) goto retry;
            ++qp;
        }

        break;
    }
    
#if 0
    w = (objTable[type]._w + 1)>>1; // character cells wide

    // left width (round down)
    wl = w>>1;

    if (x < wl) x = wl;
    else if (x + (w - wl) > 64) x = 64 - w + wl;
#endif

}


void genGalaxy()
{
    uchar i;
        
    // XX should be zero anyway once we clear BSS
    memzero(galaxy, sizeof(galaxy));

    galaxyEnd = galaxy;

    // our current location. set void
    QX = -1;
    QY = -1;
    QZ = -1;

    printf("enemies\n");

    // populate klingons
    for (i = 0; i < 20; ++i)
    {
        genEntLocation(galaxyEnd, ENT_TYPE_KLINGON, 3);
        
        // enemy has at least half its allowed energy
        ENT_SET_DAT(galaxyEnd, (rand16() & (ENT_ENERGYK_LIMIT/4-1)) + ENT_ENERGYK_LIMIT/2);
        galaxyEnd += ENT_SIZE;
    }

    printf("stars\n");

    // populate planets & stars
    for (i = 0; i < 100; ++i)
    {
        genEntLocation(galaxyEnd, ENT_TYPE_STAR, 4);
        galaxyEnd += ENT_SIZE;
        genEntLocation(galaxyEnd, ENT_TYPE_PLANET, 4);
        galaxyEnd += ENT_SIZE;
    }

    printf("federation\n");


    // our current location. setting this with prevent bases from 
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
    
    // put us in the same location
    genEntLocation(galaxyEnd, ENT_TYPE_FEDERATION, 1);
    galaxyEnd += ENT_SIZE; 

    warp(QX, QY, QZ);
}




