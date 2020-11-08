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

#define COL_W   19
#define COL_H   14
#define COL_BARV (char)0xbf
#define COL_BARH (char)0x8c

#define ROW_W   COL_W*3
#define ROW_H   4

static uchar visit;

static void renderBarV(char* cp, uchar w)
{
    char i;
    for (i = COL_H-1; i > 0; --i)
    {
        *cp = COL_BARV;
        cp += w;
    }
    *cp = (char)0x8f; // miss out bottom pixel
}

static void renderBarH(char* cp, uchar w)
{
    uchar i;
    for (i = w; i > 0; --i) *cp++ = COL_BARH;
}

static void voidMark(char* cp, char x, char y, char z)
{
    // mark void if outside grid
    if (((x & 7) != x) || ((y & 7) != y)) strcpy(cp + 6 + 2, "Void");
    else
    {
        // check we're in Z range before marking visited
        uchar v = visit && ABS(QZ-z) <= 1;
        if (!markQuadVisited(x, y, z, v)) strcpy(cp + 8, "????");
    }
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
    char j;

    char y, z;
    uchar kk = 0;

    for (j = 0; j < COL_H-1; ++j)
    {
        if ((j & 0x3) == 0)
        {
            renderBarH(cp, COL_W);
        }
        else
        {
            voidMark(cp, x0, (j>>2)+y0, (j&3)-1);

        }
        cp += COL_W;
    }

    renderBarV(col,  COL_W);

    // fill in the content
    uchar* ep;
    for (ep = galaxy; ep != galaxyEnd; ep += ENT_SIZE)
    {
        if (ENT_QX(ep) == (uchar)x0) // fail when x0 < 0
        {
            y = ENT_QY(ep);
            z = ENT_QZ(ep);
            if (!GET_EXPLORED(x0, y, z)) continue; 
            
            uchar dy = y - y0;
            if (dy < 3)  
            {
                uchar t = mainType(ep);
                if (t == ENT_TYPE_KLINGON) ++kk;
                
                uchar cy = dy*ROW_H + z + 2; // +2 for header
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

static uchar renderRow(char* row, char x0, char y0)
{
    memset(row, ' ', ROW_W*ROW_H);

    renderBarH(row, ROW_W);

    // render v bars
    char i, j;
    char* rp = row;
    for (j = 0; j < ROW_H; ++j)
    {
        for (i = 0; i < 3; ++i)  
        {
            *rp = COL_BARV;
            if (j) voidMark(rp, i+x0, y0, j-1);
            rp += COL_W;
        }
    }
    
    uchar kk = 0;

    // fill in the content
    uchar* ep;
    for (ep = galaxy; ep != galaxyEnd; ep += ENT_SIZE)
    {
        if (ENT_QY(ep) == (uchar)y0)
        {
            char x = ENT_QX(ep);
            char z = ENT_QZ(ep);
            if (!GET_EXPLORED(x, y0, z)) continue; 
            
            uchar dx = x - x0;
            if (dx < 3)  
            {
                uchar t = mainType(ep);
                if (t == ENT_TYPE_KLINGON) ++kk;

                char* cp = row + ((z + 1)*ROW_W + dx*COL_W + t*2 + 1 + 1);
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

#if 0
static void blitRow(char* row, char y)
{
    uchar* p = vidaddr(0, y);
    uchar* cp = row;
    uchar i;
    for (i = 0; i < ROW_H; ++i)
    {
        memcpy(p, cp, ROW_W);
        cp += ROW_W;
        p += colCount;
    }
}
#endif

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

static void moveR(char* col)
{
    // scroll right
    char i, j;
    for (j = COL_W-1; j >= 0; --j)
    {
        char* cp = col + j;
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

static void moveL(char* col)
{
    // scroll left
    char i, j;
    for (j = 0; j < COL_W; ++j)
    {
        char* cp = col + j;
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

static void moveD(char* row)
{
    char i;

    row += ROW_W*(ROW_H-1);
    
    for (i = 0; i < ROW_H; ++i)
    {
        // +1 for title -1 to leave last hline
        char* p = vidaddr(0, COL_H-1); 
        char y = COL_H-2-1;
        while (y)
        {
            --y;
            char* q = p - colCount;
            memcpy(p, q, ROW_W);
            p = q;
        }
        memcpy(p, row, ROW_W);
        row -= ROW_W;
    }
}

static void moveU(char* row)
{
    char i;
    for (i = 0; i < ROW_H; ++i)
    {
        char* p = vidaddr(0, 2);
        char y = COL_H-2-1; // leave last line
        while (y)
        {
            --y;
            char* q = p + colCount;
            memcpy(p, q, ROW_W);
            p = q;
        }
        memcpy(p, row, ROW_W);
        row += ROW_W;
    }
}


static void renderEdge(char qy)
{
    // draw right edge h bars
    
    char i, j;
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
        renderBarV(vidaddr(COL_W*3,1), colCount);
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
            kk += renderRow(col, qx-1, qy-1);
            moveD(col);
        }
        else if (c == KEY_ARROW_DOWN)
        {
            if (++qy > 7) { qy = 7; goto again; }
            kk += renderRow(col, qx-1, qy+1);
            moveU(col);
        }
        else break;
    }

    return c;
}

