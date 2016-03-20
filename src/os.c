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
#include "sound.h" // clobber_rti

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
// this was an attempt at roll-your-own callee saves
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


// store our own cursor position (do not use the OS location)
static uint cursorPos;

// are we in 80 col mode?
uchar cols80;
static uchar* vidRam;

// what model? (set up by initModel)
uchar TRSModel;

static uint vidoff(char x, char y)
{
    uint a = x + ((int)y<<6);
    if (cols80)
        a += (int)y<<4;
    return a;
}

uchar* vidaddr(char x, char y)
{
    uint a = vidoff(x, y);
    if (a >= VIDSIZE && !cols80 || a >= VIDSIZE80) return 0;
    return vidRam + a;
}

void outcharat(uchar x, uchar y, uchar c)
{
    *vidaddr(x,y) = c;
}

void lastLine()
{
    // put the cursor on the last line and clear it
    if (cols80)
    {
        setcursor(0, 23);
        memset(VIDRAM80 + 23*80, ' ', 80);
    }
    else
    {
        setcursor(0, 15);
        memset(VIDRAM + 15*64, ' ', 64);
    }
}

void nextLine()
{
    uint a = cursorPos;
    if (cols80)
    {
        // bump a to the next multiple of 80
        uint v = 80;
        if (a >= 800) v += 800;  // skip around half
        while (v <= a) v += 80;
        a = v;

        if (a >= VIDSIZE80)
        {
            // scroll
            memmove(VIDRAM80, VIDRAM80 + 80, 23*80);

            // place at last line and clear line
            lastLine();
            return;
        }
    }
    else
    {
        a = (a + 64) & ~63;
        if (a >= VIDSIZE)
        {
            // scroll
            memmove(VIDRAM, VIDRAM + 64, 15*64);

            // place at last line and clear line
            lastLine();
            return;
        }
    }
    cursorPos = a;
}

void outchar(char c)
{
    uint a = cursorPos;
    uchar* p = vidRam + a;
    
    if (c == '\b')
    {
        *p = ' ';
        if (a) a--;
    }
    else if (c == '\n')
    {
        nextLine();
        return;
    }
    else if (c == '\r')
    {
        // ignore
    }
    else
    {
        *p = c;
        ++a;
        if (a >= VIDSIZE && !cols80 || a >= VIDSIZE80)
        {
            // scroll and place on last line
            nextLine();
            return;
        }
    }
    cursorPos = a;
}


void setcursor(char x, char y)
{
    cursorPos = vidoff(x, y);
}

void cls()
{
    if (cols80)
    {
        memset(VIDRAM80, ' ', VIDSIZE80);
    }
    else
    {
        memset(VIDRAM, ' ', VIDSIZE);
    }
    
    cursorPos = 0;
    setWide(0);
}

#if 0
void random()
{
    __asm
    call #0x1d3
    __endasm;
}
#endif

static void outPort(uchar port, uchar val)
{
    __asm
        pop hl          ; ret
        pop bc          ; port->c, val->b
        push bc
        push hl
        out (c),b
    __endasm;
}

static uchar inPort(uchar port)
{
    __asm
        pop hl          ; ret
        pop bc          ; port->c
        push bc
        push hl
        in  l,(c)
    __endasm;
}

static uchar ramAt(uchar* p)
{
    __asm
        pop bc
        pop hl
        push hl      // p
        push bc
        inc (hl)     
        ld  a,(hl)
        dec (hl)
        sub (hl)
        ld  l, a
    __endasm;
}

static uchar getModel()
{
    uchar m = 1;

    /* model 3:
       ROM from 0000 to 37FF
    */

    // change to M4 bank 1, which maps RAM over 14K ROM
    outPort(0x84, 1); 

    if (ramAt((uchar*)0x2000))
    {
        // this is a 4 or 4P.
        // NB: leave at bank 1 for now...
        m = 4;
    }
    else
    {
        // M3 or M1
        uchar v = inPort(0xff);
        
        // toggle DISWAIT
        outPort(0xec, v ^ 0x20);

        if (inPort(0xff) != v)
        {
            // changed, we are M3
            outPort(0xEC, v);  // restore original
            m = 3;
        }
    }

    return m;
}

static void setSpeed(uchar fast)
{
    if (TRSModel >= 4)
    {
        // M4 runs at 2.02752 or 4.05504 MHz
        outPort(0xec, fast ? 0x40 : 0);
    }
}

#if 0
static void setModel3()
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


    // this does doesnt work on LDOS
    __asm
        di
        ld  a,#0x65
        RST #0x28       //@FLAGS SVC
        xor  a
        ld 14(iy),a     // OPREG$ image
        out (#0x84),a   // select base memory map
        ld 12(iy),a     // MODOUT image
        out (#0xec),a   // slow speed
        ei
     __endasm;

        outPort(0x84, 0); // 40 column
        setSpeed(0); // slow
}
#endif

#endif

// row 0..7
#define KBBASE ((uchar*)0x3800)
#define KBBASE80 ((uchar*)0xf400)

static uchar readKeyRowCol(uchar* rowcol)
{
    uchar r = 1;
    uchar i;
    static uchar kbrows[8];
    
    for (i = 0; i < 8; ++i)
    {
        uchar v = cols80 ? *(KBBASE80 + r) : *(KBBASE + r);
        r <<= 1;
        
        if (v != kbrows[i])
        {
            kbrows[i] = v;

            if (v) // press not release
            {
                *rowcol = i; // row
            
                rowcol[1] = 0;
                while (v > 1)
                {
                    v >>= 1;
                    ++rowcol[1];
                }
                return 1;
            }
        }
    }
    return 0;
}

static const char keyMatrix[] =
{
    '@', 'A', 'B', 'C', 'D', 'E', 'F', 'G',
    'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',
    'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W',
    'X', 'Y', 'Z', 'Z', 'Z', 'Z', 'Z', 'Z',
    '0', '1', '2', '3', '4', '5', '6', '7',
    '8', '9', ':', ';', ',', '-', '.', '/',
    '\r', '\b', 'Z', KEY_ARROW_UP, KEY_ARROW_DOWN, KEY_ARROW_LEFT, KEY_ARROW_RIGHT, ' ',
    'Z', 'Z', 'Z', 'Z', 'Z', 'Z', 'Z', 'Z', 
};

char readKey()
{
    uchar rc[2];

    // ignore shift & control
    while (!readKeyRowCol(rc) || rc[0] == 7) ;
    
    return keyMatrix[rc[0]*8 + rc[1]];
}

char getkey()
{
    // wait for a key
    char c;
    for (;;)
    {
        c = readKey();
        if (c) return c;
    }
}

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
        vidRam[cursorPos] = '_';
        
        // wait for key
        c = getkey();

        if (c == '\b') // backspace
        {
            if (pos)
            {
                --pos;
                outchar(c);
            }
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



void setWide(uchar v)
{
    if (TRSModel == 1)
    {
        // model I
        outPort(0xFF, v << 3); // 8 or 0
    }
    else
    {
        // get MODOUT (mirror of port 0xec)
        // NB: do not read from 0xEC
        uchar m = inPort(0xff);
        
        // set or clear MODSEL bit 
        if (v) m |= 4;
        else m &= ~4;

        outPort(0xEC, m);
    }
}

void initModel()
{
    cols80 = 0;
    vidRam = VIDRAM;

    TRSModel = getModel();

    if (TRSModel >= 4)
    {
        cols80 = 1;
        vidRam = VIDRAM80;
        
        outPort(0x84, 0x86); // M4 map, 80cols
        setSpeed(0); // slow (for now..)

    }
    else if (TRSModel == 1)
    {
        // HACK to prevent the sound routines from enabling interrupts
        //clobber_rti();
    }

    // leave interrupts off in all cases for now...
    clobber_rti();
}

#endif




