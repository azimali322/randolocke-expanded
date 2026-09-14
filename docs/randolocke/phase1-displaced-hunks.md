# Phase 1 merge: randomizer hunks displaced by the 1.17 merge

Auto-captured during the `rhh/master` merge. Each entry is the tertu-randomizer side
of a conflict that was resolved in favour of upstream. Phase 3 must re-land the
randomizer behaviour these represent.

## `data/scripts/obtain_item.inc`

### hunk 1 — randomizer side (ours)

```c
	#if RANDOMIZER_AVAILABLE == TRUE
	callnative FindHiddenItemRandomize_NativeCall
	#endif
```


## `src/battle_ai_util.c`

### hunk 1 — randomizer side (ours)

```c
u32 AI_GetBattlerAbility(u32 battler)
{
    u16 abilityToUse = GetAbilityBySpecies(gBattleMons[battler].species, gBattleMons[battler].abilityNum, gBattleMons[battler].cantRandomizeAbility);
    if (gAbilitiesInfo[abilityToUse].cantBeSuppressed)
        return abilityToUse;

    if (gStatuses3[battler] & STATUS3_GASTRO_ACID)
        return ABILITY_NONE;

    if (IsNeutralizingGasOnField()
     && abilityToUse != ABILITY_NEUTRALIZING_GAS
     && GetBattlerHoldEffectIgnoreAbility(battler, TRUE) != HOLD_EFFECT_ABILITY_SHIELD)
        return ABILITY_NONE;

    return abilityToUse;
}
```


## `src/battle_main.c`

### hunk 1 — randomizer side (ours)

```c
u32 GeneratePersonalityForGender(u32 gender, u32 species)
{
    const struct SpeciesInfo *speciesInfo = &gSpeciesInfo[species];
    if (gender == MON_GENDERLESS)
        return 0;
    else if (gender == MON_MALE)
        return ((255 - speciesInfo->genderRatio) / 2) + speciesInfo->genderRatio;
    else
        return speciesInfo->genderRatio / 2;
}

void CustomTrainerPartyAssignMoves(struct Pokemon *mon, const struct TrainerMon *partyEntry)
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

u8 CreateNPCTrainerPartyFromTrainer(struct Pokemon *party, const struct Trainer *trainer, bool32 firstTrainer, u32 battleTypeFlags, u16 seed)
{
    u32 personalityValue;
    s32 i;
    u8 monsCount;
    u8 isTrainerBossTrainer = trainer->isBossTrainer;
    if (battleTypeFlags & BATTLE_TYPE_TRAINER && !(battleTypeFlags & (BATTLE_TYPE_FRONTIER
                                                                        | BATTLE_TYPE_EREADER_TRAINER
                                                                        | BATTLE_TYPE_TRAINER_HILL)))
    {
        if (firstTrainer == TRUE)
            ZeroEnemyPartyMons();

        if (battleTypeFlags & BATTLE_TYPE_TWO_OPPONENTS)
        {
            if (trainer->partySize > PARTY_SIZE / 2)
                monsCount = PARTY_SIZE / 2;
            else
                monsCount = trainer->partySize;
        }
        else
        {
            monsCount = trainer->partySize;
        }

        u32 monIndices[monsCount];
        DoTrainerPartyPool(trainer, monIndices, monsCount, battleTypeFlags);

        for (i = 0; i < monsCount; i++)
        {
            u32 monIndex = monIndices[i];
            s32 ball = -1;
            u32 personalityHash = GeneratePartyHash(trainer, i);
            const struct TrainerMon *partyData = trainer->party;
            u32 otIdType = OT_ID_RANDOM_NO_SHINY;
            u32 fixedOtId = 0;
            u32 ability = 0;
            u16 species = partyData[monIndex].species;

            #if (RANDOMIZER_AVAILABLE)
                if(!isTrainerBossTrainer)
                    species = RandomizeTrainerMon(seed, i, monsCount, species);
            #endif

            if (trainer->doubleBattle == TRUE)
                personalityValue = 0x80;
            else if (trainer->encounterMusic_gender & F_TRAINER_FEMALE)
                personalityValue = 0x78; // Use personality more likely to result in a female Pokémon
            else
                personalityValue = 0x88; // Use personality more likely to result in a male Pokémon

            personalityValue += personalityHash << 8;
            if (partyData[monIndex].gender == TRAINER_MON_MALE)
                personalityValue = (personalityValue & 0xFFFFFF00) | GeneratePersonalityForGender(MON_MALE, partyData[monIndex].species);
            else if (partyData[monIndex].gender == TRAINER_MON_FEMALE)
                personalityValue = (personalityValue & 0xFFFFFF00) | GeneratePersonalityForGender(MON_FEMALE, partyData[monIndex].species);
            else if (partyData[monIndex].gender == TRAINER_MON_RANDOM_GENDER)
                personalityValue = (personalityValue & 0xFFFFFF00) | GeneratePersonalityForGender(Random() & 1 ? MON_MALE : MON_FEMALE, partyData[monIndex].species);
            ModifyPersonalityForNature(&personalityValue, partyData[monIndex].nature);
            if (partyData[monIndex].isShiny)
            {
                otIdType = OT_ID_PRESET;
                fixedOtId = HIHALF(personalityValue) ^ LOHALF(personalityValue);
            }

            CreateMon(&party[i], species, partyData[monIndex].lvl, 0, TRUE, personalityValue, otIdType, fixedOtId);
            SetMonData(&party[i], MON_DATA_HELD_ITEM, &partyData[monIndex].heldItem);

            CustomTrainerPartyAssignMoves(&party[i], &partyData[monIndex]);
            SetMonData(&party[i], MON_DATA_IVS, &(partyData[monIndex].iv));
            if (partyData[monIndex].ev != NULL)
            {
                SetMonData(&party[i], MON_DATA_HP_EV, &(partyData[monIndex].ev[0]));
                SetMonData(&party[i], MON_DATA_ATK_EV, &(partyData[monIndex].ev[1]));
                SetMonData(&party[i], MON_DATA_DEF_EV, &(partyData[monIndex].ev[2]));
                SetMonData(&party[i], MON_DATA_SPATK_EV, &(partyData[monIndex].ev[3]));
                SetMonData(&party[i], MON_DATA_SPDEF_EV, &(partyData[monIndex].ev[4]));
                SetMonData(&party[i], MON_DATA_SPEED_EV, &(partyData[monIndex].ev[5]));
            }
            if (partyData[monIndex].ability != ABILITY_NONE)
            {
                const struct SpeciesInfo *speciesInfo = &gSpeciesInfo[partyData[monIndex].species];
                u32 maxAbilities = ARRAY_COUNT(speciesInfo->abilities);
                for (ability = 0; ability < maxAbilities; ++ability)
                {
                    if (speciesInfo->abilities[ability] == partyData[monIndex].ability)
                        break;
                }
                if (ability >= maxAbilities)
                    ability = 0;
            }
            else if (B_TRAINER_MON_RANDOM_ABILITY)
            {
                const struct SpeciesInfo *speciesInfo = &gSpeciesInfo[partyData[monIndex].species];
                ability = personalityHash % 3;
                while (speciesInfo->abilities[ability] == ABILITY_NONE)
                {
                    ability--;
                }
            }
            SetMonData(&party[i], MON_DATA_ABILITY_NUM, &ability);
            SetMonData(&party[i], MON_DATA_CANT_RANDOMIZE_ABILITY, &isTrainerBossTrainer);
            SetMonData(&party[i], MON_DATA_FRIENDSHIP, &(partyData[monIndex].friendship));
            if (partyData[monIndex].ball != ITEM_NONE)
            {
                ball = partyData[monIndex].ball;
                SetMonData(&party[i], MON_DATA_POKEBALL, &ball);
            }
            if (partyData[monIndex].nickname != NULL)
            {
                SetMonData(&party[i], MON_DATA_NICKNAME, partyData[monIndex].nickname);
            }
            if (partyData[monIndex].isShiny)
            {
                u32 data = TRUE;
                SetMonData(&party[i], MON_DATA_IS_SHINY, &data);
            }
            if (partyData[monIndex].dynamaxLevel > 0)
            {
                u32 data = partyData[monIndex].dynamaxLevel;
                if (partyData[monIndex].shouldUseDynamax)
                    gBattleStruct->opponentMonCanDynamax |= 1 << i;
                SetMonData(&party[i], MON_DATA_DYNAMAX_LEVEL, &data);
            }
            if (partyData[monIndex].gigantamaxFactor)
            {
                u32 data = partyData[monIndex].gigantamaxFactor;
                SetMonData(&party[i], MON_DATA_GIGANTAMAX_FACTOR, &data);
            }
            if (partyData[monIndex].teraType > 0)
            {
                gBattleStruct->opponentMonCanTera |= 1 << i;
                u32 data = partyData[monIndex].teraType;
                SetMonData(&party[i], MON_DATA_TERA_TYPE, &data);
            }
            CalculateMonStats(&party[i]);

            if (B_TRAINER_CLASS_POKE_BALLS >= GEN_7 && ball == -1)
            {
                ball = gTrainerClasses[trainer->trainerClass].ball ?: ITEM_POKE_BALL;
                SetMonData(&party[i], MON_DATA_POKEBALL, &ball);
            }
        }
    }

    return trainer->partySize;
}

static u8 CreateNPCTrainerParty(struct Pokemon *party, u16 trainerNum, bool8 firstTrainer)
{
    u8 retVal;
    if (trainerNum == TRAINER_SECRET_BASE)
        return 0;
    retVal = CreateNPCTrainerPartyFromTrainer(party, GetTrainerStructFromId(trainerNum), firstTrainer, gBattleTypeFlags, trainerNum);
    return retVal;
}

void CreateTrainerPartyForPlayer(void)
{
    Script_RequestEffects(SCREFF_V1);

    ZeroPlayerPartyMons();
    gPartnerTrainerId = gSpecialVar_0x8004;
    CreateNPCTrainerPartyFromTrainer(gPlayerParty, GetTrainerStructFromId(gSpecialVar_0x8004), TRUE, BATTLE_TYPE_TRAINER, gSpecialVar_0x8004);
}
```

### hunk 2 — randomizer side (ours)

```c
                gBattleMons[battler].ability = GetAbilityBySpecies(gBattleMons[battler].species, gBattleMons[battler].abilityNum, gBattleMons[battler].cantRandomizeAbility);
                gBattleStruct->hpOnSwitchout[GetBattlerSide(battler)] = gBattleMons[battler].hp;
                gBattleMons[battler].status2 = 0;
```


## `src/battle_script_commands.c`

### hunk 1 — randomizer side (ours)

```c
    u8 battler, cantRandomizeAbility;
```


## `src/debug.c`

### hunk 1 — randomizer side (ours)

```c
#include "randomizer.h"
```

### hunk 2 — randomizer side (ours)

```c
    DEBUG_MENU_ITEM_UTILITIES,
    DEBUG_MENU_ITEM_PCBAG,
    DEBUG_MENU_ITEM_PARTY,
    DEBUG_MENU_ITEM_GIVE,
    DEBUG_MENU_ITEM_SCRIPTS,
    DEBUG_MENU_ITEM_FLAGVAR,
    //DEBUG_MENU_ITEM_BATTLE,
    DEBUG_MENU_ITEM_SOUND,
    DEBUG_MENU_ITEM_CANCEL,
};

enum UtilDebugMenu
{
    DEBUG_UTIL_MENU_ITEM_FLY,
    DEBUG_UTIL_MENU_ITEM_WARP,
    DEBUG_UTIL_MENU_ITEM_SAVEBLOCK,
    DEBUG_UTIL_MENU_ITEM_ROM_SPACE,
    DEBUG_UTIL_MENU_ITEM_WEATHER,
    DEBUG_UTIL_MENU_ITEM_FONT_TEST,
    DEBUG_UTIL_MENU_ITEM_CHECKWALLCLOCK,
    DEBUG_UTIL_MENU_ITEM_SETWALLCLOCK,
    DEBUG_UTIL_MENU_ITEM_WATCHCREDITS,
    DEBUG_UTIL_MENU_ITEM_PLAYER_NAME,
    DEBUG_UTIL_MENU_ITEM_PLAYER_GENDER,
    DEBUG_UTIL_MENU_ITEM_PLAYER_ID,
    DEBUG_UTIL_MENU_ITEM_CHEAT,
    DEBUG_UTIL_MENU_ITEM_EXPANSION_VER,
    DEBUG_UTIL_MENU_ITEM_BERRY_FUNCTIONS,
    DEBUG_UTIL_MENU_ITEM_EWRAM_COUNTERS,
    #if RANDOMIZER_AVAILABLE == TRUE
    DEBUG_UTIL_MENU_ITEM_RANDOMIZER,
    #endif
    DEBUG_UTIL_MENU_ITEM_STEVEN_MULTI,
};

enum GivePCBagDebugMenu
{
    DEBUG_PCBAG_MENU_ITEM_ACCESS_PC,
    DEBUG_PCBAG_MENU_ITEM_FILL,
    DEBUG_PCBAG_MENU_ITEM_CLEAR_BAG,
    DEBUG_PCBAG_MENU_ITEM_CLEAR_BOXES,
};

enum GivePCBagFillDebugMenu
{
    DEBUG_PCBAG_MENU_ITEM_FILL_PC_BOXES_FAST,
    DEBUG_PCBAG_MENU_ITEM_FILL_PC_BOXES_SLOW,
    DEBUG_PCBAG_MENU_ITEM_FILL_PC_ITEMS,
    DEBUG_PCBAG_MENU_ITEM_FILL_POCKET_ITEMS,
    DEBUG_PCBAG_MENU_ITEM_FILL_POCKET_BALLS,
    DEBUG_PCBAG_MENU_ITEM_FILL_POCKET_TMHM,
    DEBUG_PCBAG_MENU_ITEM_FILL_POCKET_BERRIES,
    DEBUG_PCBAG_MENU_ITEM_FILL_POCKET_KEY_ITEMS,
};

enum PartyDebugMenu
{
    DEBUG_PARTY_MENU_ITEM_MOVE_REMINDER,
    DEBUG_PARTY_MENU_ITEM_HATCH_AN_EGG,
    DEBUG_PARTY_MENU_ITEM_HEAL_PARTY,
    DEBUG_PARTY_MENU_ITEM_INFLICT_STATUS1,
    DEBUG_PARTY_MENU_ITEM_CHECK_EVS,
    DEBUG_PARTY_MENU_ITEM_CHECK_IVS,
    DEBUG_PARTY_MENU_ITEM_CLEAR_PARTY,
};

enum ScriptDebugMenu
{
    DEBUG_UTIL_MENU_ITEM_SCRIPT_1,
    DEBUG_UTIL_MENU_ITEM_SCRIPT_2,
    DEBUG_UTIL_MENU_ITEM_SCRIPT_3,
    DEBUG_UTIL_MENU_ITEM_SCRIPT_4,
    DEBUG_UTIL_MENU_ITEM_SCRIPT_5,
    DEBUG_UTIL_MENU_ITEM_SCRIPT_6,
    DEBUG_UTIL_MENU_ITEM_SCRIPT_7,
    DEBUG_UTIL_MENU_ITEM_SCRIPT_8,
```

### hunk 3 — randomizer side (ours)

```c
        {
            gTasks[taskId].tInput -= sPowersOfTen[gTasks[taskId].tDigit];
            if (gTasks[taskId].tInput < 0)
                gTasks[taskId].tInput = 0;
        }

        StringCopy(gStringVar2, gText_DigitIndicator[gTasks[taskId].tDigit]);
        ConvertIntToDecimalStringN(gStringVar3, gTasks[taskId].tInput, STR_CONV_MODE_LEADING_ZEROS, 2);
        StringCopyPadded(gStringVar3, gStringVar3, CHAR_SPACE, 15);
        StringCopy(gStringVar1, gNaturesInfo[gTasks[taskId].tInput].name);
        StringExpandPlaceholders(gStringVar4, sDebugText_PokemonNature);
        AddTextPrinterParameterized(gTasks[taskId].tSubWindowId, DEBUG_MENU_FONT, gStringVar4, 0, 0, 0, NULL);
    }

    if (JOY_NEW(A_BUTTON))
    {
        u16 abilityId;
        sDebugMonData->nature = gTasks[taskId].tInput;
        gTasks[taskId].tInput = 0;
        gTasks[taskId].tDigit = 0;

        StringCopy(gStringVar2, gText_DigitIndicator[gTasks[taskId].tDigit]);
        ConvertIntToDecimalStringN(gStringVar3, gTasks[taskId].tInput, STR_CONV_MODE_LEADING_ZEROS, 2);
        StringCopyPadded(gStringVar3, gStringVar3, CHAR_SPACE, 15);
        abilityId = GetAbilityBySpecies(sDebugMonData->species, 0, FALSE);
        u8 *end = StringCopy(gStringVar1, gAbilitiesInfo[abilityId].name);
        WrapFontIdToFit(gStringVar1, end, DEBUG_MENU_FONT, WindowWidthPx(gTasks[taskId].tSubWindowId));
        StringExpandPlaceholders(gStringVar4, sDebugText_PokemonAbility);
        AddTextPrinterParameterized(gTasks[taskId].tSubWindowId, DEBUG_MENU_FONT, gStringVar4, 0, 0, 0, NULL);

        gTasks[taskId].func = DebugAction_Give_Pokemon_SelectAbility;
    }
    else if (JOY_NEW(B_BUTTON))
    {
        PlaySE(SE_SELECT);
        Free(sDebugMonData);
        DebugAction_DestroyExtraWindow(taskId);
    }
}

static void DebugAction_Give_Pokemon_SelectAbility(u8 taskId)
{
    u16 abilityId;
    u8 abilityCount = NUM_ABILITY_SLOTS - 1; //-1 for proper iteration
    u8 i = 0;

    if (JOY_NEW(DPAD_ANY))
    {
        PlaySE(SE_SELECT);

        if (JOY_NEW(DPAD_UP))
        {
            gTasks[taskId].tInput += sPowersOfTen[gTasks[taskId].tDigit];
            if (gTasks[taskId].tInput > abilityCount)
                gTasks[taskId].tInput = abilityCount;
        }
        if (JOY_NEW(DPAD_DOWN))
        {
            gTasks[taskId].tInput -= sPowersOfTen[gTasks[taskId].tDigit];
            if (gTasks[taskId].tInput < 0)
                gTasks[taskId].tInput = 0;
        }

        while (GetAbilityBySpecies(sDebugMonData->species, gTasks[taskId].tInput - i, FALSE) == ABILITY_NONE && gTasks[taskId].tInput - i < NUM_ABILITY_SLOTS)
        {
            i++;
        }
        abilityId = GetAbilityBySpecies(sDebugMonData->species, gTasks[taskId].tInput - i, FALSE);
        StringCopy(gStringVar2, gText_DigitIndicator[gTasks[taskId].tDigit]);
        ConvertIntToDecimalStringN(gStringVar3, gTasks[taskId].tInput, STR_CONV_MODE_LEADING_ZEROS, 2);
        StringCopyPadded(gStringVar3, gStringVar3, CHAR_SPACE, 15);
        u8 *end = StringCopy(gStringVar1, gAbilitiesInfo[abilityId].name);
        WrapFontIdToFit(gStringVar1, end, DEBUG_MENU_FONT, WindowWidthPx(gTasks[taskId].tSubWindowId));
        StringExpandPlaceholders(gStringVar4, sDebugText_PokemonAbility);
        AddTextPrinterParameterized(gTasks[taskId].tSubWindowId, DEBUG_MENU_FONT, gStringVar4, 0, 0, 0, NULL);
    }

    if (JOY_NEW(A_BUTTON))
    {
        sDebugMonData->abilityNum = gTasks[taskId].tInput - i;
        gTasks[taskId].tInput = 0;
        gTasks[taskId].tDigit = 0;

        StringCopy(gStringVar2, gText_DigitIndicator[gTasks[taskId].tDigit]);
        ConvertIntToDecimalStringN(gStringVar3, gTasks[taskId].tInput, STR_CONV_MODE_LEADING_ZEROS, 2);
        StringCopyPadded(gStringVar3, gStringVar3, CHAR_SPACE, 15);
        StringCopy(gStringVar1, gTypesInfo[0].name);
        StringExpandPlaceholders(gStringVar4, sDebugText_PokemonTeraType);
        AddTextPrinterParameterized(gTasks[taskId].tSubWindowId, DEBUG_MENU_FONT, gStringVar4, 0, 0, 0, NULL);

        gTasks[taskId].func = DebugAction_Give_Pokemon_SelectTeraType;
    }
    else if (JOY_NEW(B_BUTTON))
    {
        PlaySE(SE_SELECT);
        Free(sDebugMonData);
        DebugAction_DestroyExtraWindow(taskId);
    }
}

static void DebugAction_Give_Pokemon_SelectTeraType(u8 taskId)
{
    if (JOY_NEW(DPAD_ANY))
    {
        PlaySE(SE_SELECT);

        if (JOY_NEW(DPAD_UP))
        {
            gTasks[taskId].tInput += sPowersOfTen[gTasks[taskId].tDigit];
            if (gTasks[taskId].tInput > NUMBER_OF_MON_TYPES - 1)
                gTasks[taskId].tInput = NUMBER_OF_MON_TYPES - 1;
        }
        if (JOY_NEW(DPAD_DOWN))
        {
            gTasks[taskId].tInput -= sPowersOfTen[gTasks[taskId].tDigit];
            if (gTasks[taskId].tInput < 0)
                gTasks[taskId].tInput = 0;
        }

        StringCopy(gStringVar2, gText_DigitIndicator[gTasks[taskId].tDigit]);
        ConvertIntToDecimalStringN(gStringVar3, gTasks[taskId].tInput, STR_CONV_MODE_LEADING_ZEROS, 2);
        StringCopyPadded(gStringVar3, gStringVar3, CHAR_SPACE, 15);
        StringCopy(gStringVar1, gTypesInfo[gTasks[taskId].tInput].name);
        StringExpandPlaceholders(gStringVar4, sDebugText_PokemonTeraType);
        AddTextPrinterParameterized(gTasks[taskId].tSubWindowId, DEBUG_MENU_FONT, gStringVar4, 0, 0, 0, NULL);
    }

    if (JOY_NEW(A_BUTTON))
    {
        sDebugMonData->teraType = gTasks[taskId].tInput;
        gTasks[taskId].tInput = 0;
        gTasks[taskId].tDigit = 0;

        StringCopy(gStringVar2, gText_DigitIndicator[gTasks[taskId].tDigit]);
        ConvertIntToDecimalStringN(gStringVar1, gTasks[taskId].tInput, STR_CONV_MODE_LEADING_ZEROS, 2);
        StringCopyPadded(gStringVar1, gStringVar1, CHAR_SPACE, 15);
        StringExpandPlaceholders(gStringVar4, sDebugText_PokemonDynamaxLevel);
        AddTextPrinterParameterized(gTasks[taskId].tSubWindowId, DEBUG_MENU_FONT, gStringVar4, 0, 0, 0, NULL);

        gTasks[taskId].func = DebugAction_Give_Pokemon_SelectDynamaxLevel;
    }
    else if (JOY_NEW(B_BUTTON))
    {
        PlaySE(SE_SELECT);
        Free(sDebugMonData);
        DebugAction_DestroyExtraWindow(taskId);
    }
}

static void DebugAction_Give_Pokemon_SelectDynamaxLevel(u8 taskId)
{
    if (JOY_NEW(DPAD_ANY))
    {
        PlaySE(SE_SELECT);
        Debug_HandleInput_Numeric(taskId, 0, MAX_DYNAMAX_LEVEL, 2);

        StringCopy(gStringVar2, gText_DigitIndicator[gTasks[taskId].tDigit]);
        ConvertIntToDecimalStringN(gStringVar1, gTasks[taskId].tInput, STR_CONV_MODE_LEADING_ZEROS, 2);
        StringCopyPadded(gStringVar1, gStringVar1, CHAR_SPACE, 15);
        StringExpandPlaceholders(gStringVar4, sDebugText_PokemonDynamaxLevel);
        AddTextPrinterParameterized(gTasks[taskId].tSubWindowId, DEBUG_MENU_FONT, gStringVar4, 0, 0, 0, NULL);
    }

    if (JOY_NEW(A_BUTTON))
    {
        sDebugMonData->dynamaxLevel = gTasks[taskId].tInput;
        gTasks[taskId].tInput = 0;
        gTasks[taskId].tDigit = 0;

        ConvertIntToDecimalStringN(gStringVar3, gTasks[taskId].tInput, STR_CONV_MODE_LEADING_ZEROS, 0);
        StringCopyPadded(gStringVar3, gStringVar3, CHAR_SPACE, 15);
        StringCopyPadded(gStringVar2, sDebugText_False, CHAR_SPACE, 15);
        StringExpandPlaceholders(gStringVar4, sDebugText_PokemonGmaxFactor);
        AddTextPrinterParameterized(gTasks[taskId].tSubWindowId, DEBUG_MENU_FONT, gStringVar4, 0, 0, 0, NULL);

        gTasks[taskId].func = DebugAction_Give_Pokemon_SelectGigantamaxFactor;
    }
    else if (JOY_NEW(B_BUTTON))
    {
        PlaySE(SE_SELECT);
        Free(sDebugMonData);
        FreeMonIconPalettes();
        FreeAndDestroyMonIconSprite(&gSprites[gTasks[taskId].tSpriteId]);
        DebugAction_DestroyExtraWindow(taskId);
    }
}

static void Debug_Display_StatInfo(const u8* text, u32 stat, u32 value, u32 digit, u8 windowId)
{
    StringCopy(gStringVar1, gStatNamesTable[stat]);
    StringCopy(gStringVar2, gText_DigitIndicator[digit]);
    ConvertIntToDecimalStringN(gStringVar3, value, STR_CONV_MODE_LEADING_ZEROS, 2);
    StringCopyPadded(gStringVar3, gStringVar3, CHAR_SPACE, 15);
    StringExpandPlaceholders(gStringVar4, text);
    AddTextPrinterParameterized(windowId, DEBUG_MENU_FONT, gStringVar4, 0, 0, 0, NULL);
}

static void DebugAction_Give_Pokemon_SelectGigantamaxFactor(u8 taskId)
{
    static const u8 *txtStr;

    if (JOY_NEW(DPAD_ANY))
    {
        PlaySE(SE_SELECT);
        gTasks[taskId].tInput ^= JOY_NEW(DPAD_UP | DPAD_DOWN) > 0;
        txtStr = (gTasks[taskId].tInput == TRUE) ? sDebugText_True : sDebugText_False;
        StringCopyPadded(gStringVar2, txtStr, CHAR_SPACE, 15);
        ConvertIntToDecimalStringN(gStringVar3, gTasks[taskId].tInput, STR_CONV_MODE_LEADING_ZEROS, 0);
        StringCopyPadded(gStringVar3, gStringVar3, CHAR_SPACE, 15);
        StringExpandPlaceholders(gStringVar4, sDebugText_PokemonGmaxFactor);
        AddTextPrinterParameterized(gTasks[taskId].tSubWindowId, DEBUG_MENU_FONT, gStringVar4, 0, 0, 0, NULL);
    }

    if (JOY_NEW(A_BUTTON))
    {
        sDebugMonData->gmaxFactor = gTasks[taskId].tInput;
        gTasks[taskId].tInput = 0;
        gTasks[taskId].tDigit = 0;
        Debug_Display_StatInfo(sDebugText_IVs, gTasks[taskId].tIterator, gTasks[taskId].tInput, gTasks[taskId].tDigit, gTasks[taskId].tSubWindowId);
        gTasks[taskId].func = DebugAction_Give_Pokemon_SelectIVs;
    }
    else if (JOY_NEW(B_BUTTON))
    {
        PlaySE(SE_SELECT);
        Free(sDebugMonData);
        DebugAction_DestroyExtraWindow(taskId);
    }
}

static void DebugAction_Give_Pokemon_SelectIVs(u8 taskId)
{
    if (JOY_NEW(DPAD_ANY))
    {
        PlaySE(SE_SELECT);
        Debug_HandleInput_Numeric(taskId, 0, MAX_PER_STAT_IVS, 3);
        Debug_Display_StatInfo(sDebugText_IVs, gTasks[taskId].tIterator, gTasks[taskId].tInput, gTasks[taskId].tDigit, gTasks[taskId].tSubWindowId);
    }

    //If A or B button
    if (JOY_NEW(A_BUTTON))
    {
        // Set IVs for stat
        sDebugMonData->monIVs[gTasks[taskId].tIterator] = gTasks[taskId].tInput;

        //Check if all IVs set
        if (gTasks[taskId].tIterator != NUM_STATS - 1)
        {
            gTasks[taskId].tIterator++;
            gTasks[taskId].tInput = 0;
            gTasks[taskId].tDigit = 0;

            Debug_Display_StatInfo(sDebugText_IVs, gTasks[taskId].tIterator, gTasks[taskId].tInput, gTasks[taskId].tDigit, gTasks[taskId].tSubWindowId);
            gTasks[taskId].func = DebugAction_Give_Pokemon_SelectIVs;
        }
```

### hunk 4 — randomizer side (ours)

```c
        moves[i] = sDebugMonData->monMoves[i];
    }
    for (u32 i = 0; i < NUM_STATS; i++)
    {
        EVs[i] = sDebugMonData->monEVs[i];
        IVs[i] = sDebugMonData->monIVs[i];
    }

    //Nature
    if (nature == NUM_NATURES || nature == 0xFF)
        nature = Random() % NUM_NATURES;
    CreateMonWithNature(&mon, species, level, USE_RANDOM_IVS, nature);

    //Shininess
    SetMonData(&mon, MON_DATA_IS_SHINY, &isShiny);

    // Gigantamax factor
    SetMonData(&mon, MON_DATA_GIGANTAMAX_FACTOR, &gmaxFactor);

    // Dynamax Level
    SetMonData(&mon, MON_DATA_DYNAMAX_LEVEL, &dmaxLevel);

    // tera type
    if (teraType == TYPE_NONE || teraType == TYPE_MYSTERY || teraType >= NUMBER_OF_MON_TYPES)
        teraType = GetTeraTypeFromPersonality(&mon);
    SetMonData(&mon, MON_DATA_TERA_TYPE, &teraType);

    //IVs
    for (i = 0; i < NUM_STATS; i++)
    {
        iv_val = IVs[i];
        if (iv_val != USE_RANDOM_IVS && iv_val != 0xFF)
            SetMonData(&mon, MON_DATA_HP_IV + i, &iv_val);
    }

    //EVs
    for (i = 0; i < NUM_STATS; i++)
    {
        ev_val = EVs[i];
        if (ev_val)
            SetMonData(&mon, MON_DATA_HP_EV + i, &ev_val);
    }

    //Moves
    for (i = 0; i < MAX_MON_MOVES; i++)
    {
        if (moves[i] == MOVE_NONE || moves[i] >= MOVES_COUNT)
            continue;

        SetMonMoveSlot(&mon, moves[i], i);
    }

    //Ability
    if (abilityNum == 0xFF || GetAbilityBySpecies(species, abilityNum, FALSE) == ABILITY_NONE)
    {
        do {
            abilityNum = Random() % NUM_ABILITY_SLOTS;  // includes hidden abilities
        } while (GetAbilityBySpecies(species, abilityNum, FALSE) == ABILITY_NONE);
    }

    SetMonData(&mon, MON_DATA_ABILITY_NUM, &abilityNum);

    //Update mon stats before giving it to the player
    CalculateMonStats(&mon);

    // give player the mon
    SetMonData(&mon, MON_DATA_OT_NAME, gSaveBlock2Ptr->playerName);
    SetMonData(&mon, MON_DATA_OT_GENDER, &gSaveBlock2Ptr->playerGender);
    for (i = 0; i < PARTY_SIZE; i++)
    {
        if (GetMonData(&gPlayerParty[i], MON_DATA_SPECIES, NULL) == SPECIES_NONE)
            break;
    }

    if (i >= PARTY_SIZE)
    {
        sentToPc = CopyMonToPC(&mon);
```


## `src/dexnav.c`

### hunk 1 — randomizer side (ours)

```c
            StringCopy(gStringVar1, gAbilitiesInfo[GetAbilityBySpecies(species, sDexNavSearchDataPtr->abilityNum, FALSE)].name);
            AddTextPrinterParameterized3(windowId, 0, WINDOW_COL_1 + 16, 12, sSearchFontColor, TEXT_SKIP_DRAW, gStringVar1);
```


## `src/match_call.c`

### hunk 1 — randomizer side (ours)

```c
            u16 curSpecies;
            #if RANDOMIZER_AVAILABLE == TRUE
                u8 mapGroup, mapNum;
                mapGroup = gWildMonHeaders[i].mapGroup;
                mapNum = gWildMonHeaders[i].mapNum;
            #endif
```

### hunk 2 — randomizer side (ours)

```c
                slot = GetLandEncounterSlot();
                curSpecies = gWildMonHeaders[i].landMonsInfo->wildPokemon[slot].species;

                #if RANDOMIZER_AVAILABLE == TRUE
                    curSpecies = RandomizeWildEncounter(curSpecies, mapNum, mapGroup,
                    WILD_AREA_LAND, slot);
                #endif

                species[numSpecies] = curSpecies;
```

### hunk 3 — randomizer side (ours)

```c
                slot = GetWaterEncounterSlot();
                curSpecies = gWildMonHeaders[i].waterMonsInfo->wildPokemon[slot].species;

                #if RANDOMIZER_AVAILABLE == TRUE
                    curSpecies = RandomizeWildEncounter(curSpecies, mapNum, mapGroup,
                    WILD_AREA_WATER, slot);
                #endif

                species[numSpecies] = curSpecies;
```


## `src/new_game.c`

### hunk 1 — randomizer side (ours)

```c
#include "randomizer.h"
```

### hunk 2 — randomizer side (ours)

```c
    #if (RANDOMIZER_AVAILABLE == TRUE) && (RANDOMIZER_DYNAMIC_SPECIES == TRUE)
        PreloadRandomizationTables();
    #endif
```


## `src/party_menu.c`

### hunk 1 — randomizer side (ours)

```c
        GetMonNickname(&gPlayerParty[tMonId], gStringVar1);
        StringCopy(gStringVar2, gAbilitiesInfo[GetAbilityBySpecies(tSpecies, tAbilityNum, FALSE)].name);
        StringExpandPlaceholders(gStringVar4, askText);
```

### hunk 2 — randomizer side (ours)

```c
        GetMonNickname(&gPlayerParty[tMonId], gStringVar1);
        StringCopy(gStringVar2, gAbilitiesInfo[GetAbilityBySpecies(tSpecies, tAbilityNum, FALSE)].name);
        StringExpandPlaceholders(gStringVar4, askText);
```


## `src/pokedex_area_screen.c`

### hunk 1 — randomizer side (ours)

```c
#include "randomizer.h"
```

### hunk 2 — randomizer side (ours)

```c
static u16 GetRegionMapSectionId(u8, u8);
static bool8 MapHasSpecies(const struct WildPokemonHeader *, u16);
static bool8 MonListHasSpecies(const struct WildPokemonHeader *, u16, enum WildArea);
```

### hunk 3 — randomizer side (ours)

```c
    if (MonListHasSpecies(info, species, WILD_AREA_LAND))
        return TRUE;
    if (MonListHasSpecies(info, species, WILD_AREA_WATER))
        return TRUE;
    if (MonListHasSpecies(info, species, WILD_AREA_FISHING))
        return TRUE;
    if (MonListHasSpecies(info, species, WILD_AREA_ROCKS))
```

### hunk 4 — randomizer side (ours)

```c
static bool8 MonListHasSpecies(const struct WildPokemonHeader *header, u16 species, enum WildArea area)
```


## `src/pokemon.c`

### hunk 1 — randomizer side (ours)

```c
u16 GetAbilityBySpecies(u16 species, u8 abilityNum, u8 cantRandomizeAbility)
```

### hunk 2 — randomizer side (ours)

```c
    u16 species = GetMonData(mon, MON_DATA_SPECIES, NULL);
    u8 abilityNum = GetMonData(mon, MON_DATA_ABILITY_NUM, NULL);
    u8 cantRandomizeAbility = GetMonData(mon, MON_DATA_CANT_RANDOMIZE_ABILITY, NULL);
    return GetAbilityBySpecies(species, abilityNum, cantRandomizeAbility);
```

### hunk 3 — randomizer side (ours)

```c
    dst->species = GetMonData(src, MON_DATA_SPECIES, NULL);
    dst->item = GetMonData(src, MON_DATA_HELD_ITEM, NULL);
    dst->ppBonuses = GetMonData(src, MON_DATA_PP_BONUSES, NULL);
    dst->friendship = GetMonData(src, MON_DATA_FRIENDSHIP, NULL);
    dst->experience = GetMonData(src, MON_DATA_EXP, NULL);
    dst->hpIV = GetMonData(src, MON_DATA_HP_IV, NULL);
    dst->attackIV = GetMonData(src, MON_DATA_ATK_IV, NULL);
    dst->defenseIV = GetMonData(src, MON_DATA_DEF_IV, NULL);
    dst->speedIV = GetMonData(src, MON_DATA_SPEED_IV, NULL);
    dst->spAttackIV = GetMonData(src, MON_DATA_SPATK_IV, NULL);
    dst->spDefenseIV = GetMonData(src, MON_DATA_SPDEF_IV, NULL);
    dst->personality = GetMonData(src, MON_DATA_PERSONALITY, NULL);
    dst->status1 = GetMonData(src, MON_DATA_STATUS, NULL);
    dst->level = GetMonData(src, MON_DATA_LEVEL, NULL);
    dst->hp = GetMonData(src, MON_DATA_HP, NULL);
    dst->maxHP = GetMonData(src, MON_DATA_MAX_HP, NULL);
    dst->attack = GetMonData(src, MON_DATA_ATK, NULL);
    dst->defense = GetMonData(src, MON_DATA_DEF, NULL);
    dst->speed = GetMonData(src, MON_DATA_SPEED, NULL);
    dst->spAttack = GetMonData(src, MON_DATA_SPATK, NULL);
    dst->spDefense = GetMonData(src, MON_DATA_SPDEF, NULL);
    dst->abilityNum = GetMonData(src, MON_DATA_ABILITY_NUM, NULL);
    dst->cantRandomizeAbility = GetMonData(src, MON_DATA_CANT_RANDOMIZE_ABILITY, NULL);
    dst->otId = GetMonData(src, MON_DATA_OT_ID, NULL);
    dst->types[0] = gSpeciesInfo[dst->species].types[0];
    dst->types[1] = gSpeciesInfo[dst->species].types[1];
    dst->types[2] = TYPE_MYSTERY;
    dst->isShiny = IsMonShiny(src);
    dst->ability = GetAbilityBySpecies(dst->species, dst->abilityNum, dst->cantRandomizeAbility);
```

### hunk 4 — randomizer side (ours)

```c
        heldItem = GetBoxMonData(boxMon, MON_DATA_HELD_ITEM, NULL);
        ability = GetAbilityBySpecies(species, GetBoxMonData(boxMon, MON_DATA_ABILITY_NUM, NULL), GetBoxMonData(boxMon, MON_DATA_CANT_RANDOMIZE_ABILITY, NULL));
```


## `src/pokemon_summary_screen.c`

### hunk 1 — randomizer side (ours)

```c
    u16 ability = GetAbilityBySpecies(sMonSummaryScreen->summary.species, sMonSummaryScreen->summary.abilityNum, FALSE);
```

### hunk 2 — randomizer side (ours)

```c
    u16 ability = GetAbilityBySpecies(sMonSummaryScreen->summary.species, sMonSummaryScreen->summary.abilityNum, FALSE);
    PrintTextOnWindow(AddWindowFromTemplateList(sPageInfoTemplate, PSS_DATA_WINDOW_INFO_ABILITY), gAbilitiesInfo[ability].description, 0, 17, 0, 0);
```


## `src/scrcmd.c`

### hunk 1 — randomizer side (ours)

```c
#include "randomizer.h"
```


## `src/script_pokemon_util.c`

### hunk 1 — randomizer side (ours)

```c

    // create a Pokémon with basic data
    if ((gender == MON_MALE && genderRatio != MON_FEMALE && genderRatio != MON_GENDERLESS)
     || (gender == MON_FEMALE && genderRatio != MON_MALE && genderRatio != MON_GENDERLESS)
     || (gender == MON_GENDERLESS && genderRatio == MON_GENDERLESS))
        CreateMonWithGenderNatureLetter(&mon, species, level, 32, gender, nature, 0);
    else
        CreateMonWithNature(&mon, species, level, 32, nature);

    // shininess
    if (P_FLAG_FORCE_SHINY != 0 && FlagGet(P_FLAG_FORCE_SHINY))
        isShiny = TRUE;
    else if (P_FLAG_FORCE_NO_SHINY != 0 && FlagGet(P_FLAG_FORCE_NO_SHINY))
        isShiny = FALSE;
    SetMonData(&mon, MON_DATA_IS_SHINY, &isShiny);

    // gigantamax factor
    SetMonData(&mon, MON_DATA_GIGANTAMAX_FACTOR, &gmaxFactor);

    // Dynamax Level
    SetMonData(&mon, MON_DATA_DYNAMAX_LEVEL, &dmaxLevel);

    // tera type
    if (teraType == TYPE_NONE || teraType == TYPE_MYSTERY || teraType >= NUMBER_OF_MON_TYPES)
        teraType = GetTeraTypeFromPersonality(&mon);
    SetMonData(&mon, MON_DATA_TERA_TYPE, &teraType);

    // EV and IV
    for (i = 0; i < NUM_STATS; i++)
    {
        // EV
        if (evs[i] <= MAX_PER_STAT_EVS)
            SetMonData(&mon, MON_DATA_HP_EV + i, &evs[i]);

        // IV
        if (ivs[i] <= MAX_PER_STAT_IVS)
            SetMonData(&mon, MON_DATA_HP_IV + i, &ivs[i]);
    }
    CalculateMonStats(&mon);

    // moves
    for (i = 0; i < MAX_MON_MOVES; i++)
    {
        if (moves[0] == MOVE_NONE)
            break;
        if (moves[i] >= MOVES_COUNT)
            continue;
        SetMonMoveSlot(&mon, moves[i], i);
    }

    // ability
    if (abilityNum == NUM_ABILITY_PERSONALITY)
    {
        abilityNum = GetMonData(&mon, MON_DATA_PERSONALITY) & 1;
    }
    else if (abilityNum > NUM_NORMAL_ABILITY_SLOTS || GetAbilityBySpecies(species, abilityNum, FALSE) == ABILITY_NONE)
    {
        do {
            abilityNum = Random() % NUM_ABILITY_SLOTS; // includes hidden abilities
        } while (GetAbilityBySpecies(species, abilityNum, FALSE) == ABILITY_NONE);
    }
    SetMonData(&mon, MON_DATA_ABILITY_NUM, &abilityNum);

    // ball
    if (ball > POKEBALL_COUNT)
        ball = BALL_POKE;
    SetMonData(&mon, MON_DATA_POKEBALL, &ball);

    // held item
    SetMonData(&mon, MON_DATA_HELD_ITEM, &item);

    // In case a mon with a form changing item is given. Eg: SPECIES_ARCEUS_NORMAL with ITEM_SPLASH_PLATE will transform into SPECIES_ARCEUS_WATER upon gifted.
    targetSpecies = GetFormChangeTargetSpecies(&mon, FORM_CHANGE_ITEM_HOLD, 0);
    if (targetSpecies != SPECIES_NONE)
        SetMonData(&mon, MON_DATA_SPECIES, &targetSpecies);

    // assign OT name and gender
    SetMonData(&mon, MON_DATA_OT_NAME, gSaveBlock2Ptr->playerName);
    SetMonData(&mon, MON_DATA_OT_GENDER, &gSaveBlock2Ptr->playerGender);

    if (slot < PARTY_SIZE)
    {
        if (side == 0)
            CopyMon(&gPlayerParty[slot], &mon, sizeof(struct Pokemon));
        else
            CopyMon(&gEnemyParty[slot], &mon, sizeof(struct Pokemon));
        sentToPc = MON_GIVEN_TO_PARTY;
    }
    else
    {
        // find empty party slot to decide whether the Pokémon goes to the Player's party or the storage system.
        for (i = 0; i < PARTY_SIZE; i++)
        {
            if (GetMonData(&gPlayerParty[i], MON_DATA_SPECIES, NULL) == SPECIES_NONE)
                break;
        }
        if (i >= PARTY_SIZE)
        {
            sentToPc = CopyMonToPC(&mon);
        }
        else
        {
            sentToPc = MON_GIVEN_TO_PARTY;
            CopyMon(&gPlayerParty[i], &mon, sizeof(mon));
            gPlayerPartyCount = i + 1;
        }
    }

    if (side == 0)
    {
        // set pokédex flags
        nationalDexNum = SpeciesToNationalPokedexNum(species);
        if (sentToPc != MON_CANT_GIVE)
        {
            GetSetPokedexFlag(nationalDexNum, FLAG_SET_SEEN);
            GetSetPokedexFlag(nationalDexNum, FLAG_SET_CAUGHT);
        }
    }

    return sentToPc;
```


## `src/wild_encounter.c`

### hunk 1 — randomizer side (ours)

```c
#define WILD_CHECK_REPEL    (1 << 0)
#define WILD_CHECK_KEEN_EYE (1 << 1)

#define HEADER_NONE 0xFFFF
```

### hunk 2 — randomizer side (ours)

```c
static u8 GetMaxLevelOfSpeciesInWildTable(const struct WildPokemon *wildMon, u16 species, enum WildArea area);
```

### hunk 3 — randomizer side (ours)

```c
static u8 ChooseWildMonLevel(const struct WildPokemon *wildPokemon, u8 wildMonIndex, enum WildArea area)
```

### hunk 4 — randomizer side (ours)

```c
static bool8 TryGenerateWildMon(const struct WildPokemonInfo *wildMonInfo, enum WildArea area, u8 flags)
```

### hunk 5 — randomizer side (ours)

```c
    u16 headerId, species;
    u8 index;
```

### hunk 6 — randomizer side (ours)

```c
        index = ChooseWildMonIndex_WaterRock();
        species = waterMonsInfo->wildPokemon[index].species;
```

### hunk 7 — randomizer side (ours)

```c
        index = ChooseWildMonIndex_WaterRock();
        species = waterMonsInfo->wildPokemon[index].species;
```

### hunk 8 — randomizer side (ours)

```c
        {
            u8 index;
            u16 species;
            index = ChooseWildMonIndex_WaterRock();
            species = waterMonsInfo->wildPokemon[index].species;
            #if RANDOMIZER_AVAILABLE == TRUE
                species = RandomizeWildEncounter(
                    species, gWildMonHeaders[headerId].mapNum,
                    gWildMonHeaders[headerId].mapGroup, WILD_AREA_WATER,
                    index);
            #endif

            return species;
        }
```

### hunk 9 — randomizer side (ours)

```c
static u8 GetMaxLevelOfSpeciesInWildTable(const struct WildPokemon *wildMon, u16 species, enum WildArea area)
```


## `test/battle/trainer_control.c`

### hunk 1 — randomizer side (ours)

```c
    CreateNPCTrainerPartyFromTrainer(testParty, &sTestTrainers[GetTrainerDifficultyLevelTest(currTrainer)][currTrainer], TRUE, BATTLE_TYPE_TRAINER, 0);
```

### hunk 2 — randomizer side (ours)

```c
<<<<<<< HEAD
    CreateNPCTrainerPartyFromTrainer(testParty, &sTestTrainers[difficulty][0], TRUE, BATTLE_TYPE_TRAINER);
```

### hunk 3 — randomizer side (ours)

```c
    CreateNPCTrainerPartyFromTrainer(testParty, &sTestTrainer2, TRUE, BATTLE_TYPE_TRAINER, 0);
```


## `tools/trainerproc/main.c`

### hunk 1 — randomizer side (ours)

```c
    bool boss;
    int boss_line;

    struct String starting_status;
```

### hunk 2 — randomizer side (ours)

```c
        if (trainer->boss_line)
        {
            fprintf(f, "#line %d\n", trainer->boss_line);
            fprintf(f, "        .isBossTrainer = ");
            fprint_bool(f, trainer->boss);
            fprintf(f, ",\n");
        }

        if (!is_empty_string(trainer->starting_status))
```

