# Nuzlocke rules

Everything here is enforced by the game. Settings live in `include/config/randolocke.h`;
`RANDOLOCKE_FLAG_NUZLOCKE_OFF` (flag `0x2D`) switches the whole system off for a save.
The flag is **inverted on purpose**: clear means the rules are *on*, so a save made before
the rules existed gets them without a new game.

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

A Pokémon that faints is **gone**. At the end of the battle it is:

1. stripped of its held item, which goes back to your bag
2. marked as dead
3. moved into a PC box
4. removed from your party

Field poison kills the same way. Once marked, a Pokémon **cannot be withdrawn, moved or
shifted** — the PC says "This POKéMON is gone for good." It *can* be released, so the box
can be tidied.

Becoming Champion ends the run, and from that point the mark stops being enforced: every
Pokémon you lost is yours again.

The mark is stored in `marineRibbon`. Substruct 3 is exactly full at 96 bits, so a new
field would change the size of every boxed Pokémon and invalidate saves; `marineRibbon` is
never distributed in Emerald, so it is free. The side effect is cosmetic — a dead Pokémon
shows a Marine Ribbon.

### What is not covered

The battle types where the party is not really yours are exempt: link, recorded link, the
first battle, Wally's tutorial, in-game partner battles, the Battle Frontier, and the
Safari Zone. Losing a Pokémon in any of those does not kill it.

## 3. Wiping

When the whole party is down:

| Situation | What happens |
| --- | --- |
| A living Pokémon is left in a box | The first one is moved into your party, then the usual white-out: back to the last Pokémon Center |
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

## 5. What is *not* enforced

Deliberately left to you, because the game cannot tell intent:

- **Nicknaming** — forced on (`RANDOLOCKE_FORCE_NICKNAME`), but the name is yours
- **Set battle style** — an options-menu choice
- **Item restrictions in battle** — the var `0x40F7` can disable the Bag in battles if you
  want that (1 = trainer battles, 2 = all)
- **Which Pokémon you actually use** — nothing stops you boxing a healthy one
