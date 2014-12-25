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
#include "libc.h"
#include "sound.h"

#define BASE_TSTATES 221750L

// frequencies for notes.
// "A" above middle C is 440Hz.
// f = 440 * (2^(k/12)) where k is then number of tones above or below
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

// next octave
#define NOTE_C1         523.2511306011972693557
#define NOTE_C1S        554.3652619537441924975728
#define NOTE_D1         587.329535834815120525566
#define NOTE_D1S        622.2539674441618214727431
#define NOTE_E1         659.2551138257398594716832
#define NOTE_F1         698.4564628660077688907502
#define NOTE_F1S        739.9888454232687978673908
#define NOTE_G1         783.9908719634985881713988
#define NOTE_G1S        830.6093951598902770448834
#define NOTE_A1         880.000
#define NOTE_A1S        932.327523036179832814406
#define NOTE_B1         987.766602512248223661509

#define RND(_x)     ((int)((_x) + 0.5))
#define TSTATE(_n)  RND(BASE_TSTATES/_n - 29.5)

typedef struct
{
    char                _note;
    unsigned int        _freq;
    unsigned int        _tstates;
} Note;

static const Note notes[] =
{
    { 'C', RND(NOTE_C), TSTATE(NOTE_C) },
    { 'C', RND(NOTE_CS), TSTATE(NOTE_CS) },
    { 'D', RND(NOTE_D), TSTATE(NOTE_D) },
    { 'D', RND(NOTE_DS), TSTATE(NOTE_DS) },
    { 'E', RND(NOTE_E), TSTATE(NOTE_E) },
    { 'F', RND(NOTE_F), TSTATE(NOTE_F) },
    { 'F', RND(NOTE_FS), TSTATE(NOTE_FS) },
    { 'G', RND(NOTE_G), TSTATE(NOTE_G) },
    { 'G', RND(NOTE_GS), TSTATE(NOTE_GS) },
    { 'A', RND(NOTE_A), TSTATE(NOTE_A) },
    { 'A', RND(NOTE_AS), TSTATE(NOTE_AS) },
    { 'B', RND(NOTE_B), TSTATE(NOTE_B) },

    { 'C', RND(NOTE_C1), TSTATE(NOTE_C1) },
    { 'C', RND(NOTE_C1S), TSTATE(NOTE_C1S) },
    { 'D', RND(NOTE_D1), TSTATE(NOTE_D1) },
    { 'D', RND(NOTE_D1S), TSTATE(NOTE_D1S) },
    { 'E', RND(NOTE_E1), TSTATE(NOTE_E1) },
    { 'F', RND(NOTE_F1), TSTATE(NOTE_F1) },
    { 'F', RND(NOTE_F1S), TSTATE(NOTE_F1S) },
    { 'G', RND(NOTE_G1), TSTATE(NOTE_G1) },
    { 'G', RND(NOTE_G1S), TSTATE(NOTE_G1S) },
    { 'A', RND(NOTE_A1), TSTATE(NOTE_A1) },
    { 'A', RND(NOTE_A1S), TSTATE(NOTE_A1S) },
    { 'B', RND(NOTE_B1), TSTATE(NOTE_B1) },
};

// play melodies. eg:

#if 0
static int divs(int a, uint b)
{
    // bogus routine to divide using only unsigned int
    return a >= 0 ? (int)(((uint)a)/b) : -(int)(((uint)(-a))/b);
}
#endif

void playNotes(const char* m)
{
    // [A-G][#b]?[0-9]*
    // u = up octave, d = down octave
    // t sets initial tempo divider

    const Note* octave = notes;
    const Note* n = 0;
    uchar dt = 2;
    uchar dt2 = 0;
    uchar tempo = 12;
    
    for (;;)
    {
        char c = *m++;

        if (c >= 'A' && c <= 'G' || !c)
        {
            if (n)
            {
                if (dt2)
                    dt = dt2;
            
                dt2 = 0;
                bit_sound(dt*n->_freq/tempo, n->_tstates);
            }

            if (!c) break;
            for (n = octave; n->_note != c; ++n) ;
        }
        if (c == '#')
            ++n;
        if (c == 'b')
            --n;
        if (c == 'u')
            octave += 12;
        if (c == 'd')
            octave -= 12;
        if (isdigit(c))
            dt2 = 10*dt2 + (c - '0');
        if (c == 't')
        {
            tempo = dt2;
            dt2 = 0;
        }
    }
}


