/**
 *
 *    _    __        _      __                           
 *   | |  / /____   (_)____/ /_      __ ____ _ _____ ___ 
 *   | | / // __ \ / // __  /| | /| / // __ `// ___// _ \
 *   | |/ // /_/ // // /_/ / | |/ |/ // /_/ // /   /  __/
 *   |___/ \____//_/ \__,_/  |__/|__/ \__,_//_/    \___/ 
 *                                                       
 *  Copyright (©) Voidware 2019.
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

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>

#include "defs.h"
#include "os.h"
#include "fio.h"
#include "hires.h"

// store our own cursor position (do not use the OS location)
unsigned int cursorPos;

// are we in 80 col mode?
uchar cols80;
uchar rowCount;

// location of video ram 0x3c00 or 0xf800
uchar* vidRam;

// what model? (set up by initModel)
uchar TRSModel;
uchar useSVC;

// should output be converted to upper case?
uchar TRSUppercaseOutput;

// How much memory in K  (initModel)
uchar TRSMemory;
uchar* TRSMemoryFail;

// command line used 
char* CmdLine;
char* CmdArg;

// random number seed
uint seed;

static uchar* OldStack;
static uchar* NewStack;

// point to scroll from (usually 0)
unsigned int scrollPos;

#ifdef HIRES
uchar hiresBoard;
uchar grayfx;
#endif

static uint vidoff(char x, char y)
{
    // calculate the video offset from the screen base for CHARACTER pos (x,y)
    uint a;

    a = (uint)y<<6;
    if (cols80) a += (uint)y<<4;
    return a + x;
}

uchar* vidaddrfor(uint a)
{
    // find the video ram address for offset `a'
    if (a >= VIDSIZE && !cols80 || a >= VIDSIZE80) return 0;
    return vidRam + a;
}

uchar* vidaddr(char x, char y)
{
    // return the video address of (x,y) or 0 if off end.
    return vidaddrfor(vidoff(x,y));
}

static void setChar(volatile char* a, char c)
{
    if (TRSModel <= 2)
    {
        // Each time we see a T, check for lcase mod?
        if (c == 'T')
        {
            // 20 displays the same as T, 20+64=84=T
            *a = 20;
            TRSUppercaseOutput = (*a != 20);
        }

        if (TRSUppercaseOutput) c = toupper(c);
    }

    *a = c;
}

void outcharat(char x, char y, char c)
{
    // set video character directly without affecting cursor position
    setChar(vidaddr(x, y), c);
}

static uint nextLinePos()
{
    // calculate the next line from the current cursor pos
    uint a = cursorPos;
    if (cols80)
    {
        // bump a to the next multiple of 80
        uchar q = a/80;
        a = (q + 1)*80;
    }
    else
    {
        a = (a + 64) & ~63;  // start of next line
    }
    return a;
}

void clearLine()
{
    // clear from current cursor pos to end of line
    uint a = cursorPos;
    uint b = nextLinePos();
    memset(vidaddrfor(a), ' ', b - a);
}

void lastLinex(uchar x)
{
    // put the cursor on the last line, pos x and clear to end
    setcursor(x, rowCount-1);
    clearLine();
}

void lastLine()
{
    // put the cursor on the last line, do not clear
    setcursor(0, rowCount-1);
}

static void nextLine()
{
    uchar sc;

    char* p = vidRam + scrollPos;
    cursorPos = nextLinePos();

    if (cols80)
    {
        sc = (cursorPos >= VIDSIZE80);
        if (sc)
        {
            // scroll
            memmove(p, p + 80, VIDSIZE80 - 80 - scrollPos);
        }
    }
    else
    {
        sc = (cursorPos >= VIDSIZE);
        if (sc)
        {
            // scroll
            memmove(p, p + 64, VIDSIZE - 64 - scrollPos);
        }
    }

    if (sc)
    {
        // place at last line and clear line
        lastLinex(0);
    }
}


void outchar(char c)
{
    uint a = cursorPos;
    uchar* p = vidaddrfor(a);
    
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
        setChar(p, c);
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

void clsc(uchar c)
{
    memset(vidRam, c, (cols80 ? VIDSIZE80 : VIDSIZE));
    cursorPos = 0;
    setWide(0);
}

void cls()
{
    clsc(' ');
}


void outPort(uchar port, uchar val)
{
    __asm
        pop hl          ; ret
        pop bc          ; port->c, val->b
        push bc
        push hl
        out (c),b
    __endasm;
}

uchar inPort(uchar port)
{
    __asm
        pop hl          ; ret
        pop bc          ; port->c
        push bc
        push hl
        in  l,(c)
    __endasm;
}

void enableInterrupts()
{
    __asm
        ei
    __endasm;        
}

void disableInterrupts()
{
    __asm
        di
    __endasm;        
}

static uchar ramAt(uchar* p) __naked
{
    // return 1 if we have RAM at address `p', 0 otherwise
    __asm
        pop bc
        pop hl
        push hl         // p -> hl
        push bc
        ld  a,(hl)      // get original
        ld  b,a         // save
        xor #0xff       // flip bits
        ld  (hl),a      // change all bits in RAM
        xor (hl)        // mask
        ld  (hl),b      // restore original
        ld   l,#1       // return result if ok
        ret  z          // return if ok
        dec  l          // return 0 if bad
        ret
    __endasm;
}

char* getHigh() __naked
{
    __asm
        ld  hl,#0
        ld  b,h
        ld  a,#100 // select HIGH
        RST 0x28   // @HIGH@ -> hl
        ret
    __endasm;
}
static void setOFLAGS(uchar v)
{
    __asm
        pop  hl
        dec  sp
        pop  bc
        push bc    // b = v
        inc  sp
        push hl
        ld   a,#101    // @FLAGS
        rst  0x28
        ld   14(iy),b
    __endasm;
}

static void setM4Map(uchar v)
{
    if (useSVC) // M4 only.
    {
        disableInterrupts();
        outPort(0x84, v);
        setOFLAGS(v); // tell dos 
        enableInterrupts();
    }
}

void setM4Map3()
{
    // switch to config 3; ram + ram + KB + VIDEO
    // this is the mode we will run in
    setM4Map(0x86);
}

void setM4Map4()
{
    // revert M4 to normal system mem map and tell DOS
    setM4Map(0x87);
}

static uchar testBlock(uchar a)
{
    // test 256 bytes of RAM at address `a'00
    // return 1, ok, 0 fail
    uchar* p = (uchar*)(a << 8);
    uint r;
    for (;;)
    {
        r = ramAt(p);
        if (!r) 
        {
            // test failed, remember failure address
            TRSMemoryFail = p;
            break;
        }
        ++p;
        if (((uchar)p) == 0) break;
    } 
    return r;
}

#define ADDR(_n) ((uint)&_n)
#define ADDRH(_n) ((uchar)(ADDR(_n)>>8))

uchar ramTest(uchar a, uchar n)
{
    // test `n' blocks of 256 at `a'00
    uchar r = 1;
    do
    {
        if (a < ADDRH(ramAt) || a > ADDRH(testBlock)) r = testBlock(a);
        ++a;
        --n;
    } while (r && n);
    return r;
}

static uchar getModel()
{
    uchar m = 1;
    
    // if we have RAM, then M4
    if (ramAt((uchar*)0x2000))
    {
        // this is a 4 or 4P.
        m = 4;
    }
    else
    {
        // M3 or M1
        uchar v = inPort(0xff);
        
        // toggle DISWAIT
        outPort(0xec, v ^ 0x20);

        if (inPort(0xff) != v)  // read back from mirror
        {
            // changed, we are M3
            outPort(0xEC, v);  // restore original
            m = 3;
        }
    }

    return m;
}

void setSpeed(uchar fast)
{
    if (useSVC)
    {
        // M4 runs at 2.02752 or 4.05504 MHz
        unsigned char v = inPort(0xff);
        v &= 0x3f; // drop speed bit and undefined high bit.
        if (fast) v |= 0x40;
        outPort(0xec, v);
    }
}

// call the ROM to scan a key
char scanKey()
{
    // return key pressed or 0 if none

    if (useSVC)
    {
    __asm
        ld    a,#8      // @KBD
        RST   0x28      // uses DE
        ld    l,a
    __endasm;
    }
    else
    {
      __asm
        call    0x2b
        ld      l,a
    __endasm;
    }
}

static void dsp4(char c)
{
    // model 4 charout
    __asm
        pop  hl
        dec  sp
        pop  af                 // a = char
        push af
        inc  sp
        push hl
        ld   c,a
        ld   a,#2               // @DSP
        rst  0x28
    __endasm;
}

static uchar keyIdleState;
static IdleHandler idleHandler;
static uchar idleDelay;
static uchar idleCount;

void setIdleHandler(IdleHandler h, uchar d)
{
    idleHandler = h;
    idleDelay = d;
}

static void _keyIdle()
{
    // stir random number
    ++seed;
    
    if (idleHandler)
    {
        if (!idleCount) idleCount = idleDelay + 1;
        
        if (!--idleCount)
        {
            (*idleHandler)(keyIdleState++);
        }
    }
}

char getkey()
{
    // wait for a key
    char c;

    keyIdleState = 0;
    
    for (;;)
    {
        c = scanKey();
        if (c)
        {
            if (keyIdleState) 
            {
                // force revert to state 0
                keyIdleState = 0; // back to state 0
                idleCount = 1;
                _keyIdle(); 
            }
            return c;
        }
        else
        {
            _keyIdle();
        }
    }
}

// use ROM

uchar rom4_getline(char* buf, uchar nmax) __naked
{
    // model 4 version
    __asm
        pop  bc     // ret
        pop  hl     // buf
        pop  de     // e = nmax
        push de
        push hl
        push bc
        ld   b,e    // nmax
        ld   c,#0
        ld   a,#9   // @KEYIN
        RST  0x28
        ld   l,b    // number typed
        ret        
    __endasm;
}

uchar rom_getline(char* buf, uchar nmax) __naked
{
    // emit prompt and handle backspace etc
    __asm
        pop  bc     // ret
        pop  hl     // buf
        pop  de     // e = nmax
        push de
        push hl
        push bc
        ld   b,e    // nmax
        call 0x40
        ld   l,b    // number typed
        ret
    __endasm;
}

static uchar c4row;
static uchar c4col;
static void setROMCursor()
{
    if (useSVC)
    {
        // M4
        c4row = cursorPos/80;
        c4col = cursorPos - c4row*80;
        
        __asm
            ld  a,#15   // @VDCTL
            ld  b,#3    // set cursor
            ld  hl,#_c4row
            ld  d,(hl)
            ld  hl,#_c4col
            ld  e,(hl)
            ex  de,hl
            RST 0x28
        __endasm;
    }
    else
    {
        // set the ROM cursor to our cursor
        *ROM_CURSOR = VIDRAM + cursorPos;
    }
}

uchar getline(char* buf, uchar nmax)
{
    setROMCursor();

    uchar n;

    if (useSVC)
    {
        dsp4(0x0e); // cursor on
        n = rom4_getline(buf, nmax);
        dsp4(0x0f); // cursor off
    }
    else
    {
        n = rom_getline(buf, nmax);
    }

    // terminate
    buf[n] = 0;
    return n;
}

void pause()
{
    // delay, unless key pressed
    int c = 1000;  // XX scale delay by machine speed
    while (--c)
    {
        if (scanKey()) return;
    }
}

void outs(const char* s)
{
    while (*s) outchar(*s++);
}

void outsn(const char* s, uchar n)
{
    while (n)
    {
        --n;
        outs(s);
    }
}

void outint(int v)
{
    char buf[17];
    _itoa(v, buf, 10);  // STDCC extention
    outs(buf);
}

void outuint(uint v)
{
    char buf[17];
    _uitoa(v, buf, 10);  // STDCC extention
    outs(buf);
}

int putchar(int c)
{
    outchar(c);
    return c;
}

void outsWide(const char* s)
{
    // write text in wide mode
    
   // arrange even location
    if (cursorPos & 1) outchar(' ');
    
    // write each char followed by a space
    while (*s)
    {
        outchar(*s++);
        outchar(' ');
    }
}

static int atoiparse(const uchar** pa)
{
    // atoi but skip over number
    const uchar* p = *pa;
    int v = atoi(p);
    if (*p == '-') ++p;
    while (isdigit(*p)) ++p;
    *pa = p;
    return v;
}

/* Really simple printf that handles only the very basics */
typedef void (*Emitter)(char);

static Emitter _e;

static void _pad(char w)
{
    while (w > 0) { --w; (*_e)(' '); }
}

static void _printf_simple(const char* f, va_list args)
{
    // handles:
    // %c, %d, %u, %x, %s, %ld, with field width
    for (;;)
    {
        char c = *f++;
        if (!c) break;

        if (c == '%')
        {
            char buf[33];
            char* s = 0;
            char width = atoiparse(&f); // zero if none specified
            
            c = *f++;
            if (!c) break;
            
            switch (c)
            {
            case 'd':
                _itoa(va_arg(args, int), buf, 10);  // STDCC extention
                s = buf;
                break;
            case 'x':
                _itoa(va_arg(args, int), buf, 16);  // STDCC extention
                s = buf;
                break;
            case 'u':
                _uitoa(va_arg(args, uint), buf, 10);  // STDCC extention
                s = buf;
                break;
            case 's':
                s = va_arg(args, char*);
                break;
            case 'l':
                // XX ASSUME %ld
                ++f;
                _ltoa(va_arg(args, long), buf, 10);  // STDCC extention
                s = buf;
                break;
            case 'c':
                buf[0] = va_arg(args, int);
                buf[1] = 0;
                s = buf;
                break;
            }

            if (s)
            {
                uchar* p = s;
                while (*p) ++p;
                uchar slen = (p - s);

                if (width < 0)
                    _pad(-width - slen);
                
                while (*s) (*_e)(*s++);

                if (width > 0)
                    _pad(width - slen);            
                
                continue;
            }
        }
        else if (c == '\\')
        {
            c = *f++;
            if (!c) break;
            if (c == 'n') c = '\n';
        }
        
        (*_e)(c);
    }
}

void printf_simple(const char* f, ...)
{
    va_list args;
    va_start(args, f);
    _e = outchar;
    _printf_simple(f, args);
    va_end(args);
}

void printfat(char x, char y, const char* f, ...)
{
    va_list args;

    setcursor(x, y);
    va_start(args, f);
    _e = outchar;
    _printf_simple(f, args);
    va_end(args);    
}

void getlineQuiet(char* buf, uchar nmax)
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

        if (c == '\r') break;  // enter hit
        else if (c == '\b') // backspace
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

        }
    }
    buf[pos] = 0;
}

uchar scanf_simple(const char* f, ...)
{
    // OK this is really simple.
    // supported formats: %d

    uchar n = 0;

    va_list args;
    char buf[32];
    va_start(args, f);

    getlineQuiet(buf, sizeof(buf)-1);

    const char* q = buf;
    const char* p = f;
    while (*p)
    {
        if (!*q) break;
        
        if (isspace(*p))
        {
            while (isspace(*q)) ++q;
        }
        else if (*p == '%')
        {
            ++p;
            uchar fc = *p++;
            if (fc == 'd')
            {
                int* vp = va_arg(args, int*);
                *vp = atoiparse(&q);
                ++n;
            }
        }
        else
        {
            // match
            if (*p != *q) break;
            ++p;
            ++q;
        }
    }

    va_end(args);        
    return n;
}

static char* emit_sprintf_pos;

static void emit_sprintf(char c)
{
    *emit_sprintf_pos++ = c;
}

int sprintf_simple(char* buf, const char* f, ...)
{
    va_list args;
    va_start(args, f);

    emit_sprintf_pos = buf;
    _e = emit_sprintf;
    _printf_simple(f, args);
    va_end(args);

    *emit_sprintf_pos = 0; // terminate
    
    return emit_sprintf_pos - buf;
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
    uchar* rp = (uchar*)0x4000;
    
    cols80 = 0;
    rowCount = 16;
    vidRam = VIDRAM;
    TRSMemory = 0;

    TRSModel = getModel();

#ifdef HIRES
    hiresBoard = checkgfx();
    grayfx = (hiresBoard && hiresBoard < 3);
#endif    

    if (TRSModel >= 4)
    {
        char* h = getHigh();
        
        cols80 = 1;
        rowCount = 24;
        useSVC = 1;
        vidRam = VIDRAM80;

        // switch off cursor
        dsp4(0x0f);

        setM4Map3();
        setSpeed(1); // ensure we're in fast mode!

        TRSMemory = 64;

        // m4 0xf400 - vidram is keyboard area
        NewStack = (uchar*)0xF400; 
        if (NewStack > h) NewStack = h;

        // CmdLine is already setup in crt0 for M4.
    }
    else
    {
        // how much RAM do we have?
        for (;;)
        {
            TRSMemory += 16;
            rp += 0x4000;
            if (!ramAt(rp)) break;
        }
        NewStack = rp;
        CmdLine = COMMAND_LINE_BUF_M3;

        if (TRSModel <= 2)
        {
            // convert output to upper case
            TRSUppercaseOutput = 1;
            
            CmdLine = COMMAND_LINE_BUF_M1;
        }
    }

    // switch interrupts back on now we're done poking around memory
    enableInterrupts();

    // fix the command line buf
    rp = CmdLine;

    // skip program name
    uchar c = 0;
    while (*rp != '\r')
    {
        if (*rp == ' ') ++c;
        else if (c && !CmdArg) CmdArg = rp;
        ++rp;
    }
    
    *rp = 0;
    if (!CmdArg) CmdArg = rp;
}

void setStack() __naked
{
    // locate the stack to `NewStack`
    // ASSUME we are called from main
    __asm
        pop hl
        ld (_OldStack),sp
        ld sp,(_NewStack)
        jp  (hl)
    __endasm;
}

void revertStack() __naked
{
    // put stack back to original
    // ASSUME we are called from main
    __asm
        pop hl
        ld sp,(_OldStack)
        jp (hl)
    __endasm;
}

void srand(uint v)
{
    seed = v;
}

#if 0
unsigned int rand16()
{
    // this not very good
    uint v;
    uchar a;

    v = (seed + 1)*75;
    a = v;
    a -= (v >> 8);
    seed = ((v & 0xff00) | a) - 1;
    return seed;
}

#else

unsigned int rand16()
{
    uint x = seed;
    x ^= x << 7;
    x ^= x >> 9;
    x ^= x << 8;
    seed = x;
    return x;
}
#endif

uint randn(uint n)
{
    // random [0,n-1]
    // 16 bit version
    
    uint c = 1;
    uint v;

    while (c < n) c <<= 1;
    --c;
    
    do
    {
        v = rand16() & c;
    } while (v >= n);
    
    return v;
}

uchar randc(uchar n)
{
    // random [0,n-1]
    // 8 bit version

    uchar v;
    uchar c = 0xff;
    
    if (n <= 128)
    {
        c = 1;
        while (c < n) c <<= 1;
        --c;
    }

    do
    {
        v = rand16() & c;
    } while (v >= n);
    
    return v;
}

/*** FILE IO ***********************************************/

uchar ioBuf[256];
static FCB fileIO;

static void _setFile(const char* name)
{
    char* q = fileIO;
    char c;

    while ((c = *name++) != 0)
    {
        c = toupper(c);
        if (c == '.') c = '/';
        *q++ = c;
    }

    *q++ = ':';
    *q++ = '1';
    *q = 0x0d; // terminator
}

static void _fileError(const char* name, uchar r)
{
    printf_simple("Error with file '%s', ", name);
    switch (r)
    {
    case 24:
        printf_simple("Not Found");
        break;
    case 26:
    case 27:
        printf_simple("Disk full");
        break;
    case 38:
        printf_simple("File not open");
        break;
    default:
        printf_simple("Code %d", (int)r);
        break;
    }
    outchar('\n');
}


int readFile(const char* name, char* buf, int bz)
{
    // return number of bytes read into `buf`

    //printf_simple("Reading '%s'\n", name);
    
    int cc = 0;

    _setFile(name);

    uchar r = fopen_exist(fileIO);
    while (!r)
    {
        int c = fgetc(fileIO);
        if (c < 0)
        {
            r = c; // error or EOF
        }
        else if (cc < bz)
        {
            *buf++ = c;
            ++cc;
        }
    }

    fclose(fileIO);

    if (r && r != 28) _fileError(name, r);  // EOF not error
    return cc;
}


int writeFile(const char* name, char* buf, int bz)
{
    int cc = 0;

    // printf_simple("Writing '%s' size %d\n", name, bz);
    
    _setFile(name);
    uchar r = fopen(fileIO);
    while (!r && bz)
    {
        --bz;
        r = fputc(*buf++, fileIO);
        ++cc;
    }

    fclose(fileIO);
    if (r) _fileError(name, r);
    return cc;    
}

#if 0

// records are 80 bytes
// NB: to use this version, must change the LRL in fios.s to #80
#define LRL_SIZE 80

int writeFile(const char* name, char* buf, int bz)
{
    int cc = 0;

    _setFile(name);

    uchar r = fopen(fileIO);
    if (!r)
    {
        while (bz > 0)
        {
            r = fwrite(buf, fileIO);
            if (!r)
            {
                cc += LRL_SIZE;
                bz -= LRL_SIZE;
                buf += LRL_SIZE;
            }
            else
            {
                _fileError(name, r);
                break;
            }
        }
        
        fclose(fileIO);
    }
    else _fileOpenError(name, r);
    return cc;    
}
#endif


