;; whatever

        .globl	_useSVC

        .area    _DATA
freq::  .ds   2

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
        ;;  disables interupts
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
          push ix
          in   a,(#0xff)      ; current wide status (model I) inverted
          cpl                 ; flip. Thanks to gp2000 
          and  #0x40
          rrca
          rrca
          rrca                ; wide bit into bit 3 (other models ignored)
          ld   bc,#-1
          or   a,#2
          ld   (freq),hl
          push  de
          pop   ix
.b2:
          ld   hl,(freq)
          sub  a,#1           ; a=1    
          out  (sndbit_port),a
.b1:
          add     hl,bc
          jp      c,.b1
          ld   hl,(freq)
          add  a,#1           ;a=2
          out  (sndbit_port),a
.b3:          
          add     hl,bc
          jp      c,.b3
          add     ix,bc
          jp      c,.b2
          sub     a,#2        ;a=0
          out  (sndbit_port),a
          pop ix
          ret

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
          ld    hl,#100
          ld    de,#20
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

; wibble up sound
_fx5::
          di
          ld    b,#1            ;number of times
.fx5_1:   push  bc  
          ld    hl,#400        ;freq
          ld    de,#10           ;dt
.fx5_2:   push  hl  
          push  de  
          call  __beeper4
          pop   de  
          pop   hl  
          ld    bc,#-20
          add   hl,bc           ;freq -= 100
          jr    c,.fx5_2
          inc   de
          inc   de
          inc   de
          pop   bc  
          djnz  .fx5_1
        jp    __rti


; zap/alarm??
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

