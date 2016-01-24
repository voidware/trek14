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

void main()
{
    uchar m;

    // initialise our own mini-clib
    libcInit();

    m = getModel();
    if (m >= 4)
    {
        hookClockInts();
    
        // put into model 3 mode
        setModel(3);
    }
    else if (m == 1)
    {
        // HACK to prevent the sound routines from enabling interrupts
        clobber_rti();
    }

    cls();
    
    outs("Trek 2014!\n");
    outs("Generating Galaxy...\n");

#if 1
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
