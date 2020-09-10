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

#include <ctype.h>
#include "defs.h"
#include "os.h"
#include "sound.h"

#define BASE_TSTATES_M1 221750L
#define BASE_TSTATES_M3 257230L

// frequencies for notes.
// "A" above middle C is 440Hz.
// f = 440 * (2^(k/12)) where k is the number of tones above or below
// so middle "C" is 9 tones down and therefore Fc = 440*2^(-9/12) = 261.63

// standard octave
#define NOTE_C  	261.625565290
#define NOTE_CS 	277.182631135    
#define NOTE_D  	293.664768100      
#define NOTE_DS 	311.126983881    
#define NOTE_E          329.627557039   
#define NOTE_F  	349.228231549      
#define NOTE_FS 	369.994422674     
#define NOTE_G  	391.995436072       
#define NOTE_GS 	415.304697513       
#define NOTE_A  	440.000000000      
#define NOTE_AS 	466.163761616   
#define NOTE_B  	493.883301378     

#define RND(_x)     ((int)((_x) + 0.5))
#define TSTATE1(_n)  RND(BASE_TSTATES_M1/_n)
#define TSTATE3(_n)  RND(BASE_TSTATES_M3/_n)

typedef struct
{
    char                _note;
    unsigned int        _freq;
    unsigned int        _tstatesM1;
    unsigned int        _tstatesM3;
} Note;

static const Note notes[] =
{
    { 'C', RND(NOTE_C), TSTATE1(NOTE_C), TSTATE3(NOTE_C) },
    { 'C', RND(NOTE_CS), TSTATE1(NOTE_CS), TSTATE3(NOTE_CS) },
    { 'D', RND(NOTE_D), TSTATE1(NOTE_D), TSTATE3(NOTE_D) },
    { 'D', RND(NOTE_DS), TSTATE1(NOTE_DS), TSTATE3(NOTE_DS) },
    { 'E', RND(NOTE_E), TSTATE1(NOTE_E), TSTATE3(NOTE_E) },
    { 'F', RND(NOTE_F), TSTATE1(NOTE_F), TSTATE3(NOTE_F) },
    { 'F', RND(NOTE_FS), TSTATE1(NOTE_FS), TSTATE3(NOTE_FS) },
    { 'G', RND(NOTE_G), TSTATE1(NOTE_G), TSTATE3(NOTE_G) },
    { 'G', RND(NOTE_GS), TSTATE1(NOTE_GS), TSTATE3(NOTE_GS) },
    { 'A', RND(NOTE_A), TSTATE1(NOTE_A), TSTATE3(NOTE_A) },
    { 'A', RND(NOTE_AS), TSTATE1(NOTE_AS), TSTATE3(NOTE_AS) },
    { 'B', RND(NOTE_B), TSTATE1(NOTE_B), TSTATE3(NOTE_B) },
};

// play melodies. eg:

void playVictory()
{
    playNotes("16tF6Eb+9D3C2Bb-AAb10");
}

void playNotes(const char* m)
{
    // [A-G][#b]?[0-9]*
    // + = up octave, - = down octave
    // t sets initial tempo divider

    const Note* n = 0;
    uchar dt = 2;
    uchar dt2 = 0;
    uint tempo = 12;
    char u = 0;

    for (;;)
    {
        char c = *m++;

        if (c >= 'A' && c <= 'G' || !c)
        {
            if (n)
            {
                uint a, b;
                char u2;
                
                if (dt2)
                {
                    dt = dt2;
                    dt2 = 0;
                }

                a = dt*n->_freq;
                b = TRSModel == 1 ? n->_tstatesM1 : n->_tstatesM3;
                u2 = u;
                while (u2 > 0)
                {
                    --u2;
                    a <<= 1;
                    b >>= 1;
                }
                while (u2 < 0)
                {
                    ++u2;
                    a >>= 1;
                    b <<= 1;
                }
                bit_sound(a/tempo, b - 30);
            }

            if (!c) break;
            for (n = notes; n->_note != c; ++n) ;
        }
        if (c == '#')
            ++n;
        if (c == 'b')
            --n;
        if (c == '+')
            ++u;
        if (c == '-')
            --u;
        if (isdigit(c))
            dt2 = 10*dt2 + (c - '0');
        if (c == 't')
        {
            tempo = dt2;
            dt2 = 0;
        }
    }
}


