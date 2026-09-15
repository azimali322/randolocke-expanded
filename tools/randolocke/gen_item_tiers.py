#!/usr/bin/env python3
"""Generate src/data/randomizer/item_tiers.h.

There is no community tier list for items, so this is the fallback from
RANDOLOCKE_PLAN.md 10.5: the 122 items hand-tiered in pokeemerald_rando_enh are
authoritative, and everything else in the randomizer's whitelist is placed by a heuristic
read from the ROM's own item data (hold effect, pocket, price).

The heuristic follows the same nuzlocke assumption as the hand tiers: the run uses a cheat
heal item and sets EVs manually, so healing items, vitamins and battle items are close to
worthless, while anything with a battle hold effect is worth something.

Usage:  python3 tools/randolocke/gen_item_tiers.py
"""
from __future__ import annotations

import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
FORK = Path("/Users/azima/Desktop/Python_Fun_Scripts/pokeemerald_rando_enh")
OUT = ROOT / "src/data/randomizer/item_tiers.h"
TIERS = 5


def whitelist() -> list[str]:
    s = (ROOT / "src/data/randomizer/item_whitelist.h").read_text()
    m = re.search(r"sRandomizerItemWhitelist\[\] =\s*\{(.*?)\};", s, re.S)
    return [x.strip().rstrip(",") for x in m.group(1).split("\n") if x.strip().startswith("ITEM_")]


def item_data() -> dict[str, dict]:
    s = (ROOT / "src/data/items.h").read_text()
    out = {}
    for name, body in re.findall(r"\[(ITEM_[A-Z0-9_]+)\]\s*=\s*\{(.*?)\n    \},", s, re.S):
        def f(field):
            m = re.search(rf"\.{field}\s*=\s*([A-Za-z0-9_]+)", body)
            return m.group(1) if m else None
        out[name] = {"hold": f("holdEffect"), "pocket": f("pocket"), "sort": f("sortType"),
                     "importance": f("importance"), "price": f("price")}
    return out


def hand_tiers() -> dict[str, int]:
    """The fork's hand-graded items, tier 1..5 -> index 0..4."""
    path = FORK / "src/data/pokemon/item_tiers.h"
    if not path.exists():
        print(f"note: {path} not found; heuristic only")
        return {}
    s = path.read_text()
    out = {}
    for tier, body in re.findall(r"sItemTier(\d)\[\] =\s*\{(.*?)\};", s, re.S):
        for line in body.split("\n"):
            line = line.strip().rstrip(",")
            if line.startswith("ITEM_"):
                out[line] = int(tier) - 1
    return out


# Never worth rolling in this run, whatever else the data says. In-battle stat boosters
# (X Attack and friends) and vitamins go to the floor because azim does not use them, and
# Poke Balls are cheap to buy from the Phase 11 shop so finding them is not a reward.
FLOOR_SORT_TYPES = {
    "ITEM_TYPE_X_ITEM",             # X Attack, X Defend, Dire Hit, Guard Spec
    "ITEM_TYPE_BATTLE_ITEM",
    "ITEM_TYPE_STAT_BOOST_DRINK",   # vitamins
    "ITEM_TYPE_STAT_BOOST_FEATHER",
    "ITEM_TYPE_STAT_BOOST_MOCHI",
    "ITEM_TYPE_HEALTH_RECOVERY",
    "ITEM_TYPE_STATUS_RECOVERY",
    "ITEM_TYPE_PP_RECOVERY",
    "ITEM_TYPE_MAIL",
    "ITEM_TYPE_SELLABLE",
    "ITEM_TYPE_RELIC",
    "ITEM_TYPE_SHARD",
    "ITEM_TYPE_FOSSIL",
    "ITEM_TYPE_CONTEST_HELD_ITEM",
    "ITEM_TYPE_FLUTE",
}

# Held items that do nothing without a gimmick this romhack has switched off.
DEAD_SORT_TYPES = {
    "ITEM_TYPE_MEGA_STONE",   # P_MEGA_EVOLUTIONS is FALSE
    "ITEM_TYPE_Z_CRYSTAL",    # Z-moves unavailable
    "ITEM_TYPE_TERA_SHARD",   # P_TERA_FORMS is FALSE
}


# Items only one Pokemon can use. A Fire Memory is worthless on anything but a Silvally
# and a Shock Drive on anything but a Genesect, so against a randomized dex they are dead
# weight however strong they look on paper -- the odds of the one species that wants them
# being on the team are negligible. They are forced to tier 4, the floor for things that
# at least do *something*, above tier 5 where the consumables and switched-off gimmicks
# sit. Keyed off hold effect and sort type rather than a name list, so new items of the
# same kind land in the right place without anyone remembering to add them.
#
# Plates are deliberately NOT here: since Gen 4 a plate boosts its type for any holder,
# so they are ordinary type-boost items.
ONE_SPECIES_SORT_TYPES = {
    "ITEM_TYPE_MEMORY",   # Silvally, 17 of them
    "ITEM_TYPE_DRIVE",    # Genesect, 4 of them
}

ONE_SPECIES_HOLD_EFFECTS = {
    "HOLD_EFFECT_MEMORY",           # Silvally
    "HOLD_EFFECT_DRIVE",            # Genesect
    "HOLD_EFFECT_LEEK",             # Farfetch'd, Sirfetch'd
    "HOLD_EFFECT_THICK_CLUB",       # Cubone, Marowak
    "HOLD_EFFECT_LIGHT_BALL",       # Pikachu
    "HOLD_EFFECT_SOUL_DEW",         # Latios, Latias
    "HOLD_EFFECT_DEEP_SEA_TOOTH",   # Clamperl
    "HOLD_EFFECT_DEEP_SEA_SCALE",   # Clamperl
    "HOLD_EFFECT_LUCKY_PUNCH",      # Chansey
    "HOLD_EFFECT_METAL_POWDER",     # Ditto
    "HOLD_EFFECT_QUICK_POWDER",     # Ditto
    "HOLD_EFFECT_ADAMANT_ORB",      # Dialga
    "HOLD_EFFECT_LUSTROUS_ORB",     # Palkia
    "HOLD_EFFECT_GRISEOUS_ORB",     # Giratina
    "HOLD_EFFECT_PRIMAL_ORB",       # Groudon, Kyogre -- and Primal Reversion is off anyway
    "HOLD_EFFECT_OGERPON_MASK",     # Ogerpon
    # Not one species, but the Paradox Pokemon are a closed set of about twenty out of
    # the whole dex, so on anything else this is still a blank.
    "HOLD_EFFECT_BOOSTER_ENERGY",
}


def is_one_species(d: dict) -> bool:
    return d.get("sort") in ONE_SPECIES_SORT_TYPES or d.get("hold") in ONE_SPECIES_HOLD_EFFECTS


def heuristic(name: str, d: dict) -> int:
    """Tier index 0..4 for an item with no hand grade.

    The target is "good to hold on a Pokemon in a randomized nuzlocke", so a battle hold
    effect is the signal of worth and anything consumable is near-worthless.
    """
    hold, pocket, sort = d.get("hold"), d.get("pocket"), d.get("sort")

    if sort in DEAD_SORT_TYPES or sort in FLOOR_SORT_TYPES:
        return 4
    if pocket == "POCKET_POKE_BALLS":
        return 3                          # cheap to buy, so not a find worth rewarding
    if sort in ("ITEM_TYPE_EVOLUTION_ITEM", "ITEM_TYPE_EVOLUTION_STONE",
                "ITEM_TYPE_LEVEL_UP_ITEM"):
        return 3                          # also sold cheaply, same reasoning as balls
    if hold and hold != "HOLD_EFFECT_NONE":
        if sort in ("ITEM_TYPE_HELD_ITEM", "ITEM_TYPE_SPECIAL_HELD_ITEM",
                    "ITEM_TYPE_TYPE_BOOST_HELD_ITEM", "ITEM_TYPE_EV_BOOST_HELD_ITEM"):
            return 1                      # the good stuff: real battle hold items
        return 2
    return 4


def main() -> int:
    wl = whitelist()
    data = item_data()
    hand = hand_tiers()

    tiers: list[list[str]] = [[] for _ in range(TIERS)]
    by_hand = by_heur = unknown = berries = by_species = 0
    for name in wl:
        # Berries are excluded from field-item randomization entirely: they are randomized
        # separately at berry trees, so finding one on the ground would double-dip.
        if data.get(name, {}).get("pocket") == "POCKET_BERRIES":
            berries += 1
            continue
        # Overrides both the hand grades and the heuristic: the fork graded several of
        # these on their ceiling with the right holder, which is not the question here.
        if name in data and is_one_species(data[name]):
            tiers[3].append(name); by_species += 1
        elif name in hand:
            tiers[hand[name]].append(name); by_hand += 1
        elif name in data:
            tiers[heuristic(name, data[name])].append(name); by_heur += 1
        else:
            tiers[4].append(name); unknown += 1

    L = ["// Generated by tools/randolocke/gen_item_tiers.py. Do not edit by hand.",
         "//",
         "// No community tier list exists for items, so this is the fallback from",
         "// RANDOLOCKE_PLAN.md 10.5. Items hand-graded in pokeemerald_rando_enh are",
         "// authoritative; the rest are placed from the ROM's own item data.",
         "//",
         "// Heuristic: a battle hold effect is the clearest signal of worth in a run that",
         "// ignores consumables. Held berries and Poke Balls sit mid; healing items,",
         "// vitamins and battle items sit at the bottom, matching the hand tiers' own",
         "// nuzlocke assumption.",
         "//",
         "// Items only one Pokemon can use -- memories, drives, the signature orbs and",
         "// powders -- are forced to tier 4 whatever else says, since on a randomized team",
         "// the holder that wants them is almost never there.",
         "//",
         f"// {by_hand} hand-graded, {by_heur} by heuristic, {by_species} one-species, "
         f"{unknown} unrecognised.",
         ""]
    for i, names in enumerate(tiers, 1):
        L.append(f"// Tier {i} -- {len(names)} items")
        L.append(f"static const u16 sItemTier{i}[] =")
        L.append("{")
        L += [f"    {n}," for n in names] or ["    ITEM_NONE,"]
        L.append("};")
        L.append("")
    OUT.parent.mkdir(parents=True, exist_ok=True)
    OUT.write_text("\n".join(L))
    print(f"wrote {OUT.relative_to(ROOT)}")
    print(f"  hand-graded {by_hand}, heuristic {by_heur}, one-species {by_species}, "
          f"unrecognised {unknown}, berries excluded {berries}")
    for i, names in enumerate(tiers, 1):
        print(f"  tier {i}: {len(names)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
