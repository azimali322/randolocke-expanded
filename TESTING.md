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
