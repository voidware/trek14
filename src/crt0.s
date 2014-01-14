;* Copyright (c) 2013 Voidware Ltd.
;*
;* Permission is hereby granted, free of charge, to any person obtaining a copy
;* of this software and associated documentation files (the "Software"), to
;* deal in the Software without restriction, including without limitation the
;* rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
;* sell copies of the Software, and to permit persons to whom the Software is
;* furnished to do so, subject to the following conditions:
;* 
;* The above copyright notice and this permission notice shall be included in
;* all copies or substantial portions of the Software.
;* 
;* THE SOFTWARE IS PROVIDED "AS IS," WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
;* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
;* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
;* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
;* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
;* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
;* IN THE SOFTWARE.



	.module crt0
	.globl	_main
        .globl	l__INITIALIZER
        .globl	s__INITIALIZER 
        .globl	s__INITIALIZED 

init:

        ;; save the original stack area 
        ld     (_exit+1),sp

        ;; find the top of memory for the new stack
        ;; first check the LDOS/TRDOS HIGH$ location (valid if DOS loaded)
        ;; If this is non-zero, use it.
        ;;
        ;; HIGH$ variable suggestion and info from Tony Duell. Thanks!
        ;; 
        ;; Otherwise see 48k, 32k, 16k has RAM by modifiying it.

        ld      hl,#0x4049      ;LDOS/TRSDOS HIGH$
        ld      e,(hl)
        inc     hl
        ld      d,(hl)
        ex      de,hl           ;hl = HIGH$
        ld      a,h
        or      a,l
        jr      NZ,00100$       ; high != 0, so use it

        ;; check 48 top of ram
        ld      hl,#0xffff       ;48k top
        ld      a,(hl)
        inc     a
        ld      (hl),a          ; top + 1 -> top
        sub     a,(hl)          ; changed?
        jr      Z,00100$        ; yes, hl is top

        ;; check 32k top of ram
        ld      hl,#0xbfff       ;32k top
        ld      a,(hl)
        inc     a
        ld      (hl),a          ; top + 1 -> top
        sub     a,(hl)          ; changed?
        jr      Z,00100$        ; yes, hl is top

        ;; otherwise assume 16k
        ld      hl,#0x7fff      
        
00100$:

        ld      sp,hl

        ;; Initialise global variables
        call    gsinit
	call	_main
        jp      _exit  
        
	;; Ordering of segments for the linker.
	.area	_CODE
        .area	_INITIALIZER
	.area   _GSINIT
	.area   _GSFINAL

	.area	_DATA
        .area	_INITIALIZED
	.area   _BSS
	.area   _HEAP

	.area   _CODE

_exit::
        ld sp, #0
        ret

	.area   _GSINIT
gsinit::

	ld	bc, #l__INITIALIZER
	ld	a, b
	or	a, c
	jr	Z, gsinit_next
	ld	de, #s__INITIALIZED
	ld	hl, #s__INITIALIZER
	ldir
gsinit_next:        

	.area   _GSFINAL
	ret

