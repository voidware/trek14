;; whatever

        .globl	_useSVC

        .area   _CODE

sndbit_mask     .equ  3
sndbit_port     .equ  255


__bit_close::
          xor  a                ; will clear wide mode on model I also
          out  (sndbit_port),a

__rti::
          ei
          ret

_bit_soundi::
        ;;  bit_sound(duration, frequency)
        ;;  disables interrupts
        di
        pop   hl
        exx
        call _bit_sound
        exx
        push  hl
        jp   __rti

        ;;  bit_sound(duration, frequency)
        ;; NB: ASSUME interrupts disabled
_bit_sound::
          pop bc
          pop de                ; duration
          pop hl                ; freq
          push hl
          push de
          push bc
        
__beeper4::
        ld    a,(_useSVC)
        or    a
        jr    z,__beeper
        sla   l
        rl    h         ; hl*2 for M4 (approx)

        ;; HL=freq, DE=duration
        ;; preserve the wide-char bits for model I
        ;; NB: ASSUME interrupts disabled
__beeper::
          in   a,(#0xff)      ; current wide status (model I) inverted
          cpl                 ; flip. Thanks to gp2000 
          and  #0x40
          rrca
          rrca
          rrca                ; wide bit into bit 3 (other models ignored)
          or   a,#1           ; a=1

          inc  h
          inc  l
          
          inc   d
          inc   e

          ; first phase
.b2:
          out  (sndbit_port),a                  ;t11/13/12, a=1
          inc  a                                ;t4/6/5,  a=2
          ld   b,l                              ;t4/6/5
          ld   c,h                              ;t4/6/5
                                        ;total  23,31,27
.b1:
          djnz   .b1                            ;t13/8  15/10 14/9
          dec    c                              ;t4/6/5
          jp    nz,.b1                          ;t10/12/11
                                        ;total= f*13+(f/256+1)*22
                                        ;total= f*15+(f/256+1)*28
                                        ;total= f*14+(f/256+1)*25
                                        ;NB: djnz goes f-1 times, but ++h,++l

          ; second phase
          out  (sndbit_port),a                  ;t11/13/12
          dec   a                               ;t4/6/5, a=1
          ld   b,l                              ;t4/6/5
          ld   c,h                              ;t4/6/5
.b3:
          djnz   .b3                            ;t13/8 15/10 14/9
          dec    c                              ;t4/6/5
          jp    nz,.b3                          ;t10/12/11

          dec    e                              ;t4/6/5
          jp     nz,.b4                         ;t10/12/11
          dec    d                              ;t4/6/5
          jp     nz,.b2                         ;t10/12/11
                                        ;total= 28,36,32
          
          dec    a   ; a = 0
          out  (sndbit_port),a
          ret

          ;; waste time so that the above "DEC DE" conditional
          ;; takes the same time regardless of branch
.b4:
          dec     b                             ;t4
          jp     .b2                            ;t10

; timing loops m4p
; 2*[23 + 13f + 22f/256 + 22] + 28
; 118 + 26.17f

; timing loops m4a
; 2*[31 + 15f + 28f/256 + 28] + 36
; 154 + 30.22f

; timing loops m4b
; 2*[27 + 14f + 25f/256 + 25] + 32
; 136 + 28.20f

_beep_sound::
        ;;  bit_sound(duration, frequency)
        ;; raw version, does not disable interrupts
        ;; not adjust for M4.
          pop bc
          pop de                ; duration
          pop hl                ; freq
          push hl
          push de
          push bc
          jp __beeper
                

        ;;;  explode_sound(int d)
_explode_sound::
        di
        pop bc
        pop hl                  ; HL=d
        push hl
        push bc
        ld    a,(_useSVC)       
        or    a
        jr    z,.exp0
        sla   l
        rl    h         ; hl*2 for M4 (approx)
.exp0:        
        ld     a,#1
.expl:
          push    hl
          push    af
          ld      a,#sndbit_mask
          ld      h,#0
          and     (hl)
          ld      l,a
          pop     af
          xor     l
          out  (sndbit_port),a
          pop     hl
          push    af
          ld      b,h
          ld      c,l
.dly:     dec     bc
          ld      a,b
          or      c
          jr      nz,.dly
          pop     af
          jp	__bit_close

        ; zap sound
_zapsound::
          di
          ld    a,#1
          ld    e,#100
.fx2_1:
          out  (sndbit_port),a

          xor   #sndbit_mask  
          ld    b,e  
.fx2_2:   djnz  .fx2_2  
          inc   e  
          jr    nz,.fx2_1  
          jp    __bit_close

        ; sort of laser blast sound
; blastsound(length)
_blastsound::
          di
          pop     hl
          pop     bc
          push    bc
          push    hl
          ld      a,#230
          ld      (.u2_FR_1+1),a
          xor     a
          ld      (.u2_FR_2+1),a
          ld      a,#1
.U2_LENGHT:
          ld      b,c
          ld      c,#sndbit_port
.u2_loop:
          dec     h
          jr      nz,.u2_jump
          push    af
          ld      a,(.u2_FR_1+1)
          inc     a
          ld      (.u2_FR_1+1),a
          pop     af
          xor     #sndbit_mask
          out  (c),a
.u2_FR_1:
          ld      h,#50
.u2_jump:
          inc     l
          jr      nz,.u2_loop
          push    af
          ld      a,(.u2_FR_2+1)
          inc     a
          ld      (.u2_FR_2+1),a
          pop     af
          xor     #sndbit_mask
          out  (c),a
.u2_FR_2:
          ld      l,#0
          djnz    .u2_loop
          jp	__bit_close


        ; alert sound
_alertsound::
;Strange squeak hl=300,de=2
;Game up hl=300,de=10 inc de
;-like a PACMAN sound
          di
          xor   a
          ld    b,#1  
.fx6_1:
          push  bc  
          push  af
          ld    hl,#200  
          ld    de,#8
.fx6_2:
          push  hl
          push  de
          call  __beeper4
          pop   de
          pop   hl
          ld    bc,#5
          and   a
          sbc   hl,bc
          jr    nc,.fx6_2
          pop   af
          pop   bc
          djnz  .fx6_1
          jp    __rti      

        ; like a sort of alarm power up or something??
_squoink::
          di
          ld      a,#230
          ld      (.qi_FR_1+1),a
          xor     a
          ld      (.qi_FR_2+1),a
          ld      a,#1
.qi_LENGHT:
          ld      b,#200
          ld      c,#sndbit_port
.qi_loop:
          dec     h
          jr      nz,.qi_jump
          push    af
          ld      a,(.qi_FR_1+1)
          dec     a
          ld      (.qi_FR_1+1),a
          pop     af
          xor     #sndbit_mask
          out  (c),a
.qi_FR_1:
          ld      h,#50
.qi_jump:
          inc     l
          jr      nz,.qi_loop
          push    af
          ld      a,(.qi_FR_2+1)
          inc     a
          ld      (.qi_FR_2+1),a
          pop     af
          xor     #sndbit_mask
          out  (c),a
.qi_FR_2:
          ld      l,#0
          djnz    .qi_loop
          jp	__bit_close


.if 0                

_zap1::
          di
          ld    a,#1
          ld    b,#0  
.zap1_1:  push  bc  
          xor   #sndbit_mask  ;oscillate between high and low bits...

          out  (sndbit_port),a

.zap1_2:  nop
          nop
          djnz  .zap1_2
          pop   bc
          djnz  .zap1_1
          jp    __bit_close

; sort of clakson sound. not very good
_clackson::
          di
          ld      a,#1
.clackson_LENGHT:
          ld      b,#90
.clackson_loop:
          dec     h
          jr      nz,.clackson_jump
          xor     #sndbit_mask
          out  (sndbit_port),a

.clackson_FR_1:
          ld      h,#230
.clackson_jump:
          dec     l
          jr      nz,.clackson_loop
          xor     #sndbit_mask
          out  (sndbit_port),a
.clackson_FR_2:
          ld      l,#255
          djnz    .clackson_loop
          jp    __bit_close
        

; kind of high pitched ring sound
_zap3::
          di
          ld    a,#1
.zap3_1:  push  bc
          xor   #sndbit_mask

          out  (sndbit_port),a

          push  af
          xor   a
          sub   b
          ld    b,a
          pop   af
.zap3_2:  nop
          djnz  .zap3_2
          xor   #sndbit_mask

          out  (sndbit_port),a

          pop   bc
          push  bc
.zap3_3:  nop
          djnz  .zap3_3
          pop   bc
          djnz  .zap3_1
        jp    __bit_close

.endif

