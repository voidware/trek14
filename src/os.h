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

void outchar(char c);
void outcharat(char x, char y, char c);
void outint(int v);
void outuint(uint v);
int putchar(int c);
char getkey();
char scanKey();
char scanKeyMatrix(char hold);
void setcursor(char x, char y);
void cls();
void clsc(uchar c);
void setWide(uchar v);
void initModel();
void uninitModel();
void pause();
void setStack();
void revertStack();
void enableInterrups();

void outs(const char* s);
void outsWide(const char* s);
void printf_simple(const char* f, ...);
int sprintf_simple(char* buf, const char* f, ...);
void printfat(char x, char y, const char* f, ...);
uchar scanf_simple(const char* f, ...);
uchar getline(char* buf, uchar nmax);
void lastLine();
void lastLinex(uchar x);
uchar* vidaddr(char x, char y);
uchar* vidaddrfor(uint a);
uchar ramTest(uchar a, uchar n);

typedef void (*IdleHandler)(uchar);
void setIdleHandler(IdleHandler h, uchar d);

void srand(uint v);
unsigned int rand16();
uint randn(uint n); // 16 bit version
uchar randc(uchar n); // 8 bit version

void setM4Map3();
void setM4Map4();
void outPort(uchar port, uchar val);
uchar inPort(uchar port);
void setSpeed(uchar fast);
void clearLine();

extern uchar TRSModel;
extern uchar TRSMemory;
extern uchar* TRSMemoryFail;
extern uchar cols80;
extern uchar rowCount;
extern unsigned int scrollPos;
extern unsigned int cursorPos;
extern uchar* vidRam;
extern char* CmdLine;
extern uchar hiresBoard;
extern uchar grayfx;
extern uchar useSVC;

/* file IO */
int readFile(const char* name, char* buf, int bz);
int writeFile(const char* name, char* buf, int bz);

