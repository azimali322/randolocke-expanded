#include "global.h"
#include "braille_puzzles.h"
#include "event_data.h"
#include "fldeff.h"
#include "party_menu.h"
#include "script.h"
#include "task.h"
#include "test/test.h"
#include "constants/flags.h"
#include "constants/map_groups.h"

// Flash's stand-ins for the Braille puzzles run as gPostMenuFieldCallback: straight off
// the party menu, after a fade, with the player's field controls locked. Whatever one
// does, it has to hand the player back, the way the vanilla puzzle effects end with
// UnlockPlayerFieldControls and UnfreezeObjectEvents. The Sealed Chamber's door opened
// and then the game stopped, because the callback did neither.
static void PutPlayerOnMap(u16 map)
{
    gSaveBlock1Ptr->location.mapGroup = map >> 8;
    gSaveBlock1Ptr->location.mapNum = map & 0xFF;
}

TEST("Randolocke: Flash's Regi shortcuts hand the player back")
{
    // The Sealed Chamber's door, which vanilla opens from a script that releases after.
    PutPlayerOnMap(MAP_SEALED_CHAMBER_OUTER_ROOM);
    FlagClear(FLAG_SYS_BRAILLE_DIG);
    LockPlayerFieldControls();
    EXPECT(SetUpFieldMove_Flash());
    EXPECT(gPostMenuFieldCallback == RandolockeOpenSealedChamberDoor);
    gPostMenuFieldCallback();
    EXPECT(FlagGet(FLAG_SYS_BRAILLE_DIG));
    EXPECT(!ArePlayerFieldControlsLocked());

    // Regice's wall.
    PutPlayerOnMap(MAP_ISLAND_CAVE);
    FlagClear(FLAG_SYS_BRAILLE_REGICE_COMPLETED);
    LockPlayerFieldControls();
    EXPECT(SetUpFieldMove_Flash());
    EXPECT(gPostMenuFieldCallback == RandolockeOpenRegiceWall);
    gPostMenuFieldCallback();
    EXPECT(FlagGet(FLAG_SYS_BRAILLE_REGICE_COMPLETED));
    EXPECT(!ArePlayerFieldControlsLocked());

    // The three Regi caves. This one shakes the room first, and the shaking effect ends by
    // resuming the script that started it -- which also locks the player, and off the party
    // menu there is no such script. It runs as a task, so give it frames to finish.
    PutPlayerOnMap(MAP_SEALED_CHAMBER_INNER_ROOM);
    FlagClear(FLAG_REGI_DOORS_OPENED);
    LockPlayerFieldControls();
    EXPECT(SetUpFieldMove_Flash());
    EXPECT(gPostMenuFieldCallback == RandolockeOpenRegiDoors);
    gPostMenuFieldCallback();
    EXPECT(FlagGet(FLAG_REGI_DOORS_OPENED));
    for (u32 i = 0; i < 120 && ArePlayerFieldControlsLocked(); i++)
        RunTasks();
    EXPECT(!ArePlayerFieldControlsLocked());
}
