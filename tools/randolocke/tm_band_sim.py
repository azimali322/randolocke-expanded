#!/usr/bin/env python3
"""How many TMs of each tier a set of RZ_TM_W_* weights actually produces.

The naive answer -- 50 * weight / total -- is wrong, because RzBuildTmMoveTable rejects
duplicates. Staples is the band that suffers: once 20-odd of its 46 moves are taken, half
the draws that land in it bounce and re-roll the band, leaking share into Filler and
Niche. This replays the real algorithm instead.

    python3 tools/randolocke/tm_band_sim.py              # report the current weights
    python3 tools/randolocke/tm_band_sim.py 3 25 15 5    # solve for these TM counts
"""
from __future__ import annotations

import random
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
BANDS = ["MetaDefining", "Staples", "Filler", "Niche"]
LABEL = ["Meta Defining", "Staples", "Filler", "Niche"]
WEIGHT_NAME = ["RZ_TM_W_META_DEFINING", "RZ_TM_W_STAPLES", "RZ_TM_W_FILLER", "RZ_TM_W_NICHE"]
NUM_TMS = 50
MAX_ATTEMPTS = 128   # matches RzBuildTmMoveTable


def band_sizes() -> list[int]:
    src = (ROOT / "src/data/randomizer/move_tiers.h").read_text()
    sizes = []
    for band in BANDS:
        blk = re.search(rf"sMoveTier{band}\[\] =\s*\{{(.*?)\}};", src, re.S).group(1)
        sizes.append(len([l for l in blk.split("\n") if l.strip().startswith("MOVE_")]))
    return sizes


def current_weights() -> list[int]:
    src = (ROOT / "include/config/randomizer.h").read_text()
    return [int(re.search(rf"#define {n}\s+(\d+)", src).group(1)) for n in WEIGHT_NAME]


def simulate(weights: list[int], sizes: list[int], trials: int) -> list[float]:
    totals = [0.0] * len(weights)
    span = sum(weights)
    for _ in range(trials):
        taken: set[tuple[int, int]] = set()
        got = [0] * len(weights)
        for _tm in range(NUM_TMS):
            for _ in range(MAX_ATTEMPTS):
                roll, acc = random.randrange(span), 0
                for band, w in enumerate(weights):
                    acc += w
                    if roll < acc:
                        break
                pick = (band, random.randrange(sizes[band]))
                if pick in taken:
                    continue
                taken.add(pick)
                got[band] += 1
                break
        for i, g in enumerate(got):
            totals[i] += g
    return [t / trials for t in totals]


def solve(target: list[float], sizes: list[int]) -> list[int]:
    weights = [max(1, int(1000 * t / sum(target))) for t in target]
    for _ in range(60):
        got = simulate(weights, sizes, 1200)
        if max(abs(got[i] - target[i]) for i in range(len(target))) < 0.2:
            break
        for i in range(len(weights)):
            weights[i] = max(10, round(weights[i] * (target[i] / max(got[i], 0.05)) ** 0.6))
    return weights


def report(weights: list[int], sizes: list[int]) -> None:
    got = simulate(weights, sizes, 8000)
    span, pool = sum(weights), sum(sizes)
    print(f"{'band':15s}{'moves':>7}{'weight':>9}{'share':>9}{'TMs':>8}{'vs uniform':>13}")
    print("-" * 61)
    for i, name in enumerate(LABEL):
        per = (weights[i] / span) / sizes[i]
        print(f"{name:15s}{sizes[i]:7d}{weights[i]:9d}{100*weights[i]/span:8.2f}%"
              f"{got[i]:8.2f}{per*pool:12.2f}x")
    print("-" * 61)
    print(f"{'':31s}{'':9s}{sum(got):8.2f} TMs")
    print("\nBad and Pokemon Homeless are absent by design - see sTmMoveTiers.")


def main() -> int:
    sizes = band_sizes()
    if len(sys.argv) == 1 + len(BANDS):
        target = [float(a) for a in sys.argv[1:]]
        scale = NUM_TMS / sum(target)
        if abs(scale - 1.0) > 0.001:
            print(f"note: your counts sum to {sum(target):g}, not {NUM_TMS}. "
                  f"Scaling by {scale:.3f}.\n")
        weights = solve([t * scale for t in target], sizes)
        print("suggested weights:")
        for n, w in zip(WEIGHT_NAME, weights):
            print(f"  #define {n:26s} {w}")
        print()
    else:
        weights = current_weights()
    report(weights, sizes)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
