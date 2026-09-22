#ifndef GUARD_RANDOLOCKE_NUZLOCKE_H
#define GUARD_RANDOLOCKE_NUZLOCKE_H

#include "config/randolocke.h"
#include "constants/randolocke.h"

#define RANDOLOCKE_NO_AREA  0xFFFFFFFF

#if RANDOLOCKE_PLAYER_IVS != RANDOLOCKE_IVS_VANILLA
void RandolockeSetPlayerMonIVs(struct Pokemon *mon, bool32 isGiftOrStarter);
#else
static inline void RandolockeSetPlayerMonIVs(struct Pokemon *mon, bool32 isGiftOrStarter)
{ (void)mon; (void)isGiftOrStarter; }
#endif

bool32 RandolockeSpeciesIsLegendary(enum Species species);
void RandolockeCheckEliteFourLegendaries(void);
void RandolockeAutoSetClock(void);

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
void RandolockeBoxFaintedMons(void);
void RandolockeBoxFaintedMonsFromField(void);
bool32 RandolockeAnyLivingMonInBoxes(void);
u32 RandolockeCurrentArea(void);
bool32 RandolockeAreaUsed(u32 area);
enum RandolockeCatchRule RandolockeCatchRuleForBattle(void);
enum RandolockeCatchRule RandolockeCatchRuleForBattler(enum BattlerId battler);
bool32 RandolockeEncounterIsFirst(enum BattlerId battler);
void RandolockeNoteCatch(struct Pokemon *mon);
#else
static inline enum RandolockeCatchRule RandolockeCatchRuleForBattle(void) { return RANDOLOCKE_CATCH_OK; }
static inline enum RandolockeCatchRule RandolockeCatchRuleForBattler(enum BattlerId battler) { (void)battler; return RANDOLOCKE_CATCH_OK; }
static inline bool32 RandolockeEncounterIsFirst(enum BattlerId battler) { (void)battler; return FALSE; }
static inline void RandolockeNoteCatch(struct Pokemon *mon) { (void)mon; }
static inline bool32 RandolockeMonIsDead(struct BoxPokemon *boxMon) { (void)boxMon; return FALSE; }
static inline bool32 RandolockeDeadMonsAreLocked(void) { return FALSE; }
static inline void RandolockeBoxWipedParty(void) {}
static inline void RandolockeBoxFaintedMons(void) {}
static inline void RandolockeBoxFaintedMonsFromField(void) {}
static inline bool32 RandolockeAnyLivingMonInBoxes(void) { return TRUE; }
#endif

#endif // GUARD_RANDOLOCKE_NUZLOCKE_H
