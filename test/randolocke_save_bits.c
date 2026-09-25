#include "global.h"
#include "config/randolocke.h"
#include "pokemon.h"
#include "test/test.h"

// Two bits this hack keeps in a Pokemon's saved data: the randomizer's cantRandomizeAbility,
// packed into the ribbon word beside abilityNum, and the nuzlocke's dead mark, which reuses
// the never-distributed Marine Ribbon. Getting a packed bit wrong corrupts saves quietly,
// hours into a run, so each is written and read back through the encrypted box data -- the
// form a Pokemon takes in the PC -- with the fields around it checked to be untouched.

struct Neighbours
{
    u32 abilityNum;
    u32 worldRibbon;
    u32 earthRibbon;
    u32 landRibbon;
    u32 fateful;
    u32 spDefIV;
};

static void SetNeighbours(struct BoxPokemon *boxMon)
{
    u32 abilityNum = 2, one = 1, iv = 17;

    SetBoxMonData(boxMon, MON_DATA_ABILITY_NUM, &abilityNum);
    SetBoxMonData(boxMon, MON_DATA_WORLD_RIBBON, &one);
    SetBoxMonData(boxMon, MON_DATA_EARTH_RIBBON, &one);
    SetBoxMonData(boxMon, MON_DATA_LAND_RIBBON, &one);
    SetBoxMonData(boxMon, MON_DATA_MODERN_FATEFUL_ENCOUNTER, &one);
    SetBoxMonData(boxMon, MON_DATA_SPDEF_IV, &iv);
}

static struct Neighbours ReadNeighbours(struct BoxPokemon *boxMon)
{
    struct Neighbours n =
    {
        .abilityNum = GetBoxMonData(boxMon, MON_DATA_ABILITY_NUM, NULL),
        .worldRibbon = GetBoxMonData(boxMon, MON_DATA_WORLD_RIBBON, NULL),
        .earthRibbon = GetBoxMonData(boxMon, MON_DATA_EARTH_RIBBON, NULL),
        .landRibbon = GetBoxMonData(boxMon, MON_DATA_LAND_RIBBON, NULL),
        .fateful = GetBoxMonData(boxMon, MON_DATA_MODERN_FATEFUL_ENCOUNTER, NULL),
        .spDefIV = GetBoxMonData(boxMon, MON_DATA_SPDEF_IV, NULL),
    };
    return n;
}

static void ExpectNeighbours(struct BoxPokemon *boxMon)
{
    struct Neighbours n = ReadNeighbours(boxMon);

    EXPECT_EQ(n.abilityNum, 2);
    EXPECT_EQ(n.worldRibbon, 1);
    EXPECT_EQ(n.earthRibbon, 1);
    EXPECT_EQ(n.landRibbon, 1);
    EXPECT_EQ(n.fateful, 1);
    EXPECT_EQ(n.spDefIV, 17);
    // The checksum still matches, so the game does not take it for a Bad Egg.
    EXPECT(!GetBoxMonData(boxMon, MON_DATA_SANITY_IS_BAD_EGG, NULL));
}

// Writes `field` on, reads it back from a copy of the box data -- what the PC stores -- then
// off again, each time checking the fields packed around it.
static void RoundTrip(u32 field)
{
    struct Pokemon mon;
    struct BoxPokemon stored;
    u32 on = 1, off = 0;

    CreateMon(&mon, SPECIES_WOBBUFFET, 30, 0x12345678, OTID_STRUCT_PLAYER_ID);
    SetNeighbours(&mon.box);

    SetBoxMonData(&mon.box, field, &on);
    stored = mon.box;
    EXPECT_EQ(GetBoxMonData(&stored, field, NULL), 1);
    ExpectNeighbours(&stored);

    SetBoxMonData(&mon.box, field, &off);
    stored = mon.box;
    EXPECT_EQ(GetBoxMonData(&stored, field, NULL), 0);
    ExpectNeighbours(&stored);
}

TEST("Randolocke: the randomizer's ability bit survives the PC without disturbing its neighbours")
{
    RoundTrip(MON_DATA_CANT_RANDOMIZE_ABILITY);
}

#if RANDOLOCKE_NUZLOCKE_RULES == TRUE && RANDOLOCKE_WIPE_COSTS_PARTY == TRUE
TEST("Randolocke: the dead mark survives the PC without disturbing its neighbours")
{
    RoundTrip(RANDOLOCKE_MON_DATA_FAINTED);
}
#endif
