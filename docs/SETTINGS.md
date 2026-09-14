# Randolocke-Expanded — settings reference

Every knob in the hack, what it does, and where it lives.

## The three kinds of setting

| Kind | Where | When it takes effect | Can a player change it? |
| --- | --- | --- | --- |
| **Compile-time** | `#define` in `include/config/*.h` | Next `make` | No — it is baked into the ROM |
| **Flag** | `FlagSet`/`FlagClear`, per save | Immediately | Yes, via the debug menu |
| **Var** | `VarSet`, per save | Immediately | Yes, via the debug menu |

Flags and vars live **in the save file**. A new game starts them at the values in
`RandolockeSetDefaultRandomizerOptions()` (see below); after that they are yours to
change and they persist. Compile-time settings are the same for everyone playing the
same `.bps`.

**Debug menu:** hold **R** and press **START** in the overworld → **Flags & Vars**.
Flags are listed by number, so `0x20` is `FLAG_UNUSED_0x020`.

`DEBUG_OVERWORLD_MENU` is `DISABLED_ON_RELEASE`, which means:

| Build | Debug menu |
| --- | --- |
| `make` | **On** — this is what you test with |
| `make release` | **Off** |

So in the `.bps` you distribute there is no debug menu and no way for a player to change
a flag. Whatever `NewGameInitData()` sets on a new game is the whole game. That is why
the new-game defaults below matter, and why they are worth getting right before you cut
a release build.

---

## 1. What a new game turns on

`include/config/randolocke.h`, under *New game defaults*. This is the answer to "how do
I make the shipped ROM already be Randolocke".

```c
#define RANDOLOCKE_RANDOMIZE_ON_NEW_GAME    TRUE
```

With this TRUE, `NewGameInitData()` sets the ten randomizer flags below on every new
game. Existing saves are never touched. Each feature has its own switch:

| Setting | Sets flag | What gets randomized |
| --- | --- | --- |
| `RANDOLOCKE_DEFAULT_WILD_MON` | `0x020` | Every wild encounter — grass, surf, fishing, Rock Smash |
| `RANDOLOCKE_DEFAULT_TRAINER_MON` | `0x022` | Every trainer's party |
| `RANDOLOCKE_DEFAULT_FIXED_MON` | `0x023` | Scripted and static encounters: legendaries, Sudowoodo, the Voltorbs. Also gates the legendary pool below |
| `RANDOLOCKE_DEFAULT_STARTER_GIFT_MON` | `0x024` | The ten entries in `gStarterAndGiftMonTable`: the three Hoenn starters, the three Johto starters, Beldum, Castform, Lileep, Anorith. Already **without replacement** — no two are ever the same |
| `RANDOLOCKE_DEFAULT_EGG_MON` | `0x025` | The entries in `gEggMonTable` — in Emerald that is only the Wynaut egg. Not ordinary breeding |
| `RANDOLOCKE_DEFAULT_ABILITIES` | `0x026` | Every Pokémon's ability |
| `RANDOLOCKE_DEFAULT_FIELD_ITEMS` | `0x021` | Overworld item balls and hidden items |
| `RANDOLOCKE_DEFAULT_LEARNSET` | `0x028` | Level-up learnsets — the 21-move scheme |
| `RANDOLOCKE_DEFAULT_BERRY_TREES` | `0x029` | What grows on berry trees |
| `RANDOLOCKE_DEFAULT_TM_MOVES` | `0x02A` | Which move each TM teaches |

Set any one to `FALSE` to start with that feature off. It can still be switched on
in-game from the debug menu — that is the whole point of using flags rather than
`FORCE_RANDOMIZE_*`.

### The nuclear option: `FORCE_RANDOMIZE_*`

`include/config/randomizer.h` has a commented-out block:

```c
//#define FORCE_RANDOMIZE_WILD_MON    TRUE
```

Uncommenting one makes that feature *permanently* on (or off, with `FALSE`) and removes
its flag check entirely. It is smaller and slightly faster, but the debug menu can no
longer toggle it, which breaks most of `TESTING.md`. Prefer the new-game defaults until
you have finished testing.

### Species mode

```c
#define RANDOLOCKE_DEFAULT_SPECIES_MODE     MON_RANDOM
```

Written to `VAR_UNUSED_0x404E` on a new game. Controls *how* a species is substituted:

| Mode | Behaviour |
| --- | --- |
| `MON_RANDOM` | Anything can become anything. Randolocke's own behaviour — a Route 101 Zigzagoon can be Rayquaza |
| `MON_RANDOM_LEGEND_AWARE` | Legendaries only ever replace other legendaries |
| `MON_RANDOM_BST` | The replacement has a similar base stat total — a gentler curve |
| `MON_EVOLUTION` | The replacement sits at the same evolution stage |

Change it mid-run from the debug menu by writing the var; it changes what future rolls
produce, but Pokémon already caught keep what they are.

### Legendaries

```c
#define RANDOLOCKE_UNIQUE_LEGENDARIES   TRUE
```

Legendary encounters are handled apart from the species mode, for two reasons.

**They stay legendary.** With the mode at `MON_RANDOM`, a legendary *site* would otherwise
be able to hand you a Zigzagoon. This forces `MON_RANDOM_LEGEND_AWARE` for those twelve
sites only, so Rayquaza's spot always holds *some* legendary. Ordinary encounters are
unaffected — with `MON_RANDOM`, a legendary can still turn up in the grass.

**They are drawn without replacement.** All twelve sites are assigned at once from one
pool, so no two ever give the same species. Clear Rayquaza's slot and get Mew, and Mew is
then gone from every other site.

The twelve sites, in `gLegendaryMonTable` (`src/randomizer.c`):

| Site | Vanilla occupant | Reaches the randomizer via |
| --- | --- | --- |
| Sky Pillar | Rayquaza | `setwildbattle` |
| Terra Cave | Groudon | `setwildbattle` |
| Marine Cave | Kyogre | `setwildbattle` |
| Desert Ruins | Regirock | `setwildbattle` |
| Island Cave | Regice | `setwildbattle` |
| Ancient Tomb | Registeel | `setwildbattle` |
| Southern Island / roamer | Latias | `seteventmon` / `TryAddRoamer` |
| Southern Island / roamer | Latios | `seteventmon` / `TryAddRoamer` |
| Faraway Island | Mew | `seteventmon` |
| Birth Island | Deoxys | `seteventmon` |
| Navel Rock top | Ho-Oh | `seteventmon` |
| Navel Rock bottom | Lugia | `seteventmon` |

Jirachi is absent on purpose: it has no in-game encounter, so a slot for it would consume
a legendary nobody can reach.

The roaming Lati shares the Southern Island pair's slots, so the roamer and the island
cannot disagree about who is who.

Set this to `FALSE` and each site rolls independently through the ordinary species mode —
duplicates possible, and non-legendaries possible.

### Nuzlocke rules

```c
#define RANDOLOCKE_NUZLOCKE_RULES       TRUE
#define RANDOLOCKE_FLAG_NUZLOCKE_OFF    FLAG_UNUSED_0x02D
```

Enforced in-game rather than left to the player:

| Rule | Behaviour |
| --- | --- |
| **One per area** | One catch per wild-encounter area. After that, balls are refused there |
| **Dupes clause** | A species whose evolution family you already own cannot be caught — and meeting one does **not** use the area up, so you can keep looking |
| **Shiny clause** | A shiny is always catchable and never uses the area up |

An *area* is one entry in the wild encounter tables, which is one map. Land, surfing and
fishing on the same map share an area. Places with no wild table — the legendary sites,
gift Pokémon, scripted battles, Wally's tutorial — are never restricted.

The flag is **inverted**: clear means the rules are on. That way a save made before the
rules existed gets them without a new game. Set `0x2D` in the debug menu to switch them
off for that save.

The per-area bits live in `SaveBlock1.caughtInArea`, carved out of the old dex-flag
filler at 0x988 rather than appended — so the save layout did not change.

**Permadeath** (`RANDOLOCKE_PERMADEATH`): a Pokémon that faints is stripped of its held
item, marked, boxed, and cannot be withdrawn, moved or shifted until you are Champion. It
can still be released. **Wiping** (`RANDOLOCKE_RUN_OVER_ON_WIPE`): a living box Pokémon
takes over if there is one; if there is not, the run is over and the game returns to the
title screen — the save is never deleted.

`docs/NUZLOCKE.md` has the full rules, including how the level caps line up with each
boss's ace.

### The seed

```c
#define RZ_TRAINER_ID_IS_SEED       TRUE
```

The randomizer is **deterministic**: the same seed always produces the same world. The
seed is your Trainer ID, chosen at new game. Two players who enter the same Trainer ID
get an identical run — this is how Randolocke races work. Set this to `FALSE` and the
seed comes from `VAR_UNUSED_0x40FA` / `0x40FB` instead, which you set by hand.

---

## 2. Weighted vs Strict — the tier system

This is the part worth reading carefully.

### The problem

Uniform randomization is boring in a specific way. There are 846 rollable moves. Most
are Fury Swipes and Splash. If every move is equally likely, almost every move you see
is filler, and the rare good one feels like a lottery win rather than a build.

The tier system fixes this by drawing from **community tier lists** instead of a flat
list. Six bands for moves (Meta Defining → Pokemon Homeless), six for abilities (S → F
plus a Negative band), five for items, four for TMs, five for berries. The transcribed
lists live in `docs/tiering/`; the generated C tables in `src/data/randomizer/`.

### The three modes

Each pool has its own mode in `include/config/randomizer.h`:

```c
#define RZ_TIER_MODE_MOVES          RZ_TIER_WEIGHTED
#define RZ_TIER_MODE_ABILITIES      RZ_TIER_WEIGHTED
#define RZ_TIER_MODE_ITEMS          RZ_TIER_WEIGHTED
#define RZ_TIER_MODE_TMS            RZ_TIER_WEIGHTED
#define RZ_TIER_MODE_BERRIES        RZ_TIER_WEIGHTED
```

| Mode | What it does |
| --- | --- |
| **`RZ_TIER_OFF`** | Ignore the tiers. Every entry equally likely. Stock tertu behaviour |
| **`RZ_TIER_WEIGHTED`** | Pick a *band* according to its weight, then pick uniformly inside that band. Good things are commoner, but **anything can still appear** |
| **`RZ_TIER_STRICT`** | Only the top `RZ_STRICT_TIERS` bands exist. Everything below is unreachable |

They are independent — Strict abilities with Weighted moves is fine.

### How Weighted actually works

Two draws, not one:

1. Roll a band, in proportion to its weight.
2. Roll uniformly among the members of that band.

The consequence that trips people up: **a band's weight is split across its members**,
so a big band dilutes itself. Meta Defining has 4 moves and weight 107 (1.07% of the total);
Staples has 46 and weight 1090 (10.9%). Staples takes ten times the total share, yet per
*move* Meta Defining is still the likelier draw, because 1.07/4 beats 10.9/46. Compare
weights only after dividing by the band's size.

That is exactly the mistake `tools/randolocke/tier_report.py` exists to catch — it
prints, per band, the per-entry percentage, the multiplier versus uniform, and the
band's share of the pool. **Re-run it after changing any weight.** It has already caught
two real inversions (an A ability out-drawing an S; the leftovers band out-drawing Bad).

Current move weights give roughly:

| Band | Moves | Weight | Per-entry | vs uniform |
| --- | --- | --- | --- | --- |
| Meta Defining | 4 | 1.07% | 0.2675% | **2.26×** |
| Staples | 46 | 10.9% | 0.2370% | 2.00× |
| Filler/Outclassed | 203 | 42.09% | 0.2073% | 1.75× |
| Niche | 383 | 40.26% | 0.1051% | 0.89× |
| Bad | 186 | 5.29% | 0.0284% | 0.24× |
| Pokemon Homeless | 22 | 0.39% | 0.0177% | 0.15× |

Abilities, for comparison — 308 in the pool, uniform would be 0.3247% each:

| Band | Abilities | Weight | Per-entry | vs uniform |
| --- | --- | --- | --- | --- |
| S | 12 | 9% | 0.7500% | **2.31×** |
| A | 31 | 20% | 0.6452% | 1.99× |
| B | 78 | 32% | 0.4103% | 1.26× |
| C | 87 | 24% | 0.2759% | 0.85× |
| D | 64 | 11% | 0.1719% | 0.53× |
| F | 36 | 4% | 0.1111% | 0.34× |
| Negative | 7 | 0 | never | — |

Nothing is zero, so a Splash is still possible — just rare. The one exception is
`RZ_ABILITY_W_NEGATIVE = 0`: Truant and friends are never rolled at all.

### How Strict differs

Strict truncates the list to the top `RZ_STRICT_TIERS` bands (default 2) and draws from
those only. For moves that is Meta Defining + Staples: 50 moves total.
For abilities, S + A: 43.

The trade-off is **repetition**. Fifty moves across a six-Pokémon party, four moves
each, is 24 draws from a 50-item pool — you will see duplicates constantly, and the same
five abilities on everything. Strict is a *challenge-run* setting: it makes every
Pokémon strong and every Pokémon samey. Weighted is the one to actually play.

### One-line summary

> **Weighted** biases the dice. **Strict** takes most of the dice away.

---

## 3. Compile-time settings by area

### `include/config/randolocke.h` — features written for this hack

| Setting | Default | What it does |
| --- | --- | --- |
| `RANDOLOCKE_FLAG_INFINITE_REPEL` | `FLAG_UNUSED_0x027` | Flag the **Repellant** key item toggles. While set, the repel counter never ticks down |
| `RANDOLOCKE_PORTA_HEAL_REVIVES` | `FALSE` | Whether the **Porta Heal** also revives fainted Pokémon. v1.1's default was "no" |
| `RANDOLOCKE_FIELD_MOVES_NEED_NO_USER` | `TRUE` | Use HM field moves with no party member that knows them. Badges still required. Kills HM slaves |
| `RANDOLOCKE_CATCH_RATE_PERCENT` | `150` | Multiplier on every species' base catch rate |
| `RANDOLOCKE_RELEARNER_SHOW_EVS` | `TRUE` | The move relearner shows Attack / Sp. Atk EVs where the "BATTLE MOVES" heading was, so you can pick a physical or special move without leaving the screen |
| `RANDOLOCKE_DUAL_REGISTERED_ITEMS` | `TRUE` | A second key item registered to **held** SELECT; a tap still uses the first. **Changes the save layout** |
| `RANDOLOCKE_SELECT_HOLD_FRAMES` | `20` | Frames SELECT must be held before the second item fires (60fps) |
| `RANDOLOCKE_FLASH_OPENS_REGI_CAVES` | `TRUE` | Flash substitutes for the Braille puzzles at the Sealed Chamber, Desert Ruins and Island Cave. The original puzzles still work |
| `RANDOLOCKE_FORCE_NICKNAME` | `TRUE` | Catching goes straight to the naming screen, no prompt |
| `RANDOLOCKE_TM_HOVER_INFO` | `TRUE` | The bag's TM move panel follows the cursor instead of waiting for a selection, and shows a physical/special icon |

### The four custom key items

Defined in `include/constants/items.h` (874–877):

| Item | Effect |
| --- | --- |
| **Repellant** | Toggleable infinite repel |
| **Porta Heal** | Heals the party anywhere. Reviving is off by default — see `RANDOLOCKE_PORTA_HEAL_REVIVES` |
| **Endless Candy** | A Rare Candy that is never consumed |
| **Cap Candy** | Levels a Pokémon straight to the current level cap |

### `include/config/caps.h` — level caps

| Setting | Value | Meaning |
| --- | --- | --- |
| `B_EXP_CAP_TYPE` | `EXP_CAP_HARD` | Over the cap, a Pokémon gains **no** experience at all. `EXP_CAP_SOFT` reduces it instead; `EXP_CAP_NONE` disables caps |
| `B_LEVEL_CAP_TYPE` | `LEVEL_CAP_FLAG_LIST` | The cap comes from the badge table in `src/caps.c` |
| `B_RARE_CANDY_CAP` | `TRUE` | Rare Candies cannot push past the cap |

The table itself is `sLevelCapFlagMap` in `src/caps.c` — **edit the caps there, not in the
header**:

| Badges | Cap |
| --- | --- |
| 0 | 14 |
| 1 | 21 |
| 2 | 24 |
| 3 | 29 |
| 4 | 36 |
| 5 | 43 |
| 6 | 47 |
| 7 | 50 |
| 8 | 63 |
| Champion | 100 |

### Randomizer behaviour — `include/config/randomizer.h`

| Setting | Default | What it does |
| --- | --- | --- |
| `RANDOMIZER_AVAILABLE` | `TRUE` | Master switch. `FALSE` compiles the randomizer out entirely |
| `RZ_ABILITY_STABLE_ACROSS_EVOLUTION` | `TRUE` | **Enhancement 1.** A Pokémon's ability is decided by the root of its evolution family, so evolving no longer rerolls it. Separate from turning ability randomization off |
| `RZ_RANDOMIZE_BOSS_TRAINERS` | `TRUE` | Whether trainers tagged `Boss: Yes` (gym leaders, E4, Champion, Magma/Aqua leadership) are randomized. `FALSE` keeps their designed teams as fixed landmarks |
| `RANDOMIZER_DYNAMIC_SPECIES` | `TRUE` | Build the substitution tables in EWRAM at runtime. Costs 6 bytes per species |
| `RANDOMIZER_MAX_TM` | `ITEM_TM50` | Highest TM the randomizer will hand out. TM51–TM100 are unassigned placeholders in 1.17, so raising this gives out useless TMs |
| `RZ_STRICT_TIERS` | `2` | How many top bands Strict mode keeps |

### Randomized learnsets

Active only when flag `0x028` is set.

| Setting | Default | What it does |
| --- | --- | --- |
| `RZ_LEARNSET_STAB_MOVES` | `7` | STAB moves in the 21 |
| `RZ_LEARNSET_STATUS_MOVES` | `7` | Status moves |
| `RZ_LEARNSET_DAMAGING_MOVES` | `7` | Non-STAB damaging moves |
| `RZ_LEARNSET_LEVELS` | 1…86 | The level each slot is learned at. Front-loaded so early Pokémon are not moveless |
| `RZ_STAB_MATCH_CATEGORY` | `TRUE` | A physical attacker draws physical STAB, a special attacker special STAB. Without this a pure physical attacker can roll seven special STAB moves and be unable to use any of them |
| `RZ_MIXED_ATTACKER_PERCENT` | `85` | How close base Attack and Sp. Atk must be, as a percentage of the higher, to count as **mixed** and draw STAB from both categories |

### Quality of life adopted from Modern Emerald

| Setting | File | Value | Effect |
| --- | --- | --- | --- |
| `OW_AUTO_RUN` | `overworld.h` | `TRUE` | Always run, no B |
| `TEXT_SPEED_FAST_MODIFIER` | `text.h` | `18` | Near-instant text on FAST |
| `I_REUSABLE_TMS` | `item.h` | `TRUE` | TMs are permanent |
| `P_CAN_FORGET_HIDDEN_MOVE` | `pokemon.h` | `TRUE` | HMs can be forgotten |
| `P_SUMMARY_SCREEN_IV_EV_INFO` | `summary_screen.h` | `TRUE` | Cycle Stats / IVs / EVs on the skills page |
| `P_SUMMARY_SCREEN_IV_EV_VALUES` | `summary_screen.h` | `TRUE` | Show real numbers, not letter grades |
| `P_ENABLE_MOVE_RELEARNERS` | `pokemon.h` | `TRUE` | Egg, TM and tutor relearners |
| `P_PRE_EVO_MOVES` | `pokemon.h` | `TRUE` | Learn pre-evolution moves |
| `P_ENABLE_ALL_LEVEL_UP_MOVES` | `pokemon.h` | `TRUE` | Relearn any level-up move regardless of level |
| `P_TM_MOVES_RELEARNER` / `P_ENABLE_ALL_TM_MOVES` | `pokemon.h` | `TRUE` | Relearn any compatible TM move without owning the TM |
| `B_SHOW_TYPES` | `battle.h` | `SHOW_TYPES_ALWAYS` | Type indicators next to HP bars |
| `B_DARK_VOID_FAIL` | `battle.h` | `GEN_6` | Any species may use Dark Void — it is a rollable move here |
| `P_MEGA_EVOLUTIONS`, `P_PRIMAL_REVERSIONS`, `P_ULTRA_BURST_FORMS`, `P_GIGANTAMAX_FORMS`, `P_TERA_FORMS`, `P_FUSION_FORMS` | `species_enabled.h` | `FALSE` | Gimmick forms off — they do not interact sanely with randomized species |

---

## 4. In-game vars

| Var | Setting | Values |
| --- | --- | --- |
| `VAR_UNUSED_0x404E` | Species mode | 0 `MON_RANDOM`, 1 `LEGEND_AWARE`, 2 `BST`, 3 `EVOLUTION` |
| `VAR_UNUSED_0x40F7` | `B_VAR_NO_BAG_USE` | 0 off, 1 no Bag in trainer battles, 2 no Bag in any battle |
| `VAR_UNUSED_0x40FA` / `0x40FB` | Manual seed | Only read when `RZ_TRAINER_ID_IS_SEED` is `FALSE` |

---

## 5. Changing a tier weight

1. Edit the worksheet in `docs/tiering/` (moves and abilities) or the heuristic in
   `tools/randolocke/gen_item_tiers.py` (items, TMs, berries).
2. Re-run the matching generator, e.g. `python3 tools/randolocke/gen_move_tiers.py`.
3. Edit the `RZ_*_W_*` weights in `include/config/randomizer.h`.
4. **Run the report** and read the `vs uniform` column. It ends with either
   "per-entry odds decrease monotonically down the tiers" or a warning naming the
   inversion.

   ```bash
   python3 tools/randolocke/tier_report.py            # abilities
   python3 tools/randolocke/tier_report.py --moves    # moves
   ```

   For the **TM** bands use the simulator instead — duplicate rejection makes the naive
   `50 x weight / total` badly wrong for Staples, so weights have to be solved against the
   real algorithm:

   ```bash
   python3 tools/randolocke/tm_band_sim.py            # what the current weights produce
   python3 tools/randolocke/tm_band_sim.py 3 25 15 5  # solve for these TM counts
   ```
5. `python3 tools/randolocke/validate_tiers.py` to confirm nothing is untiered or
   duplicated.
6. `make`.
