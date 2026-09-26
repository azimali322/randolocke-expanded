#include "global.h"
#include "battle.h"
#include "config/randolocke.h"
#include "event_data.h"
#include "item.h"
#include "pokemon.h"
#include "pokemon_storage_system.h"
#include "randolocke_nuzlocke.h"
#include "test/test.h"
#include "constants/items.h"

// The nuzlocke death rule (src/randolocke_nuzlocke.c): a Pokemon that faints is boxed at the
// end of the battle, marked dead and locked in the PC until the player is Champion; a wipe
// costs the whole party. A mistake here does not crash anything -- it takes a player's team,
// or quietly stops taking it -- so every part of the rule is pinned down here.

#if RANDOLOCKE_NUZLOCKE_RULES == TRUE && RANDOLOCKE_WIPE_COSTS_PARTY == TRUE

// A Pokemon at full HP in party slot `slot` -- CreateMon leaves the current HP at 0.
static void GiveMon(u32 slot, u16 species)
{
    struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][slot];
    u32 hp;

    CreateMon(mon, species, 20, 0, OTID_STRUCT_PLAYER_ID);
    CalculateMonStats(mon);
    hp = GetMonData(mon, MON_DATA_MAX_HP, NULL);
    SetMonData(mon, MON_DATA_HP, &hp);
    CalculatePlayerPartyCount();
}

// An empty PC and bag, the rules in force, and a party of `count` Pokemon, all alive.
static void SetUpRun(u32 count)
{
    static const u16 sSpecies[] = { SPECIES_WOBBUFFET, SPECIES_WYNAUT, SPECIES_ZIGZAGOON };

    ResetPokemonStorageSystem();
    ClearBag();
    ZeroPlayerPartyMons();
    FlagSet(RANDOLOCKE_FLAG_RULES_BEGIN);
    gBattleTypeFlags = BATTLE_TYPE_TRAINER;
    for (u32 i = 0; i < count && i < ARRAY_COUNT(sSpecies); i++)
        GiveMon(i, sSpecies[i]);
    EXPECT_GT(GetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_HP, NULL), 0);
}

static void Faint(u32 slot)
{
    u32 hp = 0;

    SetMonData(&gParties[B_TRAINER_PLAYER][slot], MON_DATA_HP, &hp);
}

static void Hold(u32 slot, u16 item)
{
    SetMonData(&gParties[B_TRAINER_PLAYER][slot], MON_DATA_HELD_ITEM, &item);
}

// Makes party slot `slot` an egg that has somehow reached 0 HP, to show eggs are spared on
// what they are, not on their HP.
static void MakeEgg(u32 slot)
{
    u32 isEgg = TRUE;

    SetMonData(&gParties[B_TRAINER_PLAYER][slot], MON_DATA_IS_EGG, &isEgg);
    Faint(slot);
}

// The boxed Pokemon of this species, or NULL.
static struct BoxPokemon *InPC(u16 species)
{
    for (u32 box = 0; box < TOTAL_BOXES_COUNT; box++)
    {
        for (u32 slot = 0; slot < IN_BOX_COUNT; slot++)
        {
            struct BoxPokemon *boxMon = GetBoxedMonPtr(box, slot);

            if (GetBoxMonData(boxMon, MON_DATA_SPECIES, NULL) == species)
                return boxMon;
        }
    }
    return NULL;
}

static bool32 InParty(u16 species)
{
    for (u32 i = 0; i < PARTY_SIZE; i++)
    {
        if (GetMonData(&gParties[B_TRAINER_PLAYER][i], MON_DATA_SPECIES, NULL) == species)
            return TRUE;
    }
    return FALSE;
}

TEST("Randolocke: a Pokemon that faints is boxed dead, and its item goes to the Bag")
{
    struct BoxPokemon *dead;

    ASSUME(RANDOLOCKE_FAINT_COSTS_MON == TRUE);
    SetUpRun(3);
    Faint(0);
    Hold(0, ITEM_LEFTOVERS);
    MakeEgg(2);

    RandolockeBoxFaintedMons();

    // Out of the party, which closes up around the gap; the living and the egg stay.
    EXPECT_EQ(gPartiesCount[B_TRAINER_PLAYER], 2);
    EXPECT(!InParty(SPECIES_WOBBUFFET));
    EXPECT_EQ(GetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_SPECIES, NULL), SPECIES_WYNAUT);
    EXPECT(GetMonData(&gParties[B_TRAINER_PLAYER][1], MON_DATA_IS_EGG, NULL));

    // In the PC, marked dead through the box's encryption, and holding nothing.
    dead = InPC(SPECIES_WOBBUFFET);
    EXPECT(dead != NULL);
    EXPECT(RandolockeMonIsDead(dead));
    EXPECT_EQ(GetBoxMonData(dead, MON_DATA_HELD_ITEM, NULL), ITEM_NONE);
    EXPECT(CheckBagHasItem(ITEM_LEFTOVERS, 1));

    // The survivor is not marked.
    EXPECT(!GetMonData(&gParties[B_TRAINER_PLAYER][0], RANDOLOCKE_MON_DATA_FAINTED, NULL));
    EXPECT(!RandolockeAnyLivingMonInBoxes());
}

TEST("Randolocke: a wipe boxes the whole party dead, except an egg")
{
    SetUpRun(2);
    MakeEgg(1);
    // A third Pokemon still standing: a wipe takes it too, whatever its HP.
    GiveMon(2, SPECIES_ZIGZAGOON);
    Faint(0);

    RandolockeBoxWipedParty();

    EXPECT_EQ(gPartiesCount[B_TRAINER_PLAYER], 1);
    EXPECT(GetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_IS_EGG, NULL));
    EXPECT(InPC(SPECIES_WOBBUFFET) != NULL && RandolockeMonIsDead(InPC(SPECIES_WOBBUFFET)));
    EXPECT(InPC(SPECIES_ZIGZAGOON) != NULL && RandolockeMonIsDead(InPC(SPECIES_ZIGZAGOON)));
    EXPECT(!RandolockeAnyLivingMonInBoxes());
}

TEST("Randolocke: a faint costs nothing before the five Poke Balls, or with the rules off")
{
    bool32 rulesOff = FALSE;

    PARAMETRIZE { rulesOff = FALSE; }   // before Birch's Poke Balls
    PARAMETRIZE { rulesOff = TRUE; }    // the player switched the rules off

    SetUpRun(2);
    Faint(0);
    if (rulesOff)
        FlagSet(RANDOLOCKE_FLAG_NUZLOCKE_OFF);
    else
        FlagClear(RANDOLOCKE_FLAG_RULES_BEGIN);

    RandolockeBoxFaintedMons();
    RandolockeBoxWipedParty();

    EXPECT_EQ(gPartiesCount[B_TRAINER_PLAYER], 2);
    EXPECT(InParty(SPECIES_WOBBUFFET));
    EXPECT(InPC(SPECIES_WOBBUFFET) == NULL);
    EXPECT(!RandolockeDeadMonsAreLocked());
}

TEST("Randolocke: a faint costs nothing in a battle that was never the player's to lose")
{
    u32 battleType = 0;

    PARAMETRIZE { battleType = BATTLE_TYPE_FIRST_BATTLE; }     // Birch's bag on Route 101
    PARAMETRIZE { battleType = BATTLE_TYPE_CATCH_TUTORIAL; }   // Wally's catch
    PARAMETRIZE { battleType = BATTLE_TYPE_SAFARI; }
    PARAMETRIZE { battleType = BATTLE_TYPE_LINK | BATTLE_TYPE_TRAINER; }
    PARAMETRIZE { battleType = BATTLE_TYPE_RECORDED | BATTLE_TYPE_TRAINER; }
    PARAMETRIZE { battleType = BATTLE_TYPE_INGAME_PARTNER | BATTLE_TYPE_TRAINER; }
    PARAMETRIZE { battleType = BATTLE_TYPE_BATTLE_TOWER | BATTLE_TYPE_TRAINER; }

    ASSUME(RANDOLOCKE_FAINT_COSTS_MON == TRUE);
    SetUpRun(2);
    Faint(0);
    gBattleTypeFlags = battleType;

    RandolockeBoxFaintedMons();

    EXPECT_EQ(gPartiesCount[B_TRAINER_PLAYER], 2);
    EXPECT(InPC(SPECIES_WOBBUFFET) == NULL);
    gBattleTypeFlags = 0;
}

TEST("Randolocke: the dead stay locked in the PC until the player is Champion")
{
    SetUpRun(1);
    EXPECT(RandolockeDeadMonsAreLocked());

    FlagSet(FLAG_IS_CHAMPION);
    EXPECT(!RandolockeDeadMonsAreLocked());
}

#endif // RANDOLOCKE_NUZLOCKE_RULES && RANDOLOCKE_WIPE_COSTS_PARTY
