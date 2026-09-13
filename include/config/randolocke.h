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

// --- Bag TM panel -----------------------------------------------------------

// 1.17 already shows a TM's type, power, accuracy and PP, but only once the item is
// selected. If TRUE the panel follows the cursor instead, so you can compare TMs by
// scrolling, and a physical/special icon is drawn beside the type. Ported from
// pokeemerald_rando_enh, where randomized TMs made this close to essential.
#define RANDOLOCKE_TM_HOVER_INFO        TRUE

// --- New game defaults ------------------------------------------------------

// Randolocke is meant to be randomized out of the box, but tertu's randomizer defaults
// every feature to OFF and expects the debug menu to switch them on. Rather than force
// them on at compile time (FORCE_RANDOMIZE_* in config/randomizer.h), which would remove
// the debug toggles that TESTING.md relies on, a new game sets the flags below. They stay
// flags: switchable in-game, per save, at any time.
//
// Only affects NEW games. An existing save keeps whatever flags it already has.
#define RANDOLOCKE_RANDOMIZE_ON_NEW_GAME    TRUE

// Which features a new game turns on. Set any of these to FALSE to start with that one
// off; the debug menu can still turn it on later.
#define RANDOLOCKE_DEFAULT_WILD_MON         TRUE   // wild encounters
#define RANDOLOCKE_DEFAULT_TRAINER_MON      TRUE   // trainer parties
#define RANDOLOCKE_DEFAULT_FIXED_MON        TRUE   // scripted/static encounters and legendaries
#define RANDOLOCKE_DEFAULT_STARTER_GIFT_MON TRUE   // starters, gift Pokemon, the Lati eon
#define RANDOLOCKE_DEFAULT_EGG_MON          TRUE   // eggs
#define RANDOLOCKE_DEFAULT_ABILITIES        TRUE   // abilities
#define RANDOLOCKE_DEFAULT_FIELD_ITEMS      TRUE   // overworld item balls and hidden items
#define RANDOLOCKE_DEFAULT_LEARNSET         TRUE   // the 21-move level-up learnsets
#define RANDOLOCKE_DEFAULT_BERRY_TREES      TRUE   // what grows on berry trees
#define RANDOLOCKE_DEFAULT_TM_MOVES         TRUE   // which move each TM teaches

// How species are substituted. Also only applied to new games; the debug menu writes the
// same var (RANDOMIZER_VAR_SPECIES_MODE).
//   MON_RANDOM              anything can become anything. Randolocke's own behaviour
//   MON_RANDOM_LEGEND_AWARE legendaries only ever replace other legendaries
//   MON_RANDOM_BST          replacements have a similar base stat total
//   MON_EVOLUTION           replacements sit at the same evolution stage
#define RANDOLOCKE_DEFAULT_SPECIES_MODE     MON_RANDOM

#endif // GUARD_CONFIG_RANDOLOCKE_H
