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
| V.1 | **Nicknaming is optional** | Catch a Pokémon | The base game's "Do you want to give it a nickname?" prompt. `RANDOLOCKE_FORCE_NICKNAME` ships `FALSE` |
| V.2 | Saying no works | Answer no | Keeps the species name, no crash |
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

## Phase 18 — Nuzlocke rules

`RANDOLOCKE_NUZLOCKE_RULES` (TRUE). `RANDOLOCKE_FLAG_NUZLOCKE_OFF` (0x2D) switches them
off for a save — clear means **on**, so a save made before this existed gets the rules
with no new game needed.

An "area" is one entry in the wild encounter tables, which is one map. Places with no
wild table — the legendary sites, gift Pokémon, scripted battles — are never restricted.

| # | Test | Steps | Expected |
| --- | --- | --- | --- |
| T18.1 | **One catch per area** | Catch something on Route 101, then meet another wild Pokémon there and open the bag | Balls refused: "You already caught a Pokémon in this area!" |
| T18.2 | A different area is free | Go to Route 103 and catch something | Allowed |
| T18.3 | The mark survives a reload | Catch on Route 101, save, reset, reload, try again there | Still refused |
| T18.4 | **Dupes are refused** | Meet a species whose family you already own | "You've already caught this Pokémon's family!" |
| T18.5 | **A dupe does not use the area up** | On a fresh route, meet a dupe and run. Then meet something new there | The new one is catchable — the dupe did not count |
| T18.6 | Dupes work across the family | Catch a Zigzagoon, then meet a Linoone | Refused — the whole evolution family counts |
| T18.7 | …and in the other direction | Catch a Linoone first, then meet a Zigzagoon | Refused |
| T18.8 | **Shinies are always catchable** | Meet a shiny on an area you have already used | Allowed |
| T18.9 | A shiny does not use the area up | Catch a shiny on a fresh area, then meet something new there | Still catchable |
| T18.10 | A shiny dupe is catchable | Meet a shiny of a family you own | Allowed |
| T18.11 | Fishing shares the area | Catch on land on Route 103, then fish there | Refused — one map is one area |
| T18.12 | Surfing shares the area | Same, but surf | Refused |
| T18.13 | **Legendary sites are exempt** | Reach Sky Pillar and throw a ball | Allowed — no wild table, so not an area |
| T18.14 | Gift Pokémon are exempt | Take a gift Pokémon on a used-up area | Given normally |
| T18.15 | **Wally's tutorial still works** | Play the Petalburg catching tutorial | Wally catches the Zigzagoon; the story continues |
| T18.16 | Trainer battles unaffected | Try a ball in a trainer battle | The usual "trainer blocked it" |
| T18.17 | **Off switch** | Debug → Flags, set 0x2D | Balls work everywhere again |
| T18.18 | On by default for an old save | Load a save made before this build | Rules apply, no new game needed |
| T18.19 | Areas start clear on a new game | New game, catch on Route 101 | Allowed |
| T18.20 | No false positives from the starter | New game; your starter is registered caught. Meet an unrelated species | Catchable |
| T18.21 | Safari Zone | Catch one there, then try again | Refused — the Safari Zone map is one area |
| T18.22 | Box-full still reports correctly | Fill the box, then throw | The box-full message, not a nuzlocke one |

---

## Phase 19 — Wipes, trainer EVs, and when the rules start

`RANDOLOCKE_WIPE_COSTS_PARTY`, `RANDOLOCKE_RUN_OVER_ON_WIPE`, `RZ_TRAINER_EV_SCALING`.
See `docs/NUZLOCKE.md`.

### When the rules begin

| # | Test | Steps | Expected |
| --- | --- | --- | --- |
| T19.1 | **Nothing applies before the Poké Balls** | New game. Before returning to the lab, check the bag in a wild battle | Balls behave normally; no nuzlocke messages |
| T19.2 | Losing the Route 103 rival battle is free | Lose it on purpose | Ordinary white-out; nothing is boxed |
| T19.3 | **The rules start at the five Poké Balls** | Beat the rival, return to the lab, take the balls. Then catch twice on one route | Second catch refused |
| T19.4 | The starter is not an area catch | After T19.3, catch on Route 101 | Allowed |
| T19.5 | Flag check | Debug → Flags, read `FLAG_ADVENTURE_STARTED` | Clear before the balls, set after |

### Individual faints

| # | Test | Steps | Expected |
| --- | --- | --- | --- |
| T19.6 | **A single faint costs nothing** | Let one Pokémon faint with others still standing | Stays in your party, fainted. Heal at a Center and carry on |
| T19.7 | Field poison faint | Let a poisoned Pokémon faint while walking, with others alive | Same — stays in the party |

### Wipes

| # | Test | Steps | Expected |
| --- | --- | --- | --- |
| T19.8 | **A wipe boxes the whole party** | Lose with every Pokémon down, with something living in a box | Party empty; all of them in a box, marked |
| T19.9 | Held items come back | Give one Leftovers, then wipe | Leftovers in your bag |
| T19.10 | You start at the Pokémon Center | After T19.8 | Last Pokémon Center, empty party |
| T19.11 | **Withdraw a new team** | Use the PC | Living Pokémon can be withdrawn |
| T19.12 | The wiped team cannot come back | Try to withdraw one of them | "This POKéMON is gone for good." |
| T19.13 | Nor moved or shifted | Try MOVE and SHIFT on one | Same refusal |
| T19.14 | It can be released | Choose RELEASE on one | Allowed |
| T19.15 | **No wild battles while empty** | Walk out of the Center into grass with an empty party | No encounters at all |
| T19.16 | Eggs survive a wipe | Carry an egg through one | Still in your party |
| T19.17 | **Nothing left ends the run** | Wipe with no living Pokémon anywhere | Back to the title screen |
| T19.18 | The save is not deleted | After T19.17, load the save | Loads: last Pokémon Center, empty party |
| T19.19 | The ending can be escaped | After T19.17, set flag 0x2D, withdraw a wiped Pokémon | Playable again |
| T19.20 | Champion lifts the lock | Beat the Champion, open the PC | Wiped Pokémon are withdrawable |
| T19.21 | Frontier losses are safe | Lose a Frontier battle | Nothing is boxed |
| T19.22 | Config off | Set `RANDOLOCKE_WIPE_COSTS_PARTY` to `FALSE`, rebuild, wipe | Vanilla white-out, party kept |

### Trainer EVs

| # | Test | Steps | Expected |
| --- | --- | --- | --- |
| T19.23 | **Trainers hit harder than vanilla** | Fight Roxanne at the level cap with an untrained team | Noticeably tougher than a 0-EV party |
| T19.24 | The spread follows badges | Compare an early trainer with a late one | Late trainers are bulkier and faster for their level |
| T19.25 | **The spread follows the species** | Fight a trainer whose randomized Pokémon is a special attacker | Its Sp. Atk is boosted, not its Atk |
| T19.26 | The player is not capped | EV train one of yours to 252 in a stat | Allowed — `B_EV_CAP_TYPE` is `EV_CAP_NONE` |
| T19.27 | Vitamins still work | Buy and use one | Normal |
| T19.28 | Config off | Set `RZ_TRAINER_EV_SCALING` to `FALSE`, rebuild | Trainers back to zero EVs |

### Level caps against boss levels

| # | Test | Steps | Expected |
| --- | --- | --- | --- |
| T19.29 | **Cap equals the next boss's ace** | Before each gym, check your cap and the leader's highest level | Equal at every badge: 14 / 21 / 24 / 29 / 36 / 43 / 47 / 50 |
| T19.30 | The 8-badge cap covers the Elite Four | Check after the 8th badge, then Sidney through Wallace | Cap 63; Sidney's ace 53 rising to Wallace's 63 |
| T19.31 | Champion lifts it | After beating the Champion | Cap 100 |
| T19.32 | Hard cap, not soft | Battle at the cap | **No** experience at all, not reduced |
| T19.33 | Cap Candy reaches it | Use one below the cap | Levels to the cap |

### Key items

| # | Test | Steps | Expected |
| --- | --- | --- | --- |
| T19.34 | **The Oldale old man gives two** | After the five Poké Balls, talk to the man near the Oldale Mart sign | CAP CANDY and REPELLANT |
| T19.35 | He does not repeat | Talk again | His ordinary footprints line |
| T19.36 | He waits for the balls | Talk to him before the adventure starts | The vanilla blocking-the-path scene, no items |
| T19.37 | The Littleroot boy gives the other two | Talk to the boy by the pond | PORTA HEAL and ENDLESS CANDY |
| T19.38 | Works on an existing save | Load a save from before this build, talk to both | All four handed over |
| T19.39 | Repellant toggles | Use it, walk, use it again | Repel on, then off; the step counter never ticks down while on |
| T19.40 | Cap Candy respects the cap | Use it on a Pokémon already at the cap | Refused or no-op, never over the cap |

---

## Phase 20 — BST, IVs, AI, tutors

| # | Test | Steps | Expected |
| --- | --- | --- | --- |
| T20.1 | **Similar-BST substitution** | New game, survey Route 101 | Small, weak species — no Rayquaza in the starting grass |
| T20.2 | Late routes scale up | Survey Victory Road | Strong species; the BST band moved with the originals |
| T20.3 | **Legendary sites ignore BST** | Reach Sky Pillar | Still a legendary, because those twelve force legend-aware |
| T20.4 | Mode is switchable | Set var `0x404E` to 0 | Back to anything-goes on new rolls |
| T20.5 | **Caught Pokémon have 31s** | Catch anything, check the summary IVs | 31 in all six |
| T20.6 | Starters too | New game, check your starter | 31 in all six |
| T20.7 | Gifts too | Take any gift Pokémon | 31 in all six |
| T20.8 | Hatched eggs too | Hatch the Wynaut egg | 31 in all six |
| T20.9 | **Trainers are NOT given 31s** | Fight a trainer, compare damage against a known 31-IV target | Trainer Pokémon still roll their own IVs |
| T20.10 | The alternative mode | Set `RANDOLOCKE_PLAYER_IVS` to `RANDOLOCKE_IVS_RANDOLOCKE`, rebuild | Starters/gifts get 3 perfect IVs; caught Pokémon roll random |
| T20.11 | **A cave is one area** | Catch on Granite Cave 1F, go to B1F, try again | Refused — one region map section is one area |
| T20.12 | Magma Hideout | Catch on one floor, try another | Refused. This is the case Randolocke's notes call out |
| T20.13 | Different routes still separate | Catch on Route 101, then Route 103 | Allowed |
| T20.14 | **One catch per area actually blocks now** | Catch on Route 101, meet another there, open the bag | Balls refused. This never worked before Phase 20 |
| T20.15 | **Bosses fight properly** | Fight Roxanne | Switches sensibly, targets KOs, saves her ace for last |
| T20.16 | **Every boss is omniscient** | Fight any gym leader | Plays around your moves, abilities and held items as if it has seen them — no Surf into Water Absorb, no setting up on your revenge killer |
| T20.16b | The Champion reads ahead too | Fight Wallace | Also anticipates your switches and the move you are about to pick |
| T20.16c | Ordinary trainers are not omniscient | Fight a Youngster | Still walks into your immunities the first time |
| T20.17 | Route trainers are competent, not brutal | Fight a Youngster | Avoids bad moves and goes for KOs, but no switching games |
| T20.18 | Admins and rivals sit between | Fight an Aqua Admin or your rival | Smarter switching than a Youngster, less than a leader |
| T20.19 | Config off | Set `RZ_TRAINER_AI_TIERS` to `FALSE`, rebuild | Back to the vanilla AI |
| T20.20 | **Tutors teach randomized moves** | Talk to any of the ten tutors with flag `0x2E` set | Offers something other than its vanilla move |
| T20.21 | **The offer names the right move** | Read the tutor's dialogue | The flavour line plays, then "I can teach X" naming what is actually taught |
| T20.22 | The prompt agrees | Say yes | "Which POKéMON should learn X?" — the same X |
| T20.23 | It teaches what it said | Teach it | The Pokémon learns X |
| T20.24 | No duplicates among tutors | Check all ten | Ten different moves |
| T20.25 | **No overlap with TMs** | Compare the ten tutor moves against the 50 TMs | No move appears in both |
| T20.26 | Stable across a reload | Note all ten, soft reset, check again | Identical |
| T20.27 | Flag off is vanilla | Clear flag `0x2E` | Swagger, Rollout, Fury Cutter, Mimic, Metronome, Sleep Talk, Substitute, Dynamic Punch, Double-Edge, Explosion — and the text still reads correctly |
| T20.28 | Once-only tutors still are | Teach one, come back | Refuses, as in vanilla |

---

## Phase 21 — Hidden nature roller

Debug menu → Edit Pokémon → **Roll Hidden Nature**. "Set Hidden Nature" already existed
for picking one deliberately; this is the dice version.

| # | Test | Steps | Expected |
| --- | --- | --- | --- |
| T21.1 | **It rolls** | Pick a party Pokémon, Roll Hidden Nature | A message naming the Pokémon and its new nature |
| T21.2 | **It always changes** | Roll the same Pokémon ten times | Never lands on the nature it already had |
| T21.3 | Stats update immediately | Note Attack, roll into an Adamant or Modest | Attack changes on the spot — no level-up needed |
| T21.4 | The summary agrees | Open the summary after rolling | The new nature is shown |
| T21.5 | Eggs are skipped | Choose an egg | Nothing happens, no crash |
| T21.6 | Cancelling is safe | Back out of the party menu | Nothing changes |
| T21.7 | The true nature is untouched | Roll the hidden nature, then check "Set Nature" | The personality-derived nature is unchanged; only the hidden one moved |
| T21.8 | It persists | Roll, save, reset, reload | The rolled nature is still there |

---

## Phase 22 — Summary stat editor and the LEVEL CAP option

`RANDOLOCKE_SUMMARY_STAT_EDITOR`. On the summary's skills page, press A to cycle
Stats → IVs → EVs, then **SELECT** to edit in place.

| Key | Effect |
| --- | --- |
| SELECT | start editing / stop editing |
| A | move to the next stat, in reading order |
| Up | jump to the maximum (31 IV, 252 EV) |
| Down | jump to zero |
| Right | one higher |
| Left | one lower |
| B | stop editing |

| # | Test | Steps | Expected |
| --- | --- | --- | --- |
| T22.1 | **SELECT starts editing** | Summary → skills → IVs, press SELECT | HP is highlighted in the raised-stat colour |
| T22.2 | A walks the stats | Press A repeatedly | HP → Attack → Defense → Sp. Atk → Sp. Def → Speed → back to HP, in reading order |
| T22.3 | Up maxes an IV | On an IV, press Up | 31 |
| T22.4 | Down zeroes it | Press Down | 0 |
| T22.5 | Left and Right step | Press Right three times, Left once | +2 from where it started |
| T22.6 | IVs stop at 31 | Press Right at 31 | Refused, failure beep |
| T22.7 | **Stats update live** | Edit an Attack IV and watch the Stats page | The real stat changed — no level-up needed |
| T22.8 | **EVs stop at 252** | On the EV page, press Up on one stat | 252, not more |
| T22.9 | **The 510 total holds** | Max two stats (504), then try to raise a third | Refused. 6 more points are available, so Right works 6 times and then stops |
| T22.10 | Lowering frees budget | Zero one maxed stat, then raise another | The freed points are spendable |
| T22.11 | D-pad does not leak | While editing, press Left and Right | The page does **not** change; the value does |
| T22.12 | Up/Down do not switch Pokémon | While editing, press Up and Down | The party member does **not** change |
| T22.13 | SELECT exits | Press SELECT again | Highlight gone; D-pad navigates the summary as usual |
| T22.14 | B exits without closing | Press B while editing | Editing stops, the summary stays open |
| T22.15 | **Boxed Pokémon are not editable** | Open a boxed Pokémon's summary, press SELECT on the IV page | Nothing happens |
| T22.16 | Eggs are not editable | Summary of an egg | No edit mode |
| T22.17 | The Stats page is not editable | Press SELECT on the Stats view | Nothing happens |
| T22.18 | Changes persist | Edit, close the summary, save, reset, reload | The values stuck |

### LEVEL CAP from the party menu

| # | Test | Steps | Expected |
| --- | --- | --- | --- |
| T22.19 | **The option appears** | Hold a Cap Candy, open the party menu, pick a Pokémon below the cap | A **LEVEL CAP** entry under SUMMARY |
| T22.20 | It levels to the cap | Choose it | The Pokémon goes to the current cap in one use |
| T22.21 | Hidden without the item | Toss the Cap Candy, reopen the menu | No LEVEL CAP entry |
| T22.22 | Hidden at the cap | Pick a Pokémon already at the cap | No LEVEL CAP entry |
| T22.23 | Moves and evolutions still happen | Use it on something with a level-up move on the way | It learns the move, and evolves if it should |
| T22.24 | The item is not consumed | Check the bag afterwards | Cap Candy still there |

---

## Phase 23 — Type effectiveness in move select, and the capture fix

| # | Test | Steps | Expected |
| --- | --- | --- | --- |
| T23.1 | **Effectiveness shows on a species you have never seen** | Battle a randomized wild Pokémon, open FIGHT | The PP line carries an effectiveness icon. It used to be gated on having *seen* the species, which in a randomizer is never |
| T23.2 | Super effective | Point at a move the target is weak to | A green up arrow |
| T23.3 | 4x | A double weakness | Two green up arrows |
| T23.4 | Not very effective | A resisted move | A red down arrow |
| T23.5 | 0.25x | A double resist | Two red down arrows |
| T23.6 | **No effect** | A move the target is immune to | A red X |
| T23.7 | Neutral | A neutral move | The hollow circle, unchanged |
| T23.8 | Status moves | Point at a status move | No icon — effectiveness does not apply |
| T23.9 | **Doubles picks the right target** | In a double battle, choose a target, then look at the icon | It reflects the selected target, not the other one |
| T23.10 | The L-button detail view still works | Press L on the move list | Unchanged |
| T23.11a | **STAB shows a red dot** | Point at a move whose type matches your Pokémon's | A filled red circle after the effectiveness icon |
| T23.11b | Non-STAB shows none | Point at a move of an unrelated type | No dot |
| T23.11c | Both indicators together | A super-effective STAB move | Green up arrow **and** the red dot |
| T23.11d | 4x STAB fits the window | A doubled-weakness STAB move | Two arrows and the dot, nothing clipped |
| T23.11e | Status moves get neither | Point at a status move | No arrow, no dot |
| T23.11f | Dual types both count | A Pokémon with two types, one move of each | Both show the dot |
| T23.11g | The PP label is gone | Look at the window | Icons only — the PP number was already replaced, so the label described nothing |

### The post-capture softlock

| # | Test | Steps | Expected |
| --- | --- | --- | --- |
| T23.11 | **Catching does not hang** | Catch several Pokémon on different routes, including with a Fast Ball | The battle ends, the overworld returns, the player is controllable |
| T23.12 | Catching something with a big family | Catch an Eevee or a Wurmple line member | No hang — the family walk is bounded now |
| T23.13 | Perfect IVs still applied | Check a freshly caught Pokémon's IVs | 31 across the board |
| T23.14 | Area still marked | Catch, then try to catch again on the same route | Refused |
| T23.15 | HP is not corrupted | Catch a Pokémon at low HP, check it afterwards | Sensible current and max HP |

### Other adjustments

| # | Test | Steps | Expected |
| --- | --- | --- | --- |
| T23.16 | **Battle style is SET** | New game, beat a trainer's Pokémon | No "will you switch?" prompt |
| T23.17 | Still changeable | Options → Battle Style | Can be set back to SHIFT |
| T23.18 | **Berries come in fours** | Harvest a berry tree | Four times the usual count |
| T23.19 | **The boy gives all four key items** | Talk to the boy by the Littleroot pond | Porta Heal, Endless Candy, Cap Candy, Repellant |
| T23.20 | The old man is flavour again | Talk to the Oldale footprints man | His footprints line, no items |
| T23.21 | **Registered items are vanilla** | Register a key item, press SELECT | Works as the base game does. No hold behaviour |
| T23.22 | Cap Candy pauses correctly | Use LEVEL CAP on something with a move coming up | Stops at that level, reports that level, and the summary agrees |
| T23.23 | Pressing it again continues | Use it again | Climbs to the next stop, or the cap |

---

## Phase 28 — Playtest round 5

### NPC gift items

Items an NPC hands over now go through the same randomizer as item balls, via a hook at
the top of `Std_ObtainItem` — 158 `giveitem` calls, every gift in the game. HMs and the
whole key items pocket are refused by `ShouldRandomizeItem`, and Poké Balls are held back
separately. Follows the same toggle as field items (`RANDOMIZE_FIELD_ITEMS`).

| # | Test | Steps | Expected |
| --- | --- | --- | --- |
| T28.1 | **Ordinary gifts are randomized** | Rustboro — the man who gives a Quick Claw | Something other than a Quick Claw |
| T28.2 | The message agrees with the bag | Read the "obtained the …" line, then open the bag | Same item in both. The message never names the original |
| T28.3 | The fanfare and pocket agree | Receive a gift that rolls into a TM | TM fanfare, "put away in the TM CASE" |
| T28.4 | It is stable | Save before the gift, take it, reset, take it again | The same replacement both times |
| T28.5 | **The five Poké Balls survive** | Birch's lab — the starting Poké Balls | Five Poké Balls. Not randomized |
| T28.6 | **HMs survive** | Cut, Flash, Rock Smash, Strength, Surf, Waterfall, Dive | Each is the HM it should be |
| T28.7 | **Key items survive** | Letter, Devon Goods, Devon Scope, Go-Goggles, both bikes, all three rods, Wailmer Pail, Soot Sack, Contest Pass, Meteorite, Scanner, the tickets | Each is itself. The story never blocks |
| T28.8 | Gym TM rewards are randomized | Beat a gym, take the leader's TM | Some other item or TM |
| T28.9 | Berry gifts are randomized | Route 123 Berry Master | Random items rather than the named berries |
| T28.10 | Purchases are untouched | Game Corner prizes, Lilycove rooftop, Frontier exchange | Exactly what was chosen. These use `additem`, not the gift path |
| T28.11 | Bag-full still handled | Fill the items pocket, then take a gift | "no room" message, gift not lost |

### TM descriptions name the move they teach

| # | Test | Steps | Expected |
| --- | --- | --- | --- |
| T28.12 | **The bag shows the move's description** | Bag → TM/HM pocket → hover a randomized TM | The description of the move it teaches, matching the hover panel's type and PP |
| T28.13 | It is re-wrapped, not clipped | Hover a TM whose move has a long description | Three lines at most, nothing running off the right edge |
| T28.14 | The mart agrees | Mauville or Lilycove mart → a TM | Same description as the bag |
| T28.15 | HMs read correctly | Bag → an HM | The HM's move description |
| T28.16 | Non-TMs are unchanged | Hover a Potion, a berry, a key item | Their own descriptions, wrapped as before |

### Randomized trainers get their own moves

`CustomTrainerPartyAssignMoves` kept the hand-written moveset for a Pokémon whose species
had been substituted, so Roxanne's whole team carried Tackle / Defense Curl / Rock Throw /
Rock Tomb regardless of what they became — no same-type attacks and three identical
Pokémon. It now falls back to the level-up learnset, which is itself randomized.

| # | Test | Steps | Expected |
| --- | --- | --- | --- |
| T28.17 | **Roxanne's team has its own moves** | Fight Roxanne | Three different move lists, suited to the three species. No shared Rock Tomb |
| T28.18 | Same-type moves appear | Watch a gym leader's Pokémon attack | Moves matching its own types, since the learnset gives it seven |
| T28.19 | Ordinary trainers too | Any route trainer with a written moveset | Moves that fit the species it became |
| T28.20 | Movesets are stable | Save before a battle, fight, reset, fight again | The same moves |
| T28.21 | PP is right | Check a trainer Pokémon's PP in battle | Full PP for the move it actually has |
| T28.22 | **Bosses are harder now** | Fight a gym leader | Four of the species' strongest available moves plus boss AI. Expected — note if it is too much |
| T28.23 | Low-level trainers still work | The first Route 102 trainer | Has at least one move; nothing blank or Struggle-only |
| T28.24 | Wally's Ralts | The Petalburg tutorial catch | Battle plays out normally |

---

## Phase 58 — Six-Pokémon bosses, an IV ramp, the Non-Shiny Repel, and TMs that say what they teach

### What changed

**1. Bosses and rivals bring six** (`RZ_BOSS_FULL_PARTY`). Gym leaders, the Elite Four, the
Champion, Magma and Aqua leaders and admins, and every rival battle (May or Brendan, Wally).
The added Pokémon are built from the trainer's own team: a random level between its lowest
and highest, a species randomized like any other slot, and a boss's EVs, nature, item and
IVs. They go in ahead of the ace, which moves to the last slot — where `IsAceMon` looks for
it — so it still comes out last. The data file's own Pokémon keep the seeds they always had,
so their species are unchanged. Not padded: the Route 103 rival battle, and the Mossdeep
double battle with Steven (two opponents at three each by design).

**2. A boss's perfect IVs ramp with the badges** (`RZ_BOSS_IV_RAMP`). Until now every boss
Pokémon had 31 everywhere. Now all roll like anyone's, and the best are raised, by level with
the ace first: 1 perfect at 0 badges, half a step per badge (a three-of-six Pokémon, then
another perfect one), 5 perfect at 8. The Elite Four and the Champion stay perfect throughout.

**3. No TM or tutor teaches an HM's move.** Over 64 seeds, 21 HM moves had been dealt to TMs
and tutors. The rule re-rolls those slots, so a seed that dealt one gets a different table
from that point on. **The playtest seed 0x8561D8DD never dealt one: its 50 TMs and 10 tutors
are identical before and after** (compared directly). Its only Flying-type TM is TM35
Bounce, a different move from Fly.

**4. The Non-Shiny Repel**, a toggled key item from the Littleroot boy. While on, a wild
Pokémon met walking, surfing, smashing rocks or in a mass outbreak is only met if shiny —
at any level, over the Repellant's level check and Keen Eye's. Roamers get through only if
shiny. Fishing, Sweet Scent and scripted encounters are untouched. Flag `0x02F`.

**5. Gym leaders describe the TM actually given**, and **tutors describe the move actually
taught.** A leader's explanation names the TM received, the move it teaches, and that move's
own description, laid out for the message box. It reads the item from `VAR_0x8006` — the
headless run below first showed "That TECHNICAL MACHINE, Great Ball, contains Great Ball":
the obtain script's `switch VAR_RESULT` copies the pocket number into `VAR_0x8000`, and
pocket 3 is the Great Ball's item ID. Roxanne's line that a TM is used up once is gone too;
TMs are reusable here. The tutors' offer adds the description, and their flavour lines that
described the vanilla move (Fury Cutter's whole introduction, Metronome's finger-waggling,
Substitute's copy of itself) were reworded.

**6. Every evolution item is sold.** Metal Coat, King's Rock, Razor Claw, Razor Fang, Deep
Sea Tooth and Deep Sea Scale join the twelve marts at ¥200. `add_cheap_shop.py` now reads
which items species evolve by from the species data and tops up marts it stocked before; the
Gimmighoul Coin keeps its hand-set ¥1.

**7. Held-item trade evolutions work from the Bag** (`I_USE_EVO_HELD_ITEMS_FROM_BAG`, now
TRUE). The species data gives every held-item trade evolution an item route — Onix + Metal
Coat, Seadra + Dragon Scale, Porygon + Up-Grade — and the marts sell those items, but with
the engine setting off every one of them was "can't use" in the Bag. Without a link cable,
Onix, Scyther, Seadra, Porygon, Porygon2, Rhydon, Electabuzz, Magmar, Dusclops, Clamperl,
Feebas, Poliwhirl, Slowpoke, Spritzee and Swirlix could never evolve. Only the Linking Cord
had been usable. A new test checks that every item any species evolves by with `EVO_ITEM` is
usable from the Bag; with the setting off it names the King's Rock and the Metal Coat.

The TM and tutor tables now also rebuild if the seed changes, as the legendary table did.
A soft reset already clears them, so this changes nothing in play; it lets tests look at
more than one seed.

No save-layout change. The new item is appended after the last ID, the flag was unused,
and nothing in the save structures changed. An existing save keeps working: the boy gives
the Non-Shiny Repel to anyone who does not have it.

### Headless check (real trainer data, a new game)

| Trainer | Badges | Result |
| --- | --- | --- |
| Roxanne (3 in data) | 0 | 6 Pokémon at 11–14, the level-14 ace last; 1 perfect (the ace) |
| Roxanne | 1 | + one Pokémon with 3/6 perfect |
| Wally, Mauville (1 in data) | 3 | 6 at level 16, IVs rolled (a rival, not a boss) |
| May, Route 103 | 0 | 1 — not padded |
| Brendan, Route 110 | 3 | 6 at 19–22 |
| Archie | 7 | 4 perfect + one 3/6, ranked by level |
| Tate & Liza (double) | 7 | 6; 4 perfect + one 3/6 |
| Sidney, Wallace | 8 | 6, all perfect |

Roxanne's gift, played through the real script: "Obtained the TM10!" … "That TECHNICAL
MACHINE, TM10, contains Dragon Breath. Strikes the foe with a blast of breath. May
paralyze." The Fury Cutter tutor: "There's a move I think is wickedly cool, and I love
teaching it." … "I can teach Lick to one of your POKéMON. Licks with a long tongue to
injure. May also paralyze. Would you like me to?" The Littleroot boy: hands over all five
key items to a new game, the Non-Shiny Repel included.

New and rewritten tests, each confirmed to fail with its fix reverted:

- `Randolocke: bosses and rivals bring six Pokemon, the ace still last`
- `Randolocke: the first rival battle, ordinary trainers and half teams keep their size`
- `Randolocke: a boss's perfect IVs ramp with the badges`, `… the Elite Four are perfect throughout`
- `Randolocke: no TM or tutor teaches an HM's move` (64 seeds; 21 hits with the rule removed)
- `Randolocke: the Non-Shiny Repel lets only shiny Pokemon through` (1,500 rolls: 27 met, all shiny)
- `Randolocke: every move description fits the message box`, `… a gym leader describes the TM actually handed over`
- `Randolocke: every evolution item can be used from the Bag`, `… a held-item trade evolution happens from the Bag`

The trainer tests now build their own trainers. The test build swaps `gTrainers` for the
test framework's fixtures, where `TRAINER_ROXANNE_1` has no party at all — so the old
version of the EV and item test was looping over an empty party and checking nothing.

| # | Test | Steps | Expected |
| --- | --- | --- | --- |
| T58.1 | **A gym leader's team** | Challenge any gym leader | Six Pokémon; the last one out is the strongest |
| T58.2 | Route 103 | (New game) the first rival battle | One Pokémon, as before |
| T58.3 | A rival | Brendan/May on Route 110 or Wally | Six Pokémon |
| T58.4 | Steven's double battle | Mossdeep Space Center | Three each for Maxie and Tabitha, as before |
| T58.5 | **The Non-Shiny Repel** | Talk to the boy by the Littleroot pond; use it from Key Items; walk in grass | "Only shiny POKéMON will appear." Nothing appears until a shiny does |
| T58.6 | Off again | Use it again | "switched off"; encounters return |
| T58.7 | **A gym TM** | Beat a gym leader | The explanation names the TM you received and describes its move |
| T58.8 | **A tutor** | Talk to any move tutor | The offer names and describes the randomized move |
| T58.9 | **A TM** | Look through your TMs | None teaches Cut, Fly, Surf, Strength, Flash, Rock Smash, Waterfall or Dive |
| T58.10 | **Evolution items** | Visit any Poké Mart | Metal Coat, King's Rock, Razor Claw, Razor Fang, Deep Sea Tooth and Scale at ¥200 |
| T58.11 | A trade evolution | Buy a Metal Coat, use it on an Onix from the Bag | It evolves into Steelix |
| T58.12 | Regression tests | `make check TESTS="Randolocke"` | PASS — 37 |

---

## Phase 57 — Box legendaries, no more Tackle, one-Pokémon items down a tier, evolution moves kept

### What changed

**1. The twelve legendary sites give box legendaries only** — no mythicals now, as well as
no sub-legendaries or Ultra Beasts — and **one of each, in its standard form**. The
randomizer permits six Zygardes, Complete and Mega among them (forms that exist only
mid-battle), so Zygarde drew 6 times in 33 and turned up at two sites of one seed. Keeping
only species that are their own base form leaves 27 legendaries, equally likely, none at
two sites. Changes what every seed gives; recomputed from the seed, nothing in the save.

**2. Tackle.** Every learnset is 7 STAB, 7 status and 7 non-STAB damaging moves. Each group
drew blindly from the move tiers and only then checked the filter, with 512 tries; a
narrow filter (seven physical Fairy moves, seven Bug moves for a special attacker) ran out,
and every slot left over became Tackle. Measured on two seeds before the fix:

| Seed | Species | With Tackle | Tackles | Of which padding |
| --- | --- | --- | --- | --- |
| 0x8561D8DD | 1571 | 381 | 914 | 911 |
| 0x12345678 | 1571 | 387 | 915 | 914 |

Tackle is in the Homeless tier (weight 39 of 10,000) and hardly ever comes up by right. A
group that comes up short is now finished from the moves that fit, weighted as the tiers
weight them; if not enough exist, the category is relaxed before the type. Groups that
filled on their own are untouched. After: 3, 1 and 5 Tackles across the whole dex on three
seeds, all real draws.

**3. Items only one Pokémon can use drop a tier.** The 36 already forced to tier 4
(memories, drives, Light Ball, Thick Club, Leek, Soul Dew, the signature orbs, Booster
Energy) go to tier 5, and so do 27 evolution items only one species evolves by, which the
generator now finds from the species data: Whipped Dream, Sachet, Reaper Cloth, Protector,
Electirizer, Magmarizer, Upgrade, Dubious Disc, Dragon Scale, Prism Scale, Oval Stone, the
seven Milcery sweets, both apples, both pots, both Galarica items, both armors and the
Gimmighoul Coin. Those with a real held effect (King's Rock, Metal Coat, Razor Claw/Fang)
stay put. 63 items move from tier 4 to tier 5; tiers 1–3 are unchanged. Together they were
3.26% of field items and are now 1.11%. Tier 4, now just Poké Balls, evolution stones,
Exp. Candies and Metal Coat, is shared by fewer items, so each of those is more common.
Mega stones were already in tier 5, the bottom; nothing is below it.

**4. Pokémon that evolve by a move can always learn it.** Seventeen species (the table is
in docs/SETTINGS.md): Steenee/Stomp, Bonsly and Mime Jr./Mimic, Aipom/Double Hit,
Yanma, Tangela and Piloswine/Ancient Power, Lickitung/Rollout, Girafarig/Twin Beam,
Dunsparce/Hyper Drill, Hisuian Qwilfish/Barb Barrage, Poipole/Dragon Pulse,
Clobbopus/Taunt, Dipplin/Dragon Cheer, Primeape/Rage Fist, Stantler/Psyshield Bash, and
Eevee with a Fairy move for Sylveon. Read at runtime from the evolution table. The move
goes in at the level the species learns it in its own data, or the level it first exists
at if later; any level-up move can be relearned from the summary screen.

For the playtest seed 0x8561D8DD:

| Site | Now |
| --- | --- |
| Sky Pillar | Zekrom |
| Terra Cave | Calyrex |
| Marine Cave | Ho-Oh |
| Desert Ruins | Koraidon |
| Island Cave | Solgaleo |
| Ancient Tomb | Lugia |
| Southern Island A / B | Xerneas / Palkia |
| Faraway Island | Lunala |
| Birth Island | Zacian |
| Navel Rock top / bottom | Terapagos / Necrozma |

| Species | Move | Level |
| --- | --- | --- |
| Steenee | Stomp | 28 |
| Bonsly | Mimic | 16 |
| Mime Jr. | Mimic | 32 |
| Aipom | Double Hit | 32 |
| Yanma | Ancient Power | 36 |
| Tangela | Ancient Power | 24 |
| Piloswine | Ancient Power | 36 |
| Lickitung | Rollout | 7 |
| Girafarig | Twin Beam | 32 |
| Dunsparce | Hyper Drill | 32 |
| Hisuian Qwilfish | Barb Barrage | 44 |
| Poipole | Dragon Pulse | 1 |
| Clobbopus | Taunt | 36 |
| Dipplin | Dragon Cheer | 1 |
| Primeape | Rage Fist | 36 |
| Stantler | Psyshield Bash | 1 |
| Eevee | Baby-Doll Eyes (Fairy) | 16 |

New tests, each confirmed to fail with its fix reverted (the reverted learnset code
reproduces exactly 914 Tackles):

- `Randolocke: randomized learnsets fill every group` — every slot keeps to its group's
  rule and no learnset repeats a move, on three seeds
- `Randolocke: a Pokemon that evolves by a move can always learn it` — all 17, four seeds
- `Randolocke: the legendary sites give twelve different box legendaries` — five seeds

No save-layout change; no new game needed. Pokémon already caught keep the moves they know.

| # | Test | Steps | Expected |
| --- | --- | --- | --- |
| T57.1 | **Tackle** | Look through the level-up moves of a few Bug, Fairy and Ghost Pokémon in the Pokédex or relearner | No Tackle padding; every early slot is a real move |
| T57.2 | **An evolution move** | Get a Steenee (or any species in the table) and open its summary-screen relearner | Stomp is in the list; teach it, level up, and it evolves |
| T57.3 | Eevee | Check Eevee's relearner list | A Fairy move (Baby-Doll Eyes) is there |
| T57.4 | Primeape | Teach Rage Fist and use it 20 times, then level up | Evolves into Annihilape |
| T57.5 | **A legendary site** | Clear any legendary site | A box legendary — never a mythical, sub-legendary or Ultra Beast, never a form like Zygarde Complete |
| T57.6 | Two sites | Clear a second | A different Pokémon |
| T57.7 | **Field items** | Pick up items for a while | Species-only items (memories, drives, Light Ball, Whipped Dream…) turn up about a third as often as before |
| T57.8 | Regression tests | `make check TESTS="Randolocke"` | PASS — 26 |

---

## Phase 56 — Two freezes, and legendary sites worth the walk

### What changed

**1. R on the bike froze the game.** `RANDOLOCKE_DUAL_BIKE`'s swap returned TRUE from
`ProcessPlayerFieldInput`, and TRUE there means "a script has taken over": `CB1_Overworld`
answers it with `LockPlayerFieldControls()`. The swap starts no script, so nothing ever
released those controls and the game stopped dead with the bike swapped under you. It
returns FALSE now, and the frame walks on like any other.

**2. The League door froze the game for a player who passed the rule.** The
one-legendary trigger's refusal path ended with `releaseall`; the all-clear path just
ended. That is not enough, and not for the reason it looks like: `ScriptContext_RunScript`
unlocks the field controls itself when a script finishes. What is left behind is the
*step*. The trigger fires mid-stride, and the player's object event keeps
`heldMovementActive` and `heldMovementFinished` both set, which blocks every step after it
— `releaseall` is what calls `ObjectEventClearHeldMovementIfFinished` on the player
(`ScrCmd_releaseall`, src/scrcmd.c). So the player who was *allowed* through froze on the
doorstep, while the one who was turned away was fine. Both paths release now.

The other scripts this project adds were audited for the same shape: they are all `call`ed
and end on `return`, so they carry no release of their own and need none.

**3. The twelve legendary sites draw from the box legendaries and the mythicals only**
(`RANDOLOCKE_LEGENDARY_SITES_BOX_ONLY`). `MON_RANDOM_LEGEND_AWARE` kept a site legendary
but pooled every legendary there is, so most sites handed over sub-legendaries or Ultra
Beasts — and a site could roll its own species back, which is what this seed did. The pool is
narrowed with a filter on the unique-list draw; `GetUniqueMonListFiltered` carries a
bounded fallback so a filter that rejects everything can never spin the rejection loop
forever on hardware.

**4. The Sky Pillar's legendary is level 63**, the Elite Four's cap, rather than vanilla's
70. Terra Cave and Marine Cave are untouched at 70.

Changing the pool changes what a seed produces, so an existing game gets a different set of
twelve — recomputed from the seed at the next encounter, including at a site already
visited. No save-layout change; no new game needed.

### Headless check

A throwaway autopilot (not committed) with the player's own seed forced, 0x8561D8DD:

| Site | Old pool | New pool |
| --- | --- | --- |
| Sky Pillar | Ogerpon (sub-legendary) | Zygarde |
| Terra Cave | Enamorus (sub-legendary) | Calyrex |
| Marine Cave | Registeel (sub-legendary) | Ho-Oh |
| Desert Ruins | **Regirock — its own species back** | Victini |
| Island Cave | Zygarde | Zeraora |
| Ancient Tomb | Chi-Yu (sub-legendary) | Solgaleo |
| Southern Island A | Landorus (sub-legendary) | Arceus |
| Southern Island B | Cresselia (sub-legendary) | Zygarde |
| Faraway Island | Calyrex | Kyogre |
| Birth Island | Ho-Oh | Lugia |
| Navel Rock top | Glastrier (sub-legendary) | Marshadow |
| Navel Rock bottom | Kartana (Ultra Beast) | Miraidon |

That Desert Ruins row is the answer to "is the randomizer even working there": it was, and
it rolled Regirock's own species back onto it.

The two freezes, measured on the spot:

| Case | Before | After |
| --- | --- | --- |
| R while on the Mach Bike | controls locked, player cannot move | flags 0x22 → 0x24, controls free, player moves |
| League door, one legendary | stuck on the trigger tile: `preventStep 0`, `frozen 0`, `heldMovement` active 1 finished 1 | walks through into the League |
| League door, two legendaries | refused, stepped back, released | unchanged — it always worked |

| # | Test | Steps | Expected |
| --- | --- | --- | --- |
| T56.1 | **The bike swap** | On the bike, press R | It switches between Mach and Acro with its sound, and you keep riding. Press it repeatedly |
| T56.2 | R elsewhere | Press R on foot, and on the bike indoors | On foot it does nothing (or DexNav, if that is on); nothing freezes |
| T56.3 | **The League with one legendary** | Walk to the Elite Four door carrying exactly one | You walk straight through |
| T56.4 | The League with none | Same, with no legendaries | Straight through |
| T56.5 | The League with two | Same, carrying two | "Only one legendary POKéMON may be in your party", you step back, and you can walk away and use the PC in that room |
| T56.6 | Walking the tiles sideways | Cross the two tiles in front of the door left-to-right | The rule fires each time, and never holds you |
| T56.7 | **A Regi cave** | Clear a Regi cave's puzzle | A box legendary or a mythical — never a sub-legendary or an Ultra Beast, and never the cave's own Regi |
| T56.8 | Two sites | Clear a second legendary site | A different species from the first |
| T56.9 | The Sky Pillar | Reach the top | The legendary there is level 63 |
| T56.10 | Regression tests | `make check TESTS="Randolocke"` | PASS — 23 |

---

## Phase 55 — The Steven double battle: the chosen three, and the heal that never ended

### What changed

Two defects behind the Mossdeep Space Center double battle with Steven, both fixed.

**1. The three you pick are the three you send.** `AreMultiPartiesFullTeams()` answers the
question "does each side field a full six?", and the multi battle scripts ask it *from the
overworld*, before the battle has been set up — where `gBattleTypeFlags` still describes the
**previous** battle. Walk into Steven after a wild encounter and those flags carry no
`BATTLE_TYPE_TRAINER`, so the function fell through to the trainer lookups, read
`gTrainers[...][TRAINER_NONE]`, found no half-team marking and answered "full teams".
On that answer the script skips `ReducePlayerPartyToSelectedMons`, and the three Pokemon
you had just chosen — and the order you chose them in — were thrown away: the battle used
the first three in party order instead. The fix asks the battle *being set up* rather than
the one just finished: out of battle, a partnered wild battle leaves its opponents at
`TRAINER_NONE` and a partnered trainer battle names them, so the trainers are only consulted
once there is a trainer to consult. In battle, `gBattleTypeFlags` is current and is used as
before.

**2. The Pokemon Center heal with an empty party.** The healing machine places one Poke Ball
per party Pokemon, and its state only ended *after* placing a ball and counting one off — so
with zero to place the count wrapped round and it placed a ball every 25 frames until the
sprite table was full: `src/sprite.c:453: Out of sprite slots`, the crash in the screenshot.
It needs an empty party to happen, which the nuzlocke wipe rule provides:
`RANDOLOCKE_WIPE_COSTS_PARTY` boxes the whole party, and the wipe then walks you into a
Pokemon Center. `PokeballGlowEffect_PlaceBalls` now ends the state when there is nothing left
to place, and also when it has used all six coordinates — the same guard from the other end.

**Not a bug — the whiteout.** Losing a trainer battle blacks you out here as in every
Pokemon game, and Pokemon in the PC have never counted towards that — only the party does.
In a partner battle Gen 4+ rules (`B_MULTI_BATTLE_WHITEOUT`, `GEN_LATEST`) can spare you
when Steven wins the fight on his own, but only while one of the three you *left out* of the
battle is still standing; with all six of yours down it is a loss. The Randolocke wipe rule
then boxes the party, which is what walked an empty party into the healing machine. Nothing
here changed, and neither `AreMultiPartiesFullTeams()` answer above affects it: in battle the
flags are current, both Mossdeep opponents are marked `Multi Party: Half`, and the answer was
— and stays — "not full teams". Worth knowing that the first bug fed the fight your first
three instead of the three you picked, so the loss that started all this may simply not
happen again.

No save-layout change; no new game needed.

### Headless check

A throwaway autopilot build (not committed) walked a new save to Littleroot, then:

| Case | Before | After |
| --- | --- | --- |
| The script's question after a wild battle | full teams = 1 (wrong — discards your picks) | full teams = 0 |
| The script's question after a trainer battle | full teams = 0 | full teams = 0 |
| Six Pokemon, picked 6th, 4th, 5th | sent Bulbasaur, Charmander, Squirtle | sends Totodile, Chikorita, Cyndaquil |
| The heal with an empty party | never finished; sprite table full | finished in 195 frames, 16 sprites at peak |

`test/randolocke_pokecenter_heal.c` covers the second fix: with the guard removed it
reproduces `Out of sprite slots` exactly, and with it the effect ends in 186 frames using two
sprites. The first fix has no test — `AreMultiPartiesFullTeams()` has a separate `#if TESTING`
body, so the branch that changed is compiled out of the test ROM.

| # | Test | Steps | Expected |
| --- | --- | --- | --- |
| T55.1 | **The chosen three** | With six Pokemon, talk to Steven at the Space Center and pick three that are *not* the first three — in a deliberate order | The battle sends exactly those three, in that order |
| T55.2 | After a wild battle | Have a wild encounter on the way in, then start the fight | Same as T55.1 — the previous battle no longer decides it |
| T55.3 | The party after | Win or lose, then check the party | Your full six are back, in their original order |
| T55.4 | **The heal with an empty party** | Lose the fight (or any fight) so the wipe boxes the party, and let it walk you to the Pokemon Center | The machine's animation runs with no balls and ends; the nurse hands back; no crash |
| T55.5 | The heal normally | Heal with one, three and six Pokemon | One ball per Pokemon, as before |
| T55.6 | The whiteout | Lose the double battle with a full box | Still a whiteout — the box never counts. The party is boxed and you wake in a Pokemon Center: Phase 30's wipe rule, unchanged |
| T55.7 | Steven wins it alone | Let your three faint while a Pokemon you left out of the battle is healthy | No whiteout: the fight is his to finish |
| T55.8 | Regression tests | `make check TESTS="Randolocke"` | PASS — 23 |

---

## Phase 54 — A 3x3 pond, shinies at 1 in 64, and the first encounter counts

### What changed

- **Littleroot's pond is 3x3**, at x 11-13, y 15-17, built from the same rimmed pieces as
  before, with its shore at y 18 under it and a tile of ground between it and Birch's lab.
  The rest of the old 6x4 pond is Littleroot's own ground again. Every pond tile is pond
  water, so you can fish from the shore below, from either side or from above. The boy
  with the Porta Heal and Endless Candy still stands on the shore at (11,18), facing it.
  This replaces the coordinates in T17.1–T17.5.
- **`SHINY_ODDS` 512 → 1024**: 1 in 64.
- **`RANDOLOCKE_FIRST_ENCOUNTER_COUNTS`** — the first Pokémon you meet in an area is your
  one chance there, however that battle ends: caught, knocked out, run from, gone by
  Teleport, Roar, Whirlwind or its own fleeing, or a loss. Until now only a catch used the
  area, so running from an unwanted first encounter — or knocking it out — meant another
  try. The clauses still spare the area: a shiny, a legendary, or a Pokémon whose
  evolution family you already caught, so running from a dupe, or a dupe teleporting away,
  leaves you free to keep looking. It is decided once, as the battle finishes.
- The ball refusal in a used area now reads "You've already had your one encounter in
  this area!" — "You already caught a Pokémon" is no longer the only way to get there.
- `docs/NUZLOCKE.md` said a single faint was "just a faint"; with
  `RANDOLOCKE_FAINT_COSTS_MON` it has not been since Phase 33. Corrected, along with the
  area definition (a region map section, not a map) and the legendary clause, which it
  never mentioned.

### Found on the way: shiny wild battles crashed the test ROM

Every test with a shiny wild Pokémon crashed, including expansion's own "Front anims work",
and the crash reported no result at all, so the suite just came up one test short. The test
runner's blank save has no name — all zeroes, which to the text engine is spaces, not the
end — and when a shiny wild battle ends, the TV's breaking-news code copies the player's
name, looking for its end, over the heap behind it. A real save always has a name, so the
game itself was never affected. The runner's blank save now ends its name at once.
Confirmed on the previous commit too: it predates this phase.

### Headless check

A throwaway autopilot build (not committed) fought a wild Zigzagoon on Route 101 with the
rules running and picked RUN from the battle menu each time:

| Case | Ball at the menu | After running |
| --- | --- | --- |
| A new Pokémon | allowed | Route 101 used |
| The next encounter there | refused: area used | still used |
| A dupe (Linoone caught) | refused: dupe | still open |
| A shiny | allowed | still open |

### Shiny odds

| Situation | Odds per Pokémon |
| --- | --- |
| Any wild or gift Pokémon | 1 in 64 (1.56%) |
| With a lure (one reroll) | about 1 in 32 |
| With the Shiny Charm (two rerolls; debug menu only) | about 1 in 22 |

A 50% chance of seeing one takes about 44 encounters, 90% about 146.

| # | Test | Steps | Expected |
| --- | --- | --- | --- |
| T54.1 | **The pond** | Leave your house, look east of Birch's lab | A 3x3 pond with its stone rim, a tile of grass between it and the lab |
| T54.2 | **Fishing** | Old Rod from the shore at (12,18), facing up | A bite |
| T54.3 | Fishing from the side | Stand at (10,16) facing right, or (14,16) facing left | A bite |
| T54.4 | The boy | Talk to the boy at (11,18) | Porta Heal / Endless Candy as before; he faces the pond |
| T54.5 | The town | Walk from the lab door east, and north past the pond | Everything reachable |
| T54.6 | **Running uses the area** | Rules on; run from the first Pokémon on a fresh route, then meet another | A ball is refused: "You've already had your one encounter in this area!" |
| T54.7 | **Knocking it out** | Knock out the first Pokémon on a fresh route | The area is used |
| T54.8 | **Teleport** | Let a wild Abra teleport away as the first encounter | The area is used |
| T54.9 | Dupe clause | Run from a Pokémon whose family you already caught | The area is still open |
| T54.10 | Shiny clause | Run from a shiny | The area is still open |
| T54.11 | Before the Poké Balls | Run from something before Birch's five balls | Nothing is used |
| T54.12 | Shinies | Run through grass | Roughly one in 64 is shiny |
| T54.13 | Regression tests | `make check TESTS="Randolocke"`, `TESTS="Front anims work"` | PASS — 22, including the eight new first-encounter tests; and 1 |

---

## Phase 53 — The Regi caves open with the eighth badge

### What changed

**`RANDOLOCKE_REGI_CAVES_OPEN_AT_BADGE_8`** — hold the eighth badge and the three Regi caves
are open, with no visit to the Sealed Chamber. Each of Route 111 (Desert Ruins), Route 105
(Island Cave) and Route 120 (Ancient Tomb) shuts its cave as it loads unless the Sealed
Chamber's doors flag is set; with the badge, that flag is now set first. So the Sealed
Chamber, and Flash in its inner room, agree that the doors are open: there is nothing left
for either to do. Inside each cave nothing changes — its Regi is still behind its own wall,
opened by Flash anywhere in the Desert Ruins or Island Cave, or at the center of the
Ancient Tomb, as well as by the original puzzles.

A save that already holds the eighth badge gets it the next time it loads one of those
routes — leave and come back if you are standing on one. No save-layout change.

### Headless check

A throwaway autopilot build (not committed) warped below each entrance with the doors flag
cleared, so only the badge could open them:

| Case | Doors flag after the route loads | Entrance | Walking up | Flash on the inner wall |
| --- | --- | --- | --- | --- |
| Desert Ruins, seven badges | clear | rock wall | stays on Route 111 | — |
| Desert Ruins, eighth badge | set | open | into the ruins | would open it |
| Island Cave, eighth badge | set | open | into the cave | would open it |
| Ancient Tomb, eighth badge | set | open | into the tomb; walked to (8,25) | would open it |

| # | Test | Steps | Expected |
| --- | --- | --- | --- |
| T53.1 | **Without the Sealed Chamber** | With eight badges and the Sealed Chamber never solved, go to the Desert Ruins on Route 111 | The entrance is open |
| T53.2 | Island Cave | Route 105 | Open |
| T53.3 | Ancient Tomb | Route 120 | Open |
| T53.4 | **The inner walls** | Use Flash in the Desert Ruins or Island Cave, and at the center of the Ancient Tomb | Each wall opens as in Phase 44 |
| T53.5 | Before the badge | Seven badges, Sealed Chamber not solved | All three still shut; the Sealed Chamber (or Flash in it) opens them as before |
| T53.6 | Standing on the route | Get the eighth badge, fly to Route 111 | Open on arrival — it is checked each time the route loads |
| T53.7 | The Sealed Chamber after | Visit it once the caves are open | The Braille still reads, with no rumble after it; Flash there says "Can't use that here" |
| T53.8 | Regression tests | `make check TESTS="Randolocke"` | PASS — 14, including all three routes' own load scripts with and without the badge |

---

## Phase 52 — Nowhere dark, and the battle's messages on SELECT

### What changed

- **`RANDOLOCKE_NO_DARK_AREAS`** — Granite Cave B1F and B2F, Victory Road B1F and B2F and
  Dewford Gym are fully lit from the moment you walk in. The darkness level is still
  stored — a dark cave's default, and Dewford Gym's step per trainer beaten — but
  everything reads it as 0, which also lights a save made inside a dark cave the moment it
  loads. Dewford Gym's lighting animation after each trainer now lets its script carry on
  at once: the animation is what would have restarted it. The light switch still clicks.
- Flash no longer offers to light a cave: there is nothing left to light, and using it
  would have shrunk the light to Flash's own radius. It still opens the Regi chambers.
- **`RANDOLOCKE_BATTLE_LOG`** — tap SELECT at the battle menu (FIGHT / BAG / POKéMON /
  RUN) to replay the battle's messages in the text box: "The start of the battle:" and
  everything said before the first turn, then "The last turn:" and everything since the
  last turn's moves began, including a send-out after a faint. An ability pop-up gets a
  line of its own, "The opposing Kyogre's / Drizzle!", because under Gen 5+ text the
  message after it ("It started to rain!") does not say whose ability it was. A goes to the
  next message; B or SELECT closes the replay.
- The debug ROM's battle debug menu was on SELECT; it is now a one-second hold of SELECT
  (`RANDOLOCKE_SELECT_HOLD_FRAMES`). The release ROM has no debug menu, so a press replays
  at once there. Link and recorded battles are left alone.
- The log keeps two turns at most — the start of the battle and the last turn — in 2 KB of
  EWRAM. If a turn says more than fits, the replay ends with "…and more than the log could
  hold."

### Headless checks

A throwaway autopilot build (not committed) played New Game through the quick start and on.

Dark areas, warped into each:

| Map | Stored darkness | Read as | Darkness effect |
| --- | --- | --- | --- |
| Granite Cave B1F | 7 | 0 | off |
| Victory Road B1F | 7 | 0 | off |
| Dewford Gym (no trainer beaten) | 7 | 0 | off |
| Dewford Gym, after its first trainer's light step | 6 | 0 | off |

The light step itself — the script that runs after beating a Dewford Gym trainer — finished
in 3 frames with the player's controls free.

The battle log, Mudkip against a wild Politoed with Drizzle:

| Step | Result |
| --- | --- |
| SELECT at the first battle menu | Opens in 3 frames: "The start of the battle:", "You encountered a wild Politoed!", "Go! Mudkip!", "The wild Politoed's / Drizzle!", "It started to rain!" |
| A through it | Five presses, then the battle menu, working: FIGHT opens the moves |
| SELECT, then B | Closed in 1 frame |
| Growl, then SELECT at the second menu | The five lines, then "The last turn:", "The wild Politoed used Splash!", "But nothing happened!", "Mudkip used Growl!", "The wild Politoed's Attack fell!", "Rain continues to fall." |
| Hold SELECT one second (debug ROM) | The battle debug menu opens |

| # | Test | Steps | Expected |
| --- | --- | --- | --- |
| T52.1 | **Granite Cave** | Go down to B1F and B2F | Fully lit, no Flash needed |
| T52.2 | Victory Road | B1F and B2F | Fully lit |
| T52.3 | **Dewford Gym** | Walk in; beat a trainer | Lit from the door. After each trainer the switch clicks and the battle hands straight back |
| T52.4 | Flash in a cave | Use Flash in Granite Cave | "Can't use that here" — nothing to light |
| T52.5 | Flash at the Regis | Use Flash in the Sealed Chamber | Still opens it, as in Phase 44 |
| T52.6 | An old save in the dark | Load a save made inside Granite Cave B1F on an older build | Lit on load |
| T52.7 | **The start of a battle** | Any battle; at the first menu tap SELECT | The send-outs and anything that fired on entry, each in the text box |
| T52.8 | **An ability named** | Face something with Drizzle, Drought, Intimidate or Sand Stream | Its own line — "The opposing X's / Drizzle!" — before the effect |
| T52.9 | The last turn | Tap SELECT on turn 2 or later | The start of the battle, then "The last turn:" and that turn's messages |
| T52.10 | After a faint | Knock out a trainer's Pokémon; tap SELECT at the next menu | The send-out and its entry ability are under "The last turn:" |
| T52.11 | Closing it | B or SELECT mid-replay | Straight back to the menu, which still works |
| T52.12 | Debug menu | Debug ROM: hold SELECT at the battle menu for a second | The battle debug menu |
| T52.13 | Double battles | Tap SELECT for either Pokémon | The same replay |
| T52.14 | Regression tests | `make check TESTS="Randolocke"`, and `TESTS=` `Intimidate`, `Illusion`, `Drizzle`, `Trace`, `Neutralizing Gas`, `Drought`, `Air Balloon` | PASS — 13, 16, 11, 5, 10, 19, 3, 12 |

---

## Phase 51 — Straight to Birch, the Running Shoes at the truck, shinies at 1 in 128

### What changed

- **`RANDOLOCKE_QUICK_START`** — the first morning loses its errands. Mom meets you at the
  truck, says she has set the clock, hands over the Running Shoes and sends you to
  Professor Birch, then goes back inside. No trip into the house, no bedroom clock, no Dad
  on TV, no visit next door. The clock is set to the cartridge's real-time clock — in an
  emulator that is your computer's clock — with the same call the wall clock makes.
  Everything the skipped scenes would have set is set instead: intro state 7, the rival's
  mother and the rival counted as met (town state 1, rival state 3), the rival waiting in
  their bedroom as vanilla leaves them, the movers gone. The twin moves to her state-1 spot
  at (10,1), so walking north still gets "go see what's happening".
- The Running Shoes now come early, so after the Pokédex the lab moves the town state
  straight past the scene where Mom would wait outside to give them again (3 → 4).
- The rival is MAY or BRENDAN, as always in Emerald — the game never asks for a rival name.
- **`RANDOLOCKE_SKIP_WALLY_TUTORIAL`** — the first time you enter Petalburg, the state the
  catching tutorial leaves behind is set instead: the gym at "come back with four badges"
  (gym state 2, as vanilla leaves it), the city past the tutorial, Wally, the gym's Wally
  and Wally's mother hidden, the rival gone from the lab, Birch's day reset. The gym boy
  stays where he stands and never walks you to the gym. Norman is optional until you want
  the badge. Wally's later appearances do not read any of this.
- **`SHINY_ODDS` 256 → 512** — 1 in 128.

Only a **new game** sees the quick start: a save already past the truck never comes back
to it. The Wally skip applies the first time any save enters Petalburg with the city still
at state 0. No save-layout change.

### Headless playthrough

A throwaway autopilot build (not committed) played New Game from the title screen, pressing
A every eighth frame and walking the shortest path, and printed the story state.

| | Boy | Girl |
| --- | --- | --- |
| New game → out of the truck | 935 frames (vanilla truck ride) | same |
| Quick start (Mom's scene) | 521 frames, 8.7 s | same |
| State afterwards | intro 7, town 1, rival 3, houses(Brendan) 2; shoes, dash, clock, met rival mom, TV set; Mom gone; May in her bedroom | the mirror image: houses(May) 2, Brendan in his bedroom |
| Player / twin | (4,10) facing east / (10,1) facing up, face-up movement | (13,10) / same |
| Walk north | "Go see what's happening" at (11,1), town state 1 → 2 | same |
| Route 101 | Rescue scene, player left at (11,15) | same |
| **New game → starter bag open** | **2,248 frames, 37 s** | 2,208 frames, 36 s |

A third run stepped out of the truck and warped to Petalburg: city state 3, gym state 2,
Birch state 0, Wally, the gym's Wally, Wally's mother and the lab rival hidden, the gym boy
at his usual (12,15). Walking from x=27 to x=6 along row 13 — across the gym boy's trigger
column — started no script. None of the three runs logged an assert, a skipped free or an
illegal opcode.

### Shiny odds

| Situation | Odds per Pokémon |
| --- | --- |
| Any wild or gift Pokémon | 1 in 128 (0.78%) |
| With a lure (one reroll) | about 1 in 64 |
| With the Shiny Charm (two rerolls; debug menu only) | about 1 in 43 |

A 50% chance of seeing one takes about 89 encounters, 90% about 294.

| # | Test | Steps | Expected |
| --- | --- | --- | --- |
| T51.1 | **Quick start** | New game, step off the truck | Mom comes out, welcomes you, says the clock is set, gives the Running Shoes and sends you to Birch, then goes back inside. You are left at the truck, free to move |
| T51.2 | Running Shoes | Walk | Running works straight away |
| T51.3 | **The clock is set** | Go up to your bedroom and read the clock | It shows your computer's time. No "set the clock" prompt anywhere |
| T51.4 | Your house | Go in | Mom inside, no movers, no boxes; nothing triggers |
| T51.5 | The rival's house | Visit next door | The rival's mother says they are busy; the rival is in their bedroom, as after meeting in vanilla |
| T51.6 | **The twin** | Walk to the north exit | She asks you to go and see what's happening |
| T51.7 | **Birch** | Walk onto Route 101 | The rescue plays; the bag offers three random starters — 37 s after New Game in the headless run, a little longer reading the text |
| T51.8 | Re-roll | Soft reset, New Game again | A new Trainer ID, so three different starters |
| T51.9 | After the rescue | Back to Littleroot | The rival has gone from their bedroom to Route 103 |
| T51.10 | **No second shoe scene** | Get the Pokédex, leave the lab | Mom is not waiting outside |
| T51.11 | The girl's version | T51.1–T51.10 as a girl | The same, with May's house as yours and Brendan as the rival |
| T51.12 | **No Wally tutorial** | Enter Petalburg from Route 102 | No Wally, no gym boy walking you back; the west exit to Route 104 is open |
| T51.13 | Norman | Enter the gym, talk to him | "Come back with four badges" |
| T51.14 | Norman's badge count | Earn badges | He opens the gym at four, as vanilla |
| T51.15 | Wally later | Mauville | Wally and his uncle appear and battle as normal |
| T51.16 | **Shinies** | Run through grass | Roughly one in 128 encounters is shiny |
| T51.17 | Regression tests | `make check TESTS="Randolocke"`, `TESTS="Shininess"`, `TESTS="Capture"`, `TESTS="CreateNPCTrainerPartyForTrainer"` | PASS — 8, 2, 8 and 4 |

The trainer-party test had been failing since Phase 46: it checks the party file's own IVs,
EVs and natures, which the Phase 46 trainer rules replace by design. Those checks now run
only with the matching `RZ_TRAINER_*` setting off; `test/randolocke_trainers.c` covers the
rules themselves.

---

## Phase 50 — Wild encounters are never weaker, and every route has a lottery ticket

### How the randomization works

Every wild slot is rolled on its own, seeded by map, terrain, slot number and Trainer ID,
so a route vanilla fills with three species can show twelve. The pick is uniform over every
species whose BST is in a window around the slot's vanilla species, and the slot keeps its
vanilla odds (20/20/10/10/10/10/5/5/4/4/1/1% on land).

The stock window is ±10%, so half of every roll was weaker than what vanilla put there, and
on the opening routes — vanilla's species there are 195 to 240 — that half was cocoons and
babies: Kakuna, Silcoon, Cascoon, Spewpa on three routes, Burmy, and Cosmog at 20% on
Route 104.

### What changed

- **RZ_WILD_BST_FLOOR/CEILING_PERCENT, 100 to 125** — for wild encounters only, a
  replacement is never weaker than vanilla's species and at most a quarter stronger.
  Trainers keep the stock window.
- **RZ_WILD_LOTTERY** — the two 1% land slots skip the window and roll the first stage of
  one of the ten 600-BST pseudo-legendary lines: Dratini, Larvitar, Bagon, Beldum, Gible,
  Deino, Goomy, Jangmo-o, Dreepy, Frigibax.

A form can carry a very different BST from the species that was picked, and the stock form
handling does not know about the window: across every wild slot in the game three picks
became forms of 575 and 700. A form is now kept only if it is inside the window too.

### Route 101 on the playtest seed

| Slot odds | Vanilla (BST) | Window | Before | Now |
| --- | --- | --- | --- | --- |
| 20% | Wurmple (195) | 195–243 | Weedle | **Feebas** |
| 20% | Poochyena (220) | 220–275 | Zigzagoon | **Yungoos** |
| 10% | Wurmple | | Kakuna | **Igglybuff** |
| 10% | Wurmple | | Azurill | **Ralts** |
| 10% | Poochyena | | Wimpod | **Slugma** |
| 10% | Poochyena | | Silcoon | **Makuhita** |
| 5% | Wurmple | | Wooper | **Nymble** |
| 5% | Poochyena | | Pawmi | **Wynaut** |
| 4% | Zigzagoon (240) | 240–300 | Starly | **Varoom** |
| 4% | Zigzagoon | | Togepi | **Morelull** |
| 1% | — | lottery | Gossifleur | **Deino** |
| 1% | — | lottery | Wiglett | **Frigibax** |

Weighted average BST of an encounter: 217 before, 233 now. No cocoons.

The lottery is fair over the whole game: across all 194 lottery slots each line lands 14 to
26 times against 19.4 expected, a chi-square of 4.8 on 9 degrees of freedom. The early
routes happening to draw Frigibax five times out of twelve is a small sample doing what
small samples do.

Under nuzlocke rules the first encounter in an area is the one that counts, so a 1% slot
pays out about 2% of the time, route by route.

| # | Test | Steps | Expected |
| --- | --- | --- | --- |
| T50.1 | **Route 101** | Walk in the grass on the playtest save | The table above |
| T50.2 | No cocoons early | Routes 101–104, Petalburg Woods | None of Kakuna, Silcoon, Cascoon, Metapod |
| T50.3 | A lottery hit | Encounter until a 1% slot comes up (debug menu helps) | One of the ten pseudo-legendary lines |
| T50.4 | Trainers unchanged | Route trainers | Same species as before this phase |
| T50.5 | **Regression test** | `make check TESTS="Randolocke"` | PASS — every wild slot in the game inside its window, every lottery slot a prize, on two seeds |

---

## Phase 49 — One legendary into the League; one bike that is both

### The League's one-legendary rule

RANDOLOCKE_ELITE_FOUR_LEGENDARY_LIMIT refuses the Elite Four to a party carrying more than
RANDOLOCKE_ELITE_FOUR_MAX_LEGENDARIES (1) legendaries — the same 136 species the legendary
clause and the legendary catch rate count: restricted legendaries, sub-legendaries,
mythicals and Ultra Beasts. Eggs do not count.

It is checked on the two tiles in front of the door, (9,2) and (10,2) of the League 1F,
not by the guards. The guards step aside once and stay aside (`copyobjectxytoperm`,
FLAG_ENTERED_ELITE_FOUR), so a check in their script would see your first attempt and none
after — not the one after a loss, and not a rematch. Those two tiles are where the guards
stood, so every route to the door crosses one. A refused player is stepped back one tile;
the Pokémon Center's PC is in the same room.

### One bike

RANDOLOCKE_DUAL_BIKE, ported from pokeemerald_rando_enh's "bike combined":

- Rydel gives one **BIKE** instead of asking you to choose, and on later visits reminds you
  how it works instead of offering a trade.
- **R while riding** switches Mach ↔ Acro in place — a hop sound going to Acro, the bell
  going to Mach.

The switch calls SetPlayerAvatarTransitionFlags, the same transition as getting on, so the
sprite, the avatar state and the bike's momentum reset together. The fork swapped the flags
by hand before calling it; the transition already does that. R is otherwise only DexNav's,
which is off in this build.

Both bike items are renamed BIKE, so a save that already holds the Acro Bike keeps it and
it behaves identically — it starts in Acro and R switches it.

| # | Test | Steps | Expected |
| --- | --- | --- | --- |
| T49.1 | **One legendary is fine** | One legendary in the party, walk to the Elite Four's door | Through |
| T49.2 | **Two are refused** | Two in the party | "Hold on!…", stepped back one tile |
| T49.3 | Every attempt is checked | Lose to the Elite Four, try again with two | Refused again, although the guards are aside |
| T49.4 | Fixable on the spot | Box one at the League PC, try again | Through |
| T49.5 | Ultra Beasts count | A legendary and an Ultra Beast | Refused |
| T49.6 | **Rydel gives one BIKE** | New save, Mauville | One BIKE, the R tip, no Mach/Acro menu |
| T49.7 | **R switches** | Ride, press R | Acro with a hop; R again, Mach with the bell |
| T49.8 | Mach behaviour | Mach mode on a muddy slope | Climbs it |
| T49.9 | Acro behaviour | Acro mode on rails / hold B to hop | Rides them |
| T49.10 | R on foot | Press R walking | Nothing happens |
| T49.11 | An existing Acro Bike | Save that already had one | Named BIKE; starts in Acro, R switches |
| T49.12 | Rydel again | Talk to him after | The R tip, no trade offer |
| T49.13 | Regression tests | `make check TESTS="Randolocke"` | PASS — party checks for 0, 1 and 2 legendaries, and a UB |

---

## Phase 48 — A Combee worth catching

Combee evolves at level 21 into Vespiquen, but only if it is female, and vanilla makes it
female 12.5% of the time. Under nuzlocke rules the route's one encounter is the only Combee
a run will ever see, so seven runs in eight are handed a Pokémon whose evolution does not
exist for them — 30/30/42/30/42/70 for the rest of the game.

Its gender ratio is now `PERCENT_FEMALE(95)`, which the macro resolves to 242 of 256, so
**94.5%** in practice. Species-wide rather than wild-only: a trainer's Combee and a hatched
one follow the same odds, which keeps one number to reason about.

| # | Test | Steps | Expected |
| --- | --- | --- | --- |
| T48.1 | **A wild Combee is female** | Encounter several (debug menu, or the species' routes) | Nearly all female |
| T48.2 | It evolves | Level a female one to 21 | Becomes Vespiquen |
| T48.3 | Males still exist | Keep looking | About 1 in 18 |
| T48.4 | Nothing else moved | Any other species' gender spread | Unchanged |

---

## Phase 47 — The starter moves drop, and the tutors keep teaching

### Tackle and its five friends

Every learnset in this hack is rolled from the move bands, so a move appears only if the
roll puts it there. The community list places the level-1 starter moves at Niche, which is
a fair read of a game that hands them out for free — but here Niche is the second-heaviest
band, 383 moves at 0.105% each, and a 40 BP attack with nothing attached is not worth one
of a Pokémon's 21 slots.

Tackle, Pound, Scratch, Ember, Bubble and Water Gun are now in Pokémon Homeless, the bottom
band: **0.014% each, 7.5 times rarer**. They also stop being TMs — `sTmMoveTiers` draws from
the top four bands only, by design, because a TM is permanent under I_REUSABLE_TMS.

Splash went down with them, from Bad rather than Niche: a move whose entire effect is the
message saying it had none should not be taking a learnset slot at twice the rate of the
bottom band.

Done in MOVES_PUSHDOWN, beside the self-KO and OHKO moves, so the community sheet stays as
voted and the override is one list. Peck and Vine Whip sit at Bad and were left there.
`python3 tools/randolocke/gen_move_tiers.py` regenerates after any edit.

### The town tutors teach as often as you like

The ten tutors teach once each in vanilla: a flag is set when you accept, the offer never
comes again, and the game warns you before you spend it. Ten moves for a whole run, and in
a randomized run you do not choose which ten.

RANDOLOCKE_REPEATABLE_MOVE_TUTORS drops all three halves of that gate in the `move_tutor`
macro — the check that sends you away, the "can only be learned once" warning, and the
setflag that remembers. Each tutor still teaches its own randomized move, the same one
every time; what changes is how many of your Pokémon can have it.

A save that already spent some tutors is fine: the flags it set are simply no longer read.
The Battle Frontier's two tutors are a separate script and still charge BP.

Verified in the built ROM rather than by eye: the Slateport tutor's compiled script is
`lock`, `faceplayer`, `setvar` — the `checkflag` that used to follow `faceplayer` is gone.

| # | Test | Steps | Expected |
| --- | --- | --- | --- |
| T47.1 | **A tutor teaches twice** | Any town tutor → teach → talk again → teach another Pokémon | Offers again, same move, no "only once" warning |
| T47.2 | An already-spent tutor | A tutor used before this build | Offers again |
| T47.3 | Declining still works | Say no | Declined message, nothing taught, offers again later |
| T47.4 | The Frontier is unchanged | Battle Frontier tutors | Still charge BP, still once per move |
| T47.5 | **Starter moves are rare** | Roll a few dozen learnsets (new save or the debug menu) | Tackle and friends turn up about a seventh as often |
| T47.6 | No starter-move TMs | Check the TM list on a new save | None of the seven appear as a TM |
| T47.8 | **Splash** | Same | As rare as the rest of the bottom band |
| T47.7 | Tier data regenerates clean | `python3 tools/randolocke/validate_tiers.py --moves` | All names resolved, 0 unresolved |

---

## Phase 46 — Kaizo-style trainer pressure

Emerald Kaizo's trainers run perfect IVs, 252 EVs, optimal natures and held items, with an
AI that predicts and exploits, and the bag shut in battle. Measured against that, this
project already had an AI beyond it — bosses are omniscient, the Champion predicts moves
and switches — and hard caps that put every boss at the player's own ceiling. Five things
were missing.

### IVs — a boss's are perfect, everyone else rolls

Vanilla gives a trainer one flat IV value for every stat of every Pokémon it owns, scaled
by how important the trainer is. Measured over trainers.party:

| | Pokémon | IVs |
| --- | --- | --- |
| The 55 `Boss: Yes` trainers | 255 | 70% at 31, the rest between 6 and 30 |
| Everyone else | 1570 | **40% at 0**, most of the rest 1 to 12, 1% at 31 |

A gym leader should not be fighting you with a 6 IV Pokémon, so a boss's are now perfect
across all six stats. Everyone else rolls each stat separately between 0 and 31 rather than
carrying one number six times — an average of 15.5 a stat against the 0 to 3 most of them
have now, which makes ordinary trainers the biggest gainer in this phase. Rolled from the
trainer and the slot, so a trainer is the same fight every time you meet them.

### Natures — every trainer Pokémon was Hardy

Not one entry in trainers.party carries a `Nature:` line, so all 1825 fought on the neutral
default: nothing raised, nothing lowered, while the player's Pokémon have one. They now get
the nature a player would pick, on the same reading of base stats the EVs use — the fast
ones buy Speed with the attacking stat they do not use (Jolly, Timid), the slow ones buy
power with it (Adamant, Modest). Nothing a Pokémon uses is ever what drops.

### EVs — 252 and 252, the legal 510

The old spread put one value on four stats, which at eight badges came to 512: marginally
over the 510 the player is held to, and spread too thin to be felt. It is now two stats at
the badge value, `{ 24, 48, 72, 100, 140, 180, 220, 252, 252 }`:

- an attacking stat the species can actually use, read off its base stats, since the
  species is randomized;
- then **Speed** if base Speed is at least 67 — the measured median of every species — and
  **HP** if it is not, because 252 Speed on a Shuckle is 252 EVs in the bin;
- the 6 the two 252s leave over go to its better defence.

The first three rows are the old totals, so the early gyms are where they were; from the
fourth badge it climbs, ending at a legal 510 rather than 512 spread four ways.

### Held items

142 of 1825 carried one, mostly in-battle restores the AI throws rather than something
held. A Pokémon with no item of its own now gets one — bosses always, everyone else 35% —
from eleven items that suit any species (Leftovers, Sitrus, Lum, Focus Band, Focus Sash,
Bright Powder, Quick Claw, Scope Lens, Expert Belt, Life Orb, Shell Bell) plus the booster
for the category it attacks from. Items written into trainers.party are left alone. The
roll is seeded from the trainer and the slot, so a trainer holds the same thing every time.

No Choice items: they lock the holder into one move, and an AI that mishandles that is
easier to beat, not harder.

### No bag against a trainer

RANDOLOCKE_NO_BAG_VS_TRAINERS closes the bag in trainer battles — no Potions, no Revives,
held items only. Wild battles are deliberately untouched: the same check gates Poké Balls
(item_use.c), so closing it there would mean never catching anything again. It reads the
config rather than B_VAR_NO_BAG_USE, which a new game clears, so it applies to a save
already in progress.

Boss party sizes are left alone, at Randolocke v1.1's teams.

| # | Test | Steps | Expected |
| --- | --- | --- | --- |
| T46.1 | **No bag vs a trainer** | Any trainer battle → BAG | Refused; held items still work |
| T46.2 | Bag in a wild battle | Wild encounter → BAG | Opens as before |
| T46.3 | **Poké Balls still work** | Throw a ball at a wild Pokémon | Catches normally |
| T46.4 | Trainers hit harder | Fight a gym leader | Noticeably faster and stronger than before |
| T46.5 | Boss items | Watch a boss's Pokémon | Leftovers recovery, a Berry eaten, a Focus Band survival |
| T46.6 | Same every time | Lose to a boss, fight again | Same items on the same Pokémon |
| T46.7 | Early game is not brutal | Roxanne | Close to before: same EV total at one badge |
| T46.8 | **Boss IVs** | Fight a gym leader | Stats noticeably higher than the same species elsewhere |
| T46.9 | Ordinary trainer IVs | Fight the same route trainer twice | Same Pokémon both times, not six identical IVs |
| T46.10 | Regression tests | `make check TESTS="Randolocke"` | PASS — EV total ≤ 510, exactly two stats capped, a nature that raises a stat the Pokémon uses, an item on every boss Pokémon, perfect boss IVs and rolled ordinary ones |

---

## Phase 45 — One catch rate for legendaries

105 of the 136 species the legendary clause covers — restricted legendaries, sub-legendaries,
mythicals and Ultra Beasts — sit at a base catch rate of 3, the floor. After
RANDOLOCKE_CATCH_RATE_PERCENT that is 4, against 67 for the commonest wild Pokémon and 112
for the median one: 17 to 28 times harder, about ninety Ultra Balls at a quarter health.
A randomized run that drops a legendary on a route and then cannot keep it is worse than
one that never drops it.

RANDOLOCKE_LEGENDARY_CATCH_RATE puts all of them on one rate, 45, in place of their own.
It is the final rate, so it does not move when the percentage does.

| | Effective rate | Per Ultra Ball at 25% HP | Balls for ~90% |
| --- | --- | --- | --- |
| Legendary, before | 4 | 2.6% | 89 |
| **Legendary, now** | **45** | **33.7%** | **6** |
| Commonest wild Pokémon (base 45) | 67 | 50.3% | 4 |
| Median wild Pokémon (base 75) | 112 | 78.5% | 2 |

So a legendary stays the hardest thing on the route — 1.5× the commonest wild Pokémon, 2.5×
the median — without being a different game.

### Why a flat rate and not a multiplier

The 136 do not start level. Twenty-three are already at 30, 45 or 255: Mew, Celebi, the
Ultra Beasts, Phione, Eternatus, Terapagos. Ten times their rate lands past the 255 cap,
which is a guaranteed catch with any ball at full health. A flat rate keeps every legendary
worth the same number of balls.

It does mean those twenty-three are now *harder* than they were — Mew 67 → 45, Eternatus
255 → 45. `RANDOLOCKE_LEGENDARY_CATCH_RATE_IS_FLOOR TRUE` only ever raises a rate, leaving
those twenty-three exactly as they were; it is FALSE.

| # | Test | Steps | Expected |
| --- | --- | --- | --- |
| T45.1 | **A legendary is catchable** | Wild legendary, weaken to ~25%, throw Ultra Balls | Caught in a handful, not ninety |
| T45.2 | Every legendary alike | Repeat on a different one (a mythical or an Ultra Beast) | Same difficulty |
| T45.3 | Ordinary Pokémon untouched | Catch anything non-legendary | As before |
| T45.4 | Master Ball | On a legendary | Still always catches |
| T45.5 | The clause still holds | Meet a wild legendary in a used-up area (Phase 39) | Still catchable, still does not consume the area |
| T45.6 | Safari | Catch in the Safari Zone | Unaffected — that path uses its own factor |
| T45.7 | Regression tests | `make check TESTS="Randolocke"` and `TESTS="Capture"` | PASS |

The battle test throws a real ball: Mewtwo (base 3) and Mew (base 45) both record odds of
15, a Beldum (base 3, not legendary) records 1, and a Chansey — base 30, which is 45 after
the percentage — records 15 too, which is what 45 is worth.

The eight upstream Capture tests were failing before this phase, on this fork's own
RANDOLOCKE_CATCH_RATE_PERCENT rather than on any bug: they hardcoded vanilla rates. They
now derive their expectations from the effective rate and pass.

---

## Phase 44 — Flash's Regi shortcuts hand the player back

Reported from the Sealed Chamber: Flash opened the door, and then the game stopped
responding. Not an allocator fault, and nothing in the log — a plain lock-up.

### What happened

Flash's stand-ins for the Braille puzzles (Phase 17's RANDOLOCKE_FLASH_OPENS_REGI_CAVES)
run as `gPostMenuFieldCallback`: straight off the party menu, after a fade, with the
player's field controls locked and object events frozen. Whatever the callback does, it
has to hand the player back. The vanilla puzzle effects do —
`DoBrailleRegirockEffect` and `DoBrailleRegisteelEffect` both end with
`UnlockPlayerFieldControls()` and `UnfreezeObjectEvents()`, and the Dig route ends in
`EventScript_DigSealedChamber`, whose `releaseall` does the same. Three of our four did
not:

| Room | Callback | What went wrong |
| --- | --- | --- |
| Desert Ruins (Regirock) | `SetUpPuzzleEffectRegirock` | Nothing — it goes through vanilla's field-effect chain, which ends in `DoBrailleRegirockEffect` |
| Sealed Chamber outer | `DoBrailleDigEffect` | Vanilla only ever calls it from a script that releases afterwards. Called directly, the door opened and the player stayed locked |
| Island Cave (Regice) | `RandolockeOpenRegiceWall` | Opened the wall, never unlocked |
| Sealed Chamber inner | `RandolockeOpenRegiDoors` | Ended in `DoSealedChamberShakingEffect_Short`, whose task finishes with `ScriptContext_Enable()` — which *locks* the player (script.c) and marks a script running that does not exist. Now runs a script of its own |

The outer room is the reported one: the door is the metatile swap in `DoBrailleDigEffect`.

### The fix

`RandolockeOpenRegiceWall` and the new `RandolockeOpenSealedChamberDoor` end with
`UnlockPlayerFieldControls()` and `UnfreezeObjectEvents()`, as vanilla's effects do.

The three Regi caves go further: `RandolockeOpenRegiDoors` now runs
`RandolockeEventScript_FlashOpensRegiDoors`, which is the Braille route's own sequence
minus the Relicanth and Wailord check — the long rumble, three shakes each with a door
sound, and "A door opened far away!" Its `releaseall` is what hands the player back, and
its `setflag` is what opens the caves. Ordinary Flash reaches its script the same way,
from `FldEff_UseFlash`. Before this the route gave a two-frame shake, no sound and no
message, so there was no way to tell it had worked.

Proven both ways. `test/randolocke_regi_flash.c` locks the controls, runs each callback and
checks the player is free afterwards, and for the inner room that the hand-off to the
script happened (the script itself cannot run in a test: its message box waits on the
player). With the unlocks removed it fails on the Sealed Chamber's door — the reported
bug — and passes with them in.

| # | Test | Steps | Expected |
| --- | --- | --- | --- |
| T44.1 | **The reported case** | Sealed Chamber outer room, Flash from the party menu | Door opens, and you can walk |
| T44.2 | Through the door | Walk into the inner room | Normal |
| T44.3 | **The three caves** | Sealed Chamber inner room, Flash | Music fades, the room rumbles, three door sounds, "A door opened far away!", then you can walk |
| T44.4 | Regice | Island Cave, Flash | Wall opens, you can walk |
| T44.5 | Regirock | Desert Ruins, Flash | Wall opens, you can walk (this one always worked) |
| T44.6 | Registeel | Ancient Tomb, Flash on the Braille tile | Vanilla behaviour, unchanged |
| T44.7 | The Braille puzzles still work | Solve one the vanilla way instead | Opens as before |
| T44.8 | Flash still lights caves | Any dark cave | Normal Flash |
| T44.9 | The script route still works | Dig in the Sealed Chamber outer room | Door opens, player released — the shared effect is unchanged for scripts |
| T44.10 | Regression test | `make check TESTS="Randolocke"` | PASS |

---

## Phase 43 — A bad free no longer freezes the game

Reported as random freezes with the log full of

```
[ERROR] GBA Debug:  ASSERTION FAILED  FILE=[src/malloc.c] LINE=[98]  EXP=[block->allocated == TRUE]
[WARN] GBA:         Illegal opcode: 0000efff
```

Line 98 is `Free()` finding that the block it was handed is already free — a double free,
or a stale pointer to a block someone else has since freed.

### Why a bad free froze the game

Only the debug ROM (`make`) has these asserts; `make release` compiles them out. A failed
`AGB_ASSERT` prints its line and then executes `0xEFFF`, a break opcode meant to stop the
game for a debugger. Without one, mGBA's BIOS returns from it **two bytes early**, into the
second half of the `bl MgbaPrintf` just before it, with a stale link register. The CPU
jumps a few kilobytes into unrelated code. The first `ASSERTION FAILED` line is the real
event; the repeats and the freeze are fallout.

- **RANDOLOCKE_DEBUG_ASSERTS_RESUME** — a failed `AGB_ASSERT` in the debug ROM is logged
  exactly as before, then play carries on. That is what the release ROM already did, minus
  the log. Test builds keep the break.
- **RANDOLOCKE_SKIP_BAD_FREES** — `Free()` refuses a block that is already free, or a
  pointer without the allocator's magic number, and skips it. Skipping is the safe answer
  to both: a free block is already accounted for, and a header without the magic number
  cannot be trusted to walk. Both ROMs skip; only the debug ROM prints.

### Reading the new log line

```
Free(0x2014814) skipped: already free. Called from 0x8176157, allocated at src/foo.c:133
```

- `allocated at` — the file and line whose `Alloc` made the block. The header still records
  it after the first `Free()`.
- `Called from` — the function that made the bad call. Look it up against the ELF **of the
  same build**:

  ```
  arm-none-eabi-addr2line -f -e pokeemerald.elf 0x8176157
  ```

Verified with a throwaway ROM that double-freed on purpose at boot. It printed both lines,
the failed assert logged, and the game kept running.

### A real overflow found on the way: the move relearner

With P_ENABLE_ALL_LEVEL_UP_MOVES and P_PRE_EVO_MOVES the relearner's level-up list is every
move of every stage of the family, and the randomizer gives each stage its own 21-move
learnset. A three-stage family can list 63 moves; the relearner held 60 and never checked.
`test/randolocke_relearner.c` measured 8–12 families over 60 per seed, a longest list of
63. The overflow ran off `movesToLearn` and `menuItems` onto the relearner's own task IDs
and counters. It only triggers on a Pokémon that knows moves outside its learnset, which
universal TM compatibility makes common.

- `MAX_RELEARNER_MOVES` 60 → 64.
- All four list builders (level-up, egg, TM, tutor) stop at the capacity, whatever the data.

The playtest save that reported the freeze has none of the affected species, so this is a
second bug, not the cause of that report.

### Two small fixes

- The relearner's EV line read `gParties[0][gSpecialVar_0x8004]` — the wrong Pokémon when
  the relearner was opened from the PC. It now asks `GetSelectedBoxMonFromPcOrParty()`,
  the same as the relearner itself.
- The friendship window started at tile 902, inside the move-select stats overlay (822–921,
  enlarged to 10×10 in Phase 40). Now at 922. They never showed together, so nothing
  visible changed.

| # | Test | Steps | Expected |
| --- | --- | --- | --- |
| T43.1 | **Regression test** | `make check TESTS="Randolocke"` | PASS, three seeds |
| T43.2 | Relearner on a long family | Relearner on a fully evolved three-stage Pokémon that knows TM moves | List opens, scrolls to the end, CANCEL works, nothing corrupted |
| T43.3 | Relearner from the PC | Open the relearner on a boxed Pokémon | EV line shows *that* Pokémon's Attack and Sp. Atk EVs |
| T43.4 | Friendship readout | Skills page, then a move-select screen, then the skills page again | Friendship number intact |
| T43.5 | **If it happens again** | Copy the `Free(...) skipped` line from the log | Game keeps running; the line names the caller and the allocation site |
| T43.6 | Release ROM unchanged | Play normally on `pokeemerald-release.gba` | No difference in behaviour |

---

## Phase 42 — Easy fishing

Ported from Modern Emerald's EASIER FISHING option. Once something bites, the rod reels
itself in: "Oh! A bite!" holds for RANDOLOCKE_EASY_FISHING_REEL_DELAY frames (24, about
four tenths of a second) and then the Pokémon is on the hook. Pressing A during that
window reels in immediately, so nothing got slower for a player who was going to press it
anyway.

Three ways to lose a cast are gone with it:

- **The reaction window.** Fishing_WaitForA used to send a slow thumb to FISHING_GOT_AWAY
  after 30–36 frames. "It got away!" can no longer happen.
- **The extra rounds.** Going straight to the hook skips FISHING_CHECK_MORE_DOTS, which
  could send a Super Rod back through up to five more rounds of dots.
- **The stray A press.** An A press during the dots used to cancel the cast outright.
  DoesFishingMinigameAllowCancel now says no, so it does nothing.

What did *not* change is whether anything bites. That is still the roll in
Fishing_CheckForBite — I_FISHING_BITE_ODDS, 25% Old / 50% Good / 75% Super — so "Not even
a nibble..." is still the usual answer to a bad cast, and fishing is still a way to burn
an area's nuzlocke encounter on nothing.

| # | Test | Steps | Expected |
| --- | --- | --- | --- |
| T42.1 | **A bite is always landed** | Fish until "Oh! A bite!", then touch nothing | "Pokémon on the hook!" and a battle |
| T42.2 | It got away is gone | Repeat T42.1 ten times, never pressing A | Never "It got away!" |
| T42.3 | A still reels in early | Press A the instant the bite appears | Battle starts at once, no wait |
| T42.4 | A during the dots does nothing | Mash A while the dots tick | Dots keep going; no "Not even a nibble" from the press |
| T42.5 | **One round of dots** | Fish with the Super Rod ten times | Dots appear once per cast, never twice |
| T42.6 | A miss still misses | Fish repeatedly with the Old Rod | "Not even a nibble..." still happens, roughly three casts in four |
| T42.7 | Empty water still says so | Fish somewhere with no fishing table | "Not even a nibble..." |
| T42.8 | The rod goes away cleanly | After a miss | Player stands up, the box closes, movement returns |
| T42.9 | Surfing is unaffected | Fish while surfing | Same behaviour, surf blob intact afterwards |
| T42.10 | The encounter is a real one | Land a fishing encounter on a fresh route | Randomized species, first-encounter badge, nuzlocke area consumed |
| T42.11 | Old Rod on Route 103 | The relocated fisherman's rod | Works before the first badge, as Phase 17 expects |
| T42.12 | Feebas still needs the spot | Fish the Route 119 tiles | Unchanged: easy fishing does not change what is in the water |

---

## Phase 41 — Rolling a nature or ability takes a held button

RZ_ABILITY_STABLE_ACROSS_EVOLUTION works: the randomized ability is seeded from the
evolution family's root, so every stage of a line maps the same ability slot to the same
ability, and evolving cannot change it. Verified against the game's own evolution data —
Hatenna, Hattrem and Hatterene all resolve to Hatenna.

What *can* change it is the ability slot, and the summary screen's roll used to fire on a
bare tap of START, right beside the SELECT used for the nature, on the two pages the
player visits most. It now needs the button held for about a third of a second, and
released before it fires again.

| # | Test | Steps | Expected |
| --- | --- | --- | --- |
| T41.1 | **A tap does nothing** | Info page → tap START | Ability unchanged |
| T41.2 | **A hold rolls** | Hold START for about half a second | Ability changes once |
| T41.3 | Holding does not repeat | Keep holding for several seconds | One change, not a stream |
| T41.4 | Releasing re-arms | Release, hold again | Rolls again |
| T41.5 | The nature behaves the same | Tap SELECT, then hold it | Nothing, then one nature change |
| T41.6 | Switching buttons mid-hold | Hold START halfway, then switch to SELECT | Count restarts; no roll from the partial hold |
| T41.7 | Skills page too | Both buttons, held, on the stats view | Same behaviour |
| T41.8 | The IV/EV editor is unaffected | Skills page → A to IVs → tap SELECT | Opens the editor on a tap, as before |
| T41.9 | The move relearner is unaffected | Battle moves page → tap START | Opens the relearner on a tap, as before |

### Abilities really are stable across evolution

| # | Test | Steps | Expected |
| --- | --- | --- | --- |
| T41.10 | **Evolving keeps the ability** | Note a Pokémon's ability, evolve it, check again | Identical |
| T41.11 | …through two stages | Evolve a three-stage line twice | Identical at all three |
| T41.12 | An Ability Capsule still works | Use one | Ability changes — that is the item's job |

---

## Phase 40 — Types as text, and friendship as a number

### The overlay shows both types

Two type icons fit an 80px panel on paper, but only the first ever appeared. The types are
printed as text now, on the same grid the ability below them uses, so there is no second
sprite to go missing. A pair too long for the panel steps down to a narrower font.

| # | Test | Steps | Expected |
| --- | --- | --- | --- |
| T40.1 | **A dual type shows both halves** | Forget-a-move screen → SELECT, on an Electric/Poison Pokémon | "ELECTRIC/POISON" |
| T40.2 | A single type shows one | Same on a pure Ghost | "GHOST", centred |
| T40.3 | A long pair still fits | A Fighting/Psychic or similar | Inside the panel, smaller font if it has to be |
| T40.4 | The rest of the panel is unchanged | Same screen | Stats grid and ability as before, nothing overlapping |
| T40.5 | No stray icons | Open and close the overlay, change page, reopen | Never a leftover type box anywhere |

### Friendship is a number

The heart is gone — graphic, sprite and all. Three rounds of playtesting never made it
legible. The skills page now reads the value out, bottom right of the Pokémon's picture.

| # | Test | Steps | Expected |
| --- | --- | --- | --- |
| T40.6 | **The value is shown** | Summary → skills page | "X/255" over the bottom right of the picture |
| T40.7 | It is readable over the sprite | A Pokémon whose sprite fills that corner | White text with a black shadow, legible |
| T40.8 | It tracks the Pokémon | Page up and down the party | Changes with each one |
| T40.9 | It updates | Walk around, level up, use a vitamin, then check | The number has moved |
| T40.10 | **No heart anywhere** | Every page, info through contest moves | No heart, no leftover sprite |
| T40.11 | Skills page only | Info, battle moves, contest moves | No friendship readout on those |
| T40.12 | Eggs | An egg's summary → skills page | Blank rather than a meaningless number |
| T40.13 | It does not collide | Nickname, species, ball, level, gender mark | All still readable |
| T40.14 | Max reads plainly | Debug → Set Friendship 255 | "255/255" |

---

## Phase 39 — The legendary clause

A legendary met in the wild is always catchable, on the same terms as a shiny: allowed in
an area already used up, allowed even if its family is registered, and catching it does
not consume the area.

Decision order is now: shiny → legendary → duplicate → area used → allowed.

| # | Test | Steps | Expected |
| --- | --- | --- | --- |
| T39.1 | **A wild legendary in a used-up area** | Catch something on a route, then meet a legendary there | Catchable, and the badge shows |
| T39.2 | **It does not consume the area** | Catch that legendary, then meet something else on the route | Still catchable — the legendary was a freebie |
| T39.3 | A duplicate legendary is still allowed | Meet a wild legendary whose family is registered | Catchable; the clause outranks the dupe rule |
| T39.4 | Sub-legendaries count | A wild Regi, Latias, bird or beast | Catchable |
| T39.5 | Mythicals count | A wild Mew, Celebi, Jirachi, Deoxys | Catchable |
| T39.6 | Ultra Beasts count | A wild Nihilego, Buzzwole and so on | Catchable |
| T39.7 | **Ordinary Pokémon are unaffected** | A non-legendary in a used-up area | Still refused, same message as before |
| T39.8 | Paradox Pokémon are not covered | A wild Great Tusk, Iron Valiant | Treated as ordinary. `isParadox` is a separate flag from the four the clause reads |
| T39.9 | The twelve legendary sites still work | Rayquaza, the Regis, the Lati and so on | Unchanged — those maps have no wild encounter table, so they were never area-gated |
| T39.10 | Trainers' legendaries | A trainer with a legendary | No change; trainer battles never reach the clause |

---

## Phase 38 — Shiny rate 1 in 256

`SHINY_ODDS` 8 → 256, out of 65536. Up from 1 in 8192, a 32x increase.

| # | Test | Steps | Expected |
| --- | --- | --- | --- |
| T38.1 | **Shinies actually appear** | Run through grass for a while | Roughly one in 256 encounters is shiny |
| T38.2 | **The shiny clause is reachable now** | Meet a shiny in an area already used up | Catchable, and the badge shows |
| T38.3 | A shiny does not consume the area | Catch it, then meet something else there | Still catchable — the shiny was a freebie |
| T38.4 | A shiny dupe is still catchable | Meet a shiny whose family you already have | Catchable; the shiny clause outranks the dupe clause |
| T38.5 | Rerolls still stack on top | Use a lure, or chain fish | Shinier than 1 in 256, as before |
| T38.6 | Trainer Pokémon are unaffected | Fight trainers | Their Pokémon roll the same odds as any generated Pokémon; nothing special |
| T38.7 | Sprites and palettes are fine | Catch one and view it | Shiny palette in battle, party, PC and summary |

---

## Phase 37 — The first-encounter badge asks about the right Pokémon

The badge appeared on second encounters and then vanished on the next health box redraw.
It was asking `GetCatchingBattler()` which Pokémon to judge, and during the battle intro
that function's `IsBattlerAlive` check fails the left-hand opponent and falls through to
the right-hand one — which in a single battle is not a battler at all, so the rule ran
against a stale enemy party slot left over from an earlier battle. It now asks about the
battler whose box is being drawn, and draws nothing at all while the data is unreadable.

| # | Test | Steps | Expected |
| --- | --- | --- | --- |
| T37.1 | **A first encounter is badged** | Enter a fresh area, meet a wild Pokémon | The badge is there from the moment the box appears |
| T37.2 | **A second encounter is not** | Catch one, then meet another wild Pokémon in the same area | No badge, not even for a moment |
| T37.3 | It does not appear on a redraw | Same, then open the bag and back out | Still no badge |
| T37.4 | …and a real one does not vanish | A first encounter, then bag and back out | Badge still there |
| T37.5 | **Straight after a trainer battle** | Fight a trainer, then meet a wild Pokémon in a used-up area | No badge. This is the case the stale party slot came from |
| T37.6 | A dupe is not badged | Meet a wild Pokémon whose family is already caught | No badge |
| T37.7 | A shiny is badged | Meet a shiny in a used-up area | Badge — the shiny clause still overrides |
| T37.8 | Trainers never get one | Any trainer battle | No badge on their Pokémon |
| T37.9 | Doubles | A wild double battle, one catchable and one not | The badge sits on the right box only |
| T37.10 | Safari | Safari Zone | No badge |
| T37.11 | The rules themselves are unchanged | Try to catch in a used-up area | Still refused, same message as before |

---

## Phase 36 — TM pickups are drawn without replacement

A randomized TM — found, hidden, or handed over by a gym leader — is now drawn from the
TMs the player does not already own. TMs are reusable here, so a duplicate is not a lesser
prize, it is nothing at all, and the TM *item* is drawn uniformly from 50, so the odds of a
repeat climb with every one collected: at 20 TMs it was 40%, at 35 it was 70%.

| # | Test | Steps | Expected |
| --- | --- | --- | --- |
| T36.1 | **A gym reward is never a duplicate** | Collect a dozen TMs, then beat a gym | A TM not already in the bag |
| T36.2 | Found TMs too | Pick up a TM ball with a full-ish collection | Not a duplicate |
| T36.3 | Hidden TMs too | Same via an Itemfinder spot | Not a duplicate |
| T36.4 | A TM rolled from an ordinary item | An item pickup that lands in the TM band | Not a duplicate |
| T36.5 | **The PC counts as owned** | Deposit a TM in the PC, then collect a TM | The deposited one is not handed back |
| T36.6 | Every TM owned | Collect all 50, then take another TM pickup | Hands over some TM rather than nothing; no softlock |
| T36.7 | Seeds are still stable | Save before a TM pickup, take it, reset, take it again | The same TM, as long as the bag has not changed in between |
| T36.8 | The weighting still applies | Early game, with few TMs owned | Distribution unchanged from before — the redraw only kicks in on a collision |
| T36.9 | **No HMs from TM pickups** | Collect many TMs | Never an HM. Indices 1..50 are the TMs; the HMs sit past them |
| T36.10 | No ITEM_NONE | Same | Never an empty or glitched item |

---

## Phase 35 — Items only one Pokémon can use drop to tier 4

Memories and drives sat in tier 3, alongside real held items. With 17 memories and 4
drives at tier 3's per-item rate, **10.6% of every tiered item roll was Silvally or
Genesect gear**. They are now tier 4, with 36 one-species items in total.

| tier | items | band | per item |
| --- | --- | --- | --- |
| 1 | 2 | 1.18% | 0.590% |
| 2 | 77 | 47.92% | 0.622% |
| 3 | 61 | 41.46% | 0.680% |
| **4** | **102** | **5.37%** | **0.053%** |
| 5 | 166 | 4.07% | 0.025% |

Chance a tiered roll is a memory or a drive: **10.62% → 1.11%**.

| # | Test | Steps | Expected |
| --- | --- | --- | --- |
| T35.1 | **Memories are rare** | Collect 30-odd field items and NPC gifts on a new seed | At most one memory, usually none |
| T35.2 | Drives are rare | Same | Same |
| T35.3 | The signature orbs moved too | Watch for Adamant, Lustrous, Griseous, Red and Blue Orb | Rare rather than tier-2 common |
| T35.4 | …as did the powders and sticks | Leek, Quick Powder, Metal Powder, Thick Club, Light Ball, Soul Dew, Lucky Punch, Deep Sea Tooth/Scale | All rare |
| T35.5 | **Plates did not move** | Watch for the Arceus plates | Still as common as before — a plate boosts its type for any holder |
| T35.6 | Good items got commoner | Watch tier 3 finds | Slightly more frequent: the same band weight now covers 61 items rather than 82 |
| T35.7 | Leftovers and Choice Band unchanged | Keep collecting | Still the rarest-but-best finds |
| T35.8 | Seeds are still stable | Save, collect an item, reset, collect it again | Same item both times |
| T35.9 | NPC gifts follow the same table | Rustboro's Quick Claw man and others | Memories rare there too |

---

## Phase 34 — A fainted Pokémon looks fainted in the PC

Nothing on the storage screen said which Pokémon were dead; the only clue was the refusal
when you tried to withdraw one. Three tells now, following pokeemerald_rando_enh, which
dims the box icons and greys the portrait.

| # | Test | Steps | Expected |
| --- | --- | --- | --- |
| T34.1 | **The box icon is dimmed** | PC → a box holding fainted Pokémon | Their icons are see-through against the box wallpaper; living ones are solid |
| T34.2 | **The portrait is grey** | Put the cursor on a fainted one | The PKMN DATA picture has the colour drained out of it |
| T34.3 | **The panel says FAINTED** | Same | Where the held item would be, it reads FAINTED |
| T34.4 | Living Pokémon are untouched | Move to a living one | Solid icon, full-colour portrait, its real held item |
| T34.5 | It survives a page change | Scroll to another box and back | Still dimmed, still grey |
| T34.6 | The grey does not leak | Hover a fainted one, then a living one | The living one is full colour, not grey |
| T34.7 | …nor across a fade | Hover a fainted one, open the party view, come back | Still grey, not recoloured |
| T34.8 | Item mode still dims | PC → MOVE ITEMS | Pokémon holding nothing are dimmed, as before |
| T34.9 | Champion clears it | Beat the Champion, reopen the PC | Full colour, no FAINTED label, withdrawable |
| T34.10 | Eggs are not affected | An egg in the same box | Normal egg icon and portrait |

### Which refusal is which

| # | Test | Steps | Expected |
| --- | --- | --- | --- |
| T34.11 | **Withdrawing a fainted Pokémon** | PC → a fainted one → Withdraw | "This POKéMON is gone for good." — *not* an egg message |
| T34.12 | Releasing an egg | PC → an egg → Release | "You can't release an EGG." This is the vanilla egg rule and has nothing to do with fainting |
| T34.13 | Releasing a fainted Pokémon | PC → a fainted one → Release | Allowed |

---

## Phase 33 — A fainted Pokémon is gone

The death rule. Until now only a **total wipe** boxed anything, so a Pokémon that fainted
in a gym battle walked out of the Pokémon Center good as new. Anything that reaches 0 HP
is now boxed at the end of the battle and locked there for the rest of the run.

| # | Test | Steps | Expected |
| --- | --- | --- | --- |
| T33.1 | **A faint boxes the Pokémon** | Let one faint in a trainer battle, then end the battle | It is gone from the party and sitting in the PC |
| T33.2 | **It does not heal** | Go to a Pokémon Center afterwards | It is not in the party to be healed, and the box entry stays fainted |
| T33.3 | **Its item comes back** | Faint a Pokémon holding an item | The item is in the bag, the boxed Pokémon holds nothing |
| T33.4 | **It cannot be withdrawn** | PC → the boxed Pokémon → Withdraw | Refused with the locked message |
| T33.5 | …nor moved or shifted | Try Move and Shift on it | Both refused |
| T33.6 | It can still be released | Release it | Allowed — tidying the graveyard is permitted |
| T33.7 | Survivors are untouched | Faint one of three, win the battle | The other two stay in the party at whatever HP they had |
| T33.8 | The party closes up | Faint the lead of a full party | Slots compact, no gap, no duplicate |
| T33.9 | **A whole party down still ends the run** | Lose with every Pokémon fainted and nothing living in the boxes | Soft reset to the title, save intact |
| T33.10 | …but not while the PC has someone | Same, with a living Pokémon in a box | Walk out of the Center with an empty party and withdraw a new team |
| T33.11 | **Field poison counts** | Let a poisoned Pokémon faint walking around | Boxed the same way, after the message |
| T33.12 | Field poison to a whole party | Every remaining Pokémon poisoned to 0 | The normal white-out, then the run-over check |
| T33.13 | Wild battles too | Faint against a wild Pokémon | Boxed |
| T33.14 | Champion releases the graveyard | Beat the Champion, then open the PC | Dead Pokémon can be withdrawn again — the run is over |

### Where it deliberately does not apply

| # | Test | Steps | Expected |
| --- | --- | --- | --- |
| T33.15 | **Before the rules start** | Faint during the Route 103 rival battle, before the five Poké Balls | Nothing boxed |
| T33.16 | Birch's bag | Route 101, the Zigzagoon battle | Nothing boxed |
| T33.17 | The Wally tutorial | Petalburg Gym catching tutorial | Nothing boxed |
| T33.18 | The Frontier | Lose a Battle Tower round | Nothing boxed — rentals and borrowed teams |
| T33.19 | Nuzlocke switched off | Clear the rules flag, then faint one | Nothing boxed, heals normally |

---

## Phase 32 — Any Pokémon learns any TM, HM or tutor move

Compatibility is gone from the teaching path. This is a repair as much as a convenience:
randomized TMs draw from the whole move pool, while the teachable list holds 88 moves, so
most randomized TMs taught a move **no Pokémon in the game could learn**.

| # | Test | Steps | Expected |
| --- | --- | --- | --- |
| T32.1 | **Every Pokémon reads ABLE** | Bag → any TM → open the party | ABLE next to every Pokémon, none greyed out |
| T32.2 | **A TM that was dead now works** | A TM teaching an off-list move — Extreme Speed, Fling, Play Nice | Teaches normally |
| T32.3 | Gimmick species too | Magikarp, Ditto, Wobbuffet, Caterpie | ABLE, and the move is really learned |
| T32.4 | HMs | Teach Surf, Fly, Cut to anything | Learned. (Using them still needs no teaching at all — see Phase 31) |
| T32.5 | **Tutors** | Any of the ten randomized tutors on any Pokémon | Teaches, no refusal |
| T32.6 | Boxed Pokémon | Teach a TM to one in the PC | Same behaviour |
| T32.7 | Eggs still refuse | Try a TM on an egg | "can't learn" — an egg is still an egg |
| T32.8 | Already-known still refuses | Teach a move the Pokémon has | "already knows" |
| T32.9 | Replacing a move works | Teach a TM to a Pokémon with four moves | Normal forget-a-move flow |
| T32.10 | Reusable TMs survive | Teach the same TM twice to two Pokémon | Still in the bag |

### What deliberately did **not** change

These read the real learnsets to judge what a species plausibly has, which is a different
question from what the player may choose to give it.

| # | Test | Steps | Expected |
| --- | --- | --- | --- |
| T32.11 | **The move relearner** | Open it for any Pokémon | Its own learnset, not every move in the game |
| T32.12 | Egg moves | Breed with a TM move on the parent | Inheritance unchanged |
| T32.13 | The Frontier | Apprentice, Battle Pyramid, Battle Factory | Unchanged |

---

## Phase 31 — HMs without HM slaves

Every badge-gated field move now works with no party Pokémon knowing it. The badge is
still required. The always-unlocked ones — Teleport, Dig, Sweet Scent, Soft-Boiled, Milk
Drink, Secret Power — are untouched, because those are ordinary moves rather than
infrastructure.

### The six reached by walking into something

| # | Test | Steps | Expected |
| --- | --- | --- | --- |
| T31.1 | **Cut** | A party that knows no field moves at all → face a cuttable tree → A | "Would you like to CUT?" naming the lead Pokémon, and it works |
| T31.2 | **Rock Smash** | Same party → face a breakable rock | Prompts and breaks it |
| T31.3 | **Strength** | Same party → a boulder | Prompts and enables pushing |
| T31.4 | **Surf** | Same party → face surfable water | Prompts and surfs |
| T31.5 | **Waterfall** | Surfing north at a waterfall | Prompts and climbs |
| T31.6 | **Dive** | Over deep water | Prompts and dives, and surfacing works |
| T31.7 | Rock Climb | A rock climb tile, if enabled | Prompts and climbs |
| T31.8 | **The badge is still required** | Try each before its gym | The "can't use until a new badge" refusal, not a free pass |

### The two with no overworld trigger

| # | Test | Steps | Expected |
| --- | --- | --- | --- |
| T31.9 | **Flash appears without being known** | In a dark cave, after Badge 2 → party menu → any Pokémon | FLASH in its option list |
| T31.10 | **Fly appears without being known** | After Badge 6 → party menu → any Pokémon | FLY in its option list |
| T31.11 | Neither appears before its badge | Same, before the badge | Absent |
| T31.12 | No duplicate entry | A Pokémon that really knows Fly | FLY listed once, not twice |
| T31.13 | Flash wins a tight list | A Pokémon knowing four field moves, Cap Candy in the bag | Nothing is dropped from the menu and it still fits on screen |

### The option list cannot overflow

The list is nine entries at most — the selection window is laid out as `19 - numActions*2`
rows from the top, so ten would place it off-screen — and the array holding it was eight.
A Pokémon knowing four field moves with the Cap Candy entry alongside could already run
past the end before this phase.

| # | Test | Steps | Expected |
| --- | --- | --- | --- |
| T31.14 | **A full list is intact** | Teach one Pokémon four field moves, hold the Cap Candy, open its menu | Every entry drawn, window fully on screen, no corruption |
| T31.15 | Each entry still works | Pick each one in turn | All behave normally |

---

## Phase 30 — Rolling natures where you can see them

### The heart is 16x16 now

> **Superseded by Phase 40.** The friendship heart was removed; the skills page prints
> the value as "X/255" instead. The rows below are kept as a record of what was tried
> and are not part of the suite — do not run them.

| # | Test | Steps | Expected |
| --- | --- | --- | --- |
| T30.1 | **It is obvious now** | Summary → info page | A clearly readable heart in the bottom-right corner of the picture |
| T30.2 | It sits in front of the Pokémon | A Pokémon whose sprite fills that corner — Wobbuffet, Snorlax | Heart drawn over the sprite, not behind it |
| T30.3 | It is clear of the frame edge | Any Pokémon | Bottom-right of the striped area with a few pixels of frame below the heart's point, so nothing reads as cut off |
| T30.3b | **The gold heart reads as a heart** | A Pokémon at 255 friendship | Dark outline around the gold fill, recognisably heart-shaped rather than a solid wedge |
| T30.4 | The frames still step correctly | Compare a fresh catch with a walked one | Fill rises from the bottom through six steps |
| T30.5 | **Gold only at the true maximum** | Debug → Party… → Edit Pokemon → Set Friendship 255 | Whole heart turns gold |
| T30.5b | 254 is not gold | Set Friendship 254 | Full **red** heart, not gold. Gold has to mean "cannot go higher" |
| T30.5c | **Empty at zero** | Set Friendship 0 | Dark interior, red outline, no fill. Use this to confirm the frames step at all |
| T30.6 | Nothing else moved | Nickname, species, ball, level, gender mark | All still readable |

### SELECT re-rolls the hidden nature

| # | Test | Steps | Expected |
| --- | --- | --- | --- |
SELECT rolls the nature and START rolls the ability, on **both** the info page and the
skills page — same two buttons either side, nothing to remember about which page does
what. The skills page's IV and EV views are the exception: SELECT stays the stat editor.

| # | Test | Steps | Expected |
| --- | --- | --- | --- |
| T30.7 | **SELECT rolls the nature — info page** | Info page → SELECT | Trainer Memo reads "Naive (Modest) nature," — original, then the new hidden one. The ability does **not** change |
| T30.7b | **START rolls the ability — info page** | Info page → START | ABILITY line and its description change. The nature does **not** change |
| T30.7c | **SELECT rolls the nature — skills page** | Skills page, stats view → SELECT | The red and blue stats recolour on the spot. Left to the info page confirms the new nature |
| T30.7d | **START rolls the ability — skills page** | Skills page, stats view → START | The ability in the old RIBBON slot changes on the spot |
| T30.7e | **No debug page** | SELECT on any page, in *both* ROMs | Never the Bulbasaur sprite browser |
| T30.7f | The IV/EV editor still owns SELECT | Skills page → A until it reads IVs or EVs → SELECT | Opens the stat editor. No nature roll |
| T30.7g | They are independent | Roll a nature you want, then roll the ability repeatedly | The nature stays put |
| T30.7h | One-ability species | A species with a single ability across all slots | Failure sound, nothing changes |
| T30.7i | Hidden abilities are in the pool | Roll a species with a hidden ability repeatedly | It comes up |
| T30.7j | **Rolling is free** | Note your money, roll several times | Money unchanged |
| T30.7k | START on the move pages is still RELEARN | Battle moves page → START | Opens the move relearner |
| T30.7l | Stats follow the nature | Roll on the skills page and watch the numbers | Raised stat up and red, lowered stat down and blue |

### The ability replaces the ribbon count

| # | Test | Steps | Expected |
| --- | --- | --- | --- |
| T30.18 | **The caption reads ABILITY** | Skills page | Top-right banner says ABILITY in the same white-on-yellow style as ITEM beside it, fully visible, nothing clipped by the box below |
| T30.19 | The ability is shown | Any Pokémon | Its ability, centred in the box where the ribbon count was |
| T30.20 | A long name fits | A Pokémon with a long ability name | Inside the box, not clipped off the left |
| T30.21 | It tracks the Pokémon | Page up and down through the party | Changes with each one |
| T30.22 | Eggs | An egg's summary | Nothing broken |
| T30.23 | The other pages are unchanged | Info, battle moves, contest moves | No stray ABILITY caption |
| T30.8 | It never no-ops | Press SELECT repeatedly | The parenthetical changes every time; it never rolls the nature it already had |
| T30.9 | **The stats follow** | Roll, then page right to skills | Numbers match the new nature; the raised stat is up, the lowered one down |
| T30.10 | It persists | Roll, leave the summary, come back | Same hidden nature |
| T30.11 | …and through a save | Roll, save, reset, reload | Same hidden nature |
| T30.12 | The move-select arrows follow | Roll, then reach the forget-a-move screen and press SELECT | Red/blue arrows match the rolled nature |
| T30.13 | Not on other pages | SELECT on skills, battle moves, contest moves | Skills opens the IV/EV editor; the move pages do nothing. No nature roll |
| T30.14 | Not on boxed Pokémon | PC → a boxed Pokémon's summary → SELECT | Nothing happens |
| T30.15 | Not on eggs | An egg's summary → SELECT | Nothing happens |
| T30.16 | Not on rentals | Battle Factory rental summary → SELECT | Nothing happens |
| T30.17 | The debug action still works | Debug → Party… → Edit Pokemon → Roll Hidden Nature | Still rolls, still reports the new nature |

---

## Phase 29 — Heart placement

> **Superseded by Phase 40.** The friendship heart was removed; the skills page prints
> the value as "X/255" instead. The rows below are kept as a record of what was tried
> and are not part of the suite — do not run them.

| # | Test | Steps | Expected |
| --- | --- | --- | --- |
| T29.1 | **It is where it should be** | Summary → info page | Heart inside the picture frame, bottom right, above the nickname. Not clipping the green border |
| T29.2 | It does not cover the nickname | A Pokémon with a nine-character nickname | Name fully readable |
| T29.3 | It survives page changes | L/R through all four pages | Heart on every page, same spot |
| T29.4 | Eggs still have none | An egg's summary, then page through | No heart anywhere |
| T29.5 | The stats overlay hides it | Forget-a-move screen → SELECT → SELECT | Gone, then back |
| T29.6 | The colour is right | A freshly caught Pokémon, then a maxed one | Red outline filling from the bottom; gold at 250 |

---

## Phase 27 — Playtest round 4

### Fast text by default

| # | Test | Steps | Expected |
| --- | --- | --- | --- |
| T27.1 | **A new game starts on Fast** | New game → Options | Text speed reads FAST |
| T27.2 | Still changeable | Set it to Slow, leave, come back | Slow, and it sticks |
| T27.3 | An existing save is untouched | Load a save started before this build → Options | Whatever it was already set to. Only new games get the new default |

### Indicator colours, not window colours

The previous build coloured palette entry 13, which every window on that BG draws its text
with, so one arrow turned the move names, the type line and the action menu red — and the
change survived leaving move select. The icons now carry their own palette entries.

| # | Test | Steps | Expected |
| --- | --- | --- | --- |
| T27.4 | **Move names stay black** | Battle → Battle → hover a move with no effect | The ✕ is red; both move names and "TYPE/…" stay dark grey |
| T27.5 | Green for super effective | Hover a move the foe is weak to | Green ↑ (double for 4×); names unchanged |
| T27.6 | Orange for resisted | Hover a resisted move | Orange ↓; names unchanged |
| T27.7 | **It does not leak** | Hover a no-effect move, then B out to the action menu | Battle / Bag / Pokémon / Run are black, not red |
| T27.8 | The STAB dot stays red | Hover a same-type move that is also super effective | Green arrow *and* a red dot side by side |
| T27.9 | Neutral is unremarkable | Hover a neutral move | Grey hollow circle, the same grey as the text |

### Level cap candy no longer exits the menu

`Task_LearnNextMoveOrClosePartyMenu` tested `data1` — which by then holds the move just
learned — instead of `learnMoveState`. Every move but MOVE_POUND took the "close the menu"
branch, so the Pokémon also never reached the evolution check.

| # | Test | Steps | Expected |
| --- | --- | --- | --- |
| T27.10 | **Learning a move keeps you in the menu** | Party → a Pokémon → LEVEL CAP onto a level-up move | Learns the move, then returns to the party list. Does *not* drop to the field |
| T27.11 | Replacing a move too | Same with four moves already known → forget one | Returns to the party list |
| T27.12 | Declining also returns | Same, but choose not to learn | Returns to the party list |
| T27.13 | **It evolves now** | LEVEL CAP a Pokémon onto its evolution level | Evolution scene plays |
| T27.14 | **…and comes back cleanly** | Press through the evolution and the Pokédex entry | Back in the party menu on the field. Press B → field. Not a frozen battle screen |
| T27.15 | Evolve on a level that also learns a move | A species that does both at once | Move first, then the evolution, then the party menu |
| T27.16 | Repeat immediately | LEVEL CAP the same Pokémon again | Works; the Cap Candy is never consumed |
| T27.17 | A real Rare Candy still evolves | Bag → Rare Candy on a Pokémon at its evolution level | Evolves, then back to the bag |
| T27.18 | A TM still closes the menu | Bag → any TM → teach it | Learns, then back to the bag. Unchanged |
| T27.19 | Move tutors still work | A randomized tutor → teach a move | Learns and returns to the overworld script |

### Stats on the move-select screen

| # | Test | Steps | Expected |
| --- | --- | --- | --- |
| T27.20 | **The prompt is visible** | Level a Pokémon into "which move should be forgotten?" | Top right reads SELECT + "Stats", not "Ⓐ INFO" |
| T27.21 | **SELECT shows the numbers** | Press SELECT | The picture is replaced by the Pokémon's type icons, HP / ATK / DEF / SpA / SpD / SPE, and the ability |
| T27.21b | **Dual types sit side by side** | A two-type Pokémon | Both icons across the top of the panel, inside the margins |
| T27.21c | A single type is centred | A one-type Pokémon | One icon, centred. No leftover second icon |
| T27.21d | The icons are in front | Open the overlay | Icons drawn over the white panel, not hidden behind it |
| T27.21e | They do not linger | Open the overlay, close it, change page, reopen | No stray type icons anywhere on screen |
| T27.22 | The numbers are the real ones | Compare with the skills page afterwards | Identical |
| T27.23 | Nature colouring | A Pokémon with a non-neutral nature | Raised stat red, lowered stat blue, rest black |
| T27.24 | **Hidden nature wins** | A Pokémon given a Mint or a rolled hidden nature | The colours follow the *hidden* nature, which is what moved the numbers |
| T27.25 | SELECT again hides it | Press SELECT a second time | Picture returns |
| T27.26 | Page change hides it | Open the overlay, press L or R | Overlay gone, picture back, contest page drawn normally |
| T27.27 | Confirming works with it open | Open the overlay, press A on a move | Forgets that move and returns as usual |
| T27.28 | Cancelling works with it open | Open the overlay, press B | Declines as usual |
| T27.29 | The ability fits | A long ability name, e.g. Neutralizing Gas | Drawn inside the panel, not clipped off the left edge |
| T27.30 | Not offered elsewhere | Normal summary → moves page → press SELECT | Nothing happens. Only the forget-a-move screen has it |

---

## Phase 26 — Friendship heart and a bigger bag

> **Superseded by Phase 40.** The friendship heart was removed; the skills page prints
> the value as "X/255" instead. The rows below are kept as a record of what was tried
> and are not part of the suite — do not run them.

| # | Test | Steps | Expected |
| --- | --- | --- | --- |
| T26.1 | **The heart appears** | Summary → info page | A small heart at the bottom right of the picture frame, just above the nickname |
| T26.2 | It starts nearly empty | A freshly caught Pokémon (friendship 70) | Low fill |
| T26.3 | **It fills as friendship rises** | Walk with it, level it, use vitamins | The heart fills from the bottom in steps at 42 / 85 / 128 / 170 / 212 |
| T26.4 | **Gold at maximum** | Friendship 250 or more | A gold heart |
| T26.5 | It tracks the selected Pokémon | Page up and down between party members | The heart changes with each one |
| T26.6 | **Every page** | Switch to skills, moves, contest | The heart stays put on all four |
| T26.7 | Hidden behind the stats overlay | Forget-a-move screen → SELECT | Heart gone with the picture; back when SELECT is pressed again |
| T26.8 | Eggs have none | View an egg's summary | No heart |
| T26.9 | It does not collide | Look at the ball icon, status icon and name | All still drawn correctly |
| T26.10 | Boxed Pokémon | Open a boxed Pokémon's summary | Heart shows normally |

### Bigger bag

| # | Test | Steps | Expected |
| --- | --- | --- | --- |
| T26.11 | **Items pocket holds 150** | Buy every evolution item and keep collecting | No "the bag is full" until 150 distinct items |
| T26.12 | Balls hold 40 | Buy Ultra, Fast and Timer Balls plus finds | 40 distinct entries |
| T26.13 | Berries hold 60 | Harvest randomized berry trees | 60 distinct entries |
| T26.14 | Key items hold 50 | Collect all four custom key items and the tickets | Fits comfortably |
| T26.15 | **Freed features are really gone** | Check Mystery Gift on the main menu | Absent — its save data was reclaimed for the bag |
| T26.16 | The game still saves | Play, save, reset, reload | Loads correctly; bag contents intact |
| T26.17 | **A pre-0.9.2 save does not load** | Try an older save | Expected — the bag change moved SaveBlock1. Start a new game |

---

## Phase 25 — First-encounter badge

`RANDOLOCKE_FIRST_ENCOUNTER_BADGE` (TRUE). A circled **1** appears on a wild Pokémon's
health box when catching it *here* would be legal under the nuzlocke rules — so you do
not have to throw a ball to find out.

Printed onto the healthbox sprite the same way HP numbers are, so it inherits the box's
position, slide-in and cleanup rather than owning a second sprite.

| # | Test | Steps | Expected |
| --- | --- | --- | --- |
| T25.1 | **It appears on a legal catch** | Walk into fresh grass on a route you have not caught in | A circled 1 on the wild Pokémon's health box |
| T25.2 | **Gone once the area is used** | Catch something there, then meet another | No badge |
| T25.3 | Gone on a duplicate | Meet a species whose family you already own | No badge, even on an unused area |
| T25.4 | **Shinies always show it** | Meet a shiny on a used-up area | Badge present — the shiny clause makes it catchable |
| T25.5 | It agrees with the bag | Whenever the badge shows, open the bag | Balls are allowed. No badge means they are refused |
| T25.6 | Not on trainer battles | Fight any trainer | No badge |
| T25.7 | Not on your own Pokémon | Look at your side of the screen | No badge there |
| T25.8 | Not in the Safari Zone | Enter a Safari battle | No badge |
| T25.9 | **Before the rules start** | New game, before Birch's five Poké Balls | No badge — nothing is being ruled on yet |
| T25.10 | Rules off | Set flag `0x2D` | No badge |
| T25.11 | Doubles | A double wild battle | Each opposing box is judged on its own species |
| T25.12 | **It does not corrupt the box** | Watch HP change, switch, and let the box slide out | HP numbers, nickname, level and status all draw correctly; no leftover glyph |
| T25.13 | Survives a switch | Send out a different Pokémon and back | The badge is still correct |
| T25.14 | Config off | Set `RANDOLOCKE_FIRST_ENCOUNTER_BADGE` to `FALSE`, rebuild | No badge anywhere |

---

## Phase 24 — Two registered key items, properly

`RANDOLOCKE_DUAL_REGISTERED_ITEMS` (TRUE). The first attempt silently pushed the
previously registered item into the second slot, which from the outside looked exactly
like registering having failed. It asks now.

| # | Test | Steps | Expected |
| --- | --- | --- | --- |
| T24.1 | **It asks which gesture** | Bag → a key item → REGISTER | "Register to a tap of SELECT, or to holding it down?" with TAP / HOLD |
| T24.2 | TAP fills the tap slot | Choose TAP | The usual SELECT badge appears beside the item |
| T24.3 | **HOLD fills the hold slot** | Register a second item, choose HOLD | A *differently coloured* SELECT badge beside that one |
| T24.4 | **Porta Heal registers** | Register the Porta Heal to either slot | It takes. This is the case that failed before |
| T24.5 | Tap uses slot one | In the overworld, tap SELECT | The TAP item is used |
| T24.6 | **Hold uses slot two** | Hold SELECT for about a third of a second | The HOLD item is used, not the tap one |
| T24.7 | A hold does not also fire the tap | Hold, then release | Only the hold item is used, once |
| T24.8 | Deselecting needs no prompt | Choose REGISTER on an already-registered item | It clears immediately, no TAP/HOLD question |
| T24.9 | An item lives in one slot | Register an item to TAP, then to HOLD | It moves; the tap slot is now empty |
| T24.10 | Both badges at once | Have one item in each slot, look at the bag | Two items, two different badges |
| T24.11 | Cancel is safe | Open the prompt and press B | Nothing registered, nothing cleared |
| T24.12 | Survives a reload | Register both, save, reset, reload | Both still registered to the same gestures |
| T24.13 | Non-key items are unaffected | Try to register a Potion | REGISTER is not offered, as in vanilla |

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
- **Log view** (View → Log) shows `MgbaPrintf` output. The randomizer's
  `GetSpeciesGroup:` lines go out at **Info**, and only in the debug ROM
  (`#ifndef NDEBUG`), so the Debug channel is always empty. Info is also where mGBA puts
  its own `GBA DMA:` hardware trace, which will drown anything the game prints — turn
  that category off in mGBA before looking.
- **Reset vs. reload** — some randomizer bugs only appear after a true power cycle. Use
  *File → Reset*, not just a save-state reload, when testing persistence.
