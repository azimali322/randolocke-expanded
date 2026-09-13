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
// top: 2.25x / 2.00x / 1.75x / 0.89x / 0.24x / 0.15x against uniform.
#define RZ_MOVE_W_META_DEFINING  107
#define RZ_MOVE_W_STAPLES        1090
#define RZ_MOVE_W_FILLER         4209
#define RZ_MOVE_W_NICHE          4026
#define RZ_MOVE_W_BAD            529
#define RZ_MOVE_W_HOMELESS       39

// Item weights, x100. Pool of 475. No community list exists for items, so tiers 1-2 are
// hand-graded from pokeemerald_rando_enh and the rest are placed by heuristic; see
// tools/randolocke/gen_item_tiers.py. Pool of 408; berries are excluded because they are
// randomized separately at berry trees. 2.41x / 2.30x / 2.06x / 0.30x / 0.10x vs uniform.
// Tier 4 is Poke Balls and evolution items - both sold cheaply in the Phase 11 shop, so
// finding one is not a reward. Tier 5 is healing, vitamins, X items and the mega/Z/Tera
// gear Phase 6 disabled; at 0.10x it is effectively off.
#define RZ_TIER_WEIGHTED_ITEMS     TRUE
#define RZ_ITEM_W_T1             118
#define RZ_ITEM_W_T2             4792
#define RZ_ITEM_W_T3             4146
#define RZ_ITEM_W_T4             537
#define RZ_ITEM_W_T5             407

// Share of ordinary field-item pickups that become a TM instead of an item, x100.
// Randolocke wants TMs to be a common find; with I_REUSABLE_TMS on they are permanent.
#define RZ_ITEM_W_TM_BAND          3000

// Which TM, once the TM band is chosen. These are separate from the move weights: a TM is
// permanent under I_REUSABLE_TMS, so the band leans much harder toward good moves than a
// one-off move roll does. TMs whose move is Bad or Pokemon Homeless are not in the pool
// at all. Over 40 eligible TMs this gives roughly 4.0x / 2.4x / 1.05x / 0.27x uniform.
#define RZ_TM_W_META_DEFINING      1000
#define RZ_TM_W_STAPLES            3000
#define RZ_TM_W_FILLER             5000
#define RZ_TM_W_NICHE              1000

// --- Berry trees ------------------------------------------------------------

// Berries are randomized where they are found, at berry trees, rather than in the field
// item pool. Weights x100 over 68 berries: 2.20x / 1.90x / 1.24x / 0.60x / 0.25x uniform.
// Nuzlocke logic as elsewhere: in-battle HP restoration is near-worthless when the run
// carries a cheat heal item, so pinch-berry stat boosts and status cures rank above it,
// and the berries with no hold effect at all sit at the bottom.
#define RZ_BERRY_W_T1              1618
#define RZ_BERRY_W_T2              1676
#define RZ_BERRY_W_T3              5456
#define RZ_BERRY_W_T4              441
#define RZ_BERRY_W_T5              809

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

// If TRUE, a species' STAB moves are drawn in the damage category it can actually use: a
// physical attacker gets physical STAB, a special attacker special STAB. Without this a
// pure physical attacker can roll seven special STAB moves and be unable to use any of them.
#define RZ_STAB_MATCH_CATEGORY      TRUE

// How close base Attack and base Sp. Atk must be, as a percentage of the higher, for a
// species to count as a mixed attacker and draw STAB from both categories. 85 means a
// species whose weaker attacking stat is at least 85% of its stronger one is mixed.
#define RZ_MIXED_ATTACKER_PERCENT   85

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

#ifndef FORCE_RANDOMIZE_BERRY_TREES
#define RANDOMIZER_FLAG_BERRY_TREES                   FLAG_UNUSED_0x029
#endif

#define RANDOMIZER_VAR_SPECIES_MODE                   VAR_UNUSED_0x404E

#if RANDOMIZER_SEED_IS_TRAINER_ID == FALSE
#define RANDOMIZER_VAR_SEED_L                         VAR_UNUSED_0x40FA
#define RANDOMIZER_VAR_SEED_H                         VAR_UNUSED_0x40FB
#endif

#endif // RANDOMIZER_AVAILABLE

#endif // GUARD_CONFIG_RANDOMIZER_H
