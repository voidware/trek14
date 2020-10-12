/**
 *
 *    _    __        _      __                           
 *   | |  / /____   (_)____/ /_      __ ____ _ _____ ___ 
 *   | | / // __ \ / // __  /| | /| / // __ `// ___// _ \
 *   | |/ // /_/ // // /_/ / | |/ |/ // /_/ // /   /  __/
 *   |___/ \____//_/ \__,_/  |__/|__/ \__,_//_/    \___/ 
 *                                                       
 *  Copyright (©) Voidware 2018.
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to
 *  deal in the Software without restriction, including without limitation the
 *  rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 *  sell copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 * 
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 * 
 *  THE SOFTWARE IS PROVIDED "AS IS," WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 *  IN THE SOFTWARE.
 * 
 *  contact@voidware.com
 */

#include "defs.h"
#include "os.h"
#include "ent.h"
#include "command.h"
#include "sound.h"
#include "utils.h"


// by-pass RAM test
#define SKIPxx


#ifdef SKIP
static void printStack()
{
    int v;
    printf_simple("Stack %x\n", ((uint)&v) + 4);

}
#endif

static void pressEnter()
{
    getSingleCommand("ENTER to begin");
}

static void startGame()
{
    cls();

    printf_simple("TRS-80 Model %d (%dk RAM)\n", (int)TRSModel, (int)TRSMemory);

#ifdef HIRES    
    if (grayfx) printf_simple("Grayfx Board Detected\n");
#endif

#ifndef SKIP
    // When you run this on a real TRS-80, you'll thank this RAM test!
    peformRAMTest();
#else
    // printStack();
#endif

    outs("\nTREK 2014! v1.0\n\n"
"Your mission is to BOLDY:\n\n"
"* Explore the galaxy\n"
"* Destroy the 50 Klingons\n"
"* Locate the 5 class M planets\n"
         );

#ifndef SKIP
    // play the theme tune
    playNotes("14tF6Eb+9D3C2Bb-AAb21Gb3"
              "F6F+9Eb3D2CB-b"
              "A21Ab3G9A3"
              "BC+D2EFG6G#18"
              "Ab-9Bb3C+DEb2FGbAb6A18"
              );
#endif

    pressEnter();
    cls();
    outs("Generating Galaxy...\n");

    genGalaxy();
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
            gameover = FALSE;
            if (c == 'Y') break; 
        }
    } while (!gameover);
}

int main()
{
    initModel();

    // for bbasic programs we could mostly get away without relocating
    // the stack to the top of memory.
    setStack();
    mainloop();
    revertStack();
    setM4Map4(); // revert to normal mode (if M4)
    
    return 0;   // need this to ensure call to revert (else jp)
}
