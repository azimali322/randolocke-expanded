#include "global.h"
#include "event_data.h"
#include "pokemon.h"
#include "randomizer.h"
#include "test/test.h"
#include "constants/move_relearner.h"

// The move relearner's level-up list, built the way GetRelearnerLevelUpMoves builds it
// under this project's settings -- P_ENABLE_ALL_LEVEL_UP_MOVES, so every level, and
// P_PRE_EVO_MOVES, so every stage of the family -- for a Pokemon that knows none of the
// moves. That is the longest the list can ever be.
static u32 RelearnerLevelUpListLength(enum Species species)
{
    u16 moves[256];
    u32 numMoves = 0;

    do
    {
        const struct LevelUpMove *learnset = GetSpeciesLevelUpLearnset(species);

        for (u32 i = 0; learnset[i].move != LEVEL_UP_MOVE_END; i++)
        {
            bool32 alreadyInList = FALSE;

            for (u32 j = 0; j < numMoves; j++)
            {
                if (moves[j] == learnset[i].move)
                    alreadyInList = TRUE;
            }
            if (!alreadyInList && numMoves < ARRAY_COUNT(moves))
                moves[numMoves++] = learnset[i].move;
        }
        species = GetSpeciesPreEvolution(species);
    } while (species != SPECIES_NONE);

    return numMoves;
}

// The randomizer is seeded from the full 32-bit Trainer ID.
static void SetTrainerIdSeed(u32 seed)
{
    gSaveBlock2Ptr->playerTrainerId[0] = seed;
    gSaveBlock2Ptr->playerTrainerId[1] = seed >> 8;
    gSaveBlock2Ptr->playerTrainerId[2] = seed >> 16;
    gSaveBlock2Ptr->playerTrainerId[3] = seed >> 24;
}

// Each stage of a family gets its own randomized 21-move learnset, so a three-stage family
// can list up to 63 level-up moves -- more than the 60 the relearner used to hold, which
// overran its arrays. Two-stage families top out at 42 and are not checked.
TEST("Randolocke: randomized relearner level-up lists fit within MAX_RELEARNER_MOVES")
{
    u32 seed = 0;

    PARAMETRIZE { seed = 0xEDFC0000; } // a playtest save
    PARAMETRIZE { seed = 0x12345678; } // the longest list measured: 63
    PARAMETRIZE { seed = 0x00000001; }

    FlagSet(RANDOMIZER_FLAG_LEARNSET);
    SetTrainerIdSeed(seed);

    for (u32 species = 1; species < NUM_SPECIES; species++)
    {
        enum Species parent;
        u32 length;

        if (!IsSpeciesEnabled(species))
            continue;
        parent = GetSpeciesPreEvolution(species);
        if (parent == SPECIES_NONE || GetSpeciesPreEvolution(parent) == SPECIES_NONE)
            continue;

        length = RelearnerLevelUpListLength(species);
        if (length > MAX_RELEARNER_MOVES)
            Test_MgbaPrintf("species %d lists %d level-up moves", species, length);
        EXPECT_LE(length, MAX_RELEARNER_MOVES);
    }
}
