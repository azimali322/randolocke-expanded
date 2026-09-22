#include "global.h"
#include "config/randolocke.h"
#include "test/battle.h"

// RANDOLOCKE_LEGENDARY_CATCH_RATE puts every legendary, mythical and Ultra Beast on one
// catch rate in place of its own, so a run can actually keep what it finds. At full health
// with a Poke Ball the odds are simply the effective catch rate divided by three, which is
// what makes these numbers readable: 45 / 3 = 15 for anything legendary.
ASSUMPTIONS
{
    ASSUME(gSpeciesInfo[SPECIES_MEWTWO].catchRate == 3);   // where 105 of the 136 sit
    ASSUME(gSpeciesInfo[SPECIES_MEW].catchRate == 45);     // one of the 23 above the floor
    ASSUME(gSpeciesInfo[SPECIES_BELDUM].catchRate == 3);   // not legendary, also at the floor
    ASSUME(gSpeciesInfo[SPECIES_CHANSEY].catchRate == 30); // not legendary, 30 x 150% = 45
    ASSUME(RANDOLOCKE_LEGENDARY_CATCH_RATE == 45);
    ASSUME(RANDOLOCKE_CATCH_RATE_PERCENT == 150);
}

WILD_BATTLE_TEST("Randolocke: every legendary is caught at one rate")
{
    u32 recordedOdds, expectedOdds;
    u32 species;

    // Both legendaries land on the same rate: the one at the floor is far easier than it
    // was, the one above it slightly harder.
    PARAMETRIZE { species = SPECIES_MEWTWO; expectedOdds = 15; }
    PARAMETRIZE { species = SPECIES_MEW;    expectedOdds = 15; }
    // A Pokemon that is not legendary keeps its own rate, floor and all.
    PARAMETRIZE { species = SPECIES_BELDUM; expectedOdds = 1; }
    // And 45 is worth what a rate of 45 is worth: the same as a plain 30 after the 150%.
    PARAMETRIZE { species = SPECIES_CHANSEY; expectedOdds = 15; }

    GIVEN {
        WITH_CONFIG(B_MISSING_BADGE_CATCH_MALUS, GEN_7); // no badges in a test
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(species) { Level(50); }                 // above the low-level bonus
    } WHEN {
        TURN { USE_ITEM(player, ITEM_POKE_BALL); }
    } SCENE {
        CATCHING_CHANCE(&recordedOdds);
    } THEN {
        EXPECT_EQ(expectedOdds, recordedOdds);
    }
}
