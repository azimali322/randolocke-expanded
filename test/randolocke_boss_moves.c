#include "global.h"
#include "battle.h"
#include "battle_setup.h"
#include "config/randomizer.h"
#include "data.h"
#include "event_data.h"
#include "move.h"
#include "pokemon.h"
#include "trainer_util.h"
#include "test/test.h"
#include "constants/battle_stat_change.h"
#include "constants/opponents.h"

// A boss chooses its four moves from what its species learns by its level
// (RZ_BOSS_SMART_MOVESETS): its strongest same-type attack, an attack of another type that
// covers what that one cannot, a setup move, and the best of the rest. Everyone else keeps
// the last four moves they learned. See RandolockeChooseBossMoves in src/trainer_util.c.
//
// Walking every species through the randomizer is too slow for one test (a randomized
// learnset takes about a tenth of a second to deal), so the sweeps below take every
// SPECIES_STEP-th species, on vanilla learnsets and on two randomized seeds.

#define SPECIES_STEP 9

#if RZ_BOSS_SMART_MOVESETS == TRUE

static const u8 sLevels[] = { 5, 14, 36, 63 };

// The randomizer is seeded from the full 32-bit Trainer ID.
static void SetTrainerIdSeed(u32 seed)
{
    gSaveBlock2Ptr->playerTrainerId[0] = seed;
    gSaveBlock2Ptr->playerTrainerId[1] = seed >> 8;
    gSaveBlock2Ptr->playerTrainerId[2] = seed >> 16;
    gSaveBlock2Ptr->playerTrainerId[3] = seed >> 24;
}

// Everything `species` learns by `level`, once each: the moves a boss chooses from.
static u32 Pool(enum Species species, u32 level, enum Move *pool, u32 max)
{
    const struct LevelUpMove *learnset = GetSpeciesLevelUpLearnset(species);
    u32 n = 0, i, j;

    for (i = 0; learnset[i].move != LEVEL_UP_MOVE_END && learnset[i].level <= level && n < max; i++)
    {
        if (learnset[i].level == 0)
            continue;
        for (j = 0; j < n && pool[j] != learnset[i].move; j++)
            ;
        if (j == n)
            pool[n++] = learnset[i].move;
    }
    return n;
}

static bool32 Contains(const enum Move *moves, u32 count, enum Move move)
{
    for (u32 i = 0; i < count; i++)
    {
        if (moves[i] == move)
            return TRUE;
    }
    return FALSE;
}

static bool32 IsAttack(enum Move move)
{
    return move != MOVE_NONE && GetMoveCategory(move) != DAMAGE_CATEGORY_STATUS;
}

static bool32 IsStab(enum Move move, enum Species species)
{
    return GetMoveType(move) == gSpeciesInfo[species].types[0]
        || GetMoveType(move) == gSpeciesInfo[species].types[1];
}

// A status move a Pokemon uses on itself to raise Speed, or the stat one of its first two
// moves attacks from -- the attacks a boss chooses before it chooses to set up. The spec
// for "a setup move", written out independently of the picker.
static bool32 IsSetupFor(enum Move move, enum Species species, const enum Move *moves)
{
    bool32 physical = FALSE, special = FALSE;
    u32 i;

    if (GetMoveCategory(move) != DAMAGE_CATEGORY_STATUS)
        return FALSE;
    if (GetMoveEffect(move) == EFFECT_CURSE)
    {
        if (gSpeciesInfo[species].types[0] == TYPE_GHOST || gSpeciesInfo[species].types[1] == TYPE_GHOST)
            return FALSE;
    }
    else if (GetMoveTarget(move) != TARGET_USER && GetMoveTarget(move) != TARGET_USER_AND_ALLY)
    {
        return FALSE;
    }

    for (i = 0; i < 2; i++)
    {
        if (!IsAttack(moves[i]))
            continue;
        if (GetMoveCategory(moves[i]) == DAMAGE_CATEGORY_PHYSICAL)
            physical = TRUE;
        else
            special = TRUE;
    }
    for (i = 0; i < GetMoveAdditionalEffectCount(move); i++)
    {
        const struct AdditionalEffect *effect = GetMoveAdditionalEffectById(move, i);

        if (effect->moveEffect != STAT_CHANGE_EFFECT_PLUS)
            continue;
        if (effect->speed || (physical && effect->attack) || (special && effect->spAtk))
            return TRUE;
    }
    return FALSE;
}

TEST("Randolocke: Roxanne's Geodude chooses its boss moveset from its learnset")
{
    enum Move moves[MAX_MON_MOVES];
    enum Move expected[MAX_MON_MOVES] = {MOVE_NONE};
    u32 level = 0;

    // With learnsets left alone, Geodude's own: at 14 the last four it learned would be
    // Defense Curl, Rock Polish, Rollout and Bulldoze. As a boss it drops Defense Curl for
    // Tackle; at 36 it has Earthquake, Rock Blast, Rock Polish and, with no new type left
    // that adds any coverage, the Stealth Rock the tier list rates over a second Ground move.
    PARAMETRIZE { level = 14; expected[0] = MOVE_BULLDOZE; expected[1] = MOVE_ROLLOUT; expected[2] = MOVE_ROCK_POLISH; expected[3] = MOVE_TACKLE; }
    PARAMETRIZE { level = 36; expected[0] = MOVE_EARTHQUAKE; expected[1] = MOVE_ROCK_BLAST; expected[2] = MOVE_ROCK_POLISH; expected[3] = MOVE_STEALTH_ROCK; }

    ASSUME(P_LVL_UP_LEARNSETS >= GEN_9);
    ASSUME(!FlagGet(RANDOMIZER_FLAG_LEARNSET));

    RandolockeChooseBossMoves(SPECIES_GEODUDE, level, moves);
    for (u32 i = 0; i < MAX_MON_MOVES; i++)
        EXPECT_EQ(moves[i], expected[i]);
}

TEST("Randolocke: a boss only knows moves it learns by its level")
{
    enum Move pool[64], moves[MAX_MON_MOVES];
    u32 seed = 0;

    PARAMETRIZE { seed = 0; }           // learnsets left alone
    PARAMETRIZE { seed = 0xEDFC0000; }  // randomized, a playtest save
    PARAMETRIZE { seed = 0x12345678; }  // randomized

    if (seed != 0)
    {
        FlagSet(RANDOMIZER_FLAG_LEARNSET);
        SetTrainerIdSeed(seed);
    }

    for (u32 species = 1; species < NUM_SPECIES; species += SPECIES_STEP)
    {
        if (!IsSpeciesEnabled(species))
            continue;
        for (u32 l = 0; l < ARRAY_COUNT(sLevels); l++)
        {
            u32 n = Pool(species, sLevels[l], pool, ARRAY_COUNT(pool)), known = 0;

            RandolockeChooseBossMoves(species, sLevels[l], moves);
            for (u32 i = 0; i < MAX_MON_MOVES; i++)
            {
                if (moves[i] == MOVE_NONE)
                    continue;
                known++;
                // Learned by now, and known once.
                EXPECT(Contains(pool, n, moves[i]));
                EXPECT(!Contains(moves, i, moves[i]));
            }
            // As many as it has, up to four.
            EXPECT_EQ(known, n < MAX_MON_MOVES ? n : MAX_MON_MOVES);
        }
    }
}

TEST("Randolocke: a boss brings a same-type attack, coverage and setup whenever it learns them")
{
    enum Move pool[64], moves[MAX_MON_MOVES];
    u32 seed = 0;

    PARAMETRIZE { seed = 0; }
    PARAMETRIZE { seed = 0xEDFC0000; }
    PARAMETRIZE { seed = 0x12345678; }

    if (seed != 0)
    {
        FlagSet(RANDOMIZER_FLAG_LEARNSET);
        SetTrainerIdSeed(seed);
    }

    for (u32 species = 1; species < NUM_SPECIES; species += SPECIES_STEP)
    {
        if (!IsSpeciesEnabled(species))
            continue;
        for (u32 l = 0; l < ARRAY_COUNT(sLevels); l++)
        {
            u32 n = Pool(species, sLevels[l], pool, ARRAY_COUNT(pool)), i;
            bool32 poolStab = FALSE, knowsStab = FALSE, poolSetup = FALSE, knowsSetup = FALSE;
            u32 poolTypes = 0, knownTypes = 0;

            RandolockeChooseBossMoves(species, sLevels[l], moves);
            for (i = 0; i < n; i++)
            {
                if (IsAttack(pool[i]))
                {
                    poolStab |= IsStab(pool[i], species);
                    poolTypes |= 1u << GetMoveType(pool[i]);
                }
                poolSetup |= IsSetupFor(pool[i], species, moves);
            }
            for (i = 0; i < MAX_MON_MOVES; i++)
            {
                if (IsAttack(moves[i]))
                {
                    knowsStab |= IsStab(moves[i], species);
                    knownTypes |= 1u << GetMoveType(moves[i]);
                }
                knowsSetup |= IsSetupFor(moves[i], species, moves);
            }

            if (poolStab)
                EXPECT(knowsStab);
            // Two attacking types when it learns two.
            if (poolTypes & (poolTypes - 1))
                EXPECT(knownTypes & (knownTypes - 1));
            if (poolSetup)
                EXPECT(knowsSetup);
        }
    }
}

// The test build replaces the trainer table with the framework's fixtures, so the trainers
// here are defined locally; the id is only a seed. See test/randolocke_trainers.c.
#define MON(sp, level) { .species = (sp), .lvl = (level), .gender = TRAINER_MON_RANDOM_GENDER }

static const struct TrainerMon sParty[] =
{
    MON(SPECIES_GEODUDE, 36), MON(SPECIES_MACHOKE, 36), MON(SPECIES_KADABRA, 38),
};
static const struct Trainer sBoss =
{
    .trainerName = _("LEADER"), .trainerClass = TRAINER_CLASS_LEADER, .isBossTrainer = TRUE,
    .party = sParty, .partySize = ARRAY_COUNT(sParty),
};
static const struct Trainer sOrdinary =
{
    .trainerName = _("HIKER"), .trainerClass = TRAINER_CLASS_HIKER,
    .party = sParty, .partySize = ARRAY_COUNT(sParty),
};

TEST("Randolocke: bosses choose their moves, and ordinary trainers keep the last four they learned")
{
    struct Pokemon *party = gParties[B_TRAINER_OPPONENT_A];
    const struct Trainer *trainer = NULL;
    enum Move moves[MAX_MON_MOVES];
    u32 i, j;

    PARAMETRIZE { trainer = &sBoss; }
    PARAMETRIZE { trainer = &sOrdinary; }

    FlagSet(RANDOMIZER_FLAG_LEARNSET);
    SetTrainerIdSeed(0xEDFC0000);
    CreateNPCTrainerPartyFromTrainer(party, trainer, TRAINER_ROXANNE_1);

    for (i = 0; i < PARTY_SIZE; i++)
    {
        enum Species species = GetMonData(&party[i], MON_DATA_SPECIES, NULL);

        if (species == SPECIES_NONE)
            continue;
        if (trainer->isBossTrainer)
        {
            RandolockeChooseBossMoves(species, GetMonData(&party[i], MON_DATA_LEVEL, NULL), moves);
        }
        else
        {
            struct Pokemon copy = party[i];

            GiveMonInitialMoveset(&copy);
            for (j = 0; j < MAX_MON_MOVES; j++)
                moves[j] = GetMonData(&copy, MON_DATA_MOVE1 + j, NULL);
        }
        for (j = 0; j < MAX_MON_MOVES; j++)
        {
            EXPECT_EQ(GetMonData(&party[i], MON_DATA_MOVE1 + j, NULL), moves[j]);
            EXPECT_EQ(GetMonData(&party[i], MON_DATA_PP1 + j, NULL), GetMovePP(moves[j]));
        }
    }
}

#endif // RZ_BOSS_SMART_MOVESETS
