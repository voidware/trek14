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
LDFLAGS = -mjwx -b _CODE=0x4300 -b _DATA=0x7000


OBJS = \
	crt0.rel \
	command.rel \
	lrscan.rel \
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


