;;
;;
;;    _    __        _      __                           
;;   | |  / /____   (_)____/ /_      __ ____ _ _____ ___ 
;;   | | / // __ \ / // __  /| | /| / // __ `// ___// _ \
;;   | |/ // /_/ // // /_/ / | |/ |/ // /_/ // /   /  __/
;;   |___/ \____//_/ \__,_/  |__/|__/ \__,_//_/    \___/ 
;;                                                       
;;  Copyright (©) Voidware 2014-2020.
;;
;;  Permission is hereby granted, free of charge, to any person obtaining a copy
;;  of this software and associated documentation files (the "Software"), to
;;  deal in the Software without restriction, including without limitation the
;;  rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
;;  sell copies of the Software, and to permit persons to whom the Software is
;;  furnished to do so, subject to the following conditions:
;; 
;;  The above copyright notice and this permission notice shall be included in
;;  all copies or substantial portions of the Software.
;; 
;;  THE SOFTWARE IS PROVIDED "AS IS," WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
;;  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
;;  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
;;  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
;;  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
;;  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
;;  IN THE SOFTWARE.
;; 
;;  contact@voidware.com


    .module crt0
    .globl  _main
    .globl  l__INITIALIZER
    .globl  s__INITIALIZER 
    .globl  s__INITIALIZED
    .globl  l__DATA
    .globl  s__DATA
    .globl  _CmdLine

init:

        ;; this will be enabled again after init
        di

        ;; save the original stack area 
        ld     (_exit+1),sp

        ;; Initialise global variables
        push    bc
        call    gsinit
        pop     bc
    
        ;; only valid for M4
        ld      (_CmdLine),bc

        call    _main
        jp      _exit  
        
        ;; Ordering of segments for the linker.
        .area   _CODE
        .area   _INITIALIZER
        .area   _GSINIT
        .area   _GSFINAL

        .area   _DATA
        .area   _INITIALIZED
        .area   _BSS
        .area   _HEAP

        .area   _CODE

_exit::
        ld sp, #0                   ; restores stack to original
        ei
        ret

        .area   _GSINIT
gsinit::

        ld      hl, #s__DATA
        ld      bc, #l__DATA
        
.initbss:
        ld      a, b
        or      c
        jr      Z, .initz
        ld      (hl),#0
        inc     hl
        dec     bc
        jp      .initbss
        
.initz: 
        ld      bc, #l__INITIALIZER
        ld      a, b
        or      a, c
        jr      Z, gsinit_next
        ld      de, #s__INITIALIZED
        ld      hl, #s__INITIALIZER
        ldir
gsinit_next:        

        .area   _GSFINAL
        ret

