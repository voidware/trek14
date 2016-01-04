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

char inkey()
{
    char c;

    do 
    {
        c = getkey();
    } while (c == '\n');
    return c;
}

void setcursor(uchar x, uchar y)
{
}

void getcursor(uchar* x, uchar* y)
{
    *x = 0;
    *y = 0;
}

void cls()
{
}

uchar getline(char* buf, uchar nmax)
{
    char tbuf[256];
    int m = scanf("%s", tbuf);
    strncpy(buf, tbuf, m);
    return m;
}

#else

// trs80

void outchar(char c)
{
    // print `c' at current cursor position
    // uses AF, DE, BC, HL

    __asm
    pop hl
    pop bc
    push bc
    push hl
    ld a,c
    call #0x33
   __endasm;
}

char inkey()
{
    // uses AF, DE
    __asm
     call #0x2b
     ld l,a
    __endasm;
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

#if 0
uchar getline(char* buf, uchar nmax)
{
    // get whole line
    // uses AF, DE
    // return number of chars including terminator
    
    __asm
     pop bc
     pop hl
     push hl
     push bc
     ld iy,#4
     add iy,sp
     ld b,0(iy)
     call #0x40
     ld l,b
    __endasm;
}
#endif

void setcursor(uchar x, uchar y)
{
    // 0x4020
    char** p = CURMEM;
    *p = VIDRAM + ((int)y<<6) + (int)x;
}

void getcursor(uchar* x, uchar* y)
{
    char** p = CURMEM;
    int v = *p - VIDRAM;
    *x = v & 63;
    *y = v >> 6;
}

void cls()
{
    // uses AF
    __asm
    call #0x1c9
    __endasm;
}

void random()
{
    __asm
    call #0x1d3
    __endasm;
}

void outcharat(uchar x, uchar y, uchar c)
{
    *(VIDRAM + ((int)y<<6) + (int)x) = c;
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

void setModel(uchar m)
{
    if (m == 3)
        outPort(0x84, 0);
}

#endif

// ---- LIB FUNCTIONS -----------------------------------------------------

void outs(const char* s)
{
    while (*s)
        outchar(*s++);
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

        // current cursor pos
        char* cp = *CURMEM;
        
        // emit prompt
        *cp = '_';
        
        // wait for key
        c = getkey();

        if (c == 0x08) // backspace
        {
            if (pos)
            {
                --pos;
                *cp = ' ';
                outchar(c);
            }
        }
        else 
        {
            if (pos < nmax)
            {
                buf[pos++] = c;
                if (c != 0xd)
                    outchar(c);
            }
            if (c == 0xd) break;
        }
    }
    return pos;
}
#endif

void clearline()
{
    // clear from current cursor to end of line
    // leave cursor where it is
#ifndef _WIN32
    uchar x, y;
    char* cp;

    cp = *CURMEM;
    getcursor(&x, &y);

    x = 64 - x;
    while (x)
    {
        --x;
        *cp++ = ' ';
    }
#endif    
}






