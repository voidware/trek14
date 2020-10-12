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
#include "ent.h"
#include "utils.h"
#include "warp.h"
#include "damage.h"
#include "command.h"


uchar galaxy[ENT_COUNT_MAX*ENT_SIZE];
uchar* galaxyEnd;
unsigned int stardate;
int score;
int scoremax;
const char entTypeChar[] = { 'B', 'F', 'S', 'P', 'M', 'K', 'K', 'D', 'R', 0 };

// current location
uchar QX, QY, QZ;

// current alert level (0 = normal, red alert)
uchar alertLevel;

// need to redraw screen 
uchar redrawsr;
uchar gameover;
uchar recalled;

// entities in current quadrant
uchar quadCounts[ENT_TYPE_COUNT];
uchar* quadrant[ENT_QUAD_MAX];

// RLE sprites
static const uchar base[] = { 0x33, 0x00, 0x01,
                              0x01, 0x00, 0x06,
                              0x0c, 0x00,
                              0x00 };

const uchar fedship[] = { 0x06, 0, 0x02,
                          0x01, 0x47, 0, 0x0d,
                          0x0b, 0,
                          0x00 };

static const uchar star[] = { 0x02, 0x22, 0, 6,
                              0x14, 0x10, 0, 6,
                              0x02, 0x22, 0,
                              0x00 };

static const uchar staralt[] = { 0x02, 0x22, 0, 6,
                                 0x22, 0x20, 0, 6,
                                 0x02, 0x22, 0,
                                 0x00 };

static const uchar planet[] = { 0x15, 0, 0x06,
                                0x07, 0, 0x06,
                                0x05, 0,
                                0x00  };

static const uchar planetm[] = { 0x12, 0x12, 0, 0x06,
                                0x07, 0, 0x06,
                                0x05, 0,
                                0x00  };

// basic klingon
static const uchar klingon[] = { 
    0x02, 0x72, 0, 0x0b,
    0x0b, 0, 0x07,
    0x03, 0,
    0x00
};

static const uchar klingonAlt[] = {
    0x01, 0x91, 0, 0x0b,
    0x0b, 0, 0x07,
    0x03, 0,
    0x00
};

// bigger klingon
static const uchar klingon2[] = {   // 15 pixel version
    0x03, 0x11, 0x51, 0x13, 0,
    0x0f,
    0x0f, 0,
    0x0a,
    0x05, 0,
    00
};

static const uchar klingon2Alt[] = { 
    0x03, 0x93,0,
    0x0f,
    0x0f, 0,
    0x0a,
    0x05, 0,
    00
};


#if 0
// bigger still
static const uchar klingonbs[] = {   // 17 pixel version
    0x03, 0x11, 0x71, 0x13, 0,
    0x11,
    0x0f, 0x02, 0,
    0x0c,
    0x07, 0,
    00
};
#endif



#if 0
static const uchar klingonbs[] = {   
    0x03, 0x11, 0x91, 0x13, 0,  // like a face
    0x13,
    0x0f, 0x04, 0,
    0x13,
    0x05, 0x11, 0x11, 0x11, 0x11, 0x15, 0,
    0x10,
    0x0d, 0,
    00
};
#endif


static const uchar klingon_destroyer[] = {   
    0x03, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x13, 0,
    0x13,
    0x05, 0x95, 0,
    //0x0f, 0x04, 0,
    0x13,
    0x0f, 0x04, 0,
    //0x05, 0x11, 0x11, 0x11, 0x11, 0x15, 0,
    0x0d,
    0x07, 0,
    00
};

static const uchar romulan[] = {  // birdshape
                                0x03, 0x31, 0x33, 0,
                                0x0c,
                                0x0b, 0,
                                0x08,
                                0x02, 0x12, 0,
                                00
};

#if 0
static const uchar romulan2[] = { // heavy birdshape
    0x01, 0x11, 0x11, 0x51, 0x11, 0x11, 0, 0x0f,
                                0x0f, 0, 0x0a,
                                0x05, 0,
                                00
};
#endif


// pixel to char width
#define CW(_v) (((_v)+1)>>1)
                              
const EntObj objTable[] =
{
    // high negative scores will ensure the game ends if you
    // destroy them
    { CW(12), 1, -1000, 0, base },
    { CW(16), 1, -1000, 8000, fedship }, 
    { CW(6), 1, -1000, STAR_ENERGY, star, staralt, 2 },  // recharge energy
    { CW(7), 1, -1000, PLANET_ENERGY, planet }, // planet G
    { CW(7), 1, -1000, PLANET_ENERGY_M, planetm }, // PLANET_M
    { CW(11), 1, SCORE_KLINGON, KLINGON_ENERGY, klingon, klingonAlt },
    { CW(15), 1, SCORE_KLINGON*2, KLINGON2_ENERGY, klingon2, klingon2Alt },
    { CW(19), 2, SCORE_KLINGON*4, KLINGOND_ENERGY, klingon_destroyer },
    { CW(0), 1, SCORE_KLINGON, 2000, romulan },
    { 1, 1, 0, 0, romulan }, // additional entry used for torpedo
};

uchar mainType(uchar* ep)
{
    // get type, but map all klingons onto same type
    uchar t = ENT_TYPE(ep);
    if (t > ENT_TYPE_KLINGON && t <= ENT_TYPE_KLINGON_DESTROYER)
        t = ENT_TYPE_KLINGON;
    if (t == ENT_TYPE_PLANET_M) t = ENT_TYPE_PLANET;
    return t;
}

void getQuad(uchar x, uchar y, uchar z, uchar* quadCounts, uchar** eplist)
{
    // get information about quadrant (x,y,z)
    // quadCounts is an array of counts indexed by ent type
    // eplist (optional) is a list of ent pointers, objects in this quadrant
    
    uchar* ep = galaxy;
    memset(quadCounts, 0, ENT_TYPE_COUNT);
    
    while (ep != galaxyEnd)
    {
        if (ENT_QX(ep) == x && ENT_QY(ep) == y && ENT_QZ(ep) == z)
        {
            // now seen if not already, increase explore score
            if (!ENT_MARKED(ep))
            {
                ENT_SET_MARK(ep);
                score += SCORE_EXPLORE;
            }

            if (eplist)
            {
                *eplist++ = ep;
                *eplist = 0;
            }

            ++quadCounts[mainType(ep)];
        }
        ep += ENT_SIZE;
    }
}

uchar distance(uchar* ep1, uchar* ep2)
{
    // approx Euclidean distance between two entities in the same sector
    uchar x1, y1;
    uchar x2, y2;
    ENT_SXY(ep1, x1, y1);
    ENT_SXY(ep2, x2, y2);

    // use hypot approx |a| + |b| - min(|a|,|b|)/2 
    if ((char)(x2 -= x1) < 0) x2 = -x2;
    if ((char)(y2 -= y1) < 0) y2 = -y2;
    return x2 + y2 - (((x2>y2) ? y2 : x2) >> 1);
}

uchar distm(char x1, char y1, char x2, char y2)
{
    // Manhattan distance
    char dx = x1 - x2;
    char dy = y1 - y2;
    if (dx < 0) dx = -dx;
    if (dy < 0) dy = -dy;
    return dx + dy;
}

uchar distmTo(uchar x, uchar y, uchar z)
{
    return ABSC(QX - x) + ABSC(QY - y) + ABSC(QZ - z);
}

uchar getWidth(uchar* ep)
{
    // get width in characters
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

uchar* adjacentTo(uchar* ep, uchar type)
{
    uchar** epp;
    for (epp = quadrant; *epp; ++epp)
    {
        if (ENT_TYPE(*epp) == type)
        {
            // hit or adjacent
            if (collision(ep, *epp)) return *epp;

        }
    }
    return 0;
}

char collision(uchar* ep1, uchar* ep2)
{
    // if object `ep1' overlaps or touches `ep2'
    // 
    // return >0 if overlaps
    // return <0 if touches
    // return otherwise

    char w1, x1, y1;
    char w2, x2, y2;
    char d1, d2;

    ENT_SXY(ep1, x1, y1);
    w1 = getWidth(ep1);
    x1 -= w1>>1; 

    ENT_SXY(ep2, x2, y2);
    w2 = getWidth(ep2);
    x2 -= w2>>1;

    d1 = x2 + w2 - x1;
    d2 = x1 + w1 - x2;

    y1 -= y2;
    if (y1 < 0) y1 = -y1;
    
    // for now assume everyone is the same height, so to collide they
    // must be on the same line
    if (!y1)
    {
        if (d1 > 0 && d2 > 0) return 1; // overlap => collision
        if (!d1 || !d2) return -1; // touching
        //if (x2 + w2 > x1 && x2 < x1 + w1) return ENT_TYPE(ep2) + 1;
    }
    else if (y1 == 1)
    {
        // overlap on X one line out is touching
        if (d1 > 0 && d2 > 0) return -1; 
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
            if (ep != *qp && collision(ep, *qp) > 0)
            {
                c = 1;
                break;
            }
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
        // and boundary overlap.
        x = rand16() & 63;
        y = rand16() & 15;
    } while (setSector(ep, x, y, 0));
}

static uchar genEntLocation(uchar* ep, uchar type, uchar n)
{
    // pick a random location, with none of the given type
    // create [1,n].
    // avoiding quadrant QX, QY, QZ

    uchar x, y, z;
    uchar i;
    
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
        
    } while (quadCounts[type]);

    n = randn(n) + 1;

    for (i = 0; i < n; ++i)
    {
        ENT_SET_TYPE(ep, type);
        ENT_SET_QX(ep, x);
        ENT_SET_QZ(ep, z);
        ENT_SET_QY(ep, y);
        genSector(ep);
        ep += ENT_SIZE;
    }
    return n;
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
    // return non-zero if ok.

    int e = ENT_ENERGY(ep) - d;
    if (e > 0)
    {
        ENT_SET_ENERGY(ep, e);
        return 1;
    }
    return 0;
}

uchar takeEnergy(uchar* ep, unsigned int d)
{
    // return 0 if `ep' expires.

    uchar u = enoughEnergy(ep, d);
    if (!u)
    {
        // blow up!
        if (ep != galaxy) 
        {
            // enemy ran out of energy and disappears
            removeEnt(ep);
        }
        else
            endgame(MSG_CODE_ENDGAME_EXPIRE);
    }
    return u;
}

void removeEnt(uchar *ep)
{
    // adjust score
    uchar t = ENT_TYPE(ep);
    score += objTable[t]._score;
    if (score < 0)
    {
        // attacked a planet etc.
        endgame(t == ENT_TYPE_BASE ? 
                MSG_CODE_COURT_MARTIAL : MSG_CODE_INCOMPETENCE);
    }
    else
    {
        galaxyEnd -= ENT_SIZE;
        memmove(ep, ep + ENT_SIZE, galaxyEnd - ep);

        // rebuild quadrant content
        updateQuadrant();
    }
}

static int randomEnergy(uchar t)
{
    uint e = objTable[t]._emax>>1; // half max
    return randn(e) + e;
}

static void genObject(uchar num, uchar type, uchar tmax)
{
    uchar i = 0;
    while (i < num)
    {
        uchar n = genEntLocation(galaxyEnd, type, tmax);
        i += n;

        while (n--)
        {
            ENT_SET_DAT(galaxyEnd, randomEnergy(type));
            galaxyEnd += ENT_SIZE;
        }
    }
}

void genGalaxy()
{
    uchar i;
    
    // (adjust for bogus warp to start)
    stardate = STARDATE_START - STARDATE_WARP; 

    galaxyEnd = galaxy;

    // we are the first entity in the table
    genEntLocation(galaxyEnd, ENT_TYPE_FEDERATION, 1);

    // full energy & photons
    ENT_SET_DAT(galaxyEnd, ENT_REFUEL_DATA);

    galaxyEnd += ENT_SIZE; 

    // our current location. setting this will prevent anything else from 
    // being put in this quadrant
    QX = 7;
    QY = 7;
    QZ = 2;

    // populate bases
    for (i = 0; i < TOTAL_BASES; ++i)
    {
        genEntLocation(galaxyEnd, ENT_TYPE_BASE, 1);
        galaxyEnd += ENT_SIZE; 
    }

    // last one is starfleet HQ
    // put HQ in our start quadrant (ok to move since nothing else there)
    setQuadrant(galaxyEnd - ENT_SIZE, QX, QY, QZ);

    scoremax = TOTAL_BASES + TOTAL_STARS + TOTAL_PLANETS + TOTAL_PLANETS_M*SCORE_PLANET_M;
    
    // populate klingons
    for (i = 0; i < TOTAL_KLINGONS;)
    {
        uchar k2 = 0;
        uchar n;

        // up to 4 klingons per quadrant.
        n = genEntLocation(galaxyEnd, ENT_TYPE_KLINGON, 4);
        i += n;

        while (n--)
        {
            scoremax += SCORE_KLINGON;
            uint e = randomEnergy(k2 ? ENT_TYPE_KLINGON : ENT_TYPE_KLINGON2);
            if (e > KLINGON_ENERGY)
            {
                // some are bigger
                ENT_SET_TYPE(galaxyEnd, ENT_TYPE_KLINGON2);
                ++k2;
                scoremax += SCORE_KLINGON;  // score twice
            }
            ENT_SET_DAT(galaxyEnd, e);
            galaxyEnd += ENT_SIZE;
        }
    }

    // populate stars
    // give stars a random energy between 128 and 256
    // this energy can be drawn by enemies to recharge
    genObject(TOTAL_STARS, ENT_TYPE_STAR, 2);

    // populate planets
    genObject(TOTAL_PLANETS, ENT_TYPE_PLANET, 3);
    genObject(TOTAL_PLANETS_M, ENT_TYPE_PLANET_M, 1);

#if 0
    // fill up with planets. useful for testing
    while (galaxyEnd < galaxy + sizeof(galaxy))
    {
        genEntLocation(galaxyEnd, ENT_TYPE_PLANET, 5);
        galaxyEnd += ENT_SIZE;
    }
#endif

    clearMarks();
    
    // start with everything operational
    repairAll();

    // +1 for starfleet HQ, not to count
    score = -1;

    // reset (eg new game)
    alertLevel = 0;
    recalled = 0;

    // warp to QX, QY, QZ
    warp(); 
}


