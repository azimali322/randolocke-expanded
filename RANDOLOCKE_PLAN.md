# Randolocke-Expanded — Build Plan

**Owner:** azimali322
**Repo:** `randolocke-expanded` (fork of RHH pokeemerald-expansion)
**Plan created:** 2026-09-09
**Status:** Phases 0-3 code complete (+ QoL configs from Phase 6); Phase 3 awaiting in-game verification

---

## 1. Goals

Recreate **Pokémon Randolocke v1.1** (by Istorian) as my own romhack, with three enhancements:

1. **Abilities do not randomize on evolution.** An option where a Pokémon's randomized
   ability stays stable across its evolution family, instead of rerolling when it evolves.
2. **Manual IV/EV editing.** A cheat/option toggle to manually adjust IVs and EVs, in the
   style of modern Emerald romhacks.
3. **Full Randolocke v1.1 parity.** All features from Randolocke v1.1 present in my romhack.
4. **Nature editing.** The ability to adjust a Pokémon's nature.
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
- [ ] Every Pokémon learns 21 moves at fixed levels (7 STAB / 7 status / 7 damaging)
- [ ] All Pokémon can learn every move (`ALL_TEACHABLES`)
- [x] Hard level caps per badge: 14 / 21 / 24 / 29 / 36 / 43 / 47 / 50 / 63, E4 -> 100 ✅ landed early via 1.17's built-in cap system
- [ ] Trainer parties scaled to the level caps
- [ ] Upgraded opponent AI (likely config on 1.17)

### Quality of life
- [ ] HMs usable out of battle without a party member knowing the move
- [ ] HMs forgettable like normal moves
- [x] IVs visible in the summary screen (`P_SUMMARY_SCREEN_IV_EV_INFO`) ✅ done via F3
- [ ] Expanded bag size
- [x] Key items: infinite Repellant, Porta Heal, Endless Candy, Cap Candy ✅ landed early
      *(Cap Candy takes the soonest of next cap / next level-up move / next level evolution.
      Distinct from `B_RARE_CANDY_CAP`, which only stops normal Rare Candy exceeding the cap.)*
- [ ] Press R to throw a ball

### Map & event changes
- [ ] Old Rod sailor moved to Route 103
- [ ] Scorched Slab populated with Pokémon
- [ ] Water added to Littleroot, grass added to Oldale
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
- [ ] Oldale NPC gifting $999,999
- [ ] Increased wild catch rates
- [ ] Slateport NPC selling legendary location maps (Latios/Latias, Mew, Deoxys, Ho-oh/Lugia)
- [ ] Regi caves unlocked post-Sootopolis
- [ ] Kyogre/Groudon Weather Institute events post-Sootopolis
- [ ] Zweilous evolves at 63 (before E4)
- [ ] Dark Void usable by all species

### v1.1 option toggles
- [ ] Don't randomize abilities *(nearly free — tertu flag)*
- [ ] Higher-Base-Power moves learned later (default on)
- [x] Porta Heal does not revive by default ✅ (`RANDOLOCKE_PORTA_HEAL_REVIVES`)
- [ ] ~~Don't randomize movesets~~ — deferred with move randomization
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

### Phase 4 — Enhancement 1: ability stability across evolution
- [ ] Add `RZ_ABILITY_FOLLOWS_EVOLUTION` config
- [ ] Seed `RandomizeAbility` from the evolution-family root
- [ ] Test: catch a mon, note ability, evolve, confirm ability unchanged; confirm the
      opposite with the config off

### Phase 5 — Enhancements 2 and 4: IV/EV and nature editors
- [ ] Add IV and EV editor rows to `sDebugMenu_Actions_EditPokemon[]`
- [ ] Reuse give-time digit-input widgets (`sNatureSelectionStep` already exists for nature)
- [ ] Add "Set Nature (true)" using the +256 stepping method (Enhancement 4, Option B),
      preserving gender exactly and re-setting `shinyModifier` to preserve shininess
- [ ] Leave the existing "Set Hidden Nature" (Mint-style, Option A) in place alongside it
- [ ] Check EWRAM after (§6)
- [ ] Test: edit IVs/EVs on a live party mon, confirm stats recalculate and persist
- [ ] Test: set every nature on a gender-ratio-boundary species and confirm gender never flips

### Phase 6 — Randolocke parity: config-level
Cheapest parity items first (§4, Tier 1):
- [ ] `ALL_TEACHABLES` teaching type
- [x] `P_SUMMARY_SCREEN_IV_EV_INFO` ✅ done in Phase 6a (F3)
- [ ] Move relearner configs
- [ ] Instant text config
- [ ] Explicitly disable Mega / Primal / Dynamax / Gmax / Tera / Fusion / Z-Moves
      (including 1.17's new Z-A Megas)
- [ ] Bag size, catch rates
- [x] **F1** `I_REUSABLE_TMS = TRUE` ✅ landed early
- [x] **F2** `B_SHOW_TYPES = SHOW_TYPES_ALWAYS` ✅ landed early (revisit ALWAYS vs CAUGHT/SEEN after play-testing)
- [x] **F3** `P_SUMMARY_SCREEN_IV_EV_INFO = TRUE` + `_IV_EV_VALUES = TRUE` ✅ landed early

### Phase 7 — Randolocke parity: data & systems
- [ ] 21-move level-up learnsets (format is identical between versions — generator output
      drops in)
- [x] Level caps ✅ landed early (`B_EXP_CAP_TYPE`/`B_LEVEL_CAP_TYPE` + `sLevelCapFlagMap`)
- [ ] Trainer party scaling to the caps
- [ ] HM usability + forgettable HMs
- [x] Custom key items (infinite Repellant, Porta Heal, Endless Candy, Cap Candy) ✅
- [ ] Field/gift item randomization wiring
- [ ] **F4** always-run: add `OW_AUTO_RUN`, invert the `B_BUTTON` dash gate
- [ ] **F5** second registered key item (⚠️ save-layout change — batch with other save work)
- [ ] **F6** show EVs in the move-learning screen

### Phase 8 — Randolocke parity: maps & events
- [ ] All map changes and NPC/event additions from §4
- [ ] v1.1 NPC additions and legendary unlocks

### Phase 9 — Ship
- [ ] Full playthrough test to at least Gym 3
- [ ] Save/reload stability testing (randomization must be stable)
- [ ] Produce distribution patch against `baserom.gba`
- [ ] Credits: RHH, pret, tertu-m, Zetraphes, Istorian, PChal/Pointcrow

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
