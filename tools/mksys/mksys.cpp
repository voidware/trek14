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

/* program to convert intel HEX binary to TRS80 SYSTEM CAS */

#include <stdio.h>
#include <string.h>
#include "cutils.h"
#include <vector>

int verbose = 0;

const char* readline(FILE* fp)
{
    static char buf[256];
    int c;
    char* p = buf;
    while ((c = getc(fp)) != EOF && c != '\n') *p++ = c;
    *p = 0;

    if (p == buf && c == EOF) return 0; // over
    return buf;
}

int cas_write_header(FILE *out, char *name)
{
    // expect `name' to be uppercase 6 digit name or less

    int i, namelength;
    char htname[7];
    
    strcpy(htname, "      ");

    namelength = strlen(name);
    if (namelength > 6) namelength = 6;
    else if (namelength < 1) return 0;
    
    strncpy(htname, name, namelength);
        
    // emit leader
    for (i=0; i<256; i++) fputc(0x00, out);

    // emit header sequence
    fputc(0xA5, out);
    fputc(0x55, out);
        	
    // emit 6 character program name
    for (i=0; i<6; i++) fputc(htname[i], out);
        
    return -1;
}

void cas_write_block_header(FILE *out, int blocksize, unsigned short addr)
{
    unsigned char a;

    fputc(0x3c, out);	// escape char  
    a = blocksize;
    fputc(blocksize, out); // block length(256)
    a = addr&0xFF;
    fputc(a, out); // load address lo
    a = addr>>8;
    fputc(a, out); // load address hi      		
}

void cas_write_eof(FILE *out, unsigned short start_addr)
{
    //fprintf( stderr, "Start address
    fputc( 0x78, out);
    fputc( start_addr & 0xFF, out);
    fputc( start_addr >> 8, out);
}

void emit(FILE* fpout, unsigned char* buf, int n, int addr)
{
    if (!n) return;

    if (verbose)
        printf("emit block %04X size %04X\n", addr, n);

    int ck = addr + (addr>>8);
    
    cas_write_block_header(fpout, n, addr);
                
    while (n)
    {
        --n;
        fputc(*buf, fpout);
        ck += *buf++;
    }
    
    // and bogus checksum
    fputc(ck, fpout);
}

struct Block
{
    Block() { data = 0; }
    ~Block() { delete data; }

    int                 addr;
    int                 size;
    unsigned char*      data;
};

std::vector<Block*>     blocks;

Block* blockSup(int addr)
{
    // least block whose address >= addr
    
    Block* best = 0;
    for (int i = 0; i < blocks.size(); ++i)
    {
        Block* b = blocks[i];
        if (b->data)
        {
            if (b->addr >= addr && (!best || b->addr < best->addr))
                best = b;
        }
    }
    return best;
}

void emitFile(FILE* fpout)
{
    size_t n = blocks.size();
    if (!n) return; // bail

    // XXX
    cas_write_header(fpout, "TREK14");

    int startAddr = 0;
    bool more;
    int blockSize = 256;

    // combine blocks

    int addr = 0;
    Block* b = blockSup(addr);
    while (b)
    {
        int endAddr = b->addr + b->size;
        Block* b2 = blockSup(endAddr);
        if (!b2) break;
        
        if (b2->addr == endAddr && b->size < blockSize)
        {
            // combine
            int len = b->size + b2->size;
            if (len > blockSize) len = blockSize;
            int r = len - b->size;
            
            unsigned char* d = new unsigned char[len];
            memcpy(d, b->data, b->size);
            memcpy(d + b->size, b2->data, r);

            delete b->data;
            b->data = d;
            b->size = len;

            if (r == b2->size)
            {
                delete b2->data;
                b2->data = 0;
            }
            else
            {
                memmove(b2->data, b2->data + r, b2->size - r);
                b2->addr += r;
                b2->size -= r;
            }
                
            addr = 0; // start over
            b = blockSup(addr);
        }
        else
        {
            b = b2;
        }
    }

    do
    {
        more = false;
        Block* b = blockSup(0);
        
        if (b)
        {
            more = true;

            if (!startAddr)
                startAddr = b->addr;

            emit(fpout, b->data, b->size, b->addr);

            // throw
            delete b->data;
            b->data = 0;

        }
    } while (more);

    cas_write_eof(fpout, startAddr);
}

void processFile(FILE* fpin, FILE* fpout)
{
    const char* line;
    unsigned char buf[1024];

    while ((line = readline(fpin)) != 0)
    {
        if (!*line || *line != ':') continue; 
        ++line;

        Block* b = new Block;
        unsigned char* pb = buf;
        
        b->size = (u_hexv(line[0])<<4) + u_hexv(line[1]);
        line += 2;

        if (!b->size)
            break;
        
        b->addr = (u_hexv(line[0])<<12) + (u_hexv(line[1])<<8) + (u_hexv(line[2])<<4) + u_hexv(line[3]);
        line += 4;

        // ignore type
        int type = (u_hexv(line[0])<<4) + u_hexv(line[1]);
        line += 2;

        int m = b->size;
        while (m)
        {
            --m;
            int v = (u_hexv(line[0])<<4) + u_hexv(line[1]);           
            line += 2;
            *pb++ = v;
        }

        // copy data into the block
        b->data = new unsigned char[pb - buf];
        memcpy(b->data, buf, pb - buf);

        blocks.push_back(b);
    }

    emitFile(fpout);

    // delete blocks
    for (int i = 0; i < blocks.size(); ++i)
        delete blocks[i];
}

int main(int argc, char** argv)
{
    const char* infile = 0;
    const char* outfile = 0;
    for (int i = 1; i < argc; ++i)
    {
        if (argv[i][0] == '-')
        {
            if (!strcmp(argv[i], "-v")) verbose = 1;
        }
        else
        {
            if (!infile)
                infile = argv[i];
            else
                outfile = argv[i];
        }
    }

    if (!infile || !outfile)
    {
        printf("usage: %s [-v] <infile> <outfile>\n", argv[0]);
        return 1;
    }

    FILE* fp = fopen(infile, "r");
    if (fp)
    {
        FILE* fpout = fopen(outfile, "wb");
        if (fpout)
        {
            processFile(fp, fpout);
            fclose(fpout);
        }
        else
        {
            printf("can't open output file '%s'\n", outfile);
        }
        fclose(fp);
    }
    else
    {
        printf("can't open input file '%s'\n", infile);
    }
    
    return 0;
}

