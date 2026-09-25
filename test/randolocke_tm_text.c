#include "global.h"
#include "event_data.h"
#include "item.h"
#include "move.h"
#include "randomizer.h"
#include "string_util.h"
#include "text.h"
#include "test/test.h"
#include "constants/items.h"

// Script natives in scrcmd.c; called from scripts, so no header declares them.
void RandolockeTutorMove(struct ScriptContext *ctx);
void RandolockeBufferGiftTM(struct ScriptContext *ctx);

static void SetTrainerIdSeed(u32 seed)
{
    gSaveBlock2Ptr->playerTrainerId[0] = seed;
    gSaveBlock2Ptr->playerTrainerId[1] = seed >> 8;
    gSaveBlock2Ptr->playerTrainerId[2] = seed >> 16;
    gSaveBlock2Ptr->playerTrainerId[3] = seed >> 24;
}

static bool32 IsHmMove(enum Move move)
{
    for (u32 i = 0; i < NUM_HIDDEN_MACHINES; i++)
    {
        if (GetItemTMHMMoveId(ITEM_HM01 + i) == move)
            return TRUE;
    }
    return FALSE;
}

// A TM that rolls Fly is a copy of HM02, which the story hands over anyway. Neither the TM
// table nor the tutors may deal an HM's move. Swept over 64 seeds: most seeds would not
// happen to roll one even without the rule -- the playtest seed's only Flying TM is Bounce
// -- so a handful of seeds would pass either way and prove nothing.
TEST("Randolocke: no TM or tutor teaches an HM's move")
{
    u32 hits = 0;

    FlagSet(RANDOMIZER_FLAG_TM_MOVES);
    FlagSet(RANDOMIZER_FLAG_TUTOR_MOVES);
    for (u32 n = 0; n < 64; n++)
    {
        SetTrainerIdSeed(0x8561D8DD + n * 0x9E3779B9);
        for (u32 i = 0; i < NUM_TECHNICAL_MACHINES; i++)
        {
            if (IsHmMove(GetItemTMHMMoveId(ITEM_TM01 + i)))
                hits++;
        }
        for (u32 i = 0; i < RANDOLOCKE_TUTOR_COUNT; i++)
        {
            if (IsHmMove(RandomizeTutorMove(gRandolockeTutorMoves[i])))
                hits++;
        }
    }
    Test_MgbaPrintf("HM moves dealt to TMs and tutors over 64 seeds: %d", hits);
    EXPECT_EQ(hits, 0);
}

// A laid-out message: no line wider than the message box, and never a third line in a box
// without a scroll or a new page first.
static bool32 FitsMessageBox(const u8 *str)
{
    u8 line[0x100];
    u32 len = 0, linesInBox = 1;

    for (u32 i = 0; ; i++)
    {
        u8 c = str[i];

        if (c == EOS || c == CHAR_NEWLINE || c == CHAR_PROMPT_SCROLL || c == CHAR_PROMPT_CLEAR)
        {
            line[len] = EOS;
            if (GetStringWidth(FONT_NORMAL, line, 0) > 208)
                return FALSE;
            len = 0;
            if (c == EOS)
                return TRUE;
            if (c == CHAR_NEWLINE && ++linesInBox > 2)
                return FALSE;
            if (c == CHAR_PROMPT_SCROLL)
                linesInBox = 2;
            if (c == CHAR_PROMPT_CLEAR)
                linesInBox = 1;
            continue;
        }
        line[len++] = c;
    }
}

// The tutors and the gym leaders now read out a move's own description in the field
// message box. Every move's has to fit it once re-laid-out.
TEST("Randolocke: every move description fits the message box")
{
    u32 bad = 0;

    FlagClear(RANDOMIZER_FLAG_TUTOR_MOVES);   // so the tutor native leaves the move alone
    for (u32 move = 1; move < MOVES_COUNT; move++)
    {
        VarSet(VAR_0x8005, move);
        RandolockeTutorMove(NULL);
        EXPECT_EQ(VarGet(VAR_0x8005), move);
        EXPECT_EQ(StringCompare(gStringVar1, GetMoveName(move)), 0);
        if (!FitsMessageBox(gStringVar3))
        {
            if (bad < 5)
                Test_MgbaPrintf("move %d's description does not fit the message box", move);
            bad++;
        }
    }
    EXPECT_EQ(bad, 0);
}

// A gym leader's explanation reads the TM actually given back out of VAR_0x8006, where
// Std_ObtainItem copies it, and names the move it teaches in this run, not the one it
// taught in vanilla. (VAR_0x8000 is overwritten by the obtain script's pocket switch; the
// in-game check that found that is in TESTING.md, since this test cannot run a script.)
TEST("Randolocke: a gym leader describes the TM actually handed over")
{
    u32 tm = 0;

    PARAMETRIZE { tm = ITEM_TM39; }
    PARAMETRIZE { tm = ITEM_TM08; }
    PARAMETRIZE { tm = ITEM_TM50; }

    FlagSet(RANDOMIZER_FLAG_TM_MOVES);
    SetTrainerIdSeed(0x8561D8DD);
    VarSet(VAR_0x8006, tm);
    RandolockeBufferGiftTM(NULL);

    EXPECT_EQ(StringCompare(gStringVar2, GetMoveName(GetItemTMHMMoveId(tm))), 0);
    EXPECT(FitsMessageBox(gStringVar3));
    EXPECT_NE(gStringVar3[0], EOS);
}
