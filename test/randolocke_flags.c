#include "global.h"
#include "config/battle.h"
#include "config/randolocke.h"
#include "config/randomizer.h"
#include "test/test.h"

// The flags and vars this hack borrows from the game's unused ones. Two settings on one flag
// would not crash anything: they would quietly switch each other on and off in a player's
// save. Numeric, so an alias of the same number is caught too. A new borrowed flag or var
// goes in these lists; tools/randolocke/check_borrowed_flags.py also checks that no script
// or source uses one directly.

// Each randomizer flag exists only while its FORCE_RANDOMIZE_* override is off, and the
// seed vars only when the seed is not the Trainer ID -- hence the #ifdefs.
static const u16 sBorrowedFlags[] =
{
#ifdef RANDOMIZER_FLAG_WILD_MON
    RANDOMIZER_FLAG_WILD_MON,
#endif
#ifdef RANDOMIZER_FLAG_FIELD_ITEMS
    RANDOMIZER_FLAG_FIELD_ITEMS,
#endif
#ifdef RANDOMIZER_FLAG_TRAINER_MON
    RANDOMIZER_FLAG_TRAINER_MON,
#endif
#ifdef RANDOMIZER_FLAG_FIXED_MON
    RANDOMIZER_FLAG_FIXED_MON,
#endif
#ifdef RANDOMIZER_FLAG_STARTER_AND_GIFT_MON
    RANDOMIZER_FLAG_STARTER_AND_GIFT_MON,
#endif
#ifdef RANDOMIZER_FLAG_EGG_MON
    RANDOMIZER_FLAG_EGG_MON,
#endif
#ifdef RANDOMIZER_FLAG_ABILITIES
    RANDOMIZER_FLAG_ABILITIES,
#endif
#ifdef RANDOMIZER_FLAG_LEARNSET
    RANDOMIZER_FLAG_LEARNSET,
#endif
#ifdef RANDOMIZER_FLAG_BERRY_TREES
    RANDOMIZER_FLAG_BERRY_TREES,
#endif
#ifdef RANDOMIZER_FLAG_TM_MOVES
    RANDOMIZER_FLAG_TM_MOVES,
#endif
#ifdef RANDOMIZER_FLAG_TUTOR_MOVES
    RANDOMIZER_FLAG_TUTOR_MOVES,
#endif
    RANDOLOCKE_FLAG_INFINITE_REPEL,
    RANDOLOCKE_FLAG_SHINY_REPEL,
    RANDOLOCKE_FLAG_NUZLOCKE_OFF,
    RANDOLOCKE_FLAG_HIDE_DEWFORD_OLD_ROD_FISHERMAN,
    RANDOLOCKE_FLAG_OLDALE_MONEY_GIVEN,
};

static const u16 sBorrowedVars[] =
{
    RANDOMIZER_VAR_SPECIES_MODE,
#ifdef RANDOMIZER_VAR_SEED_L
    RANDOMIZER_VAR_SEED_L,
    RANDOMIZER_VAR_SEED_H,
#endif
    B_VAR_NO_BAG_USE,
};

static bool32 AllDistinct(const u16 *values, u32 count)
{
    for (u32 i = 0; i < count; i++)
    {
        if (values[i] == 0)
            return FALSE;
        for (u32 j = i + 1; j < count; j++)
        {
            if (values[i] == values[j])
                return FALSE;
        }
    }
    return TRUE;
}

TEST("Randolocke: every flag and var the hack borrows is its own")
{
    EXPECT(AllDistinct(sBorrowedFlags, ARRAY_COUNT(sBorrowedFlags)));
    EXPECT(AllDistinct(sBorrowedVars, ARRAY_COUNT(sBorrowedVars)));

    // The rule flags lean on story flags on purpose; none of them is a borrowed one.
    for (u32 i = 0; i < ARRAY_COUNT(sBorrowedFlags); i++)
    {
        EXPECT_NE(sBorrowedFlags[i], RANDOLOCKE_FLAG_RULES_BEGIN);
        EXPECT_NE(sBorrowedFlags[i], RANDOLOCKE_MAP_SELLER_BADGE);
    }
}
