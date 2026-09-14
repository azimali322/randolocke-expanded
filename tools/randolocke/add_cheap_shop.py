#!/usr/bin/env python3
"""Stock every general Poke Mart with cheap Ultra/Fast/Timer Balls and every evolution item,
and drop those items' prices.

Randolocke needs trade and item evolutions reachable without trading, and the field-item
randomizer deliberately demotes balls and evolution items (Phase 10 tier 4) on the basis
that they are purchasable. This is what makes that true.

Idempotent: re-running adds nothing twice.

Usage:  python3 tools/randolocke/add_cheap_shop.py [--dry-run]
"""
from __future__ import annotations

import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
ITEMS = ROOT / "src/data/items.h"
PRICE = 200            # what a cheap-shop item costs
BALLS = ["ITEM_ULTRA_BALL", "ITEM_FAST_BALL", "ITEM_TIMER_BALL"]
MARKER = "@ randolocke cheap shop"


def evolution_items() -> list[str]:
    s = ITEMS.read_text()
    out = []
    for name, body in re.findall(r"\[(ITEM_[A-Z0-9_]+)\]\s*=\s*\{(.*?)\n    \},", s, re.S):
        if re.search(r"\.sortType\s*=\s*ITEM_TYPE_EVOLUTION_(ITEM|STONE)", body):
            out.append(name)
    return out


def set_prices(names: list[str], dry: bool) -> int:
    s = ITEMS.read_text()
    changed = 0
    for name in names:
        m = re.search(rf"(\[{name}\]\s*=\s*\{{.*?)\.price\s*=\s*[^,\n]+,", s, re.S)
        if not m:
            continue
        seg = m.group(0)
        new = re.sub(r"\.price\s*=\s*[^,\n]+,", f".price = {PRICE},", seg, count=1)
        if new != seg:
            s = s.replace(seg, new, 1)
            changed += 1
    if not dry:
        ITEMS.write_text(s)
    return changed


def stock_marts(names: list[str], dry: bool) -> int:
    lines_to_add = [f"\t.2byte {n}" for n in names]
    touched = 0
    for path in sorted(ROOT.glob("data/maps/*_Mart/scripts.inc")):
        s = path.read_text()
        if MARKER in s or "pokemartlistend" not in s:
            continue
        block = f"\t{MARKER}\n" + "\n".join(lines_to_add) + "\n\tpokemartlistend"
        s = s.replace("\tpokemartlistend", block)
        if not dry:
            path.write_text(s)
        touched += 1
    return touched


def main() -> int:
    dry = "--dry-run" in sys.argv
    evo = evolution_items()
    stock = BALLS + evo
    n_price = set_prices(stock, dry)
    n_mart = stock_marts(stock, dry)
    print(f"{'(dry run) ' if dry else ''}{len(stock)} items "
          f"({len(BALLS)} balls + {len(evo)} evolution items) at {PRICE} each")
    print(f"  prices set: {n_price}")
    print(f"  marts stocked: {n_mart}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
