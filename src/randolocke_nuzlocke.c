#include "global.h"
#include "config/randolocke.h"
#include "constants/pokedex.h"
#include "battle.h"
#include "event_data.h"
#include "pokedex.h"
#include "pokemon.h"
#include "randolocke_nuzlocke.h"
#include "wild_encounter.h"
#include "item.h"
#include "pokemon_storage_system.h"
#include "constants/items.h"
#include "constants/flags.h"

#if RANDOLOCKE_NUZLOCKE_RULES == TRUE

STATIC_ASSERT(ARRAY_COUNT(((struct SaveBlock1 *)0)->caughtInArea) == RANDOLOCKE_AREA_BYTES,
              RandolockeAreaBytesMismatch);

bool32 RandolockeNuzlockeActive(void)
{
    // Nothing applies until Birch hands over the five Poke Balls. Before that the player
    // has one Pokemon, no balls, and a scripted loss to Route 103's rival to survive.
    return FlagGet(RANDOLOCKE_FLAG_RULES_BEGIN) && !FlagGet(RANDOLOCKE_FLAG_NUZLOCKE_OFF);
}

// The area the player is standing in, or RANDOLOCKE_NO_AREA where the rules do not apply:
// somewhere with no wild encounter table at all, or an area past the end of the bitfield.
u32 RandolockeCurrentArea(void)
{
    u16 headerId = GetCurrentMapWildMonHeaderId();

    if (headerId == HEADER_NONE || headerId >= RANDOLOCKE_MAX_AREAS)
        return RANDOLOCKE_NO_AREA;
    return headerId;
}

bool32 RandolockeAreaUsed(u32 area)
{
    if (area == RANDOLOCKE_NO_AREA)
        return FALSE;
    return (gSaveBlock1Ptr->caughtInArea[area / 8] >> (area % 8)) & 1;
}

static void MarkAreaUsed(u32 area)
{
    if (area != RANDOLOCKE_NO_AREA)
        gSaveBlock1Ptr->caughtInArea[area / 8] |= 1 << (area % 8);
}

// True if anything in this species' evolution family is already registered as caught.
// Walks to the family root first, so catching an Ivysaur blocks a later Bulbasaur.
static bool32 FamilyAlreadyCaught(enum Species species)
{
    // Deep enough for any real family; the guard stops a cyclic evolution table hanging.
    #define RANDOLOCKE_MAX_FAMILY   24
    enum Species stack[RANDOLOCKE_MAX_FAMILY];
    u32 depth = 0;
    u32 guard;

    for (guard = 0; guard < RANDOLOCKE_MAX_FAMILY; guard++)
    {
        enum Species pre = GetSpeciesPreEvolution(species);

        if (pre == SPECIES_NONE)
            break;
        species = pre;
    }

    stack[depth++] = species;
    while (depth != 0)
    {
        const struct Evolution *evos;
        enum Species cur = stack[--depth];
        u32 i;

        if (GetSetPokedexFlag(SpeciesToNationalPokedexNum(cur), FLAG_GET_CAUGHT))
            return TRUE;

        evos = GetSpeciesEvolutions(cur);
        if (evos == NULL)
            continue;
        for (i = 0; evos[i].method != EVOLUTIONS_END; i++)
        {
            if (depth < ARRAY_COUNT(stack))
                stack[depth++] = evos[i].targetSpecies;
        }
    }
    return FALSE;
}

// Why a ball may or may not be thrown at the Pokemon currently opposite the player.
enum RandolockeCatchRule RandolockeCatchRuleForBattle(void)
{
    struct Pokemon *mon;
    u32 area;

    // The catch tutorial has to be allowed through or the story cannot continue.
    if (!RandolockeNuzlockeActive()
     || (gBattleTypeFlags & (BATTLE_TYPE_TRAINER | BATTLE_TYPE_CATCH_TUTORIAL)))
        return RANDOLOCKE_CATCH_OK;

    mon = &gParties[B_TRAINER_OPPONENT_A][gBattlerPartyIndexes[GetCatchingBattler()]];

    // The shiny clause outranks everything, including a used-up area.
    if (IsMonShiny(mon))
        return RANDOLOCKE_CATCH_OK;

    if (FamilyAlreadyCaught(GetMonData(mon, MON_DATA_SPECIES)))
        return RANDOLOCKE_CATCH_DUPE;

    area = RandolockeCurrentArea();
    if (RandolockeAreaUsed(area))
        return RANDOLOCKE_CATCH_AREA_USED;

    return RANDOLOCKE_CATCH_OK;
}

// Called once a wild Pokemon has actually been caught. A shiny or a duplicate does not
// use the area up -- that is the whole point of the two clauses.
void RandolockeNoteCatch(struct Pokemon *mon)
{
    if (!RandolockeNuzlockeActive() || (gBattleTypeFlags & BATTLE_TYPE_CATCH_TUTORIAL))
        return;
    if (IsMonShiny(mon))
        return;
    if (FamilyAlreadyCaught(GetMonData(mon, MON_DATA_SPECIES)))
        return;

    MarkAreaUsed(RandolockeCurrentArea());
}

// --- Permadeath --------------------------------------------------------------

#if RANDOLOCKE_WIPE_COSTS_PARTY == TRUE

// Dead Pokemon are locked in their box until the run is finished. Becoming Champion ends
// the run, so from that point they are yours again.
bool32 RandolockeDeadMonsAreLocked(void)
{
    return RandolockeNuzlockeActive() && !FlagGet(FLAG_IS_CHAMPION);
}

bool32 RandolockeMonIsDead(struct BoxPokemon *boxMon)
{
    return GetBoxMonData(boxMon, RANDOLOCKE_MON_DATA_FAINTED, NULL) != 0;
}

// Called on a wipe, and only on a wipe. The whole party is boxed and marked; eggs are
// spared, since they were never in the fight. Held items come back to the bag first.
void RandolockeBoxWipedParty(void)
{
    u32 i;

    if (!RandolockeNuzlockeActive())
        return;

    for (i = 0; i < PARTY_SIZE; i++)
    {
        struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][i];
        enum Item held;
        u8 mark = TRUE;
        enum Item none = ITEM_NONE;

        if (!GetMonData(mon, MON_DATA_SANITY_HAS_SPECIES, NULL))
            continue;
        if (GetMonData(mon, MON_DATA_IS_EGG, NULL))
            continue;

        held = GetMonData(mon, MON_DATA_HELD_ITEM, NULL);
        if (held != ITEM_NONE && AddBagItem(held, 1))
            SetMonData(mon, MON_DATA_HELD_ITEM, &none);

        SetMonData(mon, RANDOLOCKE_MON_DATA_FAINTED, &mark);
        if (CopyMonToPC(mon) == MON_GIVEN_TO_PC)
            ZeroMonData(mon);
    }
    CompactPartySlots();
    CalculatePlayerPartyCount();
}

bool32 RandolockeAnyLivingMonInBoxes(void)
{
    u32 box, slot;

    for (box = 0; box < TOTAL_BOXES_COUNT; box++)
    {
        for (slot = 0; slot < IN_BOX_COUNT; slot++)
        {
            struct BoxPokemon *boxMon = GetBoxedMonPtr(box, slot);

            if (GetBoxMonData(boxMon, MON_DATA_SANITY_HAS_SPECIES, NULL)
             && !GetBoxMonData(boxMon, MON_DATA_IS_EGG, NULL)
             && !RandolockeMonIsDead(boxMon))
                return TRUE;
        }
    }
    return FALSE;
}

#endif // RANDOLOCKE_WIPE_COSTS_PARTY

#endif // RANDOLOCKE_NUZLOCKE_RULES
