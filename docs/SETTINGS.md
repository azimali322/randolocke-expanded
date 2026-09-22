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

With this TRUE, `NewGameInitData()` sets the eleven randomizer flags below on every new
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
| `RANDOLOCKE_DEFAULT_TUTOR_MOVES` | `0x02E` | Which move each of the ten town move tutors teaches |

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
#define RANDOLOCKE_DEFAULT_SPECIES_MODE     MON_RANDOM_BST
```

Written to `VAR_UNUSED_0x404E` on a new game. Controls *how* a species is substituted:

| Mode | Behaviour |
| --- | --- |
| `MON_RANDOM` | Anything can become anything — a Route 101 Zigzagoon can be Rayquaza |
| `MON_RANDOM_LEGEND_AWARE` | Legendaries only ever replace other legendaries |
| `MON_RANDOM_BST` | The replacement has a similar base stat total — a gentler curve. Randolocke's own behaviour, and the default. Wild encounters get their own window: see *Wild encounters* in section 3 |
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

An *area* is one region-map section: a route, a town, or a whole cave however many floors
it has. Land, surfing and fishing there share it. Places with no wild table — the legendary
sites, gift Pokémon, scripted battles, Wally's tutorial — are never restricted.

The flag is **inverted**: clear means the rules are on. That way a save made before the
rules existed gets them without a new game. Set `0x2D` in the debug menu to switch them
off for that save.

The per-area bits live in `SaveBlock1.caughtInArea`, carved out of the old dex-flag
filler at 0x988 rather than appended — so the save layout did not change.

**Permadeath** (`RANDOLOCKE_FAINT_COSTS_MON`): a Pokémon that faints is marked, boxed, and
cannot be withdrawn, moved or shifted until you are Champion. Its held item goes back to
the bag first. It can still be released. **Wiping** (`RANDOLOCKE_WIPE_COSTS_PARTY`,
`RANDOLOCKE_RUN_OVER_ON_WIPE`): the whole party is boxed the same way, and you pick a new
team from the PC if anything there is still alive. If nothing is, the run is over and the
game returns to the title screen — the save is never deleted.

`docs/NUZLOCKE.md` has the full rules, including how the level caps line up with each
boss's ace.

### The seed

```c
#define RANDOMIZER_SEED_IS_TRAINER_ID   TRUE
```

The randomizer is **deterministic**: the same seed always produces the same world. The
seed is your Trainer ID, chosen at new game. Two players who enter the same Trainer ID
get an identical run — this is how Randolocke races work. Set this to `FALSE` and the
seed comes from `VAR_UNUSED_0x40FA` / `0x40FB` instead, which you set by hand.

`RZ_TRAINER_ID_IS_SEED`, just below it in the same file, looks like the same switch, but
nothing reads it. Changing it does nothing.

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

The header also derives `RZ_TIER_WEIGHTED_MOVES`, `RZ_TIER_WEIGHTED_ABILITIES` and
`RZ_TIER_WEIGHTED_ITEMS` from these modes. They are TRUE unless the mode is `RZ_TIER_OFF`;
change the mode, not them.

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

`RANDOLOCKE_*` settings are in `include/config/randolocke.h`. `RZ_*` and `RANDOMIZER_*`
ones are in `include/config/randomizer.h`.

### `include/config/randolocke.h` — features written for this hack

| Setting | Default | What it does |
| --- | --- | --- |
| `RANDOLOCKE_FLAG_INFINITE_REPEL` | `FLAG_UNUSED_0x027` | Flag the **Repellant** key item toggles. While set, the repel counter never ticks down |
| `RANDOLOCKE_PORTA_HEAL_REVIVES` | `FALSE` | Whether the **Porta Heal** also revives fainted Pokémon. v1.1's default was "no" |
| `RANDOLOCKE_FIELD_MOVES_NEED_NO_USER` | `TRUE` | Use HM field moves with no party member that knows them. Badges still required. Kills HM slaves |
| `RANDOLOCKE_CATCH_RATE_PERCENT` | `150` | Multiplier on every species' base catch rate |
| `RANDOLOCKE_RELEARNER_SHOW_EVS` | `TRUE` | The move relearner shows Attack / Sp. Atk EVs where the "BATTLE MOVES" heading was, so you can pick a physical or special move without leaving the screen |
| `RANDOLOCKE_DUAL_REGISTERED_ITEMS` | `TRUE` | A second key item registered to **held** SELECT; a tap still uses the first. **Changes the save layout** |
| `RANDOLOCKE_SELECT_HOLD_FRAMES` | `60` | Frames SELECT must be held before the second item fires (60fps). In the debug ROM's battles, the same hold opens the battle debug menu, since a tap now replays the battle log |
| `RANDOLOCKE_FLASH_OPENS_REGI_CAVES` | `TRUE` | Flash substitutes for the Braille puzzles at the Sealed Chamber, Desert Ruins and Island Cave. The original puzzles still work |
| `RANDOLOCKE_REGI_CAVES_OPEN_AT_BADGE_8` | `TRUE` | With the eighth badge, the Desert Ruins, Island Cave and Ancient Tomb are open without visiting the Sealed Chamber: the first time Route 111, 105 or 120 loads after the badge, the doors open just as the Sealed Chamber would open them. Each cave's inner wall still takes its puzzle or Flash. Before the eighth badge the Sealed Chamber works as usual |
| `RANDOLOCKE_FORCE_NICKNAME` | `FALSE` | If TRUE, catching goes straight to the naming screen with no prompt. Off, so catching asks as the base game does |
| `RANDOLOCKE_SUMMARY_STAT_EDITOR` | `TRUE` | The summary's IV and EV pages can be edited in place. SELECT starts, A moves between stats, D-pad changes the one you are on: Up maxes, Down zeroes, Left/Right step by one. Held to 252 per stat, 510 total, 31 for IVs. Party Pokémon only |
| `RANDOLOCKE_TM_HOVER_INFO` | `TRUE` | The bag's TM move panel follows the cursor instead of waiting for a selection, and shows a physical/special icon |

### The four custom key items

Defined in `include/constants/items.h` (874–877):

| Item | Effect |
| --- | --- |
| **Repellant** | Toggleable infinite repel |
| **Porta Heal** | Heals the party anywhere. Reviving is off by default — see `RANDOLOCKE_PORTA_HEAL_REVIVES` |
| **Endless Candy** | A Rare Candy that is never consumed |
| **Cap Candy** | Levels a Pokémon straight to the current level cap |

### Story shortcuts

| Setting | Default | What it does |
| --- | --- | --- |
| `RANDOLOCKE_QUICK_START` | `TRUE` | The first morning skips its errands. Mom meets you at the truck, sets the time from the real-time clock (your own, in an emulator), hands over the Running Shoes and sends you to Birch. No bedroom clock, no Dad on TV, no visit next door, but whatever they would have set is set, so the rest of the story finds what it expects. New games only |
| `RANDOLOCKE_SKIP_WALLY_TUTORIAL` | `TRUE` | No Wally catching tutorial. The first time you enter Petalburg, the state it would leave behind is set instead, so the boy on the west side never walks you back to the gym. The gym is optional until you want the badge. Wally's later scenes are unaffected |
| `RANDOLOCKE_NO_DARK_AREAS` | `TRUE` | Nowhere is dark. Granite Cave B1F and B2F, Victory Road B1F and B2F and Dewford Gym are fully lit from the moment you walk in, so Flash is never needed to see; Dewford Gym's trainers no longer turn the lights up one by one. Flash no longer offers to light a cave, since there is nothing to light, and still opens the Regi chambers |

### Conveniences

| Setting | Default | What it does |
| --- | --- | --- |
| `RANDOLOCKE_DUAL_BIKE` | `TRUE` | Rydel hands over one **Bike**, and R while riding switches it between Mach and Acro. From pokeemerald_rando_enh. A save that already holds the Acro Bike keeps it, and R works on it too |
| `RANDOLOCKE_EASY_FISHING` | `TRUE` | The rod reels itself in once something bites, so nothing gets away and a stray A press no longer cancels the cast. One round of dots is always enough. Whether anything bites is still the usual roll. From Modern Emerald |
| `RANDOLOCKE_EASY_FISHING_REEL_DELAY` | `24` | Frames "Oh! A bite!" stays up before the rod reels in by itself. Pressing A reels in at once |
| `RANDOLOCKE_REPEATABLE_MOVE_TUTORS` | `TRUE` | The ten town tutors teach as often as you like, not once each. Each still teaches its own move, the same one every time. The Battle Frontier's tutors still charge BP |
| `RANDOLOCKE_UNIVERSAL_TM_COMPATIBILITY` | `TRUE` | Any Pokémon can learn any TM, HM or tutor move. Without it, a TM randomized onto a move no species lists is a TM nobody can use. The move relearner, Egg moves and the Frontier keep the real learnsets |
| `RANDOLOCKE_BERRY_YIELD_MULTIPLIER` | `4` | Multiplier on the berries a tree gives, capped at 255. Randomized trees rarely give the same berry twice, so a bigger handful is worth more |
| `RANDOLOCKE_DEFAULT_TEXT_SPEED` | `OPTIONS_TEXT_SPEED_FAST` | Text speed a new save starts on, instead of the stock `OPTIONS_TEXT_SPEED_MID`. The options menu can still change it |

### Items and gifts

| Setting | Default | What it does |
| --- | --- | --- |
| `RANDOLOCKE_RANDOMIZE_NPC_GIFTS` | `TRUE` | Items NPCs hand over are randomized like item balls, while the field items flag (`0x021`) is set. HMs and key items are never touched, so nothing the story needs can be lost |
| `RANDOLOCKE_RANDOMIZE_NPC_GIFT_BALLS` | `FALSE` | Whether Poké Balls from NPCs are randomized too. Off, so the five that start the run stay Poké Balls: under nuzlocke rules, balls are the scarcest thing in the game |
| `RANDOLOCKE_TM_PICKUPS_NO_DUPES` | `TRUE` | A randomized TM, whether found, hidden or given by an NPC, is drawn from the TMs you do not own yet. TMs are reusable, so a repeat is worth nothing. The PC counts as owned. Once you own them all, any TM can come up |

### Screens and menus

| Setting | Default | What it does |
| --- | --- | --- |
| `RANDOLOCKE_SKILLS_PAGE_ABILITY` | `TRUE` | The skills page shows the ability where the ribbon count was. The count was misleading anyway: a ribbon bit is what marks a fainted Pokémon here |
| `RANDOLOCKE_SKILLS_PAGE_FRIENDSHIP` | `TRUE` | Friendship as a plain "X/255" on the skills page, at the bottom right of the Pokémon's picture. It replaces the old heart, which never read clearly |
| `RANDOLOCKE_SUMMARY_NATURE_ROLL` | `TRUE` | Hold SELECT on the summary's info or skills page to re-roll the hidden nature, the one that actually changes the stats. On the skills page it works from the plain stats view only, since the IV and EV views use SELECT for the stat editor. Party Pokémon only, never an Egg or a rental |
| `RANDOLOCKE_SUMMARY_ABILITY_ROLL` | `TRUE` | Hold START on the same pages to re-roll the ability, with the same limits. A separate button, so you can settle the nature and then the ability without one undoing the other |
| `RANDOLOCKE_ROLL_HOLD_FRAMES` | `20` | Frames a roll button must be held before it fires. You have to let go before it fires again. Stops a stray tap from replacing an ability. `0` goes back to a tap |
| `RANDOLOCKE_ROLL_COST` | `0` | Money one roll costs. Free by default: it is meant as a cheat, not an economy |
| `RANDOLOCKE_MOVE_SCREEN_STATS` | `TRUE` | SELECT on the "which move should be forgotten?" screen swaps the picture for the six stats and the ability, so you can see whether it hits harder with Attack or Sp. Atk |
| `RANDOLOCKE_TM_MOVE_DESCRIPTIONS` | `TRUE` | A TM's bag description describes the move it actually teaches, not the one it taught in vanilla |
| `RANDOLOCKE_BATTLE_LOG` | `TRUE` | Tap SELECT at the battle menu to replay the battle's messages in the text box: everything said before the first turn, then everything since the last turn began. An ability pop-up gets a line naming the Pokémon and the ability, since the message after it often does not — Drizzle's is just "It started to rain!". A goes to the next message, B or SELECT closes it. Not in link battles |

### Player Pokémon IVs

| Setting | Default | What it does |
| --- | --- | --- |
| `RANDOLOCKE_PLAYER_IVS` | `RANDOLOCKE_IVS_PERFECT` | IVs on any Pokémon that becomes yours: caught, gifted, a starter, or hatched. Trainers are never affected. `RANDOLOCKE_IVS_PERFECT` is 31 in every stat, since a nuzlocke rarely gives you a second try at a Pokémon. `RANDOLOCKE_IVS_RANDOLOCKE` is Randolocke's own rule: a few perfect IVs on starters and gifts, random everywhere else. `RANDOLOCKE_IVS_VANILLA` leaves them alone |
| `RANDOLOCKE_GIFT_PERFECT_IVS` | `3` | How many perfect IVs a starter or gift gets under `RANDOLOCKE_IVS_RANDOLOCKE` |

### Legendary rules

All of these count the same 136 species: restricted legendaries, sub-legendaries,
mythicals and Ultra Beasts.

| Setting | Default | What it does |
| --- | --- | --- |
| `RANDOLOCKE_LEGENDARY_CATCH_RATE` | `45` | One catch rate for every legendary, in place of its own. It is the final rate, so `RANDOLOCKE_CATCH_RATE_PERCENT` does not change it. Most legendaries start at 3, which takes about ninety Ultra Balls at a quarter health; 45 takes about six. `0` leaves every species on its own rate |
| `RANDOLOCKE_LEGENDARY_CATCH_RATE_IS_FLOOR` | `FALSE` | If TRUE, the rate above only ever raises a catch rate. Legendaries already at 30 or more, such as Mew and Celebi, then keep their own |
| `RANDOLOCKE_LEGENDARY_CLAUSE` | `TRUE` | A legendary met in the wild is always catchable, like a shiny: in a used-up area, with its family already caught, and without using the area up. The twelve legendary sites have no wild table, so they were never restricted |
| `RANDOLOCKE_ELITE_FOUR_LEGENDARY_LIMIT` | `TRUE` | The League will not let you through to the Elite Four with more legendaries in the party than the limit below. Eggs do not count. It is checked on the tiles in front of the door, so it catches every attempt, not just the first. The Pokémon Center's PC is in the same room |
| `RANDOLOCKE_ELITE_FOUR_MAX_LEGENDARIES` | `1` | How many legendaries the League lets through |

### Nuzlocke details

The rules themselves are in section 1.

| Setting | Default | What it does |
| --- | --- | --- |
| `RANDOLOCKE_FLAG_RULES_BEGIN` | `FLAG_ADVENTURE_STARTED` | The flag that starts the rules. It is set when you get the five Poké Balls in Birch's lab, after the Route 103 battle. Before that you have one Pokémon and no balls, so there is nothing to rule on |
| `RANDOLOCKE_FAINT_COSTS_MON` | `TRUE` | A Pokémon that reaches 0 HP is boxed at the end of the battle and locked there until you are Champion. Its held item goes back to the bag first. Skipped where the party is not really yours: Birch's bag on Route 101, Wally's tutorial, Safari, link and recorded battles, an in-game partner, the Frontier. `FALSE` means only a wipe costs anything |
| `RANDOLOCKE_WIPE_COSTS_PARTY` | `TRUE` | A wipe costs the whole party: boxed and locked the same way, held items back to the bag. You pick a new team from the PC |
| `RANDOLOCKE_MON_DATA_FAINTED` | `MON_DATA_MARINE_RIBBON` | Where the fainted mark is stored. Emerald never hands out the Marine Ribbon, and a new field would change the size of every boxed Pokémon and break saves |
| `RANDOLOCKE_FIRST_ENCOUNTER_BADGE` | `TRUE` | A circled 1 on a wild Pokémon's health box when the rules would let you catch it. The rules apply either way; this saves throwing a ball to find out |

### Randolocke v1.1 NPCs

| Setting | Default | What it does |
| --- | --- | --- |
| `RANDOLOCKE_FLAG_OLDALE_MONEY_GIVEN` | `FLAG_UNUSED_0x02B` | Set once the Oldale financier has handed over his ₽999,999, so he only does it once |
| `RANDOLOCKE_MAP_SELLER_BADGE` | `FLAG_BADGE08_GET` | Badge the map seller in Slateport harbour waits for: the eighth, which is where Randolocke's other legendary unlocks land |
| `RANDOLOCKE_MAP_PRICE` | `1` | Price of each "legendary location map". They are the four event tickets, otherwise Mystery Gift only, so the price is nominal |
| `RANDOLOCKE_FLAG_HIDE_DEWFORD_OLD_ROD_FISHERMAN` | `FLAG_UNUSED_0x02C` | Hides the Old Rod fisherman in Dewford, since Randolocke moves him to Route 103 so you can fish before the first badge. A new game sets it. An older save keeps him in Dewford too, which is harmless |

### Debug ROM stability

| Setting | Default | What it does |
| --- | --- | --- |
| `RANDOLOCKE_DEBUG_ASSERTS_RESUME` | `TRUE` | A failed `AGB_ASSERT` in the debug ROM (`make`) is logged, then play carries on, as the release ROM does. Without it, mGBA returns from the assert's break into unrelated code and the game derails. Tests still fail on a failed assert |
| `RANDOLOCKE_SKIP_BAD_FREES` | `TRUE` | `Free()` refuses a block that is already free or has a bad header, and logs it instead of asserting. The log gives the caller's address, to look up with `arm-none-eabi-addr2line -f -e pokeemerald.elf`, and for a double free, the file and line that allocated the block. Both ROMs skip; only the debug ROM prints |

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
| `RZ_TM_MOVES_TIER_MODE` | `RZ_TIER_MODE_MOVES` | Tier mode for the move each TM teaches, when TM moves are randomized. Draws from the TM bands, never Bad or Pokemon Homeless, with no repeats. HMs are never touched |
| `RZ_TUTOR_MOVES_TIER_MODE` | `RZ_TM_MOVES_TIER_MODE` | The same for the ten move tutors. Never a move a TM already teaches |

### Randomized learnsets

Active only when flag `0x028` is set.

| Setting | Default | What it does |
| --- | --- | --- |
| `RZ_LEARNSET_STAB_MOVES` | `7` | STAB moves in the 21 |
| `RZ_LEARNSET_STATUS_MOVES` | `7` | Status moves |
| `RZ_LEARNSET_DAMAGING_MOVES` | `7` | Non-STAB damaging moves |
| `RZ_LEARNSET_LEVELS` | 1…86 | The level each slot is learned at. Front-loaded so early Pokémon are not moveless |
| `RZ_LEARNSET_SORT_BY_POWER` | `TRUE` | Each group is sorted by Base Power, so stronger moves are learned later. `FALSE` fills the slots in the order they roll |
| `RZ_STAB_MATCH_CATEGORY` | `TRUE` | A physical attacker draws physical STAB, a special attacker special STAB. Without this a pure physical attacker can roll seven special STAB moves and be unable to use any of them |
| `RZ_MIXED_ATTACKER_PERCENT` | `85` | How close base Attack and Sp. Atk must be, as a percentage of the higher, to count as **mixed** and draw STAB from both categories |

The three counts add up to `RZ_LEARNSET_SLOTS`. If you change them, give
`RZ_LEARNSET_LEVELS` one level per slot, and check `MAX_RELEARNER_MOVES`: the relearner's
list holds 64, enough for a three-stage family at 21 moves each.

### Wild encounters

Active while wild randomization (flag `0x020`) is on.

| Setting | Default | What it does |
| --- | --- | --- |
| `RZ_WILD_BST_FLOOR_PERCENT` | `100` | Lowest base stat total a wild replacement may have, as a percentage of the vanilla species'. At 100 a wild Pokémon is never weaker than what vanilla put in its slot. Only with the species mode at `MON_RANDOM_BST` |
| `RZ_WILD_BST_CEILING_PERCENT` | `125` | Highest, likewise. Trainer parties keep the stock window of ±10% |
| `RZ_WILD_LOTTERY` | `TRUE` | The lottery. The rarest land slots ignore the usual roll, in any species mode, and give the first stage of a line that ends in a 600-BST pseudo-legendary: Dratini, Larvitar, Bagon, Beldum, Gible, Deino, Goomy, Jangmo-o, Dreepy or Frigibax. A route's ticket is the same every time. The nuzlocke keeps your first encounter, and that is a ticket about 2% of the time |
| `RZ_WILD_LOTTERY_FROM_SLOT` | `10` | First land slot that is a ticket. `10` means slots 10 and 11, the two 1% slots |

### Trainer difficulty

Rolls are seeded from the trainer and the party slot, so a trainer's team is the same
every time you meet it. A *boss* is one of the 55 `Boss: Yes` trainers: gym leaders, the
Elite Four, the Champion, and Aqua and Magma's leaders and admins.

| Setting | Default | What it does |
| --- | --- | --- |
| `RANDOLOCKE_NO_BAG_VS_TRAINERS` | `TRUE` | No Bag in trainer battles: no Potions or Revives mid-fight, held items only. Wild battles keep the Bag, since the same check gates Poké Balls. Unlike `B_VAR_NO_BAG_USE` (section 4), it also applies to a save already in progress |
| `RZ_TRAINER_EV_SCALING` | `TRUE` | Trainer Pokémon get EVs that grow with your badges. In vanilla every trainer runs on zero, gym leaders included |
| `RZ_TRAINER_EVS_BY_BADGE` | `{ 24, 48, 72, 100, 140, 180, 220, 252, 252 }` | EVs per stat for 0 to 8 badges, in two stats: the attacking stat the species uses, and Speed if it is fast or HP if not. At 252 the spare 6 go to the better defence, for the legal 510 |
| `RZ_TRAINER_EV_SPEED_THRESHOLD` | `67` | Base Speed at which a trainer Pokémon counts as fast, for its EVs and its nature. 67 is the median base Speed in the game |
| `RZ_TRAINER_IVS` | `TRUE` | A boss's Pokémon get 31 in every stat. Everyone else rolls each stat from 0 to 31, instead of one number six times. The IVs in `trainers.party` are no longer read |
| `RZ_TRAINER_NATURES` | `TRUE` | Trainer Pokémon get the nature a player would pick, not Hardy. Fast ones trade their unused attacking stat for Speed (Jolly, Timid), slow ones for power (Adamant, Modest) |
| `RZ_TRAINER_HELD_ITEMS` | `TRUE` | A trainer Pokémon with no item may be given one that suits any species: Leftovers, Sitrus, Lum, Focus Band, Life Orb and so on, or the booster for its attacking category. No Choice items. Items written into `trainers.party` stay |
| `RZ_TRAINER_ITEM_CHANCE` | `35` | Percent chance an ordinary trainer's Pokémon gets one |
| `RZ_TRAINER_ITEM_CHANCE_BOSS` | `100` | The same for a boss's Pokémon |
| `RZ_TRAINER_REGENERATE_MOVES` | `TRUE` | A trainer Pokémon whose species was randomized gets moves from its own level-up learnset, not the moves written for the one it replaced. Without it, a gym leader's team keeps the old team's moves, with no same-type attacks |
| `RZ_TRAINER_AI_TIERS` | `TRUE` | Raises each trainer's AI flags at battle start by how important the trainer is, on top of its own. The tiers are the next four rows |
| `RZ_AI_BASE` | Check Bad Move, Try To Faint, Check Viability | Every trainer |
| `RZ_AI_NOTABLE` | Base + HP Aware, Smart Mon Choices, Try To 2HKO | Rivals: May or Brendan, and Wally |
| `RZ_AI_BOSS` | Notable + Smart Switching, Ace Pokémon, Omniscient | Bosses. Omniscient means the AI knows your moves, abilities and held items without having seen them |
| `RZ_AI_CHAMPION` | Boss + Predict Move, Predict Switch, Predict Incoming Mon | The Champion. It reads ahead: which move you are about to use, and when you will switch and to what |

### Tier weights

Weights ×100, best band first. Section 2 explains how weights become odds, and section 5
how to change one safely.

| Setting | Default | What it does |
| --- | --- | --- |
| `RZ_MOVE_W_META_DEFINING`, `RZ_MOVE_W_STAPLES`, `RZ_MOVE_W_FILLER`, `RZ_MOVE_W_NICHE`, `RZ_MOVE_W_BAD`, `RZ_MOVE_W_HOMELESS` | `107` / `1090` / `4209` / `4026` / `529` / `39` | Move bands, for randomized learnsets |
| `RZ_ABILITY_W_S`, `RZ_ABILITY_W_A`, `RZ_ABILITY_W_B`, `RZ_ABILITY_W_C`, `RZ_ABILITY_W_D`, `RZ_ABILITY_W_F` | `900` / `2000` / `3200` / `2400` / `1100` / `400` | Ability bands. `RZ_ABILITY_W_NEGATIVE` is `0`, so the Negative band never rolls |
| `RZ_ITEM_W_T1`, `RZ_ITEM_W_T2`, `RZ_ITEM_W_T3`, `RZ_ITEM_W_T4`, `RZ_ITEM_W_T5` | `118` / `4792` / `4146` / `537` / `407` | Field item tiers. Tier 4 is Poké Balls and evolution items, which the shop sells cheaply. Tier 5 is healing, vitamins and X items, and almost never rolls. Berries are not in this pool |
| `RZ_ITEM_W_TM_BAND` | `3000` | Weight of "a TM instead" against the five item tiers, for any randomized item that is not already a TM. 3000 against their 10000 makes about 23% of those items a TM |
| `RZ_TM_W_META_DEFINING`, `RZ_TM_W_STAPLES`, `RZ_TM_W_FILLER`, `RZ_TM_W_NICHE` | `900` / `5800` / `2500` / `800` | TM bands: the move each randomized TM and tutor teaches or, with TM moves not randomized, which TM a pickup is. They lean harder toward good moves than the move bands, since a TM is permanent. Over the 50 TMs this gives about 3 Meta Defining, 26 Staples, 16 Filler and 5 Niche |
| `RZ_BERRY_W_T1`, `RZ_BERRY_W_T2`, `RZ_BERRY_W_T3`, `RZ_BERRY_W_T4`, `RZ_BERRY_W_T5` | `1618` / `1676` / `5456` / `441` / `809` | Berry tree tiers. Pinch berries that raise a stat, and status cures, rank above HP restores, since the Porta Heal makes healing cheap. Berries with no hold effect come last |

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
| `VAR_UNUSED_0x40FA` / `0x40FB` | Manual seed | Only read when `RANDOMIZER_SEED_IS_TRAINER_ID` is `FALSE` |

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
