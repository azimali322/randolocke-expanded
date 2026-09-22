#include "global.h"
#include "config/randolocke.h"
#include "randolocke_battle_log.h"
#include "string_util.h"
#include "constants/characters.h"
#include "test/battle.h"

#if RANDOLOCKE_BATTLE_LOG == TRUE

// RANDOLOCKE_BATTLE_LOG keeps the battle's messages for SELECT at the battle menu to replay:
// everything before the first turn, then everything since the last turn began. The replay
// hands them out a text box at a time, each ending in a paragraph break.
static s32 FindReplayLine(const u8 *line)
{
    u32 i, length = RandolockeBattleLog_ReplayLength();

    for (i = 0; i < length; i++)
    {
        if (StringCompare(RandolockeBattleLog_ReplayLine(i), line) == 0)
            return i;
    }
    return -1;
}

SINGLE_BATTLE_TEST("Randolocke: the battle log names the ability behind a weather message")
{
    s32 heading, ability, rain, lastTurn;

    GIVEN {
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_POLITOED) { Ability(ABILITY_DRIZZLE); }
    } WHEN {
        TURN { MOVE(player, MOVE_HARDEN); }
        TURN { MOVE(player, MOVE_GROWL); }
    } SCENE {
        ABILITY_POPUP(opponent, ABILITY_DRIZZLE);
        MESSAGE("It started to rain!");
    } THEN {
        heading = FindReplayLine(COMPOUND_STRING("The start of the battle:\p"));
        ability = FindReplayLine(COMPOUND_STRING("The opposing Politoed's\nDrizzle!\p"));
        rain = FindReplayLine(COMPOUND_STRING("It started to rain!\p"));
        lastTurn = FindReplayLine(COMPOUND_STRING("The last turn:\p"));
        EXPECT_EQ(heading, 0);
        // The pop-up is gone before the battle menu, and "It started to rain!" does not
        // say whose Drizzle it was -- the log's own line does, right before it.
        EXPECT_GT(ability, heading);
        EXPECT_EQ(rain, ability + 1);
        // The last turn is the last one only: turn 2's Growl, not turn 1's Harden.
        EXPECT_GT(lastTurn, rain);
        EXPECT_GT(FindReplayLine(COMPOUND_STRING("Wobbuffet used Growl!\p")), lastTurn);
        EXPECT_EQ(FindReplayLine(COMPOUND_STRING("Wobbuffet used Harden!\p")), -1);
    }
}

TEST("Randolocke: the battle log keeps the start and the last turn, and says when it is full")
{
    u8 filler[201];
    u32 i, length;

    RandolockeBattleLog_Reset();
    RandolockeBattleLog_AddMessage(COMPOUND_STRING("Before the first turn\p"));
    RandolockeBattleLog_BeginTurn();
    RandolockeBattleLog_AddMessage(COMPOUND_STRING("Turn one"));
    RandolockeBattleLog_BeginTurn();
    RandolockeBattleLog_AddMessage(COMPOUND_STRING("Turn two"));

    // A heading and a message for each part; turn one is gone. The first message's own
    // paragraph break is dropped, so it waits once, like every other line.
    EXPECT_EQ(RandolockeBattleLog_ReplayLength(), 4);
    EXPECT_EQ(StringCompare(RandolockeBattleLog_ReplayLine(0), COMPOUND_STRING("The start of the battle:\p")), 0);
    EXPECT_EQ(StringCompare(RandolockeBattleLog_ReplayLine(1), COMPOUND_STRING("Before the first turn\p")), 0);
    EXPECT_EQ(StringCompare(RandolockeBattleLog_ReplayLine(2), COMPOUND_STRING("The last turn:\p")), 0);
    EXPECT_EQ(StringCompare(RandolockeBattleLog_ReplayLine(3), COMPOUND_STRING("Turn two\p")), 0);

    // More than the log holds: what fits is kept, and the replay says the rest was lost.
    for (i = 0; i < ARRAY_COUNT(filler) - 1; i++)
        filler[i] = CHAR_A;
    filler[i] = EOS;
    for (i = 0; i < 20; i++)
        RandolockeBattleLog_AddMessage(filler);
    length = RandolockeBattleLog_ReplayLength();
    EXPECT_LT(length, 4 + 20);
    EXPECT_EQ(StringCompare(RandolockeBattleLog_ReplayLine(length - 1), COMPOUND_STRING("…and more than the log\ncould hold.\p")), 0);

    // A new turn starts the last-turn part afresh.
    RandolockeBattleLog_BeginTurn();
    EXPECT_EQ(RandolockeBattleLog_ReplayLength(), 2);
}

#endif // RANDOLOCKE_BATTLE_LOG
