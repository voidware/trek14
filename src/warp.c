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
#include "os.h"
#include "libc.h"
#include "utils.h"
#include "ent.h"
#include "command.h"
#include "warp.h"

char canwarp(uchar x, uchar y, uchar z)
{
    // can we warp to (x,y,z) ?
    // if we can, subtract energy needed.

    char v = (x < 8 && y < 8 && z <= 2);
    
    if (v)
    {
        // warp costs the "manhattan distance"
        uchar d = ABSC(QX - x) + ABSC(QY - y) + ABSC(QZ - z);
        
        // we are the first entry in the galaxy
        if (!enoughEnergy(galaxy, ((int)d)*100))
        {
            messageCode(MSG_CODE_INSUFENERGY);
            v = 0;
        }
    }
    return v;
}

void warp()
{
    // warp to QX, QY, QZ
    // ASSUME we can warp and energy needed is already taken

    // we are the first entry in the galaxy
    setQuadrant(galaxy, QX, QY, QZ);
        
    // takes time to warp (adjusted by 1 for this move)
    stardate += STARDATE_WARP - 1;
    
    // set position in quadrant without collision
    genSector(galaxy);
}
