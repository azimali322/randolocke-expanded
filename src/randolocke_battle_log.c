#include "global.h"
#include "config/randolocke.h"
#include "battle.h"
#include "battle_message.h"
#include "pokemon.h"
#include "string_util.h"
#include "randolocke_battle_log.h"
#include "constants/characters.h"

#if RANDOLOCKE_BATTLE_LOG == TRUE

// Messages are kept back to back, each ending in EOS: first the ones from before the first
// turn, then the ones since the current turn began. The turn before that is dropped as
// each turn begins, so the space only has to hold two turns' worth, not a whole battle's.
#define LOG_SIZE        2048
#define MESSAGE_MAX     254

struct RandolockeBattleLog
{
    u8 text[LOG_SIZE];
    u16 used;
    u16 introEnd;
    bool8 introSealed;
    bool8 introOverflow;
    bool8 turnOverflow;
};

struct LogSection
{
    const u8 *heading;
    u16 start;
    u16 end;
    u16 count;
    bool8 overflow;
};

static EWRAM_DATA struct RandolockeBattleLog sLog = {0};
static EWRAM_DATA u8 sReplayLine[MESSAGE_MAX + 2] = {0};

static const u8 sText_StartOfBattle[] = _("The start of the battle:");
static const u8 sText_LastTurn[] = _("The last turn:");
static const u8 sText_Overflow[] = _("…and more than the log\ncould hold.");
static const u8 sText_Empty[] = _("");
static const u8 sText_AbilityOwner[] = _("{B_SCR_NAME_WITH_PREFIX}'s\n");
static const u8 sText_Exclamation[] = _("!");

void RandolockeBattleLog_Reset(void)
{
    sLog.used = 0;
    sLog.introEnd = 0;
    sLog.introSealed = FALSE;
    sLog.introOverflow = FALSE;
    sLog.turnOverflow = FALSE;
}

// Called as the chosen actions start to play out. The first time, everything so far is the
// start of the battle; after that, the previous turn is no longer the last one.
void RandolockeBattleLog_BeginTurn(void)
{
    if (!sLog.introSealed)
    {
        sLog.introSealed = TRUE;
        sLog.introEnd = sLog.used;
    }
    sLog.used = sLog.introEnd;
    sLog.turnOverflow = FALSE;
}

void RandolockeBattleLog_AddMessage(const u8 *message)
{
    u32 length = StringLength(message);

    // A message that ends on a paragraph break waits for a press when it is first shown.
    // The replay gives every message one, so drop it here rather than wait twice.
    if (length != 0 && message[length - 1] == CHAR_PROMPT_CLEAR)
        length--;
    if (length == 0)
        return;

    if (length > MESSAGE_MAX || sLog.used + length + 1 > LOG_SIZE)
    {
        if (sLog.introSealed)
            sLog.turnOverflow = TRUE;
        else
            sLog.introOverflow = TRUE;
        return;
    }

    memcpy(&sLog.text[sLog.used], message, length);
    sLog.used += length;
    sLog.text[sLog.used++] = EOS;
}

// The ability pop-up names the ability, and the message after it usually does not: under
// Gen 5+ text Drizzle is just "It started to rain!". So the pop-up gets a line of its own,
// named the way the battle names the Pokemon -- Illusion included.
void RandolockeBattleLog_AddAbility(u32 battler, u32 ability)
{
    u8 line[64];
    enum BattlerId scriptingBattler = gBattleScripting.battler;
    u8 *end;

    gBattleScripting.battler = battler;
    BattleStringExpandPlaceholders(sText_AbilityOwner, line, sizeof(line));
    gBattleScripting.battler = scriptingBattler;

    end = line + StringLength(line);
    end = StringCopy(end, gAbilitiesInfo[ability].name);
    StringCopy(end, sText_Exclamation);
    RandolockeBattleLog_AddMessage(line);
}

static u32 CountMessages(u32 start, u32 end)
{
    u32 count = 0;

    for (; start < end; start++)
    {
        if (sLog.text[start] == EOS)
            count++;
    }
    return count;
}

static u32 GetSections(struct LogSection *sections)
{
    u32 introEnd = sLog.introSealed ? sLog.introEnd : sLog.used;
    u32 numSections = 0;

    sections[numSections].heading = sText_StartOfBattle;
    sections[numSections].start = 0;
    sections[numSections].end = introEnd;
    sections[numSections].count = CountMessages(0, introEnd);
    sections[numSections].overflow = sLog.introOverflow;
    if (sections[numSections].count != 0)
        numSections++;

    if (sLog.introSealed)
    {
        sections[numSections].heading = sText_LastTurn;
        sections[numSections].start = introEnd;
        sections[numSections].end = sLog.used;
        sections[numSections].count = CountMessages(introEnd, sLog.used);
        sections[numSections].overflow = sLog.turnOverflow;
        if (sections[numSections].count != 0)
            numSections++;
    }
    return numSections;
}

u32 RandolockeBattleLog_ReplayLength(void)
{
    struct LogSection sections[2];
    u32 numSections = GetSections(sections);
    u32 length = 0, i;

    for (i = 0; i < numSections; i++)
        length += 1 + sections[i].count + sections[i].overflow;
    return length;
}

// Each line ends in a paragraph break, so the text box shows its arrow and waits for A.
const u8 *RandolockeBattleLog_ReplayLine(u32 index)
{
    struct LogSection sections[2];
    u32 numSections = GetSections(sections);
    const u8 *line = NULL;
    u32 i;
    u8 *end;

    for (i = 0; i < numSections && line == NULL; i++)
    {
        if (index == 0)
        {
            line = sections[i].heading;
            break;
        }
        index--;
        if (index < sections[i].count)
        {
            line = &sLog.text[sections[i].start];
            while (index-- != 0)
                line += StringLength(line) + 1;
            break;
        }
        index -= sections[i].count;
        if (sections[i].overflow)
        {
            if (index == 0)
            {
                line = sText_Overflow;
                break;
            }
            index--;
        }
    }

    if (line == NULL)
        line = sText_Empty;
    end = StringCopy(sReplayLine, line);
    end[0] = CHAR_PROMPT_CLEAR;
    end[1] = EOS;
    return sReplayLine;
}

#endif // RANDOLOCKE_BATTLE_LOG
