/**
 * Copyright (c) 2012 Voidware Ltd.
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

// this is the min-clib that we're using.
// much of it is commented out to save space. if we need anything
// uncomment it back in!

#include "libc.h"

#define LIBC_alloc      malloc
#define LIBC_free       free

#if 0
void* memcpy(void* dst, const void* src, size_t n)
{
    char* p = (char*)dst;
    const char* q = (const char*)src;
    while (n) 
    {
        --n;
        *p++ = *q++;
    }
    return dst;
}
#endif

void* memmove(void* dst, const void* src, size_t n)
{
    char* p = (char*)dst;
    const char* q = (const char*)src;
    if (dst < src) 
    {
        while (n) 
        {
            --n;
            *p++ = *q++;
        }
    }
    else 
    {
        p += n;
        q += n;
        while (n) 
        {
            --n;
            *--p = *--q;
        }
    }
    return dst;
}

void* memset(void* ptr, smint v, size_t n)
{
    char* p = (char*)ptr;
    while (n) 
    {
        --n;
        *p++ = v;
    }
    return ptr;
}


#if 0
void* memsetInt(void* ptr, int v, size_t n)
{
    // use `int' at a time rather than char. 
    unsigned int* p = (unsigned int*)ptr;
    while (n) 
    {
        --n;
        *p++ = v;
    }
    return ptr;
}
#endif

size_t strlen(const char* s)
{
    size_t l = 0;
    while (*s) 
    {
        ++l;
        ++s;
    }
    return l;
}

#if 0
char* strcpy(char* dst, const char* src)
{
    char* p = dst;
    while ((*p = *src) != 0) 
    {
        ++p;
        ++src;
    }
    return dst;
}

char* strcat(char* dst, const char* src)
{
    char* p = dst;
    while (*p) ++p;
    while ((*p = *src) != 0) 
    {
        ++p;
        ++src;
    }
    return dst;
}

smint strcmp(const char * s1, const char * s2)
{
    // Plauger
    for(; *s1 == *s2; ++s1, ++s2)
        if(*s1 == 0)
            return 0;
    return *s2 - *s1;
}

char* strchr(const char* s, smint c)
{
    while (*s) 
    {
        if (*s == c) return (char*)s;  /* XX */
        ++s;
    }
    return 0;
}

smint stricmp(const char* s1, const char* s2)
{
    smint v;
    for (;;) 
    {
        char c1 = *s1;
        char c2 = *s2;

        if (isupper(c1)) c1 = _tolower(c1);
        if (isupper(c2)) c2 = _tolower(c2);
        v = c2 - c1;
        if (v || !*s1) break;
        ++s1;
        ++s2;
    }
    return v;
}

char* strncat(char* dst, const char* src, size_t n)
{
    char* p = dst;
    while (*p) ++p;

    while (n) 
    {
        --n;
        if ((*p = *src) == 0) break;
        ++p;
        ++src;
    }
    return dst;    
}

smint strncmp(const char* s1, const char* s2, size_t n)
{
    smint v = 0;
    while (n) 
    {
        --n;
        v = (*s2 - *s1);
        if (v || !*s1) break;
        ++s1;
        ++s2;
    }
    return v;
}

smint strnicmp(const char* s1, const char* s2, size_t n)
{
    smint v = 0;
    while (n) 
    {
        char c1 = *s1;
        char c2 = *s2;

        --n;
        
        if (isupper(c1)) c1 = _tolower(c1);
        if (isupper(c2)) c2 = _tolower(c2);
        
        v = (c2 - c1);
        if (v || !*s1) break;
        ++s1;
        ++s2;
    }
    return v;
}

char* strncpy(char* dst, const char* src, size_t n)
{
    char* p = dst;
    while (n) 
    {
        --n;
        if ((*p = *src) == 0) break;
        ++p;
        ++src;
    }
    return dst;    
}

char* strrchr(const char* s, smint c)
{
    const char* p = s;
    while (*p) ++p;
    while (p != s) 
        if (*--p == c) return (char*)p;  /* XX */
    
    return 0;
}

char* strstr(const char* s, const char* pat)
{
    /* use brute force */
    size_t lp = strlen(pat);
    size_t ls = strlen(s);

    if (lp <= ls) 
    {
        const char* e = s + ls - lp;
        while (s <= e) 
        {
            if (!strcmp(s, pat)) return (char*)s; /* XX */
            ++s;
        }
    }
    return 0;
}
#endif

typedef void (*StreamOutFn)(void*, char);
typedef int (*StreamInFn)(void*);

#define STDIO_BUFSIZE           40
#define STDIO_MAXHANDLES        2
#define TEXT_MODE(_x)           ((_x) & 1)
#define SET_TEXT_MODE(_x)       ((_x) |= 1)

static StreamRec ioTable[STDIO_MAXHANDLES];

static void bufStreamOutFn(void* ctx, char c)
{
    char** cAddr = (char**)ctx;
    **cAddr = c;
    ++(*cAddr);
}

static usmint findFreeStream()
{
    usmint i;
    for (i = 0; i < STDIO_MAXHANDLES; ++i)
        if (!ioTable[i].h_) return i;
    
    return -1;
}

static void openStream(usmint index, usmint flags)
{
    /* Keep specific buffers for stdin and stdout */
    static char stdinBuf[STDIO_BUFSIZE];
    static char stdoutBuf[STDIO_BUFSIZE];

    StreamRec* sr = ioTable + index;
    if (!index) 
    {
        sr->buf_ = stdinBuf;
        sr->h_ = BASE_OpenConsoleInput();
    }
    else if (index == 1) 
    {
        sr->buf_ = stdoutBuf;
        sr->h_ = BASE_OpenConsoleOutput();
    }
    else 
    {
        sr->buf_ = 0; // LIBC_alloc(STDIO_BUFSIZE);
        sr->fpos_ = 0;
    }
    sr->pos_ = sr->buf_;
    sr->end_ = sr->buf_;
    sr->flags_ = flags;
}

#if 0
static void closeStream(StreamRec* sr)
{
    if (sr->ioIndex_ > 1)
    {
        if (sr->buf_)
        {
            //LIBC_free(sr->buf_);
            sr->buf_ = 0;
        }

        if (sr->h_)
        {
            BASE_CloseFile(sr->h_);
            sr->h_ = 0; // mark as free
        }
    }
}
#endif

static void _initIoTable()
{
    usmint i;
    usmint flags = 0;

    for (i = 0; i < STDIO_MAXHANDLES; ++i) 
    {
        StreamRec* sr = ioTable + i;
        sr->h_ = 0;
        sr->fpos_ = 0;
        sr->ioIndex_ = i;
        sr->buf_ = 0;
    }

    SET_TEXT_MODE(flags);
    openStream(0, flags);
    openStream(1, 0);
}

static smint flushStream(StreamRec* sr)
{
    usmint n = 0;
    if (sr->buf_) 
    {
        n = sr->pos_ - sr->buf_;
        if (n) 
        {
            usmint d;
            BASE_Write(sr->h_, sr->buf_, n, &d);
            sr->pos_ = sr->buf_;
        }
    }
    return 0;
}

static void streamWriteFn(void* ctx, char c)
{
    usmint n;
    StreamRec* sr = (StreamRec*)ctx;
    if (sr->buf_) 
    {
        *sr->pos_++ = c;
        n = sr->pos_ - sr->buf_;
        if (c == '\n' || n == STDIO_BUFSIZE) 
            flushStream(sr);
    }
}

static void streamFill(StreamRec* sr)
{
    if (sr->pos_ == sr->end_) 
    {
        /* read another bufsize */
        usmint m;
        BASE_Read(sr->h_, sr->buf_, STDIO_BUFSIZE, &m);

        sr->pos_ = sr->buf_;
        sr->end_ = sr->buf_ + m;
    }
}

static void streamUnreadFn(void* ctx)
{
    /* backup 1 character. */
    StreamRec* sr = (StreamRec*)ctx;
    if (sr->buf_) 
        if (sr->pos_ > sr->buf_) --sr->pos_;
}

static int streamReadFn(void* ctx)
{
    StreamRec* sr = (StreamRec*)ctx;
    int c = 0;
    if (sr->buf_) 
    {
        streamFill(sr);
        if (sr->pos_ == sr->end_) return EOF;

        c = *sr->pos_++;

#if 0
        if (c == 0xd && TEXT_MODE(sr->flags_)) 
        {
            /* ignore character */
            streamFill(sr);
            if (sr->pos_ == sr->end_) return EOF;            
            c = *sr->pos_++;            
        }
#endif
    }
    return c;
}

#define FLAGS_VALUE_NEGATIVE    1
#define FLAGS_ALIGN_LEFT        2
#define FLAGS_PREFIX_SIGN       4
#define FLAGS_PREFIX_ZEROS      8


static usmint streamInt(StreamOutFn sf, void* ctx,
                       unsigned int val,
                       smint width,
                       usmint flags)
{
    static const unsigned int powerTable[] = 
    {
#ifdef _WIN32
        1000000000,
        100000000,
        10000000,
        1000000,
        100000,
#endif
        10000,
        1000,
        100,
        10,
        1
    };

    usmint n = 0;
    usmint i;
    usmint zeros;

    if (flags & FLAGS_VALUE_NEGATIVE) 
    {
        (*sf)(ctx, '-');
        ++n;
        --width;
        val = (unsigned int)(-(int)val);
    }
    else if (flags & FLAGS_PREFIX_SIGN) 
    {
        (*sf)(ctx, '+');
        ++n;    
        --width;    
    }

    zeros = 0;
    if ((flags & FLAGS_PREFIX_ZEROS) && !(flags & FLAGS_ALIGN_LEFT)) 
    {
        while (width > 10) 
        {
            (*sf)(ctx, '0');
            --width;
            ++n;
        }
    }
    else 
        width = 1;

    for (i = 0; i < DIM(powerTable); ++i) 
    {
        if (val >= powerTable[i]) 
        {
            unsigned int t = val/powerTable[i];
            (*sf)(ctx, t + '0');
            ++n;
            val = val - (t * powerTable[i]);
            zeros = 1;
        }
        else 
        {
            if (DIM(powerTable)-i == width) zeros = 1;
            if (zeros) 
            {
                (*sf)(ctx, '0');            
                ++n;
            }
        }
    }
    return n;
}

static usmint _doFormat(StreamOutFn sf, void* ctx, const char* fmt, va_list args)
{
    const char* p = fmt;
    usmint n = 0;
    while (*p) 
    {
        if (*p == '%') 
        {
            smint width = -1;
            smint prec = -1;
            usmint flags = 0;
            usmint t;
            char numBuf[20];
            usmint l;
            char* np;
            smint r;
            usmint emitNumBuf = 0;

            ++p;

            for (;;) 
            {
                t = flags;

                if (*p == '-') 
                    flags |= FLAGS_ALIGN_LEFT;
                else if (*p == '+') 
                    flags |= FLAGS_PREFIX_SIGN;
                else if (*p == '0') 
                    flags |= FLAGS_PREFIX_ZEROS;

                if (t == flags) break;
                ++p;
            }

            /* INCOMPATIBLE: not supporting `*' printf format */

            if (isdigit(*p)) 
            {
                width = 0;
                do 
                {
                    width = width*10 + (*p - '0');
                    ++p;
                } while (isdigit(*p));
            }

            if (*p == '.') 
            {
                ++p;
                prec = 0;
                while (isdigit(*p)) 
                {
                    prec = prec*10 + (*p - '0');
                    ++p;
                }
            }

            /* INCOMPATIBLE: cases not supported,
             * S: wide strings
             * C: wide characters
             * i: implemented same as `d' 
             */

            if (*p == 's') 
            {
                const char* s = va_arg(args, const char*);
                l = strlen(s);
                r = width - l;

                if ((flags & FLAGS_ALIGN_LEFT) == 0) 
                {
                    while (r > 0) 
                    {
                        (*sf)(ctx, ' ');
                        ++n;
                        --r;
                     }
                }

                while (*s) 
                {
                    (*sf)(ctx, *s);                    
                    ++s;
                    ++n;
                }

                while (r > 0) 
                {
                    (*sf)(ctx, ' ');
                    ++n;
                    --r;
                }
            }
            else if (*p == 'd' || *p == 'u')
            {
                int val = va_arg(args, int);
                np = numBuf;

                /* INCOMPATIBLE: what sort of person wants more than
                 * 32 leading zeros?
                 */
                if (width > (smint)sizeof(numBuf)) width = sizeof(numBuf);

                l = streamInt(bufStreamOutFn, &np,
                              (unsigned int)val, width,
                              (*p == 'd' && (val < 0)) | flags);
                
                emitNumBuf = 1;
            }
            else if (*p == 'x')
            {
                // XX only do 4 digits
                unsigned int val = va_arg(args, unsigned int);             
                smint i;
                np = numBuf + 4;
                *np = 0;
                for (i = 0; i < 4; ++i)
                {
                    usmint v = val & 0x0f;
                    --np;
                    if (v < 10)
                        *np = '0' + v;
                    else
                        *np = v - 10 + 'A';
                    
                    val >>= 4;
                }
                
                l = i; // length
                emitNumBuf = 1;
            }
            else 
            {
                (*sf)(ctx, *p);
                ++n;
            }

            if (emitNumBuf) 
            {
                emitNumBuf = 0;

                r = width - l;

                if ((flags & FLAGS_ALIGN_LEFT) == 0) 
                {
                    while (r > 0) 
                    {
                        (*sf)(ctx, ' ');
                        ++n;
                        --r;
                    }
                }

                np = numBuf;
                while (l) 
                {
                    --l;
                    (*sf)(ctx, *np);
                    ++np;
                    ++n;
                }

                while (r > 0) 
                {
                    --r;
                    (*sf)(ctx, ' ');
                    ++n;
                }
            }
        }
        else 
        {
            (*sf)(ctx, *p);
            ++n;
        }
        ++p;
    }
    return n;
}

static smint _doFormatIn(StreamInFn sf, void* ctx,
                         const char* fmt, va_list args)
{
    const char* p = fmt;
    smint n = 0;
    int c;

    c = (*sf)(ctx);
    if (c == EOF) return EOF;

    while (*p) 
    {
        if (*p == '%') 
        {
            int width;

            /* INCOMPATIBLE: not supporting `*' input format */

            ++p;
            if (isdigit(*p)) 
            {
                width = 0;
                do {
                    width = width*10 + (*p - '0');
                    ++p;
                } while (isdigit(*p));
            }
            else width = -1;

            /* INCOMPATIBLE: cases not supported,
             * S: wide strings
             * C: wide characters
             */
            if (*p == 's') 
            {
                /* string */
                char* s = va_arg(args, char*);
               
                while (c != EOF && !isspace(c) && width) 
                {
                    *s++ = c;
                    c = (*sf)(ctx);
                    --width;
                }
                *s = 0;
            }
            else if (*p == 'c') 
            {
                /* single character */
                char* s = va_arg(args, char*);
                *s = c;
            }
            else if (*p == 'd') 
            {
                /* Decimal integer */
                int* vp = va_arg(args, int*);

                usmint neg = 0;
                int val = 0;
                if (c == '-') 
                {
                    neg = 1;
                    c = (*sf)(ctx);
                }
                while (isdigit(c)) 
                {
                    val = (val * 10) + (c - '0');
                    c = (*sf)(ctx);
                }
                
                *vp = neg ? -val : val;
            }
            else if (*p == 'u') 
            {
                /* unsigned Decimal integer */
                unsigned int* vp = va_arg(args, unsigned int*);
                unsigned int val = 0;

                while (isdigit(c)) 
                {
                    val = (val * 10) + (c - '0');
                    c = (*sf)(ctx);
                }
                *vp = val;
            }
            else 
            {
                if (c != *p) break;
                c = (*sf)(ctx);            
            }
            ++p;
            ++n;
        }
        else if (isspace(*p)) 
        {
            /* eat all spaces in format */
            do ++p; while (isspace(*p));

            /* eat all spaces in input */
            while (isspace(c)) 
                c = (*sf)(ctx);
        }
        else 
        {
            if (c != *p) break;
            c = (*sf)(ctx);            
            ++p;
            ++n;
        }
    }
    return n;
}

smint sprintf(char* buf, const char* fmt, ...)
{
    va_list args;
    usmint n;

    va_start(args, fmt);
    n = _doFormat(bufStreamOutFn, &buf, fmt, args);
    va_end(args);

    return (smint)n;
}

smint printf(const char* fmt, ...)
{
    va_list args;
    usmint n;

    va_start(args, fmt);
    n = _doFormat(streamWriteFn, stdout, fmt, args);
    va_end(args);
    return (smint)n;
}

#if 0
int fprintf(FILE* fp, const char * fmt, ...)
{
    va_list args;
    int n;

    va_start(args, fmt);
    n = _doFormat(streamWriteFn, fp, fmt, args);
    va_end(args);
    return n;
}
#endif

smint vfprintf(FILE *fp, const char* fmt, va_list args)
{
    return _doFormat(streamWriteFn, fp, fmt, args);
}

smint vprintf(const char* fmt, va_list args)
{
    return vfprintf(stdout, fmt, args);
}

smint vsprintf(char* buf, const char* fmt, va_list args)
{
    return _doFormat(bufStreamOutFn, &buf, fmt, args);
}

int fputc(int c, FILE * fp)
{
    streamWriteFn(fp, c);
    return c;
}

int fputs(const char* s, FILE* fp)
{
    while (*s) 
    {
        streamWriteFn(fp, *s);
        ++s;
    }
    return 0;
}

int putc(int c, FILE* fp)
{
    return fputc(c, fp);
}

int putchar(int c)
{
    return putc(c, stdout);
}

int puts(const char* s)
{
    return fputs(s, stdout);
}

#if 0
smint fflush(FILE* fp)
{
    return flushStream(fp);
}
#endif

void flush()
{
    flushStream(stdout);
}


#if 0
size_t fwrite(const void* data, size_t size, size_t count, FILE* fp)
{
    size_t nWritten = 0;
    const char* p = (const char*)data;

    /* INCOMPATIBLE: if size*count > size_t tough! */
    size_t n = size * count;
    if (fp->buf_) {
        while (n) {
            int r;

            r = STDIO_BUFSIZE - (fp->pos_ - fp->buf_);
            if (r > n) r = n;
            memcpy(fp->pos_, p, r);
            fp->pos_ += r;
            n -= r;
            p += r;
            flushStream(fp);
            nWritten += r;
        }
    }
    return nWritten/size;
}

smint fscanf(FILE* fp, const char* fmt, ...)
{
    int n;
    va_list args;
    va_start(args, fmt);

    n = _doFormatIn(streamReadFn, fp, fmt, args);
    va_end(args);
    return n;
}
#endif

smint scanf(const char* fmt, ...)
{
    smint n;
    va_list args;
    va_start(args, fmt);

    n = _doFormatIn(streamReadFn, stdin, fmt, args);
    va_end(args);
    return n;
}

int getchar()
{
    return getc(stdin);
}

int getc(FILE* fp)
{
    return streamReadFn(fp);
}

#if 0
// file implementation
FILE* fopen(const char *name, const char *mode)
{
    FILE* fh = 0;

    int h = findFreeStream();
    if (h > 0)
    {
        int flags = 0;
        int osh = BASE_OpenFile(name, flags);
        if (osh > 0)
        {
            openStream(h, flags);
            fh = ioTable + h;
            fh->h_ = osh;

            // text mode?
            if (mode[strlen(mode)-1] != 'b')
            {
                SET_TEXT_MODE(fh->flags_);
            }
        }
    }
    return fh;
}

int fclose(FILE* fp)
{
    closeStream(fp);
    return 0;
}
#endif


void libcInit()
{
    char* cl;
        
    //mallocinit();
    _initIoTable();

#if 0
    cl = BASE_CommandLine();

    if (cl)
    {
        int argc;
        char** argv;
        const char* p;
        const char* q;
        char* r;
        int i;

        /* because we dont know how many args or how big the strings
         * are, need to malloc space. if i can instead assume a limit,
         * then we can have a static buffer. it would be nice to not
         * call malloc here because this is the only linkage to malloc
         * (unless used by the app).
         *
         * assuming a limit always leads to trouble though: remember dos?
         */
        argc = 0;
        p = cl;
        while (*p) 
        {
            while (isspace(*p)) ++p;
            if (!*p) break;
        
            ++argc;
            while (*p && !isspace(*p)) ++p;
        }

        argv = (char**)LIBC_alloc(argc * sizeof(char*));

        p = cl;
        i = 0;
        while (*p) {
            while (isspace(*p)) ++p;
            if (!*p) break;
            q = p;
            while (*q && !isspace(*q)) ++q;

            r = argv[i] = (char*)LIBC_alloc(q - p + 1);
            while (p != q) *r++ = *p++;
            *r = 0;
            ++i;
        }
    }
#endif
}
