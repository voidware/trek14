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

// operations ordered by repair importance  
#define L_SHIELDS       0
#define L_SCANS         1
#define L_IMPULSE       2
#define L_TORPS         3
#define L_WARP          4
#define L_PHASERS       5
#define L_SCANL         6
#define L_COUNT         7

extern uchar operations[];
extern uchar opCol;

#define GET_SHIELD_ENERGY       (operations[L_SHIELDS]<<3)
#define SET_SHIELD_ENERGY(_v)   operations[L_SHIELDS] = (_v)>>3

void repairAll();
void opTick();
void takeDamage(int dam);
uchar operational(uchar i);
uchar opCheck(uchar i);
uchar opCheckSR();
void redrawSidebar();
void drawOperations();
