#include "global.h"
#include "event_data.h"
#include "pokemon.h"
#include "wild_encounter.h"
#include "test/test.h"
#include "config/randolocke.h"
#include "constants/species.h"
#include "constants/vars.h"
#include "constants/wild_encounter.h"

static const struct WildPokemon sLand[NUM_LAND_MONS_ENCOUNTER_SLOTS] =
{
    {2, 3, SPECIES_ZIGZAGOON}, {2, 3, SPECIES_ZIGZAGOON}, {2, 3, SPECIES_WURMPLE},
    {2, 3, SPECIES_WURMPLE}, {2, 3, SPECIES_POOCHYENA}, {2, 3, SPECIES_POOCHYENA},
    {2, 3, SPECIES_ZIGZAGOON}, {2, 3, SPECIES_WURMPLE}, {2, 3, SPECIES_POOCHYENA},
    {2, 3, SPECIES_ZIGZAGOON}, {2, 3, SPECIES_WURMPLE}, {2, 3, SPECIES_POOCHYENA},
};
static const struct WildPokemonInfo sLandInfo = { 20, sLand };

// The Non-Shiny Repel: walking into grass generates Pokemon as ever, but only a shiny one
// is met. Driven through TryGenerateWildMon, the call every land, water and Rock Smash
// encounter makes, with the repel flag the way StandardWildEncounter passes it.
TEST("Randolocke: the Non-Shiny Repel lets only shiny Pokemon through")
{
    u32 met = 0, metShiny = 0, turnedAway = 0;

    VarSet(VAR_REPEL_STEP_COUNT, 0);
    FlagSet(RANDOLOCKE_FLAG_SHINY_REPEL);
    for (u32 i = 0; i < 1500; i++)
    {
        if (TryGenerateWildMon(&sLandInfo, WILD_AREA_LAND, WILD_CHECK_REPEL | WILD_CHECK_KEEN_EYE))
        {
            met++;
            if (IsMonShiny(&gParties[B_TRAINER_OPPONENT_A][0]))
                metShiny++;
        }
        else
        {
            turnedAway++;
        }
    }
    FlagClear(RANDOLOCKE_FLAG_SHINY_REPEL);
    Test_MgbaPrintf("Non-Shiny Repel on: %d met, all shiny: %d; %d turned away", met, metShiny, turnedAway);

    EXPECT_EQ(met, metShiny);   // nothing that is not shiny gets through
    EXPECT_GT(met, 0);          // and shinies still do: 1 in 64, so about 23 of 1500
    EXPECT_GT(turnedAway, 1000);
}

TEST("Randolocke: with the Non-Shiny Repel off, encounters are unchanged")
{
    u32 met = 0;

    VarSet(VAR_REPEL_STEP_COUNT, 0);
    FlagClear(RANDOLOCKE_FLAG_SHINY_REPEL);
    for (u32 i = 0; i < 200; i++)
    {
        if (TryGenerateWildMon(&sLandInfo, WILD_AREA_LAND, WILD_CHECK_REPEL | WILD_CHECK_KEEN_EYE))
            met++;
    }
    EXPECT_EQ(met, 200);
}
