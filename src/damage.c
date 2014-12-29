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
        message(opTable[i]);        
        outs(" inoperative, Captain!");
    }
    return u;
}

void subop(uchar op, int val)
{
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
        opCheck(op);
        alertsound();
    }
}

void repair(uchar r)
{
    // distribute `r' units of repair amongst defective operations.
    uchar i;
    uchar mino = 1;
    for (i = 1; i < L_COUNT; ++i)
    {
        uchar v = operations[i];

        if (v < operations[mino])
            mino = i;

        if (v < OP_MIN)
        {
            v = OP_MIN - v;
            if (r < v) v = r;
            operations[i] += v;
            if (OPERATIONAL(i))
            {
                // repaired!
                message(opTable[i]);
                outs(" now operational, Captain");
            }
            r -= v;
        }
    }

    // give any remaining repair units to the min operational
    addop(mino, r);
}

void takeDamage(int dam)
{
    // we've been hit with `dam' units of energy.

    int s = GET_SHIELD_ENERGY;
    
    // absorption of shields
    dam -= s;
    if (dam < 0)
    {
        // some left, give back to shields
        SET_SHIELD_ENERGY(-dam);
        messageCode(MSG_CODE_SHIELDS_OK);
    }
    else
    {
        // residual damage goes to operations
        int di[L_COUNT-2];
        uchar i, j, m;
        int dv;
        int dm;

        if (s > 0)
        {
            SET_SHIELD_ENERGY(0);
            messageCode(MSG_CODE_SHIELDS_GONE);
            alertsound();
        }

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
            dv = di[m] - dm;
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


