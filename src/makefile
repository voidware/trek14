#
# Build Trek14!
# 

CC = sdcc
AS = sdasz80
LD = sdldz80
DEBUG = 
OPT2 = --max-allocs-per-node 100000 
OPT = --opt-code-size
CFLAGS = -mz80 --std-sdcc99 $(OPT) $(DEBUG)
ASFLAGS = -l

## set up your path to where the SDCC Z80 lib is
LIBS = -l i:\sdcc\lib\z80\z80.lib

# 48k dos machine
LDFLAGS = -mjwx -b _CODE=0x5200 $(LIBS)

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
	damage.rel \
	alert.rel  \
	os.rel \
	utils.rel \
	sound.rel \
	libc.rel \
	libcbase.rel \
	soundbit.rel \
	story.rel \

%.rel: %.c
	$(CC) $(CFLAGS) -c $< 

%.rel: %.s
	$(AS) $(ASFLAGS) -o $@ $< 

all: trek14.dsk

trek14.cas: trek14.ihx
	../tools/mksys/mksys trek14.ihx trek14.cas


trek14.ihx : $(OBJS) Makefile
	$(LD) $(LDFLAGS) -i trek14.ihx $(OBJS)

trek14.cmd: trek14.cas
	../tools/trld/trld trek14.cas trek14.cmd

trek14.dsk: trek14.cmd
	rm -f trek14.dsk
	cp ../emu/blank.dsk trek14.dsk
	../tools/trswrite -o trek14.dsk trek14.cmd

trek14.zip: 
	(cd ..; zip -r trek14.zip readme.md emu doc src tools -x \*win32\* -x \*TAGS\*)

.PHONY:	 clean cleanall tags

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

cleanall: clean
	rm -f *.cmd
	rm -f *.cas


tags:
	ctags -e *.h *.c



