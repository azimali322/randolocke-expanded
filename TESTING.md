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
