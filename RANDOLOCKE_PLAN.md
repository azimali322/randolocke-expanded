# Randolocke-Expanded — Build Plan

**Owner:** azimali322
**Repo:** `randolocke-expanded` (fork of RHH pokeemerald-expansion)
**Plan created:** 2026-09-09
**Status:** Phases 0-7 and 10-12 code complete. Phase 8 largely superseded by Phase 11; its terrain edits need Porymap. Nothing verified in-game yet. All four enhancements implemented. Phase 8 (maps & events) and Phase 9 (ship) outstanding; Phases 3-7 awaiting in-game verification

---

## 1. Goals

Recreate **Pokémon Randolocke v1.1** (by Istorian) as my own romhack, with three enhancements:

1. **Abilities do not randomize on evolution.** ✅ **Implemented in Phase 4** as
   `RZ_ABILITY_STABLE_ACROSS_EVOLUTION`. A Pokémon's randomized ability stays stable across
   its evolution family instead of rerolling when it evolves.
2. **Manual IV/EV editing.** ✅ **Implemented in Phase 5** — Debug → Party → Edit Pokemon →
   Set IVs / Set EVs.
3. **Full Randolocke v1.1 parity.** All features from Randolocke v1.1 present in my romhack.
4. **Nature editing.** ✅ **Implemented in Phase 5** — Set Nature (true nature) alongside the existing Set Hidden Nature (Mint).
5. **Modern Emerald-style QoL.** Reusable TMs, battle type icons, always-run, dual
   registered key items, and IV/EV visibility in the summary and move-learning screens.
   Reference: <https://github.com/resetes12/pokeemerald>

**Reference:** <https://www.pokecommunity.com/threads/pok%C3%A9mon-randolocke-v1-1.537596/>

### 1.1 Enhancement specs

#### Enhancement 1 — Ability stability across evolution

Today `RandomizeAbility(species, abilityNum, originalAbility)` seeds its RNG from
`species | abilityNum`. Evolving changes `species`, so the ability rerolls.

**Approach:** resolve the species to its evolutionary family root before seeding, using
`GetSpeciesPreEvolution()` (`src/pokemon.c`). Gate behind a new config, e.g.
`RZ_ABILITY_FOLLOWS_EVOLUTION` in `include/config/randomizer.h`.

**Note:** this is *distinct* from Randolocke v1.1's existing "don't randomize abilities"
toggle. That one turns randomization off. This one keeps randomization but makes the
result stable within an evolution family.

#### Enhancement 2 — Manual IV/EV editor

The expansion debug menu can already set IVs/EVs **at give-time**
(`Give -> Pokémon (Complex)`), and the Party submenu can *view* them (Check EVs / Check IVs),
but there is no editor for an existing party Pokémon.

**Approach on 1.17:** add IV and EV editor rows to the existing
`sDebugMenu_Actions_EditPokemon[]` table in `src/debug.c` — which already hosts
Set Hidden Nature / Set Friendship / Set Ability. The digit-input widgets from the
give-time flow are reusable. Watch EWRAM (see §6).

#### Enhancement 4 — Nature editing

Nature is **derived**, not stored: `GetNature()` is `personality % NUM_NATURES`
(`src/pokemon.c`). Personality also determines gender, shininess, Unown letter, Wurmple's
evolution branch and Spinda spots, so editing it naively has side effects.

1.17 already stores a separate `hiddenNatureModifier:5` (`include/pokemon.h`), XORed against
the personality nature. Stat calculation reads `MON_DATA_HIDDEN_NATURE`; the summary screen
displays `GetNature()` and colours stat arrows from the mint nature. This is the Gen 8 Mint
mechanic.

**Option A — Mint-style (already shipping).** `Debug -> Party -> Edit Pokemon ->
Set Hidden Nature` exists as of Phase 1. Changes which nature governs **stats**, no side
effects, no work required.

**Option B — true nature (changes the displayed nature too).** Do *not* use
`ModifyPersonalityForNature()` (`src/battle_main.c`) directly: it shifts personality by up
to +/-12, and gender is `genderRatio > (personality & 0xFF)` with **no gender modifier
field**, so it can silently flip gender near a ratio boundary.

Instead, step personality by a **multiple of 256**:
- gender reads only the low byte, and `+256*m` leaves it byte-identical -> gender preserved
- nature is `personality % 25`, and `256 = 6 (mod 25)`; 6 is coprime to 25, so stepping by
  256 reaches every nature

Then recompute and re-set `shinyModifier:1` to preserve the original shiny state. Residual
changes are limited to Unown letter, Wurmple's branch and Spinda spots (cosmetic, rare).

**Placement:** Phase 5, alongside Enhancement 2 — same debug menu table, same test pass.

#### Enhancement 5 — Modern Emerald-style QoL

Verified against the 1.17 tree. **Three of six are one-line config flips.**

| # | Feature | Status on 1.17 | Where |
| --- | --- | --- | --- |
| F1 | TMs not consumed on use | ✅ **config exists** — `I_REUSABLE_TMS` (`include/config/item.h`), currently `FALSE` | Phase 6 |
| F2 | Type icons next to opposing Pokémon when choosing a move | ✅ **config exists** — `B_SHOW_TYPES` (`include/config/battle.h`), currently `SHOW_TYPES_NEVER`. Options: `_ALWAYS`, `_CAUGHT`, `_SEEN` | Phase 6 |
| F3 | See IVs/EVs in the summary screen | ✅ **config exists** — `P_SUMMARY_SCREEN_IV_EV_INFO`, plus `P_SUMMARY_SCREEN_IV_EV_VALUES` for numbers instead of letter grades | Phase 6 |
| F4 | Always run (no B button) | ❌ needs code — no auto-run config exists | Phase 7 |
| F5 | Two registered key items (Select / hold Select) | ❌ needs code **and a save field** | Phase 7 |
| F6 | See EVs when choosing a move to learn | ❌ needs code | Phase 7 |

**F2 note:** the config comment describes it as showing type indicators next to Pokémon HP
bars *while choosing a move after selecting a target* — exactly the requested behaviour.

**F3 note:** this also satisfies the Randolocke v1.0 parity item "IVs can be seen in the
summary screen" (§4).

**F4 implementation:** running is gated at `src/field_player_avatar.c` by
`(heldKeys & B_BUTTON)` inside the dash check. Add an `OW_AUTO_RUN` config that inverts the
test (hold B to *walk*) rather than deleting it, so the player keeps a way to walk precisely.

**F5 implementation:** vanilla stores exactly one, `u16 registeredItem` in `SaveBlock1`
(`include/global.h`). A second requires a new save field plus Select / hold-Select input
handling. ⚠️ **This changes the save layout — requires a new game.** Batch it with any
other save-affecting work so players only lose saves once.

**F6 implementation:** the move-learning UI needs the mon's EV spread surfaced so the player
can judge a physical vs. special move. No config; a UI addition.

#### Enhancement 3 — Randolocke v1.1 parity

Tracked as a checklist in §4.

---

## 2. Base version decision

**Decision: build on RHH `1.17.0` / `rhh/master`, not `1.12.3`.**

Randolocke v1.1 is itself based on expansion **1.12.1**, and tertu's randomizer is based on
**1.11.3**. Building on 1.17 is deliberately the harder path, chosen for the modern base.

Measured trial merges from the randomizer base (run, then aborted):

| Target | Conflicted files |
| --- | --- |
| `expansion/1.12.3` | 12 |
| `expansion/1.17.0` / `rhh/master` | 29 |

### What 1.17 buys us

- **Table-driven debug menu.** One row per action instead of three parallel structures
  (enum + list + dispatch table). Directly helps Enhancement 2. Also adds a
  `Party -> Edit Pokemon` submenu, which is the natural home for the IV/EV editor,
  and `Set Ability` for testing Enhancement 1.
- **`ALL_TEACHABLES` teaching type** — makes "all Pokémon can learn every move" a
  one-field-per-species change.
- **Move relearner NPCs** (`P_ENABLE_MOVE_RELEARNERS`, `P_TM_MOVES_RELEARNER`,
  `P_FLAG_TUTOR_MOVES`, `P_ENABLE_ALL_TM_MOVES`) — replaces much of Randolocke's
  hand-built "Tutor Mansion".
- **Instant text** (`include/config/text.h`, new in 1.14) — real QoL for a randomizer run.
- **Mass outbreaks** — scriptable runtime encounters; useful for guaranteeing evolution access.
- **EWRAM/IWRAM savings** (1.14 HGSS dex, 1.16 general) — matters, see §6.
- +52 species (1,627 -> 1,679), dynamic weather, daily seed, box-summary rename/relearn.

### What it costs

Five minor versions, each with a `REFACTORS` section. Detailed in §5.

---

## 3. Scope decisions

### Cut from v1

| Feature | Why |
| --- | --- |
| Galarian Yamask statue evolution | Istorian ships it marked *"this feature is untested"* in v1.1 itself. Porting an untested feature across five versions is pure cost. |
| Nuzlocke wild encounters (one catch per location) | Needs new persistent save storage on a save layout that moved. Already marked "Requires New Game" and needing testing in multi-floor areas. Players can self-enforce. |
| Six of the eight v1.1 option toggles | Each needs save storage + menu plumbing + testing. See below. |

### Deferred to v1.1

| Feature | Why |
| --- | --- |
| TM / tutor / move randomization | Not inherited from tertu (see §5.1). Must be written against 1.17's build-time teachables codegen. Its own project. |

### Kept, but explicitly

- **Mega / Primal / Dynamax / Gigantamax / Tera / Fusion / Z-Moves stay disabled.**
  1.17 *adds* Z-A Megas, so this needs more explicit disabling than Istorian needed.
- **Two option toggles are nearly free** because tertu already flag-gates them:
  "don't randomize abilities" (`RANDOMIZER_FLAG_ABILITIES`) and the ability-related
  gating. Ship these.

---

## 4. Randolocke v1.1 parity checklist

### Randomization
- [ ] Species pool through Gen 9
- [ ] Wild / trainer / gift Pokémon randomized to similar BST
- [ ] Starters & gift Pokémon guaranteed 3 perfect IVs
- [ ] Abilities randomized (Shedinja's Wonder Guard excluded)
- [ ] Field items and gift items randomized
- [ ] Fixed mapping species -> species, per Porymap slot
- [ ] *(deferred)* Moves randomized, including TMs, excluding HMs
- [ ] *(deferred)* Tutor moves randomized

### Battle
- [x] Every Pokémon learns 21 moves at fixed levels (7 STAB / 7 status / 7 damaging) ✅
- [x] All Pokémon can learn every move (`ALL_TEACHABLES`) ✅
- [x] Hard level caps per badge: 14 / 21 / 24 / 29 / 36 / 43 / 47 / 50 / 63, E4 -> 100 ✅ landed early via 1.17's built-in cap system
- [x] Trainer parties scaled to the level caps ✅
- [ ] Upgraded opponent AI (likely config on 1.17)

### Quality of life
- [x] HMs usable out of battle without a party member knowing the move ✅
- [x] HMs forgettable like normal moves ✅
- [x] IVs visible in the summary screen (`P_SUMMARY_SCREEN_IV_EV_INFO`) ✅ done via F3
- [x] Expanded bag size ✅
- [x] Key items: infinite Repellant, Porta Heal, Endless Candy, Cap Candy ✅ landed early
      *(Cap Candy takes the soonest of next cap / next level-up move / next level evolution.
      Distinct from `B_RARE_CANDY_CAP`, which only stops normal Rare Candy exceeding the cap.)*
- [ ] Press R to throw a ball

### Map & event changes
- [x] Old Rod sailor moved to Route 103 ✅ now at (22, 11); the Dewford object is hidden
      rather than deleted, so object local IDs stay put
- [ ] Scorched Slab populated with Pokémon
- [x] Water added to Littleroot, grass added to Oldale ✅ with water/fishing and land
      encounter tables to match
- [ ] Lilycove Dept. Store evolution-item sellers
- [ ] Scroll of Darkness / Scroll of Waters from the Mossdeep white rock
- [ ] Tutor Mansion in Lilycove (may be partly replaced by 1.17 relearner NPCs)
- [ ] Route 123 Bisharp / Leader's Crest event
- [ ] Fallarbor Remoraid gift (for Mantyke)
- [ ] Verdanturf Shelmet / Karrablast trades
- [ ] Mauville Game Corner Gimmighoul coins
- [ ] ~~Route 111 Yamask statue~~ — **cut**

### v1.1 additions
- [ ] Oldale NPC gifting 999 Ultra Balls
- [x] Oldale NPC gifting $999,999 ✅ one-time, `RANDOLOCKE_FLAG_OLDALE_MONEY_GIVEN`
- [x] Increased wild catch rates ✅
- [x] Slateport NPC selling legendary location maps ✅ the four event tickets at ₽1, from the
      8th badge. Sets `FLAG_ENABLE_SHIP_*` as well as giving the item
- [ ] Regi caves unlocked post-Sootopolis
- [ ] Kyogre/Groudon Weather Institute events post-Sootopolis
- [ ] Zweilous evolves at 63 (before E4)
- [ ] Dark Void usable by all species

### v1.1 option toggles
- [ ] Don't randomize abilities *(nearly free — tertu flag)*
- [x] Higher-Base-Power moves learned later (default on) ✅
- [x] Porta Heal does not revive by default ✅ (`RANDOLOCKE_PORTA_HEAL_REVIVES`)
- [x] Don't randomize movesets ✅ — clear flag `0x28` for vanilla learnsets
- [ ] ~~Enforce nicknaming~~ — deferred
- [ ] ~~Nuzlocke wild encounters~~ — **cut**
- [ ] ~~Disable bag in trainer battles~~ — deferred

---

## 5. Known hard problems

### 5.1 Move randomization is not inherited

tertu's randomizer declares but does not implement three features:

```c
RANDOMIZE_BASE_STATS,   // Not yet implemented.
RANDOMIZE_MON_TYPES,    // Not yet implemented.
RANDOMIZE_LEARNSET,     // Not yet implemented.
```

Randolocke's move/TM/tutor randomization is **Istorian's own work on 1.12.1**, not
something the merge gives us. On 1.17 it must target the build-time pipeline
(`tools/learnset_helpers/make_teachables.py` -> `src/data/pokemon/teachable_learnsets.h`,
fed by `all_learnables.json` + per-species `teachingType` + available TMs/tutors),
not a runtime table. **Deferred to v1.1.**

### 5.2 Trainer-mon randomization hook must be redesigned

tertu patches `CreateNPCTrainerPartyFromTrainer` in `src/battle_main.c`, adding a 5th
`u16 seed` parameter (passed `trainerNum`) and intercepting the `species` local before
`CreateMon`. On 1.17 that function:

- moved to `src/battle_setup.c`
- dropped to two parameters — **no `seed`, no `trainerNum` in scope**
- builds mons through `GenerateMonFromTrainerMon()` + a heap `struct TrainerGenerator`,
  so there is no `species` local to intercept
- pairs with `GetAbilityBySpecies`, whose signature changed independently to
  `enum Ability GetAbilityBySpecies(enum Species species, u8 abilityNum)`

The `seed` is what makes a trainer's randomized party stable across save/reload, so it has
to be re-plumbed or re-derived.

### 5.3 `isBossTrainer` is tertu's own trainer-pipeline extension

Not upstream in 1.12.3 or 1.17. tertu adds a `bool8 isBossTrainer:1` to `struct Trainer`,
teaches `tools/trainerproc/main.c` a `Boss:` key, and regenerates `src/data/trainers.h`
(the 24,948-line diff). 1.16/1.17 also rewrote that pipeline ("Rework special trainer IDs",
"Dynamic trainerbattle Scripts"), so this must be re-landed against a moved target.

### 5.4 The enum tax

1.17 converted species/move/item/ability from `u16` to typed enums. The **entire**
randomizer public API is `u16` — ~15 functions plus internals and both whitelist tables.
Mechanical and compiler-guided, but it touches every randomizer file.

### 5.5 `cantRandomizeAbility` save bitfield

tertu packs `cantRandomizeAbility:1` into the Pokémon save substruct. `include/pokemon.h`
took **85 commits** between 1.12.3 and 1.17. Getting the bit-packing wrong corrupts saves
silently, hours into a run. Low effort, high blast radius — test deliberately.

### 5.6 Script macro drift

tertu's `givemonrandom` macro mirrors `givemon`'s 27 parameters. 1.17 added an `IsEgg`
argument to `givemon`, so the mirror needs re-syncing.

---

## 6. Environment & build notes

- devkitARM at `/opt/devkitpro/devkitARM` (**GCC 16.1.0**); `DEVKITPRO`/`DEVKITARM` exported
  in `~/.zshrc`.
- Build with `/opt/homebrew/bin` prepended to `PATH` — `libpng-config` otherwise resolves to
  Anaconda's and breaks `gbagfx`.
- **GCC 16 fix:** `CPPFLAGS` needs `-std=gnu17` (commit `0e8e5000f6`). The build preprocesses
  and compiles separately; without it, cpp defaults to gnu23 and emits C23 `nullptr_t` into
  the `.i` file. **Both 1.12.3 and 1.17 already carry this fix upstream**, so this commit is
  superseded by the merge.
- `baserom.gba` is **not** a build input for pokeemerald-expansion — nothing in the Makefile
  references it. It is only useful for producing distribution patches. It is gitignored
  (`.gitignore:5`) and must stay that way.

### EWRAM watch

Post-randomizer-merge build on 1.11.3: **EWRAM 90.14%**, IWRAM 86.49%, ROM 78.12%.
~20 KB EWRAM headroom. The randomizer's dynamic species tables cost 6 bytes per species,
and 1.17 adds 52 more species. 1.14/1.16 EWRAM savings help, but check the memory table
after every phase.

---

## 7. Phased plan

Each phase ends at a **green build**, committed separately, so failures stay bisectable.

### Phase 0 — Baseline ✅ complete
- [x] Verify `baserom.gba` SHA-1 (`f3ae0881…d07b7`, vanilla Emerald rev 0)
- [x] Fix GCC 16 build (`-std=gnu17` in `CPPFLAGS`)
- [x] Green baseline build
- [x] Merge `upstream/tertu-randomizer` (zero conflicts, fast-forward off HEAD)
- [x] Green randomizer build; tag `pre-randomizer-merge` for rollback

### Phase 1 — Merge to 1.17 ✅ complete
Branch: `update/expansion-1.17` (commit `ba19bda633`)
- [x] Merge `rhh/master`, resolve 29 conflicts to a **compiling** state
- [x] `src/data/trainers.h` — **better than expected:** upstream deleted it and now
      generates it via `trainerproc`, so it is gitignored. The 24,948-line hand-merge
      from §5.3 evaporated; only the `Boss:` key had to be re-landed in `trainerproc`.
- [x] Green build — EWRAM 89.99%, IWRAM 86.63%, ROM 79.66%
- [x] Commit

**Carried forward:** randomizer hooks displaced by the merge are captured in
[`docs/randolocke/phase1-displaced-hunks.md`](docs/randolocke/phase1-displaced-hunks.md)
(43 hunks across 28 files). Phase 3 works from that file.

**Also fixed in passing:** `src/dexnav.c` `ENCOUNTER_TYPE_HIDDEN` read `waterMonsInfo`
(leaked from the previous `case`) instead of `hiddenMonsInfo`.

**Still stubbed, for Phase 3:** `MonListHasSpecies` in `src/pokedex_area_screen.c` was
restored to upstream's version; the randomizer's Pokédex-area display needs re-landing
(it requires `header` + `area`, which upstream's new signature no longer passes).

### Phase 2 — Enum conversion ✅ complete
Commit `2d6fbeb3b9`
- [x] Convert randomizer API from `u16` to `enum Species` / `enum Item` / `enum Ability`
      (plus `enum Type` for `RandomizeMonType`)
- [x] Convert `ability_whitelist.h` and `item_whitelist.h`
- [x] Green build, no `-Wenum-conversion` warnings; memory byte-identical to Phase 1

The raw RNG helpers (`RandomizerRand`, `RandomizerRandRange`) intentionally keep `u16` —
they return numbers, not domain values. Tests in [TESTING.md](TESTING.md) §Phase 2.

### Phase 3 — Re-land the randomizer hooks — code complete, awaiting in-game verification
Commit `e013bd220c`
- [x] Trainer-mon hook into `GenerateMonFromTrainerMon` / `TrainerGenerator`, seed carried
      on the generator struct as `rzTrainerId` / `rzSlot` / `rzTotalMons` (§5.2)
- [x] `isBossTrainer` end to end — `trainerproc` emits it (Phase 1), and it now drives
      both species exemption and `MON_DATA_CANT_RANDOMIZE_ABILITY` (§5.3)
- [x] `cantRandomizeAbility` — set on every generated trainer mon
- [x] `givemonrandom` re-synced (done in Phase 1) (§5.6)
- [x] Hidden-item hook re-landed, placed after upstream's new coins branch
- [x] Wild / fixed / starter / gift / egg / ability hooks confirmed live by audit
- [x] Green build (ROM +192 bytes, RAM unchanged)
- [ ] **Verify in emulator** — see [TESTING.md](TESTING.md) Phase 3, especially 3.3
      (trainer parties stable across a true reset) and 3.11 (hidden coins)

**Not done, carried forward:** the Pokédex area screen randomizer display
(`MonListHasSpecies` needs `header` + `area`, which upstream's signature no longer passes).

**Note:** `PreloadRandomizationTables()` was dropped from `new_game.c` by the merge but is
still called from `src/overworld.c`, so the tables are populated. Worth confirming during
Phase 3 testing that the first randomized encounter after a New Game is correct.

### Phase 4 — Enhancement 1: ability stability across evolution — code complete
- [x] Config added as **`RZ_ABILITY_STABLE_ACROSS_EVOLUTION`** (default `TRUE`) — renamed
      from the planned `RZ_ABILITY_FOLLOWS_EVOLUTION`, which read ambiguously: it was
      unclear whether TRUE meant "ability follows you through evolution" (stable) or
      "ability changes when you evolve" (the old behaviour).
- [x] `RandomizeAbility` seeds from the evolution-family root (both the seed and the
      reason-species argument)
- [x] Family root memoized in a 16-entry direct-mapped cache (64 bytes EWRAM), because
      `GetSpeciesPreEvolution()` is a linear scan over ~1,679 species and the battle AI
      calls `GetAbilityBySpecies()` repeatedly while scoring moves
- [x] Chain walk bounded by `RANDOMIZER_MAX_EVO_STAGES` so a cyclic evolution table
      cannot hang the game
- [ ] **Verify in emulator** — [TESTING.md](TESTING.md) Phase 4, especially 4.2 and 4.10

### Phase 5 — Enhancements 2 and 4: IV/EV and nature editors — code complete
- [x] Added **Set IVs**, **Set EVs** and **Set Nature** rows to
      `sDebugMenu_Actions_EditPokemon[]`
- [x] Reused the existing give-time widgets (`sIVsSelectionStep`, `sEVsSelectionStep`,
      `sNatureSelectionStep`) via the `DebugSelection` framework, following the
      `Set Friendship` pattern — no new UI code
- [x] Editors pre-fill with the mon's current values and call `CalculateMonStats()` after
- [x] "Set Nature" sets the **true** nature by stepping the personality in multiples of
      256: gender reads only `personality & 0xFF` so it is preserved exactly, and
      `256 % 25 == 6` is coprime with 25 so every nature is still reachable. Shininess is
      read before and re-asserted after, since its setter derives `shinyModifier` from the
      current personality.
- [x] The Mint-style "Set Hidden Nature" is left in place alongside it
- [x] EWRAM unchanged
- [ ] **Verify in emulator** — [TESTING.md](TESTING.md) Phase 5, especially 5.11 (gender
      never flips) and 5.12 (shininess preserved)

### Phase 6 — Randolocke parity: config-level
Cheapest parity items first (§4, Tier 1):
- [x] `ALL_TEACHABLES` teaching type ✅ forced in `make_teachables.py`
- [x] `P_SUMMARY_SCREEN_IV_EV_INFO` ✅ done in Phase 6a (F3)
- [x] Move relearner configs ✅
- [x] Instant text config ✅ (`TEXT_SPEED_FAST_MODIFIER = 18`)
- [x] Explicitly disable Mega / Primal / Dynamax / Gmax / Tera / Fusion ✅ — freed 1.8 MB ROM
- [x] Bag size ✅ (50/40/24)
- [x] Catch rates ✅ (`RANDOLOCKE_CATCH_RATE_PERCENT = 150`)
- [x] **F1** `I_REUSABLE_TMS = TRUE` ✅ landed early
- [x] **F2** `B_SHOW_TYPES = SHOW_TYPES_ALWAYS` ✅ landed early (revisit ALWAYS vs CAUGHT/SEEN after play-testing)
- [x] **F3** `P_SUMMARY_SCREEN_IV_EV_INFO = TRUE` + `_IV_EV_VALUES = TRUE` ✅ landed early

### Phase 7 — Randolocke parity: data & systems
- [x] 21-move level-up learnsets ✅ implemented as runtime randomization (`RANDOMIZE_LEARNSET`, flag `0x28`)
- [x] Level caps ✅ landed early (`B_EXP_CAP_TYPE`/`B_LEVEL_CAP_TYPE` + `sLevelCapFlagMap`)
- [x] Trainer party scaling to the caps ✅ `tools/randolocke/scale_trainers.py`
- [x] HM usability + forgettable HMs ✅ (`RANDOLOCKE_FIELD_MOVES_NEED_NO_USER`, `P_CAN_FORGET_HIDDEN_MOVE`)
- [x] Custom key items (infinite Repellant, Porta Heal, Endless Candy, Cap Candy) ✅
- [x] Field/gift item randomization wiring ✅ (done in Phase 3)
- [x] **F4** always-run ✅ `OW_AUTO_RUN`; holding B walks
- [x] **F5** second registered key item ✅ tap/hold SELECT (⚠️ save-layout change)
- [x] **F6** EVs in the move relearner ✅

### Phase 8 — Randolocke parity: maps & events — mostly superseded

**Most of this phase turned out to be unnecessary.** Randolocke's map and event additions
exist almost entirely to make every evolution reachable without trading. Phase 11's cheap
mart does that directly, and more completely, by stocking all 54 evolution items in every
Poke Mart:

| Randolocke did this | Covered by |
| --- | --- |
| Lilycove Dept. Store evolution-item sellers | ✅ the mart stocks all of them |
| Scroll of Darkness / Waters from the Mossdeep rock | ✅ both in the mart |
| Route 123 Bisharp event for the Leader's Crest | ✅ Leader's Crest in the mart |
| Mauville Game Corner Gimmighoul coins | ⚠️ partly — see below |
| Tutor Mansion in Lilycove | ✅ Phase 6's move relearner configs |
| Verdanturf Shelmet / Karrablast trades | ❌ still needed — see below |

- [x] Scorched Slab populated — 12 land slots (Numel / Slugma / Torkoal / Magcargo,
      levels 26-31) in `src/data/wild_encounters.json`; the map had no encounter table at all
- [x] Zweilous evolves at 63 rather than 64, which was above the 8-badge cap
- [x] Dark Void usable by every species (`B_DARK_VOID_FAIL = GEN_6`)
- [x] Evolution-item access, via Phase 11 rather than via new maps

#### Still genuinely outstanding

**Two real evolution gaps:**

1. **Gholdengo** needs 999 Gimmighoul Coins *in the bag*
   (`CONDITIONS({IF_BAG_ITEM_COUNT, ITEM_GIMMIGHOUL_COIN, 999})`). The mart sells them at
   200 each, so that is ~₽199,800 — reachable but a grind. Randolocke gave 999 free from an
   NPC. Options: drop the coin's price to 1, or add the NPC.
2. **Shelmet and Karrablast** use `EVO_TRADE` with each other specifically
   (`IF_TRADE_PARTNER_SPECIES`). Linking Cord does **not** satisfy that, so they remain
   unevolvable. This needs either the in-game trade NPCs Randolocke added, or a data change
   to give them an item-based evolution.

**Terrain changes — done, and Porymap turned out not to be needed:**

- Water added to Littleroot Town (pond at x 10-15, y 14-18), grass added to Oldale Town
  (two patches), both with matching entries in `src/data/wild_encounters.json`
- Old Rod sailor moved to Route 103 at (22, 11)

`map.bin` is a flat array of `metatile | collision << 10 | elevation << 12`, so the edits
are a few lines of Python. The part that genuinely needed an editor was *seeing* the
result, and `tools/randolocke/render_map.py` covers that: it composes the tileset's
metatiles and palettes and writes the layout out as a PNG. The pond's rim metatiles were
lifted from the enclosed pond on Route 123 rather than guessed.

**v1.1 NPC additions not done:** the Oldale 999-Ultra-Balls and ₽999,999 NPCs (the balls are
largely covered by the cheap mart), the Slateport legendary-location map seller, and the
post-Sootopolis Regi / Kyogre / Groudon unlocks.

### Phase 9 — Ship
- [ ] Full playthrough test to at least Gym 3
- [ ] Save/reload stability testing (randomization must be stable)
- [ ] Produce distribution patch against `baserom.gba`
- [ ] Credits: RHH, pret, tertu-m, Zetraphes, Istorian, PChal/Pointcrow

### Phase 11 — Cheap ball shop

The only Modern Emerald QoL feature azim wants that this repo does not already cover.

**Wanted:** a shop selling **Ultra Balls, Fast Balls and Timer Balls** *and every evolution
item* cheaply, stocked in ordinary Poke Marts.

Because these are buyable, both are deliberately demoted in the field-item randomizer
(Phase 10 tier 4, 0.30x uniform): finding one on the ground is not a reward when it is on
sale. The two decisions are linked - if the shop does not happen, tier 4 should go back up.

Modern Emerald puts ₽1 Ultra Balls plus evolution stones and trade-evolution items on
Lilycove Dept. Store 3F. That overlaps Randolocke's own planned Lilycove evolution-item
sellers (§4, Phase 8), so the two should be designed as one shop rather than two.

- [x] Location — all 12 general Poké Marts, available from the start
- [x] Ultra / Fast / Timer Balls at ₽200
- [x] All 54 evolution items and stones at ₽200
- [x] Fast Ball and Timer Ball confirmed present in 1.17
- [x] Lilycove sellers retired — the mart covers them

#### Explicitly declined from Modern Emerald

Reviewed and **not wanted**, recorded so they are not revisited:

| Feature | Why not |
| --- | --- |
| Nuzlocke modes (Easy/Normal/Hardcore) | Not wanted |
| Party size limit (1-5) | Not wanted |
| New Game options menu | Not wanted |
| Wonder Trade stations | Not wanted |
| Item drops from wild Pokemon | Not wanted |
| Custom type chart rebalance | 1.17's own balance is more faithful to modern generations |
| Survive Poison outside battle | Not wanted |
| Shiny colour variants, easier Feebas, Frontier bans | Modern Emerald flavour, no value here |

Already covered natively by 1.17 or by earlier phases: modern typings, Fairy, better stats,
extra legendaries, legendary abilities, new moves, nature mints, reusable TMs, opponent type
display, free TM/HM use, the Level Cap Candy and level caps.

### Phase 12 — Berry tree randomization ✅ code complete

Berries are excluded from field-item randomization (Phase 10) and randomized where they are
actually found instead: at berry trees.

- [x] Berry tiers for all **68** of 1.17's berries — `tools/randolocke/gen_berry_tiers.py`.
      The 43 hand-graded in pokeemerald_rando_enh are authoritative; the other 25 are placed
      by hold effect, the line the item data itself draws.
- [x] Hooked at `GetBerryTypeByBerryTreeId()` (`src/berry.c`), the single place a tree's
      berry is read, so the name shown, the count message and what lands in the bag agree.
- [x] Nuzlocke logic applied: in-battle HP restoration is near-worthless when the run carries
      a cheat heal item, so pinch-berry stat boosts (Salac, Liechi, Petaya) and Lum rank top,
      and the 22 berries with no hold effect at all sit at the bottom.
- [x] Gated by `RANDOMIZER_FLAG_BERRY_TREES` (`0x29`).
- [ ] **Verify in emulator** — a tree should be stable across visits, and two plots planted
      with different berries should differ.

Weights over 68 berries: 2.20x / 1.90x / 1.24x / 0.60x / 0.25x uniform.

Seeded on the tree id **and** what is planted in it, so a tree does not change when you
revisit, while replanting a different berry in the same plot gives a different result.

### Phase 10 — Tier-weighted randomization (ported from pokeemerald_rando_enh)

Port the tier systems from azim's other fork,
[`pokeemerald_rando_enh`](https://github.com/azimali322/pokeemerald_rando_enh) — a fork of
resetes12's **Modern Emerald** — so good moves, abilities and items come up more often
without eliminating variety.

#### 10.1 Why this is a port, not a copy

The two projects use **different randomizers**:

| | pokeemerald_rando_enh | randolocke-expanded |
| --- | --- | --- |
| Base | vanilla pokeemerald + Modern Emerald | pokeemerald-expansion 1.17 |
| Randomizer | TheXaman `tx_randomizer_and_challenges` | tertu `src/randomizer.c` |
| Species | 495 | **1,679** |
| Moves | 370 | **961** |
| Abilities | 82 | **319** |
| Items | 428 | **955** |

Fairy type and per-move physical/special `category` exist in both, so the *type model* is
compatible and the STAB-with-category-matching logic transfers. What does not transfer is
the integration: the tier code hooks tx_randomizer functions that do not exist here.

#### 10.2 Source of truth: the community tier list images

Copied into `docs/tiering/`:

| File | Size | Covers |
| --- | --- | --- |
| `community-moves-tierlist.png` | 2391x13271 | community-voted "All Pokemon Moves Tier List", ~19 submitted lists averaged |
| `community-abilities-tierlist.png` | 2353x4218 | community abilities list, tiers S/A/B/C/D/F/Negative |

**These images cover far more than the fork's C tables**, because those tables were capped
by Modern Emerald's content. Estimated coverage against *this* repo:

| Pool | Fork's C table | Community image (est.) | Against 1.17 |
| --- | --- | --- | --- |
| Moves | 349 | ~840 | 36% -> **~87%** |
| Abilities | 79 | ~258 | 25% -> **~81%** |

So the images, not the ported tables, are the right baseline. The fork's tables remain
useful as a cross-check for the Gen 1-3 overlap and for the nuzlocke overrides already
baked into them.

Items and berries have **no** community image; the fork's `ITEMS_BY_TIER.md` (122 items)
and `BERRIES.md` (43 berries) are the only source, covering 13% of 1.17's items.

#### 10.3 Transcription must be validated, not trusted

Reading ~840 move names and ~258 ability names out of two PNGs is error-prone. The method:

1. Transcribe band by band into `docs/tiering/*.md` worksheets.
2. **Validate every name against `include/constants/moves.h` / `abilities.h`** with a
   script that reports unmatched names, so a misread becomes a build-time list rather than
   a silently missing move.
3. Generate the C tables from the validated worksheets, never by hand.

#### 10.4 Injection points (all already exist)

| Tier system | Hook |
| --- | --- |
| Move tiers + STAB category matching | **`RzPickMoves()`** in `src/randomizer.c` — already filters by an `accept` callback and sorts by Base Power. A tier-weighted pick replaces the uniform `RandomizerNextRange` roll. |
| Ability tiers | **`RandomizeAbility()`** — currently uniform over `sRandomizerAbilityWhitelist[]`. |
| Item tiers + TM band | **`RandomizeFoundItem()`** — currently uniform over `sRandomizerItemWhitelist[]`. |
| Berry tiers | No berry-tree randomizer here yet — new work, lowest priority. |

Keep the fork's weighting maths: it computes a **per-entry rate** (`weight * 1000 / count`)
so a fat tier does not swamp a thin one, and exposes the weights as `#define`s.

#### 10.5 Coverage fallback (decide before implementing)

Whatever the images cover, some entries will fall outside every tier. With the fork's
current logic an untiered entry is **never rolled**, which would silently delete content.

Options: (a) extend the lists, (b) heuristic tier from Base Power / category / price,
(c) catch-all middle tier. Recommendation: (b) for moves and items, (a) for abilities —
ability quality has no numeric proxy.

#### 10.6 Nuzlocke heuristics to carry over

Already baked into the fork's tables and worth preserving as explicit overrides rather
than hand-edits:

- **Self-KO moves pushed to the bottom** (Explosion, Self-Destruct, Memento, Perish Song) —
  in a nuzlocke that is a permanently lost Pokemon, not a bad turn.
- **OHKO moves forced to the lowest move tier.**
- **Consumables devalued** — healing items, battle items and vitamins sit at the bottom,
  because the run uses a cheat heal item and sets EVs manually.

#### 10.7 Steps

- [x] Transcribe the abilities image — 316 abilities, 100% of the ROM's rollable set
- [x] Transcribe the moves image — 934 entries, 846 rollable, all tiered
- [x] Item tiers — 118 hand-graded plus 290 by `sortType` heuristic; berries split out to Phase 12
- [x] `RzWeightedPick()` wired into `RzPickMoves`, `RandomizeAbility`, `RandomizeFoundItem`,
      the TM band and `RandomizeBerryTree`
- [x] Weights exposed as `RZ_{ABILITY,MOVE,ITEM,TM,BERRY}_W_*` (x100) in `include/config/randomizer.h`
- [x] Off / Weighted / Strict per pool — `RZ_TIER_MODE_*`, Strict draws from the top
      `RZ_STRICT_TIERS` (2) bands
- [x] Tests in [TESTING.md](TESTING.md) — Phase 10, 21 cases

#### 10.9 How TM pickups randomize

`RandomizeFoundItem()` treats TMs as a closed set:

```c
if (IsItemTMHM(itemId))
    return RandomizerNextRange(&state, RANDOMIZER_MAX_TM - ITEM_TM01 + 1) + ITEM_TM01;
```

- A pickup that was a TM in vanilla becomes another TM; a pickup that was not can never
  become one, because the whitelist loop rejects `IsItemTMHM(result)`. The number of TM
  pickups in the game is therefore unchanged.
- HMs are never randomized: `ShouldRandomizeItem()` returns FALSE for them, so the code
  after that point can assume a TM.
- The seed is per-location (`mapGroup`, `mapNum`, `localId` plus the original item), and
  each location rolls **independently**, so the same TM can appear in several places.
  This is already **with replacement**, which is what azim wants given `I_REUSABLE_TMS`.

**On raising the TM cap:** investigated and **deliberately not done**. `ITEM_TM100` exists
as a constant, but `FOREACH_TM` in `include/constants/tms_hms.h` defines only **50 real
TMs**. `ITEM_TM51`..`ITEM_TM100` are unassigned placeholders - `.name = "TM51"`,
`.description = "?????" // Todo` - with no move attached. Raising `RANDOMIZER_MAX_TM` would
hand the player 51 TMs that teach nothing. It stays at `ITEM_TM50`.

**Implemented since:**

- **TM drops are tier-weighted.** `tools/randolocke/gen_tm_tiers.py` groups the 50 TMs by
  the tier of the move each teaches (1 Meta Defining, 5 Staples, 19 Filler, 15 Niche,
  9 Bad, 1 Homeless) and the drop reuses the move weights, so a good TM is about as likely
  as a good move.
- **Ordinary pickups can now become TMs.** Previously TMs and items were closed sets - a
  TM became a TM, and nothing else ever became one. A non-TM pickup now rolls into the TM
  band `RZ_ITEM_W_TM_BAND` (30%) or the item tiers.

**Not implemented:** randomizing *what each TM teaches*. TM26 still teaches Earthquake.
That is the deferred v1.1 item in 10.1, and it is the piece that needs the build-time
teachables pipeline.

#### 10.8 Open decisions for azim

1. Tier **weights** per pool (the fork's: moves 4/24/38/27/6, abilities 40/30/22/6).
2. Whether to ship **Strict** mode (top tiers only) as well as Weighted.
3. Which additional moves/abilities the **nuzlocke heuristics** should push down.
4. Whether the 33 MB moves PNG stays in git history or gets downscaled first.

### v1.1 items — done

- [x] **Strict mode** per pool
- [x] **Gimmighoul Coin at ₽1**, so Gholdengo's 999-coin requirement costs ₽999
- [x] **Shelmet / Karrablast** gained a Linking Cord evolution alongside the trade
- [x] **Forced nickname on catch** (`RANDOLOCKE_FORCE_NICKNAME`)
- [x] **Bag disabled in trainer battles** — `B_VAR_NO_BAG_USE` points at `VAR_UNUSED_0x40F7`;
      set it to 1 for trainer battles, 2 to include wild
- [x] **Kyogre / Groudon after Sootopolis** — needed no change. `FLAG_SYS_WEATHER_CTRL` is
      already set during the Rayquaza scene at Sky Pillar, which is exactly the point
      Randolocke describes
- [x] **Regi caves after Sootopolis** — done better, via Flash (see the Regi section)
- [x] Increased catch rates, reusable TMs, level caps, key items — earlier phases

#### Still not done, and why

- ~~**Slateport legendary-location map seller**~~ — **done.** No new items were needed: the
  "maps" are the four event tickets (Eon → Southern Island, Old Sea Map → Faraway Island,
  Aurora → Birth Island, Mystic → Navel Rock), which already exist and are otherwise
  Mystery-Gift-only. Object placement turned out not to need Porymap — `map.json` is plain
  text and the free tiles were read out of the layout's collision data.
- ~~**Oldale ₽999,999 NPC**~~ — **done**, one-time.
- **Oldale 999 Ultra Balls NPC** — not done, and not wanted: the Phase 11 cheap mart covers it.
- ~~**Terrain**~~ — **done.** See the terrain section above; no Porymap was needed.

### Later — v1.1
- [ ] TM / tutor / move randomization against the build-time teachables pipeline (§5.1)
- [ ] Reconsider the deferred option toggles

---

## 8. Credits to carry

Per RHH and Randolocke conventions:

- RHH (Rom Hacking Hideout) — pokeemerald-expansion
- pret — pokeemerald decompilation
- **tertu-m** — the randomizer implementation
- **Zetraphes** — the `tertu-randomizer` fork
- **Istorian** — Pokémon Randolocke, the design this recreates
- PChal and Pointcrow — original tournament inspiration
