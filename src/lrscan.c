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
#include "utils.h"
#include "ent.h"
#include "plot.h"
#include "command.h"
#include "damage.h"
#include "sound.h"

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
                    bp = (char*)"     Void     ";
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

char lrScan()
{
    char i;
    uchar x, y;
    char c;

    if (!opCheck(L_SCANL)) return 0;

    cls();

    y = 7; // 2*cy+1
    x = 0;
    for (i = 0; i < 4; ++i)
    {
        plotHLine(0, y, 125, 1);
        plotVLine(x, 3, 43, 1);
        plotVLine(x+1, 3, 43, 1);
        y += 4*3; // 4 lines
        x += (1 + 18)*2; // self + 18 chars
    }

    // enter interactive loop
    char qx = QX;
    char qy = QY;
    char qz = QZ;
    uchar visit = 1; // visit first pass to explore.
    for (;;)
    {
        lrScanAt(qx, qy, qz, visit);
        visit = 0;

        // subsequent scans from level 1
        qz = 1;

    again:
        
        c = getSingleCommand("Command: ");

        if (c == KEY_ARROW_LEFT)
        {
            if (--qx < 0) { qx = 0; goto again; }
        }
        else if (c == KEY_ARROW_RIGHT)
        {
            if (++qx > 7) { qx = 7; goto again; }
        }
        else if (c == KEY_ARROW_UP || c == KEY_ARROW_UP_M4)
        {
            if (--qy < 0) { qy = 0; goto again; }
        }
        else if (c == KEY_ARROW_DOWN)
        {
            if (++qy > 7) { qy = 7; goto again; }
        }
        else break;
    }

    return c;
}

