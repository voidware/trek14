del *.rel *.lk *.lst *.noi *.ihx *.map
sdcc -mz80 -c libcbase.c
sdcc -mz80 -c libc.c
sdcc -mz80 -c plot.c
sdcc -mz80 -c trek14.c
sdcc -mz80 -c ent.c
sdcc -mz80 -c os.c
sdasz80 -l -o crt0.rel crt0.s
sdasz80 -l -o crtcall.rel crtcall.s
sdasz80 -l -o divunsigned.rel divunsigned.s
sdasz80 -l -o mul.rel mul.s
REM post dos start 0x5200
sdldz80 -mjwx -i trek.ihx -b _CODE=0x4300 -b "_DATA=0x6000" crt0.rel os.rel trek14.rel ent.rel plot.rel libc.rel libcbase.rel crtcall.rel divunsigned.rel mul.rel





