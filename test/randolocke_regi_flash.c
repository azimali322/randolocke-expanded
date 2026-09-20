#include "global.h"
#include "braille_puzzles.h"
#include "event_data.h"
#include "fldeff.h"
#include "party_menu.h"
#include "script.h"
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

    // The three Regi caves. This one hands off to a script -- the rumble, the three door
    // sounds and the message, ending in the releaseall that frees the player and the
    // setflag that opens the caves. The script cannot be run from here, since its message
    // box waits on the player, so what is checked is that the hand-off happened.
    PutPlayerOnMap(MAP_SEALED_CHAMBER_INNER_ROOM);
    FlagClear(FLAG_REGI_DOORS_OPENED);
    LockPlayerFieldControls();
    EXPECT(SetUpFieldMove_Flash());
    EXPECT(gPostMenuFieldCallback == RandolockeOpenRegiDoors);
    gPostMenuFieldCallback();
    EXPECT(ScriptContext_IsEnabled());
    ScriptContext_Stop();
    UnlockPlayerFieldControls();
}
