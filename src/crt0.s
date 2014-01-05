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

        ld     (_exit+1),sp
        
	;; Stack at the top of memory.
	ld	sp,#0xffff      ;48k

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

