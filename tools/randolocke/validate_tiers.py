#!/usr/bin/env python3
"""Resolve tier-worksheet display names to real constants, and report what does not match.

Transcribing a tier list out of a PNG is error-prone, so nothing is trusted: every name in
docs/tiering/*_BY_TIER.md is resolved against the ROM's own constant headers, and anything
unresolved is printed rather than silently dropped.

Usage:  python3 tools/randolocke/validate_tiers.py [--quiet]
Exit:   0 if every name resolved, 1 otherwise.
"""
from __future__ import annotations   # PEP 585 generics on Python 3.8 (conda ships 3.8 here)

import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]

# Errors in the source images, and names whose constant differs from the display text.
ALIASES = {
    "AERIALATE": "AERILATE",          # misspelled on the tier list
    "WONDERSKIN": "WONDER_SKIN",      # rendered without the space
    "MAGMA_VEIL": "MAGMA_ARMOR",      # no such ability; the icon is Slugma's
    "AS_ONE": "AS_ONE_ICE_RIDER",     # split into two constants; Ice Rider stands in
    "GRASSY_PELT": "GRASS_PELT",      # the ability is Grass Pelt
    "X_SCIZZOR": "X_SCISSOR",         # misspelled on the moves list
}

# Never rolled: Wonder Guard trivialises or bricks a randomized fight; NONE is not an
# ability; 314 and 317 are unnamed placeholder slots in the expansion.
EXCLUDED = {"WONDER_GUARD", "NONE", "314", "317"}

# Moves never rolled. STRUGGLE is the game's fallback move, not a real option; everything
# from FIRST_Z_MOVE onward is a Z, Max or G-Max move, and Phase 6 disabled Gigantamax while
# Randolocke lists Z-moves as unavailable, so those are inert here.
MOVES_EXCLUDED = {"STRUGGLE", "NONE"}

# Pushed to the bottom tier for nuzlocke play regardless of community placement: a move that
# KOs its own user costs a permanently dead Pokemon, not a turn. Carried over from the
# pokeemerald_rando_enh tables, which the community list is not aware of.
MOVES_PUSHDOWN = {
    "EXPLOSION", "SELF_DESTRUCT", "MEMENTO", "MISTY_EXPLOSION", "FINAL_GAMBIT",
    "HEALING_WISH", "LUNAR_DANCE",                    # self-KO
    "FISSURE", "GUILLOTINE", "HORN_DRILL", "SHEER_COLD",  # OHKO
}


def z_and_max_moves() -> set[str]:
    """Every MOVE_ constant declared at or after FIRST_Z_MOVE."""
    text = (ROOT / "include/constants/moves.h").read_text().split("\n")
    start = next(i for i, l in enumerate(text) if "FIRST_Z_MOVE" in l)
    out = set()
    for line in text[start:]:
        m = re.match(r"\s*MOVE_([A-Z0-9_]+)", line)
        if m:
            out.add(m.group(1))
    return out


def constants(header: str, prefix: str) -> set[str]:
    text = (ROOT / header).read_text()
    return set(re.findall(rf"\b{prefix}([A-Z0-9_]+)\b", text))


def normalise(name: str) -> str:
    n = name.strip().upper()
    n = n.replace("(", "").replace(")", "")   # "As One (Shadow Rider)"
    n = n.replace("'", "").replace("-", "_").replace(".", "")
    n = re.sub(r"\s+", "_", n)
    return ALIASES.get(n, n)


def parse(worksheet: Path) -> dict[str, list[str]]:
    out = {}
    text = worksheet.read_text()
    for m in re.finditer(r"^## (.+?)\n(.*?)(?=\n## |\Z)", text, re.S | re.M):
        tier = m.group(1).strip()
        names = [x.strip() for x in m.group(2).replace("\n", " ").split(",") if x.strip()]
        out[tier] = names
    return out


POOLS = {
    "abilities": ("include/constants/abilities.h", "ABILITY_",
                  "docs/tiering/ABILITIES_BY_TIER.md"),
    "moves": ("include/constants/moves.h", "MOVE_",
              "docs/tiering/MOVES_BY_TIER_RANDOLOCKE.md"),
}


def main() -> int:
    quiet = "--quiet" in sys.argv
    which = "moves" if "--moves" in sys.argv else "abilities"
    header, prefix, sheet = POOLS[which]
    valid = constants(header, prefix)
    tiers = parse(ROOT / sheet)
    excluded = EXCLUDED if which == "abilities" else (MOVES_EXCLUDED | z_and_max_moves())

    bad, seen, total = [], {}, 0
    for tier, names in tiers.items():
        for name in names:
            total += 1
            c = normalise(name)
            if c not in valid:
                bad.append((tier, name, c))
            elif c in seen:
                bad.append((tier, name, f"{c} (already in {seen[c]})"))
            else:
                seen[c] = tier

    if not quiet:
        for tier in tiers:
            print(f"  {tier:<10} {len(tiers[tier]):>3}")
        print(f"  {'TOTAL':<10} {total:>3}   resolved {len(seen)}   unresolved {len(bad)}")
        roll = len([c for c in seen if c not in excluded])
        pool = len(valid - excluded)
        print(f"  rollable: {roll}/{pool} {which} ({100*roll//max(1,pool)}%)   "
              f"{len(seen)-roll} tiered but excluded")

    if bad:
        print("\nUNRESOLVED — fix the worksheet or add an alias:")
        for tier, name, c in bad:
            print(f"  [{tier}] {name!r} -> {prefix}{c}")
        return 1
    print("\nAll names resolved.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
