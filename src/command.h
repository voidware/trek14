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

void command();
void conn();
void phaserCommand();
void baseLine();
void endgame(uchar msg);

void docCommand();
void torpCommand();

void messageCode(uchar mc);

#define MSG_CODE_INSUFENERGY  0
#define MSG_CODE_NO_TARGET 1
#define MSG_CODE_DESTROYED 2
#define MSG_CODE_NO_TORPS 3
#define MSG_CODE_DOCKED 4
#define MSG_CODE_ENDGAME_RESIGN 5
#define MSG_CODE_ENDGAME_EXPIRE 6
#define MSG_CODE_ENDGAME_KILLED 7
#define MSG_CODE_ENDGAME_RELIEVED 8
#define MSG_CODE_RETURN_HQ 9


