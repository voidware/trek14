;; whatever

        .area   _CODE

sndbit_mask     .equ  3
sndbit_port     .equ  255


;;;  XX HACK to clobber the EI instruction at the __rti function
_clobber_rti::
         ld iy,#__rti
         ld (iy),#0             ; nop
         ret

__bit_close::
          xor  a
          out  (sndbit_port),a

__rti::
          ei
          ret

        ;;  bit_sound(duration, frequency)
_bit_sound::
          pop bc
          pop de                ; duration
          pop hl                ; freq
          push hl
          push de
          push bc
          xor  a                ; not wide

        ;; HL=freq, DE=duration
        ;; preserve the wide-char bits in a (if any)
__beeper::
          di
          push af
          ld   a,l
          srl  l
          srl  l
          cpl
          and  #3
          ld   c,a
          ld   b,#0
          ld   iy,#.beixp3      
          add  iy,bc
          pop  af
          push af
          or   a,#1
.beixp3:
          nop
          nop
          nop
          inc  b
          inc  c

.behllp:  dec  c
          jr   nz,.behllp
          ld   c,#0x3F
          dec  b
          jp   nz,.behllp
          xor  #sndbit_mask

          out  (sndbit_port),a

          ld   b,h
          ld   c,a
          bit  #1,a            ;if o/p go again!
          jr   nz,.be_again
          ld   a,d
          or   e
          jr   z,.be_end
          ld   a,c
          ld   c,l
          dec  de
          jp   (iy)
.be_again:
          ld   c,l
          inc  c
          jp   (iy)
.be_end:
          pop  af
          and  #0x8c            ; clear mask bits
          out  (sndbit_port),a
          jp   __rti

        ;;;  explode_sound(int d)
_explode_sound::
        di
        pop bc
        pop hl                  ; HL=d
        push hl
        push bc

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
        ;;  alertsound(wide)
_alertsound::
;Strange squeak hl=300,de=2
;Game up hl=300,de=10 inc de
;-like a PACMAN sound
          pop   bc
          pop   hl
          push  hl
          push  bc
          ld    a,l
          and   a,#1
          sla   a
          sla   a
          sla   a               ; wide to bit 3
          ld    b,#1  
.fx6_1:
          push  bc  
          push  af
          ld    hl,#300  
          ld    de,#10
.fx6_2:
          push  hl
          push  de
          call  __beeper  
          pop   de
          pop   hl
          inc  de           ;if added in makes different sound..
          ld    bc,#10
          and   a
          sbc   hl,bc
          jr    nc,.fx6_2
          pop   af
          pop   bc
          djnz  .fx6_1
          ret

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
          ld    b,#1            ;number of times
.fx5_1:   push  bc  
          ld    hl,#1200        ;freq
          ld    de,#6           ;dt
.fx5_2:   push  hl  
          push  de  
          call  __beeper  
          pop   de  
          pop   hl  
          ld    bc,#100  
          and   a               ;clr cy
          sbc   hl,bc           ;freq -= 100
          jr    nc,.fx5_2  
          pop   bc  
          djnz  .fx5_1  
          ret   


; zap/alarm??
_zap1::
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
          
; a kind of wibble
_warpcall::
          ld    hl,#1600  
          ld    (.warps+1),hl  
          ld    hl,#-800  
          ld    (.warps1+1),hl  
          ld    hl,#-100  
          ld    (.warps2+1),hl  
          ld   b,#20
.warpcall1:
          push bc
          call .warps
          pop  bc
          djnz .warpcall1
          ret   
          
.warps:   ld    hl,#1600  
          ld    de,#6  
          call  __beeper  
.warps1:  ld    hl,#-800  
.warps2:  ld    de,#-100  
          and   a  
          sbc   hl,de  
          ld    (.warps1+1),hl  
          jr    nz,.warps3  
          ld    de,#100  
          ld    (.warps2+1),de  
.warps3:  ex    de,hl  
          ld    hl,#1600  
          add   hl,de  
          ld    (.warps+1),hl  
          ret   
.endif
