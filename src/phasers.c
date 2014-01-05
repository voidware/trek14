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
#include "plot.h"
#include "srscan.h"
#include "enemy.h"
#include "phasers.h"

static void setPixel(char x, char y)
{
    plot(x, y, 1);
}

static void fillBgPixel(char x, char y)
{
    // restore background at this pixel
    fillbg(x>>1, div3tab[y]);
}

static uchar* torpCollide(uchar x, uchar y)
{
    uchar** qp;
    uchar torp[ENT_SIZE];

    // make up a bogus entity for the torpedo
    ENT_SET_TYPE(torp, ENT_TYPE_TORPEDO);
    
    // with the sector coordinates of the current point
    ENT_SET_SXY(torp, x>>1, div3tab[y]);
    
    // search current quadrant table for collision with this fake entity
    for (qp = quadrant; *qp; ++qp)
    {
        if (collision(torp, *qp) > 0)
            return *qp;
    }  
    return 0;
}


static uchar* trackPoint(uchar sx, uchar sy,
                         int s, int c,
                         plotfn* plot,
                         plotfn* unplot)
                  
{
    // nb: s > 0 means y decreases (goes up on screen).
    unsigned int t;
    char dx, dy;

    t = 0;
    dy = SIGN(s);
    dx = SIGN(c);

    s = ABS(s);
    c = ABS(c);
    
    while ((char)sx >= 0 && sy > 2 && sy < 45)
    {
        uchar* hit;
        
        if (plot)
        {
            int i;
            for (i = 0; i < 10; ++i)
                (*plot)(sx, sy);
        }

        hit = torpCollide(sx, sy);

        if (unplot) (*unplot)(sx, sy);
        
        if (hit) return hit;

        if (s <= c)
        {
            sx += dx;
            
            t += s;
            if (t >= c)
            {
                t -= c;
                sy -= dy;
            }
        }
        else
        {
            sy -= dy;
            
            t += c;
            if (t >= s)
            {
                t -= s;
                sx += dx;
            }
        }
    }
    return 0;
}

void phasers(uchar* ep, unsigned int e, uchar type)
{
    // entity `ep' fire phasers at `type' energy `e'
    
    uchar** qp;

    for (qp = quadrant; *qp; ++qp)
    {
        if (ENT_TYPE(*qp) == type)
        {
            uchar sx, sy;
            uchar ex, ey;
            char dw;
            unsigned int dam;
            uchar* hit;

            ENT_SXY(ep, sx, sy);
            ENT_SXY(*qp, ex, ey);

            dw = objTable[ENT_TYPE(ep)]._w>>1;
            if (sx <= ex) sx += dw;  // front the front
            else sx -= dw + 1; // from the back (+1 for even)

            // convert to pixels
            sx <<= 1;
            sy = sy*3+1; // take mid point

            // convert to pixel and subtract to form dx & dy
            ex = ex*2 - sx;
            ey = sy - (ey*3 + 1);

            hit = trackPoint(sx, sy, (char)ey, (char)ex, 0, 0);

            if (hit == *qp)
            {
                // do we have enough energy, if so subtract it.
                if (enoughEnergy(ep, e))
                {
                    trackPoint(sx, sy, (char)ey, (char)ex, setPixel, 0);
                    trackPoint(sx, sy, (char)ey, (char)ex, 0, fillBgPixel);

                    // calculate effective damage
                    // damage = E*exp(-dist/32)
                
                    // dist*4 = dist*128/32 (work fixed point 128)
                    dam = ((unsigned int)distance(ep, hit))<<2;
                
                    // need to shift exp(exp) down 7 (fixed 128)
                    // but since e is 14 bits, shift up 2 to get the most
                    // accuracy.
                    dam = (e*4)/(expfixed(dam)>>5);

                    // true if still there
                    if (takeEnergy(hit, dam))
                        // re-draw enemy
                        drawEnt(hit);
                    else 
                    {
                        // destroyed, so backup one slot
                        --qp;
                    }
                }
            }
        }
    }
}



void torps(uchar* ep, int dir)
{
    // entity `ep' fire one torpedo in `dir'ection

    uchar sx, sy;
    char dw;
    short s, c;
    uchar* hit;
    
    // convert direction to [-180, +180]
    if (dir > 180) dir -= 360;

    // tan(dir degrees)
    tanfxDeg(dir, &s, &c);

    ENT_SXY(ep, sx, sy);
    
    dw = objTable[ENT_TYPE(ep)]._w>>1;
    if (ABS(dir) <= 90) sx += dw;  // front the front
    else sx -= dw + 1; // from the back (+1 for even width)

    // convert to pixel position and track 
    hit = trackPoint(sx<<1, sy*3+1, s, c, setPixel, fillBgPixel);
    if (hit)
    {
        if (takeEnergy(hit, 5000))
            drawEnt(hit);        
    }
}



