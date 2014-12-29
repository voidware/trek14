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
#include "damage.h"

uchar mline;

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
    "No enemies",
    "Destroyed",
    "No torpedoes",
    "Docked",
    "Returned to Starfleet HQ",
    "Out of Energy. You die drifting through space",
    "Ship and crew killed in battle",
    "You are relieved of command",
    "You are ordered to return to HQ, Quadrant 7,7,2",
    "Shields Buckling",
    "Shields Holding",
};

void msgLine()
{
    setcursor(0, mline);
    clearline();
}

void message(const char* m)
{
    msgLine();
    outs(m);
}

void messageCode(uchar mc)
{
    message(msgTable[mc]);
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
    // set cursor for command input. always the base of the screen
    setcursor(0, 15);
    clearline();
}

void cMessage(const char* s)
{
    baseLine();
    outs(s);
}

char warpCommand()
{
    int x, y, z;
    char v = opCheck(L_WARP);

    if (v)
    {
        cMessage("Location: ");
        scanf("%d,%d,%d", &x, &y, &z);
        v = canwarp(x,y,z);
        if (v)
        {
            // our new position after warp
            QX = x;
            QY = y;
            QZ = z;
        }
    }
    return v;
}

void phaserCommand()
{
    if (opCheck(L_PHASERS))
    {
        if (quadCounts[ENT_TYPE_KLINGON])
        {
            int e;
            cMessage("Energy: ");
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
}

uchar torpCommand()
{
    // fire torpedo
    // return non-zero if successful and enemy move taken

    uchar u = opCheck(L_TORPS);
    
    if (u)
    {
        u = ENT_TORPS(galaxy);
        if (u > 0)
        {
            int dir;
            message("Direction: ");
            scanf("%d", &dir);

            // if out of range, abort command
            if (dir >= 0 && dir <= 360)
            {
                ENT_SET_TORPS(galaxy, u-1);

                // allow the enemy a move *after* torpedo direction set
                enemyMove();

                torps(galaxy, dir);
            }
            else u = 0;
        }
        else
            messageCode(MSG_CODE_NO_TORPS);
    }
    return u;
}

void docCommand()
{
    if (findAdjacent(galaxy, ENT_TYPE_BASE))
    {
        // full house
        ENT_SET_DAT(galaxy, ENT_REFUEL_DATA);
        repairAll();
        messageCode(MSG_CODE_DOCKED);

        if (QX == 7 && QY == 7 && QZ == 2)
            endgame(MSG_CODE_ENDGAME_RESIGN);
    }
}

void tick()
{
    static int recalled;

    // general ship repair
    opTick();
    
    // advance time
    if (++stardate > STARDATE_END)
    {
        if (!recalled)
        {
            // overdue
            messageCode(MSG_CODE_RETURN_HQ);
            recalled = 1;
        }

        // get a grace period to return, otherwise score suffers
        if (stardate > STARDATE_END + STARDATE_GRACE)
        {
            score -= 10;
            if (score < 0)
            {
                score = 0;
                endgame(MSG_CODE_ENDGAME_RELIEVED);
            }
        }
    }
}


// mr spock, you have the conn :-)
void conn()
{
    char c, k;
    char buf[4];
    
    k = 0;
    
    do
    {
        if (k) { c = k; k = 0; }
        else
        {
            cMessage("Command: ");
            getline2(buf, sizeof(buf));
            c = buf[0];
            if (islower(c)) c = _toupper(c);
        }
        
        mline = 14;
        
        switch (c)
        {
        case 'L':
            lrScan();
            tick();
            break;
        case 'W':
            if (warpCommand())
            {
                // complete warp command on SR view
                k = srScan('W' | 0x80); 
            }
            break;
        case 'S':
            k = srScan(0);
            break;
        case 'P':
        case 'T':
            k = srScan(c);
            break;
        default:
            c = 0;
        }
    } while (c);
}
