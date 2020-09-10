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
#include "plot.h"
#include "sound.h"
#include "ent.h"


void alert2(const char* msg1, const char* msg2, uchar len)
{
    uchar i;
    uchar s1, s2;

    cls();
    plotHLine(0,0,127, 1);
    plotHLine(0,47,127, 1);

    s1 = strlen(msg1);
    s2 = 0;
    
    if (msg2) s2 = strlen(msg2);

    // centre message
    i = 32 - (s1 + s2);
    setcursor(i,3);

    outsWide(msg1);            
    if (s2) outsWide(msg2);
    
    for (i = 0; i < len; ++i)
    {
        setWide(1);
        alertsound(); 
        setWide(0);
        alertsound();
    }

    cls();

    // need to redraw
    redrawsr = TRUE;
}

void alert(const char* msg, uchar len)
{
    alert2(msg, 0, len);
}

