#ifndef GUARD_CONFIG_RANDOLOCKE_H
#define GUARD_CONFIG_RANDOLOCKE_H

// Randolocke-specific configuration.
// See RANDOLOCKE_PLAN.md for the feature list these belong to.

// --- Custom key items -------------------------------------------------------

// Flag set while the Repellant key item's effect is active. While set, the repel
// step counter never ticks down, giving a permanent repel that can be toggled off.
#define RANDOLOCKE_FLAG_INFINITE_REPEL      FLAG_UNUSED_0x027

// If TRUE, the Porta Heal also revives fainted Pokémon. Randolocke v1.1 made
// "does not revive" the default, with reviving as the optional behaviour.
#define RANDOLOCKE_PORTA_HEAL_REVIVES       FALSE

#endif // GUARD_CONFIG_RANDOLOCKE_H
