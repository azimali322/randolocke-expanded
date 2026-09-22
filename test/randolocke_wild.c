#include "global.h"
#include "config/randomizer.h"
#include "event_data.h"
#include "pokemon.h"
#include "randomizer.h"
#include "wild_encounter.h"
#include "test/test.h"
#include "constants/wild_encounter.h"

// Wild encounters roll from a window of RZ_WILD_BST_FLOOR_PERCENT to
// RZ_WILD_BST_CEILING_PERCENT of the vanilla species' BST, and the two rarest land slots
// roll a pseudo-legendary line instead. Checked over every wild slot in the game rather
// than a sample: the window is the whole point, so no slot may fall outside it.
static u32 Bst(u32 species)
{
    const struct SpeciesInfo *info = &gSpeciesInfo[species];

    return info->baseHP + info->baseAttack + info->baseDefense
         + info->baseSpeed + info->baseSpAttack + info->baseSpDefense;
}

static bool32 IsRollable(u32 species)
{
    return species != SPECIES_NONE && gSpeciesInfo[species].baseHP != 0
        && gSpeciesInfo[species].randomizerMode != MON_RANDOMIZER_INVALID;
}

static bool32 IsLotteryPrize(u32 species)
{
    static const u16 sPrizes[] =
    {
        SPECIES_DRATINI, SPECIES_LARVITAR, SPECIES_BAGON, SPECIES_BELDUM, SPECIES_GIBLE,
        SPECIES_DEINO, SPECIES_GOOMY, SPECIES_JANGMO_O, SPECIES_DREEPY, SPECIES_FRIGIBAX,
    };

    for (u32 i = 0; i < ARRAY_COUNT(sPrizes); i++)
    {
        if (sPrizes[i] == species)
            return TRUE;
    }
    return FALSE;
}

static void CheckArea(const struct WildPokemonInfo *info, u32 slots, const struct WildPokemonHeader *header,
                      enum WildPokemonArea area, u32 *outside)
{
    if (info == NULL)
        return;

    for (u32 slot = 0; slot < slots; slot++)
    {
        u32 vanilla = info->wildPokemon[slot].species;
        u32 rolled = RandomizeWildEncounter(vanilla, header->mapNum, header->mapGroup, area, slot);

        if (!IsRollable(vanilla))
            continue;
        if (RZ_WILD_LOTTERY && area == WILD_AREA_LAND && slot >= RZ_WILD_LOTTERY_FROM_SLOT)
        {
            EXPECT(IsLotteryPrize(rolled));
            continue;
        }
        if (Bst(rolled) < Bst(vanilla) * RZ_WILD_BST_FLOOR_PERCENT / 100
         || Bst(rolled) > Bst(vanilla) * RZ_WILD_BST_CEILING_PERCENT / 100)
        {
            Test_MgbaPrintf("map %d.%d area %d slot %d: %S (BST %d) rolled %S (BST %d)",
                            header->mapGroup, header->mapNum, area, slot,
                            gSpeciesInfo[vanilla].speciesName, Bst(vanilla),
                            gSpeciesInfo[rolled].speciesName, Bst(rolled));
            (*outside)++;
        }
    }
}

TEST("Randolocke: wild replacements stay in their window, and the lottery pays out in pseudo-legendaries")
{
    u32 seed = 0, outside = 0;

    PARAMETRIZE { seed = 0xEDFC0000; } // a playtest save
    PARAMETRIZE { seed = 0x12345678; }

    gSaveBlock2Ptr->playerTrainerId[0] = seed;
    gSaveBlock2Ptr->playerTrainerId[1] = seed >> 8;
    gSaveBlock2Ptr->playerTrainerId[2] = seed >> 16;
    gSaveBlock2Ptr->playerTrainerId[3] = seed >> 24;
    FlagSet(RANDOMIZER_FLAG_WILD_MON);
    VarSet(RANDOMIZER_VAR_SPECIES_MODE, MON_RANDOM_BST);

    for (u32 h = 0; gWildMonHeaders[h].mapGroup != MAP_GROUP(MAP_UNDEFINED); h++)
    {
        for (u32 t = 0; t < TIMES_OF_DAY_COUNT; t++)
        {
            const struct WildEncounterTypes *types = &gWildMonHeaders[h].encounterTypes[t];

            CheckArea(types->landMonsInfo, NUM_LAND_MONS_ENCOUNTER_SLOTS, &gWildMonHeaders[h], WILD_AREA_LAND, &outside);
            CheckArea(types->waterMonsInfo, NUM_WATER_MONS_ENCOUNTER_SLOTS, &gWildMonHeaders[h], WILD_AREA_WATER, &outside);
            CheckArea(types->rockSmashMonsInfo, NUM_ROCK_SMASH_MONS_ENCOUNTER_SLOTS, &gWildMonHeaders[h], WILD_AREA_ROCKS, &outside);
            CheckArea(types->fishingMonsInfo, NUM_FISHING_MONS_ENCOUNTER_SLOTS, &gWildMonHeaders[h], WILD_AREA_FISHING, &outside);
        }
    }
    EXPECT_EQ(outside, 0);
}
