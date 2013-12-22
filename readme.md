# This is trek14: Startrek for the trs-80 model 1

## Introduction

TREK14 is a newly developed "classic startrek" game for the trs80 model 1. Inspired by some of the originals, but with some new ideas.

## Installing/Running

The binary TREK14.CAS is a trs80 model 1 SYSTEM CAS file, to be loaded through the cassette interface. Load using the following command:

SYSTEM (enter)
at the \*? prompt type T (enter)
the program will load with flashing asterisks in the top right corner.
at the next \*? prompt type / (enter)

The program will begin. The galaxy is generated and then you enter the main game command loop.

## Using an Emulator

If you don't have a real trs80 model 1 handy, you can run the game in an emulator. There are many excellent emulators around for the machine and even some web based ones.

I use the open source SDLTRS emulator, which is free and you can compile it yourself from scratch. There is a windows binary in the "emu" directory for convenience.

SDLTRS may be configured using the menu on F7. F11 is the "turbo" button, which you may find useful for speeding up loading.

For full details see:

    http://sdltrs.sourceforge.net/

## Compiling Trek14

The project is completely free and open source, hosted on github,
http://github.com/voidware/trek14

Get your copy of the source repo with,

    git clone https://github.com/voidware/trek14.git

The project compiles using the Small Device C Compiler (SDCC), which is free and open source,

    http://sdcc.sourceforge.net/

Download and install the SDCC binaries and you're ready to go!

The build uses make (for windows either install cygwin or mingw). To build, go to the src directory and type make.

SDCC generates Intel HEX format "binaries". You need to convert these to TRS80 CAS format. There is a utility in tools/mksys that performs this conversion. There is a windows mksys.exe compiled binary for convenience, otherwise you'll have to compile the mksys.cpp program yourself - which is only the one file with no dependencies.

The conversion from IHX to CAS is done automatically by the makefile using this  mksys utility.

## Game Design

This section explains a bit about the internal game design to help you understand the source code.

TREK14 has a 3D galaxy (X,Y,Z) of size (8,8,3) totalling 192 quadrants. 

Most (all?) of the "classic" startrek games in BASIC represented the galaxy by storing the contents of each quadrant. TREK14 represents the galaxy the other way around; it has a list of entities, their location and properties rather than a list of quadrants.

This is one of the new ideas for trek14 and a deliberate departure from the classic design. The purpose of this is so that the contents of any quadrant is not hard limited and, most importantly, that the layout of entities (and their properties) within a given quadrant is not mysteriously "forgotten" when you return to that quadrant later.

For example, if you flee during combat to a neighbouring quadrant, then return to continue the fight, the positions and combat states of the enemies within that quadrant are not "reset". This was a gameplay weakness of some implementations.

Each entity together with its state is packed in a 5 byte entity record (see ent.h for bit banging details). Approx 2K is reserved for the representation of the galaxy in this format (400 max entities * 5 = 2000 bytes).

One reason this format was not chosen by classic BASIC variants, is that scanning a 400 element table would be too slow in BASIC. TREK14 is a machine code program and manages to scan the table fast enough for purpose (even at 1.7Mhz).

### Klingons

The galaxy has 20 klingon warships, randomly positioned with initially a max of 3 per quandrant. 

### Romulans

As well as klingons, romulans are planned. Naturally these will make use of cloaking devices and are politically neutral, except when attacked. In some circumstances, the romulans will also attack the Klingons.

### Bases & Federation Ships

There are 5 star bases across the galaxy, where you can refuel and restock depleted photon torpedoes (only a max of 3 can be carried). One of the bases is starfleet HQ.

Other Federation ships may be encountered in the galaxy. Sometimes these come under enemy attack and call for your assistance.

### Phasers & Torpedoes

Federation ships have up to twice the energy capacity of enemies. Phases are the enemy's only weapon (although they might ram you!). Phaser damage decays linearly with distance. A Quadrant is about the max distance a phaser can be effective, whereas photon torpedoes may penetrate adjacent quadrants - watch out you don't blow up something by accident! You may be court marshalled.























































