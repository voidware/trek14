#
# Build Trek14!
# 

CC = sdcc
AS = sdasz80
LD = sdldz80
DEBUG = 
OPT = 
CFLAGS = -mz80 $(DEBUG)
ASFLAGS = -l

#16k machine
#LDFLAGS = -mjwx -b _CODE=0x4300 -b _DATA=0x7600

# 48k dos machine
LDFLAGS = -mjwx -b _CODE=0x5200  -b _DATA=0xE600

OBJS = \
	crt0.rel \
	command.rel \
	phasers.rel \
	enemy.rel \
	warp.rel \
	lrscan.rel \
	srscan.rel \
	plot.rel \
	trek14.rel \
	ent.rel \
	os.rel \
	utils.rel \
	libc.rel \
	libcbase.rel \
	crtcall.rel \
	divunsigned.rel \
	mul.rel \

%.rel: %.c
	$(CC) $(CFLAGS) -c $< 

%.rel: %.s
	$(AS) $(ASFLAGS) -o $@ $< 

trek14.cas: trek14.ihx
	../tools/mksys/mksys trek14.ihx trek14.cas


trek14.ihx : $(OBJS)
	$(LD) $(LDFLAGS) -i trek14.ihx $(OBJS)

trek14.cmd: trek14.cas
	../tools/trld trek14.cas trek14.cmd

clean:
	rm -f *.rel
	rm -f *.lk
	rm -f *.lst
	rm -f *~
	rm -f *.noi
	rm -f *.ihx
	rm -f *.map
	rm -f *.asm
	rm -f *.sym


