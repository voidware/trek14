/*-
 * Copyright (c) 2001 voidware
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * 
 * 1. Redistributions of the source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 
 * 2. Any redistribution solely in binary form must conspicuously
 *    reproduce the following disclaimer in documentation provided with the
 *    binary redistribution.
 * 
 * THIS SOFTWARE IS PROVIDED ``AS IS'', WITHOUT ANY WARRANTIES, EXPRESS
 * OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  LICENSOR SHALL
 * NOT BE LIABLE FOR ANY LOSS OR DAMAGES RESULTING FROM THE USE OF THIS
 * SOFTWARE, EITHER ALONE OR IN COMBINATION WITH ANY OTHER SOFTWARE.
 * 
 */

#ifdef _WIN32
#include <windows.h>
#else
#include "defs.h"
#include "os.h"
#endif

#include "libc.h"

#ifdef _WIN32
usmint BASE_OpenConsoleOutput()
{
    return (usmint)GetStdHandle(STD_OUTPUT_HANDLE);
}

usmint BASE_OpenConsoleInput()
{
    return (usmint)GetStdHandle(STD_INPUT_HANDLE);
}

void BASE_Read(usmint h, void* buf, unsigned int amt, usmint * nr)
{
    DWORD n;
    ReadFile((HANDLE)h, buf, amt, &n, 0);

    // remove whitespace on end to simulate loss of newline
    while (n && ((char*)buf)[n-1] == 0xd || ((char*)buf)[n-1] == 0xa) --n;

    // newline is lost, so add it back
    if (n >= amt) n = amt-1;
    ((char*)buf)[n] = '\n';
    ++n;

    *nr = n;
}

void BASE_Write(usmint h, const void* buf,
                usmint amt, usmint* nw)
{
    WriteFile((HANDLE)h, buf, amt, nw, 0);
}

void* BASE_MemoryAllocate(unsigned int amt)
{
    return HeapAlloc(GetProcessHeap(), 0, amt);
}

void BASE_MemoryFree(void* p)
{
    HeapFree(GetProcessHeap(), 0, p);
}

char* BASE_CommandLine()
{
    return GetCommandLine();
}

usmint BASE_OpenFile(const char* filename, usmint flags)
{
    return 0;
}

void BASE_CloseFile(usmint h)
{
}
#else

// trs80


usmint BASE_OpenConsoleOutput()
{
    return 1;
}

usmint BASE_OpenConsoleInput()
{
    return 0;
}

void BASE_Read(usmint h, void* buf, usmint amt, usmint * nr)
{
    // using our own getline
    *nr = getline2(buf, amt);
}

void BASE_Write(usmint h, const void* buf,
                usmint amt, usmint* nw)
{
    const char* p = (const char*)buf;
    while (amt)
    {
        --amt;
        outchar(*p++);
    }
}

void* BASE_MemoryAllocate(unsigned int amt)
{
    return 0;
}

void BASE_MemoryFree(void* p)
{
}

char* BASE_CommandLine()
{
    return 0;
}

usmint BASE_OpenFile(const char* filename, usmint flags)
{
    return 0;
}

void BASE_CloseFile(usmint h)
{
}


#endif // WIN32




