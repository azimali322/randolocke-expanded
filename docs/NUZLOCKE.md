# Nuzlocke rules

Everything here is enforced by the game. Settings live in `include/config/randolocke.h`;
`RANDOLOCKE_FLAG_NUZLOCKE_OFF` (flag `0x2D`) switches the whole system off for a save.
The flag is **inverted on purpose**: clear means the rules are *on*, so a save made before
the rules existed gets them without a new game.

## 0. When the rules start

Nothing applies until **Birch hands over the five Poké Balls** — after the Route 103 rival
battle and the walk back to the lab. That moment sets `FLAG_ADVENTURE_STARTED`, which is
what gates the whole system.

Before it you have one Pokémon, no balls, and a rival battle you are meant to be able to
lose, so there is nothing to rule on. The starter is not an area catch, and losing the
first battle costs you nothing.

## 1. Catching

| Rule | Behaviour |
| --- | --- |
| **One per area** | One catch per wild-encounter area. After that, Poké Balls are refused there |
| **Dupes clause** | A species whose evolution family you already own cannot be caught — and meeting one does **not** use the area up, so you can keep looking |
| **Shiny clause** | A shiny is always catchable and never uses the area up |

An *area* is one entry in the wild encounter tables, which is one map. Land, surfing and
fishing on the same map share an area. Anywhere with no wild table — the legendary sites,
gift Pokémon, scripted battles, Wally's tutorial — is not an area and is never restricted.

The dupes check walks the whole evolution family, in both directions: catch a Zigzagoon
and a later Linoone is refused; catch the Linoone first and the Zigzagoon is refused.

## 2. Death

**An individual faint is just a faint.** Heal at a Pokémon Center and carry on — no
permadeath per Pokémon.

What costs you is a **wipe**: every Pokémon in your party down at once. Then the whole
party is

1. stripped of held items, which go back to your bag
2. marked
3. moved into a PC box
4. removed from your party

and you walk out of the Pokémon Center with **nothing** and pick a new team off the PC.
A marked Pokémon **cannot be withdrawn, moved or shifted** — the PC says "This POKéMON is
gone for good." It *can* be released, so the box can be tidied. Eggs are spared: they were
never in the fight.

Wild encounters are suppressed while your party is empty, so the walk to the PC is safe.

Becoming Champion ends the run, and from that point the mark stops being enforced: every
Pokémon you lost is yours again.

The mark is stored in `marineRibbon`. Substruct 3 is exactly full at 96 bits, so a new
field would change the size of every boxed Pokémon and invalidate saves; `marineRibbon` is
never distributed in Emerald, so it is free. The side effect is cosmetic — a dead Pokémon
shows a Marine Ribbon.

### What is not covered

Anything before `FLAG_ADVENTURE_STARTED`, including the scripted Route 103 rival battle.

## 3. Wiping

When the whole party is down:

| Situation | What happens |
| --- | --- |
| A living Pokémon is left in a box | Your whole party is boxed and marked, then the usual white-out: back to the last Pokémon Center, empty-handed. Withdraw a new team |
| Nothing living is left anywhere | **The run is over.** The game returns to the title screen |

The save is never deleted. Load it and you are standing at the last Pokémon Center with an
empty party — a record of the run rather than a playable one. If you want to keep going
anyway, set flag `0x2D` in the debug menu and the box will open again.

`RANDOLOCKE_RUN_OVER_ON_WIPE FALSE` disables the ending and leaves the vanilla white-out.

## 4. Level caps

Hard caps by badge. Over the cap a Pokémon gains **no** experience at all
(`B_EXP_CAP_TYPE = EXP_CAP_HARD`), and Rare Candies cannot push past it.

The caps are not arbitrary, and they are not derived at runtime — **each one is already
the next boss's ace level**, because the trainers were scaled to them in Phase 7c:

| While you have | Cap | Next boss | Their party | Ace |
| --- | --- | --- | --- | --- |
| 0 badges | 14 | Roxanne | 11, 11, 14 | **14** |
| 1 badge | 21 | Brawly | 16, 16, 21 | **21** |
| 2 badges | 24 | Wattson | 22, 22, 23, 24 | **24** |
| 3 badges | 29 | Flannery | 24, 24, 26, 29 | **29** |
| 4 badges | 36 | Norman | 27, 27, 29, 36 | **36** |
| 5 badges | 43 | Winona | 29, 29, 32, 36, 43 | **43** |
| 6 badges | 47 | Tate & Liza | 47, 47, 47, 47 | **47** |
| 7 badges | 50 | Juan | 47, 47, 48, 48, 50 | **50** |
| 8 badges | 63 | Elite Four → Wallace | Sidney 53 → Wallace 63 | **63** |
| Champion | 100 | — | — | — |

So "cap by the next gym's highest level" is what you already have. Computing it at runtime
would be circular: the boss levels were generated *from* the caps, not the other way
round. The single source of truth is `sLevelCapFlagMap` in `src/caps.c` — change a number
there and re-run `tools/randolocke/scale_trainers.py` to bring the trainers with it.

The 8-badge cap of 63 deliberately covers the whole Elite Four, so the run ends with you
level-capped against a Champion at your own level rather than above it.

## 5. Trainer EVs

Not one of the 856 trainers in `trainers.party` specifies EVs, so in vanilla Emerald every
trainer Pokémon — gym leaders included — runs on **zero EVs**. The player has no EV cap and
can train freely, which turns any boss into a pushover the moment you bother to.

`RZ_TRAINER_EV_SCALING` gives trainers a spread that grows with your badge count:

| Badges | EVs per stat |
| --- | --- |
| 0 | 12 |
| 1 | 24 |
| 2 | 36 |
| 3 | 48 |
| 4 | 60 |
| 5 | 72 |
| 6 | 80 |
| 7 | 100 |
| 8 | 128 |

Four stats get it: HP, Speed, and whichever of Attack/Sp. Attack and Defense/Sp. Defense
the Pokémon is actually better at. That last part matters here specifically — the species
is randomized, so a fixed spread would land on the wrong half of the sheet about half the
time. Ported from `pokeemerald_rando_enh`, where it is the "scaling EVs" challenge.

The player is **not** capped (`B_EV_CAP_TYPE = EV_CAP_NONE`): EV training is worth doing,
it just is not free wins any more. Vitamins are in the mart, and the move relearner shows
Attack and Sp. Attack EVs so you can see what a Pokémon has been fed.

## 6. What is *not* enforced

Deliberately left to you, because the game cannot tell intent:

- **Nicknaming** — the base game's optional prompt. `RANDOLOCKE_FORCE_NICKNAME` makes it mandatory if you want the convention enforced; it ships off
- **Set battle style** — an options-menu choice
- **Item restrictions in battle** — the var `0x40F7` can disable the Bag in battles if you
  want that (1 = trainer battles, 2 = all)
- **Which Pokémon you actually use** — nothing stops you boxing a healthy one
