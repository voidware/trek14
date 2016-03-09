/**
 * Copyright (c) 2014 Voidware Ltd.
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
#include "ent.h"
#include "command.h"
#include "damage.h"
#include "sound.h"
#include "alert.h"
#include "srscan.h"
#include "story.h"

#define OP_MIN  0x80
#define OPERATIONAL(_i)  (operations[_i] >= OP_MIN)

// operational level of each facility
uchar operations[L_COUNT];

void repairAll()
{
    // set facilities all operational, but no messages
    memset(operations, 0xff, sizeof(operations));
}

static const char* opTable[] = 
{
    "Shields",
    "Short Range Scan",
    "Impulse Engines",
    "Photon Torpedoes",
    "Warp Drive",
    "Phasers",
    "Long Range Scan",
};

static void emitStory(const char* m)
{
    Story st;
    memset(&st, 0, sizeof(st));
    st.sub[0] = crewTable;
    st.subSize[0] = 100; // plenty
    st.sub[1] = opTable;
    st.subSize[1] = DIM(opTable);

    // emit on the message line
    lastLine();
    story(m, &st);    
}

void addop(uchar op, uchar v)
{
    // add `v' units to operation `op'
    uchar u = operations[op];
    uchar t = u + v;
    if (t < u) t = 0xff;
    operations[op] = t;
}

uchar opCheckSR()
{
    return OPERATIONAL(L_SCANS);
}

uchar opCheck(uchar i)
{
    // check facility `i' is operational.
    // return non-zero if ok
    uchar u = OPERATIONAL(i);
    if (!u)
    {
        char buf[64];
        sprintf(buf, "^4: ^b%d [are out|inoperative|[still |]dunny' work], ^0!", (int)i);
        emitStory(buf);
        alertsound();  // also pause
    }
    return u;
}

void subop(uchar op, int val)
{
    // subtract (16 bit) value from operation.
    
    uchar u, v;
    if (val > 0xff) v = 0xff; // 0xff will reduce to zero anyway
    else v = val;

    u = operations[op];
    if (v >= u) v = 0;
    else v = u - v;
    operations[op] = v;
    if (u >= OP_MIN && v < OP_MIN)
    {
        // emit message
        alert2(opTable[op], " Inoperative", 2);

        // signal we should redraw SR view
        redrawsr = true;
    }
}

void repair(uchar r)
{
    // distribute `r' units of repair amongst defective operations
    // in order of priority.
    uchar i;
    for (i = 1; i < L_COUNT; ++i)
    {
        uchar v = operations[i];
        if (v < OP_MIN)
        {
            v = OP_MIN - v;
            if (r < v) v = r;
            operations[i] += v;
            if (OPERATIONAL(i))
            {
                // repaired!
                char buf[64];
                sprintf(buf, "^4: ^b%d now [operational|repaired|working], ^0", (int)i);
                emitStory(buf);
            }
            r -= v;
        }
    }
}

void takeDamage(int dam)
{
    // we've been hit with `dam' units of energy.

    int s = GET_SHIELD_ENERGY;
    uchar i, j, m;

    for (i = 1; i < 11; ++i)
    {
        // shake the ship
        for (j = 0; j < 200; ++j) setWide(i & 1);
    }
    
    
    // absorption of shields
    dam -= s;
    if (dam <= 0)
    {
        // if some left, give back to shields
        SET_SHIELD_ENERGY(-dam);
        messageCode(MSG_CODE_SHIELDS_OK);
    }
    else
    {
        // residual damage goes to operations
        int di[L_COUNT-2];
        int dv;
        int dm;

        if (s > 0)
        {
            SET_SHIELD_ENERGY(0);
            messageCode(MSG_CODE_SHIELDS_GONE);
            alertsound();
        }

        /* allocate the damage randomly to each operation (not shields).
         * generate n-1 random numbers (mod dam+1).
         * take the smallest and allocate this damage (will be <= dam)
         * then take the next smallest and allocate this much minus previous
         * give any remaining amount to the last
         */

        dm = dam + 1;

        // generate n-1 partitions of the damage value
        for (i = 0; i < L_COUNT-2; ++i)
        {
            uint r = rand16();
            uint q = r/(uint)dm;
            di[i] = r - q*dm;
        }

        dm = 0;
        for (i = 1; i < L_COUNT-1; ++i)
        {
            // find min partition
            m = 0;
            for (j = 1; j < L_COUNT-2; ++j)
                if (di[j] < di[m]) m = j;
            
            // take damage to operations i
            dv = (di[m] - dm) >> 3; // amount of operational units
            dm = di[m];
            subop(i, dv);
            di[m] = 0x7fff;
        }

        // remainder of total damage and last partition to last operation
        subop(L_COUNT-1, dam - dm);
    }
}

void opTick()
{
    repair(10);
    addop(L_SHIELDS, 10);
}


