#include "global.h"
#include "config/randolocke.h"
#include "event_data.h"
#include "fldeff.h"
#include "overworld.h"
#include "script.h"
#include "test/overworld_script.h"
#include "test/test.h"

#if RANDOLOCKE_NO_DARK_AREAS == TRUE

// RANDOLOCKE_NO_DARK_AREAS: the Flash caves and Dewford Gym are lit from the start.
TEST("Randolocke: nowhere is dark, whatever darkness is set")
{
    // A dark cave's default, and what Dewford Gym's scripts set trainer by trainer.
    SetFlashLevel(7);
    EXPECT_EQ(GetFlashLevel(), 0);
    RUN_OVERWORLD_SCRIPT(
        setflashlevel 4;
    );
    EXPECT_EQ(GetFlashLevel(), 0);
}

TEST("Randolocke: Dewford Gym's lighting animation neither runs nor holds up its script")
{
    // The animation locks the player until it finishes and is what restarts the script;
    // with nothing dark it must do neither, or the gym would stop after each trainer.
    VarSet(VAR_TEMP_0, 0);
    RUN_OVERWORLD_SCRIPT(
        animateflash 3;
        setvar VAR_TEMP_0, 1;
    );
    EXPECT_EQ(VarGet(VAR_TEMP_0), 1);
    EXPECT(!ArePlayerFieldControlsLocked());
}

TEST("Randolocke: Flash does not offer to light a cave that is already lit")
{
    bool32 wasCave = gMapHeader.cave, offered;

    gMapHeader.cave = TRUE;
    FlagClear(FLAG_SYS_USE_FLASH);
    offered = SetUpFieldMove_Flash();
    gMapHeader.cave = wasCave;
    EXPECT(!offered);
}

#endif // RANDOLOCKE_NO_DARK_AREAS
