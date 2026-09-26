#!/usr/bin/env python3
"""Check the game flags and vars this hack borrows from the unused ones.

Every setting in include/config/ that is defined as a FLAG_UNUSED_* or VAR_UNUSED_* has
borrowed that flag or var for itself. Two things would break it quietly, in a player's
save rather than at build time:

  1. two settings borrowing the same one, and
  2. a script or source file using the borrowed one by its raw name.

This finds every borrowed flag and var on its own, so a new one is covered without being
listed anywhere. test/randolocke_flags.c checks the same flags numerically, in-game.

    python3 tools/randolocke/check_borrowed_flags.py
"""
from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parents[2]

DEFINE = re.compile(r"^#define\s+([A-Z0-9_]+)\s+((?:FLAG|VAR)_UNUSED_\w+)\b", re.M)
# Where the unused flags and vars themselves are declared; naming them there is expected.
DECLARED_IN = {"flags.h", "vars.h", "flags_frlg.h", "vars_frlg.h"}
SEARCHED = ("src", "data", "include")
EXTENSIONS = {".c", ".h", ".inc", ".s", ".pory", ".json", ".party"}


def borrowed():
    """{raw flag or var: [config settings defined as it]}"""
    found = {}
    for header in sorted((ROOT / "include" / "config").glob("*.h")):
        for name, raw in DEFINE.findall(header.read_text(encoding="utf-8", errors="replace")):
            found.setdefault(raw, []).append(f"{name} ({header.relative_to(ROOT)})")
    return found


def raw_uses(names):
    pattern = re.compile(r"\b(" + "|".join(map(re.escape, names)) + r")\b")
    uses = []
    for top in SEARCHED:
        for path in (ROOT / top).rglob("*"):
            if not path.is_file() or path.suffix not in EXTENSIONS:
                continue
            if path.name in DECLARED_IN and path.parent.name == "constants":
                continue
            if path.parent == ROOT / "include" / "config":
                continue
            text = path.read_text(encoding="utf-8", errors="replace")
            for number, line in enumerate(text.splitlines(), 1):
                match = pattern.search(line)
                if match:
                    uses.append(f"{path.relative_to(ROOT)}:{number}: {match.group(1)}")
    return uses


def main():
    found = borrowed()
    problems = []
    for raw, settings in sorted(found.items()):
        if len(settings) > 1:
            problems.append(f"{raw} is borrowed by more than one setting: " + ", ".join(settings))
    for use in raw_uses(list(found)):
        problems.append(f"{use} -- use the setting's name, not the borrowed flag or var")

    if problems:
        print("\n".join(problems))
        return 1
    print(f"{len(found)} borrowed flags and vars, each used by one setting and by name only.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
