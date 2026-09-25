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

// Badges earned, 0 to 8: what the trainer EVs and a boss's IV ramp both scale with.
static u32 UNUSED RandolockeBadgeCount(void)
{
    static const u16 sBadgeFlags[] = {
        FLAG_BADGE01_GET, FLAG_BADGE02_GET, FLAG_BADGE03_GET, FLAG_BADGE04_GET,
        FLAG_BADGE05_GET, FLAG_BADGE06_GET, FLAG_BADGE07_GET, FLAG_BADGE08_GET,
    };
    u32 badges = 0, i;

    for (i = 0; i < ARRAY_COUNT(sBadgeFlags); i++)
    {
        if (FlagGet(sBadgeFlags[i]))
            badges++;
    }
    return badges;
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
    enum Species species = GetMonData(mon, MON_DATA_SPECIES, NULL);
    u8 ev;

    ev = sEvsByBadge[RandolockeBadgeCount()];
    if (ev == 0)
        return;

    // The attacking stat the species can actually use.
    if (gSpeciesInfo[species].baseAttack >= gSpeciesInfo[species].baseSpAttack)
        SetMonData(mon, MON_DATA_ATK_EV, &ev);
    else
        SetMonData(mon, MON_DATA_SPATK_EV, &ev);

    // Then Speed if it is fast enough to be worth the investment, HP if it is not.
    if (gSpeciesInfo[species].baseSpeed >= RZ_TRAINER_EV_SPEED_THRESHOLD)
        SetMonData(mon, MON_DATA_SPEED_EV, &ev);
    else
        SetMonData(mon, MON_DATA_HP_EV, &ev);

    // 252 and 252 leaves 6 of the legal 510. A player would put them somewhere, so the
    // better defence gets them.
    if (ev >= MAX_PER_STAT_EVS)
    {
        u8 spare = MAX_TOTAL_EVS - (2 * MAX_PER_STAT_EVS);

        if (gSpeciesInfo[species].baseDefense >= gSpeciesInfo[species].baseSpDefense)
            SetMonData(mon, MON_DATA_DEF_EV, &spare);
        else
            SetMonData(mon, MON_DATA_SPDEF_EV, &spare);
    }
}
#endif

#if RZ_TRAINER_IVS == TRUE
static const u32 sIvFields[NUM_STATS] =
{
    MON_DATA_HP_IV, MON_DATA_ATK_IV, MON_DATA_DEF_IV,
    MON_DATA_SPEED_IV, MON_DATA_SPATK_IV, MON_DATA_SPDEF_IV,
};

// Rolled per stat. A boss's are perfect instead -- all of them, or with RZ_BOSS_IV_RAMP the
// strongest few, which RandolockeApplyBossIVRamp raises once the whole team exists. See the
// config for what the data file hands out instead.
static void RandolockeGiveTrainerIVs(struct Pokemon *mon, u32 trainerId, u32 slot, bool32 isBoss)
{
    struct Sfc32State state;
    u32 i;
    u8 iv;

    if (isBoss && !RZ_BOSS_IV_RAMP)
    {
        iv = MAX_PER_STAT_IVS;
        for (i = 0; i < NUM_STATS; i++)
            SetMonData(mon, sIvFields[i], &iv);
        return;
    }

    state = RandomizerRandSeed(RANDOMIZER_REASON_TRAINER_IV, trainerId, slot);
    for (i = 0; i < NUM_STATS; i++)
    {
        iv = RandomizerNextRange(&state, MAX_PER_STAT_IVS + 1);
        SetMonData(mon, sIvFields[i], &iv);
    }
}

#if RZ_BOSS_IV_RAMP == TRUE
// Three of six: the attacking stat the species uses, HP, and Speed if it is fast enough to
// use it, its better defence if not -- the same reading of base stats the EVs make.
static void SetKeyIVsPerfect(struct Pokemon *mon)
{
    enum Species species = GetMonData(mon, MON_DATA_SPECIES, NULL);
    const struct SpeciesInfo *info = &gSpeciesInfo[species];
    u8 iv = MAX_PER_STAT_IVS;

    SetMonData(mon, info->baseAttack >= info->baseSpAttack ? MON_DATA_ATK_IV : MON_DATA_SPATK_IV, &iv);
    SetMonData(mon, MON_DATA_HP_IV, &iv);
    if (info->baseSpeed >= RZ_TRAINER_EV_SPEED_THRESHOLD)
        SetMonData(mon, MON_DATA_SPEED_IV, &iv);
    else
        SetMonData(mon, info->baseDefense >= info->baseSpDefense ? MON_DATA_DEF_IV : MON_DATA_SPDEF_IV, &iv);
}

// A boss's perfect IVs, rationed by badges: one perfect Pokemon at none, and each badge
// adds half a step -- a three-of-six Pokemon, then a second perfect one, and so on, to five
// perfect at eight badges. The Elite Four and the Champion are the end of the ramp, perfect
// throughout. Handed out by level, highest first, so the ace is always the first to get
// them; a tie goes to the later slot, which is where the ace sits.
void RandolockeApplyBossIVRamp(struct Pokemon *party, u32 count, const struct Trainer *trainer)
{
    u8 order[PARTY_SIZE];
    u32 perfect, keyed, n = 0, i, j;

    if (!trainer->isBossTrainer)
        return;

    if (trainer->trainerClass == TRAINER_CLASS_ELITE_FOUR || trainer->trainerClass == TRAINER_CLASS_CHAMPION)
    {
        perfect = PARTY_SIZE;
        keyed = 0;
    }
    else
    {
        u32 badges = RandolockeBadgeCount();

        perfect = 1 + badges / 2;
        keyed = badges % 2;
    }

    for (i = 0; i < count && i < PARTY_SIZE; i++)
    {
        if (GetMonData(&party[i], MON_DATA_SPECIES, NULL) != SPECIES_NONE)
            order[n++] = i;
    }
    // Insertion sort, highest level first; on a tie the later slot goes first.
    for (i = 1; i < n; i++)
    {
        u8 key = order[i];
        u32 level = GetMonData(&party[key], MON_DATA_LEVEL, NULL);

        for (j = i; j > 0; j--)
        {
            u32 prev = GetMonData(&party[order[j - 1]], MON_DATA_LEVEL, NULL);

            if (prev > level || (prev == level && order[j - 1] > key))
                break;
            order[j] = order[j - 1];
        }
        order[j] = key;
    }

    for (i = 0; i < n && i < perfect + keyed; i++)
    {
        struct Pokemon *mon = &party[order[i]];

        if (i < perfect)
        {
            u8 iv = MAX_PER_STAT_IVS;

            for (j = 0; j < NUM_STATS; j++)
                SetMonData(mon, sIvFields[j], &iv);
        }
        else
        {
            SetKeyIVsPerfect(mon);
        }
        CalculateMonStats(mon);
    }
}
#endif
#endif

#if RZ_BOSS_FULL_PARTY == TRUE
// Whether this trainer's team is brought up to six: a boss, or a rival -- except the
// first rival battle on Route 103, fought with a lone level 5 starter and no Poke Balls.
bool32 RandolockeTrainerGetsFullParty(const struct Trainer *trainer, u16 trainerId)
{
    switch (trainerId)
    {
    case TRAINER_NONE:
    case TRAINER_BRENDAN_ROUTE_103_MUDKIP:
    case TRAINER_BRENDAN_ROUTE_103_TREECKO:
    case TRAINER_BRENDAN_ROUTE_103_TORCHIC:
    case TRAINER_MAY_ROUTE_103_MUDKIP:
    case TRAINER_MAY_ROUTE_103_TREECKO:
    case TRAINER_MAY_ROUTE_103_TORCHIC:
        return FALSE;
    default:
        return trainer->isBossTrainer || trainer->trainerClass == TRAINER_CLASS_RIVAL;
    }
}

// An entry for one of the Pokemon added to a team: a copy of one of the trainer's own,
// cycling through the ones ahead of the ace, so its species is randomized from something
// in the same league as the rest. It takes a random level between the team's lowest and
// highest, and none of the template's particulars -- no nickname, item, ability, moves,
// EVs or shininess, all of which the randomizer's trainer handling fills in afresh.
struct TrainerMon RandolockeFillerTrainerMon(const struct Trainer *trainer, const u32 *monIndices,
                                             u32 monsCount, u16 trainerId, u32 slot)
{
    struct Sfc32State state = RandomizerRandSeed(RANDOMIZER_REASON_TRAINER_PAD, trainerId, slot);
    u32 templates = (monsCount > 1) ? monsCount - 1 : 1;
    struct TrainerMon filler = trainer->party[monIndices[slot % templates]];
    u32 lowest = 255, highest = 0, i;

    for (i = 0; i < monsCount; i++)
    {
        u32 level = trainer->party[monIndices[i]].lvl;

        if (level < lowest)
            lowest = level;
        if (level > highest)
            highest = level;
    }

    filler.lvl = lowest + RandomizerNextRange(&state, highest - lowest + 1);
    filler.nickname = NULL;
    filler.ev = NULL;
    filler.heldItem = ITEM_NONE;
    filler.ability = ABILITY_NONE;
    filler.isShiny = FALSE;
    filler.gigantamaxFactor = FALSE;
    filler.shouldUseDynamax = FALSE;
    for (i = 0; i < MAX_MON_MOVES; i++)
        filler.moves[i] = MOVE_NONE;
    return filler;
}
#endif

#if RZ_TRAINER_NATURES == TRUE
// Trainer Pokemon carry no Nature of their own, so every one of them fights on Hardy,
// which modifies nothing. This hands out the nature a player would have picked, read off
// the same base stats the EV spread reads: the fast ones buy Speed with the attacking stat
// they do not use, the slow ones buy power with it. Written as the hidden nature because
// that is what CalculateMonStats reads here, and because rerolling the personality to move
// a nature would take the Pokemon's gender and shininess with it.
static void RandolockeGiveTrainerNature(struct Pokemon *mon)
{
    enum Species species = GetMonData(mon, MON_DATA_SPECIES, NULL);
    bool32 physical = gSpeciesInfo[species].baseAttack >= gSpeciesInfo[species].baseSpAttack;
    bool32 fast = gSpeciesInfo[species].baseSpeed >= RZ_TRAINER_EV_SPEED_THRESHOLD;
    u32 nature;

    if (physical)
        nature = fast ? NATURE_JOLLY : NATURE_ADAMANT;   // -Sp. Atk either way
    else
        nature = fast ? NATURE_TIMID : NATURE_MODEST;    // -Attack either way

    SetMonData(mon, MON_DATA_HIDDEN_NATURE, &nature);
}
#endif

#if RZ_TRAINER_HELD_ITEMS == TRUE
// Items that suit any species: recovery, a survival aid, a damage boost. No Choice items --
// they lock the holder into one move, and an AI that mishandles that is easier to beat.
static const u16 sRandolockeTrainerItems[] =
{
    ITEM_LEFTOVERS, ITEM_SITRUS_BERRY, ITEM_LUM_BERRY, ITEM_FOCUS_BAND, ITEM_FOCUS_SASH,
    ITEM_BRIGHT_POWDER, ITEM_QUICK_CLAW, ITEM_SCOPE_LENS, ITEM_EXPERT_BELT, ITEM_LIFE_ORB,
    ITEM_SHELL_BELL,
};

// A Pokemon with no item of its own is given one, seeded from the trainer and the slot so
// the same trainer is always holding the same thing.
static void RandolockeGiveTrainerHeldItem(struct Pokemon *mon, u32 trainerId, u32 slot, bool32 isBoss)
{
    struct Sfc32State state;
    enum Species species;
    u32 chance, roll;
    u16 item;

    if (GetMonData(mon, MON_DATA_HELD_ITEM, NULL) != ITEM_NONE)
        return;

    state = RandomizerRandSeed(RANDOMIZER_REASON_TRAINER_ITEM, trainerId, slot);
    chance = isBoss ? RZ_TRAINER_ITEM_CHANCE_BOSS : RZ_TRAINER_ITEM_CHANCE;
    if (RandomizerNextRange(&state, 100) >= chance)
        return;

    // One more slot than the shared list: the last is the booster for the category this
    // species attacks from.
    roll = RandomizerNextRange(&state, ARRAY_COUNT(sRandolockeTrainerItems) + 1);
    if (roll < ARRAY_COUNT(sRandolockeTrainerItems))
    {
        item = sRandolockeTrainerItems[roll];
    }
    else
    {
        species = GetMonData(mon, MON_DATA_SPECIES, NULL);
        item = (gSpeciesInfo[species].baseAttack >= gSpeciesInfo[species].baseSpAttack)
             ? ITEM_MUSCLE_BAND : ITEM_WISE_GLASSES;
    }
    SetMonData(mon, MON_DATA_HELD_ITEM, &item);
}
#endif

static void CustomTrainerPartyAssignMoves(struct Pokemon *mon, const struct TrainerMon *partyEntry, bool32 speciesWasRandomized)
{
    bool32 noMoveSet = TRUE;
    u32 j;

    for (j = 0; j < MAX_MON_MOVES; ++j)
    {
        if (partyEntry->moves[j] != MOVE_NONE)
            noMoveSet = FALSE;
    }
    // randolocke: a hand-written moveset belongs to the species it was written for. Once
    // the species has been substituted it is worse than useless -- Roxanne's three
    // Pokemon all carried Tackle / Defense Curl / Rock Throw / Rock Tomb whatever they
    // turned into, so none of them had a single same-type move and all three fought
    // identically. Fall back to the level-up moveset, which GetSpeciesLevelUpLearnset
    // routes through the randomizer, so the Pokemon gets moves that suit the species it
    // actually is: same-type attacks included, strongest first.
    #if RZ_TRAINER_REGENERATE_MOVES == TRUE && RANDOMIZER_AVAILABLE == TRUE
    // The same applies once learnsets are randomized even if the species survived: this
    // run's Geodude does not learn Rock Tomb either way, so a hand-written moveset is a
    // list of moves the Pokemon in front of you cannot actually have.
    if (speciesWasRandomized || RandomizerFeatureEnabled(RANDOMIZE_LEARNSET))
        noMoveSet = TRUE;
    #endif
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
    CustomTrainerPartyAssignMoves(mon, trainerMon, species != trainerMon->species);
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

    #if RZ_TRAINER_IVS == TRUE
        RandolockeGiveTrainerIVs(mon, trainer->rzTrainerId, trainer->rzSlot,
                                 trainer->rzIsBossTrainer);
    #endif
    #if RZ_TRAINER_EV_SCALING == TRUE
        RandolockeGiveTrainerEVs(mon);
    #endif
    #if RZ_TRAINER_NATURES == TRUE
        RandolockeGiveTrainerNature(mon);
    #endif
    #if RZ_TRAINER_HELD_ITEMS == TRUE
        RandolockeGiveTrainerHeldItem(mon, trainer->rzTrainerId, trainer->rzSlot,
                                      trainer->rzIsBossTrainer);
    #endif

    CalculateMonStats(mon);
    SetMonData(mon, MON_DATA_OT_NAME, trainer->name);
    data = trainer->gender;
    SetMonData(mon, MON_DATA_OT_GENDER, &data);
}
