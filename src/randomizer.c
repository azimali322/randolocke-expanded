#include "randomizer.h"
#include "move.h"
#include "constants/battle_move_effects.h"

#if RANDOMIZER_AVAILABLE == TRUE
#include "main.h"
#include "new_game.h"
#include "item.h"
#include "event_data.h"
#include "field_control_avatar.h"
#include "pokemon.h"
#include "script.h"
#include "data.h"
#include "data/randomizer/special_form_tables.h"
#include "constants/abilities.h"
#include "data/randomizer/ability_whitelist.h"
#include "data/randomizer/ability_tiers.h"
#include "data/randomizer/move_tiers.h"
#include "data/randomizer/item_tiers.h"
#include "data/randomizer/tm_tiers.h"
#include "data/randomizer/berry_tiers.h"
#include "constants/abilities.h"

// Add the mons you wish to be randomized when given as starter/gift mon to this list
const enum Species gStarterAndGiftMonTable[STARTER_AND_GIFT_MON_COUNT] =
{
    SPECIES_CYNDAQUIL,
    SPECIES_TOTODILE,
    SPECIES_CHIKORITA,
    SPECIES_TREECKO,
    SPECIES_TORCHIC,
    SPECIES_MUDKIP,
    SPECIES_BELDUM,
    SPECIES_CASTFORM_NORMAL,
    SPECIES_LILEEP,
    SPECIES_ANORITH,
};

// Every legendary encounter site in the game, keyed by the species that stands there in
// vanilla. RandomizeLegendaryMon assigns these twelve a legendary each, without
// replacement, so no two sites give the same Pokemon. Order matters only in that changing
// it reshuffles every existing seed's mapping.
//
// Jirachi is deliberately absent: it has no in-game encounter, so a slot for it would
// consume a legendary nobody can reach.
const enum Species gLegendaryMonTable[LEGENDARY_MON_COUNT] =
{
    SPECIES_RAYQUAZA,       // Sky Pillar          setwildbattle
    SPECIES_GROUDON,        // Terra Cave          setwildbattle
    SPECIES_KYOGRE,         // Marine Cave         setwildbattle
    SPECIES_REGIROCK,       // Desert Ruins        setwildbattle
    SPECIES_REGICE,         // Island Cave         setwildbattle
    SPECIES_REGISTEEL,      // Ancient Tomb        setwildbattle
    SPECIES_LATIAS,         // Southern Island / roamer
    SPECIES_LATIOS,         // Southern Island / roamer
    SPECIES_MEW,            // Faraway Island      seteventmon
    SPECIES_DEOXYS_NORMAL,  // Birth Island        seteventmon
    SPECIES_HO_OH,          // Navel Rock top      seteventmon
    SPECIES_LUGIA,          // Navel Rock bottom   seteventmon
};

// Add the mons you wish to be randomized when given as egg mon to this list
const enum Species gEggMonTable[EGG_MON_COUNT] =
{
    SPECIES_WYNAUT, 
};

// This is a list of baby Pokémon that should not cause their evolution
// to count as an evolved pokemon.
// XXX: put this somewhere else?
static const u16 sPreevolutionBabyMons[] =
{
    SPECIES_PICHU,
    SPECIES_CLEFFA,
    SPECIES_IGGLYBUFF,
    SPECIES_TYROGUE,
    SPECIES_SMOOCHUM,
    SPECIES_ELEKID,
    SPECIES_MAGBY,
    SPECIES_AZURILL,
    SPECIES_WYNAUT,
    SPECIES_BUDEW,
    SPECIES_CHINGLING,
    SPECIES_BONSLY,
    SPECIES_MIME_JR,
    SPECIES_HAPPINY,
    SPECIES_MUNCHLAX,
    SPECIES_MANTYKE,
};

bool32 RandomizerFeatureEnabled(enum RandomizerFeature feature)
{
    switch(feature)
    {
        case RANDOMIZE_WILD_MON:
            #ifdef FORCE_RANDOMIZE_WILD_MON
                return FORCE_RANDOMIZE_WILD_MON;
            #else
                return FlagGet(RANDOMIZER_FLAG_WILD_MON);
            #endif
        case RANDOMIZE_FIELD_ITEMS:
            #ifdef FORCE_RANDOMIZE_FIELD_ITEMS
                return FORCE_RANDOMIZE_FIELD_ITEMS;
            #else
                return FlagGet(RANDOMIZER_FLAG_FIELD_ITEMS);
            #endif
        case RANDOMIZE_TRAINER_MON:
            #ifdef FORCE_RANDOMIZE_TRAINER_MON
                return FORCE_RANDOMIZE_TRAINER_MON;
            #else
                return FlagGet(RANDOMIZER_FLAG_TRAINER_MON);
            #endif
        case RANDOMIZE_FIXED_MON:
            #ifdef FORCE_RANDOMIZE_FIXED_MON
                return FORCE_RANDOMIZE_FIXED_MON;
            #else
                return FlagGet(RANDOMIZER_FLAG_FIXED_MON);
            #endif
        case RANDOMIZE_STARTER_AND_GIFT_MON:
            #ifdef FORCE_RANDOMIZE_STARTER_AND_GIFT_MON
                return FORCE_RANDOMIZE_STARTER_AND_GIFT_MON;
            #else
                return FlagGet(RANDOMIZER_FLAG_STARTER_AND_GIFT_MON);
            #endif
        case RANDOMIZE_ABILITIES:
            #ifdef FORCE_RANDOMIZE_ABILITIES
                return FORCE_RANDOMIZE_ABILITIES;
            #else
                return FlagGet(RANDOMIZER_FLAG_ABILITIES);
            #endif
        case RANDOMIZE_BERRY_TREES:
            #ifdef FORCE_RANDOMIZE_BERRY_TREES
                return FORCE_RANDOMIZE_BERRY_TREES;
            #else
                return FlagGet(RANDOMIZER_FLAG_BERRY_TREES);
            #endif
        case RANDOMIZE_TM_MOVES:
            #ifdef FORCE_RANDOMIZE_TM_MOVES
                return FORCE_RANDOMIZE_TM_MOVES;
            #else
                return FlagGet(RANDOMIZER_FLAG_TM_MOVES);
            #endif
        case RANDOMIZE_LEARNSET:
            #ifdef FORCE_RANDOMIZE_LEARNSET
                return FORCE_RANDOMIZE_LEARNSET;
            #else
                return FlagGet(RANDOMIZER_FLAG_LEARNSET);
            #endif
        default:
            return FALSE;
    }
}

// --- Tier-weighted selection ------------------------------------------------
// A tier's weight is shared by its members, so the per-entry rate is weight/count. Picking
// proportionally to weight (not to size) is what makes a small top tier actually favoured.
// tools/randolocke/tier_report.py prints the resulting odds.

struct RzTier
{
    const u16 *entries;
    u16 count;
    u16 weight;     // x100
};

// Picks a tier in proportion to weight, then an entry uniformly inside it. `accept` may
// reject an entry (wrong type, already dealt, and so on); on repeated rejection the caller
// falls back to its own uniform path, so a filter that matches nothing cannot hang.
// Off means uniform across the whole pool, which is the same thing as weighting each
// band by how many entries it holds.
static inline u32 RzTierWeight(const struct RzTier *tier, u32 mode)
{
    return (mode == RZ_TIER_OFF) ? tier->count : tier->weight;
}

static u16 RzWeightedPickMode(struct Sfc32State *state, const struct RzTier *tiers,
                              u32 tierCount, bool32 (*accept)(u16, u32), u32 arg, u32 mode)
{
    u32 attempts;
    u32 total = 0;
    u32 i;

    // Strict ignores everything below the top bands, so a run draws only from the best.
    if (mode == RZ_TIER_STRICT && tierCount > RZ_STRICT_TIERS)
        tierCount = RZ_STRICT_TIERS;

    for (i = 0; i < tierCount; i++)
    {
        if (tiers[i].count != 0)
            total += RzTierWeight(&tiers[i], mode);
    }
    if (total == 0)
        return 0;

    for (attempts = 0; attempts < 64; attempts++)
    {
        u32 roll = RandomizerNextRange(state, total);
        u32 acc = 0;

        for (i = 0; i < tierCount; i++)
        {
            if (tiers[i].count == 0)
                continue;
            acc += RzTierWeight(&tiers[i], mode);
            if (roll < acc)
            {
                u16 pick = tiers[i].entries[RandomizerNextRange(state, tiers[i].count)];

                if (accept == NULL || accept(pick, arg))
                    return pick;
                break;
            }
        }
    }
    return 0;   // caller falls back
}

static u16 RzWeightedPick(struct Sfc32State *state, const struct RzTier *tiers, u32 tierCount,
                          bool32 (*accept)(u16, u32), u32 arg)
{
    return RzWeightedPickMode(state, tiers, tierCount, accept, arg, RZ_TIER_WEIGHTED);
}

#define RZ_TIER(arr, w) { (arr), ARRAY_COUNT(arr), (w) }

static const struct RzTier sAbilityTiers[] =
{
    RZ_TIER(sAbilityTierS,        RZ_ABILITY_W_S),
    RZ_TIER(sAbilityTierA,        RZ_ABILITY_W_A),
    RZ_TIER(sAbilityTierB,        RZ_ABILITY_W_B),
    RZ_TIER(sAbilityTierC,        RZ_ABILITY_W_C),
    RZ_TIER(sAbilityTierD,        RZ_ABILITY_W_D),
    RZ_TIER(sAbilityTierF,        RZ_ABILITY_W_F),
    RZ_TIER(sAbilityTierNegative, RZ_ABILITY_W_NEGATIVE),
};

static const struct RzTier sItemTiers[] =
{
    RZ_TIER(sItemTier1, RZ_ITEM_W_T1),
    RZ_TIER(sItemTier2, RZ_ITEM_W_T2),
    RZ_TIER(sItemTier3, RZ_ITEM_W_T3),
    RZ_TIER(sItemTier4, RZ_ITEM_W_T4),
    RZ_TIER(sItemTier5, RZ_ITEM_W_T5),
};

// TMs grouped by the tier of the move they teach, weighted with the move weights so a
// good TM is about as likely as a good move would be.
static const struct RzTier sBerryTiers[] =
{
    RZ_TIER(sBerryTier1, RZ_BERRY_W_T1),
    RZ_TIER(sBerryTier2, RZ_BERRY_W_T2),
    RZ_TIER(sBerryTier3, RZ_BERRY_W_T3),
    RZ_TIER(sBerryTier4, RZ_BERRY_W_T4),
    RZ_TIER(sBerryTier5, RZ_BERRY_W_T5),
};

static const struct RzTier sTmTiers[] =
{
    RZ_TIER(sTmTierMetaDefining, RZ_TM_W_META_DEFINING),
    RZ_TIER(sTmTierStaples,      RZ_TM_W_STAPLES),
    RZ_TIER(sTmTierFiller,       RZ_TM_W_FILLER),
    RZ_TIER(sTmTierNiche,        RZ_TM_W_NICHE),
};

static const struct RzTier sMoveTiers[] =
{
    RZ_TIER(sMoveTierMetaDefining, RZ_MOVE_W_META_DEFINING),
    RZ_TIER(sMoveTierStaples,      RZ_MOVE_W_STAPLES),
    RZ_TIER(sMoveTierFiller,       RZ_MOVE_W_FILLER),
    RZ_TIER(sMoveTierNiche,        RZ_MOVE_W_NICHE),
    RZ_TIER(sMoveTierBad,          RZ_MOVE_W_BAD),
    RZ_TIER(sMoveTierHomeless,     RZ_MOVE_W_HOMELESS),
};

// The bands a randomized TM may teach from. Bad and Pokemon Homeless are absent by
// design, mirroring what gen_tm_tiers.py does to the vanilla TM list: a TM is permanent
// under I_REUSABLE_TMS, so a bad one is dead weight in the bag forever. The TM weights
// rather than the move weights, for the same reason - a TM should lean much harder
// toward good moves than a one-off move roll does. Over 50 TMs this aims at roughly
// 5 Meta Defining, 15 Staples, 25 Filler and 5 Niche; only 4 Meta Defining moves exist,
// so in practice all four become TMs and the spare draw spills into Staples.
static const struct RzTier sTmMoveTiers[] =
{
    RZ_TIER(sMoveTierMetaDefining, RZ_TM_W_META_DEFINING),
    RZ_TIER(sMoveTierStaples,      RZ_TM_W_STAPLES),
    RZ_TIER(sMoveTierFiller,       RZ_TM_W_FILLER),
    RZ_TIER(sMoveTierNiche,        RZ_TM_W_NICHE),
};

u32 GetRandomizerSeed(void)
{
    #if RANDOMIZER_SEED_IS_TRAINER_ID == TRUE
        return GetTrainerId(gSaveBlock2Ptr->playerTrainerId);
    #else
        u32 result;
        result = ((u32)VarGet(RANDOMIZER_VAR_SEED_H) << 16) | VarGet(RANDOMIZER_VAR_SEED_L);
        return result;
    #endif
}

// Sets the seed that will be used for the randomizer if doing so is possible.
bool32 SetRandomizerSeed(u32 newSeed)
{
    #if RANDOMIZER_SEED_IS_TRAINER_ID == TRUE
        // It isn't possible to set the randomizer seed in this case.
        return FALSE;
    #else
        VarSet(RANDOMIZER_VAR_SEED_L, (u16)newSeed);
        VarSet(RANDOMIZER_VAR_SEED_H, (u16)(newSeed >> 16));
        return TRUE;
    #endif
}

static bool32 IsSpeciesPermitted(u16 species)
{
    // SPECIES_NONE is never valid.
    if (species == SPECIES_NONE)
        return FALSE;
    // This is used to indicate a disabled species.
    if (gSpeciesInfo[species].baseHP == 0)
        return FALSE;
    if (gSpeciesInfo[species].randomizerMode == MON_RANDOMIZER_INVALID)
        return FALSE;

    return TRUE;
};

u32 GenerateSeedForRandomizer(void)
{
    u32 data;
    const u32 vblankCounter = gMain.vblankCounter1;
    data = Random32();
    return data ^ vblankCounter;
}

u16 GetRandomizerOption(enum RandomizerOption option)
{
    switch(option) {
        case RANDOMIZER_OPTION_SPECIES_MODE:
            return VarGet(RANDOMIZER_VAR_SPECIES_MODE);
        default: // Unknown option.
            return 0;
    }
}

/* Seeds an SFC32 random number generator state for the randomizer.

SFC32 can be seeded with up to 96 bits of data.
32 are used for the randomizer reason, which is mixed with the seed.
data1 and data2 are 64 bits of data that a caller can use for
any purpose they wish. Certain groups of functions will assign a purpose to
these: for example, RandomizeMon uses data2 for the original species number.
data2 is also mixed with the seed.
*/
struct Sfc32State RandomizerRandSeed(enum RandomizerReason reason, u32 data1, u32 data2)
{
    struct Sfc32State state;
    u32 i;
    const u32 randomizerSeed = GetRandomizerSeed();

    state.a = randomizerSeed + (u32)reason;
    state.b = randomizerSeed ^ data2;
    state.c = data1;
    state.ctr = RANDOMIZER_STREAM;

    for (i = 0; i < 10; i++)
    {
        _SFC32_Next_Stream(&state, RANDOMIZER_STREAM);
    }

    return state;
}


// This uses a slightly accelerated bitmasking method.
u32 RandomizerNextRange(struct Sfc32State* state, u32 range)
{
    u32 next_power_of_two, mask, result;
    if (range < 2)
        return 0;
    else if (range == UINT32_MAX)
        return _SFC32_Next_Stream(state, RANDOMIZER_STREAM);

    next_power_of_two = range;
    --next_power_of_two;
    next_power_of_two |= next_power_of_two >> 1;
    next_power_of_two |= next_power_of_two >> 2;
    next_power_of_two |= next_power_of_two >> 4;
    next_power_of_two |= next_power_of_two >> 8;
    ++next_power_of_two;

    mask = next_power_of_two - 1;

    do
    {
        result = _SFC32_Next_Stream(state, RANDOMIZER_STREAM) & mask;
    } while (result >= range);

    return result;
}

// Functions for producing single seeded random numbers.
u16 RandomizerRand(enum RandomizerReason reason, u32 data1, u32 data2)
{
    struct Sfc32State state;
    state = RandomizerRandSeed(reason, data1, data2);
    return RandomizerNext(&state);
}

u16 RandomizerRandRange(enum RandomizerReason reason, u32 data1, u32 data2, u16 range)
{
    struct Sfc32State state;
    state = RandomizerRandSeed(reason, data1, data2);
    return RandomizerNextRange(&state, range);
}

// Utility functions for the field item randomizer.
static inline bool32 IsItemTMHM(enum Item itemId)
{
    return GetItemPocket(itemId) == POCKET_TM_HM;
}

static inline bool32 IsItemHM(enum Item itemId)
{
    return itemId >= ITEM_HM01 && IsItemTMHM(itemId);
}

static inline bool32 IsKeyItem(u16 itemId)
{
    return GetItemPocket(itemId) == POCKET_KEY_ITEMS;
}

// Don't randomize HMs or key items, that can make the game unwinnable.
// ITEM_NONE also should not be randomized as it is invalid.
static inline bool32 ShouldRandomizeItem(u16 itemId)
{
    return !(IsItemHM(itemId) || IsKeyItem(itemId) || itemId == ITEM_NONE);
}

#include "data/randomizer/item_whitelist.h"

// Given a found item and its location in the game, returns a replacement for that item.
// Given a berry tree and the berry planted in it, returns the berry it actually bears.
// Seeded on both, so a tree is stable across visits while two plantings of different
// berries in the same plot still differ.
u8 RandomizeBerryTree(u8 treeId, u8 plantedBerry)
{
    struct Sfc32State state;
    u16 result;

    if (plantedBerry == 0 || !RandomizerFeatureEnabled(RANDOMIZE_BERRY_TREES))
        return plantedBerry;

    state = RandomizerRandSeed(RANDOMIZER_REASON_FIELD_ITEM,
                               ((u32)treeId << 8) | plantedBerry, plantedBerry);

    result = RzWeightedPickMode(&state, sBerryTiers, ARRAY_COUNT(sBerryTiers), NULL, 0,
                                RZ_TIER_MODE_BERRIES);
    if (result == ITEM_NONE)
        return plantedBerry;

    return ItemIdToBerryType(result);
}

static bool32 IsMoveIllegalForLearnset(enum Move move);

// --- TM move reassignment ---------------------------------------------------
// Which move each TM teaches, drawn from the tier-weighted pool with no duplicates: a
// duplicate TM would be dead weight under I_REUSABLE_TMS. Safe because Phase 6 set
// ALL_TEACHABLES globally, so no Pokemon's compatibility depends on a TM's move.
// HMs are never touched - randomizing Surf or Strength would soft-lock a run.

static EWRAM_DATA u16 sRzTmMoves[NUM_TECHNICAL_MACHINES] = {0};
static EWRAM_DATA bool8 sRzTmMovesBuilt = FALSE;

static void RzBuildTmMoveTable(void)
{
    struct Sfc32State state;
    u32 i, j;

    state = RandomizerRandSeed(RANDOMIZER_REASON_LEARNSET, 0x7C5EED, GetRandomizerSeed());

    for (i = 0; i < NUM_TECHNICAL_MACHINES; i++)
    {
        u32 attempts;

        sRzTmMoves[i] = MOVE_NONE;
        for (attempts = 0; attempts < 128 && sRzTmMoves[i] == MOVE_NONE; attempts++)
        {
            u16 move = RzWeightedPickMode(&state, sTmMoveTiers, ARRAY_COUNT(sTmMoveTiers),
                                          NULL, 0, RZ_TM_MOVES_TIER_MODE);
            bool32 dupe = FALSE;

            if (move == MOVE_NONE || IsMoveIllegalForLearnset(move))
                continue;
            for (j = 0; j < i; j++)
            {
                if (sRzTmMoves[j] == move)
                    dupe = TRUE;
            }
            if (!dupe)
                sRzTmMoves[i] = move;
        }
        // If the pool could not produce a fresh move, leave MOVE_NONE and the caller
        // falls back to the vanilla mapping for that slot.
    }
    sRzTmMovesBuilt = TRUE;
}

enum Move RandomizeTMMove(u16 tmIndex)
{
    if (tmIndex == 0 || tmIndex > NUM_TECHNICAL_MACHINES)
        return MOVE_NONE;                       // HM or not a machine
    if (!RandomizerFeatureEnabled(RANDOMIZE_TM_MOVES))
        return MOVE_NONE;

    if (!sRzTmMovesBuilt)
        RzBuildTmMoveTable();

    return sRzTmMoves[tmIndex - 1];
}

u16 RandomizeTMMoveReverse(enum Move move)
{
    u32 i;

    if (move == MOVE_NONE || !RandomizerFeatureEnabled(RANDOMIZE_TM_MOVES))
        return ITEM_NONE;

    if (!sRzTmMovesBuilt)
        RzBuildTmMoveTable();

    for (i = 0; i < NUM_TECHNICAL_MACHINES; i++)
    {
        if (sRzTmMoves[i] == move)
            return GetTMHMItemId(i + 1);
    }
    return ITEM_NONE;
}

// Which TM a pickup becomes. Normally weighted by sTmTiers, which groups TM items by the
// tier of the move they teach *in vanilla*. Once RANDOMIZE_TM_MOVES re-points them that
// grouping is stale - TM01 is not Focus Punch any more - and weighting by it would be
// weighting by nothing. It does not need replacing, though: RzBuildTmMoveTable already
// draws every TM's move through the same TM bands, so the 50 assigned moves are already
// spread across them. A uniform pick over the assigned TMs reproduces that spread, and
// reflects what this ROM actually contains rather than what vanilla did.
static enum Item RzPickTmItem(struct Sfc32State *state)
{
    if (RandomizerFeatureEnabled(RANDOMIZE_TM_MOVES))
        return GetTMHMItemId(RandomizerNextRange(state, NUM_TECHNICAL_MACHINES) + 1);

    #if RZ_TIER_WEIGHTED_ITEMS == TRUE
        return RzWeightedPickMode(state, sTmTiers, ARRAY_COUNT(sTmTiers), NULL, 0,
                                  RZ_TIER_MODE_TMS);
    #else
        return ITEM_NONE;
    #endif
}

enum Item RandomizeFoundItem(enum Item itemId, u8 mapNum, u8 mapGroup, u8 localId)
{
    struct Sfc32State state;
    u16 result;
    u32 mapSeed;

    if (!ShouldRandomizeItem(itemId))
        return itemId;

    // Seed the generator using the original item and the object event that led up
    // to this call.
    mapSeed = ((u32)mapGroup) << 16;
    mapSeed |= ((u32)mapNum) << 8;
    mapSeed |= localId;

    state = RandomizerRandSeed(RANDOMIZER_REASON_FIELD_ITEM, mapSeed, itemId);

    // A TM pickup stays a TM, but which one is weighted by the tier of the move it
    // teaches. HMs never reach here: ShouldRandomizeItem() rejects them above.
    if (IsItemTMHM(itemId))
    {
        result = RzPickTmItem(&state);
        if (result != ITEM_NONE)
            return result;
        return RandomizerNextRange(&state, RANDOMIZER_MAX_TM - ITEM_TM01 + 1) + ITEM_TM01;
    }

    // Everything else becomes either an item or a TM. Randolocke wants TMs to be a common
    // find, so an ordinary pickup can roll into the TM band as well.
    #if RZ_TIER_WEIGHTED_ITEMS == TRUE
    {
        u32 attempts;

        for (attempts = 0; attempts < 32; attempts++)
        {
            u32 total = RZ_ITEM_W_TM_BAND + RZ_ITEM_W_T1 + RZ_ITEM_W_T2 + RZ_ITEM_W_T3
                      + RZ_ITEM_W_T4 + RZ_ITEM_W_T5;

            if (RandomizerNextRange(&state, total) < RZ_ITEM_W_TM_BAND)
            {
                result = RzPickTmItem(&state);
                if (result != ITEM_NONE)
                    return result;
                continue;
            }

            result = RzWeightedPickMode(&state, sItemTiers, ARRAY_COUNT(sItemTiers), NULL, 0,
                                        RZ_TIER_MODE_ITEMS);
            if (result != ITEM_NONE && ShouldRandomizeItem(result) && !IsItemTMHM(result))
                return result;
        }
        // fall through to the flat whitelist
    }
    #endif

    do {
        result = sRandomizerItemWhitelist[RandomizerNextRange(&state, ITEM_WHITELIST_SIZE)];
    } while(!ShouldRandomizeItem(result) || IsItemTMHM(result));

    return result;

}

// Takes a SpecialVar as an argument to simplify handling separate scripts.
static inline void RandomizeFoundItemScript(u16 *scriptVar)
{
    if (RandomizerFeatureEnabled(RANDOMIZE_FIELD_ITEMS))
    {
        // Pull the object event information from the current object event.
        u8 objEvent = gSelectedObjectEvent;
        *scriptVar = RandomizeFoundItem(
            *scriptVar,
            gObjectEvents[objEvent].mapGroup,
            gObjectEvents[objEvent].mapNum,
            gObjectEvents[objEvent].localId);
    }
}

// These functions are invoked by the scripts that handle found items and
// write the results of the randomization to the correct script variable.
void FindItemRandomize_NativeCall(struct ScriptContext *ctx)
{
    RandomizeFoundItemScript(&gSpecialVar_0x8000);
}

void FindHiddenItemRandomize_NativeCall(struct ScriptContext *ctx)
{
    RandomizeFoundItemScript(&gSpecialVar_0x8005);
}

// Both legendary and mythical Pokémon are included in this category.
static inline bool32 IsRandomizerLegendary(enum Species species)
{
    return gSpeciesInfo[species].isRestrictedLegendary
        || gSpeciesInfo[species].isSubLegendary
        || gSpeciesInfo[species].isMythical
        || gSpeciesInfo[species].isUltraBeast;
}

struct SpeciesTable
{
    // Stores the group records for each species.
    u16 groupData[RANDOMIZER_SPECIES_COUNT];
    u16 speciesToGroupIndex[RANDOMIZER_SPECIES_COUNT];
    // Maps a group data index to a species.
    u16 groupIndexToSpecies[RANDOMIZER_SPECIES_COUNT];
};

#define GROUP_INVALID   0xFFFF

static inline u16 GetSpeciesGroup(const struct SpeciesTable* table, u16 species)
{
    u16 groupEntry;
    groupEntry = table->groupData[table->speciesToGroupIndex[species]];

    #ifndef NDEBUG
        MgbaPrintf(MGBA_LOG_INFO, "GetSpeciesGroup: input %lu group %lu",
            (unsigned long)species+1, (unsigned long)groupEntry);
    #endif

    return groupEntry;

}

static void GetGroupRange(u16 group, enum RandomizerSpeciesMode mode, u16 *resultMin, u16 *resultMax)
{
    // This should never be called on a GROUP_INVALID mon, but if it happens,
    // GROUP_INVALID should be the only valid group.
    if (group == GROUP_INVALID)
    {
        *resultMax = *resultMin = group;
        return;
    }

    // BST mode: species can randomize to species with similar BST.
    if (mode == MON_RANDOM_BST)
    {
        // Choose a 10.24% range around the base BST.
        s32 base, minScaled, maxScaled;
        base = group * 1024;
        minScaled = (base - group * 100) / 1024;
        maxScaled = (base + group * 100) / 1024;
        *resultMin = (u16)max(minScaled, 0);
        *resultMax =(u16)min(maxScaled, GROUP_INVALID-1);
    }
    // Species in the same category can randomize to each other.
    else
    {
        *resultMax = *resultMin = group;
    }
}

//
static void GetIndicesFromGroupRange(const struct SpeciesTable *table, u16 minGroup, u16 maxGroup, u16 *start, u16 *end)
{
    u16 index, leftBound, rightBound, maxRightBound;
    maxRightBound = RANDOMIZER_SPECIES_COUNT-1;
    maxGroup = min(0xFFFEu, maxGroup);
    minGroup = min(0xFFFEu, minGroup);
    leftBound = 0;
    rightBound = RANDOMIZER_SPECIES_COUNT-1;
    // Do leftmost binary search to find the lower limit.
    while (leftBound < rightBound)
    {
        u16 leftFoundGroup;
        index = (leftBound + rightBound) / 2;
        leftFoundGroup = table->groupData[index];
        if (leftFoundGroup < minGroup)
            leftBound = index + 1;
        else
        {
            if (leftFoundGroup > maxGroup)
                maxRightBound = index;
            rightBound = index;
        }
    }
    *start = leftBound;

    rightBound = maxRightBound;

    // Do rightmost binary search to find the upper limit.
    while (leftBound < rightBound)
    {
        index = (leftBound + rightBound) / 2;
        if (table->groupData[index] > maxGroup)
            rightBound = index;
        else
            leftBound = index + 1;
    }
    *end = rightBound - 1;
}

#if RANDOMIZER_DYNAMIC_SPECIES == TRUE

struct RamSpeciesTable
{
    enum RandomizerSpeciesMode mode;
    bool16 tableInitialized;
    struct SpeciesTable speciesTable;
};

EWRAM_DATA static struct RamSpeciesTable sRamSpeciesTable = {0};

static void FillSpeciesGroupsRandom(struct SpeciesTable* entries)
{
    u16 i;
    for (i = 0; i < RANDOMIZER_SPECIES_COUNT; i++)
    {
        entries->groupIndexToSpecies[i] = i;
        if (IsSpeciesPermitted(i))
            entries->groupData[i] = 0;
        else
            entries->groupData[i] = GROUP_INVALID;
    }
}

static void FillSpeciesGroupsBST(struct SpeciesTable* entries)
{
    u16 i;
    for(i = 0; i < RANDOMIZER_SPECIES_COUNT; i++)
    {
        const struct SpeciesInfo *curSpeciesInfo;
        u16 group;

        entries->groupIndexToSpecies[i] = i;

        if (IsSpeciesPermitted(i))
        {
            curSpeciesInfo = &gSpeciesInfo[i];

            group = curSpeciesInfo->baseAttack;
            group += curSpeciesInfo->baseDefense;
            group += curSpeciesInfo->baseSpAttack;
            group += curSpeciesInfo->baseSpDefense;
            group += curSpeciesInfo->baseHP;
            group += curSpeciesInfo->baseSpeed;

        }
        else
            group = GROUP_INVALID;

        entries->groupData[i] = group;
    }
}

static void FillSpeciesGroupsLegendary(struct SpeciesTable* entries)
{
    u16 i;
    for(i = 0; i < RANDOMIZER_SPECIES_COUNT; i++)
    {
        entries->groupIndexToSpecies[i] = i;
        if (!IsSpeciesPermitted(i))
            entries->groupData[i] = GROUP_INVALID;
        else
            entries->groupData[i] = IsRandomizerLegendary(i);
    }
}

static void MarkEvolutions(struct SpeciesTable *entries, u16 species, u16 stage)
{
    const struct Evolution *evos;
    if (stage == RANDOMIZER_MAX_EVO_STAGES)
        return;

    evos = GetSpeciesEvolutions(species);
    if (evos != NULL)
    {
        u32 i;
        for (i = 0; evos[i].method != 0xFFFF; i++)
        {
            if(entries->groupData[species-1] <= stage)
                MarkEvolutions(entries, evos[i].targetSpecies, stage+1);
        }
    }
    entries->groupIndexToSpecies[species] = species;
    entries->groupData[species] = stage;
}

static void FillSpeciesGroupsEvolution(struct SpeciesTable* entries)
{
    u16 i;
    static const u8 EVO_GROUP_LEGENDARY = 0x81;
    static const u8 EVO_GROUP_NO_EVO = RANDOMIZER_MAX_EVO_STAGES+1;

    // Step 0: zero everything
    memset(entries, 0, sizeof(sRamSpeciesTable.speciesTable));

    // Step 1: pre-visit the special babies, and mark them as basic mons.
    for (i = 0; i < ARRAY_COUNT(sPreevolutionBabyMons); i++)
    {
        u16 babyMonIndex = sPreevolutionBabyMons[i];
        entries->groupIndexToSpecies[babyMonIndex] = babyMonIndex;
        if(IsSpeciesPermitted(babyMonIndex))
            entries->groupData[babyMonIndex] = 0;
        else
            entries->groupData[babyMonIndex] = GROUP_INVALID;
    }

    for(i = 0; i < RANDOMIZER_SPECIES_COUNT; i++)
    {
        if (entries->groupIndexToSpecies[i] == 0)
        {
            // This entry hasn't been visited yet, so we don't know if it evolves.
            const struct Evolution *evos = GetSpeciesEvolutions(i);
            entries->groupIndexToSpecies[i] = i;
            if (!IsSpeciesPermitted(i)) // This shouldn't show up in randomization.
                entries->groupData[i] = GROUP_INVALID;
            else if (IsRandomizerLegendary(i)) // Legendaries get their own group.
                entries->groupData[i] = EVO_GROUP_LEGENDARY;
            else if (evos == NULL || evos->method == 0xFFFF)
                entries->groupData[i] = EVO_GROUP_NO_EVO;
            else // There are evolutions! Let's check it out.
                MarkEvolutions(entries, i, 0);
        }
    }
}

static inline u16 LeftChildIndex(u16 index)
{
    return 2*index + 1;
}

static inline void SwapSpeciesAndGroup(struct SpeciesTable* table, u16 indexA, u16 indexB)
{
    u16 temp;
    SWAP(table->groupData[indexA], table->groupData[indexB], temp);
    SWAP(table->groupIndexToSpecies[indexA], table->groupIndexToSpecies[indexB], temp);
}

static void BuildRandomizerSpeciesTable(enum RandomizerSpeciesMode mode)
{
    u16 i, start, end;
    struct SpeciesTable* speciesTable;

    sRamSpeciesTable.tableInitialized = TRUE;
    sRamSpeciesTable.mode = mode;
    speciesTable = &sRamSpeciesTable.speciesTable;

    switch(mode)
    {
        case MON_RANDOM_LEGEND_AWARE:
            FillSpeciesGroupsLegendary(speciesTable);
            break;
        case MON_RANDOM_BST:
            FillSpeciesGroupsBST(speciesTable);
            break;
        case MON_EVOLUTION:
            FillSpeciesGroupsEvolution(speciesTable);
            break;
        case MON_RANDOM:
        default:
            FillSpeciesGroupsRandom(speciesTable);
    }

    // Heap sort the table.
    start = RANDOMIZER_SPECIES_COUNT/2;
    end = RANDOMIZER_SPECIES_COUNT-1;

    while (end > 1)
    {
        u16 root;
        if (start > 0)
            start = start - 1;
        else
        {
            end = end - 1;
            SwapSpeciesAndGroup(speciesTable, end, 0);
        }
        root = start;
        while(LeftChildIndex(root) < end)
        {
            u16 child;
            child = LeftChildIndex(root);

            if (child+1 < end
                && speciesTable->groupData[child] < speciesTable->groupData[child+1])
            {
                child = child + 1;
            }

            if (speciesTable->groupData[root] < speciesTable->groupData[child])
            {
                SwapSpeciesAndGroup(speciesTable, root, child);
                root = child;
            }
            else
                break;
        }
    }


    // Build the species index. This is needed for getting a group from a species.
    for (i = 0; i < RANDOMIZER_SPECIES_COUNT; i++)
    {
        u16 targetIndex = speciesTable->groupIndexToSpecies[i];
        speciesTable->speciesToGroupIndex[targetIndex] = i;
    }
}

static const struct SpeciesTable* GetSpeciesTable(enum RandomizerSpeciesMode mode)
{
    if (!sRamSpeciesTable.tableInitialized || mode != sRamSpeciesTable.mode )
        BuildRandomizerSpeciesTable(mode);

    return &sRamSpeciesTable.speciesTable;
}

void PreloadRandomizationTables(void)
{
    GetSpeciesTable(GetRandomizerOption(RANDOMIZER_OPTION_SPECIES_MODE));
}

#endif

static u16 RandomizeMonTableLookup(struct Sfc32State* state, enum RandomizerSpeciesMode mode, u16 species)
{
    u16 minGroup, maxGroup, originalGroup, resultIndex;
    u16 minIndex, maxIndex;
    const struct SpeciesTable *table;

    table = GetSpeciesTable(mode);
    originalGroup = GetSpeciesGroup(table, species);

    if (originalGroup == GROUP_INVALID)
        return species;

    GetGroupRange(originalGroup, mode, &minGroup, &maxGroup);
    GetIndicesFromGroupRange(table, minGroup, maxGroup, &minIndex, &maxIndex);
    resultIndex = RandomizerNextRange(state, maxIndex - minIndex + 1) + minIndex;
    return table->groupIndexToSpecies[resultIndex];
}

static u16 RandomizeMonFromSeed(struct Sfc32State *state, enum RandomizerSpeciesMode mode, u16 species)
{
    if (!IsSpeciesPermitted(species))
        return species;

    if (mode >= MAX_MON_MODE)
        mode = MON_RANDOM;

    return RandomizeMonTableLookup(state, mode, species);

}

// Fills an array with count Pokémon, with no repeats.
void GetUniqueMonList(enum RandomizerReason reason, enum RandomizerSpeciesMode mode, u32 seed1, u16 seed2, u8 count, const u16 *originalSpecies, u16 *resultSpecies)
{
    u32 i, curMon;
    u32 seenMonBitVector[(RANDOMIZER_SPECIES_COUNT-1)/32+1] = {};
    struct Sfc32State state = RandomizerRandSeed(reason, seed1, seed2);

    for (i = 0; i < count; i++)
    {
        u16 curOriginal = originalSpecies[i];
        bool32 foundNextMon = FALSE;
        if (!IsSpeciesPermitted(curOriginal))
        {
            // If there's non-permitted Pokémon in here, something is wrong.
            // Just pass them through without marking.
            curMon = curOriginal;
            continue;
        }

        // Find the next mon.
        while (!foundNextMon)
        {
            u16 wordIndex, adjustedCurMon;
            u32 bitVectorWord;
            u8 bitIndex;

            // Generate a Pokémon. If it has already been generated, keep generating new ones
            // until one that hasn't been seen is picked.

            curMon = RandomizeMonFromSeed(&state, mode, curOriginal);

            // Compute the bit address of this mon.
            adjustedCurMon = curMon - 1;
            wordIndex = adjustedCurMon / 32;
            bitIndex = adjustedCurMon & 31;
            bitVectorWord = seenMonBitVector[wordIndex];

            // If set, this mon has been seen already.
            if (bitVectorWord & (1 << bitIndex))
                continue;

            bitVectorWord |= 1 << bitIndex;
            seenMonBitVector[wordIndex] = bitVectorWord;
            foundNextMon = TRUE;
        }
        resultSpecies[i] = curMon;
    }
}

enum Species RandomizeMonBaseForm(enum RandomizerReason reason, enum RandomizerSpeciesMode mode, u32 seed, enum Species species)
{
    struct Sfc32State state;
    state = RandomizerRandSeed(reason, seed, species);
    return RandomizeMonFromSeed(&state, mode, species);
}

static u16 ChooseRandomForm(struct Sfc32State *state, const u16 baseSpecies)
{
    const u16 *formsTable = gSpeciesInfo[baseSpecies].formSpeciesIdTable;
    if (formsTable)
    {
        u32 formCount = 0;
        while (formsTable[formCount] != FORM_SPECIES_END)
        {
            formCount++;
        }
        return formsTable[RandomizerNextRange(state, formCount)];
    }

    return baseSpecies;
}

static u16 GetFormFromRareFormInfo(struct Sfc32State *state, const struct RandomizerRareFormInfo *info)
{
    if (RandomizerNextRange(state, info->inverseRareFormChance) > 0)
        return info->commonForm;
    else
        return info->rareForm;
}

#define RANDOM_FROM_ARRAY(arr)  (arr[RandomizerNextRange(state, ARRAY_COUNT(arr))])
#define RARE_FORM(infoStruct)   (GetFormFromRareFormInfo(state, &infoStruct))
static u16 ChooseFormSpecial(struct Sfc32State *state, const u16 baseSpecies)
{
    switch (baseSpecies) {
        // These species do almost ordinary ordinary random form selection processes.
        // However, their form tables include forms that shouldn't normally be
        // selected, so they need to have special hard-coded form tables.
        case SPECIES_FLOETTE:
            return RANDOM_FROM_ARRAY(sFloetteFormChoices);
        case SPECIES_TAUROS_PALDEA_COMBAT:
            return RANDOM_FROM_ARRAY(sPaldeanTaurosFormChoices);
        case SPECIES_MINIOR:
            return RANDOM_FROM_ARRAY(sMiniorFormChoices);
        // These are species, first appearing in Gen 8, that have one common
        // form and one rare form.
        // Note that as Maushold can only appear in raid battles in Gen 9, it
        // normally does not behave this way in the wild, but for a randomizer
        // this seems like a reasonable choice.
        case SPECIES_MAUSHOLD:
            return RARE_FORM(sMausholdRareFormInfo);
        case SPECIES_SINISTEA:
            return RARE_FORM(sSinisteaRareFormInfo);
        case SPECIES_SINISTCHA:
            return RARE_FORM(sSinistchaRareFormInfo);
        case SPECIES_POLTEAGEIST:
            return RARE_FORM(sPolteageistRareFormInfo);
        case SPECIES_DUDUNSPARCE:
            return RARE_FORM(sDudunsparceRareFormInfo);
        default:
            return baseSpecies;
    }

}
#undef RANDOM_FROM_ARRAY
#undef RARE_FORM

enum Species RandomizeMon(enum RandomizerReason reason, enum RandomizerSpeciesMode mode, u32 seed, enum Species species)
{
    u32 speciesMode;
    u16 resultSpecies;
    struct Sfc32State state;

    if (!IsSpeciesPermitted(species))
        return species;

    state = RandomizerRandSeed(reason, seed, species);

    resultSpecies = RandomizeMonFromSeed(&state, mode, species);
    speciesMode = gSpeciesInfo[resultSpecies].randomizerMode;

    switch (speciesMode)
    {
        case MON_RANDOMIZER_RANDOM_FORM:
            return ChooseRandomForm(&state, resultSpecies);
        case MON_RANDOMIZER_SPECIAL_FORM:
            return ChooseFormSpecial(&state, resultSpecies);
        case MON_RANDOMIZER_NORMAL:
        default:
            return resultSpecies;
    }
}

enum Species RandomizeWildEncounter(enum Species species, u8 mapNum, u8 mapGroup, enum WildPokemonArea area, u8 slot)
{
    if (RandomizerFeatureEnabled(RANDOMIZE_WILD_MON))
    {
        // Randomization is done based on the map number, the WildArea, and the encounter slot.
        // This means a distinct species can appear in each encounter slot.
        u32 seed;
        seed = ((u32)mapGroup) << 24;
        seed |= ((u32)mapNum) << 16;
        seed |= ((u32)area) << 8;
        seed |= slot;

        return RandomizeMon(RANDOMIZER_REASON_WILD_ENCOUNTER, GetRandomizerOption(RANDOMIZER_OPTION_SPECIES_MODE), seed, species);
    }

    return species;
}


// This is used in the Pokédex area map code.
bool32 IsRandomizationPossible(enum Species originalSpecies, enum Species targetSpecies)
{
    const enum RandomizerSpeciesMode mode = GetRandomizerOption(RANDOMIZER_OPTION_SPECIES_MODE);
    if (!IsSpeciesPermitted(targetSpecies) || !IsSpeciesPermitted(originalSpecies))
    {
        // For a species that is not permitted, randomization is disabled.
        // Therefore, if the species are the same, they will "randomize".
        return originalSpecies == targetSpecies;
    }

    if (mode != MON_RANDOM && mode < MAX_MON_MODE)
    {
        u16 minGroupOriginal, maxGroupOriginal, minGroupTarget, maxGroupTarget,
            originalGroup, targetGroup;
        const struct SpeciesTable* table;
        table = GetSpeciesTable(mode);
        originalGroup = GetSpeciesGroup(table, originalSpecies);
        targetGroup = GetSpeciesGroup(table, targetSpecies);
        GetGroupRange(originalGroup, mode, &minGroupOriginal, &maxGroupOriginal);
        GetGroupRange(targetGroup, mode, &minGroupTarget, &maxGroupTarget);

        // If the group ranges intersect, randomization is possible.
        return maxGroupOriginal >= minGroupTarget && minGroupOriginal <= maxGroupTarget;
    }

    return TRUE;
}

enum Species RandomizeTrainerMon(u16 trainerId, u8 slot, u8 totalMons, enum Species species)
{
    if (RandomizerFeatureEnabled(RANDOMIZE_TRAINER_MON))
    {
        // The seed is based on the internal trainer number, the number of
        // Pokémon in that trainer's party, and which party position it is in.
        u32 seed;
        seed = (u32)trainerId << 16;
        seed |= (u32)totalMons << 8;
        seed |= slot;

        return RandomizeMon(RANDOMIZER_REASON_TRAINER_PARTY, GetRandomizerOption(RANDOMIZER_OPTION_SPECIES_MODE), seed, species);
    }

    return species;
}

#if RANDOLOCKE_UNIQUE_LEGENDARIES == TRUE
EWRAM_DATA static u32 sLastLegendarySeed = 0;
EWRAM_DATA static u16 sRandomizedLegendaries[LEGENDARY_MON_COUNT] = {0};

// Returns the legendary standing in for `species`, or SPECIES_NONE if `species` is not a
// legendary encounter. MON_RANDOM_LEGEND_AWARE is forced regardless of the player's
// species mode: it is what keeps a legendary site legendary. GetUniqueMonList does the
// without-replacement part, the same way the starter list avoids duplicate starters.
enum Species RandomizeLegendaryMon(enum Species species)
{
    u32 i;

    for (i = 0; i < LEGENDARY_MON_COUNT; i++)
    {
        if (gLegendaryMonTable[i] == species)
            break;
    }
    if (i == LEGENDARY_MON_COUNT)
        return SPECIES_NONE;

    if (sLastLegendarySeed != GetRandomizerSeed() || sRandomizedLegendaries[0] == SPECIES_NONE)
    {
        GetUniqueMonList(RANDOMIZER_REASON_FIXED_ENCOUNTER, MON_RANDOM_LEGEND_AWARE,
                         0x1E6E4D, 0, LEGENDARY_MON_COUNT, gLegendaryMonTable,
                         sRandomizedLegendaries);
        sLastLegendarySeed = GetRandomizerSeed();
    }
    return sRandomizedLegendaries[i];
}
#endif

// Scripted encounters created with seteventmon rather than setwildbattle - Mew, Deoxys,
// Ho-Oh, Lugia and the Southern Island pair. These never reached the randomizer at all
// before; CreateEnemyEventMon just used the species the script named.
enum Species RandomizeEventEncounterMon(enum Species species)
{
    if (!RandomizerFeatureEnabled(RANDOMIZE_FIXED_MON))
        return species;

    #if RANDOLOCKE_UNIQUE_LEGENDARIES == TRUE
    {
        enum Species legendary = RandomizeLegendaryMon(species);

        if (legendary != SPECIES_NONE)
            return legendary;
    }
    #endif

    // Not a legendary site. Seed off the map, as the setwildbattle path does.
    return RandomizeMon(RANDOMIZER_REASON_FIXED_ENCOUNTER,
                        GetRandomizerOption(RANDOMIZER_OPTION_SPECIES_MODE),
                        ((u32)gSaveBlock1Ptr->location.mapNum << 16)
                            | ((u32)gSaveBlock1Ptr->location.mapGroup << 8),
                        species);
}

enum Species RandomizeFixedEncounterMon(enum Species species, u8 mapNum, u8 mapGroup, u8 localId)
{
    if (RandomizerFeatureEnabled(RANDOMIZE_FIXED_MON))
    {
        #if RANDOLOCKE_UNIQUE_LEGENDARIES == TRUE
        {
            // A legendary site keeps its own pool, so it cannot roll a Zigzagoon and
            // cannot repeat a legendary another site already took.
            enum Species legendary = RandomizeLegendaryMon(species);

            if (legendary != SPECIES_NONE)
                return legendary;
        }
        #endif

        // The seed is based on the location of the object event.
        u32 seed;
        seed = (u32)mapNum << 16;
        seed |= (u32)mapGroup << 8;
        seed |= localId;

        return RandomizeMon(RANDOMIZER_REASON_FIXED_ENCOUNTER, GetRandomizerOption(RANDOMIZER_OPTION_SPECIES_MODE), seed, species);
    }

    return species;
}

EWRAM_DATA static u32 sLastMonRandomizerSeed = 0;
EWRAM_DATA static u16 sRandomizedMons[STARTER_AND_GIFT_MON_COUNT] = {0};

enum Species RandomizeStarterAndGiftMon(u16 originalSlot, const enum Species* originalStarterAndGiftMons)
{
    if (RandomizerFeatureEnabled(RANDOMIZE_STARTER_AND_GIFT_MON))
    {
        if (sLastMonRandomizerSeed != GetRandomizerSeed() || sRandomizedMons[0] == SPECIES_NONE)
        {
            // The randomized starter table is stale or uninitialized. Fix that!

            // Hash the starter list so that which starters there are influences the seed.
            u32 starterHash = 5381;
            u32 i;
            for (i = 0; i < STARTER_AND_GIFT_MON_COUNT; i++)
            {
                u16 originalStarter = originalStarterAndGiftMons[i];
                starterHash = ((starterHash << 5) + starterHash) ^ (u8)originalStarter;
                starterHash = ((starterHash << 5) + starterHash) ^ (u8)(originalStarter >> 8);
            }

            GetUniqueMonList(RANDOMIZER_REASON_STARTER_AND_GIFT_MON, GetRandomizerOption(RANDOMIZER_OPTION_SPECIES_MODE),
                starterHash, 0, STARTER_AND_GIFT_MON_COUNT, originalStarterAndGiftMons, sRandomizedMons);
        }
        return sRandomizedMons[originalSlot];
    }

    return originalStarterAndGiftMons[originalSlot];
}

EWRAM_DATA static u32 sLastEggMonRandomizerSeed = 0;
EWRAM_DATA static u16 sRandomizedEggMons[EGG_MON_COUNT] = {0};

enum Species RandomizeEggMon(u16 originalSlot, const enum Species* originalEggMons)
{
    if (RandomizerFeatureEnabled(RANDOMIZE_EGG_MON))
    {
        if (sLastEggMonRandomizerSeed != GetRandomizerSeed() || sRandomizedEggMons[0] == SPECIES_NONE)
        {
            // The randomized egg table is stale or uninitialized. Fix that!

            // Hash the egg list so that which eggs there are influences the seed.
            u32 eggHash = 5381;
            u32 i;
            for (i = 0; i < EGG_MON_COUNT; i++)
            {
                u16 originalEgg = originalEggMons[i];
                eggHash = ((eggHash << 5) + eggHash) ^ (u8)originalEgg;
                eggHash = ((eggHash << 5) + eggHash) ^ (u8)(originalEgg >> 8);
            }

            GetUniqueMonList(RANDOMIZER_REASON_EGG, GetRandomizerOption(RANDOMIZER_OPTION_SPECIES_MODE),
                eggHash, 0, EGG_MON_COUNT, originalEggMons, sRandomizedEggMons);
        }
        return sRandomizedEggMons[originalSlot];
    }

    return originalEggMons[originalSlot];
}

#if RZ_ABILITY_STABLE_ACROSS_EVOLUTION == TRUE

// GetSpeciesPreEvolution() is a linear scan over every species, and RandomizeAbility
// runs often during battle (the AI calls it while scoring moves), so memoize the
// family root in a small direct-mapped cache. 16 entries covers a full double battle
// plus both parties comfortably.
#define RZ_FAMILY_CACHE_SIZE 16
static EWRAM_DATA u16 sFamilyRootKey[RZ_FAMILY_CACHE_SIZE] = {0};
static EWRAM_DATA u16 sFamilyRootVal[RZ_FAMILY_CACHE_SIZE] = {0};

// Returns the base species of this Pokemon's evolution family (e.g. Pichu for
// Pikachu and Raichu), so every stage seeds the same randomized ability.
static enum Species GetAbilityFamilyRoot(enum Species species)
{
    u32 slot = species % RZ_FAMILY_CACHE_SIZE;
    enum Species current = species;
    u32 guard;

    if (sFamilyRootKey[slot] == species && sFamilyRootVal[slot] != SPECIES_NONE)
        return sFamilyRootVal[slot];

    // Bounded so a malformed or cyclic evolution table can never hang the game.
    for (guard = 0; guard < RANDOMIZER_MAX_EVO_STAGES; guard++)
    {
        enum Species prev = GetSpeciesPreEvolution(current);

        if (prev == SPECIES_NONE)
            break;
        current = prev;
    }

    sFamilyRootKey[slot] = species;
    sFamilyRootVal[slot] = current;
    return current;
}

#endif // RZ_ABILITY_STABLE_ACROSS_EVOLUTION

// --- Learnset randomization -------------------------------------------------
// Every Pokemon learns the same 21 moves at the same levels: 7 STAB, 7 status and
// 7 non-STAB damaging, with higher Base Power learned later within each damaging
// group. Built on demand into an EWRAM buffer and cached by species, because
// GetSpeciesLevelUpLearnset() is called from 18 places including the battle AI.

static EWRAM_DATA struct LevelUpMove sRzLearnsetBuf[RZ_LEARNSET_SLOTS + 1] = {0};
static EWRAM_DATA u16 sRzLearnsetSpecies = SPECIES_NONE;

static const u8 sRzLearnsetLevels[RZ_LEARNSET_SLOTS] = RZ_LEARNSET_LEVELS;

// Moves that would be unfair or nonsensical as a guaranteed level-up move.
static bool32 IsMoveIllegalForLearnset(enum Move move)
{
    if (move == MOVE_NONE || move == MOVE_STRUGGLE)
        return TRUE;
    // EFFECT_PLACEHOLDER marks moves that are not implemented yet.
    if (GetMoveEffect(move) == EFFECT_PLACEHOLDER)
        return TRUE;
    if (GetMoveEffect(move) == EFFECT_OHKO)
        return TRUE;
    return FALSE;
}

// Picks `count` moves into dest, choosing only moves accepted by `accept`.
// Damaging groups are then sorted by Base Power so stronger moves come later.
static void RzPickMoves(struct Sfc32State *state, enum Move *dest, u32 count,
                        bool32 (*accept)(enum Move, enum Type, enum Type, u32),
                        enum Type t1, enum Type t2, u32 category, bool32 sortByPower)
{
    u32 filled = 0;
    u32 attempts = 0;

    while (filled < count && attempts < 512)
    {
        enum Move move;
        u32 i;
        bool32 dupe = FALSE;

        attempts++;

        #if RZ_TIER_WEIGHTED_MOVES == TRUE
            // Weighted draw first; a filtered category (STAB of one type, say) can exhaust
            // the tier attempts, so fall back to a uniform roll rather than spin.
            move = RzWeightedPickMode(state, sMoveTiers, ARRAY_COUNT(sMoveTiers), NULL, 0,
                                      RZ_TIER_MODE_MOVES);
            if (move == MOVE_NONE)
                move = RandomizerNextRange(state, MOVES_COUNT - 1) + 1;
        #else
            move = RandomizerNextRange(state, MOVES_COUNT - 1) + 1;
        #endif
        if (IsMoveIllegalForLearnset(move) || !accept(move, t1, t2, category))
            continue;
        for (i = 0; i < filled; i++)
        {
            if (dest[i] == move)
                dupe = TRUE;
        }
        if (dupe)
            continue;
        dest[filled++] = move;
    }

    // Pad if the pool was too small to fill every slot.
    while (filled < count)
        dest[filled++] = MOVE_TACKLE;

    if (sortByPower)
    {
        u32 i, j;

        for (i = 1; i < count; i++)
        {
            enum Move key = dest[i];
            u32 power = GetMovePower(key);

            for (j = i; j > 0 && GetMovePower(dest[j - 1]) > power; j--)
                dest[j] = dest[j - 1];
            dest[j] = key;
        }
    }
}

// DAMAGE_CATEGORY_NONE is used here to mean "either category will do" - a mixed attacker.
static bool32 RzCategoryOk(enum Move move, u32 wanted)
{
    if (wanted == DAMAGE_CATEGORY_NONE)
        return TRUE;
    return GetMoveCategory(move) == wanted;
}

// Which category a species can actually attack in. A pure physical attacker rolling seven
// special STAB moves cannot use any of them, which is the failure this prevents.
static u32 RzPreferredCategory(enum Species species)
{
    u32 atk, spa, hi, lo;

    #if RZ_STAB_MATCH_CATEGORY == FALSE
        return DAMAGE_CATEGORY_NONE;
    #endif

    atk = gSpeciesInfo[species].baseAttack;
    spa = gSpeciesInfo[species].baseSpAttack;
    hi = (atk > spa) ? atk : spa;
    lo = (atk > spa) ? spa : atk;

    // Close enough to use both: draw STAB from either category.
    if (hi == 0 || lo * 100 >= hi * RZ_MIXED_ATTACKER_PERCENT)
        return DAMAGE_CATEGORY_NONE;

    return (atk > spa) ? DAMAGE_CATEGORY_PHYSICAL : DAMAGE_CATEGORY_SPECIAL;
}

static bool32 RzAcceptStab(enum Move move, enum Type t1, enum Type t2, u32 category)
{
    if (GetMoveCategory(move) == DAMAGE_CATEGORY_STATUS)
        return FALSE;
    if (!RzCategoryOk(move, category))
        return FALSE;
    return GetMoveType(move) == t1 || GetMoveType(move) == t2;
}

static bool32 RzAcceptStatus(enum Move move, enum Type t1, enum Type t2, u32 category)
{
    return GetMoveCategory(move) == DAMAGE_CATEGORY_STATUS;
}

static bool32 RzAcceptDamaging(enum Move move, enum Type t1, enum Type t2, u32 category)
{
    if (GetMoveCategory(move) == DAMAGE_CATEGORY_STATUS)
        return FALSE;
    if (!RzCategoryOk(move, category))
        return FALSE;
    // Non-STAB, so it complements the STAB block rather than duplicating it.
    return GetMoveType(move) != t1 && GetMoveType(move) != t2;
}

// Returns the randomized 21-move level-up learnset for a species, or NULL when
// learnset randomization is switched off.
const struct LevelUpMove *RandomizeLevelUpLearnset(enum Species species)
{
    struct Sfc32State state;
    enum Move picks[RZ_LEARNSET_SLOTS];
    enum Type t1, t2;
    u32 stabFromT1, i, category;

    if (!RandomizerFeatureEnabled(RANDOMIZE_LEARNSET))
        return NULL;

    if (sRzLearnsetSpecies == species)
        return sRzLearnsetBuf;

    t1 = gSpeciesInfo[species].types[0];
    t2 = gSpeciesInfo[species].types[1];
    category = RzPreferredCategory(species);
    state = RandomizerRandSeed(RANDOMIZER_REASON_LEARNSET, species, species);

    // 7 STAB. Dual types split 4/3 across the two.
    stabFromT1 = (t1 == t2) ? RZ_LEARNSET_STAB_MOVES : (RZ_LEARNSET_STAB_MOVES + 1) / 2;
    RzPickMoves(&state, &picks[0], stabFromT1, RzAcceptStab, t1, t1, category, TRUE);
    if (stabFromT1 < RZ_LEARNSET_STAB_MOVES)
        RzPickMoves(&state, &picks[stabFromT1], RZ_LEARNSET_STAB_MOVES - stabFromT1,
                    RzAcceptStab, t2, t2, category, TRUE);

    // 7 status, in no particular order - Base Power does not apply to them.
    RzPickMoves(&state, &picks[RZ_LEARNSET_STAB_MOVES], RZ_LEARNSET_STATUS_MOVES,
                RzAcceptStatus, t1, t2, DAMAGE_CATEGORY_NONE, FALSE);

    // 7 non-STAB damaging, weakest first.
    RzPickMoves(&state, &picks[RZ_LEARNSET_STAB_MOVES + RZ_LEARNSET_STATUS_MOVES],
                RZ_LEARNSET_DAMAGING_MOVES, RzAcceptDamaging, t1, t2, category, TRUE);

    // Interleave the three groups so each level band mixes categories, and keep the
    // within-group power ordering so stronger moves still arrive later.
    for (i = 0; i < RZ_LEARNSET_SLOTS; i++)
    {
        u32 group = i % 3;
        u32 index = i / 3;
        u32 src;

        if (group == 0)
            src = index;                                             // STAB
        else if (group == 1)
            src = RZ_LEARNSET_STAB_MOVES + index;                    // status
        else
            src = RZ_LEARNSET_STAB_MOVES + RZ_LEARNSET_STATUS_MOVES + index;

        sRzLearnsetBuf[i].move = picks[src];
        sRzLearnsetBuf[i].level = sRzLearnsetLevels[i];
    }
    sRzLearnsetBuf[RZ_LEARNSET_SLOTS].move = LEVEL_UP_MOVE_END;
    sRzLearnsetBuf[RZ_LEARNSET_SLOTS].level = 0;

    sRzLearnsetSpecies = species;
    return sRzLearnsetBuf;
}

static inline bool32 IsAbilityIllegal(enum Ability ability)
{
    if (ability == ABILITY_NONE || ability == ABILITY_WONDER_GUARD)
        return TRUE;
    return FALSE;
}

// Given a species and an abilityNum, returns a replacement for that ability.
enum Ability RandomizeAbility(enum Species species, u8 abilityNum, enum Ability originalAbility)
{
    if (RandomizerFeatureEnabled(RANDOMIZE_ABILITIES) && originalAbility != ABILITY_NONE)
    {  
        struct Sfc32State state;
        u16 result;
        u32 seed;

        enum Species seedSpecies = species;

        #if RZ_ABILITY_STABLE_ACROSS_EVOLUTION == TRUE
            // Seed from the family root so evolving does not reroll the ability.
            seedSpecies = GetAbilityFamilyRoot(species);
        #endif

        // Seed the generator using the species and the abilityNum
        seed = ((u32)seedSpecies) << 8;
        seed |= abilityNum;

        state = RandomizerRandSeed(RANDOMIZER_REASON_ABILITIES, seed, seedSpecies);

        // Randomize abilities
        #if RZ_TIER_WEIGHTED_ABILITIES == TRUE
            result = RzWeightedPickMode(&state, sAbilityTiers, ARRAY_COUNT(sAbilityTiers),
                                        NULL, 0, RZ_TIER_MODE_ABILITIES);
            if (result != ABILITY_NONE && !IsAbilityIllegal(result))
                return result;
            // fall through to the flat whitelist if the tiers could not produce one
        #endif

        do
        {
            result = sRandomizerAbilityWhitelist[RandomizerNextRange(&state, ABILITY_WHITELIST_SIZE)];
        } while(IsAbilityIllegal(result));

        return result;
    }

    return originalAbility;
}

#else // RANDOMIZER_AVAILABLE

// item.h calls these unconditionally, so they must exist even with the randomizer compiled
// out. Returning nothing here makes every caller fall back to the vanilla TM mapping.
enum Move RandomizeTMMove(u16 tmIndex)      { return MOVE_NONE; }
u16 RandomizeTMMoveReverse(enum Move move)  { return ITEM_NONE; }

#endif // RANDOMIZER_AVAILABLE
