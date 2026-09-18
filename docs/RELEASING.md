# Cutting a release

## 1. Build

Two ROMs, from the same source:

```bash
make -j8            # -> pokeemerald.gba          debug menu ON  (R + START)
make release -j8    # -> pokeemerald-release.gba  debug menu OFF
```

`DEBUG_OVERWORLD_MENU` is `DISABLED_ON_RELEASE`, and that is the only difference that
matters. Ship the release build; ship the debug build alongside it only while the hack is
still being tested, and label it clearly.

Because the release build has no debug menu, **a player cannot switch any randomizer
feature on**. That is what `RANDOLOCKE_RANDOMIZE_ON_NEW_GAME` exists for — see
`docs/SETTINGS.md` §1. Test T14.9 covers exactly this and is worth running before every
release.

## 2. Patch

`tools/randolocke/make_bps.py` writes BPS patches, so no Flips install is needed:

```bash
python3 tools/randolocke/make_bps.py create baserom.gba pokeemerald-release.gba \
    dist/randolocke-expanded-<version>.bps
```

It verifies its own output by applying the patch back and comparing, and refuses to write
a patch that does not reproduce the ROM. To check one independently afterwards:

```bash
python3 tools/randolocke/make_bps.py verify baserom.gba dist/<patch>.bps pokeemerald-release.gba
```

Expect roughly 17–18 MB, about 53% of the ROM. That is normal: the ROM grew from 16 MB to
32 MB and most of the new bytes are Gen 1–9 sprites, cries and data that are not in vanilla
Emerald at all, so there is nothing for a delta encoder to reference. The encoder does find
about 5.7 MB of relocated vanilla data, and run-length-encodes the ~10 MB padding tail down
to nothing. Zipping saves a further ~4 MB.

## 3. Never ship

- `baserom.gba`, `pokeemerald.gba`, `pokeemerald-release.gba`, or any other `.gba`
- Anything under `dist/` except the `.bps` files and `README.txt`

`dist/` is gitignored precisely so a ROM cannot be committed by accident.

## 4. Reference checksums

| File | Size | CRC32 |
| --- | --- | --- |
| Vanilla Emerald (US) — the only valid base | 16,777,216 | `1f1c08fb` |
| 0.9.1 release | 33,554,432 | `9d3b03d1` |
| 0.9.1 test build | 33,554,432 | `eb51e720` |
| 0.9.0 release | 33,554,432 | `c165e622` |
| 0.9.0 test build | 33,554,432 | `359e5b0f` |

## 5. Before calling it 1.0

- [ ] A playthrough to at least Gym 3 on the **release** build
- [ ] Save, soft reset, reload — randomization must come back identical
- [ ] §R regression rows in `TESTING.md`
- [ ] Apply the `.bps` to a *fresh* copy of vanilla and confirm the CRC32 above
