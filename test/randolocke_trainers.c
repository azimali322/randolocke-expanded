#include "global.h"
#include "battle.h"
#include "battle_setup.h"
#include "config/randomizer.h"
#include "data.h"
#include "event_data.h"
#include "pokemon.h"
#include "test/test.h"
#include "constants/items.h"
#include "constants/opponents.h"

// Trainer Pokemon are built by CreateNPCTrainerPartyFromTrainer, which is where the EV
// spread, the nature, the held item, the IVs and a boss's extra Pokemon are handed out.
//
// The test build replaces gTrainers with the test framework's own fixtures, so a real
// trainer id reads some unrelated test trainer -- or an empty one: TRAINER_ROXANNE_1 has no
// party there at all, which quietly made the first version of these tests check nothing.
// So the trainers below are defined here, shaped like the real ones. The id passed alongside
// is only a seed, and a name the Route 103 exception can recognise.

#define MON(sp, level) { .species = (sp), .lvl = (level), .gender = TRAINER_MON_RANDOM_GENDER }

static const struct TrainerMon sLeaderParty[] =
{
    MON(SPECIES_GEODUDE, 11), MON(SPECIES_NOSEPASS, 11), MON(SPECIES_GRAVELER, 14),
};
static const struct TrainerMon sEliteFourParty[] =
{
    MON(SPECIES_MIGHTYENA, 50), MON(SPECIES_SHIFTRY, 52), MON(SPECIES_CACTURNE, 50),
    MON(SPECIES_CRAWDAUNT, 52), MON(SPECIES_ABSOL, 53),
};
static const struct TrainerMon sRivalParty[] = { MON(SPECIES_RALTS, 16) };
static const struct TrainerMon sOrdinaryParty[] = { MON(SPECIES_GEODUDE, 20), MON(SPECIES_MACHOP, 21) };
static const struct TrainerMon sAdminParty[] = { MON(SPECIES_NUMEL, 45), MON(SPECIES_MIGHTYENA, 46), MON(SPECIES_CAMERUPT, 47) };

static const struct Trainer sLeader =
{
    .trainerName = _("LEADER"), .trainerClass = TRAINER_CLASS_LEADER, .isBossTrainer = TRUE,
    .party = sLeaderParty, .partySize = ARRAY_COUNT(sLeaderParty),
};
static const struct Trainer sEliteFour =
{
    .trainerName = _("ELITE"), .trainerClass = TRAINER_CLASS_ELITE_FOUR, .isBossTrainer = TRUE,
    .party = sEliteFourParty, .partySize = ARRAY_COUNT(sEliteFourParty),
};
static const struct Trainer sRival =
{
    .trainerName = _("RIVAL"), .trainerClass = TRAINER_CLASS_RIVAL,
    .party = sRivalParty, .partySize = ARRAY_COUNT(sRivalParty),
};
static const struct Trainer sOrdinary =
{
    .trainerName = _("HIKER"), .trainerClass = TRAINER_CLASS_HIKER,
    .party = sOrdinaryParty, .partySize = ARRAY_COUNT(sOrdinaryParty),
};
static const struct Trainer sHalfTeamAdmin =
{
    .trainerName = _("ADMIN"), .trainerClass = TRAINER_CLASS_MAGMA_ADMIN, .isBossTrainer = TRUE,
    .multiTeamSize = MULTI_TEAM_SIZE_HALF,
    .party = sAdminParty, .partySize = ARRAY_COUNT(sAdminParty),
};

static const u32 sIvFields[NUM_STATS] =
{
    MON_DATA_HP_IV, MON_DATA_ATK_IV, MON_DATA_DEF_IV,
    MON_DATA_SPEED_IV, MON_DATA_SPATK_IV, MON_DATA_SPDEF_IV,
};

static void SetBadges(u32 badges)
{
    for (u32 i = 0; i < NUM_BADGES; i++)
    {
        if (i < badges)
            FlagSet(FLAG_BADGE01_GET + i);
        else
            FlagClear(FLAG_BADGE01_GET + i);
    }
}

static struct Pokemon *Mon(u32 slot)
{
    return &gParties[B_TRAINER_OPPONENT_A][slot];
}

static u32 PartyCount(void)
{
    u32 n = 0;

    for (u32 i = 0; i < PARTY_SIZE; i++)
    {
        if (GetMonData(Mon(i), MON_DATA_SPECIES, NULL) != SPECIES_NONE)
            n++;
    }
    return n;
}

static u32 PerfectIVs(struct Pokemon *mon)
{
    u32 n = 0;

    for (u32 stat = 0; stat < NUM_STATS; stat++)
    {
        if (GetMonData(mon, sIvFields[stat], NULL) == MAX_PER_STAT_IVS)
            n++;
    }
    return n;
}

TEST("Randolocke: trainer Pokemon get a legal EV spread, a nature and an item")
{
    static const u32 sEvFields[NUM_STATS] =
    {
        MON_DATA_HP_EV, MON_DATA_ATK_EV, MON_DATA_DEF_EV,
        MON_DATA_SPEED_EV, MON_DATA_SPATK_EV, MON_DATA_SPDEF_EV,
    };

    SetBadges(NUM_BADGES);
    CreateNPCTrainerPartyFromTrainer(gParties[B_TRAINER_OPPONENT_A], &sLeader, TRAINER_ROXANNE_1);
    EXPECT_EQ(PartyCount(), PARTY_SIZE);

    for (u32 i = 0; i < PARTY_SIZE; i++)
    {
        struct Pokemon *mon = Mon(i);
        u32 total = 0, maxed = 0, nature;

        for (u32 stat = 0; stat < NUM_STATS; stat++)
        {
            u32 ev = GetMonData(mon, sEvFields[stat], NULL);

            total += ev;
            if (ev == MAX_PER_STAT_EVS)
                maxed++;
            EXPECT_LE(ev, MAX_PER_STAT_EVS);
        }
        // The legal budget, spent the way a player would spend it: two stats at the cap.
        EXPECT_LE(total, MAX_TOTAL_EVS);
        EXPECT_EQ(maxed, 2);

        // A nature that raises something. Hardy, the old default, raises nothing.
        nature = GetMonData(mon, MON_DATA_HIDDEN_NATURE, NULL);
        EXPECT(nature == NATURE_ADAMANT || nature == NATURE_JOLLY
            || nature == NATURE_MODEST || nature == NATURE_TIMID);

        // And it moves the numbers: the same Pokemon on the old neutral default is worse
        // in the stat the nature raises -- so CalculateMonStats is reading it.
        {
            static const u32 sStatFields[NUM_STATS] =
            {
                MON_DATA_MAX_HP, MON_DATA_ATK, MON_DATA_DEF,
                MON_DATA_SPEED, MON_DATA_SPATK, MON_DATA_SPDEF,
            };
            struct Pokemon neutral = *mon;
            u32 hardy = NATURE_HARDY;
            u32 field = sStatFields[gNaturesInfo[nature].statUp];

            SetMonData(&neutral, MON_DATA_HIDDEN_NATURE, &hardy);
            CalculateMonStats(&neutral);
            EXPECT_GT(GetMonData(mon, field, NULL), GetMonData(&neutral, field, NULL));
        }

        // Boss trainers always hold something -- the added Pokemon included.
        EXPECT_NE(GetMonData(mon, MON_DATA_HELD_ITEM, NULL), ITEM_NONE);
    }
}

// A boss's perfect IVs ramp with the badges (RZ_BOSS_IV_RAMP): one perfect Pokemon at none,
// half a step per badge -- a three-of-six Pokemon, then another perfect one -- to five at
// eight. The ace gets them first.
TEST("Randolocke: a boss's perfect IVs ramp with the badges")
{
    u32 badges = 0, perfect = 0, keyed = 0;

    PARAMETRIZE { badges = 0; }
    PARAMETRIZE { badges = 1; }
    PARAMETRIZE { badges = 2; }
    PARAMETRIZE { badges = 5; }
    PARAMETRIZE { badges = 8; }

    SetBadges(badges);
    CreateNPCTrainerPartyFromTrainer(gParties[B_TRAINER_OPPONENT_A], &sLeader, TRAINER_ROXANNE_1);
    for (u32 i = 0; i < PARTY_SIZE; i++)
    {
        u32 n = PerfectIVs(Mon(i));

        if (n == NUM_STATS)
            perfect++;
        else if (n >= 3)
            keyed++;
    }
    EXPECT_EQ(perfect, 1 + badges / 2);
    EXPECT_EQ(keyed, badges % 2);
    // The ace, still last, is always the first to be made perfect.
    EXPECT_EQ(PerfectIVs(Mon(PARTY_SIZE - 1)), NUM_STATS);
}

TEST("Randolocke: the Elite Four are perfect throughout")
{
    SetBadges(NUM_BADGES);
    CreateNPCTrainerPartyFromTrainer(gParties[B_TRAINER_OPPONENT_A], &sEliteFour, TRAINER_SIDNEY);
    EXPECT_EQ(PartyCount(), PARTY_SIZE);
    for (u32 i = 0; i < PARTY_SIZE; i++)
        EXPECT_EQ(PerfectIVs(Mon(i)), NUM_STATS);
}

// Vanilla gives every Pokemon of a trainer one flat IV value, scaled by the trainer's
// importance: 40% of ordinary trainers' Pokemon run 0 across the board. An ordinary
// trainer's should not be six copies of the same number.
TEST("Randolocke: an ordinary trainer's IVs are rolled")
{
    u32 identicalSpreads = 0;

    SetBadges(NUM_BADGES);
    CreateNPCTrainerPartyFromTrainer(gParties[B_TRAINER_OPPONENT_A], &sOrdinary, TRAINER_SAWYER_1);
    EXPECT_EQ(PartyCount(), sOrdinary.partySize);
    for (u32 i = 0; i < sOrdinary.partySize; i++)
    {
        u32 first = GetMonData(Mon(i), sIvFields[0], NULL);
        bool32 allSame = TRUE;

        for (u32 stat = 0; stat < NUM_STATS; stat++)
        {
            u32 iv = GetMonData(Mon(i), sIvFields[stat], NULL);

            EXPECT_LE(iv, MAX_PER_STAT_IVS);
            if (iv != first)
                allSame = FALSE;
        }
        if (allSame)
            identicalSpreads++;
    }
    // Six independent rolls landing on the same number is a 1 in 33 million accident; six
    // copies of one number is what the flat vanilla value looked like.
    EXPECT_EQ(identicalSpreads, 0);
}

// Bosses and rivals bring six (RZ_BOSS_FULL_PARTY). The added Pokemon take levels from the
// team's own range and go in ahead of the ace, which stays last.
TEST("Randolocke: bosses and rivals bring six Pokemon, the ace still last")
{
    const struct Trainer *trainer = NULL;
    u32 trainerId = 0, lowest = 255, highest = 0;

    PARAMETRIZE { trainer = &sLeader;    trainerId = TRAINER_ROXANNE_1; }
    PARAMETRIZE { trainer = &sEliteFour; trainerId = TRAINER_SIDNEY; }
    PARAMETRIZE { trainer = &sRival;     trainerId = TRAINER_WALLY_MAUVILLE; }

    SetBadges(NUM_BADGES);
    for (u32 i = 0; i < trainer->partySize; i++)
    {
        lowest = min(lowest, trainer->party[i].lvl);
        highest = max(highest, trainer->party[i].lvl);
    }

    CreateNPCTrainerPartyFromTrainer(gParties[B_TRAINER_OPPONENT_A], trainer, trainerId);
    EXPECT_EQ(PartyCount(), PARTY_SIZE);
    for (u32 i = 0; i < PARTY_SIZE; i++)
    {
        u32 level = GetMonData(Mon(i), MON_DATA_LEVEL, NULL);

        EXPECT_GE(level, lowest);
        EXPECT_LE(level, highest);
    }
    // The data file's last Pokemon is the ace, and it is still the last one out.
    EXPECT_EQ(GetMonData(Mon(PARTY_SIZE - 1), MON_DATA_SPECIES, NULL), trainer->party[trainer->partySize - 1].species);
    EXPECT_EQ(GetMonData(Mon(PARTY_SIZE - 1), MON_DATA_LEVEL, NULL), trainer->party[trainer->partySize - 1].lvl);
}

TEST("Randolocke: the first rival battle, ordinary trainers and half teams keep their size")
{
    const struct Trainer *trainer = NULL;
    u32 trainerId = 0, flags = 0;

    PARAMETRIZE { trainer = &sRival;         trainerId = TRAINER_MAY_ROUTE_103_MUDKIP; }
    PARAMETRIZE { trainer = &sRival;         trainerId = TRAINER_BRENDAN_ROUTE_103_TORCHIC; }
    PARAMETRIZE { trainer = &sOrdinary;      trainerId = TRAINER_SAWYER_1; }
    PARAMETRIZE { trainer = &sHalfTeamAdmin; trainerId = TRAINER_TABITHA_MOSSDEEP;
                  flags = BATTLE_TYPE_TRAINER | BATTLE_TYPE_TWO_OPPONENTS | BATTLE_TYPE_MULTI | BATTLE_TYPE_INGAME_PARTNER; }

    SetBadges(NUM_BADGES);
    gBattleTypeFlags = flags;
    CreateNPCTrainerPartyFromTrainer(gParties[B_TRAINER_OPPONENT_A], trainer, trainerId);
    gBattleTypeFlags = 0;
    EXPECT_EQ(PartyCount(), trainer->partySize);
}

TEST("Randolocke: a trainer's Pokemon are the same every time")
{
    enum Species species[PARTY_SIZE];
    u32 levels[PARTY_SIZE], items[PARTY_SIZE], ivs[PARTY_SIZE];

    SetBadges(3);
    CreateNPCTrainerPartyFromTrainer(gParties[B_TRAINER_OPPONENT_A], &sLeader, TRAINER_ROXANNE_1);
    for (u32 i = 0; i < PARTY_SIZE; i++)
    {
        species[i] = GetMonData(Mon(i), MON_DATA_SPECIES, NULL);
        levels[i] = GetMonData(Mon(i), MON_DATA_LEVEL, NULL);
        items[i] = GetMonData(Mon(i), MON_DATA_HELD_ITEM, NULL);
        ivs[i] = GetMonData(Mon(i), MON_DATA_ATK_IV, NULL);
    }

    CreateNPCTrainerPartyFromTrainer(gParties[B_TRAINER_OPPONENT_A], &sLeader, TRAINER_ROXANNE_1);
    for (u32 i = 0; i < PARTY_SIZE; i++)
    {
        EXPECT_EQ(species[i], GetMonData(Mon(i), MON_DATA_SPECIES, NULL));
        EXPECT_EQ(levels[i], GetMonData(Mon(i), MON_DATA_LEVEL, NULL));
        EXPECT_EQ(items[i], GetMonData(Mon(i), MON_DATA_HELD_ITEM, NULL));
        EXPECT_EQ(ivs[i], GetMonData(Mon(i), MON_DATA_ATK_IV, NULL));
    }
}
