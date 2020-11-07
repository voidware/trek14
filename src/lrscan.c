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

#include <stdlib.h>
#include "defs.h"
#include "os.h"
#include "utils.h"
#include "ent.h"
#include "plot.h"
#include "command.h"
#include "damage.h"
#include "sound.h"

/*
static void lrScanAt(char qx, char qy, char qz, uchar visit)
{
    // long range scan

    char i, j;
    uchar x, y, z;
    uchar cx, cy;

    printfat(0, 0, "Long Range Scan, Quadrant %d,%d,%d\n",
             (int)qx, (int)qy, (int)qz);

    cy = 3;
    y = qy-1;
    for (i = 0; i <= 2; ++i, ++y)  // loop quadrant Y
    {
        cx = 2; 
        x = qx-1;
        for (j = 0; j <= 2; ++j, ++x) // loop quadrant X
        {
            char k;
            
            if (!i) printfat(cx + 8, 1, "%d", (int)x);
            z = qz-1;

            for (k = 0; k <= 2; ++k, ++z) // loop quadrant Z
            {
                char buf[ENT_TYPE_COUNT*2+1];
                char* bp;
                if (x < 8 && y < 8 && z < 3)
                {
                    // counts for each type
                    uchar quad[ENT_TYPE_COUNT];
                    
                    // find out what we have in the quadrant
                    uchar* ents[ENT_QUAD_MAX];
                    getQuad(x, y, z, quad, ents);
                    if (visit) markVisited(ents);
                    
                    uchar* cp = quad;
                    uchar** epp = ents;
                    bp = buf;
                    
                    const char* tc = entTypeChar;
                    for (tc = entTypeChar; *tc; ++tc)
                    {
                        uchar c = *cp;

                        // find out if any visible
                        uchar vis = 0;
                        while (c)
                        {
                            --c;
                            if (ENT_MARKED(*epp)) ++vis;
                            ++epp;
                        }

                        if (!strchr("HMWD", *tc))
                        {
                            if (vis)
                            {
                                *bp = *tc;
                                bp[1] = '0' + *cp;

                                if (*bp == 'K')
                                    playNotes("18t+EC"); // nameF, nameH
                            }
                            else
                            {
                                *bp = ' ';
                                bp[1] = ' ';
                            }
                            bp += 2;
                        }
                        ++cp;
                    }

                    *bp = 0;
                    bp = buf;
                    


                }
                else
                {
                    // each window has 16 chars for max 8 totals
                    // BFPSKR
                    //bp = (char*)"123456Void789012";
                    bp = (char*)"      Void      ";
                }
                printfat(cx, cy + k, bp);
            }
            cx += 19;
        }
        --cx;
        printfat(cx, cy, "%2d", (int)(qz-1));
        printfat(cx, cy + 1, "%2d %d", (int)qz, (int)y);
        printfat(cx, cy + 2, "%2d", (int)(qz+1));
        cy += 4;
    }
}
*/

#define COL_W   19
#define COL_H   14
#define COL_BARV (char)0xbf
#define COL_BARH (char)0x8c

static uchar visit;

static void renderBar(char* cp, uchar w)
{
    char i;
    for (i = 0; i < COL_H-1; ++i)
    {
        *cp = COL_BARV;
        cp += w;
    }
    *cp = (char)0x8f; // miss out bottom pixel
}

static uchar renderCol(char* col, char x0, char y0)
{
    // render 1 column of (x,y),(x,y+1),(x,y+2)
    // into a screen buffer `col`
    
    memset(col, ' ', COL_W*COL_H);

    // NB: x0 can be < 0
    _itoa(x0, col + COL_W/2, 10); // SDCC extention

    // draw the bars
    char* cp = col + COL_W;
    char i,j;

    char y;
    char z;
    uchar kk = 0;

    for (j = 0; j < COL_H-1; ++j)
    {
        if ((j & 0x3) == 0)
        {
            for (i = 0; i < COL_W; ++i) *cp++ = COL_BARH;
        }
        else
        {
            y = (j>>2) + y0;

            // mark void if outside grid
            if (((x0 & 7) != x0) || ((y & 7) != y)) strcpy(cp + 6 + 2, "Void");
            else
            {
                // check we're in Z range before marking visited
                z = (j&3)-1;
                uchar v = visit && ABS(QZ-z) <= 1;
                if (!markQuadVisited(x0, y, z, v)) strcpy(cp + 8, "????");
            }
            cp += COL_W;
        }
    }

    renderBar(col, COL_W);

    // fill in the content
    uchar* ep;
    for (ep = galaxy; ep != galaxyEnd; ep += ENT_SIZE)
    {
        if (ENT_QX(ep) == (uchar)x0)
        {
            y = ENT_QY(ep);
            z = ENT_QZ(ep);
            if (!GET_EXPLORED(x0, y, z)) continue; 
            
            uchar dy = y - y0;
            if (dy < 3)  
            {
                uchar t = mainType(ep);
                if (t == ENT_TYPE_KLINGON) ++kk;
                
                uchar cy = dy*4 + z + 2; // +2 for header
                cp = col + (cy*COL_W + t*2 + 2 + 1); // +2 for bar+margin
                if (*cp == ' ')
                {
                    *cp = '0';
                    cp[-1] = entTypeChar[t];
                }
                ++*cp;
            }
        }
    }
    return kk;
}

static void blitCol(char* col, char x)
{
    uchar* p = vidaddr(x, 1); // always row 1
    uchar* cp = col;
    uchar i;
    for (i = 0; i < COL_H; ++i)
    {
        memcpy(p, cp, COL_W);
        cp += COL_W;
        p += colCount;
    }
}

#if 1
// need these line movements to be fast
// C code below works but slower
static void _moveR(char* p)
{
    __asm
        pop     bc             ; ret
        pop     hl             ; p
        push    hl
        push    bc
        ld      de, #0x38
        add     hl,de
        ld      b,#0x38
        ld      e,l
        ld      d,h
.01$:
        dec     de
        ld      a,(de)
        ld      (hl),a
        ld      l,e
        ld      h,d
        djnz    .01$
    __endasm;
}

static void _moveL(char* p)
{
    __asm
        pop     bc             ; ret
        pop     hl             ; p
        push    hl
        push    bc
        ld      de, #0xffc8    ; -38
        add     hl,de
        ld      b,#0x38
        ld      e,l
        ld      d,h
.02$:
        inc     de
        ld      a,(de)
        ld      (hl),a
        ld      l,e
        ld      h,d
        djnz    .02$
    __endasm;
}

#else
static void _moveR(char* p)
{
    char* q = p + (COL_W*3-1);
    char c = COL_W*3-1;
    do
    {
        char* t = q;
        --t;
        *q = *t;
        q = t;
    } while (--c);
}

static void _moveL(char* p)
{
    char* q = p - (COL_W*3-1);
    uchar c = COL_W*3-1;

    while (c)
    {
        --c;
        
        char* t = q;
        ++t;
        *q = *t;
        q = t;
    }
}

#endif


static void moveR(uchar* col)
{
    char i, j;
    for (j = COL_W-1; j >= 0; --j)
    {
        uchar* cp = col + j;
        char* p = vidaddr(0, 1); // always row 1
        for (i = COL_H; i > 0; --i)
        {
            _moveR(p);
            *p = *cp;
            cp += COL_W;
            p += colCount;
        }
    }
}


static void moveL(uchar* col)
{
    char i, j;
    for (j = 0; j < COL_W; ++j)
    {
        uchar* cp = col + j;
        char* p = vidaddr(COL_W*3-1, 1); // always row 1
        for (i = COL_H; i > 0; --i)
        {
            _moveL(p);
            *p = *cp;
            p += colCount;
            cp += COL_W;
        }
    }
}

static void renderEdge(char qy)
{
    char i, j;

    // draw right edge h bars
    char* p = vidaddr(COL_W*3, 2);
    for (j = 0; j < COL_H-1; ++j)
    {
        if ((j & 0x3) == 0)
        {
            if (visit)
            {
                char* q = p;
                for (i = 0; i < 6; ++i) *q++ = COL_BARH;
            }
        }
        else 
        {
            char z = (j&3)-1;
            p[1] = '0' + z;
            if (z == 1)
            {
                char y = (j>>2) + qy-1;
                _itoa(y, p + 4, 10); // SDCC extention
            }
        }
        p += colCount;
    }

    if (visit)
    {
        renderBar(vidaddr(COL_W*3,1), colCount);
    }
}

char lrScan()
{
    char c;

    if (!opCheck(L_SCANL)) return 0;

    cls();

    // enter interactive loop
    char qx = QX;
    char qy = QY;
    char qz = QZ;
    char col[COL_W*COL_H];
    uchar redraw = 1;
    uchar kk;
        
    visit = 1; // visit first pass to explore.

    for (;;)
    {
        printfat(0, 0, "Long Range Scan from %d,%d,%d\n",
                 (int)qx, (int)qy, (int)qz);

        if (redraw)
        {
            redraw = 0;
            kk = 0;
            
            kk += renderCol(col, qx-1, qy-1);
            blitCol(col, 0);
        
            kk += renderCol(col, qx, qy-1);
            blitCol(col, COL_W);
        
            kk += renderCol(col, qx+1, qy-1);
            blitCol(col, COL_W*2);
        }
        
        renderEdge(qy);
        visit = 0;

        // subsequent scans from level 1
        qz = 1;

        if (kk > 2) kk = 2;
        while (kk)
        {
            --kk;
            playNotes("18t+EC"); // nameF, nameH
        }

    again:
        
        c = getACommand();

        if (c == KEY_ARROW_LEFT)
        {
            if (--qx < 0) { qx = 0; goto again; }
            kk += renderCol(col, qx-1, qy-1);            
            moveR(col);
        }
        else if (c == KEY_ARROW_RIGHT)
        {
            if (++qx > 7) { qx = 7; goto again; }
            kk += renderCol(col, qx+1, qy-1);
            moveL(col);            
        }
        else if (c == KEY_ARROW_UP || c == KEY_ARROW_UP_M4)
        {
            if (--qy < 0) { qy = 0; goto again; }
            redraw =1 ;
        }
        else if (c == KEY_ARROW_DOWN)
        {
            if (++qy > 7) { qy = 7; goto again; }
            redraw =1 ;
        }
        else break;
    }

    return c;
}

