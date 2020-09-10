# Trek14: Star Trek for the TRS-80

## Introduction

_TREK14_ is a newly developed "classic startrek" game for the [TRS-80](https://en.wikipedia.org/wiki/TRS-80), inspired by the originals, but with new ideas.

The graphics are retro and blocky, just like the originals. 

![](doc/trek14.gif)

## Installing/Running

### Model I Cassette 
The binary `TREK14.CAS` is a trs-80 model I SYSTEM CAS file, to be loaded through the cassette interface. Load using the following commands:

    SYSTEM (enter)
    *? (press T enter)

The program will load with flashing asterisks in the top right corner.
at the next `*?` prompt type `/` (enter).

The program will begin. The galaxy is generated and then you enter the main game command loop.

### Model I DOS
`go1.bat`

At the command prompt type, `TREK14`


### Model III
`go3.bat`

At the command prompt type, `TREK14`

### Model IV
`go4.bat`

At the command prompt type, `TREK14`

## Compiling Trek14

The project is completely free and open source, hosted on github,
http://github.com/voidware/trek14

Get your copy of the source repo with,

    git clone https://github.com/voidware/trek14.git

The project compiles using the Small Device C Compiler (SDCC), which is free and open source, http://sdcc.sourceforge.net

Download and install the SDCC binaries and you're ready to go!

The build uses make, for windows install mingw64.

To build;

edit `makefile` and change the first line to the location of your installed `sdcc` eg;

```
SDCCDIR = d:/sdcc
```

Then `make` should work.

SDCC generates Intel HEX format "binaries". These are converted to TRS80 CAS format. There is a utility in tools/mksys that performs this conversion. There is a windows mksys.exe compiled binary for convenience, otherwise you'll have to compile the mksys.cpp program yourself - which is only the one file with no dependencies.

The conversion from IHX to CAS is done automatically by the makefile using this  mksys utility.

## Game Design and Motivation

The game is based on Star Trek 3.5 by Lance Micklus, originally for the TRS-80, which was one of the best implementations for the time.

This "remake" has been done to have a similar flavour, but also as an attempt to improve the original gameplay. 

TREK14 has a 3D galaxy (X,Y,Z) of size (8,8,3) totalling 192 quadrants. 

Most (all?) of the "classic" Star Trek games represented the galaxy by storing the contents of each quadrant. TREK14 represents the galaxy the other way around; it has a list of entities, their location and properties rather than a list of quadrants.

This is one of the new ideas for trek14 and a deliberate departure from the classic design. The purpose of this is so that the contents of any quadrant is not hard limited and, most importantly, that the layout of entities (and their properties) within a given quadrant is not mysteriously "forgotten" when you return to that quadrant later.

For example, if you flee during combat to a neighbouring quadrant, then return to continue the fight, the positions and combat states of the enemies within that quadrant are not "reset". This was a gameplay weakness of the originals.

Each entity together with its state is packed in a 5 byte entity record (see ent.h for bit banging details). Approx 2K is reserved for the representation of the galaxy in this format (400 max entities * 5 = 2000 bytes).

One reason this format was not chosen by classic BASIC variants, is that scanning a 400 element table would be too slow in BASIC. TREK14 is a machine code program and manages to scan the table fast enough for purpose (even at 1.77Mhz).

### Klingons

The galaxy has 20 Klingon warships, randomly positioned with initially a max of 3 per quadrant. 

### Romulans

As well as Klingons, romulans are planned. Naturally these will make use of cloaking devices and are politically neutral, except when attacked. In some circumstances, the romulans will also attack the Klingons.

_Er, No they don't, Romulans TODO!_

### Bases & Federation Ships

There are 5 star bases across the galaxy, where you can refuel and restock depleted photon torpedoes (only a max of 3 can be carried). Additionally Star fleet HQ is located in the galaxy far corner (7,7,1). Dock with HQ when you wish to finish your game and receive your rating.

Other Federation ships may be encountered in the galaxy. Sometimes these come under enemy attack and call for your assistance. Sometimes, they may even help you fight the Klingons.

_Er, No they don't, TODO!_

### Phasers & Torpedoes

Federation ships have up to twice the energy capacity of enemies. Phases are the enemy's only weapon (although they might ram you!). Phaser damage decays linearly with distance. A quadrant is about the max distance a phaser can be effective, whereas photon torpedoes may penetrate adjacent quadrants - watch out you don't blow up something by accident! You may face court martial.

## User Manual

For complete details, see the manual [doc/manual.md](http://github.com/voidware/trek14/blob/master/doc/manual.md).

## Changes/News/Updates

#### 4/4P 80 Column mode support 

It currently runs on the M4 and selects the faster speed. However, the sound has not been fixed to adjust for the speed, so it's too fast.

Also, the plan is to use the additional screen columns for extra info. _TODO!_


#### Improved Klingon movement AI

Enemies can now route around obstacles. They will chase you round a planet or star, for example. Klingons can also recharge from stars, from which they have to be adjacent to do so. A, low on power, enemy will route to a star to recharge, then attack again, if given the opportunity.

#### Klingon Dodge

Klingons get a chance to dodge a photon torpedo. A good captain knows this and can accommodate in battle. However, be sure not to hit a star or a planet as this is a breach of the prime directive.

#### Battle screen shake

Wide screen mode is fixed for III/4/4P. Used in announcing alerts and "screen shake" during battle impacts, for added effect.

#### Improved Dialogue

Reports now come from named members of your crew, who now address you in the associated vernacular. New "story" based text generation allows alternative words and phrases to be used in messages. This adds variety and some levity to the dialogue. Expansion in this area is planned. Also perhaps to move the text patterns out of main memory.

#### Score and Review

You now receive an end-game report when you dock with HQ. The report is currently limited, but the plan is to make it relevant to your performance. _TODO!_

#### Explosion!

A fabulous explosion with sound is implemented for entitles. 

#### SR Scan Improvements

Short range scan now displays shield energy and RED/GREEN condition. Can press "S" inside SR scan to scan enemy energy levels - but this costs a move.

#### Enemies and Animation

Enemies now animate menacingly. Added a new, bigger and beefier Klingon ship. Plan to add the Klingon Destroyer Ships at some point too - these will be badass. _TODO!_/




























