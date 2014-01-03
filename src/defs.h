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

typedef unsigned char uchar;
typedef unsigned int uint;

// 16 bitints
typedef unsigned int uint16;
typedef int int16;

#define VIDRAM ((char*)0x3c00)
#define VIDEND ((char*)0x3fff)
#define CURMEM ((char**)0x4020)

#define ABS(_c) ((char)(_c) < 0 ? -(_c) : (_c))

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
#endif

