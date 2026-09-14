# Randomizable Moves — community tier mapping

**Source: `docs/community-moves-tierlist.png`** (community rankings, 19 submitted lists).
Every one of Modern Emerald's **367** randomizable moves was located on that list and given its community
tier. The list covers ~891 moves across all generations; the 524 this ROM does not have are ignored.

Used by the VGC move pool (Phase 5) and TM weighting (Phase 8c).

## Tier counts

| Tier | Community name | Moves | Share |
|---|---|---:|---:|
| 1 | Meta Defining | 3 | 0.8% |
| 2 | Staples | 26 | 7.1% |
| 3 | Filler/Outclassed | 90 | 24.5% |
| 4 | Niche | 157 | 42.8% |
| 5 | Bad | 72 | 19.6% |
| 6 | Pokemon Homeless | 19 | 5.2% |
| | **Total** | **367** | |

## Weights — implemented

**4 / 24 / 38 / 27 / 6 / 1.** Divided by tier size, per-move odds fall monotonically:

| Tier | n | Weight | Per-move | × uniform |
|---|---:|---:|---:|---:|
| 1 Meta Defining | 3 | 4% | 1.333% | 4.89× |
| 2 Staples | 26 | 24% | 0.923% | 3.39× |
| 3 Filler | 90 | 38% | 0.422% | 1.55× |
| 4 Niche | 157 | 27% | 0.172% | 0.63× |
| 5 Bad | 72 | 6% | 0.083% | 0.31× |
| 6 Homeless | 19 | 1% | 0.053% | 0.19× |

Uniform baseline is 0.272% per move. *Strict* mode draws from tiers 1+2 together (29 moves) — tier 1
alone is 3 moves, which would give every Pokémon the same moveset.

## Nuzlocke overrides — applied

Moves forced to the bottom tier regardless of their community rank, because a self-KO in a nuzlocke
is a permanently lost Pokémon:

| Move | Community tier | Forced to | Why |
|---|---:|---:|---|
| SELF_DESTRUCT | 4 (Niche) | **6** | guaranteed self-KO |
| EXPLOSION | 4 (Niche) | **6** | guaranteed self-KO |
| MEMENTO | 5 (Bad) | **6** | guaranteed self-KO |


## Nuzlocke-hostile moves — all resolved

Everything below has been applied.

**Guaranteed self-KO** — the same category as Self-Destruct. Strongest case for forcing:

| Move | Tier | Effect |
|---|---:|---|
| *(Self-Destruct, Explosion, Memento — already forced)* | — | — |

Healing Wish, Lunar Dance and Final Gambit are **not in this ROM**, so the forced three are the whole set.

**Can self-KO through recoil or HP cost** — these will sometimes kill your own Pokémon:

| Move | Tier | Why it can end a run |
|---|---:|---|
| `CURSE` | 3 | Ghost-type version pays **half max HP**; can faint the user outright |
| `DOUBLE_EDGE` | 4 | 1/3 recoil |
| `TAKE_DOWN` | 4 | 1/4 recoil |
| `VOLT_TACKLE` | 4 | 1/3 recoil |
| `JUMP_KICK` | 5 | crash damage on miss |
| `HI_JUMP_KICK` | 6 | crash damage on miss — *already bottom* |
| `SUBMISSION` | 6 | 1/4 recoil — *already bottom* |
| `STRUGGLE` | 5 | recoil; only appears when out of PP |

**Loses the Pokémon indirectly:**

| Move | Tier | Why |
|---|---:|---|
| `PERISH_SONG` | **2** | faints **both** sides after 3 turns. On your own Pokémon in a nuzlocke this is a death sentence unless you can switch — and it is currently in the second-highest tier |
| `DESTINY_BOND` | 4 | does not self-KO; listed only so you can rule it out |

**My recommendation:** force `CURSE` and `PERISH_SONG` down. Curse is tier 3 and costs half your HP on a Ghost;
Perish Song at tier 2 is the sharpest mismatch on the whole list for a nuzlocke — the community rates it highly
*because* it wins stall wars, which is exactly the situation where you cannot afford to lose the Pokémon.
The recoil moves are riskier than average but they are also genuinely good moves, so demoting all of them
would cost you a lot of usable options.

---
# Moves by tier

`[ ]` is there if you want to override a placement. `pow`/`acc`/`pp` are this ROM's values.

## Tier 1 — Meta Defining (3)

```
  [ ] EXTREME_SPEED              pow= 80 acc=100 NORMAL  PHYS pp= 5  QUICK_ATTACK
  [ ] BELLY_DRUM                 pow=  0 acc=  0 NORMAL  STAT pp=10  BELLY_DRUM
  [ ] PROTECT               [TM] pow=  0 acc=  0 NORMAL  STAT pp=10  PROTECT
```

## Tier 2 — Staples (26)

```
  [ ] ERUPTION                   pow=150 acc=100 FIRE    SPEC pp= 5  ERUPTION
  [ ] FUTURE_SIGHT               pow=120 acc=100 PSYCHIC SPEC pp=10  FUTURE_SIGHT
  [ ] AEROBLAST                  pow=100 acc= 95 FLYING  SPEC pp= 5  HIGH_CRITICAL
  [ ] EARTHQUAKE            [TM] pow=100 acc=100 GROUND  PHYS pp=10  EARTHQUAKE
  [ ] BUG_BUZZ                   pow= 90 acc=100 BUG     SPEC pp=10  SPECIAL_DEFENSE_DOWN
  [ ] HYPER_VOICE                pow= 90 acc=100 NORMAL  SPEC pp=10  HIT
  [ ] BODY_SLAM                  pow= 85 acc=100 NORMAL  PHYS pp=15  PARALYZE_HIT
  [ ] BOUNCE                     pow= 85 acc= 85 FLYING  PHYS pp= 5  SEMI_INVULNERABLE
  [ ] CRUNCH                     pow= 80 acc=100 DARK    PHYS pp=15  DEFENSE_DOWN_HIT
  [ ] SHADOW_BALL           [TM] pow= 80 acc=100 GHOST   SPEC pp=15  SPECIAL_DEFENSE_DOWN
  [ ] GIGA_DRAIN            [TM] pow= 75 acc=100 GRASS   SPEC pp=10  ABSORB
  [ ] FACADE                [TM] pow= 70 acc=100 NORMAL  PHYS pp=20  FACADE
  [ ] ANCIENT_POWER              pow= 60 acc=100 ROCK    SPEC pp= 5  ALL_STATS_UP_HIT
  [ ] FAKE_OUT                   pow= 40 acc=100 NORMAL  PHYS pp=10  FAKE_OUT
  [ ] ASTONISH                   pow= 30 acc=100 GHOST   PHYS pp=15  FLINCH_MINIMIZE_HIT
  [ ] AGILITY                    pow=  0 acc=  0 PSYCHIC STAT pp=30  SPEED_UP_2
  [ ] AROMATHERAPY               pow=  0 acc=  0 GRASS   STAT pp= 5  HEAL_BELL
  [ ] BULK_UP               [TM] pow=  0 acc=  0 FIGHTINGSTAT pp=20  BULK_UP
  [ ] DRAGON_DANCE               pow=  0 acc=  0 DRAGON  STAT pp=20  DRAGON_DANCE
  [ ] ENDURE                     pow=  0 acc=  0 NORMAL  STAT pp=10  ENDURE
  [ ] FOLLOW_ME                  pow=  0 acc=100 NORMAL  STAT pp=20  FOLLOW_ME
  [ ] GLARE                      pow=  0 acc=100 NORMAL  STAT pp=30  PARALYZE
  [ ] GROWTH                     pow=  0 acc=  0 NORMAL  STAT pp=20  SPECIAL_ATTACK_UP
  [ ] LEECH_SEED                 pow=  0 acc= 90 GRASS   STAT pp=10  LEECH_SEED
  [ ] PERISH_SONG                pow=  0 acc=  0 NORMAL  STAT pp= 5  PERISH_SONG
  [ ] RECOVER                    pow=  0 acc=  0 NORMAL  STAT pp= 5  RESTORE_HP
  [ ] SWORDS_DANCE               pow=  0 acc=  0 NORMAL  STAT pp=20  ATTACK_UP_2
```

## Tier 3 — Filler/Outclassed (90)

```
  [ ] BLAST_BURN                 pow=150 acc= 90 FIRE    SPEC pp= 5  RECHARGE
  [ ] FRENZY_PLANT               pow=150 acc= 90 GRASS   SPEC pp= 5  RECHARGE
  [ ] HYDRO_CANNON               pow=150 acc= 90 WATER   SPEC pp= 5  RECHARGE
  [ ] HYPER_BEAM            [TM] pow=150 acc= 90 NORMAL  SPEC pp= 5  RECHARGE
  [ ] WATER_SPOUT                pow=150 acc=100 WATER   SPEC pp= 5  ERUPTION
  [ ] DOOM_DESIRE                pow=140 acc=100 STEEL   SPEC pp= 5  FUTURE_SIGHT
  [ ] OVERHEAT              [TM] pow=130 acc= 90 FIRE    SPEC pp= 5  OVERHEAT
  [ ] BLIZZARD              [TM] pow=110 acc= 70 ICE     SPEC pp= 5  FREEZE_HIT
  [ ] FIRE_BLAST            [TM] pow=110 acc= 85 FIRE    SPEC pp= 5  BURN_HIT
  [ ] THRASH                     pow=105 acc=100 NORMAL  PHYS pp=10  RAMPAGE
  [ ] DYNAMIC_PUNCH              pow=100 acc= 50 FIGHTINGPHYS pp= 5  CONFUSE_HIT
  [ ] OUTRAGE                    pow=100 acc=100 DRAGON  PHYS pp=10  RAMPAGE
  [ ] HEAT_WAVE                  pow= 95 acc= 90 FIRE    SPEC pp=10  BURN_HIT
  [ ] MOONBLAST                  pow= 95 acc=100 FAIRY   SPEC pp=15  SPECIAL_ATTACK_DOWN_
  [ ] DIVE                  [HM] pow= 90 acc=100 WATER   PHYS pp=10  SEMI_INVULNERABLE
  [ ] EARTH_POWER                pow= 90 acc=100 GROUND  SPEC pp=10  SPECIAL_DEFENSE_DOWN
  [ ] FLAMETHROWER          [TM] pow= 90 acc=100 FIRE    SPEC pp=15  BURN_HIT
  [ ] ICE_BEAM              [TM] pow= 90 acc=100 ICE     SPEC pp=10  FREEZE_HIT
  [ ] LEAF_BLADE                 pow= 90 acc=100 GRASS   PHYS pp=15  HIGH_CRITICAL
  [ ] MUDDY_WATER                pow= 90 acc= 85 GROUND  SPEC pp=10  ACCURACY_DOWN_HIT
  [ ] THUNDERBOLT           [TM] pow= 90 acc=100 ELECTRICSPEC pp=15  PARALYZE_HIT
  [ ] BLAZE_KICK                 pow= 85 acc= 90 FIRE    PHYS pp=10  BLAZE_KICK
  [ ] DRAGON_PULSE               pow= 85 acc=100 DRAGON  SPEC pp=10  HIT
  [ ] DIG                   [TM] pow= 80 acc=100 GROUND  PHYS pp=10  SEMI_INVULNERABLE
  [ ] DRAGON_CLAW           [TM] pow= 80 acc=100 DRAGON  PHYS pp=15  HIT
  [ ] EXTRASENSORY               pow= 80 acc=100 PSYCHIC SPEC pp=20  FLINCH_MINIMIZE_HIT
  [ ] FLASH_CANNON               pow= 80 acc=100 STEEL   SPEC pp=10  SPECIAL_DEFENSE_DOWN
  [ ] FLY                   [HM] pow= 80 acc= 95 FLYING  PHYS pp=15  SEMI_INVULNERABLE
  [ ] POISON_JAB                 pow= 80 acc=100 POISON  PHYS pp=20  POISON_HIT
  [ ] STRENGTH              [HM] pow= 80 acc=100 NORMAL  PHYS pp=15  HIT
  [ ] TRI_ATTACK                 pow= 80 acc=100 NORMAL  SPEC pp=10  TRI_ATTACK
  [ ] AIR_SLASH                  pow= 75 acc= 95 FLYING  SPEC pp=15  FLINCH_HIT
  [ ] BRICK_BREAK           [TM] pow= 75 acc=100 FIGHTINGPHYS pp=15  BRICK_BREAK
  [ ] DIZZY_PUNCH                pow= 70 acc=100 NORMAL  PHYS pp=10  CONFUSE_HIT
  [ ] SHADOW_CLAW                pow= 70 acc=100 GHOST   PHYS pp=15  HIGH_CRITICAL
  [ ] SLASH                      pow= 70 acc=100 NORMAL  PHYS pp=20  HIGH_CRITICAL
  [ ] KNOCK_OFF                  pow= 65 acc=100 DARK    PHYS pp=20  KNOCK_OFF
  [ ] STOMP                      pow= 65 acc=100 NORMAL  PHYS pp=20  FLINCH_MINIMIZE_HIT
  [ ] AERIAL_ACE            [TM] pow= 60 acc=  0 FLYING  PHYS pp=20  ALWAYS_HIT
  [ ] AIR_CUTTER                 pow= 60 acc= 95 FLYING  SPEC pp=25  HIGH_CRITICAL
  [ ] BITE                       pow= 60 acc=100 DARK    PHYS pp=25  FLINCH_HIT
  [ ] REVENGE                    pow= 60 acc=100 FIGHTINGPHYS pp=10  REVENGE
  [ ] SHADOW_PUNCH               pow= 60 acc=  0 GHOST   PHYS pp=20  ALWAYS_HIT
  [ ] RAPID_SPIN                 pow= 50 acc=100 NORMAL  PHYS pp=40  RAPID_SPIN
  [ ] WEATHER_BALL               pow= 50 acc=100 NORMAL  SPEC pp=10  WEATHER_BALL
  [ ] MACH_PUNCH                 pow= 40 acc=100 FIGHTINGPHYS pp=30  QUICK_ATTACK
  [ ] QUICK_ATTACK               pow= 40 acc=100 NORMAL  PHYS pp=30  QUICK_ATTACK
  [ ] ROLLOUT                    pow= 30 acc= 90 ROCK    PHYS pp=20  ROLLOUT
  [ ] BONE_RUSH                  pow= 25 acc= 80 GROUND  PHYS pp=10  MULTI_HIT
  [ ] BULLET_SEED           [TM] pow= 20 acc=100 GRASS   PHYS pp=30  MULTI_HIT
  [ ] ARM_THRUST                 pow= 15 acc=100 FIGHTINGPHYS pp=20  MULTI_HIT
  [ ] BEAT_UP                    pow= 10 acc=100 DARK    PHYS pp=10  BEAT_UP
  [ ] COUNTER                    pow=  1 acc=100 FIGHTINGPHYS pp=20  COUNTER
  [ ] ENDEAVOR                   pow=  1 acc=100 NORMAL  PHYS pp= 5  ENDEAVOR
  [ ] FLAIL                      pow=  1 acc=100 NORMAL  PHYS pp=15  FLAIL
  [ ] FRUSTRATION           [TM] pow=  1 acc=100 NORMAL  PHYS pp=20  FRUSTRATION
  [ ] MIRROR_COAT                pow=  1 acc=100 PSYCHIC SPEC pp=20  MIRROR_COAT
  [ ] RETURN                [TM] pow=  1 acc=100 NORMAL  PHYS pp=20  RETURN
  [ ] SUPER_FANG                 pow=  1 acc= 90 NORMAL  PHYS pp=10  SUPER_FANG
  [ ] ACID_ARMOR                 pow=  0 acc=  0 POISON  STAT pp=20  DEFENSE_UP_2
  [ ] AMNESIA                    pow=  0 acc=  0 PSYCHIC STAT pp=20  SPECIAL_DEFENSE_UP_2
  [ ] BATON_PASS                 pow=  0 acc=  0 NORMAL  STAT pp=40  BATON_PASS
  [ ] COSMIC_POWER               pow=  0 acc=  0 PSYCHIC STAT pp=20  COSMIC_POWER
  [ ] CURSE                      pow=  0 acc=  0 MYSTERY STAT pp=10  CURSE
  [ ] DETECT                     pow=  0 acc=  0 FIGHTINGSTAT pp= 5  PROTECT
  [ ] DISABLE                    pow=  0 acc=100 NORMAL  STAT pp=20  DISABLE
  [ ] GROWL                      pow=  0 acc=100 NORMAL  STAT pp=40  ATTACK_DOWN
  [ ] HEAL_BELL                  pow=  0 acc=  0 NORMAL  STAT pp= 5  HEAL_BELL
  [ ] HELPING_HAND               pow=  0 acc=100 NORMAL  STAT pp=20  HELPING_HAND
  [ ] HOWL                       pow=  0 acc=  0 NORMAL  STAT pp=40  ATTACK_UP
  [ ] LIGHT_SCREEN          [TM] pow=  0 acc=  0 PSYCHIC STAT pp=30  LIGHT_SCREEN
  [ ] LOCK_ON                    pow=  0 acc=100 NORMAL  STAT pp= 5  LOCK_ON
  [ ] MOONLIGHT                  pow=  0 acc=  0 FAIRY   STAT pp= 5  MOONLIGHT
  [ ] MORNING_SUN                pow=  0 acc=  0 NORMAL  STAT pp= 5  MORNING_SUN
  [ ] PAIN_SPLIT                 pow=  0 acc=100 NORMAL  STAT pp=20  PAIN_SPLIT
  [ ] RECYCLE                    pow=  0 acc=100 NORMAL  STAT pp=10  RECYCLE
  [ ] REST                  [TM] pow=  0 acc=  0 PSYCHIC STAT pp= 5  REST
  [ ] SCARY_FACE                 pow=  0 acc=100 NORMAL  STAT pp=10  SPEED_DOWN_2
  [ ] SLACK_OFF                  pow=  0 acc=100 NORMAL  STAT pp= 5  RESTORE_HP
  [ ] SOFT_BOILED                pow=  0 acc=100 NORMAL  STAT pp= 5  SOFTBOILED
  [ ] SPIKES                     pow=  0 acc=  0 GROUND  STAT pp=20  SPIKES
  [ ] SPORE                      pow=  0 acc=100 GRASS   STAT pp=15  SLEEP
  [ ] SUBSTITUTE                 pow=  0 acc=  0 NORMAL  STAT pp=10  SUBSTITUTE
  [ ] TAUNT                 [TM] pow=  0 acc=100 DARK    STAT pp=20  TAUNT
  [ ] THUNDER_WAVE               pow=  0 acc= 90 ELECTRICSTAT pp=20  PARALYZE
  [ ] TICKLE                     pow=  0 acc=100 NORMAL  STAT pp=20  TICKLE
  [ ] TORMENT               [TM] pow=  0 acc=100 DARK    STAT pp=15  TORMENT
  [ ] TOXIC                 [TM] pow=  0 acc= 70 POISON  STAT pp=10  TOXIC
  [ ] WILL_O_WISP                pow=  0 acc= 85 FIRE    STAT pp=15  WILL_O_WISP
  [ ] WISH                       pow=  0 acc=100 NORMAL  STAT pp=10  WISH
  [ ] YAWN                       pow=  0 acc=100 NORMAL  STAT pp=10  YAWN
```

## Tier 4 — Niche (157)

```
  [ ] FOCUS_PUNCH           [TM] pow=150 acc=100 FIGHTINGPHYS pp=20  FOCUS_PUNCH
  [ ] PSYCHO_BOOST               pow=140 acc= 90 PSYCHIC SPEC pp= 5  OVERHEAT
  [ ] DOUBLE_EDGE                pow=120 acc=100 NORMAL  PHYS pp=15  DOUBLE_EDGE
  [ ] HYDRO_PUMP                 pow=120 acc= 80 WATER   SPEC pp= 5  HIT
  [ ] MEGA_KICK                  pow=120 acc= 75 NORMAL  PHYS pp= 5  HIT
  [ ] SOLAR_BEAM            [TM] pow=120 acc=100 GRASS   SPEC pp=10  SOLAR_BEAM
  [ ] SUPERPOWER                 pow=120 acc=100 FIGHTINGPHYS pp= 5  SUPERPOWER
  [ ] VOLT_TACKLE                pow=120 acc=100 ELECTRICPHYS pp=15  DOUBLE_EDGE
  [ ] THUNDER               [TM] pow=110 acc= 70 ELECTRICSPEC pp=10  THUNDER
  [ ] CROSS_CHOP                 pow=100 acc= 80 FIGHTINGPHYS pp= 5  HIGH_CRITICAL
  [ ] DREAM_EATER                pow=100 acc=100 PSYCHIC SPEC pp=15  DREAM_EATER
  [ ] EGG_BOMB                   pow=100 acc= 90 NORMAL  PHYS pp=10  HIT
  [ ] IRON_TAIL             [TM] pow=100 acc= 75 STEEL   PHYS pp=15  DEFENSE_DOWN_HIT
  [ ] METEOR_MASH                pow=100 acc= 90 STEEL   PHYS pp=10  ATTACK_UP_HIT
  [ ] PETAL_DANCE                pow=100 acc=100 GRASS   SPEC pp=10  RAMPAGE
  [ ] SACRED_FIRE                pow=100 acc= 95 FIRE    PHYS pp= 5  THAW_HIT
  [ ] CRABHAMMER                 pow= 90 acc= 90 WATER   PHYS pp=10  HIGH_CRITICAL
  [ ] SURF                  [HM] pow= 90 acc=100 WATER   SPEC pp=15  HIT
  [ ] TAKE_DOWN                  pow= 90 acc= 85 NORMAL  PHYS pp=20  RECOIL
  [ ] DRILL_PECK                 pow= 85 acc=100 FLYING  PHYS pp=20  HIT
  [ ] SKY_UPPERCUT               pow= 85 acc= 90 FIGHTINGPHYS pp=15  SKY_UPPERCUT
  [ ] DARK_PULSE                 pow= 80 acc=100 DARK    SPEC pp=15  FLINCH_HIT
  [ ] HYPER_FANG                 pow= 80 acc= 90 NORMAL  PHYS pp=15  FLINCH_HIT
  [ ] POWER_GEM                  pow= 80 acc=100 ROCK    SPEC pp=20  HIT
  [ ] SLAM                       pow= 80 acc= 75 NORMAL  PHYS pp=20  HIT
  [ ] WATERFALL             [HM] pow= 80 acc=100 WATER   PHYS pp=15  HIT
  [ ] CRUSH_CLAW                 pow= 75 acc= 95 NORMAL  PHYS pp=10  DEFENSE_DOWN_HIT
  [ ] ROCK_SLIDE                 pow= 75 acc= 90 ROCK    PHYS pp=10  FLINCH_HIT
  [ ] SIGNAL_BEAM                pow= 75 acc=100 BUG     SPEC pp=15  CONFUSE_HIT
  [ ] HEADBUTT                   pow= 70 acc=100 NORMAL  PHYS pp=15  FLINCH_HIT
  [ ] LUSTER_PURGE               pow= 70 acc=100 PSYCHIC SPEC pp= 5  SPECIAL_DEFENSE_DOWN
  [ ] MIST_BALL                  pow= 70 acc=100 PSYCHIC SPEC pp= 5  SPECIAL_ATTACK_DOWN_
  [ ] PSYCHO_CUT                 pow= 70 acc=100 PSYCHIC PHYS pp=20  HIGH_CRITICAL
  [ ] SECRET_POWER          [TM] pow= 70 acc=100 NORMAL  PHYS pp=20  SECRET_POWER
  [ ] SMELLING_SALT              pow= 70 acc=100 NORMAL  PHYS pp=10  SMELLINGSALT
  [ ] STEEL_WING            [TM] pow= 70 acc= 90 STEEL   PHYS pp=25  DEFENSE_UP_HIT
  [ ] VITAL_THROW                pow= 70 acc=100 FIGHTINGPHYS pp=10  VITAL_THROW
  [ ] AURORA_BEAM                pow= 65 acc=100 ICE     SPEC pp=20  ATTACK_DOWN_HIT
  [ ] BONE_CLUB                  pow= 65 acc= 85 GROUND  PHYS pp=20  FLINCH_HIT
  [ ] BUBBLE_BEAM                pow= 65 acc=100 WATER   SPEC pp=20  SPEED_DOWN_HIT
  [ ] HORN_ATTACK                pow= 65 acc=100 NORMAL  PHYS pp=25  HIT
  [ ] POISON_FANG                pow= 65 acc=100 POISON  PHYS pp=15  POISON_FANG
  [ ] PSYBEAM                    pow= 65 acc=100 PSYCHIC SPEC pp=20  CONFUSE_HIT
  [ ] SPARK                      pow= 65 acc=100 ELECTRICPHYS pp=20  PARALYZE_HIT
  [ ] TWISTER                    pow= 65 acc=100 DRAGON  SPEC pp=20  TWISTER
  [ ] COVET                      pow= 60 acc=100 NORMAL  PHYS pp=20  THIEF
  [ ] DRAGON_BREATH              pow= 60 acc=100 DRAGON  SPEC pp=20  PARALYZE_HIT
  [ ] FAINT_ATTACK               pow= 60 acc=  0 DARK    PHYS pp=20  ALWAYS_HIT
  [ ] FLAME_WHEEL                pow= 60 acc=100 FIRE    PHYS pp=25  THAW_HIT
  [ ] LEECH_LIFE                 pow= 60 acc=100 BUG     PHYS pp=10  ABSORB
  [ ] MAGICAL_LEAF               pow= 60 acc=  0 GRASS   SPEC pp=20  ALWAYS_HIT
  [ ] ROCK_TOMB             [TM] pow= 60 acc= 95 ROCK    PHYS pp=15  SPEED_DOWN_HIT
  [ ] SWIFT                      pow= 60 acc=  0 NORMAL  SPEC pp=20  ALWAYS_HIT
  [ ] ICY_WIND                   pow= 55 acc= 95 ICE     SPEC pp=15  SPEED_DOWN_HIT
  [ ] MUD_SHOT                   pow= 55 acc= 95 GROUND  SPEC pp=15  SPEED_DOWN_HIT
  [ ] RAZOR_LEAF                 pow= 55 acc= 95 GRASS   PHYS pp=25  HIGH_CRITICAL
  [ ] VICE_GRIP                  pow= 55 acc=100 NORMAL  PHYS pp=30  HIT
  [ ] BONEMERANG                 pow= 50 acc= 90 GROUND  PHYS pp=10  DOUBLE_HIT
  [ ] CONFUSION                  pow= 50 acc=100 PSYCHIC SPEC pp=25  CONFUSE_HIT
  [ ] METAL_CLAW                 pow= 50 acc= 95 STEEL   PHYS pp=35  ATTACK_UP_HIT
  [ ] NIGHT_SHADE                pow= 50 acc=100 GHOST   SPEC pp=15  HIT
  [ ] ROCK_THROW                 pow= 50 acc= 90 ROCK    PHYS pp=15  HIT
  [ ] SNORE                      pow= 50 acc=100 NORMAL  SPEC pp=15  SNORE
  [ ] THIEF                 [TM] pow= 50 acc=100 DARK    PHYS pp=25  THIEF
  [ ] ACID                       pow= 40 acc=100 POISON  SPEC pp=30  SPECIAL_DEFENSE_DOWN
  [ ] EMBER                      pow= 40 acc=100 FIRE    SPEC pp=25  BURN_HIT
  [ ] FALSE_SWIPE                pow= 40 acc=100 NORMAL  PHYS pp=40  FALSE_SWIPE
  [ ] GUST                       pow= 40 acc=100 FLYING  SPEC pp=35  GUST
  [ ] MEGA_DRAIN                 pow= 40 acc=100 GRASS   SPEC pp=15  ABSORB
  [ ] PAY_DAY                    pow= 40 acc=100 NORMAL  PHYS pp=20  PAY_DAY
  [ ] POUND                      pow= 40 acc=100 NORMAL  PHYS pp=35  HIT
  [ ] PURSUIT                    pow= 40 acc=100 DARK    PHYS pp=20  PURSUIT
  [ ] ROCK_SMASH            [HM] pow= 40 acc=100 FIGHTINGPHYS pp=15  DEFENSE_DOWN_HIT
  [ ] SCRATCH                    pow= 40 acc=100 NORMAL  PHYS pp=35  HIT
  [ ] TACKLE                     pow= 40 acc=100 NORMAL  PHYS pp=35  HIT
  [ ] WATER_GUN                  pow= 40 acc=100 WATER   SPEC pp=25  HIT
  [ ] CLAMP                      pow= 35 acc= 85 WATER   PHYS pp=15  TRAP
  [ ] FIRE_SPIN                  pow= 35 acc= 95 FIRE    SPEC pp=15  TRAP
  [ ] SAND_TOMB                  pow= 35 acc= 95 GROUND  PHYS pp=15  TRAP
  [ ] WHIRLPOOL                  pow= 35 acc= 95 WATER   SPEC pp=15  TRAP
  [ ] BUBBLE                     pow= 30 acc=100 WATER   SPEC pp=30  SPEED_DOWN_HIT
  [ ] COMET_PUNCH                pow= 30 acc=100 NORMAL  PHYS pp=15  MULTI_HIT
  [ ] ICE_BALL                   pow= 30 acc= 90 ICE     PHYS pp=20  ROLLOUT
  [ ] LICK                       pow= 30 acc=100 GHOST   PHYS pp=30  PARALYZE_HIT
  [ ] ROCK_BLAST                 pow= 25 acc= 90 ROCK    PHYS pp=10  MULTI_HIT
  [ ] ABSORB                     pow= 20 acc=100 GRASS   SPEC pp=25  ABSORB
  [ ] BARRAGE                    pow= 20 acc=100 NORMAL  PHYS pp=20  MULTI_HIT
  [ ] FURY_CUTTER                pow= 20 acc= 95 BUG     PHYS pp=20  FURY_CUTTER
  [ ] ICICLE_SPEAR               pow= 20 acc=100 ICE     PHYS pp=30  MULTI_HIT
  [ ] MUD_SLAP                   pow= 20 acc=100 GROUND  SPEC pp=10  ACCURACY_DOWN_HIT
  [ ] RAGE                       pow= 20 acc=100 NORMAL  PHYS pp=20  RAGE
  [ ] FURY_SWIPES                pow= 18 acc= 80 NORMAL  PHYS pp=15  MULTI_HIT
  [ ] POISON_STING               pow= 15 acc=100 POISON  PHYS pp=35  POISON_HIT
  [ ] WRAP                       pow= 15 acc= 90 NORMAL  PHYS pp=20  TRAP
  [ ] TRIPLE_KICK                pow= 10 acc= 90 FIGHTINGPHYS pp=10  TRIPLE_KICK
  [ ] BIDE                       pow=  1 acc=100 NORMAL  PHYS pp=10  BIDE
  [ ] REVERSAL                   pow=  1 acc=100 FIGHTINGPHYS pp=15  FLAIL
  [ ] SEISMIC_TOSS               pow=  1 acc=100 FIGHTINGPHYS pp=20  LEVEL_DAMAGE
  [ ] ASSIST                     pow=  0 acc=100 NORMAL  STAT pp=20  ASSIST
  [ ] BARRIER                    pow=  0 acc=  0 PSYCHIC STAT pp=20  DEFENSE_UP_2
  [ ] BLOCK                      pow=  0 acc=100 NORMAL  STAT pp= 5  MEAN_LOOK
  [ ] CALM_MIND             [TM] pow=  0 acc=  0 PSYCHIC STAT pp=20  CALM_MIND
  [ ] CAMOUFLAGE                 pow=  0 acc=100 NORMAL  STAT pp=20  CAMOUFLAGE
  [ ] CHARGE                     pow=  0 acc=100 ELECTRICSTAT pp=20  CHARGE
  [ ] CONFUSE_RAY                pow=  0 acc=100 GHOST   STAT pp=10  CONFUSE
  [ ] CONVERSION                 pow=  0 acc=  0 NORMAL  STAT pp=30  CONVERSION
  [ ] CONVERSION_2               pow=  0 acc=100 NORMAL  STAT pp=30  CONVERSION_2
  [ ] COTTON_SPORE               pow=  0 acc=100 GRASS   STAT pp=40  SPEED_DOWN_2
  [ ] DESTINY_BOND               pow=  0 acc=  0 GHOST   STAT pp= 5  DESTINY_BOND
  [ ] ENCORE                     pow=  0 acc=100 NORMAL  STAT pp= 5  ENCORE
  [ ] FAKE_TEARS                 pow=  0 acc=100 DARK    STAT pp=20  SPECIAL_DEFENSE_DOWN
  [ ] FEATHER_DANCE              pow=  0 acc=100 FLYING  STAT pp=15  ATTACK_DOWN_2
  [ ] FLASH                 [HM] pow=  0 acc=100 NORMAL  STAT pp=20  ACCURACY_DOWN
  [ ] FLATTER                    pow=  0 acc=100 DARK    STAT pp=15  FLATTER
  [ ] FOCUS_ENERGY               pow=  0 acc=  0 NORMAL  STAT pp=30  FOCUS_ENERGY
  [ ] FORESIGHT                  pow=  0 acc=100 NORMAL  STAT pp=40  FORESIGHT
  [ ] GRUDGE                     pow=  0 acc=100 GHOST   STAT pp= 5  GRUDGE
  [ ] HARDEN                     pow=  0 acc=  0 NORMAL  STAT pp=30  DEFENSE_UP
  [ ] HAZE                       pow=  0 acc=  0 ICE     STAT pp=30  HAZE
  [ ] IMPRISON                   pow=  0 acc=100 PSYCHIC STAT pp=10  IMPRISON
  [ ] INGRAIN                    pow=  0 acc=100 GRASS   STAT pp=20  INGRAIN
  [ ] IRON_DEFENSE               pow=  0 acc=  0 STEEL   STAT pp=15  DEFENSE_UP_2
  [ ] LEER                       pow=  0 acc=100 NORMAL  STAT pp=30  DEFENSE_DOWN
  [ ] LOVELY_KISS                pow=  0 acc= 75 NORMAL  STAT pp=10  SLEEP
  [ ] MAGIC_COAT                 pow=  0 acc=100 PSYCHIC STAT pp=15  MAGIC_COAT
  [ ] MEAN_LOOK                  pow=  0 acc=100 NORMAL  STAT pp= 5  MEAN_LOOK
  [ ] METAL_SOUND                pow=  0 acc= 85 STEEL   STAT pp=40  SPECIAL_DEFENSE_DOWN
  [ ] MIND_READER                pow=  0 acc=100 NORMAL  STAT pp= 5  LOCK_ON
  [ ] MINIMIZE                   pow=  0 acc=  0 NORMAL  STAT pp=10  MINIMIZE
  [ ] MIST                       pow=  0 acc=  0 ICE     STAT pp=30  MIST
  [ ] NIGHTMARE                  pow=  0 acc=100 GHOST   STAT pp=15  NIGHTMARE
  [ ] POISON_GAS                 pow=  0 acc= 90 POISON  STAT pp=40  POISON
  [ ] POISON_POWDER              pow=  0 acc= 75 POISON  STAT pp=35  POISON
  [ ] PSYCH_UP                   pow=  0 acc=  0 NORMAL  STAT pp=10  PSYCH_UP
  [ ] RAIN_DANCE            [TM] pow=  0 acc=  0 WATER   STAT pp= 5  RAIN_DANCE
  [ ] REFLECT               [TM] pow=  0 acc=  0 PSYCHIC STAT pp=20  REFLECT
  [ ] REFRESH                    pow=  0 acc=100 NORMAL  STAT pp=20  REFRESH
  [ ] ROAR                  [TM] pow=  0 acc=100 NORMAL  STAT pp=20  ROAR
  [ ] ROLE_PLAY                  pow=  0 acc=100 PSYCHIC STAT pp=10  ROLE_PLAY
  [ ] SAFEGUARD             [TM] pow=  0 acc=  0 NORMAL  STAT pp=25  SAFEGUARD
  [ ] SANDSTORM             [TM] pow=  0 acc=  0 ROCK    STAT pp=10  SANDSTORM
  [ ] SCREECH                    pow=  0 acc= 85 NORMAL  STAT pp=40  DEFENSE_DOWN_2
  [ ] SHARPEN                    pow=  0 acc=  0 NORMAL  STAT pp=30  ATTACK_UP
  [ ] SING                       pow=  0 acc= 55 NORMAL  STAT pp=15  SLEEP
  [ ] SLEEP_POWDER               pow=  0 acc= 75 GRASS   STAT pp=15  SLEEP
  [ ] SLEEP_TALK                 pow=  0 acc=  0 NORMAL  STAT pp=10  SLEEP_TALK
  [ ] SPIDER_WEB                 pow=  0 acc=100 BUG     STAT pp=10  MEAN_LOOK
  [ ] SPITE                      pow=  0 acc=100 GHOST   STAT pp=10  SPITE
  [ ] STRING_SHOT                pow=  0 acc= 95 BUG     STAT pp=40  SPEED_DOWN_2
  [ ] STUN_SPORE                 pow=  0 acc= 75 GRASS   STAT pp=30  PARALYZE
  [ ] SUNNY_DAY             [TM] pow=  0 acc=  0 FIRE    STAT pp= 5  SUNNY_DAY
  [ ] SWAGGER                    pow=  0 acc= 85 NORMAL  STAT pp=15  SWAGGER
  [ ] SWEET_KISS                 pow=  0 acc= 75 FAIRY   STAT pp=10  CONFUSE
  [ ] SWEET_SCENT                pow=  0 acc=100 NORMAL  STAT pp=20  EVASION_DOWN
  [ ] SYNTHESIS                  pow=  0 acc=  0 GRASS   STAT pp= 5  SYNTHESIS
  [ ] TAIL_GLOW                  pow=  0 acc=100 BUG     STAT pp=20  SPECIAL_ATTACK_UP_2
  [ ] TEETER_DANCE               pow=  0 acc=100 NORMAL  STAT pp=20  TEETER_DANCE
  [ ] TELEPORT                   pow=  0 acc=  0 PSYCHIC STAT pp=20  TELEPORT
  [ ] WHIRLWIND                  pow=  0 acc=100 NORMAL  STAT pp=20  ROAR
```

## Tier 5 — Bad (72)

```
  [ ] SKY_ATTACK                 pow=140 acc= 90 FLYING  PHYS pp= 5  SKY_ATTACK
  [ ] FOCUS_BLAST                pow=120 acc= 70 FIGHTINGSPEC pp= 5  SPECIAL_DEFENSE_DOWN
  [ ] MEGAHORN                   pow=120 acc= 85 BUG     PHYS pp=10  HIT
  [ ] ZAP_CANNON                 pow=120 acc= 50 ELECTRICSPEC pp= 5  PARALYZE_HIT
  [ ] SKULL_BASH                 pow=100 acc=100 NORMAL  PHYS pp=10  SKULL_BASH
  [ ] SPIT_UP                    pow=100 acc=100 NORMAL  SPEC pp=10  SPIT_UP
  [ ] PLAY_ROUGH                 pow= 90 acc= 90 FAIRY   PHYS pp=10  ATTACK_DOWN_HIT
  [ ] PSYCHIC               [TM] pow= 90 acc=100 PSYCHIC SPEC pp=10  SPECIAL_DEFENSE_DOWN
  [ ] SLUDGE_BOMB           [TM] pow= 90 acc=100 POISON  SPEC pp=10  POISON_HIT
  [ ] JUMP_KICK                  pow= 85 acc= 95 FIGHTINGPHYS pp=10  RECOIL_IF_MISS
  [ ] MEGA_PUNCH                 pow= 80 acc= 85 NORMAL  PHYS pp=20  HIT
  [ ] RAZOR_WIND                 pow= 80 acc=100 NORMAL  SPEC pp=10  RAZOR_WIND
  [ ] FIRE_PUNCH                 pow= 75 acc=100 FIRE    PHYS pp=15  BURN_HIT
  [ ] ICE_PUNCH                  pow= 75 acc=100 ICE     PHYS pp=15  FREEZE_HIT
  [ ] UPROAR                     pow= 70 acc=100 NORMAL  SPEC pp=10  UPROAR
  [ ] OCTAZOOKA                  pow= 65 acc= 85 WATER   SPEC pp=10  ACCURACY_DOWN_HIT
  [ ] SLUDGE                     pow= 65 acc=100 POISON  SPEC pp=20  POISON_HIT
  [ ] HIDDEN_POWER          [TM] pow= 60 acc=100 MYSTERY SPEC pp=15  HIDDEN_POWER
  [ ] NEEDLE_ARM                 pow= 60 acc=100 GRASS   PHYS pp=15  FLINCH_MINIMIZE_HIT
  [ ] ROLLING_KICK               pow= 60 acc= 85 FIGHTINGPHYS pp=15  FLINCH_HIT
  [ ] SHOCK_WAVE            [TM] pow= 60 acc=  0 ELECTRICSPEC pp=20  ALWAYS_HIT
  [ ] SILVER_WIND                pow= 60 acc=100 BUG     SPEC pp= 5  ALL_STATS_UP_HIT
  [ ] WATER_PULSE           [TM] pow= 60 acc=100 WATER   SPEC pp=20  CONFUSE_HIT
  [ ] WING_ATTACK                pow= 60 acc=100 FLYING  PHYS pp=35  HIT
  [ ] CUT                   [HM] pow= 50 acc= 95 GRASS   PHYS pp=30  HIT
  [ ] KARATE_CHOP                pow= 50 acc=100 FIGHTINGPHYS pp=25  HIGH_CRITICAL
  [ ] POISON_TAIL                pow= 50 acc=100 POISON  PHYS pp=25  POISON_TAIL
  [ ] STRUGGLE                   pow= 50 acc=100 NORMAL  PHYS pp= 1  RECOIL
  [ ] VINE_WHIP                  pow= 45 acc=100 GRASS   PHYS pp=25  HIT
  [ ] POWDER_SNOW                pow= 40 acc=100 ICE     SPEC pp=25  FREEZE_HIT
  [ ] THUNDER_SHOCK              pow= 40 acc=100 ELECTRICSPEC pp=30  PARALYZE_HIT
  [ ] PECK                       pow= 35 acc=100 FLYING  PHYS pp=35  HIT
  [ ] CONSTRICT                  pow= 25 acc=100 NORMAL  PHYS pp=35  SPEED_DOWN_HIT
  [ ] TWINEEDLE                  pow= 25 acc=100 BUG     PHYS pp=20  TWINEEDLE
  [ ] DOUBLE_SLAP                pow= 20 acc= 85 NORMAL  PHYS pp=10  MULTI_HIT
  [ ] PIN_MISSILE                pow= 20 acc= 95 BUG     PHYS pp=20  MULTI_HIT
  [ ] SPIKE_CANNON               pow= 20 acc=100 NORMAL  PHYS pp=15  MULTI_HIT
  [ ] BIND                       pow= 15 acc= 85 NORMAL  PHYS pp=20  TRAP
  [ ] FURY_ATTACK                pow= 15 acc= 85 NORMAL  PHYS pp=20  MULTI_HIT
  [ ] DRAGON_RAGE                pow=  1 acc=100 DRAGON  SPEC pp=10  DRAGON_RAGE
  [ ] LOW_KICK                   pow=  1 acc=100 FIGHTINGPHYS pp=20  LOW_KICK
  [ ] MAGNITUDE                  pow=  1 acc=100 GROUND  PHYS pp=30  MAGNITUDE
  [ ] PRESENT                    pow=  1 acc= 90 NORMAL  PHYS pp=15  PRESENT
  [ ] PSYWAVE                    pow=  1 acc=100 PSYCHIC SPEC pp=15  PSYWAVE
  [ ] SONIC_BOOM                 pow=  1 acc= 90 NORMAL  SPEC pp=20  SONICBOOM
  [ ] ATTRACT               [TM] pow=  0 acc=100 NORMAL  STAT pp=15  ATTRACT
  [ ] CHARM                      pow=  0 acc=100 FAIRY   STAT pp=20  ATTACK_DOWN_2
  [ ] DEFENSE_CURL               pow=  0 acc=  0 NORMAL  STAT pp=40  DEFENSE_CURL
  [ ] GRASS_WHISTLE              pow=  0 acc= 55 GRASS   STAT pp=15  SLEEP
  [ ] HAIL                  [TM] pow=  0 acc=  0 ICE     STAT pp=10  HAIL
  [ ] HYPNOSIS                   pow=  0 acc= 60 PSYCHIC STAT pp=20  SLEEP
  [ ] KINESIS                    pow=  0 acc= 80 PSYCHIC STAT pp=15  ACCURACY_DOWN
  [ ] MEDITATE                   pow=  0 acc=  0 PSYCHIC STAT pp=40  ATTACK_UP
  [ ] METRONOME                  pow=  0 acc=  0 NORMAL  STAT pp=10  METRONOME
  [ ] MILK_DRINK                 pow=  0 acc=  0 NORMAL  STAT pp=10  SOFTBOILED
  [ ] MIMIC                      pow=  0 acc=100 NORMAL  STAT pp=10  MIMIC
  [ ] MIRROR_MOVE                pow=  0 acc=  0 FLYING  STAT pp=20  MIRROR_MOVE
  [ ] MUD_SPORT                  pow=  0 acc=100 GROUND  STAT pp=15  MUD_SPORT
  [ ] NATURE_POWER               pow=  0 acc= 95 NORMAL  STAT pp=20  NATURE_POWER
  [ ] ODOR_SLEUTH                pow=  0 acc=100 NORMAL  STAT pp=40  FORESIGHT
  [ ] SAND_ATTACK                pow=  0 acc=100 GROUND  STAT pp=15  ACCURACY_DOWN
  [ ] SKETCH                     pow=  0 acc=  0 NORMAL  STAT pp= 1  SKETCH
  [ ] SKILL_SWAP            [TM] pow=  0 acc=100 PSYCHIC STAT pp=10  SKILL_SWAP
  [ ] SNATCH                [TM] pow=  0 acc=100 DARK    STAT pp=10  SNATCH
  [ ] SPLASH                     pow=  0 acc=  0 NORMAL  STAT pp=40  SPLASH
  [ ] STOCKPILE                  pow=  0 acc=  0 NORMAL  STAT pp=10  STOCKPILE
  [ ] SUPERSONIC                 pow=  0 acc= 55 NORMAL  STAT pp=20  CONFUSE
  [ ] SWALLOW                    pow=  0 acc=  0 NORMAL  STAT pp=10  SWALLOW
  [ ] TAIL_WHIP                  pow=  0 acc=100 NORMAL  STAT pp=30  DEFENSE_DOWN
  [ ] TRANSFORM                  pow=  0 acc=  0 NORMAL  STAT pp=10  TRANSFORM
  [ ] WATER_SPORT                pow=  0 acc=100 WATER   STAT pp=15  WATER_SPORT
  [ ] WITHDRAW                   pow=  0 acc=  0 WATER   STAT pp=40  DEFENSE_UP
```

## Tier 6 — Pokemon Homeless (19)

```
  [ ] EXPLOSION                     pow=250 acc=100 NORMAL  PHYS  pp= 5  EXPLOSION
  [ ] SELF_DESTRUCT                 pow=200 acc=100 NORMAL  PHYS  pp= 5  EXPLOSION
  [ ] HI_JUMP_KICK                  pow=100 acc= 90 FIGHTINGPHYS  pp=10  RECOIL_IF_MISS
  [ ] JUMP_KICK                     pow= 85 acc= 95 FIGHTINGPHYS  pp=10  RECOIL_IF_MISS
  [ ] SUBMISSION                    pow= 80 acc= 80 FIGHTINGPHYS  pp=20  RECOIL
  [ ] THUNDER_PUNCH                 pow= 75 acc=100 ELECTRICPHYS  pp=15  PARALYZE_HIT
  [ ] STRUGGLE                      pow= 50 acc=100 NORMAL  PHYS  pp= 1  RECOIL
  [ ] DOUBLE_KICK                   pow= 30 acc=100 FIGHTINGPHYS  pp=30  DOUBLE_HIT
  [ ] SMOG                          pow= 30 acc= 70 POISON  SPEC  pp=20  POISON_HIT
  [ ] FISSURE                       pow=  1 acc= 30 GROUND  PHYS  pp= 5  OHKO
  [ ] GUILLOTINE                    pow=  1 acc= 30 NORMAL  PHYS  pp= 5  OHKO
  [ ] HORN_DRILL                    pow=  1 acc= 30 NORMAL  PHYS  pp= 5  OHKO
  [ ] SHEER_COLD                    pow=  1 acc= 30 ICE     SPEC  pp= 5  OHKO
  [ ] CURSE                         pow=  0 acc=  0 MYSTERY STAT  pp=10  CURSE
  [ ] DOUBLE_TEAM              [TM] pow=  0 acc=  0 NORMAL  STAT  pp=15  EVASION_UP
  [ ] MEMENTO                       pow=  0 acc=100 DARK    STAT  pp=10  MEMENTO
  [ ] PERISH_SONG                   pow=  0 acc=  0 NORMAL  STAT  pp= 5  PERISH_SONG
  [ ] SMOKESCREEN                   pow=  0 acc=100 NORMAL  STAT  pp=20  ACCURACY_DOWN
  [ ] TRICK                         pow=  0 acc=100 PSYCHIC STAT  pp=10  TRICK
```

