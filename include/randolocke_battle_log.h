#ifndef GUARD_RANDOLOCKE_BATTLE_LOG_H
#define GUARD_RANDOLOCKE_BATTLE_LOG_H

// The battle's messages, kept so SELECT at the battle menu can show them again. See
// RANDOLOCKE_BATTLE_LOG in include/config/randolocke.h.

void RandolockeBattleLog_Reset(void);
void RandolockeBattleLog_BeginTurn(void);
void RandolockeBattleLog_AddMessage(const u8 *message);
void RandolockeBattleLog_AddAbility(u32 battler, u32 ability);

// The replay, one text box at a time: a heading, then the messages from before the first
// turn; a heading, then the messages since the last turn began.
u32 RandolockeBattleLog_ReplayLength(void);
const u8 *RandolockeBattleLog_ReplayLine(u32 index);

#endif // GUARD_RANDOLOCKE_BATTLE_LOG_H
