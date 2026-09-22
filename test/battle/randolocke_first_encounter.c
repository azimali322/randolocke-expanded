#include "global.h"
#include "config/randolocke.h"
#include "event_data.h"
#include "pokedex.h"
#include "randolocke_nuzlocke.h"
#include "constants/region_map_sections.h"
#include "test/battle.h"

#if RANDOLOCKE_NUZLOCKE_RULES == TRUE && RANDOLOCKE_FIRST_ENCOUNTER_COUNTS == TRUE

// RANDOLOCKE_FIRST_ENCOUNTER_COUNTS: the first Pokemon met in an area is the one chance
// there, however the battle ends -- unless a clause spares it: a shiny, or a Pokemon whose
// evolution family is already caught.

// Route 101 with the rules running, nothing used and nothing caught yet.
static void SetUpRoute101(bool32 rulesStarted)
{
    if (rulesStarted)
        FlagSet(RANDOLOCKE_FLAG_RULES_BEGIN);
    else
        FlagClear(RANDOLOCKE_FLAG_RULES_BEGIN);
    FlagClear(RANDOLOCKE_FLAG_NUZLOCKE_OFF);
    gSaveBlock1Ptr->location.mapGroup = MAP_GROUP(MAP_ROUTE101);
    gSaveBlock1Ptr->location.mapNum = MAP_NUM(MAP_ROUTE101);
    gMapHeader.regionMapSectionId = MAPSEC_ROUTE_101;
    memset(gSaveBlock1Ptr->caughtInArea, 0, sizeof(gSaveBlock1Ptr->caughtInArea));
    ResetPokedex();
}

WILD_BATTLE_TEST("Randolocke: a first encounter that teleports away uses up the area")
{
    GIVEN {
        SetUpRoute101(TRUE);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_ABRA);
    } WHEN {
        TURN { MOVE(opponent, MOVE_TELEPORT); }
    } THEN {
        EXPECT(RandolockeAreaUsed(MAPSEC_ROUTE_101));
    }
}

WILD_BATTLE_TEST("Randolocke: a first encounter the player escapes from uses up the area")
{
    GIVEN {
        SetUpRoute101(TRUE);
        PLAYER(SPECIES_ABRA);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_TELEPORT); }
    } THEN {
        EXPECT(RandolockeAreaUsed(MAPSEC_ROUTE_101));
    }
}

WILD_BATTLE_TEST("Randolocke: a first encounter that is knocked out uses up the area")
{
    GIVEN {
        SetUpRoute101(TRUE);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_ZIGZAGOON) { HP(1); }
    } WHEN {
        TURN { MOVE(player, MOVE_TACKLE); }
    } THEN {
        EXPECT(RandolockeAreaUsed(MAPSEC_ROUTE_101));
    }
}

WILD_BATTLE_TEST("Randolocke: a first encounter that is caught uses up the area")
{
    GIVEN {
        SetUpRoute101(TRUE);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_ZIGZAGOON);
    } WHEN {
        TURN { USE_ITEM(player, ITEM_MASTER_BALL); }
    } THEN {
        EXPECT(RandolockeAreaUsed(MAPSEC_ROUTE_101));
    }
}

WILD_BATTLE_TEST("Randolocke: a dupe that teleports away leaves the area open")
{
    GIVEN {
        SetUpRoute101(TRUE);
        // Kadabra already caught: Abra is the same family.
        GetSetPokedexFlag(SpeciesToNationalPokedexNum(SPECIES_KADABRA), FLAG_SET_CAUGHT);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_ABRA);
    } WHEN {
        TURN { MOVE(opponent, MOVE_TELEPORT); }
    } THEN {
        EXPECT(!RandolockeAreaUsed(MAPSEC_ROUTE_101));
    }
}

WILD_BATTLE_TEST("Randolocke: escaping from a dupe leaves the area open")
{
    GIVEN {
        SetUpRoute101(TRUE);
        // Linoone caught somewhere else: Zigzagoon is the same family.
        GetSetPokedexFlag(SpeciesToNationalPokedexNum(SPECIES_LINOONE), FLAG_SET_CAUGHT);
        PLAYER(SPECIES_ABRA);
        OPPONENT(SPECIES_ZIGZAGOON);
    } WHEN {
        TURN { MOVE(player, MOVE_TELEPORT); }
    } THEN {
        EXPECT(!RandolockeAreaUsed(MAPSEC_ROUTE_101));
    }
}

WILD_BATTLE_TEST("Randolocke: a shiny that teleports away leaves the area open")
{
    GIVEN {
        SetUpRoute101(TRUE);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_ABRA) { Shiny(TRUE); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_TELEPORT); }
    } THEN {
        EXPECT(!RandolockeAreaUsed(MAPSEC_ROUTE_101));
    }
}

WILD_BATTLE_TEST("Randolocke: before the five Poke Balls, nothing uses up an area")
{
    GIVEN {
        SetUpRoute101(FALSE);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_ABRA);
    } WHEN {
        TURN { MOVE(opponent, MOVE_TELEPORT); }
    } THEN {
        EXPECT(!RandolockeAreaUsed(MAPSEC_ROUTE_101));
    }
}

#endif
