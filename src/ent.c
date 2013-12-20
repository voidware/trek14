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
//#include "os.h"
#include "libc.h"

uchar galaxy[ENT_COUNT_MAX*ENT_SIZE];
uchar quadrants[192*3];

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
    // pick a random location, ensuring type `type' does not
    // exceed `tmax'

    uchar x, y, z;
    uchar* c;
    
    do
    {
        // generate quadrant
        // call rand separately for each part because the lower bits
        // are more random.

        x = rand16() & 0x7;
        y = rand16() & 0x7;
        do z = rand16() & 0x3; while (z == 3);
        c = QUAD(x,y,z) + type/2; // class is type/2

    } while (*c >= tmax);

    // one more now.
    ++*c;

    ENT_SET_TYPE(ep, type);
    ENT_SET_QZ(ep, z);
    ENT_SET_QX(ep, x);
    ENT_SET_QY(ep, y);

    // put at a random location within the quadrant
    // NB: no check for collisions!
    ENT_SET_SXY(ep, rand16() & 64, rand16() & 15);
}


void genGalaxy()
{
    uchar i;
    uchar* ep;
        
    // XX should be zero anyway once we clear BSS
    memset(galaxy, 0, sizeof(galaxy));
    memset(quadrants, 0, sizeof(quadrants));

    ep = galaxy;

    printf("enemies\n");

    // populate klingons
    for (i = 0; i < 100; ++i)
    {
        genEntLocation(ep, ENT_TYPE_KLINGON, 3);
        
        // enemy has at least half its allowed energy
        ENT_SET_DAT(ep, (rand16() & (ENT_ENERGYK_LIMIT/4-1)) + ENT_ENERGYK_LIMIT/2);
        ep += 5;
    }

    printf("stars\n");

    // populate planets & stars
    for (i = 0; i < 100; ++i)
    {
        genEntLocation(ep, ENT_TYPE_STAR, 4);
        ep += 5;
        genEntLocation(ep, ENT_TYPE_PLANET, 4);
        ep += 5;
    }
}



