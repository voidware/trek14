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
#include "lrscan.h"
#include "warp.h"

uchar galaxy[ENT_COUNT_MAX*ENT_SIZE];
uchar* galaxyEnd;
uchar QX, QY, QZ;
const char entTypeChar[] = { 'B', 'F', 'S', 'P', 'K', 'R', 0 };


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

                              
const EntObj objTable[] =
{
    { 12, 3, base },
    { 16, 3, fedship },
    { 6, 3, star },
    { 7, 3, planet },
    { 11, 3, klingon },
    { 0, 0, romulan },
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


static void genEntLocation(uchar* ep, uchar type, uchar tmax)
{
    // pick a random location, ensuring type' does not
    // exceed `tmax'
    // avoiding quadrant QX, QY, QZ

    uchar x, y, z;
    uchar quad[ENT_TYPE_COUNT];
    uchar w, wl;
    
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

        lrScanQuad(x, y, z, quad);
        
    } while (quad[type] >= tmax);

    ENT_SET_TYPE(ep, type);
    ENT_SET_QZ(ep, z);
    ENT_SET_QX(ep, x);
    ENT_SET_QY(ep, y);

    // put at a random location within the quadrant
    // NB: no check for collisions!
    x = rand16() & 63;

    do
    {
        y = rand16() & 15;
    } while (y == 0 || y == 15);
    
    w = (objTable[type]._w + 1)>>1; // character cells wide

    // left width (round down)
    wl = w>>1;

    if (x < wl) x = wl;
    else if (x + (w - wl) > 64) x = 64 - w + wl;

    ENT_SET_SXY(ep, x, y);
}


void genGalaxy()
{
    uchar i;
    uchar quad[ENT_TYPE_COUNT];
        
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




