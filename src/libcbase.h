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

usmint BASE_OpenConsoleOutput();
usmint BASE_OpenConsoleInput();
void BASE_Read(usmint h, void* buf, unsigned int amt, usmint*);
void BASE_Write(usmint h, const void* buf, usmint amt, usmint* nw);
void* BASE_MemoryAllocate(unsigned int amt);
void BASE_MemoryFree(void* p);
char* BASE_CommandLine();
usmint BASE_OpenFile(const char* path, usmint flags);
void BASE_CloseFile(usmint h);
