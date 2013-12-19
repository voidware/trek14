del *.rel *.lk *.lst *.noi *.ihx *.map
sdcc -mz80 -c trek14.c
sdcc -mz80 -c ent.c
sdcc -mz80 -c os.c
sdasz80 -l -o crt0.rel crt0.s
REM use _CODE=0x5200 for DOS
sdldz80 -mjwx -i trek.ihx -b _CODE=0x4300 -b "_DATA=0x6000" crt0.rel os.rel trek14.rel ent.rel




