#ifndef GUARD_RANDOLOCKE_NUZLOCKE_H
#define GUARD_RANDOLOCKE_NUZLOCKE_H

#include "config/randolocke.h"
#include "constants/randolocke.h"

#define RANDOLOCKE_NO_AREA  0xFFFFFFFF

enum RandolockeCatchRule
{
    RANDOLOCKE_CATCH_OK,
    RANDOLOCKE_CATCH_AREA_USED,   // something was already caught here
    RANDOLOCKE_CATCH_DUPE,        // this evolution family is already registered as caught
};

#if RANDOLOCKE_NUZLOCKE_RULES == TRUE
bool32 RandolockeNuzlockeActive(void);
bool32 RandolockeMonIsDead(struct BoxPokemon *boxMon);
bool32 RandolockeDeadMonsAreLocked(void);
void RandolockeBoxWipedParty(void);
bool32 RandolockeAnyLivingMonInBoxes(void);
u32 RandolockeCurrentArea(void);
bool32 RandolockeAreaUsed(u32 area);
enum RandolockeCatchRule RandolockeCatchRuleForBattle(void);
void RandolockeNoteCatch(struct Pokemon *mon);
#else
static inline enum RandolockeCatchRule RandolockeCatchRuleForBattle(void) { return RANDOLOCKE_CATCH_OK; }
static inline void RandolockeNoteCatch(struct Pokemon *mon) { (void)mon; }
static inline bool32 RandolockeMonIsDead(struct BoxPokemon *boxMon) { (void)boxMon; return FALSE; }
static inline bool32 RandolockeDeadMonsAreLocked(void) { return FALSE; }
static inline void RandolockeBoxWipedParty(void) {}
static inline bool32 RandolockeAnyLivingMonInBoxes(void) { return TRUE; }
#endif

#endif // GUARD_RANDOLOCKE_NUZLOCKE_H
