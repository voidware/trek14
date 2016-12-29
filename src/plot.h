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


extern const unsigned char div3tab[];

void plot(uchar x, uchar y, uchar c);
uchar plotSpan(uchar x0, uchar y, uchar n, uchar c);
void drawRLE(char x, char y, const uchar* dp, uchar c);
void moveRLE(char x, char y, const uchar* dp, signed char dx);

void plotHLine(uchar x1, uchar y, uchar x2, uchar c);
void plotVLine(uchar x, uchar y1, uchar y2, uchar c);

typedef void plotfn(char x, char y);
void plotLine(char x1, char y1, char x2, char y2, plotfn* fn);
uchar pixelsRLE(const uchar* dp, char* pix);

