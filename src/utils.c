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

// helper utility functions

#include "defs.h"
#include "os.h"
#include "libc.h"

void memzero(void* ptr, size_t n)
{
    memset(ptr, 0, n);
}

void printfat(uchar x, uchar y, const char* fmt, ...)
{
    // printf at (x,y) character position
    // NB: text is automatically flushed (without need for "\n")
    
    va_list args;
    setcursor(x, y);    
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    flush();
}

#if 0
unsigned char isqrt16(unsigned short a)
{
    // 16 bit version, valid up to 16383
    unsigned short rem = 0;
    unsigned char root = 0;
    unsigned char i;
    for (i = 0; i < 8; ++i)
    {
        root <<= 1;
        rem = (rem << 2) + (a >> 14);
        a <<= 2;
        if (root < rem)
        {
            ++root;
            rem -= root;
            ++root;
        }
    }
    return root >> 1;
}
#endif

