#include "global.h"
#include "config/randomizer.h"
#include "event_data.h"
#include "pokemon.h"
#include "randomizer.h"
#include "test/test.h"

// Enhancement 1 (RZ_ABILITY_STABLE_ACROSS_EVOLUTION): a randomized ability is seeded from the
// family's first stage, so a Pokemon keeps its ability when it evolves instead of rolling a
// new one. Checked for every evolved species against its pre-evolution, slot by slot.
//
// Every species' pre-evolution is a walk over the whole species table, so each seed is split
// in two, every other species to a part, to keep a part well inside the runner's 60 seconds.
#define ABILITY_PARTS 2

#if RZ_ABILITY_STABLE_ACROSS_EVOLUTION == TRUE
// The randomizer is seeded from the full 32-bit Trainer ID.
static void SetTrainerIdSeed(u32 seed)
{
    gSaveBlock2Ptr->playerTrainerId[0] = seed;
    gSaveBlock2Ptr->playerTrainerId[1] = seed >> 8;
    gSaveBlock2Ptr->playerTrainerId[2] = seed >> 16;
    gSaveBlock2Ptr->playerTrainerId[3] = seed >> 24;
}

TEST("Randolocke: a randomized ability survives evolution")
{
    static const u32 sSeeds[] = { 0xEDFC0000, 0x12345678 };
    u32 seed = 0, part = 0;

    for (u32 i = 0; i < ARRAY_COUNT(sSeeds); i++)
    {
        for (u32 p = 0; p < ABILITY_PARTS; p++)
            PARAMETRIZE { seed = sSeeds[i]; part = p; }
    }

    FlagSet(RANDOMIZER_FLAG_ABILITIES);
    SetTrainerIdSeed(seed);

    for (u32 species = 1 + part; species < NUM_SPECIES; species += ABILITY_PARTS)
    {
        enum Species parent;

        if (!IsSpeciesEnabled(species))
            continue;
        parent = GetSpeciesPreEvolution(species);
        if (parent == SPECIES_NONE)
            continue;

        for (u32 slot = 0; slot < NUM_ABILITY_SLOTS; slot++)
        {
            enum Ability evolved = gSpeciesInfo[species].abilities[slot];
            enum Ability before = gSpeciesInfo[parent].abilities[slot];

            // An empty slot is not randomized, so there is nothing to keep.
            if (evolved == ABILITY_NONE || before == ABILITY_NONE)
                continue;
            EXPECT_EQ(RandomizeAbility(species, slot, evolved), RandomizeAbility(parent, slot, before));
        }
    }
}

TEST("Randolocke: a Pokemon keeps its randomized ability through a whole evolution line")
{
    static const u16 sLine[] = { SPECIES_TORCHIC, SPECIES_COMBUSKEN, SPECIES_BLAZIKEN };
    struct Pokemon mon;
    enum Ability first;

    FlagSet(RANDOMIZER_FLAG_ABILITIES);
    SetTrainerIdSeed(0xEDFC0000);

    // The way evolution does it: the species changes under the same Pokemon.
    CreateMon(&mon, sLine[0], 20, 0, OTID_STRUCT_PLAYER_ID);
    first = GetMonAbility(&mon);
    EXPECT_NE(first, ABILITY_NONE);
    for (u32 i = 1; i < ARRAY_COUNT(sLine); i++)
    {
        u16 species = sLine[i];

        SetMonData(&mon, MON_DATA_SPECIES, &species);
        EXPECT_EQ(GetMonAbility(&mon), first);
    }
}
#endif
