#include "global.h"
#include "battle_setup.h"
#include "config/randomizer.h"
#include "data.h"
#include "event_data.h"
#include "pokemon.h"
#include "test/test.h"
#include "constants/items.h"
#include "constants/opponents.h"

// Trainer Pokemon are built by CreateNPCTrainerPartyFromTrainer, which is where the EV
// spread, the nature and the held item are handed out. Roxanne is a `Boss: Yes` trainer,
// so every one of hers should be holding something.
static void GiveAllBadges(void)
{
    for (u32 i = 0; i < NUM_BADGES; i++)
        FlagSet(FLAG_BADGE01_GET + i);
}

TEST("Randolocke: trainer Pokemon get a legal EV spread, a nature and an item")
{
    static const u32 sEvFields[NUM_STATS] =
    {
        MON_DATA_HP_EV, MON_DATA_ATK_EV, MON_DATA_DEF_EV,
        MON_DATA_SPEED_EV, MON_DATA_SPATK_EV, MON_DATA_SPDEF_EV,
    };
    const struct Trainer *trainer = GetTrainerStructFromId(TRAINER_ROXANNE_1);

    GiveAllBadges();
    CreateNPCTrainerPartyFromTrainer(gParties[B_TRAINER_OPPONENT_A], trainer, TRAINER_ROXANNE_1);

    for (u32 i = 0; i < trainer->partySize; i++)
    {
        struct Pokemon *mon = &gParties[B_TRAINER_OPPONENT_A][i];
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
        EXPECT_NE(gNaturesInfo[nature].statUp, gNaturesInfo[nature].statDown);

        // And it moves the numbers: the same Pokemon on the old neutral default is worse
        // in the stat the nature raises. This is the part worth checking -- the nature is
        // written as the hidden one, and CalculateMonStats has to be reading that.
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

        // Boss trainers always hold something.
        EXPECT_NE(GetMonData(mon, MON_DATA_HELD_ITEM, NULL), ITEM_NONE);
    }
}

// Vanilla gives every Pokemon of a trainer one flat IV value, scaled by the trainer's
// importance: 40% of ordinary trainers' Pokemon run 0 across the board, and 30% of a
// boss's are below 31. A boss should not be fighting with a 6 IV Pokemon, and an ordinary
// trainer's should not be six copies of the same number.
TEST("Randolocke: a boss's Pokemon are perfect, an ordinary trainer's are rolled")
{
    static const u32 sIvFields[NUM_STATS] =
    {
        MON_DATA_HP_IV, MON_DATA_ATK_IV, MON_DATA_DEF_IV,
        MON_DATA_SPEED_IV, MON_DATA_SPATK_IV, MON_DATA_SPDEF_IV,
    };
    const struct Trainer *boss = GetTrainerStructFromId(TRAINER_ROXANNE_1);
    const struct Trainer *ordinary = GetTrainerStructFromId(TRAINER_SAWYER_1);
    u32 identicalSpreads = 0;

    GiveAllBadges();

    CreateNPCTrainerPartyFromTrainer(gParties[B_TRAINER_OPPONENT_A], boss, TRAINER_ROXANNE_1);
    for (u32 i = 0; i < boss->partySize; i++)
    {
        for (u32 stat = 0; stat < NUM_STATS; stat++)
            EXPECT_EQ(GetMonData(&gParties[B_TRAINER_OPPONENT_A][i], sIvFields[stat], NULL), MAX_PER_STAT_IVS);
    }

    CreateNPCTrainerPartyFromTrainer(gParties[B_TRAINER_OPPONENT_A], ordinary, TRAINER_SAWYER_1);
    for (u32 i = 0; i < ordinary->partySize; i++)
    {
        struct Pokemon *mon = &gParties[B_TRAINER_OPPONENT_A][i];
        u32 first = GetMonData(mon, sIvFields[0], NULL);
        bool32 allSame = TRUE;

        for (u32 stat = 0; stat < NUM_STATS; stat++)
        {
            u32 iv = GetMonData(mon, sIvFields[stat], NULL);

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

TEST("Randolocke: a trainer's Pokemon are the same every time")
{
    const struct Trainer *trainer = GetTrainerStructFromId(TRAINER_ROXANNE_1);
    u32 firstItems[PARTY_SIZE] = {0};

    GiveAllBadges();
    CreateNPCTrainerPartyFromTrainer(gParties[B_TRAINER_OPPONENT_A], trainer, TRAINER_ROXANNE_1);
    for (u32 i = 0; i < trainer->partySize; i++)
        firstItems[i] = GetMonData(&gParties[B_TRAINER_OPPONENT_A][i], MON_DATA_HELD_ITEM, NULL);

    CreateNPCTrainerPartyFromTrainer(gParties[B_TRAINER_OPPONENT_A], trainer, TRAINER_ROXANNE_1);
    for (u32 i = 0; i < trainer->partySize; i++)
        EXPECT_EQ(firstItems[i], GetMonData(&gParties[B_TRAINER_OPPONENT_A][i], MON_DATA_HELD_ITEM, NULL));

    // Rolled IVs have to be stable too, or an ordinary trainer is a different fight every
    // time you walk back into them.
    {
        const struct Trainer *ordinary = GetTrainerStructFromId(TRAINER_SAWYER_1);
        u32 firstIvs[PARTY_SIZE] = {0};

        CreateNPCTrainerPartyFromTrainer(gParties[B_TRAINER_OPPONENT_A], ordinary, TRAINER_SAWYER_1);
        for (u32 i = 0; i < ordinary->partySize; i++)
            firstIvs[i] = GetMonData(&gParties[B_TRAINER_OPPONENT_A][i], MON_DATA_ATK_IV, NULL);

        CreateNPCTrainerPartyFromTrainer(gParties[B_TRAINER_OPPONENT_A], ordinary, TRAINER_SAWYER_1);
        for (u32 i = 0; i < ordinary->partySize; i++)
            EXPECT_EQ(firstIvs[i], GetMonData(&gParties[B_TRAINER_OPPONENT_A][i], MON_DATA_ATK_IV, NULL));
    }
}
