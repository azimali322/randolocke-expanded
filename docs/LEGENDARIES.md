# Legendary encounters

Where every legendary encounter in Randolocke Expanded is, how to reach it and how to catch it.
Everything here was checked against the hack's own map scripts and puzzle code, using its default
settings.

## At a glance

| Encounter | How you get there | Level | What you do |
| --- | --- | --- | --- |
| Sky Pillar | Route 131, after the Sootopolis crisis | 63 | Climb the tower on the Mach Bike |
| Desert Ruins | Route 111, open from the 8th badge | 40 | Use Flash in the first room |
| Island Cave | Route 105, open from the 8th badge | 40 | Use Flash in the first room |
| Ancient Tomb | Route 120, open from the 8th badge | 40 | Use Flash in the exact middle of the first room |
| Southern Island | Eon Ticket, ferry from Lilycove | 50 | Press A on the stone at the far end |
| Faraway Island | Old Sea Map, ferry from Lilycove | 30 | Chase Mew through the tall grass |
| Birth Island | Aurora Ticket, ferry from Lilycove | 30 | Solve the triangle puzzle |
| Navel Rock summit | Mystic Ticket, ferry from Lilycove | 70 | Climb to the top |
| Navel Rock base | Mystic Ticket, ferry from Lilycove | 70 | Climb down to the bottom |
| Roaming Lati | Loose in Hoenn after the Champion | 40 | Track it down and stop it fleeing |
| Terra Cave | Weather Institute on Route 119, after the Champion | 70 | Find the cave on the drought route |
| Marine Cave | Weather Institute on Route 119, after the Champion | 70 | Dive at the rough water on the rainy route |

## What's different in this hack

- **You won't meet the original legendaries.** Every legendary spot, and the roaming Lati, gives
  a random *box legendary* instead: Mewtwo, Lugia, Ho-Oh, the weather trio, Dialga, Palkia and the
  rest, 27 in all, each equally likely. No two give the same one, and each keeps its pick for the
  whole save. [Settings reference: Legendaries](SETTINGS.md#legendaries) has the details.
- **The overworld still shows the original.** The sprite you walk up to, its cry and the
  "... flew away!" message still name the original Pokémon (Rayquaza, Mew and so on). The battle
  is against the random one.
- **They're much easier to catch.** Every legendary has a catch rate of 45 here. In the original,
  most have 3.
- **The nuzlocke rules don't get in the way.** These spots aren't areas, so the one-per-area rule
  never applies. The legendary clause lets you catch a legendary you meet on a route, like the
  roaming Lati, even on a route you've used, and meeting one never uses the route up.
- **The Elite Four let in one legendary at most.** This matters if you go back to beat them again,
  for example to bring back a legendary you knocked out.
- **No Pokémon has to know an HM.** Surf, Dive, Flash and the other badge moves work as long as
  you have the badge. Flash is in every Pokémon's field-move list in the party menu.
- **There's one Bike.** Press R while riding to switch it between Mach and Acro.

### If it goes wrong

| Encounter | If you run | If you knock it out |
| --- | --- | --- |
| Sky Pillar, the three Regis | Leave and come back in | Gone for good |
| Southern Island, Faraway Island, Navel Rock | Leave and come back in | It returns after you beat the Elite Four again |
| Birth Island | Leave the island, come back and redo the puzzle | It returns after you beat the Elite Four again; redo the puzzle |
| Roaming Lati | It keeps roaming, with the damage and status you gave it | Gone for good |
| Terra Cave, Marine Cave | Go back in while the weather lasts | Gone for good |

## Before the Champion

### Rayquaza: the Sky Pillar

**When:** any time after Rayquaza stops Groudon and Kyogre in Sootopolis. It's **level 63** here,
the Elite Four's level cap, rather than the original 70.

**What you need:** Surf, and the Bike in Mach mode.

**Getting there:** fly to Pacifidlog Town and surf east onto Route 131. The Sky Pillar's cave is on
the north side of the route. Go through it, cross to the tower's door and start climbing. 1F, 3F
and 5F are ordinary floors. 2F and 4F have cracked floors.

**Cracked floors.** The rules, straight from the game's code:

- **Only the Mach Bike at full speed gets across a crack.** Walking, running, the Acro Bike or a
  slow Mach Bike drops you through to the floor below, and you climb back up.
- **You're at full speed from the second tile of a ride,** so start at least one tile back from the
  first crack.
- **Keep holding the D-pad.** Your speed carries round corners. Letting go, or riding into a wall,
  while you're on a crack drops you.
- **A crack you've crossed turns into a hole,** so don't double back over it.

**2F:** ride round the tower from where you come up (`S`) to the stairs up (`U`). The two halves
only meet across cracked floor.

```text
##############
###U######S###
........#.....
.....#.##.....
......#...#...
....######xxxx
##..######xxxx
..xx######...#
xxxx######....
xxxx######....
xx..######xx..
x..#xx...#xxxx
..#.xx....xxxx
...xxx.......#
```

`x` cracked floor · `#` wall · `.` solid floor

**4F: the trap.** From the stairs you come up by (`S`), there is no way to the stairs up (`U`), even
across the cracks. The way on is to drop through a crack on purpose, into a sealed-off room on 3F.

```text
##############
###U###A###S##
........#.....
xx########...#
##L.x.xx.x#...
.#..######.xxx
....######.xxx
.x#x######.x#.
.x#x######...#
#x#x######....
#x..######....
...x.#.x.#xx..
..#.x.x.x.xx..
...x.x.#.xxx..
```

1. Ride from `S` round to the short row at the top left, and stop on `L` by riding into the wall
   beyond it.
2. Face right. Hold Right just long enough to cross the first crack, about two tiles, then let go.
3. You either coast to a stop on the solid tile past the crack, or drop straight through one of
   the next two. If you stopped, press Right once more to drop through.
4. You land in a small closed room on 3F. Take its stairs up: they come out on 4F at `A`, just
   along from the stairs to 5F.
5. If you land in the big part of 3F instead, you held on too long. Go back up and try again.

**Summit:** walk up to the legendary and press A.

### The three Regis

All three are **level 40**. Each cave is a small tomb: a first room with a Braille wall at the top,
and the Regi's chamber behind that wall.

**Opening the caves:** once you have the **8th badge**, all three are open. Each door is open the
first time you visit its route after the badge. Before that, you open them from the
[Sealed Chamber](#before-the-8th-badge-the-sealed-chamber).

**Opening the wall:** this hack adds a shortcut. Use **Flash** from the party menu in the first
room and the Braille wall opens. In the Desert Ruins and Island Cave, anywhere in the room works.
In the Ancient Tomb, you have to stand on the exact middle tile. The original Braille puzzles still
work too.

Then walk into the chamber, up to the Regi, and press A.

| Regi's spot | Cave | Where | Flash | The original puzzle |
| --- | --- | --- | --- | --- |
| Regirock | Desert Ruins | Route 111, in the south-east part of the desert (you need the Go-Goggles) | Anywhere in the first room | Stand below the middle of the Braille, walk 2 left and 2 down, and use Rock Smash (a Pokémon has to know it) |
| Regice | Island Cave | Route 105, a small island near the north end, on the west side | Anywhere in the first room | Read the Braille, then walk one full lap of the room, keeping right against the wall |
| Registeel | Ancient Tomb | Route 120, on the west side, about halfway down | Only on the middle tile: stand where you read the Braille, then walk 4 steps straight down | The same: Flash in the middle |

#### Before the 8th badge: the Sealed Chamber

You only need this if you want the Regis before the 8th badge.

1. On **Route 134**, the sea route just east of Slateport, dive in the deep water in the east part
   of the route. Swim into the chamber and surface inside.
2. **First room:** use Flash anywhere in it, and the way into the second room opens. The original
   way is to stand right below the middle of the top wall and use Dig, which a Pokémon has to know.
3. **Second room:** use Flash anywhere in it. The room shakes, and a message says a door opened far
   away: that's all three caves. The original way is to put Wailord first and Relicanth last in
   your party, then read the Braille on the back wall.

## After the Champion

### Getting to the islands: the ferry from Lilycove

1. **Buy the tickets.** The chart seller stands beside the ship in **Slateport Harbor**. From the
   8th badge he sells all four for ₽1 each: the Eon Ticket, the Old Sea Map, the Aurora Ticket and
   the Mystic Ticket.
2. **Beat the Champion.** Until then the ferry won't sail to the islands.
3. **Sail from Lilycove, not Slateport.** The ferry in Slateport only goes to Lilycove and the
   Battle Frontier. Fly to Lilycove. From the Pokémon Center, walk south past the Contest Hall to
   the beach, then go left (west). The harbor is the building in the bottom-left corner of the city.
4. **Talk to the sailor at the counter.** You don't need the S.S. Ticket. The four tickets just
   have to be in your Bag.

The first trips go in a set order:

- **First talk:** no menu. The Old Sea Map comes first. Mr. Briney steps in and sails you straight
  to **Faraway Island**. There's no yes/no, so heal and save before you talk to the sailor.
- **Second talk:** the sailor asks which of the other three you want: Southern Island, Navel Rock
  or Birth Island.
- **From then on:** a normal menu lists every island, plus Slateport, and the Battle Frontier once
  you've met Scott on the S.S. Tidal.

**Coming back:** talk to the sailor on the island's dock and he sails you back to Lilycove Harbor.

| Ticket | Island | Original legendary |
| --- | --- | --- |
| Eon Ticket | Southern Island | Latias or Latios |
| Old Sea Map | Faraway Island | Mew |
| Aurora Ticket | Birth Island | Deoxys |
| Mystic Ticket | Navel Rock | Ho-Oh (summit) and Lugia (base) |

### Southern Island (Eon Ticket)

1. From the dock, walk north into the middle of the island.
2. Keep going to the stone at the far end. Stand directly below it, facing it, and press A.
3. A Pokémon flies in and the battle starts. It's **level 50** and holds a **Soul Dew**, which you
   keep if you catch it.

Which one you meet depends on how you answered Mom about the TV news when you got home as
Champion.

- **Red:** Latias's slot roams Hoenn, and Latios's slot is on the island.
- **Blue:** the other way round.

In this hack each slot is its own random legendary. The slot that isn't on the island is the
roaming one (see [The roaming Lati](#the-roaming-lati)).

### Faraway Island (Old Sea Map)

**Getting in:** from the dock, follow the winding path north. The way into the grassy clearing is
the opening at the island's north end, a little right of centre.

**What happens:** as you walk in, Mew spots you, darts three steps up into the tall grass and turns
invisible. Then it plays hide-and-seek. Its rules, straight from the game's code:

- **It moves only when you do.** Every step you take, it takes one. Turning on the spot doesn't
  move it.
- **It runs directly away from you.** It only moves through the tall grass, and never onto your
  tile.
- **It shows itself once every 8 steps.** It stays visible until you move again, so stop and look.
- **The grass it moves into rustles once every 4 steps.** That's how you follow it while it's
  invisible.
- **It doesn't move once every 9 steps.**
- **You can talk to it while it's invisible.** Pressing A while facing it works, and walking into it
  just bumps you into it.

**How to catch it:**

1. Follow the rustling grass and stay a step or two behind it.
2. Push it toward a corner of the grass patch, where it has fewer ways out.
3. When a step leaves you right beside it (it couldn't get away, or it was on its still step), turn
   to face it and press A.
4. The battle starts at **level 30**.

**Don't use Cut in the grass.** Mew leaves ("The feeling of being watched faded...") until you walk
out and come back in.

The clearing, drawn from the game's map data:

```text
..##............###
.##..............##
#..."""""""""""".#.
#..""""""".""".".#.
#.."".""""O"""O"#..
#..""O""""""""""#..
##.""""""""""""".#.
##.""""""."""""..#.
.#.."""""O""""".##.
..#...""""""""..##.
...#...."""....###.
...##..........##..
#..##....M....###..
....##.......####..
....####vv####.....
```

| Symbol | Meaning |
| --- | --- |
| `"` | tall grass: the only ground Mew can move on |
| `O` | rock |
| `#` | wall |
| `.` | ground Mew can't use |
| `v` | where you come in |
| `M` | where Mew waits before it hides |

Mew watches out for the four rocks and won't let itself be pinned against them, so herd it toward
the patch's corners instead.

### Birth Island (Aurora Ticket)

**How the puzzle works:** the floating triangle in the middle of the island is the puzzle. Press A
on it and it changes colour and jumps to another spot. Walk to it by the shortest route and press A
again. After 10 jumps, the 11th press breaks it open and the battle starts, at **level 30**.

- **One step more than the shortest route resets it.** Your next press sends the triangle drifting
  slowly back to the middle, with a different sound, and you start over.
- **Turning on the spot and bumping into things don't count as steps.** Running is fine: each tile
  is one step either way.
- **Leaving the island also resets it.**

**The route.** From the dock, walk straight up until you bump into the triangle, so you're standing
directly below it. Then:

| Press A | Then walk | Steps |
| --- | --- | --- |
| 1 | Down 1, Left 3 | 4 |
| 2 | Right 3, Up 5 | 8 |
| 3 | Down 5, Right 3 | 8 |
| 4 | Up 3, Left 5 | 8 |
| 5 | Right 4 | 4 |
| 6 | Down 3, Left 1 | 4 |
| 7 | Left 4 | 4 |
| 8 | Right 6 | 6 |
| 9 | Left 3, then tap Down to face it | 3 |
| 10 | Up 3 | 3 |
| 11 | The battle starts | |

- **No room for a wrong step.** Every walk uses the full number of steps allowed.
- **You end each walk facing the triangle,** except after press 9.
- **If you slip up,** press A on it anyway to send it back to the middle. Then stand directly below
  it and start again from press 1. The route only works when press 1 is made from below.

Where the triangle sits before each press:

```text
########.......########
#######.........#######
#######....B....#######
#####.............#####
#####......H......#####
####....D.....E....####
####.......S........###
##..................###
##.....A...F...C.....##
##..##.....G.....##..##
#...##...........##...#
###########.###########
###########.###########
###########.###########
##########...##########
##########...##########
##########...##########
##########...##########
##########.^.##########
```

`^` is where you arrive from the dock.

| Press | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | 11 |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| Triangle at | S | A | B | C | D | E | F | A | C | G | H |

### Navel Rock (Mystic Ticket)

Navel Rock has two legendaries, one at the summit (Ho-Oh's spot) and one at the base (Lugia's).
Both are **level 70**.

1. From the dock, walk north into the cave, then follow the path through the next two caves.
2. You come out at the bottom of a very long corridor. Walk all the way up to the room at the top,
   which has two ladders:
   - **The left ladder** climbs to the summit.
   - **The right ladder** goes down to the base.
3. Either way leads through a string of small rooms with two ladders each. In each room, take the
   other ladder. The summit is **4** rooms up. The base is **11** rooms down.

**Summit:** walk up the narrow path. Near the top, the legendary swoops down to you and the battle
starts. A **Sacred Ash** is hidden on the tile straight in front of you. After the battle, press A
without moving to pick it up.

**Base:** walk up the narrow path to the legendary and press A.

### The roaming Lati

The TV news you watched at home after becoming Champion set one of the Southern Island pair roaming
Hoenn. Here it's that slot's random legendary, at **level 40**.

- **Where it is:** it moves to another route every time you change maps. On whichever route it's
  on, each wild battle has a 1-in-4 chance of being it.
- **Tracking it:** once you've met it, its Pokédex **Area** page shows where it is right now.
- **It flees on its first turn,** but keeps any damage and status you gave it.
- **Stopping it:** lead with a Pokémon whose ability traps from the start, such as Shadow Tag or
  Arena Trap. Arena Trap doesn't hold anything flying or levitating, and nothing traps a Ghost
  type. A Master Ball on the first turn always works.
- **Knocking it out loses it for good.**

### Terra Cave and Marine Cave

These are Groudon's and Kyogre's caves. Each gives its own random legendary, at **level 70**.

1. After becoming Champion, go to the **Weather Institute on Route 119**. Talk to the scientist on
   the second floor.
2. He names a route with strange weather:
   - **A drought** means Terra Cave. A new cave entrance appears on Route 114, 115, 116 or 118.
   - **Heavy rain** means Marine Cave. A patch of rough, dark water appears on Route 105, 125, 127
     or 129. Dive there.
3. Go through the cave to the end to find the legendary.

- **The weather doesn't wait.** It ends after about 1,000 steps outside the caves. If it's gone,
  ask the scientist again.
- **Each cave is a one-off.** Catching its legendary or knocking it out closes that cave for good.
  After that, the scientist only reports the other one.
