# Randolocke-Expanded — In-Game Test Plan

Companion to [RANDOLOCKE_PLAN.md](RANDOLOCKE_PLAN.md). Tests are added phase by phase as
the project progresses. Run the tests for the phase you just built, **plus the regression
set** (§R) every time.

**How to run the ROM:** see §B below to build, then open `pokeemerald.gba` in mGBA. See §P
for patching/distribution. You do **not** need to patch anything to test your own build.

**Recommended emulator:** [mGBA](https://mgba.io/) — it has a save-state system, a memory
viewer, and prints `MgbaPrintf` debug output from the randomizer (View → Log).

**Debug menu:** hold `R` and press `START` in the overworld (`DEBUG_OVERWORLD_MENU` in
`include/config/debug.h`).

---

## §B — Building

Open Terminal and run:

```bash
make -j$(sysctl -n hw.ncpu)
```

That's it. `-j$(sysctl -n hw.ncpu)` builds in parallel across all your CPU cores; plain
`make` works too, just slower.

**No environment setup is needed.** `DEVKITPRO` and `DEVKITARM` are exported from your
`~/.zshrc`, and the Makefile falls back to `/opt/devkitpro/devkitARM` if they are not set,
so the build works in any shell. The Makefile puts the toolchain on `PATH` itself, so
`arm-none-eabi-gcc` does not need to be on your `PATH` beforehand.

### `arm-none-eabi-gcc: command not found`

```
bash: arm-none-eabi-gcc: command not found
make: *** [pokeemerald.elf] Error 127
```

This means the Makefile could not find the toolchain. Almost always it is a **stale shell**:
a Terminal window opened *before* `~/.zshrc` was edited keeps the old environment forever.

Fix, in that terminal:

```bash
source ~/.zshrc
```

Or just open a new Terminal tab. The Makefile fallback should prevent this entirely now; if
you still see it, check that `/opt/devkitpro/devkitARM/bin/arm-none-eabi-gcc` exists.

### What success looks like

The last lines should be a memory table followed by the `gbafix` steps:

```
Memory region         Used Size  Region Size  %age Used
           EWRAM:      235896 B       256 KB     89.99%
           IWRAM:       28388 B        32 KB     86.63%
             ROM:    26730796 B        32 MB     79.66%
...
arm-none-eabi-objcopy -O binary pokeemerald.elf pokeemerald.gba
```

The playable ROM is **`pokeemerald.gba`** in the repo root. Drag it into mGBA.

### Timings

- **First build after `make clean`:** 10-20 minutes (builds `tools/`, then ~3000 objects).
- **Incremental build** after editing a few files: seconds to a couple of minutes.
- Editing a header like `include/pokemon.h` rebuilds most of the tree — expect several
  minutes.

### Useful variants

| Command | Use |
| --- | --- |
| `make -j$(sysctl -n hw.ncpu)` | normal build |
| `make clean` | wipe all build output; forces a full rebuild |
| `make -j$(sysctl -n hw.ncpu) -k` | keep going after errors, to see *all* of them at once |
| `make clean-teachables` | regenerate `all_learnables.json` (only relevant to the deferred move-randomization work) |

### If a build fails

1. **Read the first `error:`, not the last.** With `-j` the output interleaves, so the last
   line is rarely the real cause. `make -j8 2>&1 | grep -m1 "error:"` finds it.
2. **After changing a config `#define`**, a normal `make` is enough; the build tracks
   header dependencies.
3. **`P_SUMMARY_SCREEN_IV_EV_TILESET`** is the documented exception — changing that one
   needs `make clean` first.
4. **Conflict markers** (`<<<<<<<`) produce a flood of nonsense syntax errors. Check with
   `grep -rn '^<<<<<<<' src/ include/`.

---

## Legend

- **PASS** — works as intended, move on.
- **FAIL** — file it; note the phase, the step, and what you saw instead.
- **EXPECTED BROKEN** — known-not-yet-implemented at this phase. **Not a bug.** Do not
  chase these; they are listed so you don't waste time.

---

## §R — Regression set (run every phase)

| # | Test | Steps | Expected |
| --- | --- | --- | --- |
| R1 | ROM boots | Load `pokeemerald.gba` in mGBA | Title screen appears, no white/black screen, no crash |
| R2 | New game starts | New Game → through intro → step outside | You reach Littleroot with a controllable player |
| R3 | Save works | Save in the player's house, then reset the emulator | Save loads, player is where you saved |
| R4 | Battle runs | Get a Pokémon, enter a wild battle, attack, win | Battle plays through with no freeze or corrupt text |
| R5 | Debug menu opens | Hold `R` + `START` | Debug menu appears and closes cleanly |
| R6 | No corrupt text | Read several NPC dialogues and menus | No stray `?`, `-`, or garbage glyphs |
| R7 | Build fits memory | Read the `make` memory table | EWRAM/IWRAM/ROM all under 100%; note any jump over ~2 points from last phase |

> **Note on R3:** several features are flagged "Requires New Game." Whenever you change
> save-data structures, **start a fresh save** rather than loading an old one, and expect
> old saves to be invalid. Keep one known-good save state per phase for comparison.

---

## §F — Enabling the randomizer (read this before testing randomization)

**Every randomizer feature is flag-gated and defaults to OFF.** `RandomizerFeatureEnabled()`
in `src/randomizer.c` calls `FlagGet(RANDOMIZER_FLAG_*)`, and flags start clear on a new
game. A fresh save is a *normal* Emerald until you set these.

This is why picking up the Route 102 Potion gives you a Potion: field-item randomization is
simply switched off, not broken.

| Feature | Flag | Hex |
| --- | --- | --- |
| Wild Pokémon | `RANDOMIZER_FLAG_WILD_MON` | `0x20` |
| Field items | `RANDOMIZER_FLAG_FIELD_ITEMS` | `0x21` |
| Trainer Pokémon | `RANDOMIZER_FLAG_TRAINER_MON` | `0x22` |
| Fixed encounters | `RANDOMIZER_FLAG_FIXED_MON` | `0x23` |
| Starters & gifts | `RANDOMIZER_FLAG_STARTER_AND_GIFT_MON` | `0x24` |
| Egg Pokémon | `RANDOMIZER_FLAG_EGG_MON` | `0x25` |
| Abilities | `RANDOMIZER_FLAG_ABILITIES` | `0x26` |
| Learnsets (21 moves) | `RANDOMIZER_FLAG_LEARNSET` | `0x28` |
| Berry trees | `RANDOMIZER_FLAG_BERRY_TREES` | `0x29` |

**To turn them on for a test session:** Debug menu → Flags & Vars → Flags, and set the hex
values above.

**To turn them on permanently** (recommended once you reach parity work): uncomment the
matching `FORCE_RANDOMIZE_*` defines in `include/config/randomizer.h`, which override the
flags entirely.

### ⚠️ The seed is your Trainer ID

`RZ_TRAINER_ID_IS_SEED` is `TRUE`, so `GetRandomizerSeed()` returns your Trainer ID. Two
consequences:

- Two saves with the same Trainer ID get **identical** randomization. That is the intended
  design and it is what makes runs shareable.
- **A Trainer ID of `00000` means a seed of 0.** If you see that, you likely started via
  debug quickstart rather than a real New Game — `InitPlayerTrainerId()` in
  `src/new_game.c` only runs on the normal new-game path. Start a proper New Game before
  judging any randomization result.

---

## Phase 1 — 1.17 merge base

**What this build is:** expansion 1.17.1 with the randomizer merged and *compiling*.
Randomizer hooks are only partially ported. The goal here is to prove the **base** is sane
before Phase 3 changes behaviour.

### EXPECTED BROKEN in Phase 1 — do not file these

- **Trainer Pokémon are probably NOT randomized.** The trainer hook (`§5.2` in the plan)
  is not ported yet.
- **Pokédex area screen** does not show randomized locations (`MonListHasSpecies` was
  restored to upstream's version; re-lands in Phase 3).
- Any randomizer feature may be inert. Phase 1 makes no correctness claims.

### Tests

| # | Test | Steps | Expected |
| --- | --- | --- | --- |
| 1.1 | Full regression | Run §R1–R7 | All pass |
| 1.2 | Version reads 1.17.1 | Debug menu → Utilities → Expansion Version | Shows **1.17.1** |
| 1.3 | New 1.17 species exist | Debug → Give → Pokémon (Basic), scroll to the end of the species list | Gen 9 / Legends Z-A species are present (~1,679 total) |
| 1.4 | Trainer data compiles correctly | Battle May/Brendan on Route 103 | Trainer has a valid party; no `SPECIES_NONE`, no crash. *(Species may be un-randomized — see above)* |
| 1.5 | `Boss:` key survived | `grep -n "Boss:" src/data/trainers.party` then battle that trainer | Builds without a trainerproc error; the battle runs |
| 1.6 | Debug Edit Pokemon submenu | Debug → Party → Edit Pokemon | Submenu opens with Set Hidden Nature / Set Friendship / Set Ability |
| 1.7 | Set Ability works | Edit Pokemon → Set Ability on a party mon, then check the summary | Ability changes and persists |
| 1.11 | Set Hidden Nature (Mint) | Edit Pokemon → Set Hidden Nature, then open the summary | ⚠️ The **displayed nature does NOT change** — that is correct Mint behaviour. What *should* change is the **stat numbers** and the red/blue stat arrow colouring. Compare stats before and after |
| 1.8 | Wild encounters work | Walk in grass on Route 101 until an encounter | A wild battle starts with a valid species |
| 1.9 | Hidden encounters (dexnav bug fix) | If DexNav is enabled, search for a hidden encounter | Hidden search returns hidden-table species, not water species |
| 1.10 | Save/reload stability | Save, reset, reload, walk around, enter a battle | No corruption; party intact |

### Phase 1 baseline capture (do this once)

Record these so later phases have something to compare against:

- [ ] Note your **Trainer ID** (randomizer seed — `RZ_TRAINER_ID_IS_SEED` is `TRUE`)
- [ ] Screenshot your starter's species and ability
- [ ] Screenshot the first three wild species you meet on Route 101
- [ ] Save an mGBA save state named `phase1-baseline`
- [ ] Record the `make` memory table numbers

---

## Phase 2 — Enum conversion

**What this build is:** the randomizer's API converted from `u16` to 1.17's typed enums
(`enum Species` / `enum Item` / `enum Ability` / `enum Type`). This is a **type-safety
change only** — no behaviour should differ from Phase 1 in any way.

**The whole point of these tests is to prove nothing changed.** If you see *any* behavioural
difference from your Phase 1 baseline, that is a bug, because this phase should be inert.

### EXPECTED BROKEN in Phase 2 — unchanged from Phase 1

Same list as Phase 1: trainer randomization and the Pokédex area screen are still
unported. Nothing in Phase 2 addresses them.

### Tests

| # | Test | Steps | Expected |
| --- | --- | --- | --- |
| 2.1 | Full regression | Run §R1–R7 | All pass |
| 2.2 | Memory did not move | Compare the `make` memory table against Phase 1 | **Byte-identical.** Enums lower to the same integers; any change means something real was altered |
| 2.3 | Same seed, same starter | Load `phase1-baseline` save state; check starter species + ability | Identical to your Phase 1 screenshot |
| 2.4 | Same seed, same wild mons | From the same state, walk Route 101 grass | The same species you recorded in Phase 1 |
| 2.5 | Trainer ID unchanged | Check your Trainer ID | Same as Phase 1 (it is the randomizer seed) |
| 2.6 | Item randomization stable | **First set flag `0x21`** (see §F), then pick up the Route 102 Potion | Same item as Phase 1 *for the same Trainer ID*. Without the flag set you correctly get a plain Potion |
| 2.7 | Ability display correct | Open a party mon's summary | Ability name renders correctly, not a number or blank |
| 2.8 | No enum truncation | Give yourself a **high-ID** species (Debug → Give → Pokémon, pick a Gen 9 / Z-A mon) | Correct species appears; not `SPECIES_NONE`, not a wrong mon. *This is the key test — it would catch an enum narrowed to the wrong width* |
| 2.9 | High-ID item | Debug → Give item, pick a high-ID item | Correct item, correct name and icon |

### Why 2.8 and 2.9 matter

Converting `u16` to an enum changes the type the compiler uses for storage and comparison.
If any conversion silently narrowed a value, the failure would only show up at **high IDs**
— above 255, or above 1,627 for the species added in 1.17. Low-ID Pokémon like Treecko
would keep working and hide the bug. Always test at the top of the range.

---

## Phase 3 — Randomizer hooks re-landed

**What this build is:** the first build where the ROM should actually *behave* like a
randomizer. Trainer parties and hidden items are reconnected.

### ⚠️ Read §F first

Nothing below will do anything until you **set the randomizer flags** and start from a
**real New Game** (not debug quickstart — a Trainer ID of `00000` means a seed of 0).

Set at minimum: `0x20` (wild), `0x21` (field items), `0x22` (trainer), `0x26` (abilities).

### Still EXPECTED BROKEN

- **Pokédex area screen** still does not show randomized locations (Phase 3 leftover).
- Move/TM/tutor randomization — deferred to v1.1 by design, was never in tertu.

### Tests

| # | Test | Steps | Expected |
| --- | --- | --- | --- |
| 3.1 | Full regression | Run §R1–R7 | All pass |
| 3.2 | **Trainer mons randomize** | Set flag `0x22`, New Game, battle the first trainer on Route 103 | Their Pokémon are **not** the vanilla species |
| 3.3 | **Trainer parties are stable** | Note the trainer's team, then reset (File → Reset, not a save state) and re-battle | **Identical team.** This is the single most important test — it proves the seed re-plumbing works |
| 3.4 | Stable across save/reload | Save, power-cycle, reload, re-battle a trainer | Same team again |
| 3.5 | Per-slot variety | Battle a trainer with 3+ mons | Slots differ from one another (not the same species repeated) |
| 3.6 | Different trainers differ | Battle two different trainers | Different randomized teams |
| 3.7 | Boss trainers exempt | Mark a trainer `Boss: Yes` in `trainers.party`, rebuild, battle them | Their team is the **designed** one, un-randomized |
| 3.8 | Boss abilities exempt | Same boss trainer, check abilities in battle | Abilities are the species' real ones, not randomized |
| 3.9 | Ability follows randomized species | Battle a normal trainer, note a mon's ability | Ability is legal for the **randomized** species, not the designed one |
| 3.10 | **Hidden items randomize** | Set flag `0x21`, find a hidden item (e.g. with the Itemfinder/Dowsing Machine) | Item differs from vanilla |
| 3.11 | **Hidden coins still work** | Find a hidden-coins spot in the Game Corner area | You receive **Coins**, not a randomized item. *This verifies the callnative sits after the coins branch* |
| 3.12 | Visible items still randomize | Pick up a visible item ball | Randomized (this path was never broken) |
| 3.13 | Partner party unaffected | Reach a multi-battle with Steven | Steven's team is his designed one, not randomized garbage |
| 3.14 | Debug battles unaffected | Debug → Party → Start Debug Battle | Runs normally; debug trainers pass `TRAINER_NONE` |
| 3.15 | Wild encounters still fine | Set flag `0x20`, walk in grass | Randomized and stable across reloads |

### Why 3.3 and 3.11 matter most

**3.3** — tertu seeded trainer randomization from `trainerNum`, and 1.17 removed that
parameter entirely. The whole hook was rebuilt around carrying the id on
`struct TrainerGenerator`. If the seed is not reaching the randomizer, teams will reroll
every battle instead of staying fixed. Use a true **reset**, not a save-state reload.

**3.13** — `MakePartnerGenerator` writes into an uninitialized stack local. If its new
fields were ever left unset, Steven's party would randomize on a garbage seed. A sane
Steven team confirms the initialization.

**3.11** — the hidden-item hook was deliberately placed *after* upstream's coins branch,
because `VAR_0x8005 == 0` means coins rather than an item. Randomizing before that check
would turn coin piles into random items.

---

## Phase 7a — Hard level caps (landed early)

**What this build is:** Randolocke's per-badge hard level caps, using 1.17's built-in cap
system (`include/config/caps.h`, `src/caps.c`). No custom code.

```c
B_EXP_CAP_TYPE    = EXP_CAP_HARD        // at/over the cap, no experience at all
B_LEVEL_CAP_TYPE  = LEVEL_CAP_FLAG_LIST // cap comes from the badge flag table
B_RARE_CANDY_CAP  = TRUE                // Rare Candy cannot push past the cap
```

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
| Elite Four beaten | 100 |

### Tests

| # | Test | Steps | Expected |
| --- | --- | --- | --- |
| 7a.1 | Full regression | Run §R1–R7 | All pass |
| 7a.2 | **Cap blocks EXP at 14** | New Game, battle until a mon reaches level 14, keep battling | Mon gains **no** experience past 14 |
| 7a.3 | Cap is exactly 14, not 15 | Check the mon stops at 14 | Level 14 — this is the value Randolocke uses; upstream's default was 15 |
| 7a.4 | Rare Candy respects the cap | Debug → Give item → Rare Candy; use it on a level-14 mon with 0 badges | Refuses / does not exceed 14 |
| 7a.5 | Cap raises with a badge | Debug → Flags, set `FLAG_BADGE01_GET`, then battle | Mon can now progress toward 21 |
| 7a.6 | Second badge → 24 | Set `FLAG_BADGE02_GET` | Cap becomes 24 |
| 7a.7 | Champion → 63 | Set `FLAG_BADGE08_GET`, then `FLAG_IS_CHAMPION` | Cap 63 with 8 badges; after `FLAG_IS_CHAMPION` is set, cap is 100 |
| 7a.8 | Under-cap mons still level normally | A level-5 mon with 0 badges | Gains EXP normally up to 14 |
| 7a.9 | Traded/caught mons obey the cap | Catch a wild mon at/near the cap | Cannot exceed the cap |

### Note on the flag table's semantics

`sLevelCapFlagMap` returns the cap for the **first flag that is still unset**. So the row
`{FLAG_BADGE01_GET, 14}` means "cap 14 while badge 1 has not been earned." Beating the
Elite Four sets `FLAG_IS_CHAMPION`, the loop falls through, and `MAX_LEVEL` (100) applies —
which is exactly Randolocke's "Elite Four defeated: 100."

### Not included: the Cap Candy

`B_RARE_CANDY_CAP` makes the **normal** Rare Candy respect the cap. Randolocke's **Cap
Candy** is a different, custom item that levels a Pokémon *up to* the next meaningful
point (next level cap, next move learned, or next evolution). That still needs a new item
and effect — see Phase 7.

---

## Phase 7b — The four custom key items

**What this build is:** Randolocke's four QoL key items. All are **key items** in the Key
Items pocket, so none of them is ever consumed.

| Item | Effect |
| --- | --- |
| **Repellant** | Toggles a permanent repel on/off |
| **Porta Heal** | Portable Pokémon Center; by default does **not** revive fainted Pokémon |
| **Endless Candy** | Raises a Pokémon's level by 1 |
| **Cap Candy** | Raises a Pokémon to the next level cap, next level-up move, or next level evolution — whichever comes first |

Config lives in `include/config/randolocke.h`:
`RANDOLOCKE_FLAG_INFINITE_REPEL` and `RANDOLOCKE_PORTA_HEAL_REVIVES` (default `FALSE`).

**To get them:** Debug → Give → Give item XYZ… and pick each by name.

### Tests

| # | Test | Steps | Expected |
| --- | --- | --- | --- |
| 7b.1 | Full regression | Run §R1–R7 | All pass |
| 7b.2 | Items exist and are key items | Give all four, open the Bag | All four in the **Key Items** pocket with sensible names, icons and descriptions |
| 7b.3 | Repellant turns on | Use it | "The Repellant is now active!" |
| 7b.4 | Repellant actually repels | Walk 100+ steps in grass with a higher-level lead | No wild encounters from weaker Pokémon |
| 7b.5 | **Repellant never wears off** | Walk several hundred steps | Still active; no "repel wore off" message |
| 7b.6 | Repellant toggles off | Use it again | "The Repellant was switched off"; wild encounters resume |
| 7b.7 | Repellant survives save/reload | Toggle on, save, reset, reload | Still active (it is a flag, so it persists) |
| 7b.8 | Not consumed | Check the bag after each use | All four still present |
| 7b.9 | Porta Heal restores HP/PP | Damage a mon and use PP, then use it | HP and PP restored, status cured |
| 7b.10 | **Porta Heal does NOT revive** | Let a mon faint, then use it | Fainted mon **stays fainted** — this is Randolocke's default. Healthy mons still heal |
| 7b.11 | Endless Candy +1 | Use on a level-5 mon | Becomes level 6, stat screen shown |
| 7b.12 | Endless Candy respects the cap | Use on a mon at the cap (14 with 0 badges) | "It won't have any effect" |
| 7b.13 | **Cap Candy jumps to the cap** | Use on a low-level mon with 0 badges | Jumps to **14** in one use |
| 7b.14 | Cap Candy stops at a move | Use on a mon that learns a move before the cap | Stops at that level, not the cap |
| 7b.15 | Cap Candy stops at an evolution | Use on a mon that evolves by level before the cap | Stops at the evolution level |
| 7b.16 | Cap Candy at the cap | Use on a mon already at the cap | "It won't have any effect" |
| 7b.17 | Stats recalculate | After any candy, open the summary | Stats match the new level |

### What to watch for

- **7b.5** is the point of the Repellant. `UpdateRepelCounter()` returns early while the
  flag is set, so the step counter never decrements.
- **7b.10** is the Randolocke-specific behaviour. If you would rather it revive, set
  `RANDOLOCKE_PORTA_HEAL_REVIVES` to `TRUE` in `include/config/randolocke.h` and rebuild.
- **7b.13–7b.15** exercise the three branches of the Cap Candy's target calculation. It
  takes the *soonest* of cap / next move / next evolution, so a mon that learns a move at
  level 10 with a cap of 14 should stop at 10.

---

## Phase 4 — Ability stability across evolution (Enhancement 1)

**What this build is:** a randomized ability is now decided by the **root of the evolution
family** rather than the current species, so evolving no longer rerolls it.

Config: `RZ_ABILITY_STABLE_ACROSS_EVOLUTION` (`include/config/randomizer.h`), default `TRUE`.

⚠️ **This is not the same as turning ability randomization off.** Abilities are still
randomized — they are just stable across a family now. The on/off switch remains
`RANDOMIZER_FLAG_ABILITIES` (`0x26`).

### Prerequisite

Set flag `0x26` (abilities) — see §F — and use a real New Game.

### Tests

| # | Test | Steps | Expected |
| --- | --- | --- | --- |
| 4.1 | Full regression | Run §R1–R7 | All pass |
| 4.2 | **Ability survives evolution** | Catch a mon that evolves by level. Note its ability. Level it up (Endless/Cap Candy is quickest) until it evolves. Check the ability | **Unchanged** |
| 4.3 | Two-stage evolution | Evolve a three-stage family twice (e.g. a Treecko line) | Ability is the same at all three stages |
| 4.4 | Whole family shares an ability | Catch or debug-give the base, middle and final forms separately, same ability slot | All three show the **same** randomized ability |
| 4.5 | Ability is still randomized | Compare against the species' real ability | It is a **different**, randomized ability — not the vanilla one |
| 4.6 | Different families differ | Compare two unrelated families | Different abilities |
| 4.7 | Ability slots still differ | Give the same species with ability slot 0 vs slot 1 | Different abilities per slot |
| 4.8 | Baby forms count as the root | A family with a baby stage (Pichu → Pikachu → Raichu) | All three share the ability |
| 4.9 | Config off restores old behaviour | Set `RZ_ABILITY_STABLE_ACROSS_EVOLUTION` to `FALSE`, rebuild, evolve a mon | Ability **changes** on evolution again |
| 4.10 | **No battle slowdown** | Fight a full 6v6 trainer battle, watch AI turns | No stutter or lag when the AI picks moves |
| 4.11 | Stable across reload | Note a mon's ability, save, reset, reload | Same ability |

### Why 4.10 is on the list

`GetSpeciesPreEvolution()` is a linear scan over all ~1,679 species, and the AI calls
`GetAbilityBySpecies()` repeatedly while scoring moves. Walking the chain naively would
have meant thousands of iterations per AI decision. The lookup is memoized in a 16-entry
direct-mapped cache (64 bytes of EWRAM), which covers a full double battle plus both
parties. If you ever *do* notice AI slowdown, that cache is the first place to look.

---

## Phase 5 — IV / EV / nature editors (Enhancements 2 and 4)

**What this build is:** three new rows under **Debug → Party → Edit Pokemon**, operating on
an existing party Pokémon:

| Row | Effect |
| --- | --- |
| **Set IVs** | Edit all six IVs (0–31), pre-filled with current values |
| **Set EVs** | Edit all six EVs (0–252), pre-filled with current values |
| **Set Nature** | Set the **true** nature — the one shown on the summary screen |

The existing **Set Hidden Nature** (Mint-style, affects stats only) stays alongside it.

### Tests

| # | Test | Steps | Expected |
| --- | --- | --- | --- |
| 5.1 | Full regression | Run §R1–R7 | All pass |
| 5.2 | Rows appear | Debug → Party → Edit Pokemon | Set IVs, Set EVs, Set Nature present |
| 5.3 | IVs pre-fill | Set IVs on a mon whose IVs you know (check first with Check IVs) | Starting values match the mon's current IVs, not zeros |
| 5.4 | IV edit sticks | Set all six IVs to 31, confirm, open the summary | Summary IV page shows 31 across the board |
| 5.5 | **Stats recalculate** | After 5.4, look at the stat numbers | Stats increased — `CalculateMonStats` ran |
| 5.6 | IVs persist | Save, reset, reload, check again | Still 31 |
| 5.7 | EVs pre-fill and edit | Set EVs, e.g. 252 HP / 252 Speed | Summary EV page matches; stats recalculate |
| 5.8 | Per-stat cursor | Step through all six substeps | Each stat is editable individually, labelled correctly |
| 5.9 | Cancel does nothing | Open Set IVs, press B | Values unchanged |
| 5.10 | **Nature changes on the summary** | Set Nature → Adamant, then open the summary | Summary shows **Adamant** *(this is the difference from Set Hidden Nature, which deliberately does not change the label)* |
| 5.11 | **Gender never flips** | Pick a species with a split gender ratio (e.g. a starter, 87.5% male). Note the gender, then set every nature in turn, checking gender each time | Gender is **identical** every time |
| 5.12 | Shininess preserved | Debug-give a shiny mon, then change its nature | Still shiny |
| 5.13 | Non-shiny stays non-shiny | Change a normal mon's nature repeatedly | Never becomes shiny |
| 5.14 | Nature affects stats | Set a nature with a clear spread and check the stat arrows/colours | Stats and colouring reflect the new nature |
| 5.15 | Both nature editors coexist | Use Set Nature, then Set Hidden Nature, to different values | Summary shows the **true** nature; stats follow the **hidden** one |

### Why 5.11 and 5.12 matter

Nature lives inside the personality value, which also determines gender, shininess, Unown
letter, Wurmple's evolution and Spinda spots. The naive fix — upstream's
`ModifyPersonalityForNature()` — nudges the personality by up to ±12, which can flip gender
near a ratio boundary.

This editor instead steps the personality by **multiples of 256**:

- gender reads only `personality & 0xFF`, and `+256` never touches the low byte → gender is
  preserved *exactly*
- nature is `personality % 25`, and `256 % 25 == 6`, which is coprime with 25 → stepping by
  256 can still reach all 25 natures

Shininess hashes the whole personality, so it is read before the change and re-asserted
afterwards (the `MON_DATA_IS_SHINY` setter recomputes `shinyModifier` from the *current*
personality, so the order matters).

Residual, accepted changes: Unown letter, Wurmple's evolution branch and Spinda spots.

---

## Phase 6a — Modern Emerald QoL configs (landed early)

**What this build is:** three config flips pulled forward from Phase 6 because they are
independent of the randomizer. No custom code.

| Config | Value | File |
| --- | --- | --- |
| `I_REUSABLE_TMS` | `TRUE` | `include/config/item.h` |
| `B_SHOW_TYPES` | `SHOW_TYPES_ALWAYS` | `include/config/battle.h` |
| `P_SUMMARY_SCREEN_IV_EV_INFO` | `TRUE` | `include/config/summary_screen.h` |
| `P_SUMMARY_SCREEN_IV_EV_VALUES` | `TRUE` | `include/config/summary_screen.h` |

### Tests

| # | Test | Steps | Expected |
| --- | --- | --- | --- |
| 6a.1 | Full regression | Run §R1–R7 | All pass |
| 6a.2 | TM is not consumed | Debug → Give item → a TM. Check bag quantity, teach it to a mon, check the bag again | TM is **still in the bag** at the same quantity |
| 6a.3 | TM still teaches | After 6a.2, open the mon's summary | It knows the move |
| 6a.4 | TM reusable on a second mon | Teach the same TM to a different party mon | Works; TM still in the bag |
| 6a.5 | Type icons appear | Enter any battle, choose FIGHT, then select a target | Type indicator(s) show next to the opposing Pokémon's HP bar |
| 6a.6 | Type icons are correct | Target a Pokémon whose types you know (e.g. a Zigzagoon → Normal) | Icons match the species' actual types |
| 6a.7 | Dual-type display | Target a dual-type Pokémon | **Both** types shown |
| 6a.8 | IV/EV cycling in summary | Open a mon's summary → Skills page → press the cycle input | Page cycles Stats → IVs → EVs |
| 6a.9 | IVs show raw numbers | On the IV page | Numbers `0`–`31`, not letter grades (F/D/C/B/A/S) |
| 6a.10 | EVs read plausibly | On the EV page for a freshly caught mon | All `0`; they rise after battles |
| 6a.11 | IV page matches debug | Compare the summary IV page against Debug → Party → Check IVs | Identical values |
| 6a.12 | ROM size sanity | Check the memory table | ROM up ~1.4 KB vs Phase 2; EWRAM/IWRAM unchanged |

### Notes

- **`B_SHOW_TYPES` is a one-word change.** If always-on feels like it removes discovery,
  switch to `SHOW_TYPES_CAUGHT` or `SHOW_TYPES_SEEN` in `include/config/battle.h`.
- **`P_SUMMARY_SCREEN_IV_EV_TILESET` was left `FALSE`.** Setting it `TRUE` re-labels the
  "STATS" header but requires a `make clean`. Cosmetic only.
- 6a.8's cycle input is defined by the summary screen implementation — if it is not obvious
  in-game, try left/right or Select on the Skills page.

---

## Phase 6 / 7 — Parity configs and systems

**What this build is:** the config-level Randolocke parity pass, plus three code features.

### Tests

| # | Test | Steps | Expected |
| --- | --- | --- | --- |
| 6.1 | Full regression | Run §R1–R7 | All pass |
| 6.2 | **Every mon learns every move** | Debug-give any Pokémon, take it to a TM/move relearner | It can learn essentially any TM/tutor move (89 teachables per species) |
| 6.3 | Move relearners work | Use the summary-screen relearner | Egg, TM and tutor moves offered |
| 6.4 | HMs are forgettable | Teach an HM move, then try to forget it | Allowed |
| 6.5 | **HMs need no user** | Face surfable water with **no** party mon knowing Surf, with the Balance Badge | You can still Surf |
| 6.6 | Badge gating still applies | Try the same **without** the badge | Blocked |
| 6.7 | Fast text is near-instant | Options → Text Speed → Fast, read dialogue | Text prints almost instantly |
| 6.8 | **No Mega/Tera/Dynamax** | Check the battle menu and species list | No gimmick forms available |
| 6.9 | Bigger bag | Collect more than 30 distinct items | Bag holds up to 50 (items), 40 (key), 24 (balls) |
| 6.10 | Catch rates raised | Throw Poké Balls at wild mons | Noticeably easier than vanilla (base rate ×1.5) |
| 7.1 | **Always run** | Walk around without touching B | Player runs by default |
| 7.2 | Hold B to walk | Hold B while moving | Player walks — useful for ledges |
| 7.3 | Running still badge-gated | Before the Running Shoes | Cannot run |

### Note on 6.2

Teachable learnsets are generated at build time. If you change TMs or tutors later, run
`make clean-teachables` then `make` to regenerate. Every species is forced to
`ALL_TEACHABLES` from `tools/learnset_helpers/make_teachables.py` — a single switch rather
than an edit to all ~1,679 species entries.

### Note on 6.8

Disabling the gimmick forms freed **1.8 MB of ROM** (79.67% → 74.14%), which is useful
headroom for the map and event work still to come.

---

## Phase 7c — Trainer scaling, boss tags, 21-move learnsets

### Randomizer flags recap (see §F)

A new one: **`0x28` = learnset randomization**. Set it alongside `0x20`–`0x26`.

### Tests — trainer level scaling

| # | Test | Steps | Expected |
| --- | --- | --- | --- |
| 7c.1 | Full regression | Run §R1–R7 | All pass |
| 7c.2 | Early trainers near the cap | Battle Route 102/103 trainers | Levels in the low teens, at or under the 0-badge cap of 14 |
| 7c.3 | **Roxanne sits at the cap** | Battle Roxanne | Her ace is level 14 — the 0-badge cap |
| 7c.4 | Mid-game inflation | Battle trainers around Petalburg/Fortree | Noticeably higher than vanilla (vanilla 33 → 43). Expect this to be the hardest stretch |
| 7c.5 | Champion at 63 | Battle Wallace | Level 63 |
| 7c.6 | Post-game unchanged | Battle Frontier or rematch trainers | Still 63–78, not inflated |

### Tests — boss trainers

| # | Test | Steps | Expected |
| --- | --- | --- | --- |
| 7c.7 | **Gym leaders ARE randomized** | With flag `0x22` set, battle Roxanne | Her team is randomized, not Geodude/Nosepass — bosses are randomized by default |
| 7c.8 | Boss exemption works | Set `RZ_RANDOMIZE_BOSS_TRAINERS` to `FALSE`, rebuild, battle Roxanne | Her designed team and real abilities return |
| 7c.9 | Non-bosses unaffected by that toggle | With the toggle `FALSE`, battle a route trainer | Still randomized |

### Tests — 21-move learnsets

| # | Test | Steps | Expected |
| --- | --- | --- | --- |
| 7c.10 | **21 moves at fixed levels** | Set flag `0x28`, then check a mon in the move relearner or level it up | Moves at 1, 4, 7, 10, 13, 16, 20, 24, … |
| 7c.11 | Every species uses the same levels | Compare two unrelated species | Identical level ladder, different moves |
| 7c.12 | STAB present | Check a Fire-type's moves | Several Fire moves among them |
| 7c.13 | Status moves present | Same mon | Roughly a third are status moves |
| 7c.14 | **Stronger moves later** | Compare the Base Power of early vs late damaging moves | Later ones hit harder |
| 7c.15 | No OHKO moves | Scan several species' learnsets | No Fissure / Sheer Cold / Horn Drill |
| 7c.16 | Flag off = vanilla | Clear flag `0x28`, check a starter | Vanilla learnset returns |
| 7c.17 | Stable across reload | Note a species' learnset, save, reset, reload | Identical |
| 7c.18 | **No AI slowdown** | Full 6v6 battle | No stutter — the learnset is cached per species, but the AI queries it often |
| 7c.19 | Relearner agrees | Open the move relearner | Offers the same randomized moves the mon levels into |

### What to watch

- **7c.4** is the known consequence of your cap curve, not a bug. The jump is concentrated
  between badges 4 and 6 because your caps rise much faster there than vanilla's did.
- **7c.7 vs 7c.8** verify the `Boss:` tag is now only a *label*: it identifies bosses, and
  `RZ_RANDOMIZE_BOSS_TRAINERS` decides whether the label exempts them.
- **7c.18** matters because the learnset is rebuilt whenever the queried species changes.
  In a battle with many distinct species the cache turns over; if you feel lag, that single
  -species cache is the thing to enlarge.

---

## Phase 8 — Map and data edits

| # | Test | Steps | Expected |
| --- | --- | --- | --- |
| 8.1 | Full regression | Run §R1–R7 | All pass |
| 8.2 | **Scorched Slab has encounters** | Walk in Scorched Slab | Wild battles occur — the map had no encounter table before |
| 8.3 | Scorched Slab levels | Note the levels | 26–31 (randomized species if flag `0x20` is set) |
| 8.4 | **Zweilous evolves before the E4** | Get a Zweilous to level 63 | Evolves into Hydreigon. It needed 64 before, which is above the 8-badge cap of 63 |
| 8.5 | Dark Void on any species | Teach Dark Void to something that is not Darkrai, use it | It works rather than failing |

### Not done, do not test for

Water in Littleroot, grass in Oldale, and the relocated Old Rod sailor need Porymap. The
Slateport legendary-map seller and the Kyogre/Groudon Weather Institute events are also not
implemented.

---

## Phase 10 — Tier-weighted randomization

**Prerequisite:** flags `0x21` (items), `0x26` (abilities), `0x28` (learnsets), and a real
New Game. See §F.

Tier data is generated, so re-run the tools after editing any worksheet:

```
python3 tools/randolocke/validate_tiers.py [--moves]
python3 tools/randolocke/tier_report.py [--moves]
python3 tools/randolocke/gen_ability_tiers.py
python3 tools/randolocke/gen_move_tiers.py
python3 tools/randolocke/gen_item_tiers.py
python3 tools/randolocke/gen_tm_tiers.py
python3 tools/randolocke/gen_berry_tiers.py
```

| # | Test | Steps | Expected |
| --- | --- | --- | --- |
| 10.1 | Full regression | Run §R1–R7 | All pass |
| 10.2 | **Abilities skew good** | Debug-give 10 Pokémon, note abilities | Noticeably more S/A/B abilities than D/F. An S ability is 2.31x uniform, an F 0.34x |
| 10.3 | **Negative abilities never appear** | Check many Pokémon | No Truant, Slow Start, Defeatist, Stall, Wimp Out, Ball Fetch, Honey Gather — that tier is weight 0 |
| 10.4 | Wonder Guard never appears | Check many Pokémon | Never — excluded by policy |
| 10.5 | **Moves skew good** | Check several randomized learnsets | More Staples/Filler than Bad/Homeless. A Meta Defining move is 2.26x uniform, a Homeless one 0.15x |
| 10.6 | No Z/Max/G-Max moves | Scan learnsets | None — 87 are excluded structurally |
| 10.7 | No Struggle in a learnset | Scan learnsets | Never |
| 10.8 | **Self-KO moves are rare** | Look for Explosion, Self-Destruct, Memento, Final Gambit | Very rare (0.15x) — the community list had Explosion in *Niche*, the nuzlocke pushdown moved it |
| 10.9 | No OHKO moves in learnsets | Look for Fissure, Guillotine, Horn Drill, Sheer Cold | Very rare — same pushdown |
| 10.10 | **STAB matches the category** | Debug-give a strong physical attacker (e.g. a Machamp-like) and check its STAB moves | Physical. A special attacker should get special STAB |
| 10.11 | Mixed attackers get both | Give a species with near-equal Atk and Sp. Atk | STAB moves from both categories |
| 10.12 | STAB power grows with level | Compare the level-1 STAB move with the level-70 one | The later one hits harder — picks are sorted ascending by Base Power |
| 10.13 | Coverage moves are off-type | Check the level 7/16/28/… slots | Not the mon's own types |
| 10.14 | **Items skew to held items** | Set flag `0x21`, pick up several field items | Mostly held items or TMs |
| 10.15 | Consumables are rare | Same | Potions, vitamins and X items almost never — tier 5 is 0.10x |
| 10.16 | **Ordinary pickups can be TMs** | Pick up several non-TM item balls | Some are TMs — a 30% band |
| 10.17 | TM drops skew good | Note which TMs appear | Staples and Filler mostly; Niche rarely (0.27x) |
| 10.18 | **No bad TMs at all** | Watch for Water Pulse, Hail, Hidden Power, Psychic, Double Team, Shock Wave, Sludge Bomb, Attract, Skill Swap, Snatch | None ever — those 10 are removed from the TM pool |
| 10.19 | Balls and evolution items are rare finds | Pick up many items | Rare (0.30x) because the mart sells them |
| 10.20 | **No berries from field items** | Pick up many items | Never a berry — they come from trees instead (Phase 12) |
| 10.21 | No AI slowdown | Full 6v6 battle | No stutter |

---

## Phase 11 — Cheap mart

| # | Test | Steps | Expected |
| --- | --- | --- | --- |
| 11.1 | Full regression | Run §R1–R7 | All pass |
| 11.2 | **Balls on sale cheaply** | Visit any Poké Mart | Ultra Ball, Fast Ball and Timer Ball at ₽200 each |
| 11.3 | **Every evolution item on sale** | Same shop list | All 54 — stones, Linking Cord, Scrolls of Darkness/Waters, Leader's Crest, Metal Alloy, the Sweets, Galarica items… at ₽200 |
| 11.4 | Available from the start | Check Oldale Town Mart before any badge | Already stocked |
| 11.5 | All 12 general marts | Spot-check three different towns | Same extra stock everywhere |
| 11.6 | An item evolution works | Buy a Fire Stone, use it on an eligible Pokémon | Evolves |
| 11.7 | **A trade evolution works without trading** | Buy a Linking Cord, use it on e.g. a Machoke | Evolves — this is what replaces Randolocke's trade NPCs |
| 11.8 | Specialty shops untouched | Visit the Lilycove Dept. Store and the Lavaridge Herb Shop | Normal stock, no injected list |

### Known gaps

- **Gholdengo** needs 999 Gimmighoul Coins in the bag. At ₽200 each that is ~₽199,800 —
  possible but a grind. Not yet solved.
- **Shelmet and Karrablast** evolve by trading with *each other* specifically, which the
  Linking Cord does not satisfy. Still unevolvable.

---

## Phase 12 — Berry tree randomization

**Prerequisite:** flag **`0x29`**.

| # | Test | Steps | Expected |
| --- | --- | --- | --- |
| 12.1 | Full regression | Run §R1–R7 | All pass |
| 12.2 | **Berries randomize** | Find a berry tree and pick it | A different berry than vanilla planted there |
| 12.3 | The name matches the yield | Read the tree's message, then check the bag | Same berry in both — the hook is the single read point, so they cannot disagree |
| 12.4 | **A tree is stable** | Pick a tree, leave, come back, plant and grow again | Same berry each time for the same planted berry |
| 12.5 | Replanting differs | Plant a *different* berry in the same plot | Different result — the seed uses both tree and planted berry |
| 12.6 | Berries skew useful | Check several trees | Lum, Sitrus, Salac, Liechi, Petaya more often; no-hold-effect berries rarely (0.25x) |
| 12.7 | Flag off = vanilla | Clear `0x29` | Trees give what was planted |

---

## F5 — Two registered key items

⚠️ **This changed the save layout.** Start a **new game**; an older save is invalid.

| # | Test | Steps | Expected |
| --- | --- | --- | --- |
| F5.1 | Full regression | Run §R1–R7 | All pass |
| F5.2 | Register one item | Bag → a key item → Register. Tap SELECT in the overworld | It is used |
| F5.3 | **Register a second** | Register a different key item, then tap SELECT | The **new** item is used; the old one moved to slot two |
| F5.4 | **Hold SELECT uses slot two** | Hold SELECT for about a third of a second | The older item is used |
| F5.5 | A hold does not also tap | Hold SELECT once | **Only** the second item fires, not both — the tap fires on release |
| F5.6 | Unregister | Register an already-registered item again | It is cleared from whichever slot it was in |
| F5.7 | Both survive a save | Save, reset, reload, try tap and hold | Both still registered |
| F5.8 | Empty slot two | Register only one item, hold SELECT | Nothing happens, no crash |

---

## F6 — EVs in the move relearner

| # | Test | Steps | Expected |
| --- | --- | --- | --- |
| F6.1 | **EVs shown** | Open the move relearner on a party Pokémon, battle-moves page | The top line reads `Atk EV nnn  SpA EV nnn` instead of "BATTLE MOVES" |
| F6.2 | Values are right | Compare against Debug → Party → Check EVs | Identical |
| F6.3 | ⚠️ **Nothing is clipped** | Look at the whole panel | The EV line fits, and the type/power/accuracy rows and the description below are unaffected. This replaced the heading's row because the panel had no spare line — **the one test here that needs a careful look** |
| F6.4 | Updates per Pokémon | Open the relearner on a different party member | Shows that Pokémon's EVs |
| F6.5 | Contest page unaffected | Switch to contest moves | Normal |

---

## Regi caves via Flash

| # | Test | Steps | Expected |
| --- | --- | --- | --- |
| RG.1 | **Sealed Chamber outer** | Reach the Sealed Chamber outer room, use Flash anywhere in it | The door to the inner room opens |
| RG.2 | Works away from the Braille | Use Flash from across the room, not on the Braille tile | Still works |
| RG.3 | **Sealed Chamber inner opens all three** | In the inner room, use Flash | The shake plays and `FLAG_REGI_DOORS_OPENED` is set |
| RG.4 | Cave entrances appear | Leave, then visit Route 111, 105 and 120 | All three cave entrances are open. *You must leave and re-enter the route — the check runs on map transition, same as vanilla* |
| RG.5 | Desert Ruins | Inside Desert Ruins, use Flash | Regirock's wall opens |
| RG.6 | **Island Cave** | Inside Island Cave, use Flash | Regice's wall opens **at the right place** — x 7–9, y 19–20, with only the middle bottom tile walkable. *I had these coordinates wrong at first; worth a careful look* |
| RG.7 | Ancient Tomb still works | Use Flash in Ancient Tomb | Registeel's wall opens (this was vanilla behaviour) |
| RG.8 | Braille puzzles still work | Solve one the old way instead | Also works — Flash is an additional route, not a replacement |
| RG.9 | No double-open | Use Flash again in an already-opened room | Nothing happens, no crash |
| RG.10 | Flash still lights caves | Use Flash in an ordinary dark cave | Lights the cave as normal |

---

## v1.1 options and Strict mode

### Strict mode

`RZ_TIER_MODE_{MOVES,ABILITIES,ITEMS,TMS,BERRIES}` each take `RZ_TIER_OFF`,
`RZ_TIER_WEIGHTED` (default) or `RZ_TIER_STRICT`. Strict draws only from the top
`RZ_STRICT_TIERS` (2) bands.

| # | Test | Steps | Expected |
| --- | --- | --- | --- |
| S.1 | Weighted is the default | Build unchanged | Behaves as Phase 10 describes |
| S.2 | **Strict narrows the pool** | Set `RZ_TIER_MODE_ABILITIES` to `RZ_TIER_STRICT`, rebuild, give 10 Pokémon | Only S and A abilities — 43 of them, so expect repeats |
| S.3 | Strict on moves | Same for `RZ_TIER_MODE_MOVES` | Only Meta Defining and Staples — 50 moves, heavy repetition |
| S.4 | Off restores uniform | Set a pool to `RZ_TIER_OFF` | Every entry equally likely again |
| S.5 | Modes are independent | Strict abilities, Weighted moves | Each behaves per its own setting |

### v1.1 options

| # | Test | Steps | Expected |
| --- | --- | --- | --- |
| V.1 | **Forced nickname** | Catch a Pokémon | Goes **straight to the naming screen** — no "Do you want to nickname?" prompt |
| V.2 | Naming still cancellable | Press B on the naming screen | Keeps the species name, no crash |
| V.3 | **Bag disabled in trainer battles** | Debug → Vars, set `VAR_UNUSED_0x40F7` to 1. Enter a trainer battle | The Bag is unusable |
| V.4 | Wild battles unaffected at 1 | Same var at 1, enter a wild battle | Bag still usable |
| V.5 | Value 2 disables both | Set the var to 2 | Bag unusable in wild battles too |
| V.6 | Default is off | Leave the var at 0 | Bag works everywhere |
| V.7 | **Gholdengo is reachable** | Buy 999 Gimmighoul Coins (₽1 each = ₽999), level a Gimmighoul | Evolves into Gholdengo |
| V.8 | **Shelmet / Karrablast evolve** | Buy a Linking Cord, use it on each | Karrablast → Escavalier, Shelmet → Accelgor. The trade still works too |
| V.9 | Kyogre / Groudon caves | After the Rayquaza scene at Sky Pillar, check the abnormal weather routes | Available — `FLAG_SYS_WEATHER_CTRL` is set there in vanilla, so this needed no change |

---

## Phase 13 — TM to move randomization and the bag move panel

`RANDOMIZER_FLAG_TM_MOVES` (0x2A) re-points every TM at a different move, tier-weighted
through the TM bands. The bag panel is the companion feature: with the TM's move no longer
predictable from its number, the only way to shop your own bag is to read the move.

### TM to move randomization

| # | Test | Steps | Expected |
| --- | --- | --- | --- |
| T13.1 | **Flag off is vanilla** | New game, leave flag 0x2A clear. Check TM01 in the bag | Focus Punch, as in vanilla |
| T13.2 | **Flag on re-points TMs** | Debug → Flags, set 0x2A. Check TM01 | Some other move |
| T13.3 | Same seed, same TMs | Note TM01–TM10, soft reset, reload | Identical list |
| T13.4 | Different seed, different TMs | New game with a different Trainer ID | A different mapping |
| T13.5 | The mapping is a bijection | Read TM01–TM50 | No move appears twice — every TM is a distinct move |
| T13.6 | **Teaching matches the panel** | Teach a TM to a Pokémon | The move learnt is the one the panel showed, not the vanilla move |
| T13.7 | Move tutors are unaffected | Talk to any move tutor | Vanilla moves |
| T13.8 | HMs are untouched | Check HM01–HM08 | Cut, Fly, Surf, Strength, Flash, Rock Smash, Waterfall, Dive |
| T13.9 | Marts sell the re-pointed move | Buy a TM from any mart | The bag shows the randomized move |
| T13.10 | **Bands are respected** | With 0x2A set, survey all 50 TMs | **No Bad and no Pokemon Homeless moves at all** — those two bands are excluded from `sTmMoveTiers`, not merely made rare |
| T13.10b | The spread matches the TM weights | Same survey, bucket by tier | Roughly 3 Meta Defining, 26 Staples, 16 Filler, 5 Niche. `tools/randolocke/tm_band_sim.py` prints the prediction |
| T13.10c | **Found TMs are uniform once moves are randomized** | 0x2A set, collect 20 field TMs | Any of TM01–TM50, evenly. The tier spread lives in the assignment now, not in which TM number drops |
| T13.10d | Found TMs are tier-weighted when moves are *not* randomized | 0x2A clear, 0x21 set, collect 20 field TMs | Only TMs whose vanilla move is Meta Defining / Staples / Filler / Niche |
| T13.11 | Reverse lookup is consistent | Use a move-relearner or a battle that names the TM's move | Names agree with the bag panel |

### The bag move panel

`RANDOLOCKE_TM_HOVER_INFO` (TRUE) in `include/config/randolocke.h`. The panel is an 8x8
white box on BG1 at tile (5, 4), drawn over the bag sprite.

| # | Test | Steps | Expected |
| --- | --- | --- | --- |
| T13.12 | **Panel follows the cursor** | Open the bag, go to the TM pocket, scroll | The panel updates on **every** cursor move — no need to press A |
| T13.13 | Type icon | Hover a TM | Its move's type icon in the top-left of the panel |
| T13.14 | **Damage category icon** | Hover a physical TM, then a special one | PHYSICAL / SPECIAL icon to the right of the type icon |
| T13.15 | Status moves show no category | Hover a status TM (e.g. Toxic) | Type icon only; the category column stays empty |
| T13.16 | Power, accuracy, PP | Hover any damaging TM | Three right-aligned values matching the move |
| T13.17 | Dashes where a value is absent | Hover a status move, and a never-miss move | `---` for power, `---` for accuracy |
| T13.18 | **Description stays visible** | Hover a TM | The description box at the bottom still shows the item description — the panel does not replace it |
| T13.19 | Panel draws over the bag sprite | Look at the bag graphic behind the panel | The panel is on top and fully opaque, with rounded corners |
| T13.20 | **Hidden outside the TM pocket** | Scroll to Items, Poké Balls, Berries, Key Items | No panel |
| T13.21 | Hidden on Cancel | Scroll to the CANCEL row of the TM pocket | No panel |
| T13.22 | Survives a pocket round trip | TM pocket → Items → back to TM pocket | Panel returns, correct for the hovered TM |
| T13.23 | Panel while the context menu is open | Hover a TM, press A | Panel stays; "USE / GIVE / …" appears; description reads "TM01 is selected" |
| T13.24 | Returning from the context menu | Press B | Panel still correct, description restored |
| T13.25 | Panel during item swap | Press SELECT to start a swap, move the item | No graphical corruption |
| T13.26 | Empty TM pocket | Toss every TM | No panel, no crash |
| T13.27 | Battle bag | Open the bag mid-battle, TM pocket is absent | No regression |
| T13.28 | Wally's tutorial bag | Play the Wally catching tutorial | No regression |
| T13.29 | **Sell / deposit screens** | Sell a TM at a mart; deposit one in the PC | Panel behaves, money window does not overlap it |
| T13.30 | Config off | Set `RANDOLOCKE_TM_HOVER_INFO` to `FALSE`, rebuild | Panel appears only after pressing A, as in stock 1.17 |

---

## Phase 9 — Ship

Build and patching are done; see `docs/RELEASING.md`. What remains is yours to run.

### Credits

| # | Test | Steps | Expected |
| --- | --- | --- | --- |
| T9.1 | **Randolocke credits appear** | Beat the Elite Four and watch the credits | Five new pages right after the title card: RANDOLOCKE EXPANDED, Original Randolocke / Istorian, Randomizer / tertu-m / Zetraphes, Built On / pokeemerald-expansion / RHH and pret, Inspiration / PChal / Pointcrow |
| T9.2 | The original roll still plays | Keep watching | The Game Freak staff roll follows, unchanged |
| T9.3 | Scene pacing is not broken | Watch the whole sequence | The bike scenes and Pokémon interludes still land — `PAGE_INTERVAL` is `PAGE_COUNT / 9`, and five extra pages shift it slightly |
| T9.4 | No text overflow | Read each new page | Nothing clipped at the screen edges |

### Patch and distribution

| # | Test | Steps | Expected |
| --- | --- | --- | --- |
| T9.5 | **The patch applies** | Apply `randolocke-expanded-0.9.0.bps` to a fresh vanilla Emerald at <https://www.marcrobledo.com/RomPatcher.js/> | Succeeds |
| T9.6 | Result checksum | CRC32 the output | `c165e622` for the release patch, `359e5b0f` for the test patch |
| T9.7 | **The patched ROM boots** | Load it in mGBA | Title screen, new game works |
| T9.8 | Wrong base is refused | Try to apply it to a different ROM | Rejected — the patch stores the source checksum |
| T9.9 | Flips agrees | Apply the same patch with Flips instead | Identical output |
| T9.10 | **The release build has no debug menu** | On the release patch, hold R and press START | Nothing opens |
| T9.11 | **…and is still randomized** | New game on the release patch, walk into grass | Randomized encounters. This is the whole point of Phase 14 |
| T9.12 | The test build does have it | Same on the test patch | The debug menu opens |

### The playthrough

| # | Test | Steps | Expected |
| --- | --- | --- | --- |
| T9.13 | **Reach Gym 3** | Play the release build to Dewford or beyond | No crashes, no softlocks, no missing NPCs |
| T9.14 | **Randomization is stable across a reload** | Note a wild table, a trainer's party and an ability. Save, soft reset, reload | All identical |
| T9.15 | Stable across a power cycle | Same, but File → Reset in mGBA rather than a save state | All identical |
| T9.16 | Save states agree | Save state before an encounter, reload it several times | The same species each time |
| T9.17 | §R regressions | Run the §R rows on the release build | All pass |
| T9.18 | Level caps hold all the way | Check the cap at each badge | 14 / 21 / 24 / 29 / 36 / 43 / 47 / 50 / 63, then 100 |
| T9.19 | No save corruption | Play for an hour, save often, reload | No "save file is corrupted" |

---

## Phase 17 — Terrain

Layout edits made by writing `data/layouts/*/map.bin` directly and checking the result
with `tools/randolocke/render_map.py`, which renders a layout to a PNG. No Porymap.

```bash
python3 tools/randolocke/render_map.py LAYOUT_LITTLEROOT_TOWN /tmp/littleroot.png
```

### Littleroot pond

Water at x 10-15, y 14-18: stone rim at y14, water y15-17, near shore y18.

| # | Test | Steps | Expected |
| --- | --- | --- | --- |
| T17.1 | **The pond is there** | New game, leave your house | A pond east of Birch's lab, stone rim on three sides |
| T17.2 | It is not walkable | Try to walk onto the water | Blocked |
| T17.3 | The shore is walkable | Walk along y=18 below the pond | Passable — this is the fishing spot |
| T17.4 | **The town is still connected** | Walk from the lab door to the east side of town | Reachable. The y=17 route east is now water; go along y=18 or above the pond |
| T17.5 | The boy moved | Look for the boy who stood at (14, 17) | Now at (11, 18), on the shore, facing the water |
| T17.6 | **Fishing works** | Get the Old Rod (Phase 17 below), face the pond, use it | A bite |
| T17.7 | Fishing encounters are randomized | 0x20 set | Randomized species, not Magikarp |
| T17.8 | **Surfing works** | With Surf, step onto the pond | Surfable |
| T17.9 | Surf encounters | Surf around | Water encounters at rate 4 |
| T17.10 | No graphical seams | Walk a full lap around the pond | No torn tiles at the rim or where it meets the lab |
| T17.11 | Level cap still applies | Fish up something over the cap | It gains no experience |

### Oldale tall grass

Two patches: x 2-4 / y 12-16 west of the Pokémon Center, and x 4-7 / y 2-3 north-west.

| # | Test | Steps | Expected |
| --- | --- | --- | --- |
| T17.12 | **Grass is there** | Walk into Oldale Town | Two tall-grass patches |
| T17.13 | Encounters trigger | Walk in the grass | Wild battles at rate 20 |
| T17.14 | Levels are right | Several encounters, flag 0x20 clear | Poochyena / Zigzagoon / Wurmple at 2-4 |
| T17.15 | Randomized | Flag 0x20 set | Randomized species |
| T17.16 | Nothing is blocked | Walk to the Mart, Pokémon Center, both houses and both exits | All reachable |
| T17.17 | The financier is reachable | Talk to the GENTLEMAN at (6, 12) | Fine — he stands on the path, not in the grass |

### The Old Rod fisherman

| # | Test | Steps | Expected |
| --- | --- | --- | --- |
| T17.18 | **He is on Route 103** | Go north from Oldale, east along Route 103 to (22, 11) | A FISHERMAN on the shore, facing the water |
| T17.19 | He gives the rod | Talk to him, say yes | OLD ROD received, `FLAG_RECEIVED_OLD_ROD` set |
| T17.20 | Saying no | Talk to him, say no | Declines cleanly, can ask again |
| T17.21 | Only once | Talk again after taking it | The "how's the fishing" line |
| T17.22 | **Gone from Dewford** | New game, reach Dewford Town | No fisherman at (12, 14) |
| T17.23 | Briney's boat still works | Sail with Mr. Briney from Dewford to Petalburg | The scene plays — the hidden object kept its local ID, so the boat is still object 4 |
| T17.24 | Existing saves keep him | Load a save from before this change, go to Dewford | He is still there, and gives nothing new if you already have the rod |
| T17.25 | Rod is usable immediately | Take the rod, fish in the Littleroot pond | Works before the first badge |

---

## Phase 16 — Legendaries

`RANDOLOCKE_UNIQUE_LEGENDARIES` (TRUE). Gated by the fixed-encounter flag `0x23`.
See `docs/SETTINGS.md` §1.

| # | Test | Steps | Expected |
| --- | --- | --- | --- |
| T16.1 | **A legendary site gives a legendary** | 0x23 set, species mode `MON_RANDOM` (0). Beat Rayquaza's encounter at Sky Pillar | Some legendary — never an ordinary Pokémon, despite the mode |
| T16.2 | **No repeats** | Work through Sky Pillar, Desert Ruins, Island Cave, Ancient Tomb | Four different legendaries |
| T16.3 | Repeats stay impossible across the whole set | Reach all twelve sites | Twelve distinct species |
| T16.4 | Stable across a reload | Note what stands at Desert Ruins, soft reset, look again | The same one |
| T16.5 | Stable across a save/load | Same, but save and reload the save | The same one |
| T16.6 | Seed drives the mapping | New game, different Trainer ID | A different assignment |
| T16.7 | **`seteventmon` sites are randomized** | Reach Faraway Island (Mew), Birth Island (Deoxys), Navel Rock (Ho-Oh, Lugia) | All four randomized. **These were not randomized at all before this phase** |
| T16.8 | Southern Island | Use the Eon Ticket, reach Southern Island | Latios/Latias replaced by a legendary from the pool |
| T16.9 | **The roamer agrees with the island** | Trigger the TV Lati event, then meet the roamer | The roaming Pokémon is the same species the Southern Island slot gives for that Lati |
| T16.10 | Flag off restores vanilla | Clear 0x23 | Rayquaza is Rayquaza, Mew is Mew |
| T16.11 | Config off | Set `RANDOLOCKE_UNIQUE_LEGENDARIES` to `FALSE`, rebuild | Sites roll independently through the species mode — duplicates and non-legendaries both possible |
| T16.12 | **Legendaries in the wild** | 0x20 set, mode `MON_RANDOM`, walk in grass for a while | Legendaries *can* appear — the wild pool is unrestricted in this mode |
| T16.13 | Legend-aware mode changes that | Set `0x404E` to 1 (`MON_RANDOM_LEGEND_AWARE`), walk in grass | No legendaries in the grass; legendary sites still legendary |
| T16.14 | Starters are already unique | New game, check the three starters offered | Three different species |
| T16.16 | **Starter matches its preview** | New game, look at the starter preview picture, then take that Pokémon | The same species. By-slot lookup used to give the starter screen a different answer from every other caller |
| T16.17 | An off-list gift is untouched | Any scripted `givemon` of a species not in `gStarterAndGiftMonTable` | Given as written, no garbage species — this path read one past the end of the table before |
| T16.18 | The Wynaut egg | Get the Lavaridge egg with 0x25 set | A randomized species, and no garbage |
| T16.15 | No species-table thrashing | Trigger a legendary encounter, then a wild one, then another legendary | No stutter — the table is rebuilt at most twice per boot |

---

## Phase 15 — v1.1 NPCs

### Oldale financier

One-time ₽999,999 gift. `RANDOLOCKE_FLAG_OLDALE_MONEY_GIVEN` (0x2B) records that he paid.

| # | Test | Steps | Expected |
| --- | --- | --- | --- |
| T15.1 | **He exists** | Walk into Oldale Town | A GENTLEMAN standing at (6, 12), facing down |
| T15.2 | He pays | Talk to him | ₽999,999, item fanfare, then his second line |
| T15.3 | **Once only** | Talk to him again | The "that was everything I had" line, no more money |
| T15.4 | Flag is set | Debug → Flags, read 0x2B | Set after the first conversation, clear before it |
| T15.5 | Survives a reload | Take the money, save, soft reset, talk again | Still refuses |
| T15.6 | He does not block anything | Walk around him | No collision problem, no NPC overlap |

### Slateport map seller

Sells the four event tickets at ₽1 each, from the 8th badge. Located in Slateport
**Harbor** — the tickets are used from **Lilycove** Harbor.

| # | Test | Steps | Expected |
| --- | --- | --- | --- |
| T15.7 | **Locked before the 8th badge** | Enter Slateport Harbor early, talk to the man at (13, 12) | Turns you away — no menu |
| T15.8 | **Opens at the 8th badge** | Debug → Flags, set `FLAG_BADGE08_GET`. Talk to him | A five-option menu: SOUTHERN ISLAND, FARAWAY ISLAND, BIRTH ISLAND, NAVEL ROCK, EXIT |
| T15.9 | Buying works | Pick SOUTHERN ISLAND with money in hand | ₽1 deducted, EON TICKET in the Key Items pocket |
| T15.10 | **The ship flag is set too** | After T15.9, debug → Flags, read `FLAG_ENABLE_SHIP_SOUTHERN_ISLAND` | Set. Without it the S.S. Tidal ignores the ticket |
| T15.11 | The menu loops | Buy one, do not press B | The menu reappears so you can buy the next |
| T15.12 | EXIT closes it | Pick EXIT | Conversation ends |
| T15.13 | B closes it | Press B on the menu | Conversation ends, nothing bought |
| T15.14 | **Already owned** | Buy the same ticket twice | "You have that one already", no second charge |
| T15.15 | No money | Set money to 0, try to buy | Refused, no ticket |
| T15.16 | All four | Buy every ticket | Four Key Items, all four `FLAG_ENABLE_SHIP_*` set |
| T15.17 | **Southern Island reachable** | With the Eon Ticket, talk to the Lilycove sailor | SOUTHERN ISLAND appears in the destination list |
| T15.18 | Faraway Island | Same with the Old Sea Map | FARAWAY ISLAND appears — Mew |
| T15.19 | Birth Island | Same with the Aurora Ticket | BIRTH ISLAND appears — Deoxys |
| T15.20 | Navel Rock | Same with the Mystic Ticket | NAVEL ROCK appears — Ho-Oh and Lugia |
| T15.21 | **The legendaries are randomized** | Flag 0x23 set, reach one of the islands | Not Mew / Deoxys / Lugia — a randomized species |
| T15.22 | Harbour scenes still work | Replay the Team Aqua submarine scene at Slateport Harbor | Unchanged; the seller does not stand in the way |
| T15.23 | Hidden patrons unaffected | Before beating the game, check the harbour | The vanilla patrons are still hidden by their flag; the seller is always visible |

---

## Phase 14 — New game defaults

`RANDOLOCKE_RANDOMIZE_ON_NEW_GAME` in `include/config/randolocke.h`. See
`docs/SETTINGS.md` §1.

| # | Test | Steps | Expected |
| --- | --- | --- | --- |
| T14.1 | **A new game is already randomized** | New game, walk into the first patch of grass | A randomized species — no debug menu needed |
| T14.2 | All ten flags are set | New game, then debug → Flags, read 0x20–0x2A | 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x28, 0x29, 0x2A all **set**; 0x27 (infinite repel) **clear** |
| T14.3 | Species mode var | Debug → Vars, read `0x404E` | 0 (`MON_RANDOM`) |
| T14.4 | **An existing save is untouched** | Load a save made before this change | Its flags are whatever they were — no silent re-randomization |
| T14.5 | Flags remain switchable | Clear 0x20, walk into grass | Vanilla encounters again |
| T14.6 | The change survives a save/reload | Set defaults, save, soft reset, load | Flags still set |
| T14.7 | Per-feature opt-out | Set `RANDOLOCKE_DEFAULT_ABILITIES` to `FALSE`, rebuild, new game | 0x26 clear, the other nine set |
| T14.8 | Master switch | Set `RANDOLOCKE_RANDOMIZE_ON_NEW_GAME` to `FALSE`, rebuild, new game | No flags set — stock tertu behaviour |
| T14.9 | **Release build has no debug menu** | `make release`, boot, hold R + START | Nothing opens — and T14.1 must still pass, which is the whole point of this phase |
| T14.10 | Seed still drives everything | Two new games with the same Trainer ID | Identical starters, identical Route 101 |

---

## §P — Patching and distribution

### To play your own build: no patching needed

`make` produces **`pokeemerald.gba`** in the repo root. That is a complete, playable
32 MB ROM. Open it directly in mGBA. `baserom.gba` is **not** used by the build at all
(nothing in the Makefile references it) — it exists only to create patches.

### To distribute: create a patch, never the ROM

Distributing a built ROM means distributing Nintendo's copyrighted game. The convention is
to distribute a small **patch file** that players apply to their own legally-obtained copy.

You need two files:

| Role | File | CRC32 |
| --- | --- | --- |
| Source (vanilla) | `baserom.gba` | `1f1c08fb` ✅ verified |
| Target (your hack) | `pokeemerald.gba` | changes every build |

Randolocke v1.1 states the same expected vanilla CRC32 (`1f1c08fb`), so your base matches
the one its players use.

**Format:** use **BPS**. It handles the 16 MB → 32 MB size change correctly and verifies
the source ROM's checksum before applying. (IPS cannot address beyond 16 MB, so it is not
suitable here.)

**Creating the patch — Flips (recommended, offline):**
1. Get [Floating IPS (Flips)](https://github.com/Alcaro/Flips/releases).
2. Choose *Create Patch*.
3. Original file → `baserom.gba`; Modified file → `pokeemerald.gba`.
4. Save as `randolocke-expanded-vX.Y.bps`.

**Applying a patch — online, no install:**
- **<https://www.marcrobledo.com/RomPatcher.js/>** — the standard browser-based patcher.
  Runs entirely client-side (your ROM is not uploaded anywhere). Load the vanilla ROM,
  load the `.bps`, click Apply, download the result.

Alternatives: [Rom Patcher JS on GitHub](https://github.com/marcrobledo/RomPatcher.js),
or Flips itself (*Apply Patch*).

**Distribute:** the `.bps` file plus a readme naming the expected vanilla CRC32
(`1f1c08fb`). Never the `.gba`.

### Sanity check before releasing

- [ ] Apply your own `.bps` to a *fresh* copy of vanilla Emerald
- [ ] Confirm the result's CRC32 matches your `pokeemerald.gba`
- [ ] Boot the patched ROM and run §R

---

## Appendix — useful mGBA techniques

- **Save states** (Shift+F1–F9 save, F1–F9 load) — invaluable for testing randomization
  stability: state-save before an event, reload, and confirm you get the *same* result.
- **Log view** (View → Log, enable "Game Error"/"Debug") shows `MgbaPrintf` output; the
  randomizer prints `GetSpeciesGroup:` lines in debug builds.
- **Reset vs. reload** — some randomizer bugs only appear after a true power cycle. Use
  *File → Reset*, not just a save-state reload, when testing persistence.
