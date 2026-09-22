#include "global.h"
#include "config/randolocke.h"
#include "event_data.h"
#include "pokemon.h"
#include "random.h"
#include "randolocke_nuzlocke.h"
#include "test/test.h"

// The League's one-legendary rule. RandolockeCheckEliteFourLegendaries is what the trigger
// in front of the Elite Four's door calls, so testing it is testing the door.
static void SetParty(u32 a, u32 b)
{
    ZeroPlayerPartyMons();
    CreateMon(&gParties[B_TRAINER_PLAYER][0], a, 50, Random32(), OTID_STRUCT_PRESET(0x12345678));
    if (b != SPECIES_NONE)
        CreateMon(&gParties[B_TRAINER_PLAYER][1], b, 50, Random32(), OTID_STRUCT_PRESET(0x12345678));
    CalculatePlayerPartyCount();
}

TEST("Randolocke: the League lets one legendary through, not two")
{
    ASSUME(RANDOLOCKE_ELITE_FOUR_MAX_LEGENDARIES == 1);

    SetParty(SPECIES_WOBBUFFET, SPECIES_NONE);
    RandolockeCheckEliteFourLegendaries();
    EXPECT_EQ(gSpecialVar_Result, FALSE);

    // One is the limit, not over it.
    SetParty(SPECIES_WOBBUFFET, SPECIES_MEWTWO);
    RandolockeCheckEliteFourLegendaries();
    EXPECT_EQ(gSpecialVar_Result, FALSE);

    // A restricted legendary and a mythical -- and the count the refusal quotes.
    SetParty(SPECIES_MEWTWO, SPECIES_MEW);
    RandolockeCheckEliteFourLegendaries();
    EXPECT_EQ(gSpecialVar_Result, TRUE);
    EXPECT_EQ(gSpecialVar_0x8004, 2);

    // Ultra Beasts count, as they do for the clause and the catch rate.
    SetParty(SPECIES_MEWTWO, SPECIES_POIPOLE);
    RandolockeCheckEliteFourLegendaries();
    EXPECT_EQ(gSpecialVar_Result, TRUE);
}
