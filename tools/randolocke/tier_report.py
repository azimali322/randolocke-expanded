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
from validate_tiers import (ROOT, constants, normalise, parse, EXCLUDED,  # noqa: E402
                            MOVES_EXCLUDED, MOVES_PUSHDOWN, POOLS, z_and_max_moves)

ORDERS = {
    "abilities": ["S", "A", "B", "C", "D", "F", "Negative"],
    "moves": ["Meta Defining", "Staples", "Filler/Outclassed", "Niche", "Bad",
              "Pokemon Homeless"],
}

# Weight 0 means never rolled.
DEFAULTS = {
    "abilities": {"S": 9, "A": 20, "B": 32, "C": 24, "D": 11, "F": 4, "Negative": 0},
    # Deliberately compressed at the top (1.5x down to 1.2x) and steep at the bottom:
    # good moves should be a little more likely, bad ones markedly rarer.
    "moves": {"Meta Defining": 1.18, "Staples": 10.87,
              "Filler/Outclassed": 41.99, "Niche": 40.32,
              "Bad": 5.13, "Pokemon Homeless": 0.51},
}


def counts(which: str) -> dict[str, int]:
    header, prefix, sheet = POOLS[which]
    valid = constants(header, prefix)
    order = ORDERS[which]
    tiers = parse(ROOT / sheet, order)
    excluded = EXCLUDED if which == "abilities" else (MOVES_EXCLUDED | z_and_max_moves())
    bottom = order[-1]

    out = {t: 0 for t in order}
    for tier in order:
        for name in tiers.get(tier, []):
            c = normalise(name)
            if c not in valid or c in excluded:
                continue
            # nuzlocke: self-KO and OHKO moves drop to the bottom band
            dest = bottom if (which == "moves" and c in MOVES_PUSHDOWN) else tier
            out[dest] += 1
    return out


def main() -> int:
    which = "moves" if "--moves" in sys.argv else "abilities"
    order = ORDERS[which]
    weights = dict(DEFAULTS[which])
    for arg in sys.argv[1:]:
        if arg.startswith("--weights"):
            for pair in arg.split("=", 1)[1].split(","):
                k, v = pair.split("=") if "=" in pair else (pair, "0")
                weights[k.strip()] = float(v)

    n = counts(which)
    pool = sum(c for t, c in n.items() if weights.get(t, 0) > 0)
    total_w = sum(weights.get(t, 0) for t in order)
    uniform = 100.0 / pool

    print(f"selectable pool: {pool} {which}     uniform draw: {uniform:.4f}% each")
    print(f"weights sum to {total_w:g}\n")
    print(f"{'tier':<20}{'count':>6}{'weight':>8}{'per-entry':>13}{'vs uniform':>12}   {'share of pool':>13}")
    print("-" * 66)

    rates = {}
    for t in order:
        c, w = n[t], weights.get(t, 0)
        if c == 0:
            continue
        if w == 0:
            print(f"{t:<20}{c:>6}{'0':>8}{'never':>13}{'--':>12}   {'excluded':>13}")
            continue
        per = (w / total_w * 100) / c
        rates[t] = per
        print(f"{t:<20}{c:>6}{w:>7g}%{per:>12.4f}%{per/uniform:>11.2f}x   {c/pool*100:>12.1f}%")

    order = [t for t in order if t in rates]
    bad = [(a, b) for a, b in zip(order, order[1:]) if rates[b] > rates[a]]
    if bad:
        print("\n  WARNING -- a lower tier draws better than a higher one:")
        for a, b in bad:
            print(f"    {b} ({rates[b]:.4f}%) beats {a} ({rates[a]:.4f}%) per entry")
        print("    Raise the higher tier's weight, or merge them.")
        return 1
    print("\n  per-entry odds decrease monotonically down the tiers.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
