#include "global.h"
#include "item.h"
#include "item_use.h"
#include "pokemon.h"
#include "test/test.h"
#include "constants/items.h"
#include "constants/species.h"

// Every item some species evolves by with EVO_ITEM has to be usable from the Bag on a
// Pokemon. With I_USE_EVO_HELD_ITEMS_FROM_BAG off, the held-item ones -- Metal Coat, Dragon
// Scale, the Up-Grade, the Electirizer and the rest -- were "can't use", so the item route
// the species data gives every held-item trade evolution led nowhere, and without a link
// cable Onix, Scyther, Seadra, Porygon and a dozen more could never evolve.
TEST("Randolocke: every evolution item can be used from the Bag")
{
    u32 checked = 0, unusable = 0;

    for (u32 species = 1; species < NUM_SPECIES; species++)
    {
        const struct Evolution *evos;

        if (!IsSpeciesEnabled(species))
            continue;
        evos = GetSpeciesEvolutions(species);
        for (u32 i = 0; evos != NULL && evos[i].method != EVOLUTIONS_END; i++)
        {
            enum Item item = evos[i].param;

            if (evos[i].method != EVO_ITEM)
                continue;
            checked++;
            if (gItemsInfo[item].fieldUseFunc != ItemUseOutOfBattle_EvolutionStone
             || gItemsInfo[item].type != ITEM_USE_PARTY_MENU)
            {
                if (unusable < 5)
                    Test_MgbaPrintf("item %d evolves species %d but cannot be used from the Bag", item, species);
                unusable++;
            }
        }
    }
    EXPECT_GT(checked, 0);
    EXPECT_EQ(unusable, 0);
}

// And using one does what it says: the evolution check for "this item was used on it".
TEST("Randolocke: a held-item trade evolution happens from the Bag")
{
    u32 species = 0, item = 0, target = 0;
    struct Pokemon mon;

    PARAMETRIZE { species = SPECIES_ONIX;       item = ITEM_METAL_COAT;     target = SPECIES_STEELIX; }
    PARAMETRIZE { species = SPECIES_SEADRA;     item = ITEM_DRAGON_SCALE;   target = SPECIES_KINGDRA; }
    PARAMETRIZE { species = SPECIES_PORYGON;    item = ITEM_UPGRADE;        target = SPECIES_PORYGON2; }
    PARAMETRIZE { species = SPECIES_CLAMPERL;   item = ITEM_DEEP_SEA_TOOTH; target = SPECIES_HUNTAIL; }
    PARAMETRIZE { species = SPECIES_POLIWHIRL;  item = ITEM_KINGS_ROCK;     target = SPECIES_POLITOED; }
    PARAMETRIZE { species = SPECIES_KADABRA;    item = ITEM_LINKING_CORD;   target = SPECIES_ALAKAZAM; }

    CreateMon(&mon, species, 30, 0, OTID_STRUCT_PLAYER_ID);
    CalculateMonStats(&mon);
    EXPECT_EQ(GetEvolutionTargetSpecies(&mon, EVO_MODE_ITEM_USE, item, NULL, NULL, CHECK_EVO), target);
}
