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

void phasers(uchar* ep, unsigned int e, uchar type)
{
    // entity `ep' fire phasers at `type' energy `e'
    
    uchar** qp;

    for (qp = quadrant; *qp; ++qp)
    {
        if (ENT_TYPE(*qp) == type)
        {
            // do we have enough energy, if so subtract it.
            if (enoughEnergy(ep, e))
            {
                uchar sx, sy;
                uchar ex, ey;
                char dw;
                unsigned int dam;

                ENT_SXY(ep, sx, sy);
                ENT_SXY(*qp, ex, ey);

                dw = objTable[ENT_TYPE(ep)]._w>>1;
                if (sx <= ex) sx += dw;  // front the front
                else sx -= dw; // from the back

                // convert to pixels
                sx <<= 1;
                sy = sy*3+1; // take mid point
                ex <<= 1; 
                ey = ey*3+1;

                // zap line over and over
                for (dw = 0; dw < 80; ++dw)
                {
                    plotLine(sx, sy, ex, ey, 
                             (dw & 1) ? fillBgPixel : setPixel);
                }

                // redraw attacker
                drawEnt(ep);

                // calculate effective damage
                // damage = E*exp(-dist/32)
                
                // dist*4 = dist*128/32 (work fixed point 128)
                dam = ((unsigned int)distance(ep, *qp))<<2;
                
                // need to shift exp(exp) down 7 (fixed 128)
                // but since e is 14 bits, shift up 2 to get the most
                // accuracy.
                dam = (e*4)/(expfixed(dam)>>5);

                // true if still there
                if (takeEnergy(*qp, dam))
                    // re-draw enemy
                    drawEnt(*qp);
                else 
                {
                    // destroyed, so backup one slot
                    --qp;
                }
            }
        }
    }
}

uchar* torpCollide(uchar x, uchar y)
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
        if (*qp != galaxy && collision(torp, *qp) > 0)
            return *qp;
    }  
    return 0;
}

void torps(uchar* ep, int dir)
{
    // entity `ep' fire one torpedo in `dir'ection

    uchar sx, sy;
    char dw;
    short s, c;
    short t;
    char dx, dy;
    
    // convert direction to [-180, +180]
    if (dir > 180) dir -= 360;

    // tan(dir degrees)
    tanfxDeg(dir, &s, &c);

    //printfat(0,15, "%d/%d ", s, c);

    ENT_SXY(ep, sx, sy);
    
    dw = (objTable[ENT_TYPE(ep)]._w>>1);
    if (ABS(dir) <= 90) sx += dw;  // front the front
    else sx -= dw; // from the back

    // convert to pixels
    sx <<= 1;
    sy = sy*3+1; // take mid point

    t = 0;
    dy = SIGN(s);
    dx = SIGN(c);

    s = ABS(s);
    c = ABS(c);
    
    while ((char)sx >= 0 && sy > 2 && sy < 45)
    {
        uchar i;
        uchar* hit;

        for (i = 0; i < 10; ++i) plot(sx, sy, 1);

        hit = torpCollide(sx, sy);
        
        fillBgPixel(sx, sy);

        if (hit)
        {
            if (takeEnergy(hit, 5000)) drawEnt(hit);
            break;
        }

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
}



