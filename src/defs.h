/**
 *
 *    _    __        _      __                           
 *   | |  / /____   (_)____/ /_      __ ____ _ _____ ___ 
 *   | | / // __ \ / // __  /| | /| / // __ `// ___// _ \
 *   | |/ // /_/ // // /_/ / | |/ |/ // /_/ // /   /  __/
 *   |___/ \____//_/ \__,_/  |__/|__/ \__,_//_/    \___/ 
 *                                                       
 *  Copyright (�) Voidware 2019.
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

#ifndef __defs_h__
#define __defs_h__

#include <stdbool.h>
#include <string.h> 

typedef unsigned char uchar;
typedef unsigned int uint;

typedef uchar BOOL;
#define TRUE 1
#define FALSE 0

// 16 bitints
typedef unsigned int uint16;
typedef int int16;

#define VIDRAM ((char*)0x3c00)
#define VIDSIZE 1024

#define VIDRAM80 ((char*)0xf800)
#define VIDSIZE80 (80*24)

#define HIGH48K ((char*)0xFFFF)
#define HIGH32K 0xBFFF
#define HIGH16K 0x7FFF

#define COMMAND_LINE_BUF_M1 ((char*)0x4318)
#define COMMAND_LINE_BUF_M3 ((char*)0x4225)

// row 0..7
#define KBBASE ((uchar*)0x3800)
#define KBBASE80 ((uchar*)0xf400)

// TRSDOS and LDOS location of HIGH$
//#define DOS_HIGH ((int*)0x4049)

#define ROM_CURSOR  ((char**)0x4020)

#define ABSC(_c) ((char)(_c) < 0 ? -(_c) : (_c))
#define ABS(_c) ((_c) < 0 ? -(_c) : (_c))
#define SIGN(_c) ((_c) < 0 ? -1 : 1)
#define DIM(_x)  (sizeof(_x)/sizeof((_x)[0]))

#ifdef _WIN32
#define KEY_ARROW_LEFT  '4'
#define KEY_ARROW_RIGHT '6'
#define KEY_ARROW_UP '8'
#define KEY_ARROW_DOWN '2'
#else
#define KEY_ARROW_LEFT  8
#define KEY_ARROW_RIGHT 9
#define KEY_ARROW_UP 91
#define KEY_ARROW_DOWN 10
#define KEY_ARROW_UP_M4 11
#endif

//#pragma callee_saves outchar

#endif // __defs_h__

