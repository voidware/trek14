/**
 * Copyright (c) 2016 Voidware Ltd.
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
 * 
 * contact@voidware.com
 */

#include "defs.h"
#include "story.h"
#include "os.h"
#include "ent.h"

static const char* skipTerm(const char* s)
{
    uchar level = 0;
    char c;
    while ((c = *s) != 0)
    {
        if (c == '[') ++level;
        else if (c == ']')
        {
            if (!--level) break;
        }
        else if (c == '|')
        {
            if (!level) ++level;  // start 
            else if (level == 1) break;
        }
        ++s;
    }
    return s;
}

static void emitword(Story* st)
{
    // emit buffered word
    uchar i;
    for (i = 0; i < st->wordp; ++i) 
    {
        outchar(st->wordbuf[i]);
        ++st->hpos;
    }
    st->wordp = 0;
}

static void emitnl(Story* st)
{
    outchar('\n');
    st->hpos = 0;

    // and indent
    while (st->hpos < st->lmargin)
    {
        outchar(' ');
        ++st->hpos;
    }
}

static void emit(uchar c, Story* st)
{
    if (st->wrap)
    {
        if (c == ' ' || c == '\n')
        {
            if (st->hpos + st->wordp > st->wrap)
            {
                emitnl(st);
                emitword(st);
            }
            else
            {
                emitword(st);
                if (c == '\n') emitnl(st);
            }

            if (c == ' ')
            {
                outchar(c);
                ++st->hpos;
            }
        }
        else
        {
            st->wordbuf[st->wordp++] = c;
        }
    }
    else
    {
        if (c == '\n') emitnl(st);
        else 
        {
            ++st->hpos;
            outchar(c);
        }
    }
}

static void _story(const char* s, Story* st)
{
    char c;
    while ((c = *s) != 0)
    {
        if (c == '[')
        {
            // count alternative terms
            uchar n = 0;
            uchar t;
            const char* s1 = s;
            for (;;)
            {
                ++n;
                s1 = skipTerm(s1);
                if (!*s1 || *s1 == ']') break;
            }

            // choose term
            t = randn(n);  // [0,n-1]

            // skip t terms
            n -= t;
            while (t) {--t; s = skipTerm(s); }

            // process chosen term
            _story(s + 1, st);

            // skip remainder
            while (n) { --n; s = skipTerm(s); }

            if (!*s) break;
        }
        else if (c == '|' || c == ']')
        {
            break;
        }
        else if (c == '^')
        {
            // sub-story
            char v = -1;
            char i = -1;

            // first character is optional table index (a,b,c,...)
            // otherwise the number is global across all tables
            c = s[1];
            if (c >= 'a' && c <= 'z')
            {
                ++s;
                c = c - 'a';

                // select table
                if (c < DIM(st->sub))
                {
                    i = c;
                    if (s[1] == 'n')
                    {
                        s += 2;

                        // (eg "an" means any from table a)
                        // choose it
                        v = randn(st->subSize[i]);
                    }
                }
            }
            
            if (v < 0)
            {
                // select index within table
                v = 0;
                for (;;)
                {
                    c = *++s;
                    if (c >= '0' && c <= '9') v = v*10 + (c - '0');
                    else break;
                }
            }

            if (i < 0)
            {
                // find index across all tables
                for (i = 0; i < DIM(st->sub); ++i)
                {
                    if (v >= (char)st->subSize[i]) v -= st->subSize[i];
                    else
                        break;
                }
            }

            if (i >= 0)
            {
                if (v < (char)st->subSize[i])
                    _story(st->sub[i][v], st);
            }

            continue; // no ++
        }
        else
            emit(c, st);

        ++s;
    }
}

void story(const char* s, Story* st)
{
    _story(s, st);
    emitword(st);
}

void story0(const char* s)
{
    story(s, 0);
}




