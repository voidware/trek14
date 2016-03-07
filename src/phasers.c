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
#include "sound.h"

static void setPixel(char x, char y)
{
    plot(x, y, 1);
}

static void fillBgPixel(char x, char y)
{
    // restore background at this pixel
    fillbg(x>>1, div3tab[y]);
}

static uchar* torpCollide(uchar* ep, uchar x, uchar y)
{
    // do we collide with anything at (x,y) other than `ep'
    uchar** qp;
    uchar torp[ENT_SIZE];

    // make up a bogus entity for the torpedo
    ENT_SET_TYPE(torp, ENT_TYPE_TORPEDO);
    
    // with the sector coordinates of the current point
    ENT_SET_SXY(torp, x>>1, div3tab[y]);
    
    // search current quadrant table for collision with this fake entity
    for (qp = quadrant; *qp; ++qp)
    {
        if (ep != *qp && collision(torp, *qp) > 0)
            return *qp;
    }  
    return 0;
}


static uchar* trackPoint(uchar* ep,
                         uchar sx, uchar sy,
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

    if (s < 0) s = -s;
    if (c < 0) c = -c;
    
    while ((char)sx >= 0 && sy > 2 && sy < 45)
    {
        uchar* hit;
        
        if (plot)
        {
            uchar i;
            for (i = 0; i < 10; ++i)
                (*plot)(sx, sy);
        }

        hit = torpCollide(ep, sx, sy);

        if (unplot)
        {
            (*unplot)(sx, sy);
            drawEnt(ep);
        }
        
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

static void corner(uchar* ep, int dir, uchar* x, uchar* y)
{
    // find corner of ship to launch from

    uchar sx, sy;
    char w;

    ENT_SXY(ep, sx, sy);
    sy *= 3;
    ++sy;
    w = getWidth(ep);
    sx -= (w>>1) + 1;
    ++w;
    if (dir <= 170)
    {
        sx += w;
    }
    else if (dir > 180)
    {
        ++sy;
        if (dir > 190) sx += w;
    }
    *x = sx<<1;
    *y = sy;
}


uchar phasers(uchar* ep, unsigned int e, uchar type)
{
    // entity `ep' fire phasers at `type' energy `e'
    
    uchar** qp;
    uchar c = 0;

    for (qp = quadrant; *qp; ++qp)
    {
        if (ENT_TYPE(*qp) == type)
        {
            uchar x, y;
            uchar ex, ey;
            uchar* hit;

            corner(ep, 0, &x, &y);

            ENT_SXY(*qp, ex, ey);
            
            // convert to pixel and subtract to form dx & dy
            ex = ex*2 - x;
            ey = y - (ey*3 + 1);
            
            hit = trackPoint(ep, x, y, (char)ey, (char)ex, 0, 0);

            if (hit == *qp)
            {
                unsigned int dam;

                // calculate effective damage if we hit
                // damage = E*exp(-dist/32)
                
                // dist*4 = dist*128/32 (work fixed point 128)
                dam = ((unsigned int)distance(ep, hit))<<2;
                
                // need to shift exp(exp) down 7 (fixed 128)
                // but since e is 14 bits, shift up 2 to get the most
                // accuracy.
                dam = (e*4)/(expfixed(dam)>>5);

                // only fire if damage impact is at least half energy
                // otherwise enemy too far.
                
                // do we have enough energy, if so subtract it.
                if (dam >= (e>>1) && enoughEnergy(ep, e))
                {
                    trackPoint(ep, x, y, (char)ey, (char)ex, setPixel, 0);

                    if (ep == galaxy)  // enemy fire at you sound
                        blastsound(255);
                    else
                        zapsound();  // your phaser sound

                    trackPoint(ep, x, y, (char)ey, (char)ex, 0, fillBgPixel);

                    ++c;
                    
                    // true if still there
                    if (hitEnergy(hit, dam))
                    {
                        // re-draw enemy
                        drawEnt(hit);
                    }
                    else 
                    {
                        // destroyed, so backup one slot
                        --qp;
                    }
                }
            }
        }
    }

    // return the number of phasers fired
    return c;
}


void torps(uchar* ep, int dir)
{
    // entity `ep' fire one torpedo in `dir'ection

    short s, c;
    uchar* hit;
    uchar x, y;

    corner(ep, dir, &x, &y);
    
    // convert direction to [-180, +180]
    if (dir > 180) dir -= 360;

    // tan(dir degrees)
    tanfxDeg(dir, &s, &c);
    
    // adjust for pixel aspect ratio
    s /= 2;

    // figure out what we hit without drawing anything
    hit = trackPoint(ep, x, y, s, c, 0, 0);

    if (hit)
    {
        if (ENT_TYPE(hit) == ENT_TYPE_KLINGON)
        {
            uchar sx, sy;
            char dx, dy;
            
            ENT_SXY(ep, sx, sy);
            
            // dodge?
            moveAway(hit, sx, sy, &dx, &dy);
            
            // can expire here
            moveEnt(hit, dx, dy);
        }
    }

    // bogus sound
    squoink();

    hit = trackPoint(ep, x, y, s, c, setPixel, fillBgPixel);
    if (hit)
    {
        if (hitEnergy(hit, 4000)) drawEnt(hit);        
    }
}



