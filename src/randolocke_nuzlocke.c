#include "global.h"
#include "config/randolocke.h"
#include "constants/pokedex.h"
#include "battle.h"
#include "event_data.h"
#include "rtc.h"
#include "pokedex.h"
#include "pokemon.h"
#include "randolocke_nuzlocke.h"
#include "wild_encounter.h"
#include "overworld.h"
#include "constants/region_map_sections.h"
#include "item.h"
#include "pokemon_storage_system.h"
#include "constants/items.h"
#include "constants/flags.h"

// The four categories the randomizer's own legend-aware substitution treats as legendary,
// so both the catch clause and the legendary catch rate cover exactly the set that
// substitution can drop onto a route. Mirrors IsRandomizerLegendary; kept separate because
// that one is static to the randomizer. Outside the nuzlocke guard: the catch rate in
// GetBattleMonCatchRate wants it whether or not the nuzlocke rules are on.
bool32 RandolockeSpeciesIsLegendary(enum Species species)
{
    if (species == SPECIES_NONE || species >= NUM_SPECIES)
        return FALSE;

    return gSpeciesInfo[species].isRestrictedLegendary
        || gSpeciesInfo[species].isSubLegendary
        || gSpeciesInfo[species].isMythical
        || gSpeciesInfo[species].isUltraBeast;
}

#if RANDOLOCKE_QUICK_START == TRUE
// Sets the in-game clock to the real-time clock's current time, which is what a player
// would dial into the bedroom clock anyway: in an emulator the RTC is the host's own
// clock. Same call the wall clock makes on confirming, so days start counting from here.
void RandolockeAutoSetClock(void)
{
    struct SiiRtcInfo rtc;

    RtcGetInfo(&rtc);
    RtcInitLocalTimeOffset(ConvertBcdToBinary(rtc.hour), ConvertBcdToBinary(rtc.minute));
    FlagSet(FLAG_SET_WALL_CLOCK);
}
#endif

#if RANDOLOCKE_ELITE_FOUR_LEGENDARY_LIMIT == TRUE
// VAR_RESULT = TRUE if the party carries more legendaries than the League allows, and
// VAR_0x8004 = how many it carries, so the refusal can say. Called from the trigger in
// front of the Elite Four's door; see the config.
void RandolockeCheckEliteFourLegendaries(void)
{
    u32 i, count = 0;

    for (i = 0; i < PARTY_SIZE; i++)
    {
        struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][i];

        if (!GetMonData(mon, MON_DATA_SANITY_HAS_SPECIES, NULL) || GetMonData(mon, MON_DATA_IS_EGG, NULL))
            continue;
        if (RandolockeSpeciesIsLegendary(GetMonData(mon, MON_DATA_SPECIES, NULL)))
            count++;
    }
    gSpecialVar_0x8004 = count;
    gSpecialVar_Result = (count > RANDOLOCKE_ELITE_FOUR_MAX_LEGENDARIES);
}
#endif

#if RANDOLOCKE_NUZLOCKE_RULES == TRUE

STATIC_ASSERT(ARRAY_COUNT(((struct SaveBlock1 *)0)->caughtInArea) == RANDOLOCKE_AREA_BYTES,
              RandolockeAreaBytesMismatch);

bool32 RandolockeNuzlockeActive(void)
{
    // Nothing applies until Birch hands over the five Poke Balls. Before that the player
    // has one Pokemon, no balls, and a scripted loss to Route 103's rival to survive.
    return FlagGet(RANDOLOCKE_FLAG_RULES_BEGIN) && !FlagGet(RANDOLOCKE_FLAG_NUZLOCKE_OFF);
}

// The area the player is standing in, or RANDOLOCKE_NO_AREA where the rules do not apply.
//
// Keyed by region map section, not by map, so a cave counts once however many floors it
// has -- every floor of Magma Hideout being its own catch was too generous, and it is the
// case Randolocke's own notes single out. A map with no wild encounter table at all is
// still not an area: that is what keeps the legendary sites and scripted battles free.
u32 RandolockeCurrentArea(void)
{
    u32 mapSec;

    if (GetCurrentMapWildMonHeaderId() == HEADER_NONE)
        return RANDOLOCKE_NO_AREA;

    mapSec = gMapHeader.regionMapSectionId;
    if (mapSec >= RANDOLOCKE_MAX_AREAS)
        return RANDOLOCKE_NO_AREA;
    return mapSec;
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
    // Walks the evolution family looking for anything already registered as caught.
    //
    // The graph is data, not something to trust: regional forms and branching evolutions
    // can reach the same species twice, and a table that pointed back at an ancestor would
    // make a naive breadth-first walk run forever -- which, in the middle of a capture,
    // is a hung game. So: a seen-set, a bounded queue, and a hard iteration cap. Any of
    // the three alone would do; together the loop cannot fail to terminate.
    #define RANDOLOCKE_MAX_FAMILY   24
    enum Species queue[RANDOLOCKE_MAX_FAMILY];
    enum Species seen[RANDOLOCKE_MAX_FAMILY];
    u32 depth = 0, seenCount = 0, steps, i;

    if (species == SPECIES_NONE || species >= NUM_SPECIES)
        return FALSE;

    for (steps = 0; steps < RANDOLOCKE_MAX_FAMILY; steps++)
    {
        enum Species pre = GetSpeciesPreEvolution(species);

        if (pre == SPECIES_NONE || pre >= NUM_SPECIES)
            break;
        species = pre;
    }

    queue[depth++] = species;
    for (steps = 0; steps < RANDOLOCKE_MAX_FAMILY * 4 && depth != 0; steps++)
    {
        const struct Evolution *evos;
        enum Species cur = queue[--depth];
        bool32 alreadySeen = FALSE;

        for (i = 0; i < seenCount; i++)
        {
            if (seen[i] == cur)
                alreadySeen = TRUE;
        }
        if (alreadySeen)
            continue;
        if (seenCount < ARRAY_COUNT(seen))
            seen[seenCount++] = cur;

        if (GetSetPokedexFlag(SpeciesToNationalPokedexNum(cur), FLAG_GET_CAUGHT))
            return TRUE;

        evos = GetSpeciesEvolutions(cur);
        if (evos == NULL)
            continue;
        for (i = 0; evos[i].method != EVOLUTIONS_END; i++)
        {
            if (depth < ARRAY_COUNT(queue) && evos[i].targetSpecies < NUM_SPECIES)
                queue[depth++] = evos[i].targetSpecies;
        }
    }
    return FALSE;
}

// Why a ball may or may not be thrown at the Pokemon currently opposite the player.
// The wild Pokemon a given battler is, if it can be read yet. NULL while the battle is
// still setting itself up.
static struct Pokemon *RandolockeWildMon(enum BattlerId battler)
{
    struct Pokemon *mon;
    u32 partyIndex;

    if (battler >= MAX_BATTLERS_COUNT)
        return NULL;

    partyIndex = gBattlerPartyIndexes[battler];
    if (partyIndex >= PARTY_SIZE)
        return NULL;

    mon = &gParties[B_TRAINER_OPPONENT_A][partyIndex];
    if (!GetMonData(mon, MON_DATA_SANITY_HAS_SPECIES, NULL))
        return NULL;

    return mon;
}

enum RandolockeCatchRule RandolockeCatchRuleForBattler(enum BattlerId battler)
{
    struct Pokemon *mon;
    u32 area;

    // The catch tutorial has to be allowed through or the story cannot continue.
    if (!RandolockeNuzlockeActive()
     || (gBattleTypeFlags & (BATTLE_TYPE_TRAINER | BATTLE_TYPE_CATCH_TUTORIAL)))
        return RANDOLOCKE_CATCH_OK;

    mon = RandolockeWildMon(battler);
    if (mon == NULL)
        return RANDOLOCKE_CATCH_OK;   // let the throw happen rather than block progress

    // The shiny clause outranks everything, including a used-up area.
    if (IsMonShiny(mon))
        return RANDOLOCKE_CATCH_OK;

    // And the legendary clause on the same terms. Species randomization can put one in
    // any route's encounter table.
    #if RANDOLOCKE_LEGENDARY_CLAUSE == TRUE
    if (RandolockeSpeciesIsLegendary(GetMonData(mon, MON_DATA_SPECIES)))
        return RANDOLOCKE_CATCH_OK;
    #endif

    if (FamilyAlreadyCaught(GetMonData(mon, MON_DATA_SPECIES)))
        return RANDOLOCKE_CATCH_DUPE;

    area = RandolockeCurrentArea();
    if (RandolockeAreaUsed(area))
        return RANDOLOCKE_CATCH_AREA_USED;

    return RANDOLOCKE_CATCH_OK;
}

enum RandolockeCatchRule RandolockeCatchRuleForBattle(void)
{
    return RandolockeCatchRuleForBattler(GetCatchingBattler());
}

// Whether the health box for this battler should carry the first-encounter badge.
//
// Deliberately stricter than RandolockeCatchRuleForBattler, and about a specific battler
// rather than "whichever one a ball would hit". The health box is drawn during the battle
// intro, before gBattleMons is populated, and GetCatchingBattler decides via
// IsBattlerAlive -- so at that moment it failed the left-hand opponent and fell through
// to the right-hand one, which in a single battle is not a battler at all. The rule then
// ran against a garbage Pokemon whose shiny bit read as set often enough to return
// CATCH_OK, which is why the badge turned up on second encounters and vanished on the
// next redraw once the real data had arrived.
//
// Anything unreadable now means no badge. A badge that appears a frame late is invisible;
// one that appears wrongly is what was reported.
bool32 RandolockeEncounterIsFirst(enum BattlerId battler)
{
    if (!RandolockeNuzlockeActive())
        return FALSE;
    if (gBattleTypeFlags & (BATTLE_TYPE_TRAINER | BATTLE_TYPE_CATCH_TUTORIAL
                          | BATTLE_TYPE_SAFARI | BATTLE_TYPE_FRONTIER))
        return FALSE;
    if (battler >= MAX_BATTLERS_COUNT || IsOnPlayerSide(battler))
        return FALSE;
    if (RandolockeWildMon(battler) == NULL)
        return FALSE;

    return RandolockeCatchRuleForBattler(battler) == RANDOLOCKE_CATCH_OK;
}

// Whether meeting this Pokemon uses the area up. Anything a clause lets through does not --
// that is the whole point of them: a shiny, a legendary, or a Pokemon whose evolution family
// is already caught.
static bool32 EncounterUsesArea(struct Pokemon *mon)
{
    enum Species species = GetMonData(mon, MON_DATA_SPECIES);

    if (species == SPECIES_NONE || IsMonShiny(mon))
        return FALSE;
    #if RANDOLOCKE_LEGENDARY_CLAUSE == TRUE
    if (RandolockeSpeciesIsLegendary(species))
        return FALSE;
    #endif
    return !FamilyAlreadyCaught(species);
}

// Called once a wild Pokemon has actually been caught. Has to run before the dex caught
// flag for this catch is written, or FamilyAlreadyCaught would see the Pokemon that was
// just caught and no catch would ever mark an area; see the call site.
void RandolockeNoteCatch(struct Pokemon *mon)
{
    if (!RandolockeNuzlockeActive() || (gBattleTypeFlags & BATTLE_TYPE_CATCH_TUTORIAL))
        return;
    if (EncounterUsesArea(mon))
        MarkAreaUsed(RandolockeCurrentArea());
}

#if RANDOLOCKE_FIRST_ENCOUNTER_COUNTS == TRUE
// Called as every battle finishes. The first encounter in an area is the one chance there,
// however the battle ends -- run from, knocked out, gone by Teleport, Roar or its own
// fleeing, or a loss -- not only if it is caught. A catch has already been counted by
// RandolockeNoteCatch, and by now its family is registered as caught, so it is not counted
// twice. In a double battle, either wild Pokemon counting is enough.
void RandolockeNoteWildBattleEnd(void)
{
    u32 i, wildMons;

    // No BATTLE_TYPE_RECORDED here: playback only ever replays trainer, link and Frontier
    // battles, which are out already, and the test runner flags its wild battles as
    // recorded, so leaving it out is also what lets the rule be tested.
    if (!RandolockeNuzlockeActive()
     || (gBattleTypeFlags & (BATTLE_TYPE_TRAINER | BATTLE_TYPE_LINK | BATTLE_TYPE_FIRST_BATTLE
                           | BATTLE_TYPE_CATCH_TUTORIAL | BATTLE_TYPE_FRONTIER)))
        return;

    wildMons = (gBattleTypeFlags & BATTLE_TYPE_DOUBLE) ? 2 : 1;
    for (i = 0; i < wildMons; i++)
    {
        if (EncounterUsesArea(&gParties[B_TRAINER_OPPONENT_A][i]))
        {
            MarkAreaUsed(RandolockeCurrentArea());
            return;
        }
    }
}
#endif

// --- Player Pokemon IVs ------------------------------------------------------

#if RANDOLOCKE_PLAYER_IVS != RANDOLOCKE_IVS_VANILLA

// Called wherever a Pokemon becomes the player's -- caught, gifted, a starter, or hatched.
// Not from CreateBoxMon: trainer parties go through that too, and their IVs are their own
// business.
void RandolockeSetPlayerMonIVs(struct Pokemon *mon, bool32 isGiftOrStarter)
{
    #if RANDOLOCKE_PLAYER_IVS == RANDOLOCKE_IVS_PERFECT
        u32 i, iv = MAX_PER_STAT_IVS;

        (void)isGiftOrStarter;
        for (i = 0; i < NUM_STATS; i++)
            SetMonData(mon, MON_DATA_HP_IV + i, &iv);
        CalculateMonStats(mon);
    #else
        // Randolocke's own rule: starters and gifts are guaranteed a few perfect IVs,
        // everything else keeps whatever it rolled.
        if (!isGiftOrStarter)
            return;
        SetBoxMonPerfectIVs(&mon->box, RANDOLOCKE_GIFT_PERFECT_IVS);
        CalculateMonStats(mon);
    #endif
}

#endif

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

// Boxes party Pokemon and marks them dead. Held items are taken off first and go back to
// the bag, so a run does not lose its Leftovers along with the Pokemon holding them, and
// so the item is not locked in a box the player can never withdraw from. Eggs are spared:
// an egg was never in the fight.
//
// faintedOnly TRUE takes just the ones at 0 HP, which is what a faint costs. FALSE takes
// the whole party, which is what a wipe costs.
static void RandolockeBoxParty(bool32 faintedOnly)
{
    u32 i;
    bool32 boxedAny = FALSE;

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
        if (faintedOnly && GetMonData(mon, MON_DATA_HP, NULL) != 0)
            continue;

        held = GetMonData(mon, MON_DATA_HELD_ITEM, NULL);
        if (held != ITEM_NONE && AddBagItem(held, 1))
            SetMonData(mon, MON_DATA_HELD_ITEM, &none);

        SetMonData(mon, RANDOLOCKE_MON_DATA_FAINTED, &mark);
        if (CopyMonToPC(mon) == MON_GIVEN_TO_PC)
        {
            ZeroMonData(mon);
            boxedAny = TRUE;
        }
    }

    if (boxedAny)
    {
        CompactPartySlots();
        CalculatePlayerPartyCount();
    }
}

// Called on a wipe, and only on a wipe: the whole party, whatever its HP.
void RandolockeBoxWipedParty(void)
{
    if (!RandolockeNuzlockeActive())
        return;

    RandolockeBoxParty(FALSE);
}

// The nuzlocke death rule. A Pokemon that hits 0 HP is gone -- boxed at the end of the
// battle and locked there for the rest of the run, rather than walking it off at the
// nearest Pokemon Center.
//
// Battles the party is not really the player's, or cannot lose a Pokemon in, are skipped:
// Birch's bag on Route 101, the Wally catching tutorial, Safari, link and recorded
// battles, an in-game partner's team, and the Frontier, which runs on borrowed or rental
// Pokemon and heals between rounds anyway.
void RandolockeBoxFaintedMons(void)
{
    if (!RANDOLOCKE_FAINT_COSTS_MON || !RandolockeNuzlockeActive())
        return;

    if (gBattleTypeFlags & (BATTLE_TYPE_LINK
                          | BATTLE_TYPE_FIRST_BATTLE
                          | BATTLE_TYPE_CATCH_TUTORIAL
                          | BATTLE_TYPE_INGAME_PARTNER
                          | BATTLE_TYPE_RECORDED
                          | BATTLE_TYPE_SAFARI
                          | BATTLE_TYPE_FRONTIER))
        return;

    RandolockeBoxParty(TRUE);
}

// The same rule for a Pokemon that runs out of HP to field poison, where there is no
// battle and so no battle type to check.
void RandolockeBoxFaintedMonsFromField(void)
{
    if (!RANDOLOCKE_FAINT_COSTS_MON || !RandolockeNuzlockeActive())
        return;

    RandolockeBoxParty(TRUE);
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
