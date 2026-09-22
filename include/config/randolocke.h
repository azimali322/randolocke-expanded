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

// If TRUE, the badge-gated field moves -- Cut, Flash, Rock Smash, Strength, Surf, Fly,
// Dive, Waterfall -- work without any party Pokemon knowing them, so no HM slave is
// needed and no move slot is spent on an HM. The badge is still required for each.
//
// Most of them are reached by walking into the thing they work on, which goes through
// ScrCmd_checkfieldmove; when nobody knows the move it hands the script the lead Pokemon
// to name and animate. Fly and Flash have no such trigger, so they are offered in the
// party menu's field move list instead, whether or not the Pokemon knows them.
//
// The always-unlocked field moves are deliberately untouched: Teleport, Dig, Sweet Scent,
// Soft-Boiled, Milk Drink and Secret Power are ordinary moves a Pokemon has to have
// earned, not infrastructure the game blocks progress behind.
#define RANDOLOCKE_FIELD_MOVES_NEED_NO_USER  TRUE

// If TRUE, nowhere is dark: the Flash caves -- Granite Cave B1F and B2F, Victory Road B1F
// and B2F -- and Dewford Gym, which vanilla lights one trainer at a time, are fully lit
// from the moment you walk in. Flash no longer lights a cave, because there is nothing
// left to light; it still opens the Regi chambers (see Regi caves below).
#define RANDOLOCKE_NO_DARK_AREAS             TRUE

// --- Catching ---------------------------------------------------------------

// Percentage applied to a species' base catch rate. 100 is unchanged; Randolocke
// "moderately increased wild catch rates".
#define RANDOLOCKE_CATCH_RATE_PERCENT        150

// If TRUE, the Bag cannot be opened in a battle against a trainer: no Potions, no Revives,
// no Full Restores mid-fight, held items only. Wild battles are untouched, which they have
// to be -- the same check gates Poke Balls, so blocking the Bag there would mean never
// catching anything again.
//
// This reads the config directly rather than B_VAR_NO_BAG_USE, which a new game clears, so
// it applies to a save already in progress.
#define RANDOLOCKE_NO_BAG_VS_TRAINERS        TRUE

// The catch rate a legendary, mythical or Ultra Beast is caught at, in place of its own.
// 0 leaves every species on its own rate. This is the final rate, so it does not move when
// RANDOLOCKE_CATCH_RATE_PERCENT above does.
//
// 105 of the 136 species this covers sit at the floor of 3, which after the percentage
// above is 4 -- against 67 for the commonest wild Pokemon and 112 for the median one, so
// 17 to 28 times harder to catch, or about ninety Ultra Balls at a quarter health. At 45
// that is 1.5 to 2.5 times harder, around six balls: still the hardest thing on the route
// without being a different game.
//
// One flat rate rather than a multiplier because the 136 do not start level. Twenty-three
// of them are already at 30, 45 or 255 -- Mew, Celebi, the Ultra Beasts, Eternatus,
// Terapagos -- and multiplying those lands past the 255 cap, which is a guaranteed catch
// with any ball at full health. A flat rate does mean those twenty-three get *harder*
// than they are now: set RANDOLOCKE_LEGENDARY_CATCH_RATE_IS_FLOOR to TRUE to only ever
// raise a rate, which leaves those twenty-three exactly as they are.
#define RANDOLOCKE_LEGENDARY_CATCH_RATE          45
#define RANDOLOCKE_LEGENDARY_CATCH_RATE_IS_FLOOR FALSE

// --- Move relearner ---------------------------------------------------------

// If TRUE, the move relearner's battle-move panel shows the Pokemon's Attack and Sp. Atk
// EVs in place of the "BATTLE MOVES" heading, so a physical/special choice can be made
// without leaving the screen. The panel has no spare row, hence the swap rather than an
// extra line; the heading is redundant when you are already looking at the move list.
#define RANDOLOCKE_RELEARNER_SHOW_EVS   TRUE

// --- Move tutors --------------------------------------------------------------

// The ten tutors in the towns teach once each in vanilla: a flag is set when you accept,
// the offer never comes again, and the game warns you before you spend it. Ten moves for
// the whole run, and in a randomized one you do not get to choose which ten.
//
// If TRUE, they teach as often as you like -- the flag is neither checked nor set, and the
// "this move can only be learned once" confirmation is skipped with it. Each tutor still
// teaches its own randomized move, the same one every time, so what changes is only how
// many of your Pokemon can have it. A save that already spent some of them is unaffected
// by the flags it has already set, since nothing reads them any more.
//
// The Battle Frontier's two tutors are a separate script and still charge BP per move.
#define RANDOLOCKE_REPEATABLE_MOVE_TUTORS   TRUE

// --- The Elite Four ----------------------------------------------------------

// If TRUE, the League will not let you through to the Elite Four with more than
// RANDOLOCKE_ELITE_FOUR_MAX_LEGENDARIES legendaries in the party -- the same 136 species the
// legendary clause and the legendary catch rate cover: restricted legendaries,
// sub-legendaries, mythicals and Ultra Beasts. Eggs do not count.
//
// Checked on the two tiles in front of the door rather than by the guards, because the
// guards only step aside once: after the first visit you walk straight past them, and a
// check that lived in their script would miss every attempt after a loss. The Pokemon
// Center and its PC are in the same room, so a party that is refused can be fixed on the
// spot.
#define RANDOLOCKE_ELITE_FOUR_LEGENDARY_LIMIT   TRUE
#define RANDOLOCKE_ELITE_FOUR_MAX_LEGENDARIES   1

// --- Bike ----------------------------------------------------------------------

// One bike, both bikes. Rydel hands over a single BIKE, and pressing R while riding
// switches it between Mach and Acro on the spot -- no trip back to Mauville to trade.
// Ported from pokeemerald_rando_enh's "bike combined".
//
// The switch runs the same transition as getting on, so the sprite, the avatar state and
// the bike's momentum all reset together. Both bike items are renamed BIKE, so a save that
// already holds the Acro Bike keeps it and it behaves identically: R switches whichever
// one you are riding.
#define RANDOLOCKE_DUAL_BIKE                    TRUE

// --- Story shortcuts ----------------------------------------------------------

// If TRUE, the first morning in Littleroot skips its errands. Mom meets you at the truck,
// sets the clock herself -- to the cartridge's real-time clock, which in an emulator is
// your own -- hands over the Running Shoes, and sends you to Professor Birch. The bedroom
// clock, Dad on TV and the visit next door are skipped, and the states they would have
// set are set, so the rest of the story finds what it expects: the rival counts as met,
// which is what the twin at the north exit waits for, and May or Brendan is still waiting
// on Route 103 for the battle.
//
// The Running Shoes come early, so the lab moves straight past the scene where Mom would
// wait outside to give them after the Pokedex. The rival's name is the game's default,
// MAY or BRENDAN, as it always is in Emerald.
//
// Only affects new games: a save already past the truck never comes back to it.
#define RANDOLOCKE_QUICK_START              TRUE

// If TRUE, there is no Wally catching tutorial. Vanilla makes it effectively mandatory:
// until it runs, a boy at Petalburg's west side walks you back to the gym every time you
// cross his row. The first time you enter Petalburg, the state the tutorial leaves behind
// is set instead -- Wally and his mother hidden, Norman already asking for four badges --
// so the gym boy never stops you and the gym is optional until you want the badge.
// Wally's later appearances do not depend on the tutorial and are unaffected.
#define RANDOLOCKE_SKIP_WALLY_TUTORIAL      TRUE

// --- Battle log --------------------------------------------------------------

// If TRUE, tapping SELECT at the battle menu (FIGHT / BAG / POKEMON / RUN) replays the
// battle's messages in the text box: everything said before the first turn -- the send-
// outs, and the abilities that fired on entry, named, since the pop-up that names them is
// gone before you can choose -- then everything since the last turn began. A goes to the
// next message, B or SELECT closes it. The debug ROM's battle debug menu, which was on
// SELECT, moves to a hold of it (RANDOLOCKE_SELECT_HOLD_FRAMES).
#define RANDOLOCKE_BATTLE_LOG               TRUE

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

// If TRUE, the three Regi caves -- Desert Ruins on Route 111, Island Cave on Route 105 and
// the Ancient Tomb on Route 120 -- are open once you hold the eighth badge, with no visit
// to the Sealed Chamber. The first time one of those routes loads after the badge, the
// doors are opened just as the Sealed Chamber opens them, so everything that asks whether
// they are open agrees. Inside, each cave still keeps its Regi behind its own wall, opened
// by its puzzle or by Flash as above. Before the eighth badge, the Sealed Chamber still
// opens them as usual.
#define RANDOLOCKE_REGI_CAVES_OPEN_AT_BADGE_8  TRUE

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

// --- Friendship readout -------------------------------------------------------

// The Pokemon's friendship, as a plain "X/255" on the skills page, in the bottom-right of
// its picture where the filling heart used to be.
//
// The heart is gone. It was a seven-frame graphic that filled from the bottom and turned
// gold at maximum, and across three rounds of playtesting it was never legible: too small
// at 8x8, still ambiguous at 16x16 over the Pokemon's own sprite, and the gold frame read
// as a wedge rather than a heart until its outline was darkened. A number cannot be
// misread, and it answers the question the heart could only gesture at -- how far along
// the value actually is.
#define RANDOLOCKE_SKILLS_PAGE_FRIENDSHIP   TRUE

// --- First-encounter badge ---------------------------------------------------

// If TRUE, a circled 1 is drawn on a wild Pokemon's health box when catching it would be
// a legal nuzlocke catch here: the area is unused, the family is new, or it is shiny. The
// rules are enforced either way -- this just stops you throwing a ball to find out.
#define RANDOLOCKE_FIRST_ENCOUNTER_BADGE    TRUE

// --- Nuzlocke rules ----------------------------------------------------------

// If TRUE the classic rules are enforced in-game rather than left to the player:
//
//   One per area   The first Pokemon you meet in a wild-encounter area is your one chance
//                  there. Catch it, or the area is used; after that, balls are refused.
//                  With RANDOLOCKE_FIRST_ENCOUNTER_COUNTS it is used however that battle
//                  ends -- see there. Without it, only a catch uses the area.
//   Dupes clause   A species whose evolution family you have already caught cannot be
//                  caught again -- and meeting one does not use up the area, so you can
//                  keep looking for something new.
//   Shiny clause   A shiny is always catchable and never uses up the area.
//
// An "area" is one region map section: a route, a town, a cave however many floors it has.
// Land, surfing and fishing there share it. Places with no wild encounter table at all --
// the legendary sites, gift Pokemon, most scripted battles -- are not areas and are never
// restricted.
//
// Set RANDOLOCKE_FLAG_NUZLOCKE_OFF in the debug menu to switch the rules off for a save.
// The flag is inverted deliberately: a save made before this existed has it clear, so the
// rules are on there too, with no new game needed.
#define RANDOLOCKE_NUZLOCKE_RULES           TRUE

// If TRUE, the first encounter in an area uses it up however the battle ends: caught,
// knocked out, run from, gone by Teleport, Roar or Whirlwind, fled on its own, or a loss.
// Only the clauses spare the area -- a shiny, a legendary, or a Pokemon whose evolution
// family is already caught, which is how running from a dupe leaves you free to keep
// looking. FALSE counts a catch only, so a knockout or a flee means another try.
#define RANDOLOCKE_FIRST_ENCOUNTER_COUNTS   TRUE

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

// The nuzlocke death rule: a Pokemon that reaches 0 HP is boxed at the end of the battle
// and locked there for the rest of the run, instead of being walked off at a Pokemon
// Center. Its held item comes off first and goes back to the bag, so the run does not
// lose the item along with the holder, and so nothing valuable ends up locked in a box.
//
// Skipped where the party is not really the player's or cannot lose a Pokemon: Birch's
// bag on Route 101, the Wally catching tutorial, Safari, link and recorded battles, an
// in-game partner's team, and the Frontier.
#define RANDOLOCKE_FAINT_COSTS_MON          TRUE

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

// --- Fishing -----------------------------------------------------------------

// Easy fishing, ported from Modern Emerald's "EASIER FISHING" option: the rod reels
// itself in. Once something bites the catch is yours -- there is no reaction window to
// miss, so "It got away!" can no longer happen, and a stray A press during the dots no
// longer cancels the cast. One round of dots is always enough too: the extra rounds that
// can send a Super Rod through six of them are skipped.
//
// What it does not do is hand out a bite. Whether anything bites at all is still the roll
// in Fishing_CheckForBite -- I_FISHING_BITE_ODDS, 25/50/75% by rod -- so "Not even a
// nibble..." is still the usual answer to a bad cast. This only removes the part of
// fishing that tests the player's thumb instead of their luck.
#define RANDOLOCKE_EASY_FISHING             TRUE

// How long "Oh! A bite!" stays up before the rod reels itself in, in frames. Pressing A
// reels in immediately, so this is only the wait for a player who doesn't.
#define RANDOLOCKE_EASY_FISHING_REEL_DELAY  24

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

// The skills page shows the Pokemon's ability where the ribbon count used to be. The
// count was never worth the space, and in this hack it is actively misleading: a ribbon
// bit is what marks a Pokemon as having fainted under nuzlocke rules, so the number counts
// something the player never earned. Randomized abilities, meanwhile, matter constantly --
// and this is the page where the stats an ability has to suit are displayed.
#define RANDOLOCKE_SKILLS_PAGE_ABILITY      TRUE

// Re-rolling a Pokemon's hidden nature and its ability, from the summary screen. Both work
// the same way on the Pokemon Info page and on the Pokemon Skills page, so there is no
// need to remember which page does what:
//   SELECT  hidden nature -- the one CalculateMonStats reads, so the one that moves the
//           numbers. The info page's Trainer Memo shows it as "Naive (Modest) nature,";
//           on the skills page the raised and lowered stats recolour immediately.
//   START   ability, printed on both pages.
// On the skills page this applies to the plain stats view only -- the IV and EV views keep
// SELECT for the stat editor. Separate buttons so a spread can be aimed at: fix the nature
// you want, then work on the ability, without one undoing the other. The debug menu's
// Roll Hidden Nature still works. Party Pokemon only, never an egg, never a rental.
#define RANDOLOCKE_SUMMARY_NATURE_ROLL      TRUE
#define RANDOLOCKE_SUMMARY_ABILITY_ROLL     TRUE

// Frames the roll button has to be held before it fires, and the button has to be
// released before it will fire again. A bare tap used to be enough, on the two screens
// the player visits most and with START sitting next to SELECT -- so a stray press
// silently replaced a Pokemon's ability, which is exactly what the "abilities stay as
// caught" rule is supposed to prevent. 20 frames is a third of a second: enough to stop
// an accident, cheap enough to repeat when aiming for a spread. 0 restores the tap.
#define RANDOLOCKE_ROLL_HOLD_FRAMES         20

// What one roll costs. 0 by default: this is meant to be a cheat, not an economy. Set it
// to a price if you would rather rolling were a decision -- though the Oldale NPC hands
// over P999,999, so any price is a brake rather than a wall.
#define RANDOLOCKE_ROLL_COST                0

// --- TM, HM and tutor compatibility -----------------------------------------

// If TRUE, any Pokemon can be taught any TM, HM or tutor move.
//
// This is not only a convenience. Randomized TMs and tutors draw from the whole move
// pool, while a species' teachable learnset lists the moves it can learn -- so a TM
// reassigned to a move that appears on no teachable list is a TM no Pokemon in the game
// can use. Compatibility was already meaningless besides: every species shares one
// teachable list here, since make_teachables.py forces ALL_TEACHABLES. All this does is
// finish the job and hand the choice of what to teach back to the player.
//
// Scoped to the teaching path -- TMs, HMs, tutors, and the "ABLE / NOT ABLE" line in the
// party menu. The move relearner, Egg move inheritance, the AI's Illusion guess and the
// Battle Frontier keep the real learnsets, because those read them to decide what a
// species plausibly has rather than what the player may choose to give it.
#define RANDOLOCKE_UNIVERSAL_TM_COMPATIBILITY   TRUE

// --- TM pickups --------------------------------------------------------------

// If TRUE, a randomized TM -- found on the ground, hidden, or handed over by an NPC such
// as a gym leader -- is drawn from the TMs the player does not already own, rather than
// from all of them.
//
// TMs are reusable under I_REUSABLE_TMS, so a second copy of one is not a lesser prize,
// it is nothing at all. And because randomized TM assignment makes the vanilla TM tiers
// meaningless, the TM *item* is drawn uniformly, so the odds of a repeat climb with every
// TM collected: past forty of them, better than a third of gym rewards were already in
// the bag.
//
// The player's PC counts as owned too, so depositing a TM cannot be used to make it
// drawable again. When every TM is already owned the draw falls back to the plain one.
#define RANDOLOCKE_TM_PICKUPS_NO_DUPES      TRUE

// The legendary clause. A legendary met in the wild can always be caught, on the same
// terms as the shiny clause: it is allowed even in an area already used up and even if
// its family is already registered, and catching it does not consume the area.
//
// Species randomization can drop a legendary into any route's encounter table, and being
// told "you already caught something here" while a Rayquaza stands in front of you is the
// kind of moment a run is remembered for. The twelve legendary sites are unaffected --
// those maps have no wild encounter table, so they were never area-gated to begin with.
//
// Counts the same four categories the randomizer's own legend-aware substitution does:
// restricted legendaries, sub-legendaries, mythicals and Ultra Beasts.
#define RANDOLOCKE_LEGENDARY_CLAUSE         TRUE

// --- Debug ROM stability -----------------------------------------------------

// The debug ROM -- `make`, not `make release` -- keeps the old AGB_ASSERT checks, and a
// failed one ends in a break opcode meant to stop the game for a debugger. mGBA has no
// debugger attached for a player, so its BIOS returns from the opcode two bytes early:
// into the second half of the `bl MgbaPrintf` just before it, with a stale link register.
// The CPU lands a few kilobytes away in unrelated code, and whatever happens next -- more
// asserts, a freeze -- is fallout from that one failure. The release ROM compiles every
// AGB_ASSERT out and carries on.
//
// If TRUE, a failed AGB_ASSERT in the debug ROM is logged exactly as before and then play
// carries on, which is what the release ROM already does minus the log line. Test builds
// are unaffected: a failed assert still fails the test.
#define RANDOLOCKE_DEBUG_ASSERTS_RESUME     TRUE

// If TRUE, Free() refuses a pointer it can tell is wrong -- a block that is already free,
// or one whose header lacks the allocator's magic number -- and logs it instead of
// asserting. Skipping is the safe response to both: a block already free is already
// accounted for, and a header without the magic number cannot be trusted to walk. The
// log line names the function that called Free(), as an address to look up with
// `arm-none-eabi-addr2line -f -e pokeemerald.elf`, and for a block freed twice, the file
// and line that allocated it. Applies to both ROMs; only the debug ROM prints.
#define RANDOLOCKE_SKIP_BAD_FREES           TRUE

#endif // GUARD_CONFIG_RANDOLOCKE_H
