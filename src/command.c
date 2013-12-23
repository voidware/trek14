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
#include "command.h"

void command()
{
    for (;;)
    {
        cls();
        printf("(S)hort Range Scan\n");
        printf("(L)ong Range Scan\n");
        printf("(W)warp\n");
        conn();
    }
}

void warpCommand()
{
    int x, y, z;
    
    printf(" Location: "); flush();
    scanf("%d,%d,%d", &x, &y, &z);

    warp(x,y,z);
}

// mr spock, you have the conn :-)
void conn()
{
    char buf[4];
    char c;

    do
    {
        printfat(0, 15, "Command: ");
        getline(buf, sizeof(buf));
    
        c = buf[0];

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
            c = srScan();
            if (c) goto again;
            break;
        default:
            c = 0;
        }
    } while (c);
}
