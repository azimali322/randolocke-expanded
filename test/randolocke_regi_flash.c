#include "global.h"
#include "config/randolocke.h"
#include "braille_puzzles.h"
#include "event_data.h"
#include "fldeff.h"
#include "party_menu.h"
#include "script.h"
#include "test/test.h"
#include "constants/flags.h"
#include "constants/map_groups.h"
#include "constants/map_scripts.h"

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

#if RANDOLOCKE_REGI_CAVES_OPEN_AT_BADGE_8 == TRUE
extern const u8 Route105_MapScripts[];
extern const u8 Route111_MapScripts[];
extern const u8 Route120_MapScripts[];

// A route's own load script: the one that shuts its Regi cave while the doors are closed.
static const u8 *GetOnLoadScript(const u8 *mapScripts)
{
    for (; *mapScripts != 0; mapScripts += 5)
    {
        if (*mapScripts == MAP_SCRIPT_ON_LOAD)
            return T2_READ_PTR(&mapScripts[1]);
    }
    return NULL;
}

TEST("Randolocke: the Regi caves open with the eighth badge, no Sealed Chamber needed")
{
    const u8 *mapScripts = NULL;

    PARAMETRIZE { mapScripts = Route105_MapScripts; } // Island Cave
    PARAMETRIZE { mapScripts = Route111_MapScripts; } // Desert Ruins
    PARAMETRIZE { mapScripts = Route120_MapScripts; } // Ancient Tomb

    // Any save this late got the Devon Scope before Fortree's badge. Without it, Route 120's
    // load script would move its bridge Kecleon, which is not on the map in a test.
    FlagSet(FLAG_RECEIVED_DEVON_SCOPE);

    // Seven badges and no Sealed Chamber: the entrance stays shut.
    FlagClear(FLAG_REGI_DOORS_OPENED);
    FlagClear(FLAG_BADGE08_GET);
    RunScriptImmediately(GetOnLoadScript(mapScripts));
    EXPECT(!FlagGet(FLAG_REGI_DOORS_OPENED));

    // The eighth: the next time the route loads, the doors are open.
    FlagSet(FLAG_BADGE08_GET);
    RunScriptImmediately(GetOnLoadScript(mapScripts));
    EXPECT(FlagGet(FLAG_REGI_DOORS_OPENED));
}
#endif
