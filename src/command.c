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
#include "lrscan.h"
#include "srscan.h"
#include "warp.h"
#include "phasers.h"
#include "enemy.h"
#include "command.h"
#include "damage.h"
#include "sound.h"
#include "story.h"

static uchar conn();

void command()
{
    do 
    {
        playNotes("+1G"); // nameE

        cls();
        printf_simple("\n  (S)hort Range Scan\n"
               "  (L)ong Range Scan\n"
               "  (W)arp\n"
               "  (P)hasers\n"
               "  (T)orpedoes\n"
               "  (C)omputer\n"
               "  (D)amage Report\n"
                      );
    } while (!conn());
}

const char* crewTable[] = 
{
    "Capn'", // 0
    "Keptin",  // 1
    "Captain", // 2
    "?", // 3
    "Scotty",  // 4
    "Chekov",  // 5 weapons
    "Spock",  //6
    "Uhura", // 7
    "Bones", // 8
    "Sulu", // 9 helmsman
    "Jim",
};

// syntax allows message variations in [] and references to another 
// message table with ^
static const char* msgTable[] = 
{
    "^4: [Insufficient|Not Enough] Energy, ^0",
    "^5: No enemies here, ^1!",
    "Destroyed!",
    "^5: [No torpedoes[ left|]|Out of torpedoes], ^1",
    "^9: Docked, ^2",
    "Returned to Starfleet HQ",
    "Out of Energy. You die drifting through space",
    "Ship and crew killed in battle",
    "^6: ^2, you are relieved of command.", // for disobeying orders
    "^7: ^2, Starfleet orders you to HQ 772",
    "^4: Shields [Buckling|Collapsing], ^0!",
    "^4: Shields [holding|absorbed it], ^0",
    "^5: Phasers can't lock on, ^1!",
    "^4: [Nothing to dock wi'|Ya canny' dock], ^0!",
    "^6: Destroying a Federation base is grounds for court martial!",
    "^6: You [violated the prime directive.|destroyed it!] You are relieved of command.",
    "^6: Enemy shields absorbed impact, ^2",
    "^6: Lifeless G Planet, ^2",
    "^6: Class M Planet, ^2. Fascinating!",
    "^9: We've already been here, ^2!",
    "^8: I've got sick people down here, ^10!",
};

static void emitStory(const char* m, Story* st)
{
    // select message tables and emit message "story"
    st->sub[0] = crewTable;
    st->subSize[0] = DIM(crewTable);
    st->sub[1] = msgTable;
    st->subSize[1] = DIM(msgTable);
    story(m, st);    
}

static void emitStoryCmd(const char* m)
{
    Story st;
    memset(&st, 0, sizeof(st));
    emitStory(m, &st);    
}

void emitStoryCmdM(uchar mc)
{
    char buf[16];
    sprintf_simple(buf, "^b%d", (int)mc);
    emitStoryCmd(buf);
}

void clearMessage()
{
    lastLinex(MSG_X);
}

#if 0
void message(const char* m)
{
    // emit a given message on the message line
    clearMessage();
    emitStoryCmd(m);
    pause();
    clearMessage();
}
#endif

void messageCode(uchar mc)
{
    // emit a message with a code on the message line.
    // pause and clear it
    clearMessage();
    emitStoryCmdM(mc);
    pause();
    clearMessage();
}

void cMessage(const char* s)
{
    // set cursor for command input. always the base of the screen
    clearMessage();
    emitStoryCmd(s);
}

typedef struct 
{
    char       _score;
    const char* _review;
} Review;

static const Review reviewTable[] =
{
 { 15, "promoted." },   // 15/16 = 93%
 { 14, "decorated." },  // 87.5%
 { 12, "reassigned." }, // 75%
 { 8, "demoted." },  // 50%
 { 0, "resigned!" },
 { -1, "executed!!" },
};

void endgame(uchar msg)
{
    char buf[16];
    Story st;
    memset(&st, 0, sizeof(st));
    st.wrap = 55;
    st.lmargin = 3;

    cls();

    sprintf_simple(buf, "\n\n^b%d", (int)msg);
    emitStory(buf, &st);

    if (msg == MSG_CODE_ENDGAME_RESIGN)
    {
        // score review
        story("\n\nAdmiral [Fitzpatrick|Komack|Westervliet|Barstow|Fitzgerald] here;\nCaptain, after reviewing your tapes and logs, I [can only|must|have to] recommend that you be ", &st);

        const Review* rp = reviewTable;
        for (;;)
        {
            // score table is fractions of 1/16.
            // scoremax < 65535/16 = 4095
            // so multiply up to 16 is ok.
            char s = rp->_score;
            if (s < 0) break;
            uint slim = (((uint)s)*((uint)scoremax))>>4;
            if (score >= slim) break;
            ++rp;
        }
        story(rp->_review, &st);
    }

    if (score < 0) score = 0;
    printf_simple("\n\n   Your score is %d\n", score);
    gameover = TRUE;
}

char warpCommand()
{
    int x, y, z;
    if (opCheck(L_WARP))
    {
        cMessage("^9, [Location|Destination|Course]: ");

        // can type 7,7,2 or 772
        uchar c = scanf_simple("%d,%d,%d", &x, &y, &z);
        clearMessage();
        
        if (c == 1)
        {
            z = x % 10;
            y = (x / 10) % 10;
            x /= 100;
        }
        else if (c != 3) return FALSE;

        if (canwarp(x, y, z))
        {
            // our new position after warp
            QX = x;
            QY = y;
            QZ = z;
            return TRUE;
        }
    }
    return FALSE;
}

void phaserCommand()
{
    if (opCheck(L_PHASERS))
    {
        if (quadCounts[ENT_TYPE_KLINGON])
        {
            int e;
            cMessage("^5, Energy: ");
            scanf_simple("%d", &e);
            clearMessage();

            if (e > 0)
            {
                // ensure keep some back.
                if (e <= (int)(ENT_ENERGY(galaxy) - 500))
                {
                    // phaser lock sound; doesnt work properly
                    //playNotes("CE++F--B++D-F#-B+"); // nameP, C, L, B, G, K,  D
                    if (!phasers(galaxy, e, ENT_TYPE_KLINGON))
                        messageCode(MSG_CODE_PHASERS_NO_LOCK);
                    
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

void torpCommand()
{
    // fire torpedo
    
    if (opCheck(L_TORPS))
    {
        uchar u = ENT_TORPS(galaxy);
        if (u > 0)
        {
            int dir;
            cMessage("^5, [Direction|Angle|Vector]: ");
            scanf_simple("%d", &dir);
            clearMessage();

            // if out of range, abort command
            if (dir >= 0 && dir <= 360)
            {
                ENT_SET_TORPS(galaxy, u-1);
                torps(galaxy, dir);
            }
        }
        else
            messageCode(MSG_CODE_NO_TORPS);
    }
}

static uchar orbit(uchar t)
{
    // 0 => nothing here
    // !0 => t here and not orbit already
    uchar* ep = adjacentTo(galaxy, t);
    if (ep)
    {
        if (ENT_DOCKED(ep)) messageCode(MSG_CODE_ORBIT_ALREADY);
        else
        {
            // mark we've been here
            ENT_SET_DOCKED(ep);
            return 1;
        }
    }
    return 0;
}

void dockCommand()
{
    if (adjacentTo(galaxy, ENT_TYPE_BASE))
    {
        emitStoryCmdM(MSG_CODE_DOCKED);
        
        // full house
        ENT_SET_DAT(galaxy, ENT_REFUEL_DATA);
        repairAll();
        redrawSidebar();
        
        // refuel sound
        upSound();

        clearMessage();
    }
    else if (adjacentTo(galaxy, ENT_TYPE_BASEHQ))
    {
        endgame(MSG_CODE_ENDGAME_RESIGN);
    }
    else
    {
        if (orbit(ENT_TYPE_PLANET)) messageCode(MSG_CODE_PLANET_G);
        else if (orbit(ENT_TYPE_PLANET_M))
        {
            emitStoryCmdM(MSG_CODE_PLANET_M);
            upSound();
            score += SCORE_PLANET_M;
            pause();
            clearMessage();
        }
        else
        {
            // nothing
            messageCode(MSG_CODE_NO_DOCK);
        }
    }
}

void tick()
{
    if (alertLevel)
    {
        // running cost 10 units per tick extra if in red alert
        // NB: game can end here.
        takeEnergy(galaxy, 10);
    }

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
                // did not return to HQ.
                endgame(MSG_CODE_ENDGAME_RELIEVED);
            }
        }
    }
}

static void computerScan(uchar type)
{
    uchar* ep;
    uchar c = 0;
    uchar nearest = 0xff;
    uchar nx, ny, nz;

    cls();
    printf_simple("Ship's Computer Memory Bank Scan for %c\n", entTypeChar[type]);

    for (ep = galaxy; ep != galaxyEnd; ep += ENT_SIZE)
    {
        if (mainType(ep) == type)
        {
            uchar x, y, z, d;
            
            x = ENT_QX(ep);
            y = ENT_QY(ep);
            z = ENT_QZ(ep);

            if (!GET_EXPLORED(x, y, z)) continue;

            d = distmTo(x, y, z);
            if (d < nearest)
            {
                nearest = d;
                nx = x; 
                ny = y;
                nz = z;
            }

            if ((c++ & 0x3) == 0) outs("\n");
            printf_simple("%c%d,%d,%d ", entTypeChar[ENT_TYPE(ep)], (int)x, (int)y, (int)z);
        }
    } 

    if (!c) printf_simple("\nNone Known\n");
    else printf_simple("\n\nNearest %d,%d,%d\n", (int)nx, (int)ny, (int)nz);
    
    getkey();
}

static void computerCommand()
{
    char c;

    do
    {
        cls();
    
        printf_simple("Search Ship's memory banks for:\n\n"
           "  (K) Klingon locations\n"
           "  (B) Bases\n"
           "  (P) Planets\n"
           "  (R) Return\n"
           );

        c = getSingleCommand("Operation: ");
    
        switch (c)
        {
        case 'K':
            computerScan(ENT_TYPE_KLINGON);
            break;
        case 'B':
            computerScan(ENT_TYPE_BASE);
            break;
        case 'P':
            computerScan(ENT_TYPE_PLANET);
            break;
        }
    } while (c != '\r' && c != 'R');
}

static void damageReport()
{
    cls();

    uchar c = opCol;
    opCol = 10;

    drawOperations();
    
    opCol = c;
}

char getACommand()
{
    char c = getSingleCommand("Command: ");
    clearMessage();
    return c;
}

// mr spock, you have the conn :-)
static uchar conn()
{
    char c, k;
    
    k = 0;
    
    do
    {
        if (k) { c = k; k = 0; }
        else
            c = getACommand();
        
        switch (c)
        {
        case 'L':
            k = lrScan();
            tick();
            break;
        case 'W':
            if (warpCommand())
            {
                // complete warp command on SR view
                k = srScan('W' | 0x80);  // XX flag command chosen
            }
            break;
        case 'S':
            k = srScan(0);
            break;
        case 'P':
        case 'T':
            k = srScan(c);
            break;
        case 'D':
            damageReport();
            break;
        case 'C':
            computerCommand();
            // fall through
        default:
            c = 0;
        }

        if (gameover) return TRUE;

    } while (c);
    return 0;
}
