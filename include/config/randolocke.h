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
// Turned off: the two-slot behaviour did not work reliably in play -- registering a
// Porta Heal would not take. Back to the base game's single registered item until the
// pokeemerald_rando_enh version (separate tap/hold icons, and a prompt explaining the
// hold) can be ported properly.
#define RANDOLOCKE_DUAL_REGISTERED_ITEMS  TRUE

// Frames SELECT must be held before the second item fires, at 60fps.
// A third of a second was far too twitchy at fast-forward speeds -- a normal tap read
// as a hold. One second, matching pokeemerald_rando_enh.
#define RANDOLOCKE_SELECT_HOLD_FRAMES     60

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

// If TRUE, catching a Pokemon goes straight to the naming screen instead of asking.
// A nuzlocke convention, but conventions are not rules: left FALSE so catching behaves
// like the base game and asks. Nothing else depends on it.
#define RANDOLOCKE_FORCE_NICKNAME       FALSE

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

// --- Summary screen stat editor ----------------------------------------------

// If TRUE, the summary screen's IV and EV pages can be edited in place: press SELECT to
// start, A to move between stats, and the D-pad to change the one you are on --
//   Up    jump to the maximum        Left   one lower
//   Down  jump to zero               Right  one higher
// EVs are held to 252 per stat and 510 in total, IVs to 31; a change that would break
// either is simply refused. Party Pokemon only, since a boxed one has no stats to
// recalculate. This is the readable version of the debug menu's stat editors.
#define RANDOLOCKE_SUMMARY_STAT_EDITOR  TRUE

// --- Player Pokemon quality ---------------------------------------------------

// How good the IVs are on a Pokemon that becomes yours -- caught, gifted, a starter or
// hatched. Trainers are never affected.
//   RANDOLOCKE_IVS_PERFECT   31 across the board. In a run where a Pokemon usually cannot
//                            be re-caught, IV luck is a tax rather than a decision
//   RANDOLOCKE_IVS_RANDOLOCKE  3 guaranteed perfect IVs on starters and gifts, random
//                            everywhere else. Randolocke's own behaviour
//   RANDOLOCKE_IVS_VANILLA   untouched
#define RANDOLOCKE_IVS_VANILLA      0
#define RANDOLOCKE_IVS_RANDOLOCKE   1
#define RANDOLOCKE_IVS_PERFECT      2

#define RANDOLOCKE_PLAYER_IVS       RANDOLOCKE_IVS_PERFECT

// How many IVs a starter or gift is guaranteed under RANDOLOCKE_IVS_RANDOLOCKE.
#define RANDOLOCKE_GIFT_PERFECT_IVS 3

// --- Berries ------------------------------------------------------------------

// Multiplier on how many berries a tree gives when harvested. Randomized berry trees mean
// you rarely get the one you wanted twice, so a bigger handful of each is worth more than
// a second trip. Capped at 255, which is the field's limit.
#define RANDOLOCKE_BERRY_YIELD_MULTIPLIER   4

// --- Friendship heart ---------------------------------------------------------

// If TRUE, the summary's info page shows a heart under the Pokemon's picture that fills
// from the bottom as friendship rises, and turns gold only at MAX_FRIENDSHIP, so a gold
// heart means the value cannot go higher. Graphic ported from pokeemerald_rando_enh.
#define RANDOLOCKE_FRIENDSHIP_HEART         TRUE

// --- First-encounter badge ---------------------------------------------------

// If TRUE, a circled 1 is drawn on a wild Pokemon's health box when catching it would be
// a legal nuzlocke catch here: the area is unused, the family is new, or it is shiny. The
// rules are enforced either way -- this just stops you throwing a ball to find out.
#define RANDOLOCKE_FIRST_ENCOUNTER_BADGE    TRUE

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

// The rules do not apply until Birch hands over the five Poke Balls, which is what
// FLAG_ADVENTURE_STARTED marks: after the Route 103 rival battle and the walk back to the
// lab. Before that you have one Pokemon and no balls, so there is nothing to rule on.
#define RANDOLOCKE_FLAG_RULES_BEGIN         FLAG_ADVENTURE_STARTED

// A wipe -- every Pokemon in the party down -- costs you the whole party. They are boxed
// and marked, and cannot be withdrawn, moved or shifted again; you go to the PC and pick
// a new team out of whatever is left. Individual faints are just faints: heal and carry
// on. Held items come back to the bag on the way, because losing those as well is
// attrition rather than drama. Becoming Champion ends the run and lifts the lock.
#define RANDOLOCKE_WIPE_COSTS_PARTY         TRUE

// Where the "this one is gone" mark is stored. Substruct 3 is exactly full at 96 bits, so
// there is no room for a new field without changing the size of every boxed Pokemon and
// invalidating saves. marineRibbon is never distributed in Emerald, so it is free.
#define RANDOLOCKE_MON_DATA_FAINTED         MON_DATA_MARINE_RIBBON

// If nothing living is left in any box after a wipe, the run is over and the game returns
// to the title screen. The save is never deleted: load it and you are standing at the last
// Pokemon Center with an empty party. Restarting is the player's call, not the game's.
#define RANDOLOCKE_RUN_OVER_ON_WIPE         TRUE
#define RANDOLOCKE_FLAG_NUZLOCKE_OFF        FLAG_UNUSED_0x02D

// --- Key item delivery -------------------------------------------------------

// The four key items are fully implemented but nothing in the game ever handed them out.
// Each now has exactly one source, and both work on a save that already exists:
//   Cap Candy, Repellant       the old man in Oldale Town, once the adventure has started
//   Porta Heal, Endless Candy  the boy by the Littleroot pond

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
#define RANDOLOCKE_DEFAULT_TUTOR_MOVES      TRUE   // which move each move tutor teaches

// How species are substituted. Also only applied to new games; the debug menu writes the
// same var (RANDOMIZER_VAR_SPECIES_MODE).
//   MON_RANDOM              anything can become anything. Randolocke's own behaviour
//   MON_RANDOM_LEGEND_AWARE legendaries only ever replace other legendaries
//   MON_RANDOM_BST          replacements have a similar base stat total
//   MON_EVOLUTION           replacements sit at the same evolution stage
// Randolocke substitutes a Pokemon of similar base stat total, which is what keeps the
// early game survivable: a Route 101 Zigzagoon can no longer roll into Rayquaza. The
// twelve legendary sites ignore this and use MON_RANDOM_LEGEND_AWARE regardless, so they
// still hand out legendaries.
#define RANDOLOCKE_DEFAULT_SPECIES_MODE     MON_RANDOM_BST

// Text speed a new save starts on. The stock default is OPTIONS_TEXT_SPEED_MID. A
// randomized run reads a great deal of unfamiliar text -- every species, every move on
// every trainer -- so it starts on FAST instead. Still changeable in the options menu.
#define RANDOLOCKE_DEFAULT_TEXT_SPEED       OPTIONS_TEXT_SPEED_FAST

// Press SELECT on the "which move should be forgotten?" screen to swap the Pokemon's
// picture for its six stats and its ability. Randomized learnsets hand out physical and
// special moves indiscriminately, so deciding which of four attacks to keep needs to know
// whether this Pokemon actually hits harder with Attack or with Sp. Attack -- and that
// screen is reached from a level-up, with no way back to the skills page.
#define RANDOLOCKE_MOVE_SCREEN_STATS        TRUE

// A TM/HM's bag description is the description of the move it actually teaches. A
// randomized TM keeps its own printed description otherwise, which describes whatever
// move it taught in the base game -- the stats panel and the teaching flow already show
// the real move, so only the description disagreed. Move descriptions are written for the
// summary screen's wider window and get re-wrapped to fit.
#define RANDOLOCKE_TM_MOVE_DESCRIPTIONS     TRUE

// Items handed over by NPCs are randomized the same way item balls are -- the man in
// Rustboro hands over something other than a Quick Claw. HMs and key items are never
// touched (ShouldRandomizeItem rejects them), so nothing the story needs can be lost.
#define RANDOLOCKE_RANDOMIZE_NPC_GIFTS      TRUE

// Poke Balls given by NPCs are left alone even so. The five from the rival on Route 103
// are what start the run, and nuzlocke rules make balls the scarcest resource in the
// game; turning them into a random item would be the single most punishing roll in it.
// Set TRUE to let them be randomized like anything else.
#define RANDOLOCKE_RANDOMIZE_NPC_GIFT_BALLS FALSE

// Re-rolling a Pokemon's hidden nature and its ability, from the summary screen:
//   Pokemon Info page   SELECT  hidden nature -- the one CalculateMonStats reads, so the
//                               one that moves the numbers. The Trainer Memo shows it as
//                               "Naive (Modest) nature,".
//                       START   ability, printed two lines above.
//   Pokemon Skills page SELECT  ability, so it can be rolled while looking at the stats
//                               it has to suit. Only in the plain stats view -- the IV and
//                               EV views keep SELECT for the stat editor.
// Separate buttons so a spread can be aimed at: fix the nature you want, then work on the
// ability, without one undoing the other. The debug menu's Roll Hidden Nature still works.
// Party Pokemon only, never an egg, never a Frontier rental.
#define RANDOLOCKE_SUMMARY_NATURE_ROLL      TRUE
#define RANDOLOCKE_SUMMARY_ABILITY_ROLL     TRUE

// What one roll costs. 0 by default: this is meant to be a cheat, not an economy. Set it
// to a price if you would rather rolling were a decision -- though the Oldale NPC hands
// over P999,999, so any price is a brake rather than a wall.
#define RANDOLOCKE_ROLL_COST                0

#endif // GUARD_CONFIG_RANDOLOCKE_H
