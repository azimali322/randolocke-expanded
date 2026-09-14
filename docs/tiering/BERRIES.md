# Berry tiers

Berries were pulled out of the item pool and given their own, drawn when you pick a berry tree.

**Why.** Berries were 43 of the 165 tiered items — 26% of the pool — and because 20 of them sat in
tier 3 they accounted for **38.6% of every item the randomizer handed out**. They now come from
trees instead, so they stay common without crowding out everything else.

**How they are graded.** The item data already draws the line: 22 berries have a `holdEffect`, 21
have none. The 22 are ranked by what that effect is worth in a fight; the 21 sit together at the
bottom — reachable at 0.457% each, not removed.

Applies when `ITEM POOL` is Weighted or Strict. With it off, berries stay in the item pool and
trees behave as they always did.

| Tier | Meaning | Berries | Share | Each |
|---|---|---:|---:|---:|
| 1 | Battle staples | 5 | 36.70% | **7.340%** |
| 2 | Strong situational | 6 | 27.50% | **4.583%** |
| 3 | Single status cure | 6 | 17.00% | **2.833%** |
| 4 | Pinch heal, may confuse | 5 | 9.20% | **1.840%** |
| 5 | No hold effect | 21 | 9.60% | **0.457%** |

Strict mode draws tiers 1–2 only: 11 berries.

## Tier 1 — Battle staples (5)

```
  [ ] LUM        CURE_STATUS
  [ ] SITRUS     RESTORE_PCT_HP
  [ ] SALAC      SPEED_UP
  [ ] LIECHI     ATTACK_UP
  [ ] PETAYA     SP_ATTACK_UP
```

## Tier 2 — Strong situational (6)

```
  [ ] CHESTO     CURE_SLP
  [ ] LEPPA      RESTORE_PP
  [ ] GANLON     DEFENSE_UP
  [ ] APICOT     SP_DEFENSE_UP
  [ ] STARF      RANDOM_STAT_UP
  [ ] LANSAT     CRITICAL_UP
```

## Tier 3 — Single status cure (6)

```
  [ ] CHERI      CURE_PAR
  [ ] PERSIM     CURE_CONFUSION
  [ ] ORAN       RESTORE_HP
  [ ] PECHA      CURE_PSN
  [ ] RAWST      CURE_BRN
  [ ] ASPEAR     CURE_FRZ
```

## Tier 4 — Pinch heal, may confuse (5)

```
  [ ] FIGY       CONFUSE_SPICY
  [ ] WIKI       CONFUSE_DRY
  [ ] MAGO       CONFUSE_SWEET
  [ ] AGUAV      CONFUSE_BITTER
  [ ] IAPAPA     CONFUSE_SOUR
```

## Tier 5 — No hold effect (21)

```
  [ ] RAZZ       NONE
  [ ] BLUK       NONE
  [ ] NANAB      NONE
  [ ] WEPEAR     NONE
  [ ] PINAP      NONE
  [ ] POMEG      NONE
  [ ] KELPSY     NONE
  [ ] QUALOT     NONE
  [ ] HONDEW     NONE
  [ ] GREPA      NONE
  [ ] TAMATO     NONE
  [ ] CORNN      NONE
  [ ] MAGOST     NONE
  [ ] RABUTA     NONE
  [ ] NOMEL      NONE
  [ ] SPELON     NONE
  [ ] PAMTRE     NONE
  [ ] WATMEL     NONE
  [ ] DURIN      NONE
  [ ] BELUE      NONE
  [ ] ENIGMA     NONE
```
