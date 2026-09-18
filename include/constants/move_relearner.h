#ifndef GUARD_CONSTANTS_MOVE_RELEARNER_H
#define GUARD_CONSTANTS_MOVE_RELEARNER_H

// Max number of moves shown by the move relearner.
// Increased from 25 to 60 so Mew can display all TMs/HMs.
// If you plan on adding more TMs, increase this number too.
//
// randolocke: 64. With P_ENABLE_ALL_LEVEL_UP_MOVES and P_PRE_EVO_MOVES the level-up list
// is every move of every stage of the family, and the randomizer gives each stage its own
// 21-move learnset (RZ_LEARNSET_SLOTS), so a three-stage family can list 3 x 21 = 63
// moves that barely overlap. Measured, not guessed: test/randolocke_relearner.c finds
// 8-12 families over 60 for a given seed and a longest list of 63. At 60 the extra moves
// ran off the end of movesToLearn and menuItems and over the relearner's own bookkeeping.
#define MAX_RELEARNER_MOVES 64

// Move Relearner menu change constants
enum MoveRelearnerStates
{
    MOVE_RELEARNER_LEVEL_UP_MOVES,
    MOVE_RELEARNER_EGG_MOVES,
    MOVE_RELEARNER_TM_MOVES,
    MOVE_RELEARNER_TUTOR_MOVES,
    MOVE_RELEARNER_COUNT,
};

enum RelearnMode
{
    RELEARN_MODE_NONE = 0,
    RELEARN_MODE_SCRIPT = 1,                     // Relearning moves through an event script
    // These two must stay 2 and 3, they are tied to the summary screen pages
    RELEARN_MODE_PSS_PAGE_BATTLE_MOVES = 2,      // Relearning moves through the summary screen's battle moves page
    RELEARN_MODE_PSS_PAGE_CONTEST_MOVES = 3,     // Relearning moves through the summary screen's contest moves page (defaults to contest page on relearner screen)
};

#endif // GUARD_CONSTANTS_MOVE_RELEARNER_H
