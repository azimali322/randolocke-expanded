#include "global.h"
#include "event_data.h"
#include "move.h"
#include "pokemon.h"
#include "randomizer.h"
#include "test/test.h"

// The randomizer is seeded from the full 32-bit Trainer ID.
static void SetTrainerIdSeed(u32 seed)
{
    gSaveBlock2Ptr->playerTrainerId[0] = seed;
    gSaveBlock2Ptr->playerTrainerId[1] = seed >> 8;
    gSaveBlock2Ptr->playerTrainerId[2] = seed >> 16;
    gSaveBlock2Ptr->playerTrainerId[3] = seed >> 24;
}

// Whether `move` is one this species needs to evolve -- the slots the evolution-move
// guarantee writes on purpose, whatever group they fall in.
static bool32 IsEvolutionMove(enum Species species, enum Move move)
{
    const struct Evolution *evos = GetSpeciesEvolutions(species);

    for (u32 i = 0; evos != NULL && evos[i].method != EVOLUTIONS_END; i++)
    {
        const struct EvolutionParam *params = evos[i].params;

        for (u32 j = 0; params != NULL && params[j].condition != CONDITIONS_END; j++)
        {
            if ((params[j].condition == IF_KNOWS_MOVE || params[j].condition == IF_USED_MOVE_X_TIMES)
             && params[j].arg1 == move)
                return TRUE;
            if (params[j].condition == IF_KNOWS_MOVE_TYPE && params[j].arg1 == GetMoveType(move))
                return TRUE;
        }
    }
    return FALSE;
}

// Every learnset is 7 STAB, 7 status and 7 non-STAB damaging moves, interleaved so slot i
// belongs to group i % 3. A slot that breaks its group's rule is one the deal could not
// fill: those used to be padded with Tackle -- 911 of the 914 Tackles in two seeds'
// learnsets -- which is neither STAB for most species nor non-STAB for Normal ones, and a
// second pad in a group repeats it. So: every slot keeps to its group, and no learnset
// lists a move twice.
TEST("Randolocke: randomized learnsets fill every group")
{
    u32 seed = 0;
    u32 broken = 0, repeated = 0, tackles = 0;

    PARAMETRIZE { seed = 0x8561D8DD; } // a playtest save
    PARAMETRIZE { seed = 0x12345678; }
    PARAMETRIZE { seed = 0x00000001; }

    FlagSet(RANDOMIZER_FLAG_LEARNSET);
    SetTrainerIdSeed(seed);

    for (u32 species = 1; species < NUM_SPECIES; species++)
    {
        const struct LevelUpMove *learnset;
        enum Type t1, t2;

        if (!IsSpeciesEnabled(species))
            continue;
        learnset = GetSpeciesLevelUpLearnset(species);
        t1 = gSpeciesInfo[species].types[0];
        t2 = gSpeciesInfo[species].types[1];

        for (u32 i = 0; learnset[i].move != LEVEL_UP_MOVE_END; i++)
        {
            enum Move move = learnset[i].move;
            bool32 stab = (GetMoveType(move) == t1 || GetMoveType(move) == t2);
            bool32 status = (GetMoveCategory(move) == DAMAGE_CATEGORY_STATUS);
            bool32 fits;

            if (move == MOVE_TACKLE)
                tackles++;
            for (u32 j = 0; j < i; j++)
            {
                if (learnset[j].move == move)
                    repeated++;
            }
            if (IsEvolutionMove(species, move))
                continue;

            switch (i % 3)
            {
            case 0:  fits = !status && stab;  break;   // STAB
            case 1:  fits = status;           break;   // status
            default: fits = !status && !stab; break;   // non-STAB damaging
            }
            if (!fits)
            {
                if (broken < 5)
                    Test_MgbaPrintf("species %d slot %d: move %d does not fit its group", species, i, move);
                broken++;
            }
        }
    }
    Test_MgbaPrintf("%d Tackles left across every species", tackles);
    EXPECT_EQ(broken, 0);
    EXPECT_EQ(repeated, 0);
}

// Seventeen species evolve by knowing a move, using one twenty times, or knowing a move
// of a type. The randomized learnset must still teach it, or the evolution cannot happen.
// Read from the evolution table the same way the game reads it, so a species the data
// adds later is checked without being listed here.
TEST("Randolocke: a Pokemon that evolves by a move can always learn it")
{
    u32 seed = 0;
    u32 checked = 0;

    PARAMETRIZE { seed = 0x8561D8DD; }
    PARAMETRIZE { seed = 0x12345678; }
    PARAMETRIZE { seed = 0x00000001; }
    PARAMETRIZE { seed = 0xDEADBEEF; }

    FlagSet(RANDOMIZER_FLAG_LEARNSET);
    SetTrainerIdSeed(seed);

    for (u32 species = 1; species < NUM_SPECIES; species++)
    {
        const struct Evolution *evos;
        bool32 counted = FALSE;

        if (!IsSpeciesEnabled(species))
            continue;
        evos = GetSpeciesEvolutions(species);
        for (u32 i = 0; evos != NULL && evos[i].method != EVOLUTIONS_END; i++)
        {
            const struct EvolutionParam *params = evos[i].params;

            for (u32 j = 0; params != NULL && params[j].condition != CONDITIONS_END; j++)
            {
                enum EvolutionConditions condition = params[j].condition;
                const struct LevelUpMove *learnset;
                bool32 learns = FALSE;

                if (condition != IF_KNOWS_MOVE && condition != IF_USED_MOVE_X_TIMES
                 && condition != IF_KNOWS_MOVE_TYPE)
                    continue;

                learnset = GetSpeciesLevelUpLearnset(species);
                for (u32 k = 0; learnset[k].move != LEVEL_UP_MOVE_END; k++)
                {
                    if (condition == IF_KNOWS_MOVE_TYPE ? GetMoveType(learnset[k].move) == params[j].arg1
                                                        : learnset[k].move == params[j].arg1)
                        learns = TRUE;
                }
                if (!learns)
                    Test_MgbaPrintf("species %d cannot learn what it needs to evolve into %d",
                                    species, evos[i].targetSpecies);
                EXPECT(learns);
                if (!counted)
                {
                    checked++;
                    counted = TRUE;
                }
            }
        }
    }
    // Steenee, Bonsly, Mime Jr., Aipom, Yanma, Tangela, Piloswine, Lickitung, Girafarig,
    // Dunsparce, Hisuian Qwilfish, Poipole, Clobbopus, Dipplin, Primeape, Stantler, Eevee.
    EXPECT_GE(checked, 17);
}

// The twelve legendary sites deal from the box legendaries only, one entry per Pokemon:
// no sub-legendary, mythical or Ultra Beast, no battle-only form (Zygarde Complete and
// Mega are permitted species), and never the same Pokemon at two sites.
TEST("Randolocke: the legendary sites give twelve different box legendaries")
{
    u32 seed = 0;
    enum Species dealt[LEGENDARY_MON_COUNT];

    PARAMETRIZE { seed = 0x8561D8DD; }
    PARAMETRIZE { seed = 0x8527357E; }
    PARAMETRIZE { seed = 0x12345678; }
    PARAMETRIZE { seed = 0x00000001; }
    PARAMETRIZE { seed = 0xDEADBEEF; }

    SetTrainerIdSeed(seed);

    for (u32 i = 0; i < LEGENDARY_MON_COUNT; i++)
    {
        dealt[i] = RandomizeLegendaryMon(gLegendaryMonTable[i]);
        EXPECT(gSpeciesInfo[dealt[i]].isRestrictedLegendary);
        EXPECT_EQ(dealt[i], GET_BASE_SPECIES_ID(dealt[i]));
        for (u32 j = 0; j < i; j++)
            EXPECT_NE(dealt[j], dealt[i]);
    }
}
