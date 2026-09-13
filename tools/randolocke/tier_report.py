#!/usr/bin/env python3
"""Report the selection odds each tier's weights actually produce.

A tier's weight is shared across its members, so a big tier dilutes itself. This prints the
per-entry chance and how far it lands from a uniform draw, and warns when a lower tier ends
up with better odds per entry than a higher one -- which is easy to do by accident.

Usage:  python3 tools/randolocke/tier_report.py [--weights S=11,A=20,...]
"""
from __future__ import annotations

import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from validate_tiers import ROOT, constants, normalise, parse, EXCLUDED  # noqa: E402

ORDER = ["S", "A", "B", "C", "D", "F", "Negative"]

# Weight 0 means never rolled.
DEFAULT_WEIGHTS = {"S": 9, "A": 20, "B": 32, "C": 24, "D": 11, "F": 4, "Negative": 0}


def counts() -> dict[str, int]:
    valid = constants("include/constants/abilities.h", "ABILITY_")
    tiers = parse(ROOT / "docs/tiering/ABILITIES_BY_TIER.md")
    out = {}
    for tier in ORDER:
        n = 0
        for name in tiers.get(tier, []):
            c = normalise(name)
            if c in valid and c not in EXCLUDED:
                n += 1
        out[tier] = n
    return out


def main() -> int:
    weights = dict(DEFAULT_WEIGHTS)
    for arg in sys.argv[1:]:
        if arg.startswith("--weights"):
            for pair in arg.split("=", 1)[1].split(","):
                k, v = pair.split("=") if "=" in pair else (pair, "0")
                weights[k.strip()] = float(v)

    n = counts()
    pool = sum(c for t, c in n.items() if weights.get(t, 0) > 0)
    total_w = sum(weights.get(t, 0) for t in ORDER)
    uniform = 100.0 / pool

    print(f"selectable pool: {pool} abilities     uniform draw: {uniform:.4f}% each")
    print(f"weights sum to {total_w:g}\n")
    print(f"{'tier':<9}{'count':>6}{'weight':>8}{'per-ability':>13}{'vs uniform':>12}   {'share of pool':>13}")
    print("-" * 66)

    rates = {}
    for t in ORDER:
        c, w = n[t], weights.get(t, 0)
        if c == 0:
            continue
        if w == 0:
            print(f"{t:<9}{c:>6}{'0':>8}{'never':>13}{'--':>12}   {'excluded':>13}")
            continue
        per = (w / total_w * 100) / c
        rates[t] = per
        print(f"{t:<9}{c:>6}{w:>7g}%{per:>12.4f}%{per/uniform:>11.2f}x   {c/pool*100:>12.1f}%")

    order = [t for t in ORDER if t in rates]
    bad = [(a, b) for a, b in zip(order, order[1:]) if rates[b] > rates[a]]
    if bad:
        print("\n  WARNING -- a lower tier draws better than a higher one:")
        for a, b in bad:
            print(f"    {b} ({rates[b]:.4f}%) beats {a} ({rates[a]:.4f}%) per ability")
        print("    Raise the higher tier's weight, or merge them.")
        return 1
    print("\n  per-ability odds decrease monotonically down the tiers.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
