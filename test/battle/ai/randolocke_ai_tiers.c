#include "global.h"
#include "battle.h"
#include "battle_ai_main.h"
#include "battle_setup.h"
#include "config/randomizer.h"
#include "data.h"
#include "recorded_battle.h"
#include "test/test.h"
#include "constants/battle_ai.h"
#include "constants/trainers.h"

// RZ_TRAINER_AI_TIERS raises the story's own trainers to the AI their place in it
// deserves. These pin the tiers and, more to the point, where they apply: an ordinary
// trainer battle, and nothing else.
//
// The tiers used to be added after GetAiFlags had chosen by battle type, which put them on
// every battle type. The test runner plays each battle test back as a recorded battle
// against its fixture trainer 2 -- a Rival -- so 43 AI tests fought with the Notable tier
// on top of the flags they asked for. The Frontier took them too, through trainer ids that
// index its own table and only coincide with story trainers' numbers.
//
// The test build replaces the trainer table with the framework's fixtures, so there is no
// real Roxanne or Wallace to look up: the table is checked through RandolockeAiTierFlags,
// and where it applies through whichever fixture trainer is a Rival.

#if RZ_TRAINER_AI_TIERS == TRUE
static bool32 HasAll(u64 flags, u64 wanted)
{
    return (flags & wanted) == wanted;
}

// Flags a battle of `battleType` gives `trainerId`; anything a dynamic AI function adds is
// left out, since whether one is registered depends on what ran before.
//
// The trainer is also put in TRAINER_BATTLE_PARAM.opponentA, as a real battle against it
// would be. GetAiFlags returns nothing at all when IsSmartBattle sees a link, union-room or
// secret-base opponent there, and the battle tests leave one behind: every singles battle
// test fights TRAINER_LINK_OPPONENT. Run after one of those -- the order depends on how the
// suite is split across runners -- this test saw no flags and failed, while passing alone.
static u64 FlagsIn(u32 battleType, u16 trainerId)
{
    u32 saved = gBattleTypeFlags;
    u16 savedOpponent = TRAINER_BATTLE_PARAM.opponentA;
    u64 flags;

    gBattleTypeFlags = battleType;
    TRAINER_BATTLE_PARAM.opponentA = trainerId;
    flags = RandolockeTestGetAiFlags(trainerId, B_BATTLER_1) & ~AI_FLAG_DYNAMIC_FUNC;
    TRAINER_BATTLE_PARAM.opponentA = savedOpponent;
    gBattleTypeFlags = saved;
    return flags;
}

static u16 FixtureRival(void)
{
    for (u32 id = 1; id < 32; id++)
    {
        if (GetTrainerClassFromId(id) == TRAINER_CLASS_RIVAL)
            return id;
    }
    return TRAINER_NONE;
}

TEST("Randolocke: story trainers fight with the AI tier their place deserves")
{
    // The Champion reads ahead; anyone tagged Boss -- gym leaders, the Elite Four, the
    // team leaders -- is omniscient and switches well.
    EXPECT(RandolockeAiTierFlags(TRAINER_CLASS_CHAMPION, TRUE) == RZ_AI_CHAMPION);
    EXPECT(RandolockeAiTierFlags(TRAINER_CLASS_LEADER, TRUE) == RZ_AI_BOSS);
    EXPECT(RandolockeAiTierFlags(TRAINER_CLASS_ELITE_FOUR, TRUE) == RZ_AI_BOSS);
    EXPECT(RandolockeAiTierFlags(TRAINER_CLASS_MAGMA_LEADER, TRUE) == RZ_AI_BOSS);

    // Rivals and admins are notable without the tag.
    EXPECT(RandolockeAiTierFlags(TRAINER_CLASS_RIVAL, FALSE) == RZ_AI_NOTABLE);
    EXPECT(RandolockeAiTierFlags(TRAINER_CLASS_AQUA_ADMIN, FALSE) == RZ_AI_NOTABLE);

    // Everyone else gets the base tier, and none of these tiers includes the one above.
    EXPECT(RandolockeAiTierFlags(TRAINER_CLASS_HIKER, FALSE) == RZ_AI_BASE);
    EXPECT(!(RZ_AI_NOTABLE & AI_FLAG_OMNISCIENT));
    EXPECT(!(RZ_AI_BOSS & AI_FLAG_PREDICT_MOVE));
}

TEST("Randolocke: only an ordinary trainer battle adds the AI tiers")
{
    const u64 facility = AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT;
    u16 rival = FixtureRival();
    u64 recorded, flags;

    ASSUME(rival != TRAINER_NONE);

    // A story battle against a rival: the Notable tier, whatever its file says.
    EXPECT(HasAll(FlagsIn(BATTLE_TYPE_TRAINER, rival), RZ_AI_NOTABLE));

    // The Frontier, the Trainer Hill and secret bases have their own AI, and a trainer id
    // there means something else entirely. (BATTLE_TYPE_FRONTIER is every facility at once,
    // the Factory included, which has a branch of its own; a real battle sets one.)
    EXPECT(FlagsIn(BATTLE_TYPE_TRAINER | BATTLE_TYPE_BATTLE_TOWER, rival) == facility);
    EXPECT(FlagsIn(BATTLE_TYPE_TRAINER | BATTLE_TYPE_DOME, rival) == facility);
    EXPECT(FlagsIn(BATTLE_TYPE_TRAINER | BATTLE_TYPE_TRAINER_HILL, rival) == facility);
    EXPECT(FlagsIn(BATTLE_TYPE_TRAINER | BATTLE_TYPE_SECRET_BASE, rival) == facility);

    // A recorded battle plays back what it was recorded with. The only bits GetAiFlags may
    // add are the ones it always derives from others.
    recorded = GetAiScriptsInRecordedBattle(B_BATTLER_1);
    flags = FlagsIn(BATTLE_TYPE_TRAINER | BATTLE_TYPE_RECORDED, rival);
    EXPECT(!(flags & ~recorded & ~(AI_FLAG_SMART_MON_CHOICES | AI_FLAG_PREDICT_SWITCH)));
}
#endif
