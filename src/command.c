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
#include "lrscan.h"
#include "srscan.h"
#include "warp.h"
#include "phasers.h"
#include "enemy.h"
#include "command.h"

void command()
{
    for (;;)
    {
        cls();
        printf("\n  (S)hort Range Scan\n"
               "  (L)ong Range Scan\n"
               "  (W)arp\n"
               "  (P)hasers\n"
               "  (T)orpedoes\n");
        conn();
    }
}

static const char* msgTable[] = 
{
    "Not Enough Energy",
    "No Target",
    "Destroyed",
    "No Torpedoes",
    "Docked",
    "Returned to Starfleet HQ",
    "Out of Energy. You die drifting through space",
    "Ship and crew killed in battle",
    "You are relieved of command"
};

void messageCode(uchar mc)
{

    baseLine();
    outs(msgTable[mc]);
    outs(", captain!");

}

void endgame(uchar msg)
{
    cls();
    
    setcursor(3,3);
    outs(msgTable[msg]);
    
    printf("\n   Your score is %d\n", score);

    while (1) ;
}

void baseLine()
{
    // prompt at the base of the screen, clearing anything already there
    setcursor(63, 15);
    clearline();
}

void warpCommand()
{
    int x, y, z;
    
    baseLine();
    printf("Location: "); flush();
    scanf("%d,%d,%d", &x, &y, &z);
    warp(x,y,z);
}

void phaserCommand()
{
    int e;
    
    if (quadCounts[ENT_TYPE_KLINGON])
    {
        baseLine();
        printf("Energy: "); flush();
        scanf("%d", &e);

        if (e > 0)
        {
            if (e <= ENT_ENERGY(galaxy))
            {
                phasers(galaxy, e, ENT_TYPE_KLINGON);
                showState();
            }
            else
                messageCode(MSG_CODE_INSUFENERGY);
        }
    }
    else
        messageCode(MSG_CODE_NO_TARGET);
}

void torpCommand()
{
    // fire torpedo
    int dir;

    int t = ENT_TORPS(galaxy);
    if (t > 0)
    {
        baseLine();
        printf("Direction: "); flush();
        scanf("%d", &dir);

        // if out of range, abort command
        if (dir >= 0 && dir <= 360)
        {
            ENT_SET_TORPS(galaxy, t-1);
            torps(galaxy, dir);
        }
    }
    else
        messageCode(MSG_CODE_NO_TORPS);
    
}

void docCommand()
{
    if (findAdjacent(galaxy, ENT_TYPE_BASE))
    {
        // full house
        ENT_SET_DAT(galaxy, ENT_REFUEL_DATA);
        messageCode(MSG_CODE_DOCKED);

        if (QX == 7 && QY == 7 && QZ == 2)
            endgame(MSG_CODE_ENDGAME_RESIGN);
    }
}

// mr spock, you have the conn :-)
void conn()
{
    char buf[4];
    char c;

    do
    {
        printfat(0, 15, "Command: ");
        getline2(buf, sizeof(buf));
    
        c = buf[0];

        // clear bottom line
        baseLine();

    again:

        if (islower(c)) c = _toupper(c);

        switch (c)
        {
        case 'L':
            lrScan();
            break;
        case 'W':
            warpCommand();
            // fall through
        case 'S':
            c = srScan(0);
            if (c) goto again;
            break;
        case 'P':
        case 'T':
            c = srScan(c);
            if (c) goto again;
            break;
        default:
            c = 0;
        }
    } while (c);
}
