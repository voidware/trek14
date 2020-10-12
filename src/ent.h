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

/* Galactic entities!
 *
 * An entity and its data are stored in 5 bytes as follows. The macros below
 * perform the actual bit banging. The layout is assumed only through the 
 * macros.

 Byte1:
 +---+---+---+---|---+---+---+---+
 | M | D | Z | Z | T | Y | P | E |
 +---+---+---+---|---+---+---+---+

 M = mark bit (explored)
 D = docked bit (orbit)
 Z = quadrant Z coordinate (0-3)
 TYPE = entity type


 Bytes 2&3
 +---+---+---+---|---+---+---+---+  +---+---+---+---|---+---+---+---+
 | x | x | x | x | x | X | X | X |  | y | y | y | y | x | Y | Y | Y |
 +---+---+---+---|---+---+---+---+  +---+---+---+---|---+---+---+---+

 X = quadrant X coordinate (0-7)
 Y = quadrant Y coordinate (0-7)
 x = sector x (0-63)
 y = sector y (0-15)

 The sector coordate is the screen character position for the short range
 scan. This scheme allows all entities to retain both their global and 
 local coordinates so that entities can be at the same place when revisiting
 a quadrant.

 NB: the LSb of the sector-x is stored in byte 3

 Bytes 4 & 5
 +---+---+---+---|---+---+---+---+  +---+---+---+---|---+---+---+---+
 | t | t |   | E | E | E | E | E |  | E | E | E | E | E | E | E | E |
 +---+---+---+---|---+---+---+---+  +---+---+---+---|---+---+---+---+

 t = number of photon torpedoes (0-3)
 E = energy (0-8192)

*/

#define ENT_TYPE(_p)  (*(_p) & 0xf)
#define ENT_MARKED(_p) (*((char*)(_p)) < 0)

#define ENT_DOCKED(_p) (*(_p) & 0x40)
#define ENT_SET_DOCKED(_p)  *(_p) |= 0x40

// NB: used to initialise whole byte
#define ENT_SET_TYPE(_p, _v) *(_p) = (_v)
#define ENT_SET_MARK(_p)   *(_p) |= 0x80
#define ENT_CLEAR_MARK(_p)  *(_p) &= 0x7f

#define ENT_QX(_p) ((_p)[1] & 7)
#define ENT_QY(_p) ((_p)[2] & 7)
#define ENT_QZ(_p) (((*(_p)) >> 4) & 0x3)

#define ENT_SET_QZ(_p, _v)  *(_p) = ((*(_p) & ~0x30) | ((_v)<<4))
#define ENT_SET_QX(_p, _v) (_p)[1] = (((_p)[1] & ~0x7) | (_v))
#define ENT_SET_QY(_p, _v) (_p)[2] = (((_p)[2] & ~0x7) | (_v))

// sx5, qx3, (sy4, sx1), qy3
#define ENT_SXY(_p, _x, _y)     \
{                               \
    _x = ((_p)[1] >> 2) & ~1;   \
    _y = (_p)[2] >> 4;          \
    if ((_p)[2] & (1<<3)) ++_x; \
}

#define ENT_SET_SXY(_p, _x, _y)                                 \
{                                                               \
    (_p)[1] = (((_x) & 0x3e) << 2) | ENT_QX(_p);                \
    (_p)[2] = ((((_y) << 1) | ((_x)&1)) << 3) | ENT_QY(_p);     \
}

// _d is 16 bits
#define ENT_DAT(_p) (*((uint16*)((uchar*)(_p) + 3)))
#define ENT_SET_DAT(_p, _d)  ENT_DAT(_p) = (_d)

// 13 bits
#define ENT_ENERGY_BITS 13
#define ENT_ENERGY_LIMIT (1<<ENT_ENERGY_BITS)

#define ENT_ENERGY(_p) (ENT_DAT(_p) & (ENT_ENERGY_LIMIT-1))

#define ENT_SET_ENERGY(_p, _v)                  \
{                                               \
    uint16* p = (uint16*)((_p) + 3);            \
    *p = (*p & ~(ENT_ENERGY_LIMIT-1)) | (_v);   \
}

// torps only applies to federation ships
#define ENT_TORPS(_p) (ENT_DAT(_p) >> ENT_ENERGY_BITS)
#define ENT_SET_TORPS(_p, _v)                                           \
{                                                                       \
    uint16* p = (uint16*)((_p) + 3);                                    \
    *p = (*p & (ENT_ENERGY_LIMIT-1)) | ((_v) << ENT_ENERGY_BITS);       \
}

// at 13 bits, refuel to 8000 rather than 8192.
#define ENT_REFUEL_DATA ((3<<ENT_ENERGY_BITS) + 8000)

#define KLINGON_ENERGY     4000
#define KLINGON2_ENERGY    5000
#define KLINGOND_ENERGY    8000

// planets do not actually have energy.
// this value is used as a "habitability" factor.
// high "energy" planets are more inhabitable.
#define PLANET_ENERGY  128
#define PLANET_ENERGY_M 256

#define STAR_ENERGY 256

#define ENT_SIZE  5
#define ENT_COUNT_MAX   300
#define ENT_QUAD_MAX    10

#define ENT_TYPE_BASE   0
#define ENT_TYPE_FEDERATION 1
#define ENT_TYPE_STAR 2
#define ENT_TYPE_PLANET  3
#define ENT_TYPE_PLANET_M  4
#define ENT_TYPE_KLINGON 5
#define ENT_TYPE_KLINGON2 6
#define ENT_TYPE_KLINGON_DESTROYER 7
#define ENT_TYPE_ROMULAN 8
#define ENT_TYPE_TORPEDO 9
#define ENT_TYPE_COUNT 10

// total of entities should not exceed ENT_COUNT_MAX
#define TOTAL_BASES     10
#define TOTAL_KLINGONS  50
#define TOTAL_PLANETS   95
#define TOTAL_PLANETS_M 5
#define TOTAL_STARS     100

typedef struct 
{
    // width in chars
    uchar               _w;

    // height in chars
    uchar               _h;
    
    int                 _score;

    // max energy
    int                 _emax;
    
    // main RLE sprite
    const uchar*        _sprite;

    // alternate, for animation if defined.
    const uchar*        _spriteAlt;

    uchar               _altRateDiv;
    
} EntObj;


#define SCORE_KLINGON  50

// every entity when seen gives a small score
#define SCORE_EXPLORE   1
#define SCORE_PLANET_M  99 // NB: +1 for seeing it at all =100

#define MASK_RIGHT  1
#define MASK_LEFT   2
#define MASK_TOP    4
#define MASK_BOT    8

#define STARDATE_START 20140
#define STARDATE_END 21000
#define STARDATE_GRACE 100
#define STARDATE_WARP 10

// -- data

extern uchar galaxy[];
extern uchar* quadrant[];
extern uchar quadCounts[];

extern uchar QX, QY, QZ;
extern uchar* galaxyEnd;
extern unsigned int stardate;
extern int score;
extern int scoremax;
extern uchar alertLevel;
extern uchar redrawsr;
extern uchar gameover;
extern uchar recalled;
extern const char entTypeChar[];
extern const EntObj objTable[];
extern const uchar fedship[];

// -- functions

void getQuad(uchar x, uchar y, uchar z, uchar* quad, uchar** eplist);
void genGalaxy();
void genSector(uchar* ep);
uchar mainType(uchar* ep);
char collision(uchar* ep1, uchar* ep2);
char collisionBorder(uchar* ep);
uchar setSector(uchar* ep, uchar x, uchar y, uchar cancross);
uchar setQuadrant(uchar* ep, uchar x, uchar y, uchar z);
void updateQuadrant();
uchar distance(uchar* ep1, uchar* ep2);
uchar getWidth(uchar* ep);
uchar enoughEnergy(uchar* ep, unsigned int d);
uchar takeEnergy(uchar* ep, unsigned int d);
void removeEnt(uchar *ep);
uchar* adjacentTo(uchar* ep, uchar type);
uchar distm(char x1, char y1, char x2, char y2);
uchar distmTo(uchar x, uchar y, uchar z);
