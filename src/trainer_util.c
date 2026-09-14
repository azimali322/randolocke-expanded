#include "global.h"
#include "constants/flags.h"
#include "event_data.h"
#include "config/randomizer.h"
#include "main.h"
#include "data.h"
#include "move.h"
#include "random.h"
#include "string_util.h"
#include "trainer_util.h"
#include "randomizer.h"
#include "constants/opponents.h"
#include "text.h"

#include "constants/battle_ai.h"
#include "constants/pokeball.h"

rng_value_t GeneratePartySeed(const struct Trainer *trainer)
{
    u32 seed = Crc32B((const u8 *)trainer, sizeof(struct Trainer)) ^ READ_OTID_FROM_SAVE;
    return LocalRandomSeed(seed);
}

#if RZ_TRAINER_EV_SCALING == TRUE
// Trainer Pokemon get an EV spread that grows with the player's badge count. Which stats
// it lands on is decided from the Pokemon's own base stats rather than fixed, because the
// species may have been randomized into something that wants the other half of the sheet.
// Only ordinary trainers reach here -- Frontier and Battle Tower parties are built
// elsewhere -- so there is no facility check to make.
static void RandolockeGiveTrainerEVs(struct Pokemon *mon)
{
    static const u8 sEvsByBadge[] = RZ_TRAINER_EVS_BY_BADGE;
    static const u16 sBadgeFlags[] = {
        FLAG_BADGE01_GET, FLAG_BADGE02_GET, FLAG_BADGE03_GET, FLAG_BADGE04_GET,
        FLAG_BADGE05_GET, FLAG_BADGE06_GET, FLAG_BADGE07_GET, FLAG_BADGE08_GET,
    };
    enum Species species = GetMonData(mon, MON_DATA_SPECIES, NULL);
    u32 badges = 0, i;
    u8 ev;

    for (i = 0; i < ARRAY_COUNT(sBadgeFlags); i++)
    {
        if (FlagGet(sBadgeFlags[i]))
            badges++;
    }
    ev = sEvsByBadge[badges];
    if (ev == 0)
        return;

    SetMonData(mon, MON_DATA_HP_EV, &ev);
    SetMonData(mon, MON_DATA_SPEED_EV, &ev);
    if (gSpeciesInfo[species].baseAttack >= gSpeciesInfo[species].baseSpAttack)
        SetMonData(mon, MON_DATA_ATK_EV, &ev);
    else
        SetMonData(mon, MON_DATA_SPATK_EV, &ev);
    if (gSpeciesInfo[species].baseDefense >= gSpeciesInfo[species].baseSpDefense)
        SetMonData(mon, MON_DATA_DEF_EV, &ev);
    else
        SetMonData(mon, MON_DATA_SPDEF_EV, &ev);
}
#endif

static void CustomTrainerPartyAssignMoves(struct Pokemon *mon, const struct TrainerMon *partyEntry)
{
    bool32 noMoveSet = TRUE;
    u32 j;

    for (j = 0; j < MAX_MON_MOVES; ++j)
    {
        if (partyEntry->moves[j] != MOVE_NONE)
            noMoveSet = FALSE;
    }
    if (noMoveSet)
    {
        GiveMonInitialMoveset(mon);
        // TODO: Figure out a default strategy when moves are not set, to generate a good moveset
        return;
    }

    for (j = 0; j < MAX_MON_MOVES; ++j)
    {
        u32 pp = GetMovePP(partyEntry->moves[j]);
        SetMonData(mon, MON_DATA_MOVE1 + j, &partyEntry->moves[j]);
        SetMonData(mon, MON_DATA_PP1 + j, &pp);
    }
}

u32 GeneratePersonalityForGender(u32 gender, u32 species)
{
    const struct SpeciesInfo *speciesInfo = &gSpeciesInfo[species];
    if (gender == MON_MALE)
    {
        assertf(speciesInfo->genderRatio < MON_FEMALE, "species %d cannot be male", species);
        return ((255 - speciesInfo->genderRatio) / 2) + speciesInfo->genderRatio;
    }
    if (gender == MON_FEMALE)
    {
        assertf(speciesInfo->genderRatio != MON_MALE && speciesInfo->genderRatio != MON_GENDERLESS, "species %d cannot be female", species);
        return speciesInfo->genderRatio / 2;
    }
    if (gender == MON_GENDERLESS)
        assertf(speciesInfo->genderRatio == MON_GENDERLESS, "species %d cannot be genderless", species);
    else
        errorf("GeneratePersonalityForGender called with invalid gender value %d", gender);
    return 0;
}

const u8 sModuloLUT[25] = {0, 21, 17, 13, 9, 5, 1, 22, 18, 14, 10, 6, 2, 23, 19, 15, 11, 7, 3, 24, 20, 16, 12, 8, 4};

static void ModifyPersonalityForNature(u32 *personality, s32 newNature)
{
    s32 nature = GetNatureFromPersonality(*personality);
    s32 diff = abs(newNature - nature);
    s32 sign = (newNature > nature) ? 1 : -1;
    if (diff > NUM_NATURES / 2)
    {
        diff = NUM_NATURES - diff;
        sign *= -1;
    }
    *personality += (sModuloLUT[diff] * 0x100 * sign);
}

static bool32 SetCorrectAbilityNum(struct Pokemon *mon, enum Species species, enum Ability ability)
{
    const struct SpeciesInfo *speciesInfo = &gSpeciesInfo[species];
    u32 abilityNum;
    u32 maxAbilityNum = ARRAY_COUNT(speciesInfo->abilities);
    for (abilityNum = 0; abilityNum < maxAbilityNum; ++abilityNum)
    {
        if (speciesInfo->abilities[abilityNum] == ability)
            break;
    }
    assertf(abilityNum < maxAbilityNum, "illegal ability %S for %S", gAbilitiesInfo[ability].name, speciesInfo->speciesName)
    {
        return FALSE;
    }
    SetMonData(mon, MON_DATA_ABILITY_NUM, &abilityNum);
    return TRUE;
}

void MakeTrainerGenerator(struct TrainerGenerator *trainerGen, const struct Trainer *trainer, u16 trainerId)
{
    trainerGen->rzTrainerId = trainerId;
    trainerGen->rzIsBossTrainer = trainer->isBossTrainer;
    trainerGen->rzSlot = 0;
    trainerGen->rzTotalMons = trainer->partySize;
    trainerGen->gender = trainer->gender;
    if (trainer->aiFlags & AI_FLAG_SMART_TERA)
        trainerGen->smartTera = TRUE;
    trainerGen->isFrontier = FALSE;
    StringCopyN(trainerGen->name, trainer->trainerName, TRAINER_NAME_LENGTH + 1);
    trainerGen->trainerClass = trainer->trainerClass;
    trainerGen->otID = OTID_STRUCT_RANDOM_NO_SHINY;
    trainerGen->localRngState = GeneratePartySeed(trainer);
}

void MakePartnerGenerator(struct TrainerGenerator *trainerGen, const struct Trainer *partner)
{
    u32 otID;
    // Callers pass a stack local, so these must be set explicitly or they are garbage.
    // Partner parties are not randomized (matching tertu's original behaviour).
    trainerGen->rzTrainerId = TRAINER_NONE;
    trainerGen->rzIsBossTrainer = FALSE;
    trainerGen->rzSlot = 0;
    trainerGen->rzTotalMons = partner->partySize;
    trainerGen->gender = partner->gender;
    if (partner->aiFlags & AI_FLAG_SMART_TERA)
        trainerGen->smartTera = TRUE;
    trainerGen->isFrontier = FALSE;
    StringCopyN(trainerGen->name, partner->trainerName, TRAINER_NAME_LENGTH + 1);
    trainerGen->trainerClass = partner->trainerClass;
    otID = Crc32B((const u8 *)partner, sizeof(struct Trainer));
    trainerGen->otID = OTID_STRUCT_PRESET(otID);
    trainerGen->localRngState = LocalRandomSeed(otID);
}

void GenerateMonFromTrainerMon(struct Pokemon *mon, const struct TrainerMon *trainerMon, struct TrainerGenerator *trainer)
{
    u32 data;
    u32 personality = (LocalRandom32(&trainer->localRngState) & 0xFFFFDF00) + 0x1000;
    u32 genderValue = 0;
    if (trainerMon->gender == TRAINER_MON_RANDOM_GENDER)
        genderValue = LocalRandom32(&trainer->localRngState) & 0x000000FF;
    else if (trainerMon->gender == TRAINER_MON_MALE)
        genderValue = GeneratePersonalityForGender(MON_MALE, trainerMon->species);
    else if (trainerMon->gender == TRAINER_MON_FEMALE)
        genderValue = GeneratePersonalityForGender(MON_FEMALE, trainerMon->species);
    else
        errorf("Unkwown trainer mon gender value %d", trainerMon->gender);
    personality |= genderValue;
    ModifyPersonalityForNature(&personality, trainerMon->nature);

    enum Species species = trainerMon->species;
    #if RANDOMIZER_AVAILABLE == TRUE
        // `Boss: Yes` is only a label; RZ_RANDOMIZE_BOSS_TRAINERS decides whether it
        // exempts anyone. TRAINER_NONE means the caller had no trainer id
        // (debug/synthetic trainers), so those are always left alone.
        if ((RZ_RANDOMIZE_BOSS_TRAINERS || !trainer->rzIsBossTrainer)
         && trainer->rzTrainerId != TRAINER_NONE)
        {
            species = RandomizeTrainerMon(trainer->rzTrainerId, trainer->rzSlot,
                                          trainer->rzTotalMons, species);
        }
    #endif

    CreateMon(mon, species, trainerMon->lvl, personality, trainer->otID);
    {
        // Abilities follow the same rule as species.
        u8 cantRandomizeAbility = (!RZ_RANDOMIZE_BOSS_TRAINERS && trainer->rzIsBossTrainer);
        SetMonData(mon, MON_DATA_CANT_RANDOMIZE_ABILITY, &cantRandomizeAbility);
    }
    if (trainerMon->nickname != NULL)
        SetMonData(mon, MON_DATA_NICKNAME, trainerMon->nickname);
    if (trainerMon->ev) //ev in struct TrainerMon are stored in Showdown order not vanilla Emerald order
    {
        SetMonData(mon, MON_DATA_HP_EV, &trainerMon->ev[0]);
        SetMonData(mon, MON_DATA_ATK_EV, &trainerMon->ev[1]);
        SetMonData(mon, MON_DATA_DEF_EV, &trainerMon->ev[2]);
        SetMonData(mon, MON_DATA_SPATK_EV, &trainerMon->ev[3]);
        SetMonData(mon, MON_DATA_SPDEF_EV, &trainerMon->ev[4]);
        SetMonData(mon, MON_DATA_SPEED_EV, &trainerMon->ev[5]);
    }

    SetMonData(mon, MON_DATA_IVS, &trainerMon->iv);
    CustomTrainerPartyAssignMoves(mon, trainerMon);
    SetMonData(mon, MON_DATA_HELD_ITEM, &trainerMon->heldItem);

    bool32 abilitySet = FALSE;
    if (trainerMon->ability)
    {
        abilitySet = SetCorrectAbilityNum(mon, species, trainerMon->ability);
    }

    if (!abilitySet)
    {
        if (B_TRAINER_MON_RANDOM_ABILITY == 2)
        {
            do {
                data = Random() % NUM_ABILITY_SLOTS; // includes hidden abilities
            } while (GetAbilityBySpecies(species, data, FALSE) == ABILITY_NONE);
            SetMonData(mon, MON_DATA_ABILITY_NUM, &data);
        }
        else if (B_TRAINER_MON_RANDOM_ABILITY == 0)
        {
            data = 0;
            SetMonData(mon, MON_DATA_ABILITY_NUM, &data);
        }
        // else B_TRAINER_MON_RANDOM_ABILITY == 1
        // this is the default from CreateMon (random non-hidden ability based on personality)
    }

    if (trainerMon->ball < POKEBALL_COUNT)
    {
        data = trainerMon->ball;
        SetMonData(mon, MON_DATA_POKEBALL, &data);
    }
    else if (B_TRAINER_CLASS_POKE_BALLS >= GEN_7 && trainer->trainerClass && trainerMon->ball == POKEBALL_COUNT)
    {
        data = gTrainerClasses[trainer->trainerClass].ball ?: BALL_POKE;
        SetMonData(mon, MON_DATA_POKEBALL, &data);
    }
    else if (trainerMon->ball > POKEBALL_COUNT)
    {
        errorf("Invalid ball for %S in %S's party", GetMonData(mon, MON_DATA_NICKNAME), trainer->name);
    }

    SetMonData(mon, MON_DATA_FRIENDSHIP, &trainerMon->friendship);

    data = trainerMon->isShiny;
    SetMonData(mon, MON_DATA_IS_SHINY, &data);
    if (trainerMon->shouldUseDynamax)
    {
        data = trainerMon->dynamaxLevel;
    }
    else
    {
        data = BLOCK_AI_DYNAMAX;
    }
    SetMonData(mon, MON_DATA_DYNAMAX_LEVEL, &data);
    if (trainerMon->gigantamaxFactor)
    {
        data = trainerMon->gigantamaxFactor;
        SetMonData(mon, MON_DATA_GIGANTAMAX_FACTOR, &data);
    }
    if (trainerMon->teraType)
    {
        data = trainerMon->teraType;
        SetMonData(mon, MON_DATA_TERA_TYPE, &data);
    }
    else if (!trainer->smartTera)
    {
        data = TYPE_MYSTERY;
        SetMonData(mon, MON_DATA_TERA_TYPE, &data);
    }

    #if RZ_TRAINER_EV_SCALING == TRUE
        RandolockeGiveTrainerEVs(mon);
    #endif

    CalculateMonStats(mon);
    SetMonData(mon, MON_DATA_OT_NAME, trainer->name);
    data = trainer->gender;
    SetMonData(mon, MON_DATA_OT_GENDER, &data);
}
