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

// --- Field moves ------------------------------------------------------------

// If TRUE, HM/field moves can be used out of battle without a party Pokemon that
// knows the move. Randolocke pairs this with P_CAN_FORGET_HIDDEN_MOVE so HM slaves
// are unnecessary. Badge requirements still apply.
#define RANDOLOCKE_FIELD_MOVES_NEED_NO_USER  TRUE

// --- Catching ---------------------------------------------------------------

// Percentage applied to a species' base catch rate. 100 is unchanged; Randolocke
// "moderately increased wild catch rates".
#define RANDOLOCKE_CATCH_RATE_PERCENT        150

// --- Move relearner ---------------------------------------------------------

// If TRUE, the move relearner's battle-move panel shows the Pokemon's Attack and Sp. Atk
// EVs in place of the "BATTLE MOVES" heading, so a physical/special choice can be made
// without leaving the screen. The panel has no spare row, hence the swap rather than an
// extra line; the heading is redundant when you are already looking at the move list.
#define RANDOLOCKE_RELEARNER_SHOW_EVS   TRUE

// --- Registered key items ---------------------------------------------------

// If TRUE, a second key item can be registered and used by *holding* SELECT, while a tap
// still uses the first. Registering pushes the previous first item into the second slot,
// so two registrations fill both without any new bag UI.
// WARNING: this adds a field to SaveBlock1 and therefore changes the save layout.
#define RANDOLOCKE_DUAL_REGISTERED_ITEMS  TRUE

// Frames SELECT must be held before the second item fires, at 60fps.
#define RANDOLOCKE_SELECT_HOLD_FRAMES     20

// --- Regi caves -------------------------------------------------------------

// If TRUE, Flash replaces the Braille puzzles that gate the Regis. Used anywhere in the
// relevant room rather than only on the Braille tile:
//   Sealed Chamber outer room -> opens the door to the inner room (was Dig)
//   Sealed Chamber inner room -> opens the three Regi caves (was Relicanth + Wailord)
//   Desert Ruins              -> opens Regirock's wall (was Rock Smash)
//   Island Cave               -> opens Regice's wall (was the walk-the-perimeter puzzle)
//   Ancient Tomb              -> already Flash in vanilla Emerald, left alone
// The Braille puzzles still work; this is an additional way in. A randomized run cannot
// rely on having a Relicanth or a Wailord, which is what makes the vanilla gate unfair.
#define RANDOLOCKE_FLASH_OPENS_REGI_CAVES   TRUE

// --- Nuzlocke options -------------------------------------------------------

// If TRUE, catching a Pokemon goes straight to the naming screen instead of asking. A
// nuzlocke convention: naming a Pokemon is what makes losing it mean something.
#define RANDOLOCKE_FORCE_NICKNAME       TRUE

#endif // GUARD_CONFIG_RANDOLOCKE_H
