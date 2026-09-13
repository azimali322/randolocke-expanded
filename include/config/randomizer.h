#ifndef GUARD_CONFIG_RANDOMIZER_H
#define GUARD_CONFIG_RANDOMIZER_H
#include "item.h"

// Global control. If FALSE, no randomizer functionality will be enabled.
// If this is TRUE, that doesn't necessarily mean that a particular part of the randomizer
// will be enabled.
#define RANDOMIZER_AVAILABLE                   TRUE
#define RANDOMIZER_SEED_IS_TRAINER_ID          TRUE

#if RANDOMIZER_AVAILABLE == TRUE

// If TRUE, the trainer ID (including secret ID) will be the randomizer seed.
#define RZ_TRAINER_ID_IS_SEED       TRUE

// If TRUE, dynamically generated randomization tables stored in EWRAM are used.
// This consumes 6 bytes for each species present.
#define RANDOMIZER_DYNAMIC_SPECIES    TRUE

#if RANDOMIZER_DYNAMIC_SPECIES == TRUE

// If the longest evolutionary chain (excluding babies) is longer than this,
// the dynamic evolutionary stage randomization table will be generated
// incorrectly.
#define RANDOMIZER_MAX_EVO_STAGES   5

#endif // RANDOMIZER_DYNAMIC_SPECIES

// If TRUE, a Pokemon's randomized ability is decided by the root of its evolution
// family rather than by its current species, so evolving no longer rerolls the
// ability. Note this is separate from turning ability randomization off entirely
// (RANDOMIZER_FLAG_ABILITIES): abilities are still randomized, just stably.
#define RZ_ABILITY_STABLE_ACROSS_EVOLUTION   TRUE

// If TRUE, trainers tagged `Boss: Yes` in trainers.party (gym leaders, the Elite Four,
// the Champion, Magma/Aqua leaders and admins) are randomized like anyone else. Set to
// FALSE to keep their hand-designed teams and real abilities as fixed landmarks in an
// otherwise randomized game.
#define RZ_RANDOMIZE_BOSS_TRAINERS           TRUE

// --- Tier-weighted selection ------------------------------------------------

// When TRUE, randomized moves and abilities are drawn from community tier lists with the
// weights below rather than uniformly. See docs/tiering/ and RANDOLOCKE_PLAN.md Phase 10.
// Re-run tools/randolocke/tier_report.py after changing any weight: a tier's weight is
// split across its members, so a large tier dilutes itself and it is easy to make a lower
// tier out-draw a higher one by accident.
#define RZ_TIER_WEIGHTED_MOVES      TRUE
#define RZ_TIER_WEIGHTED_ABILITIES  TRUE

// Ability weights, x100 so they stay integers. Pool of 308; 2.31x down to 0.34x uniform.
#define RZ_ABILITY_W_S              900
#define RZ_ABILITY_W_A             2000
#define RZ_ABILITY_W_B             3200
#define RZ_ABILITY_W_C             2400
#define RZ_ABILITY_W_D             1100
#define RZ_ABILITY_W_F              400
#define RZ_ABILITY_W_NEGATIVE         0   // never rolled

// Move weights, x100. Pool of 846 (every rollable move is tiered). Compressed at the
// top: 2.49x / 2.00x / 1.75x / 0.88x / 0.24x / 0.15x against uniform.
#define RZ_MOVE_W_META_DEFINING  118
#define RZ_MOVE_W_STAPLES        1090
#define RZ_MOVE_W_FILLER         4209
#define RZ_MOVE_W_NICHE          4015
#define RZ_MOVE_W_BAD            529
#define RZ_MOVE_W_HOMELESS       39

// Item weights, x100. Pool of 475. No community list exists for items, so tiers 1-2 are
// hand-graded from pokeemerald_rando_enh and the rest are placed by heuristic; see
// tools/randolocke/gen_item_tiers.py. 1.71x / 1.65x / 1.52x / 0.45x / 0.35x vs uniform:
// compressed at the top because tiers 4-5 hold 48% of the pool at the floor.
#define RZ_TIER_WEIGHTED_ITEMS     TRUE
#define RZ_ITEM_W_T1             72
#define RZ_ITEM_W_T2             2953
#define RZ_ITEM_W_T3             5184
#define RZ_ITEM_W_T4             568
#define RZ_ITEM_W_T5             1223

// --- Learnset randomization -------------------------------------------------

// Every Pokemon learns the same 21 moves at the same levels: 7 STAB, 7 status and
// 7 non-STAB damaging, with higher Base Power learned later. Gated at runtime by
// RANDOMIZER_FLAG_LEARNSET; with the flag clear you get vanilla learnsets.
#define RZ_LEARNSET_STAB_MOVES      7
#define RZ_LEARNSET_STATUS_MOVES    7
#define RZ_LEARNSET_DAMAGING_MOVES  7
#define RZ_LEARNSET_SLOTS  (RZ_LEARNSET_STAB_MOVES + RZ_LEARNSET_STATUS_MOVES + RZ_LEARNSET_DAMAGING_MOVES)

// The level each of the 21 slots is learned at. Front-loaded so early Pokemon are
// not moveless, with the last few above the pre-Elite-Four cap of 63.
#define RZ_LEARNSET_LEVELS  { 1, 4, 7, 10, 13, 16, 20, 24, 28, 32, 36, 40, 44, 48, 52, 56, 60, 64, 70, 78, 86 }

#define RANDOMIZER_MAX_TM           ITEM_TM50

// Vars and features

// These features allow you to force enable or disable individual randomization
// features.
// If undefined, the feature will be enabled if one of the flags below is set.
// If defined and set to TRUE, the feature will always be enabled.
// If defined and set to FALSE, the feature will always be disabled.
//#define FORCE_RANDOMIZE_WILD_MON                  TRUE
//#define FORCE_RANDOMIZE_FIELD_ITEMS               TRUE
//#define FORCE_RANDOMIZE_TRAINER_MON               TRUE
//#define FORCE_RANDOMIZE_FIXED_MON                 TRUE
//#define FORCE_RANDOMIZE_STARTER_AND_GIFT_MON      TRUE
//#define FORCE_RANDOMIZE_EGG_MON                   TRUE
//#define FORCE_RANDOMIZE_ABILITIES                 TRUE

// These flags control whether a particular randomization feature is active.
// They are ignored and disabled if the flags above are set.
#ifndef FORCE_RANDOMIZE_WILD_MON
#define RANDOMIZER_FLAG_WILD_MON                      FLAG_UNUSED_0x020
#endif

#ifndef FORCE_RANDOMIZE_FIELD_ITEMS
#define RANDOMIZER_FLAG_FIELD_ITEMS                   FLAG_UNUSED_0x021
#endif

#ifndef FORCE_RANDOMIZE_TRAINER_MON
#define RANDOMIZER_FLAG_TRAINER_MON                   FLAG_UNUSED_0x022
#endif

#ifndef FORCE_RANDOMIZE_FIXED_MON
#define RANDOMIZER_FLAG_FIXED_MON                     FLAG_UNUSED_0x023
#endif

#ifndef FORCE_RANDOMIZE_STARTER_AND_GIFT_MON
#define RANDOMIZER_FLAG_STARTER_AND_GIFT_MON          FLAG_UNUSED_0x024
#endif

#ifndef FORCE_RANDOMIZE_EGG_MON
#define RANDOMIZER_FLAG_EGG_MON                       FLAG_UNUSED_0x025
#endif

#ifndef FORCE_RANDOMIZE_ABILITIES
#define RANDOMIZER_FLAG_ABILITIES                     FLAG_UNUSED_0x026
#endif

#ifndef FORCE_RANDOMIZE_LEARNSET
#define RANDOMIZER_FLAG_LEARNSET                      FLAG_UNUSED_0x028
#endif

#define RANDOMIZER_VAR_SPECIES_MODE                   VAR_UNUSED_0x404E

#if RANDOMIZER_SEED_IS_TRAINER_ID == FALSE
#define RANDOMIZER_VAR_SEED_L                         VAR_UNUSED_0x40FA
#define RANDOMIZER_VAR_SEED_H                         VAR_UNUSED_0x40FB
#endif

#endif // RANDOMIZER_AVAILABLE

#endif // GUARD_CONFIG_RANDOMIZER_H
