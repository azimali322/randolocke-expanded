#include "global.h"
#include "field_effect.h"
#include "pokemon.h"
#include "sprite.h"
#include "task.h"
#include "constants/field_effects.h"
#include "test/test.h"

// The nuzlocke wipe rule boxes the whole party, so the player arrives at the healing
// machine with none. The machine places one ball per Pokemon and only ended the state
// after placing one and counting down, so zero wrapped round: it placed a ball every 25
// frames until the sprite table ran out, and the Pokemon Center crashed on
// "OUT OF SPRITE SLOTS".
static u32 ActiveSprites(void)
{
    u32 i, count = 0;

    for (i = 0; i < MAX_SPRITES; i++)
    {
        if (gSprites[i].inUse)
            count++;
    }
    return count;
}

TEST("Randolocke: the Pokemon Center heal ends with an empty party")
{
    u32 i, before, worst;

    ResetTasks();
    ResetSpriteData();
    ZeroPlayerPartyMons();
    CalculatePlayerPartyCount();
    EXPECT_EQ(gPartiesCount[B_TRAINER_PLAYER], 0);

    // Only the overworld's own setup clears this list, and a test never runs it: every
    // slot still reads 0 rather than 0xFF, so an Add finds no room and the effect would
    // never register as active -- the loop below would run zero frames and prove nothing.
    FieldEffectActiveListClear();
    before = ActiveSprites();
    worst = before;
    FieldEffectStart(FLDEFF_POKECENTER_HEAL);
    for (i = 0; i < 60 * 30 && FieldEffectActiveListContains(FLDEFF_POKECENTER_HEAL); i++)
    {
        RunTasks();
        AnimateSprites();
        if (ActiveSprites() > worst)
            worst = ActiveSprites();
    }

    // It ends, and it never fills the sprite table doing it.
    EXPECT(!FieldEffectActiveListContains(FLDEFF_POKECENTER_HEAL));
    EXPECT_LT(worst, before + PARTY_SIZE + 4);

    // The machine's own task outlives the effect on the field, where the nurse's script
    // takes it from here; nothing outside a Pokemon Center tidies it, so the test does.
    ResetTasks();
    ResetSpriteData();
}
