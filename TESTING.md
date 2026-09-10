# Randolocke-Expanded — In-Game Test Plan

Companion to [RANDOLOCKE_PLAN.md](RANDOLOCKE_PLAN.md). Tests are added phase by phase as
the project progresses. Run the tests for the phase you just built, **plus the regression
set** (§R) every time.

**How to run the ROM:** build with `make`, then open `pokeemerald.gba` in mGBA. See §P for
patching/distribution. You do **not** need to patch anything to test your own build.

**Recommended emulator:** [mGBA](https://mgba.io/) — it has a save-state system, a memory
viewer, and prints `MgbaPrintf` debug output from the randomizer (View → Log).

**Debug menu:** hold `R` and press `START` in the overworld (`DEBUG_OVERWORLD_MENU` in
`include/config/debug.h`).

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
