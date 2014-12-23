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

#ifndef __cutils_h__
#define __cutils_h__

#define DIM(_x)  (sizeof(_x)/sizeof(_x[0]))

// define our own set here
#define u__tolower(_c)    ( (_c)-'A'+'a' )
#define u__toupper(_c)    ( (_c)-'a'+'A' )

#define u_tolower(_c)    (u_isupper(_c) ? u__tolower(_c) : (_c))
#define u_toupper(_c)    (u_islower(_c) ? u__toupper(_c) : (_c))

#define u_isalpha(_c)  (u_isupper(_c) || u_islower(_c))
#define u_isupper(_c)  (((_c) >= 'A') & ((_c) <= 'Z'))
#define u_islower(_c)  (((_c) >= 'a') & ((_c) <= 'z'))
#define u_isdigit(_c)  (((_c) >= '0') & ((_c) <= '9'))
#define u_isspace(_c)  (((_c) == ' ') | ((_c) == '\t') | ((_c) == '\n') | ((_c == '\r')))
#define u_isalnum(_c)  (u_isalpha(_c) || u_isdigit(_c))
#define u_isprint(_c)  (((_c) >= 0x20) & ((_c) <= 127))

#define u_ishex(_c)    (u_isdigit(_c) || ((_c) >= 'A' && (_c) <= 'F') || ((_c) >= 'a' && (_c) <= 'f'))

#define u_hexv(_c) ((int)(u_isdigit(_c) ? (_c) - '0' :  (((_c) >= 'A' && (_c) <= 'F') ? (_c) - 'A' + 10 :  (((_c) >= 'a' && (_c) <= 'f') ? (_c) - 'a' + 10 : 0))))

#define u_hex(_v) ((_v) < 10 ? '0' + (_v) : 'A' - 10 + (_v))


inline int u_stricmp(const char* s1, const char* s2)
{
    int v;
    for (;;) 
    {
        int c1 = u_tolower(*s1);
        int c2 = u_tolower(*s2);
        
        v = c2 - c1;
        if (v || !*s1) break;
        ++s1;
        ++s2;
    }
    return v;
}

inline int u_strnicmp(const char* s1, const char* s2, size_t n)
{
    int v = 0;
    while (n) 
    {
        --n;

        int c1 = u_tolower(*s1);
        int c2 = u_tolower(*s2);

        v = (c2 - c1);
        if (v || !*s1) break;
        ++s1;
        ++s2;
    }
    return v;
}

inline char* u_strchr(const char* s, int c)
{
    while (*s) 
    {
        if (*s == c) return (char*)s;  /* XX */
        ++s;
    }
    return 0;
}


#define c_abs(_x)  ((_x) >= 0 ? (_x) : -(_x))

#define u_max(_a, _b) ((_a) >= (_b) ? (_a) : (_b))
#define u_min(_a, _b) ((_a) <= (_b) ? (_a) : (_b))


#endif // __cutils_h__
