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
// Selection modes, matching pokeemerald_rando_enh:
//   RZ_TIER_OFF      every entry equally likely
//   RZ_TIER_WEIGHTED good entries come up more often, but anything can still appear
//   RZ_TIER_STRICT   draw only from the top RZ_STRICT_TIERS bands. Expect heavy
//                    repetition - a six-Pokemon party will duplicate moves.
#define RZ_TIER_OFF                 0
#define RZ_TIER_WEIGHTED            1
#define RZ_TIER_STRICT              2

// How many top bands Strict draws from.
#define RZ_STRICT_TIERS             2

#define RZ_TIER_MODE_MOVES          RZ_TIER_WEIGHTED
#define RZ_TIER_MODE_ABILITIES      RZ_TIER_WEIGHTED
#define RZ_TIER_MODE_ITEMS          RZ_TIER_WEIGHTED
#define RZ_TIER_MODE_TMS            RZ_TIER_WEIGHTED
#define RZ_TIER_MODE_BERRIES        RZ_TIER_WEIGHTED

// Kept so existing checks still read naturally.
#define RZ_TIER_WEIGHTED_MOVES      (RZ_TIER_MODE_MOVES != RZ_TIER_OFF)
#define RZ_TIER_WEIGHTED_ABILITIES  (RZ_TIER_MODE_ABILITIES != RZ_TIER_OFF)

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
#define RZ_TIER_WEIGHTED_ITEMS     (RZ_TIER_MODE_ITEMS != RZ_TIER_OFF)
#define RZ_ITEM_W_T1             118
#define RZ_ITEM_W_T2             4792
#define RZ_ITEM_W_T3             4146
#define RZ_ITEM_W_T4             537
#define RZ_ITEM_W_T5             407

// Share of ordinary field-item pickups that become a TM instead of an item, x100.
// Randolocke wants TMs to be a common find; with I_REUSABLE_TMS on they are permanent.
#define RZ_ITEM_W_TM_BAND          3000

// Which TM, once the TM band is chosen, and which move a TM teaches when TM moves are
// randomized. Separate from the move weights: a TM is permanent under I_REUSABLE_TMS, so
// the band leans much harder toward good moves than a one-off move roll does. Bad and
// Pokemon Homeless are not in the pool at all.
//
// Tuned against the actual algorithm rather than by hand - duplicate rejection distorts
// the naive share, badly for Staples, where 26 of the band's 46 moves end up used. Over
// the 50 TMs this lands at about 3 Meta Defining (of the 4 that exist), 26 Staples,
// 16 Filler and 5 Niche. Per *move* that is 14.3x / 8.0x / 0.78x / 0.13x uniform.
// Re-run tools/randolocke/tm_band_sim.py after changing these.
#define RZ_TM_W_META_DEFINING       900
#define RZ_TM_W_STAPLES            5800
#define RZ_TM_W_FILLER             2500
#define RZ_TM_W_NICHE               800

// --- Trainer EVs ------------------------------------------------------------

// Not one of the 856 trainers in trainers.party specifies EVs, so every trainer Pokemon
// in vanilla Emerald -- gym leaders included -- runs on zero. The player has no EV cap
// and can train freely, which turns any boss into a pushover the moment you bother.
//
// If TRUE, trainers are given an EV spread that grows with your badge count, applied to
// HP, Speed, and whichever of the attacking and defending pairs the Pokemon is actually
// better at. That last part matters here: the species is randomized, so a fixed spread
// would land on the wrong stats half the time. Ported from pokeemerald_rando_enh.
#define RZ_TRAINER_EV_SCALING       TRUE

// EVs per stat, by badges earned, given to TWO stats. 252 + 252 + the 6 left over is 510,
// the most any Pokemon may legally hold and exactly the budget the player is held to --
// the summary screen's editor enforces the same. The old spread put a smaller number on
// four stats, which at eight badges came to 512, marginally over the player's limit and
// spread too thin to be felt; two stats at the cap is both legal and the shape that makes
// a boss frightening. The first three rows are the old totals, so the early gyms are where
// they were.
#define RZ_TRAINER_EVS_BY_BADGE   { 24, 48, 72, 100, 140, 180, 220, 252, 252 }

// Which two stats. One is an attacking stat the species can actually use -- it is
// randomized, so this is read off its base stats rather than fixed. The other is Speed if
// the species is fast enough for that to be worth 252, and HP if it is not: 252 Speed on a
// Shuckle is 252 EVs thrown away. The threshold is the median base Speed of every species
// in the game, measured, so it splits the roster down the middle.
#define RZ_TRAINER_EV_SPEED_THRESHOLD   67

// --- Trainer IVs --------------------------------------------------------------

// Vanilla gives a trainer one flat IV value for every stat of every Pokemon, scaled by how
// important the trainer is. Measured over trainers.party: of the 1570 Pokemon belonging to
// ordinary trainers, 40% run 0 across the board and most of the rest 1 to 12, while of the
// 255 belonging to the 55 `Boss: Yes` trainers, 70% are already perfect and the other 30%
// sit between 6 and 30.
//
// If TRUE, a boss's Pokemon are perfect, all six stats, all of them -- a gym leader should
// not be fighting you with a 6 IV Pokemon -- and everyone else rolls each stat separately
// between 0 and 31 instead of carrying the same number six times. That averages 15.5 a
// stat against the 0 to 3 most of them have now, so ordinary trainers gain the most here.
//
// Rolled from the trainer and the party slot, so a trainer's Pokemon are the same every
// time you meet them. The IVs line in trainers.party is left in place but no longer read
// for anyone this touches.
#define RZ_TRAINER_IVS              TRUE

// --- Trainer natures ----------------------------------------------------------

// Not one of the 1825 trainer Pokemon in trainers.party specifies a Nature either, so all
// of them fight on Hardy: neither stat raised nor lowered. A nature is a flat 10% on two
// stats, and the player's Pokemon have one.
//
// If TRUE, a trainer's Pokemon is given the nature a player would have picked for it, on
// the same reading of its base stats the EVs use: the fast ones trade their unused
// attacking stat for Speed (Jolly, Timid), the slow ones trade it for power (Adamant,
// Modest). Nothing a Pokemon actually uses is ever the stat that drops.
#define RZ_TRAINER_NATURES          TRUE

// --- Trainer held items -------------------------------------------------------

// 142 of the 1825 carry an item, and most of those are the in-battle restores the AI
// throws rather than something held. If TRUE, a Pokemon that has no item of its own is
// given one from a small list of battle items that suit any species -- Leftovers, Sitrus,
// Lum, Focus Band, Life Orb and so on, plus the damage booster matching the category it
// attacks from. Items written into trainers.party are left alone.
//
// Choice items are deliberately not on the list: they lock the holder into one move, and
// an AI that mishandles that is easier to beat, not harder.
//
// The roll is seeded from the trainer and the party slot, like every other randomizer
// decision, so a trainer's items are the same every time you meet them.
#define RZ_TRAINER_HELD_ITEMS       TRUE

// Percent chance of being given one. Bosses -- the 55 `Boss: Yes` trainers -- always are.
#define RZ_TRAINER_ITEM_CHANCE          35
#define RZ_TRAINER_ITEM_CHANCE_BOSS    100

// --- Trainer AI ---------------------------------------------------------------

// Measured from trainers.party: 640 trainers run Check Bad Move alone and 173 run Basic
// Trainer -- and every boss is in the second group. Archie, Matt, Shelly and the gym
// leaders all fight with the weakest AI in the game, which is the single largest
// difficulty gap left once the levels and EVs are scaled.
//
// If TRUE, AI flags are raised at battle start by trainer importance rather than edited
// into the data file, so the tiers stay one readable place and trainers.party stays a
// diff of levels and parties.
//
//   every trainer   Check Bad Move, Try To Faint, Check Viability
//   notable         + HP Aware, Smart Mon Choices, Try To 2HKO
//   boss            + Smart Switching, Ace Pokemon, Omniscient
//   Champion        + move and switch prediction
//
// Omniscience means the AI knows your moves, abilities and held items without having seen
// them: it will not Surf into a Water Absorb it has never met, and it will not set up on
// something that outspeeds and KOs it. All 55 boss trainers get it -- the gym leaders,
// the Elite Four, Archie, Maxie and their admins. Weigh Ability Prediction is dropped
// from the boss tier because omniscience supersedes it.
//
// "Notable" is by trainer class: rivals, the Aqua and Magma admins, and the Elite Four.
// Bosses are the `Boss: Yes` tag from Phase 7c.
#define RZ_TRAINER_AI_TIERS         TRUE

#define RZ_AI_BASE      (AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_TRY_TO_FAINT | AI_FLAG_CHECK_VIABILITY)
#define RZ_AI_NOTABLE   (RZ_AI_BASE | AI_FLAG_HP_AWARE | AI_FLAG_SMART_MON_CHOICES | AI_FLAG_TRY_TO_2HKO)
#define RZ_AI_BOSS      (RZ_AI_NOTABLE | AI_FLAG_SMART_SWITCHING | AI_FLAG_ACE_POKEMON \
                         | AI_FLAG_OMNISCIENT)
// The Champion also reads ahead: which move you are about to use, and when you are about
// to switch and to what. Both flags are documented as wanting omniscience, which it has.
#define RZ_AI_CHAMPION  (RZ_AI_BOSS | AI_FLAG_PREDICT_MOVE | AI_FLAG_PREDICT_SWITCH \
                         | AI_FLAG_PREDICT_INCOMING_MON)

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

// --- TM moves ---------------------------------------------------------------

// Reassigns what each TM teaches. Drawn through the RZ_TM_W_* bands above, not the move
// bands: Bad and Pokemon Homeless are excluded outright, exactly as they are from the
// vanilla TM list, because a TM is permanent under I_REUSABLE_TMS. No duplicates - a
// second TM for a move you already have is dead weight. Safe here because Phase 6 set
// ALL_TEACHABLES globally, so no Pokemon's compatibility depends on which move a given
// TM carries. HMs are never touched.
//
// Note this also takes over the TM *pickup* weighting: with TM moves reassigned, the
// vanilla TM->tier grouping in sTmTiers no longer describes anything, so a found TM is
// picked uniformly and the spread comes from this assignment instead.
#define RZ_TM_MOVES_TIER_MODE       RZ_TIER_MODE_MOVES

// Reassigns what each of Emerald's ten move tutors teaches. Drawn through the same bands
// as TMs, with no duplicates and no overlap with the TM list -- a tutor that teaches a
// move you can already buy on a reusable TM is a wasted tutor.
#define RZ_TUTOR_MOVES_TIER_MODE    RZ_TM_MOVES_TIER_MODE

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
// If TRUE, each of the three move groups is sorted by Base Power so stronger moves are
// learned later -- Randolocke's default. FALSE fills the 21 slots in whatever order they
// roll, which is the older, wilder behaviour.
#define RZ_LEARNSET_SORT_BY_POWER   TRUE

// A trainer Pokemon whose species was substituted gets a fresh moveset from its level-up
// learnset instead of the moves written for the species it replaced. Without this, a
// randomized gym leader's whole team carries the original team's moves -- no same-type
// attacks, and every Pokemon on the team fighting the same way.
#define RZ_TRAINER_REGENERATE_MOVES TRUE

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

#ifndef FORCE_RANDOMIZE_TM_MOVES
#define RANDOMIZER_FLAG_TM_MOVES                      FLAG_UNUSED_0x02A
#endif

#ifndef FORCE_RANDOMIZE_TUTOR_MOVES
#define RANDOMIZER_FLAG_TUTOR_MOVES                   FLAG_UNUSED_0x02E
#endif

#define RANDOMIZER_VAR_SPECIES_MODE                   VAR_UNUSED_0x404E

#if RANDOMIZER_SEED_IS_TRAINER_ID == FALSE
#define RANDOMIZER_VAR_SEED_L                         VAR_UNUSED_0x40FA
#define RANDOMIZER_VAR_SEED_H                         VAR_UNUSED_0x40FB
#endif

#endif // RANDOMIZER_AVAILABLE

#endif // GUARD_CONFIG_RANDOMIZER_H
