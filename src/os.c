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

// ---- OS FUNCTIONS -----------------------------------------------------

#ifdef _WIN32

#include <stdio.h>

void outchar(char c)
{
    putchar(c);
}

void outcharat(uchar x, uchar y, uchar c)
{
    setcursor(x,y);
    outchar(c);
}

char getkey()
{
    char c;
    scanf("%c", &c);
    return c;
}

void setcursor(uchar x, uchar y)
{
}

static void getcursor(uchar* x, uchar* y)
{
    *x = 0;
    *y = 0;
}

void cls()
{
}

static uchar getline(char* buf, uchar nmax)
{
    char tbuf[256];
    int m = scanf("%s", tbuf);
    strncpy(buf, tbuf, m);
    return m;
}

#else

/* trs80 ----------------------------------------- */

#if 0
typedef void (*Fv)();
typedef void (*Fvb)(uchar);
typedef uint (*Fwbb)(uchar, uchar);

uchar args[16];

#define BP(_n) (args + sizeof(Fv*))[_n]
#define WP(_n) ((uint*)(args + sizeof(Fv*)))[_n]

static void callee_vb()
{
    (*((Fvb*)args))(BP(0));
}

static void callee_wbb()
{
    WP(0) = (*((Fwbb*)args))(BP(2),BP(3));
}

#define CALL_f(__f)  *((Fv**)args) = (Fv*)(__f)

#define CALL_vb(_f, _a)                         \
{                                               \
    CALL_f(_f);                                 \
    BP(0) = (uchar)(_a);                        \
__asm                                           \
    exx                                         \
    call _callee_vb                             \
    exx                                         \
 __endasm;                                      \
}

#define CALL_wbb(_f, _w, _a, _b)                \
{                                               \
    CALL_f(_f);                                 \
    BP(2) = (uchar)(_a);                        \
    BP(3) = (uchar)(_b);                        \
__asm                                           \
    exx                                         \
    call _callee_wbb                            \
    exx                                         \
 __endasm;                                      \
    _w = WP(0);                                 \
}

#endif


// store our own cursor position (do not use the ROM location)
static uint cursorPos;

void outcharat(uchar x, uchar y, uchar c)
{
    // no checking!
    *(VIDRAM + ((int)y<<6) + (int)x) = c;
}

void outchar(char c)
{
    uint a = cursorPos;
    
    if (c == '\b')
    {
        VIDRAM[a] = ' ';
        if (a) --a;
    }
    else if (c == '\n')
    {
        a = (a + 64) & ~63;
        if (a >= VIDSIZE)
        {
            // scroll
            memmove(VIDRAM, VIDRAM + 64, VIDSIZE-64);
            lastLine();
            return;
        }
    }
    else if (c == '\r')
    {
        // ignore
    }
    else
    {
        VIDRAM[a] = c;
        a = (a + 1) & (VIDSIZE-1);
    }
    cursorPos = a;
}

char getkey()
{
    // wait for a key
    // uses AF, DE
    __asm
     call #0x49
     ld l,a
    __endasm;
}

void setcursor(char x, char y)
{
    cursorPos = ((int)y<<6) + x;
}

void cls()
{
    memset(VIDRAM, ' ', VIDSIZE);
    cursorPos = 0;
    setWide(0);
}

void random()
{
    __asm
    call #0x1d3
    __endasm;
}

uchar getModel()
{
    // code thanks to gp2000
    // return 1, 3 or 4 for Model I, III or 4.

    __asm
	in	a,(0xff)	; read OUTMOD latches
	ld	b,a		; save original settings
	ld	c,#0x60
	xor	c		; invert CPU Fast, DISWAIT
	out	(0xec),a	; set latches
	in	a,(0xff)	; read latches
	xor	c		; flip to original value
	xor	b		; compare against original
	ld	c,#0xec
	out	(c),b		; return original settings
	rlca
	rlca
	jr	nc,m4		; CPU Fast unchanged, must be Model 4
	rlca
	ld	l,#3
	ret	nc		; DISWAIT same, Model III
	ld	l,#1		; otherwise, its a Model I
	ret
m4:	ld	l,#4
	ret
     __endasm;
}

void outPort(uchar port, uchar val)
{
    __asm
        pop hl          ; ret
        pop bc          ; port->c, val->b
        push bc
        push hl
        out (c),b
        ret
    __endasm;
}

void hookClockInts()
{
    __asm
        di

        ;; XXX hack disable clock ints for now...
        xor  a
        out  (0xe0),a
        ei
        ret

      ld      hl,(0x4013)
      ld      (.clockchain+1),hl
      ld      hl,#.clockirq
      ld      (0x4013),hl
      ei
      ret

.clockirq:
.clockchain:
        jp      0
    __endasm;

}

void setModel(uchar m)
{
 // from gp2000:
 //
 // Putting 0 out port 0x84 will put a Model 4 into Model III mode. You'll be
 // changing the way memory is mapped so TRSDOS (or LSDOS) won't work once you
 // start messing with port 0x84 so interrupts should be disabled.
 // 
 // The bottom 2 bits of port 0x84 select the memory map:
 // 
 // 00 - Model III memory map (ROM, keyboard, video in the expected locations).
 // 01 - Model III without ROM (keyboard and video mapped in per Model III, but first 14 KB is RAM)
 // 10 - Model 4 -- all memory is RAM except 0xf400 - 0xf7ff for keyboard, 0xf800 - 0xffff video RAM.
 // 11 - All RAM
 // 
 // Bit 2 of 0x84 is set to turn on 80 column mode. Bit 3 for inverse video
 // (characters 0x80 - 0xff display as inverted versions of characters 0x00 -
 // 0x7f). Bit 7 of 0x84 selects the video page which gives you access to the
 // other KB of video memory if you're in Model III mode or swaps the pages in
 // Model 4 mode. I can never remember exactly how it works, but if you're in map
 // 2 and 0xf800 - 0xffff isn't mapping nicely to the display then try setting
 // that bit.
 // 
 // The remaining 3 bits of 0x84 are for accessing the other 64KB of RAM if you
 // have 128KB.

 // In terms of speed, the Model I's Z-80 runs at 1.77408 MHz and the Model III is
 // only a bit faster at 2.02752 MHz.
 // 
 // The Model 4 can run at twice the speed of a Model III depending on the
 // version. Sending 0x40 to port 0xec will double the clock rate, but on earlier
 // models will impose 2 or 1 extra T-States per M1 cycle (which is kinda per
 // instruction but not exactly). Since you run at slower speeds I assume you'd
 // just as well run a Model 4 at Model III speeds by sending 0 to port 0xec.
 // 
 // In any case, your sound delay loops will have to use about 1.16 more clocks on
 // the Model III to sound the same as the Model I.


    if (m == 3)
    {
        outPort(0x84, 0); // 40 column
    }
}

#endif

// ---- LIB FUNCTIONS -----------------------------------------------------

void outs(const char* s)
{
    while (*s) outchar(*s++);
}

void outsWide(const char* s)
{
    // arrange even location
    if (cursorPos & 1) outchar(' ');
    
    while (*s)
    {
        outchar(*s++);
        outchar(' ');
    }
}

#ifdef _WIN32
uchar getline2(char* buf, uchar nmax)
{
    return getline(buf, nmax);
}

#else
uchar getline2(char* buf, uchar nmax)
{
    // our own version of `getline' 
    // the os version always prints the newline, but this one
    // does not. This can be useful when we dont want the screen to scroll
    // because of our input.
    uchar pos = 0;
    for (;;)
    {
        char c;

        // emit prompt
        VIDRAM[cursorPos] = '_';
        
        // wait for key
        c = getkey();

        if (c == '\b') // backspace
        {
            if (pos--)
                outchar(c);
        }
        else 
        {
            if (pos < nmax)
            {
                buf[pos++] = c;
                outchar(c);
            }
            if (c == '\r') break;
        }
    }
    return pos;
}
#endif

void lastLine()
{
    // put the cursor on the last line and clear it

    memset(VIDRAM + VIDSIZE - 64, ' ', 64);
    cursorPos = VIDSIZE - 64;
}

void setWide(uchar v)
{
    // model I
    outPort(0xFF, v ? 0x08 : 0);
    
    // other models
    outPort(0xEF, v ? 0x04 : 0);
}





