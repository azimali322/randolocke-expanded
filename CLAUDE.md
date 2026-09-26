# Randolocke Expanded — notes for agents

A romhack of **RHH pokeemerald-expansion 1.17** plus tertu's randomizer, recreating
Randolocke v1.1 (hard level caps, nuzlocke rules, tier-weighted randomization, tougher
trainers). **Emerald only**: the FireRed and LeafGreen targets do not build here.

- Plan and history: `RANDOLOCKE_PLAN.md`. Every setting: `docs/SETTINGS.md`.
- Manual test plan, one section per phase, newest first after §F: `TESTING.md`.
- Hack settings: `include/config/randolocke.h` and `include/config/randomizer.h`.
- Hack code is marked with `randolocke:` comments where it touches upstream files.

## Build and test

| Command | What |
| --- | --- |
| `make -j$(nproc)` | the ROM, `pokeemerald.gba` |
| `make -j$(nproc) release` | the ROM players get: `RELEASE`, `NDEBUG`, LTO, no debug menus |
| `make -j$(nproc) check` | every test, expansion's and the hack's. About 16 minutes on 4 cores (6 to build, 10 to run); about 25 in CI |
| `make check TESTS="Randolocke"` | the hack's own tests only — about 15 seconds once the test ROM is built |

`TESTS` takes one pattern per run: a test-name prefix (`TESTS="Capture"`), `*infix`, or an
exact file (`TESTS="test/randolocke_boss_moves.c"`). The test ROM relinks on every
`make check`, so several runs with different patterns are cheap.

Name every test of the hack's own behaviour `"Randolocke: ..."` so the prefix run finds it.

## CI (`.github/workflows/build.yml`)

On every pull request and every push to `master`:

| Job | What it proves |
| --- | --- |
| `build-emerald` | the ROM compiles (`UNUSED_ERROR` and `DEPRECATED_ERROR` on) |
| `release` | the release build compiles and links with LTO |
| `test` | the whole `make check` suite passes |
| `checks` | generated tier tables match their sources; tier worksheets resolve; borrowed flags/vars are each used once, by name |
| `docs_validate` | `docs/SUMMARY.md` lists every doc under `docs/` |
| `build` | the gate: runs always, fails if anything above failed. Require this one in branch protection |

Deliberately gone, and to be kept out when merging an update from expansion: the FireRed
and LeafGreen jobs, the `allcontributors` jobs, `labels.yml` (expansion's PR label rules)
and `docs.yml` (published expansion's docs to GitHub Pages, which this repo does not use —
restore it from history if a docs site is ever wanted).

## When a test fails: the rules

The test job was once disabled because the suite "asserts vanilla behaviour and can never
go green". That was wrong: of 58 failures, 43 were one real bug, one was a real text bug and
one was the hack's own test timing out; only 13 were the hack's intended changes, and each
of those had a fix that kept the test running. Triage every failure by its cause:

1. **The test found a bug → fix the code.** 43 AI tests failed because the AI tiers were
   added to *every* battle type, including the recorded battles the test runner plays and
   the Battle Frontier. The fix was one line in `GetAiFlags`. The item-description test
   caught the Cap Candy's text overflowing the Bag by one pixel.
2. **The test pins upstream's numbers → re-pin them to the hack's.** `test/save.c` holds
   the save-structure sizes. The hack's `SaveBlock1` is 13920 bytes, not expansion's 15568.
   Re-pinned, the test now guards players' saves against *accidental* layout changes —
   change the number only on purpose, knowing old saves stop loading.
3. **The hack changes the mechanic on purpose → gate only the assertions that check the
   vanilla value**, on the setting that changes it, and leave the rest of the test running:
   ```c
   #if RANDOLOCKE_PLAYER_IVS == RANDOLOCKE_IVS_VANILLA
       EXPECT_EQ(GetMonData(mon, MON_DATA_HP_IV), 7);
   #endif
   ```
   The checks come back by themselves if the setting is ever turned off. Then make sure a
   `Randolocke:` test covers the new behaviour (here, `test/randolocke_player_ivs.c`).
   Examples: `test/battle/trainer_control.c`, `test/pokemon.c`, `test/battle/capture.c`
   (which derives its expectation from the configured catch rate instead).
4. **A rule makes the test's setup impossible, but the test is about something else → put
   the setup back, or lift the rule for that test.** The daycare IV test restores the
   parents' IVs after `givemon`; the experience tests set every badge and the Champion flag
   so the level cap is `MAX_LEVEL`.
5. **Never** disable the test job, delete a test, or skip one to get green.
   `KNOWN_FAILING` is only for a real bug being fixed later — never for intended behaviour —
   and it fails the run once the test starts passing.
6. `ASSUME(...)` failures count as **skips** on this fork (`TEST_SKIP_IS_FAIL` is only on
   for rh-hideout), so an `ASSUME` quietly removes coverage. Tie it to a setting, as above.
7. **Watch for tests that pass by comparing nothing.** Under the level caps, experience
   tests compared 0 with 0 and passed. A pass is not proof the test ran what it says.

## Test-build facts that bite

- **The trainer table is replaced by the test fixtures** (`test/battle/trainer_control.party`).
  Real ids like `TRAINER_ROXANNE_1` read an empty or unrelated fixture. Define `struct Trainer`
  values locally (see `test/randolocke_trainers.c`) and pass a real id only as a seed.
- **Every battle test is played back as a recorded battle** (`BATTLE_TYPE_RECORDED`) against
  fixture trainer 2, `TRAINER_LEAF_TEST`, whose class is **Rival**. Anything keyed on battle
  type or trainer class reaches the tests through that.
- `CreateMon` leaves current HP at 0: set `MON_DATA_HP` for a Pokémon meant to be alive.
- Flags and vars are cleared before each test; **the PC is not** (`ResetPokemonStorageSystem()`),
  and nor are static/EWRAM caches — key a cache by the seed as well as the species.
- `FLAG_SET` in a battle test takes one flag per test; call `FlagSet` directly for more.
- `Test_MgbaPrintf` has no `%x`. `EXPECT_EQ` prints with `%d`: compare 64-bit AI flags with
  `EXPECT((a & b) == b)`.
- **The timeout is 60 emulated seconds without finishing a parametrization.** Dealing one
  randomized learnset takes about 0.1 s, so a sweep over every species must be split with
  `PARAMETRIZE` (see `test/randolocke_relearner.c`) or sampled
  (`test/randolocke_boss_moves.c`). Emulated time is deterministic, but it differs between
  compilers: a test near the limit can pass on a Mac and time out in CI.
- `BATTLE_TYPE_FRONTIER` is every facility at once, the Factory included; set one facility.

## Adding to the hack

- **A new setting**: add it to `docs/SETTINGS.md` and cover it with a `Randolocke:` test.
- **A borrowed flag or var** (`FLAG_UNUSED_*`, `VAR_UNUSED_*`): add it to
  `test/randolocke_flags.c`. `tools/randolocke/check_borrowed_flags.py` (run in CI) catches
  two settings on one, or a script using one by its raw name.
- **Save data**: a new field changes `test/save.c`'s sizes and breaks existing saves. Batch
  such changes and say "requires a new game" in `TESTING.md`.
- **Tier tables** (`src/data/randomizer/*_tiers.h`) are generated: edit the worksheets in
  `docs/tiering/` and rerun `tools/randolocke/gen_*_tiers.py`; CI fails on a stale table.
  `gen_item_tiers.py` and `gen_berry_tiers.py` read a local copy of another fork and cannot
  run in CI.
- Record each change as a phase in `TESTING.md`: what changed, manual steps, and the
  regression commands with their pass counts.

## Merging an update from expansion

Expect conflicts in `build.yml` and in upstream tests carrying `randolocke:` gates; keep both
sides' intent. Then run the **full** `make check`, not only the `Randolocke` prefix: new
upstream tests are exactly where the hack's changes show up. Triage what fails with the
rules above.
