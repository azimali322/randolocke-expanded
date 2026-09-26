#include "global.h"
#include "battle_util.h"
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

#if RZ_BOSS_SMART_MOVESETS == TRUE && RANDOMIZER_AVAILABLE == TRUE
// --- A boss's moveset ------------------------------------------------------------
// A boss chooses its four moves from what its species learns by its level -- the
// randomized learnset when that is on -- instead of keeping the last four it learned,
// which is how every other trainer Pokemon gets its moves. A randomized learnset deals
// seven same-type attacks, seven status moves and seven other attacks across the levels,
// so from the midgame on a boss has a real choice to make:
//
//   1. its strongest same-type attack;
//   2. the attack of another type that best covers what the first does not hit well --
//      for a dual type, often its other same-type attack;
//   3. a setup move raising the stat those attack from, or Speed -- or, with none to
//      hand, the status move the move tier list rates best, if it rates it Filler or
//      better (Toxic, Will-O-Wisp, Protect, Recover);
//   4. an attack of a new type if one still adds coverage, else that kind of status move,
//      else the strongest attack left, else whatever is left.
//
// "Strongest" is power times accuracy times the base stat the move attacks from, marked
// down for the obvious drawbacks. It ranks moves against each other; it does not predict
// damage. Nothing here is random, so a boss brings the same moves every time.

#define BOSS_POOL_MAX 64

struct BossChoice
{
    enum Species species;
    u32 poolSize;
    u32 chosen;
    bool32 haveAttack;
    bool32 physical;    // the attacks chosen so far include a physical one
    bool32 special;     // ... and a special one
    u32 attackTypes;    // a bit for each type the chosen attacks are
    enum Move *moves;
    enum Move pool[BOSS_POOL_MAX];
    bool8 taken[BOSS_POOL_MAX];
    u8 best[NUMBER_OF_MON_TYPES];   // the best matchup the chosen attacks have on each type
};

// How hard `move` hits for this species, before type matchups. Zero for a status move.
static u32 BossAttackScore(enum Move move, enum Species species)
{
    const struct SpeciesInfo *info = &gSpeciesInfo[species];
    u32 power = GetMovePower(move);
    u32 accuracy = GetMoveAccuracy(move);
    u32 hits = GetMoveStrikeCount(move);
    u32 stat, score;

    if (GetMoveCategory(move) == DAMAGE_CATEGORY_STATUS)
        return 0;

    // A power of 1 marks damage the power does not decide -- Seismic Toss, Super Fang,
    // Counter, Low Kick -- so those rank as a middling attack rather than the weakest.
    if (power <= 1)
        power = 60;
    if (IsMultiHitMove(move))
        hits = 3;       // two to five hits; three is about the average
    else if (hits == 0)
        hits = 1;
    if (accuracy == 0)
        accuracy = 100; // never misses

    if (GetMoveEffect(move) == EFFECT_BODY_PRESS)
        stat = info->baseDefense;
    else if (GetMoveCategory(move) == DAMAGE_CATEGORY_PHYSICAL)
        stat = info->baseAttack;
    else
        stat = info->baseSpAttack;

    score = power * hits * accuracy / 100 * stat;

    // The boss is gone after one, so it only makes the cut when nothing near as strong is
    // to hand: Explosion's 250 ranks with a 40-power attack.
    if (IsExplosionMove(move))
        return score / 6;
    if (MoveHasAdditionalEffectSelf(move, MOVE_EFFECT_RECHARGE))
        return score / 2;   // and the next turn is the player's

    switch (GetMoveEffect(move))
    {
    // A charge turn, a delay, or a condition that often is not met.
    case EFFECT_SOLAR_BEAM:
    case EFFECT_TWO_TURNS_ATTACK:
    case EFFECT_SKY_DROP:
    case EFFECT_FUTURE_SIGHT:
    case EFFECT_FOCUS_PUNCH:
    case EFFECT_FIRST_TURN_ONLY:
    case EFFECT_SHELL_TRAP:
    case EFFECT_UPPER_HAND:
    case EFFECT_REFLECT_DAMAGE:
    case EFFECT_BIDE:
        return score / 2;
    // Fails outright unless something else happened first.
    case EFFECT_DREAM_EATER:
    case EFFECT_SNORE:
    case EFFECT_LAST_RESORT:
    case EFFECT_BELCH:
    case EFFECT_SYNCHRONOISE:
    case EFFECT_SPIT_UP:
    case EFFECT_NATURAL_GIFT:
    case EFFECT_FLING:
    case EFFECT_POLTERGEIST:
    case EFFECT_STEEL_ROLLER:
    case EFFECT_FINAL_GAMBIT:
        return score / 4;
    default:
        return score;
    }
}

static bool32 BossIsStab(enum Move move, enum Species species)
{
    enum Type type = GetMoveType(move);

    return type == gSpeciesInfo[species].types[0] || type == gSpeciesInfo[species].types[1];
}

// A matchup in whole steps, for counting coverage: 0 immune, 1 resisted, 2 neutral, 3 super.
static u32 BossMatchup(enum Type attack, enum Type defend)
{
    uq4_12_t modifier = GetTypeModifier(attack, defend);

    if (modifier == UQ_4_12(0.0))
        return 0;
    if (modifier < UQ_4_12(1.0))
        return 1;
    if (modifier == UQ_4_12(1.0))
        return 2;
    return 3;
}

// How many steps an attack of `type` adds over the best the chosen attacks already manage,
// summed over every type it could face.
static u32 BossCoverageGain(const struct BossChoice *choice, enum Type type)
{
    u32 gain = 0;

    for (enum Type defend = TYPE_NORMAL; defend < NUMBER_OF_MON_TYPES; defend++)
    {
        u32 step;

        if (defend == TYPE_MYSTERY || defend == TYPE_STELLAR)
            continue;
        step = BossMatchup(type, defend);
        if (step > choice->best[defend])
            gain += step - choice->best[defend];
    }
    return gain;
}

enum BossAttackPick
{
    BOSS_STRONGEST_STAB,    // same-type attacks, on strength alone
    BOSS_NEW_TYPE,          // attacks of a type none of the chosen ones is
    BOSS_NEW_COVERAGE,      // ... and only if it adds to the coverage
    BOSS_ANY_ATTACK,
};

// The best attack not yet taken that `which` allows, or -1. Beyond the strongest same-type
// attack, a same-type attack counts half again and each is weighed by what it adds to the
// coverage of the attacks already chosen.
static s32 BossBestAttack(const struct BossChoice *choice, enum BossAttackPick which)
{
    u32 bestScore = 0, i;
    s32 pick = -1;

    for (i = 0; i < choice->poolSize; i++)
    {
        enum Move move = choice->pool[i];
        u32 score = BossAttackScore(move, choice->species);
        u32 gain;

        if (choice->taken[i] || score == 0)
            continue;
        if (which == BOSS_STRONGEST_STAB)
        {
            if (!BossIsStab(move, choice->species))
                continue;
        }
        else
        {
            if (which != BOSS_ANY_ATTACK && (choice->attackTypes & (1u << GetMoveType(move))))
                continue;
            gain = choice->haveAttack ? BossCoverageGain(choice, GetMoveType(move)) : 0;
            if (which == BOSS_NEW_COVERAGE && gain == 0)
                continue;
            if (BossIsStab(move, choice->species))
                score = score * 3 / 2;
            score = score * (8 + gain) / 8;
        }
        // On a tie the later move wins: learned later, and the learnset puts stronger later.
        if (score >= bestScore)
        {
            bestScore = score;
            pick = i;
        }
    }
    return pick;
}

// How much `move` sets this boss up: stages raised in the stat its attacks use, counted
// three times over, Speed twice, the defences once, less any stages it costs itself. Zero
// for a move that raises neither the attacking stat nor Speed -- Iron Defense is not
// setting up, and nor is evasion, which is the kind of difficulty nobody enjoys.
static u32 BossSetupScore(const struct BossChoice *choice, enum Move move)
{
    u32 offense = 0, speed = 0, bulk = 0, cost = 0, count, i;

    if (GetMoveCategory(move) != DAMAGE_CATEGORY_STATUS)
        return 0;
    // Curse raises stats only for a Pokemon that is not a Ghost; a Ghost pays HP instead.
    if (GetMoveEffect(move) == EFFECT_CURSE)
    {
        if (gSpeciesInfo[choice->species].types[0] == TYPE_GHOST
         || gSpeciesInfo[choice->species].types[1] == TYPE_GHOST)
            return 0;
    }
    else if (GetMoveTarget(move) != TARGET_USER && GetMoveTarget(move) != TARGET_USER_AND_ALLY)
    {
        return 0;
    }

    count = GetMoveAdditionalEffectCount(move);
    for (i = 0; i < count; i++)
    {
        const struct AdditionalEffect *effect = GetMoveAdditionalEffectById(move, i);

        // Belly Drum's "maximum" counts as three stages: a strong boost, but a costly one.
        if (effect->moveEffect == STAT_CHANGE_EFFECT_PLUS)
        {
            if (choice->physical)
                offense += min(effect->attack, 3);
            if (choice->special)
                offense += min(effect->spAtk, 3);
            speed += min(effect->speed, 3);
            bulk += effect->defense + effect->spDef;
        }
        else if (effect->moveEffect == STAT_CHANGE_EFFECT_MINUS)
        {
            cost += effect->attack + effect->defense + effect->spAtk + effect->spDef + effect->speed;
        }
    }
    if (offense == 0 && speed == 0)
        return 0;
    if (3 * offense + 2 * speed + bulk <= cost)
        return 1;
    return 3 * offense + 2 * speed + bulk - cost;
}

static s32 BossBestSetup(const struct BossChoice *choice)
{
    u32 bestScore = 0, i;
    s32 pick = -1;

    for (i = 0; i < choice->poolSize; i++)
    {
        u32 score = choice->taken[i] ? 0 : BossSetupScore(choice, choice->pool[i]);

        if (score != 0 && score >= bestScore)
        {
            bestScore = score;
            pick = i;
        }
    }
    return pick;
}

// The best status move not yet taken that the tier list rates `worstTier` or better: the
// better tier first, then the better setup move, then the one learned later. -1 if none.
static s32 BossBestStatus(const struct BossChoice *choice, u32 worstTier)
{
    u32 bestTier = RANDOMIZER_MOVE_TIER_UNRATED + 1, bestSetup = 0, i;
    s32 pick = -1;

    for (i = 0; i < choice->poolSize; i++)
    {
        u32 tier, setup;

        if (choice->taken[i] || GetMoveCategory(choice->pool[i]) != DAMAGE_CATEGORY_STATUS)
            continue;
        // Helping Hand and the like only help a partner; most boss battles have none.
        if (GetMoveTarget(choice->pool[i]) == TARGET_ALLY)
            continue;
        tier = RandomizerGetMoveTier(choice->pool[i]);
        if (tier > worstTier)
            continue;
        setup = BossSetupScore(choice, choice->pool[i]);
        if (tier < bestTier || (tier == bestTier && setup >= bestSetup))
        {
            bestTier = tier;
            bestSetup = setup;
            pick = i;
        }
    }
    return pick;
}

// The move learned latest that is not yet taken, or -1: the last resort, so a boss never
// goes into battle with an empty slot it could have filled.
static s32 BossLatestLeft(const struct BossChoice *choice)
{
    s32 i;

    for (i = choice->poolSize - 1; i >= 0; i--)
    {
        if (!choice->taken[i])
            return i;
    }
    return -1;
}

static void BossTake(struct BossChoice *choice, s32 index)
{
    enum Move move = choice->pool[index];

    choice->taken[index] = TRUE;
    choice->moves[choice->chosen++] = move;
    if (BossAttackScore(move, choice->species) == 0)
        return;

    choice->haveAttack = TRUE;
    choice->attackTypes |= 1u << GetMoveType(move);
    if (GetMoveCategory(move) == DAMAGE_CATEGORY_PHYSICAL)
        choice->physical = TRUE;
    else
        choice->special = TRUE;
    for (enum Type defend = TYPE_NORMAL; defend < NUMBER_OF_MON_TYPES; defend++)
    {
        u32 step = BossMatchup(GetMoveType(move), defend);

        if (step > choice->best[defend])
            choice->best[defend] = step;
    }
}

void RandolockeChooseBossMoves(enum Species species, u32 level, enum Move *moves)
{
    const struct LevelUpMove *learnset = GetSpeciesLevelUpLearnset(species);
    struct BossChoice choice = {.species = species, .moves = moves};
    s32 pick;
    u32 i, j;

    for (i = 0; i < MAX_MON_MOVES; i++)
        moves[i] = MOVE_NONE;

    // Everything it has learned by this level, once each -- read the way the last-four
    // moveset reads it, so the two draw from exactly the same moves.
    for (i = 0; learnset[i].move != LEVEL_UP_MOVE_END && choice.poolSize < BOSS_POOL_MAX; i++)
    {
        if (learnset[i].level > level)
            break;
        if (learnset[i].level == 0)
            continue;
        for (j = 0; j < choice.poolSize && choice.pool[j] != learnset[i].move; j++)
            ;
        if (j == choice.poolSize)
            choice.pool[choice.poolSize++] = learnset[i].move;
    }

    // 1. The strongest same-type attack -- or, early on when it has learned none, its
    // strongest attack of any type.
    pick = BossBestAttack(&choice, BOSS_STRONGEST_STAB);
    if (pick < 0)
        pick = BossBestAttack(&choice, BOSS_NEW_TYPE);
    if (pick >= 0)
        BossTake(&choice, pick);

    // 2. The attack of another type that best covers the rest.
    pick = BossBestAttack(&choice, BOSS_NEW_TYPE);
    if (pick >= 0)
        BossTake(&choice, pick);

    // 3. Setting up, for the stat those attacks use -- read off the species when the pool
    // offered no attack at all -- or failing that, a status move worth the slot.
    if (!choice.haveAttack)
    {
        choice.physical = gSpeciesInfo[species].baseAttack >= gSpeciesInfo[species].baseSpAttack;
        choice.special = !choice.physical;
    }
    pick = BossBestSetup(&choice);
    if (pick < 0)
        pick = BossBestStatus(&choice, RANDOMIZER_MOVE_TIER_FILLER);
    if (pick >= 0)
        BossTake(&choice, pick);

    // 4. The rest: new coverage, then a status move worth the slot, then the strongest
    // attack left even if its type is taken, then any status move, then anything at all.
    while (choice.chosen < MAX_MON_MOVES)
    {
        pick = BossBestAttack(&choice, BOSS_NEW_COVERAGE);
        if (pick < 0)
            pick = BossBestStatus(&choice, RANDOMIZER_MOVE_TIER_FILLER);
        if (pick < 0)
            pick = BossBestAttack(&choice, BOSS_ANY_ATTACK);
        if (pick < 0)
            pick = BossBestStatus(&choice, RANDOMIZER_MOVE_TIER_UNRATED);
        if (pick < 0)
            pick = BossLatestLeft(&choice);
        if (pick < 0)
            break;
        BossTake(&choice, pick);
    }
}

static void RandolockeGiveBossMoveset(struct Pokemon *mon)
{
    enum Move moves[MAX_MON_MOVES];
    u32 i;

    RandolockeChooseBossMoves(GetMonData(mon, MON_DATA_SPECIES, NULL), GetLevelFromMonExp(mon), moves);
    for (i = 0; i < MAX_MON_MOVES; i++)
        SetMonMoveSlot(mon, moves[i], i);
}
#endif // RZ_BOSS_SMART_MOVESETS

static void CustomTrainerPartyAssignMoves(struct Pokemon *mon, const struct TrainerMon *partyEntry, bool32 speciesWasRandomized, bool32 isBoss)
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
        // randolocke: a boss chooses its four (RZ_BOSS_SMART_MOVESETS); everyone else keeps
        // the last four it learned.
        #if RZ_BOSS_SMART_MOVESETS == TRUE && RANDOMIZER_AVAILABLE == TRUE
        if (isBoss)
        {
            RandolockeGiveBossMoveset(mon);
            return;
        }
        #endif
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
    CustomTrainerPartyAssignMoves(mon, trainerMon, species != trainerMon->species, trainer->rzIsBossTrainer);
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
