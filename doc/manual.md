TREK14: Classic Star Trek for the TRS-80 Model I/III/4/4P
=========================================================

Your mission is to explore the galaxy and destroy the 50 Klingons before star-date 2100.

The galaxy is a 8x8x3 grid of 192 quadrants, each of which is subdivided into 64x14 sectors, corresponding to the X and Y screen position.

Along with Klingons, there are also Federation bases where you can refuel, as well as planets and stars.

## Long Range Scan

Press "L" for long range scan, either from the command menu or short range scan screen.

The long range scan shows a 3x3x3 quadrant view of entities centred around your current location. So, for example, if you're at (1,1,1) you will see quadrants in the range (0-2, 0-2, 0-2). Quadrants outside the galaxy are displayed as "VOID" and are inaccessible.

The long range scan shows a summary of content in each quadrant, For example:

    F1S1P2

Means, 1 *Federation* vessel, 1 *Star* and 2 *Planets*. Other codes are K for Klingons and B for bases.

## Warp Drive

Press "W" from the command menu or short range scan.

Warp drive can be used to move from one quadrant to another, it is quicker than
impulse engines but uses more energy. it takes the same time to cross any distance with warp, but the energy varies with distance. You can cross quadrant boundaries on impulse engines, but it is slower.

When prompted for the location enter the three coordinates X, Y and Z separated by commas. For example, you start your mission in quadrant 7,7,2 - the bottom right corner of the galaxy.

## Short Range Scan

Press "S" from the command menu.

The short range scan is the main display of the current quadrant and the view for tactical combat.

Your ship can move on impulse engines left, right, up and down using the arrow keys. You can access most of the commands from this screen by just pressing their command letter. Impulse movement takes one unit of energy and takes one unit of time. An extra 10 units of energy is consumed during red alert.

Red alert is automatically engaged when enemies are in your quadrant.

You can scan enemies from within the short range scan by pressing "S". This will show you how much energy they have. However, this takes a turn and those enemies will have another chance to move or attack.

## Klingons

Klingons cluster in bunches of 1 to 4. Their ships are smaller and have less energy capacity than a starship. They have no photon torpedoes and rely wholly on phasers for weapons. Klingons can manoeuvre slightly better on impulse engines and can move diagonally; they can close on a ship quite fast.

Klingons do not retreat but they do step back somewhat to recharge. This is possible when a quadrant has a star. The Klingons can slowly recharge from the star's energy, then later come in for another attack.

Some Klingon ships are slightly bigger and have more energy, but are still inferior to a starship.

## Phasers

Press "P" from the command menu or from the short range scan.

Phasers are locked on to near enough enemy targets automatically. You are prompted for the amount of energy to deploy to phasers. The phasers will fire at all enemies within the quadrant that are in line of sight (ie not behind another object), each one consuming the specified energy, providing it's available.

Be sure not to deplete too much energy during a phaser attack. It can leave your ship unable to escape.

The damage potential of phasers decreases with distance according to an exponential decay law, so that the power emitted may be as little as 10% across the width of a quadrant.

Klingons will tend to close rapidly for attack, fire at close range, then move back somewhat, so as to inflict the most damage on their foe.

## Photon Torpedoes

Press "T" from the command menu or from the short range scan.

You can carry at most 3 photon torpedoes at any time. Torpedoes are very powerful and will often destroy an enemy ship in a single hit. Unlike phasers, the damage potential of torpedoes does not decrease with distance, so it's the ideal weapon to pick of an enemy before it gets too close. However, enemies will try to dodge the torpedo if not at close range.

Torpedoes do not aim themselves, you have to manually aim them by setting their direction. The direction is entered as a bearing in degrees from 0-360, where 0 points directly ahead of your ship.

direction chart:


                         90
            135          |           45
              \          |          /
                \        |        /
                  \      |      /
                    \    |    /
                      \  |  /
                        \|/
    180 ---------------- o ---------------- 0
                        /|\
                      /  |  \
                    /    |    \
                  /      |      \
                /        |        \
              /          |          \
             225         |          315
                        270


Be careful when aiming photon torpedoes because, if you miss, you might destroy a non-enemy object - which can result in court martial!

Be wary of that old Klingon trick, to place itself between you and a star or planet, dodge the attack and have you hit the planet.

## Bases

Press "D" to dock with an adjacent base.

There are 10 Federation bases in the galaxy where you can doc to refuel. Spent photon torpedoes are also replenished. To dock, manoeuvre your ship adjacent to the base and press "D".

Note that the base in quadrant 7,7,2 should **NOT** be docked, unless you wish to end your mission. This base is star fleet HQ, docking there voluntarily ends the mission giving your report and score.

When star-date 2100 is reached, you will be recalled to HQ. you _must_ return asap otherwise your score is affected. If you do not return soon enough, you will be relieved of command.

## Damage

The impact energy of an enemy blast must be absorbed. Ideally this energy is absorbed by your shields which take the impact energy cost from your ship's energy banks. If this energy is more than your reserves, you are destroyed.

The capacity of your shields to absorb energy depends on their strength. Fully charged shields can absorb about half the total energy of a Klingon warship. However, depleted shields take time to recharge.

The Shield strength is shown on the top of the short range scan as "S"

Any excess impact energy over the strength of your shields results in damage to the ship. Any of the operations can be affected; scanners (both long and short range), movement including the warp drive and impulse engines as well as your ability to retaliate with photon torpedoes and phasers.

For example, if the impulse engines are damaged, you cannot move around the quadrant until they are repaired - you are a sitting duck!

Worse still is if the short range scanner are inoperative. In such a case you are totally blind. The scan shows an empty quadrant, with all positions unknown.

Your engineering crew will work hard to restore damaged operations. They give priority to fixing your tactical display and impulse movement.

Tactical combat is the crux of the game and the proving grounds of the competent captain.

## Ship's Computer

Press "C" from the command menu.

As you explore the galaxy, information from the long range scan is accumulated in the ship's memory banks.

You can search those memory banks for known locations of enemies or bases. For example, if you are low on energy and need to refuel at the nearest known star base, you can us the ship's computer to list all bases discovered.

Helpfully, the computer will tell you the closest base or enemy.

## Tips and Tricks

Be competent at the photon torpedo aiming angle. Whatever you do, make sure you do not accidentally hit a planet, star or base.

Klingons will often manage to dodge a torpedo fired across a distance. Sometimes however there is no dodge space for them in which case they can't move. Alternatively if there are two Klingons on the same line, only 1 will manage to dodge.

If there is a star in the quadrant, after an attack, Klingons will approach the star and collect energy. In such cases, try to head off the Klingon or destroy it before it can recharge.

Klingons will navigate around obstacles as part of their attack.

You can cross quadrant borders using the arrow keys (impulse engines). If you are losing a battle (esp if warp is damaged), you can retreat across the border. Klingons won't pursue you (at the moment!).

Always go into battle against 3 or more Klingons with a full set of torpedoes.

Do not use phasers until enemies are close, the energy depletion with distance will otherwise result in a lot less damage.

Systematically explore the galaxy by navigating in the 1 plane, ie (X, Y, 1). Good positions are eg (1,1,1), (6,6,1), (3,6,1) and so on that will long range scan a lot of nearby quadrants.

Good Luck!

























