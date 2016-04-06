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
#include "ent.h"
#include "command.h"
#include "sound.h"
#include "utils.h"

#define SKIPxx

static void peformRAMTest()
{
    uchar a = 0;
    uchar n = TRSMemory;
    if (n >= 64) n -= 3; // dont test the top 3k screen RAM + KB

    // loop 1K at a time.
    printfat(0,1,"RAM TEST ");
    do
    {
        uchar b = a<<2;
        ++a;
        if (TRSMemory < 64) b += 0x40; 
        
        printfat(9,1, "%dK ", (int)a);
        if (!ramTest(b, 4)) break;
        --n;
    } while (n);

    if (!n)
        printf("OK\n");
    else
        printf("FAILED at %x\n", (uint)TRSMemoryFail);
}

static void startGame()
{
    uchar v;
    
    cls();

    printf("TRS-80 Model %d (%dk RAM)\n", (int)TRSModel, (int)TRSMemory);

    peformRAMTest();

#ifdef SKIP
    printf("Stack %x\n", ((int)&v) + 4);
#endif

    outs("\n\nTrek 2014!\n");
    outs("Generating Galaxy...\n");

#ifndef SKIP
    playNotes("14tF6Eb+9D3C2Bb-AAb21Gb3"
              "F6F+9Eb3D2CB-b"
              "A21Ab3G9A3"
              "BC+D2EFG6G#18"
              "Ab-9Bb3C+DEb2FGbAb6A18"
              );
#endif

    genGalaxy();
    outs("done\n");

    command();
}

static void mainloop()
{
    do
    {
        startGame();

        for (;;)
        {
            char c = getSingleCommand("Play Again (Y/N)? ");
            if (c == 'N') break; // leave gameover true
            gameover = 0;
            if (c == 'Y') break; 
        }
    } while (!gameover);
}

void main()
{
    initModel();
    
    // initialise our own mini-clib
    libcInit();
    
    mainloop();
}
