#include "global.h"
#include "caps.h"
#include "event_data.h"
#include "test/battle.h"

// Randolocke's hard level caps (src/caps.c): 14 with no badges, then 21 / 24 / 29 / 36 / 43 /
// 47 / 50, 63 for the Elite Four, and nothing once the player is Champion. With
// B_EXP_CAP_TYPE hard, a Pokemon at its cap gains no experience at all.

static const u8 sCapByBadges[] = { 14, 21, 24, 29, 36, 43, 47, 50, 63 };

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

TEST("Randolocke: the level cap rises with each badge, and lifts for the Champion")
{
    ASSUME(B_LEVEL_CAP_TYPE == LEVEL_CAP_FLAG_LIST);

    for (u32 badges = 0; badges <= NUM_BADGES; badges++)
    {
        SetBadges(badges);
        EXPECT_EQ(GetCurrentLevelCap(), sCapByBadges[badges]);
    }
    FlagSet(FLAG_IS_CHAMPION);
    EXPECT_EQ(GetCurrentLevelCap(), MAX_LEVEL);
}

WILD_BATTLE_TEST("Randolocke: a Pokemon at its level cap gains no experience")
{
    u32 badges = 0;

    PARAMETRIZE { badges = 0; }     // capped at 14: at the cap
    PARAMETRIZE { badges = 1; }     // capped at 21: under it

    GIVEN {
        ASSUME(B_EXP_CAP_TYPE == EXP_CAP_HARD);
        ASSUME(B_LEVEL_CAP_TYPE == LEVEL_CAP_FLAG_LIST);
        SetBadges(badges);
        PLAYER(SPECIES_WOBBUFFET) { Level(14); }
        OPPONENT(SPECIES_CATERPIE) { Level(10); HP(1); }
    } WHEN {
        TURN { MOVE(player, MOVE_SCRATCH); }
    } SCENE {
        MESSAGE("The wild Caterpie fainted!");
    } THEN {
        u32 start = gExperienceTables[gSpeciesInfo[SPECIES_WOBBUFFET].growthRate][14];

        if (badges == 0)
            EXPECT_EQ(GetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_EXP), start);
        else
            EXPECT_GT(GetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_EXP), start);
    }
}
