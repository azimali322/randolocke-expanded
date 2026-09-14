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

// --- Legendaries ------------------------------------------------------------

// If TRUE, every legendary encounter in the game draws from the legendary pool, and does
// so *without replacement*: the twelve sites in gLegendaryMonTable are assigned twelve
// different legendaries, so beating Rayquaza's slot and then Registeel's gives two
// distinct Pokemon. Without this each site rolls independently, so with
// RANDOLOCKE_DEFAULT_SPECIES_MODE at MON_RANDOM a legendary site can hand you a Zigzagoon,
// and two sites can hand you the same species.
//
// This is deliberately independent of the species mode. The mode governs ordinary
// encounters; legendary *sites* stay legendary either way, which is what makes them worth
// travelling to.
#define RANDOLOCKE_UNIQUE_LEGENDARIES   TRUE

// --- Nuzlocke rules ----------------------------------------------------------

// If TRUE the classic rules are enforced in-game rather than left to the player:
//
//   One per area   You may catch one Pokemon per wild-encounter area. After that, balls
//                  are refused there.
//   Dupes clause   A species whose evolution family you have already caught cannot be
//                  caught again -- and meeting one does not use up the area, so you can
//                  keep looking for something new.
//   Shiny clause   A shiny is always catchable and never uses up the area.
//
// "Area" is one entry in the wild encounter tables, which is one map. Places with no
// wild table -- the legendary sites, gift Pokemon, scripted battles -- are not areas and
// are never restricted.
//
// Set RANDOLOCKE_FLAG_NUZLOCKE_OFF in the debug menu to switch the rules off for a save.
// The flag is inverted deliberately: a save made before this existed has it clear, so the
// rules are on there too, with no new game needed.
#define RANDOLOCKE_NUZLOCKE_RULES           TRUE

// Permadeath. A Pokemon that faints is boxed, marked, and cannot be withdrawn, moved or
// shifted again until you are Champion -- at which point the run is over and they come
// back. It can still be released, so the box can be tidied. Its held item is returned to
// the bag on the way, because losing the item too is punishing without being interesting.
#define RANDOLOCKE_PERMADEATH               TRUE

// Where the "this one is gone" mark is stored. Substruct 3 is exactly full at 96 bits, so
// there is no room for a new field without changing the size of every boxed Pokemon and
// invalidating saves. marineRibbon is never distributed in Emerald, so it is free.
#define RANDOLOCKE_MON_DATA_FAINTED         MON_DATA_MARINE_RIBBON

// What happens when the whole party is down.
//   If any living Pokemon is left in a box, the first one is moved into the party and the
//   usual white-out happens. If there are none, the run is over: the game returns to the
//   title screen. The save is never deleted -- load it and you are standing at the last
//   Pokemon Center with an empty party, which is a record of the run, not a playable one.
#define RANDOLOCKE_RUN_OVER_ON_WIPE         TRUE
#define RANDOLOCKE_FLAG_NUZLOCKE_OFF        FLAG_UNUSED_0x02D

// --- Key item delivery -------------------------------------------------------

// The four key items are fully implemented but nothing in the game ever handed them out.
// A new game now starts with all four in the bag. For a save that already exists, the
// boy by the Littleroot pond gives you whichever ones you are missing.
#define RANDOLOCKE_START_WITH_KEY_ITEMS     TRUE

// --- Terrain -----------------------------------------------------------------

// Randolocke moves the Old Rod fisherman from Dewford to Route 103, so fishing is
// available before the first badge. Rather than delete the Dewford object -- object local
// IDs are positional, and Mr. Briney's boat scene depends on the ones after it -- the
// object is kept and hidden behind this flag, which a new game sets. An existing save
// keeps its Dewford fisherman; he is harmless, his script already handles "you have one".
#define RANDOLOCKE_FLAG_HIDE_DEWFORD_OLD_ROD_FISHERMAN  FLAG_UNUSED_0x02C

// --- v1.1 NPCs --------------------------------------------------------------

// Set once the Oldale financier has handed over his money, so he only does it once.
#define RANDOLOCKE_FLAG_OLDALE_MONEY_GIVEN  FLAG_UNUSED_0x02B

// Badge the Slateport harbour map seller waits for. Randolocke's other legendary
// unlocks land after Sootopolis, which is the eighth badge.
#define RANDOLOCKE_MAP_SELLER_BADGE         FLAG_BADGE08_GET

// What each "legendary location map" costs. They are the four event tickets, which are
// otherwise unobtainable without Mystery Gift; the price is nominal on purpose.
#define RANDOLOCKE_MAP_PRICE                1

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
