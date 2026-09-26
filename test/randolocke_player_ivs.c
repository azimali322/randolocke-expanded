#include "global.h"
#include "config/randolocke.h"
#include "egg_hatch.h"
#include "event_data.h"
#include "test/battle.h"
#include "test/overworld_script.h"

// RANDOLOCKE_PLAYER_IVS: every Pokemon that becomes the player's -- given, hatched or
// caught -- has perfect IVs, whatever the script, the egg or the wild Pokemon had. Trainer
// parties are not the player's and keep theirs (test/randolocke_trainers.c).
//
// The upstream givemon and daycare tests check the IVs a script asks for, which this rule
// overrides by design; they run those checks only with the rule off.

#if RANDOLOCKE_PLAYER_IVS == RANDOLOCKE_IVS_PERFECT

static void ExpectPerfect(struct Pokemon *mon)
{
    for (u32 stat = 0; stat < NUM_STATS; stat++)
        EXPECT_EQ(GetMonData(mon, MON_DATA_HP_IV + stat, NULL), MAX_PER_STAT_IVS);
}

TEST("Randolocke: a Pokemon the player is given has perfect IVs, whatever the script says")
{
    ZeroPlayerPartyMons();
    RUN_OVERWORLD_SCRIPT(
        givemon SPECIES_WOBBUFFET, 50, hpIv=7, atkIv=8, defIv=9, speedIv=10, spAtkIv=11, spDefIv=12;
    );
    ExpectPerfect(&gParties[B_TRAINER_PLAYER][0]);
}

TEST("Randolocke: a hatched Pokemon has perfect IVs")
{
    ZeroPlayerPartyMons();
    RUN_OVERWORLD_SCRIPT(
        givemon SPECIES_WOBBUFFET, 5, hpIv=0, atkIv=0, defIv=0, speedIv=0, spAtkIv=0, spDefIv=0, isEgg=TRUE;
    );
    gSpecialVar_0x8004 = 0;
    ScriptHatchMon();
    EXPECT(!GetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_IS_EGG, NULL));
    ExpectPerfect(&gParties[B_TRAINER_PLAYER][0]);
}

WILD_BATTLE_TEST("Randolocke: a caught Pokemon has perfect IVs")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_CATERPIE) { HPIV(0); AttackIV(1); DefenseIV(2); SpAttackIV(3); SpDefenseIV(4); SpeedIV(5); }
    } WHEN {
        TURN { USE_ITEM(player, ITEM_MASTER_BALL); }
    } SCENE {
        ANIMATION(ANIM_TYPE_SPECIAL, B_ANIM_BALL_THROW, player);
    } THEN {
        EXPECT_EQ(GetMonData(&gParties[B_TRAINER_PLAYER][1], MON_DATA_SPECIES), SPECIES_CATERPIE);
        ExpectPerfect(&gParties[B_TRAINER_PLAYER][1]);
    }
}

#endif // RANDOLOCKE_PLAYER_IVS
