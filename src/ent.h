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
 * perform the actual bit banging. The layout is only assumed through the 
 * macros.

 Byte1:
 +---+---+---+---|---+---+---+---+
 | M |   | Z | Z | T | Y | P | E |
 +---+---+---+---|---+---+---+---+

 M = mark bit
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

 Bytes 4 & 5 (format for federation ships)
 +---+---+---+---|---+---+---+---+  +---+---+---+---|---+---+---+---+
 | t | t | E | E | E | E | E | E |  | E | E | E | E | E | E | E | E |
 +---+---+---+---|---+---+---+---+  +---+---+---+---|---+---+---+---+

 t = number of photon torpedoes (0-3)
 E = energy (0-16383)

*/

#define ENT_TYPE(_p)  ((*(_p)) & 0xf)
#define ENT_MARK(_p)  (*((char*)(_p)) < 0)

// NB: never change type
#define ENT_SET_TYPE(_p, _v) *(_p) |= (_v);
#define ENT_SET_MARK(_p)   *(_p) |= 0x80
#define ENT_CLEAR_MARK(_p)  *(_p) &= 0x7f

#define ENT_QX(_p) ((_p)[1] & 7)
#define ENT_QY(_p) ((_p)[1] & 7)
#define ENT_QZ(_p) (((*(_p)) >> 4) & 0x3)

#define ENT_SET_QZ(_p, _v)  *(_p) = ((*(_p) & ~0x30) | (_v))
#define ENT_SET_QX(_p, _v) (_p)[1] = (((_p)[1] & ~0x3) | (_v))
#define ENT_SET_QY(_p, _v) (_p)[2] = (((_p)[2] & ~0x3) | (_v))

// sx5, qx3, (sy4, sx1), qy3
#define ENT_SXY(_p, _x, _y)     \
{                               \
    _x = ((_p)[1] >> 2) & ~1;   \
    _y = (_p)[2] >> 3;          \
    if (_y & 1) ++_x;           \
    _y >>= 1;                   \
}

#define ENT_SET_SXY(_p, _x, _y)                                 \
{                                                               \
    (_p)[1] = (((_x) & 0x3e) << 2) | ENT_QX(_p);                \
    (_p)[2] = ((_y) << 4) | (((_x)&1) << 3) | ENT_QY(_p);       \
}

// _d is 16 bits
#define ENT_DAT(_p) (*(uint16*)(_p + 3))
#define ENT_SET_DAT(_p, _d)  ENT_DAT(_p) = (_d)

// 14 bits
#define ENT_ENERGY_BITS 14
#define ENT_ENERGY_LIMIT (1<<ENT_ENERGY_BITS)

#define ENT_ENERGY(_p) (ENT_DAT(_p) & (ENT_ENERGY_LIMIT-1))
#define ENT_SET_ENERGY(_p, _v) \
    ENT_SET_DAT(_p, (ENT_DAT(_p) >> ENT_ENERGY_BITS) | (_v))

// klingons have half energy max
#define ENT_ENERGYK_BITS  ENT_ENERGY_BITS
#define ENT_ENERGYK_LIMIT (1<<ENT_ENERGYK_BITS)


#define ENT_SIZE  5
#define ENT_COUNT_MAX   800

#define ENT_TYPE_ENTERPRISE 0
#define ENT_TYPE_BASE   1
#define ENT_TYPE_STAR 2
#define ENT_TYPE_PLANET  3
#define ENT_TYPE_KLINGON 4
#define ENT_TYPE_ROMULAN 5

#define QUAD(_x, _y, _z)   \
    quadrants + (((_z)<<6) + ((_y)<<3) + (_x))*3;


// -- functions

void genGalaxy();
unsigned int rand16();
