#!/usr/bin/env python3
"""Rescale trainer Pokemon levels to Randolocke's badge caps, and tag boss trainers.

Vanilla Emerald's trainer levels already encode progression, and upstream's default
level-cap table (15/19/24/29/31/33/42/46/58) tracks vanilla gym-leader levels, so we
remap piecewise-linearly from those anchors onto Randolocke's caps.

Levels above the Champion anchor (post-game, Frontier, rematches) are left untouched.

Usage:  python3 tools/randolocke/scale_trainers.py [--dry-run]
"""
import re
import sys
from pathlib import Path

PARTY = Path("src/data/trainers.party")

# (vanilla anchor, randolocke cap)
ANCHORS = [(0, 0), (15, 14), (19, 21), (24, 24), (29, 29),
           (31, 36), (33, 43), (42, 47), (46, 50), (58, 63)]

# Classes treated as bosses. Tagging is purely a label; whether bosses are actually
# exempted from randomization is decided at runtime by RZ_RANDOMIZE_BOSS_TRAINERS.
BOSS_CLASSES = {"Leader", "Elite Four", "Champion",
                "Magma Leader", "Aqua Leader", "Magma Admin", "Aqua Admin"}


def remap(level: int) -> int:
    """Piecewise-linear remap of a vanilla level onto the Randolocke caps."""
    if level > ANCHORS[-1][0]:
        return level                      # post-game: leave alone
    for i in range(1, len(ANCHORS)):
        v0, r0 = ANCHORS[i - 1]
        v1, r1 = ANCHORS[i]
        if level <= v1:
            if v1 == v0:
                return r1
            frac = (level - v0) / (v1 - v0)
            return max(2, min(100, round(r0 + frac * (r1 - r0))))
    return level


def main() -> int:
    dry = "--dry-run" in sys.argv
    text = PARTY.read_text()
    lines = text.split("\n")

    out, changed, tagged = [], 0, 0
    cur_class = None
    for line in lines:
        if line.startswith("=== TRAINER_"):
            cur_class = None
        elif line.startswith("Class: "):
            cur_class = line[len("Class: "):].strip()

        m = re.fullmatch(r"Level: (\d+)", line)
        if m:
            old = int(m.group(1))
            new = remap(old)
            if new != old:
                changed += 1
            out.append(f"Level: {new}")
            continue

        out.append(line)

        # Tag bosses right after their Class line, if not already tagged.
        if line.startswith("Class: ") and cur_class in BOSS_CLASSES:
            out.append("Boss: Yes")
            tagged += 1

    if "Boss: Yes" in text:
        print("Refusing to run: trainers.party already contains Boss tags.")
        return 1

    print(f"levels changed: {changed}, bosses tagged: {tagged}")
    for sample in (12, 17, 26, 31, 33, 40, 46, 58, 63, 78):
        print(f"   vanilla {sample:>3} -> {remap(sample):>3}")
    if not dry:
        PARTY.write_text("\n".join(out))
        print(f"wrote {PARTY}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
