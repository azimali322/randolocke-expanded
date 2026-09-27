#include "global.h"
#include "battle.h"
#include "battle_controllers.h"
#include "battle_interface.h"
#include "randolocke_stat_tags.h"
#include "sprite.h"
#include "string_util.h"
#include "text.h"
#include "test/test.h"
#include "constants/battle.h"

// The stat stage tags shown while a move is chosen (RANDOLOCKE_STAT_STAGE_TAGS): what they say,
// where they go, and that they come and go cleanly.

#define MOVE_MENU_TOP   112 // the battle text box's frame starts on tile row 14

enum { FORMAT_SINGLES, FORMAT_DOUBLES, FORMAT_DOUBLES_PLAYER_ALONE };

struct Rect { s32 left, top, right, bottom; }; // right and bottom exclusive

extern const struct Coords16 sTypeIconPositions[][2];

static bool32 Overlaps(struct Rect a, struct Rect b)
{
    return a.left < b.right && b.left < a.right && a.top < b.bottom && b.top < a.bottom;
}

static void ResetStages(enum BattlerId battler)
{
    for (u32 stat = 0; stat < NUM_BATTLE_STATS; stat++)
        gBattleMons[battler].statStages[stat] = DEFAULT_STAT_STAGE;
}

TEST("Randolocke: stat tags read the multiplier the engine applies")
{
    static const u16 sStat[MAX_STAT_STAGE + 1]     = {25, 29, 33, 40, 50, 67, 100, 150, 200, 250, 300, 350, 400};
    static const u16 sAccuracy[MAX_STAT_STAGE + 1] = {33, 36, 43, 50, 60, 75, 100, 133, 166, 200, 233, 266, 300};

    for (u32 stage = MIN_STAT_STAGE; stage <= MAX_STAT_STAGE; stage++)
    {
        EXPECT_EQ(RandolockeStatStageMultiplier100(STAT_ATK, stage), sStat[stage]);
        EXPECT_EQ(RandolockeStatStageMultiplier100(STAT_DEF, stage), sStat[stage]);
        EXPECT_EQ(RandolockeStatStageMultiplier100(STAT_SPATK, stage), sStat[stage]);
        EXPECT_EQ(RandolockeStatStageMultiplier100(STAT_SPDEF, stage), sStat[stage]);
        EXPECT_EQ(RandolockeStatStageMultiplier100(STAT_SPEED, stage), sStat[stage]);
        EXPECT_EQ(RandolockeStatStageMultiplier100(STAT_ACC, stage), sAccuracy[stage]);
        EXPECT_EQ(RandolockeStatStageMultiplier100(STAT_EVASION, stage), sAccuracy[stage]);
    }
}

TEST("Randolocke: a stat tag reads like Showdown's")
{
    u8 text[STAT_TAG_TEXT_LENGTH];
    enum Stat stat = STAT_ATK;
    u32 stage = 0;
    const u8 *expected = NULL;

    PARAMETRIZE { stat = STAT_ATK;     stage = DEFAULT_STAT_STAGE + 1; expected = COMPOUND_STRING("1.5× Atk"); }
    PARAMETRIZE { stat = STAT_SPATK;   stage = DEFAULT_STAT_STAGE - 1; expected = COMPOUND_STRING("0.67× SpA"); }
    PARAMETRIZE { stat = STAT_SPEED;   stage = MAX_STAT_STAGE;         expected = COMPOUND_STRING("4× Spe"); }
    PARAMETRIZE { stat = STAT_DEF;     stage = MIN_STAT_STAGE;         expected = COMPOUND_STRING("0.25× Def"); }
    PARAMETRIZE { stat = STAT_SPDEF;   stage = DEFAULT_STAT_STAGE - 5; expected = COMPOUND_STRING("0.29× SpD"); }
    PARAMETRIZE { stat = STAT_SPEED;   stage = DEFAULT_STAT_STAGE + 3; expected = COMPOUND_STRING("2.5× Spe"); }
    PARAMETRIZE { stat = STAT_ACC;     stage = DEFAULT_STAT_STAGE + 1; expected = COMPOUND_STRING("1.33× Acc"); }
    PARAMETRIZE { stat = STAT_EVASION; stage = DEFAULT_STAT_STAGE - 5; expected = COMPOUND_STRING("0.36× Eva"); }
    PARAMETRIZE { stat = STAT_EVASION; stage = DEFAULT_STAT_STAGE + 5; expected = COMPOUND_STRING("2.66× Eva"); }

    RandolockeStatTagText(text, stat, stage);
    EXPECT_EQ(StringCompare(text, expected), 0);
}

TEST("Randolocke: stat tags list the changed stats in Showdown's order")
{
    struct StatTag tags[STAT_TAG_MAX];

    ResetStages(B_BATTLER_0);
    EXPECT_EQ(RandolockeGetStatTags(B_BATTLER_0, tags), 0);

    gBattleMons[B_BATTLER_0].statStages[STAT_EVASION] = DEFAULT_STAT_STAGE + 1;
    gBattleMons[B_BATTLER_0].statStages[STAT_SPEED] = DEFAULT_STAT_STAGE + 2;
    gBattleMons[B_BATTLER_0].statStages[STAT_ATK] = DEFAULT_STAT_STAGE - 1;
    gBattleMons[B_BATTLER_0].statStages[STAT_ACC] = DEFAULT_STAT_STAGE - 1;
    EXPECT_EQ(RandolockeGetStatTags(B_BATTLER_0, tags), 4);
    EXPECT_EQ(tags[0].stat, STAT_ATK);
    EXPECT_EQ(tags[0].stage, DEFAULT_STAT_STAGE - 1);
    EXPECT_EQ(tags[1].stat, STAT_SPEED);
    EXPECT_EQ(tags[1].stage, DEFAULT_STAT_STAGE + 2);
    EXPECT_EQ(tags[2].stat, STAT_ACC);
    EXPECT_EQ(tags[3].stat, STAT_EVASION);

    for (u32 stat = STAT_ATK; stat < NUM_BATTLE_STATS; stat++)
        gBattleMons[B_BATTLER_0].statStages[stat] = MIN_STAT_STAGE;
    EXPECT_EQ(RandolockeGetStatTags(B_BATTLER_0, tags), STAT_TAG_MAX);
    EXPECT_EQ(tags[0].stat, STAT_ATK);
    EXPECT_EQ(tags[1].stat, STAT_DEF);
    EXPECT_EQ(tags[2].stat, STAT_SPATK);
    EXPECT_EQ(tags[3].stat, STAT_SPDEF);
    EXPECT_EQ(tags[4].stat, STAT_SPEED);
    EXPECT_EQ(tags[5].stat, STAT_ACC);
    EXPECT_EQ(tags[6].stat, STAT_EVASION);
}

// The widest of the 84 tags there can be.
static u32 WidestStatTag(void)
{
    u32 widest = 0;

    for (u32 stat = STAT_ATK; stat < NUM_BATTLE_STATS; stat++)
    {
        for (u32 stage = MIN_STAT_STAGE; stage <= MAX_STAT_STAGE; stage++)
        {
            if (stage != DEFAULT_STAT_STAGE && RandolockeStatTagWidth(stat, stage) > widest)
                widest = RandolockeStatTagWidth(stat, stage);
        }
    }
    return widest;
}

TEST("Randolocke: every stat tag fits three to a row")
{
    u32 widest = WidestStatTag();

    // A width of only the padding would mean the text measured as nothing.
    EXPECT_GT(widest, 20);
    EXPECT_LE(widest * STAT_TAGS_PER_ROW + STAT_TAG_GAP * (STAT_TAGS_PER_ROW - 1), STAT_TAG_ROW_WIDTH);
}

static void SetUpFormat(u32 format)
{
    gBattleTypeFlags = (format == FORMAT_SINGLES) ? 0 : BATTLE_TYPE_DOUBLE;
    gBattlersCount = (format == FORMAT_SINGLES) ? 2 : MAX_BATTLERS_COUNT;
    gPartiesCount[B_TRAINER_PLAYER] = (format == FORMAT_DOUBLES) ? 2 : 1;
    gPartiesCount[B_TRAINER_OPPONENT_A] = (format == FORMAT_SINGLES) ? 1 : 2;
    for (enum BattlerId battler = 0; battler < MAX_BATTLERS_COUNT; battler++)
        gBattlerPositions[battler] = battler;
}

static bool32 IsPresent(u32 format, enum BattlerId battler)
{
    if (battler >= gBattlersCount)
        return FALSE;
    return !(format == FORMAT_DOUBLES_PLAYER_ALONE && GetBattlerPosition(battler) == B_POSITION_PLAYER_RIGHT);
}

// The drawn part of an HP box: where the box sits in its sprites, from the graphics in
// graphics/battle_interface/healthbox_*.png, placed at the box's centre.
static struct Rect HealthboxArt(enum BattlerId battler)
{
    s16 x, y;

    GetBattlerHealthboxCoords(battler, &x, &y);
    if (GetBattlerCoordsIndex(battler) == BATTLE_COORDS_SINGLES && IsOnPlayerSide(battler))
        return (struct Rect){ x - 32 + 7, y - 32 + 2, x - 32 + 104, y - 32 + 38 };  // two 64x64
    if (GetBattlerCoordsIndex(battler) == BATTLE_COORDS_SINGLES)
        return (struct Rect){ x - 32 + 1, y - 16 + 2, x - 32 + 101, y - 16 + 30 };  // two 64x32
    return (struct Rect){ x - 32 + 1, y - 16 + 2, x - 32 + 101, y - 16 + 27 };      // two 64x32
}

// Both type icons of a Pokemon once they have slid 10 pixels towards its box: 8x16 each, the
// second 11 pixels below the first. See src/type_icons.c.
static struct Rect TypeIcons(u32 format, enum BattlerId battler)
{
    bool32 doubles = format != FORMAT_SINGLES && !(format == FORMAT_DOUBLES_PLAYER_ALONE && IsOnPlayerSide(battler));
    enum BattlerPosition position = GetBattlerPosition(battler);
    s32 x = sTypeIconPositions[position][doubles].x;
    s32 y = sTypeIconPositions[position][doubles].y;

    if (doubles)
        x += IsOnPlayerSide(battler) ? -10 : 10;
    else
        x += IsOnPlayerSide(battler) ? 10 : -10;
    return (struct Rect){ x - 4, y - 8, x + 4, y + 11 + 8 };
}

static struct Rect StatTagRow(enum BattlerId battler, u32 row, u32 width)
{
    s32 x, y;
    bool32 alignRight;

    if (!RandolockeStatTagRowPos(battler, row, &x, &y, &alignRight))
        return (struct Rect){ 0, 0, 0, 0 };
    if (alignRight)
        x -= width;
    return (struct Rect){ x, y, x + width, y + STAT_TAG_HEIGHT };
}

TEST("Randolocke: stat tags stay clear of the HP boxes, the type icons and the move menu")
{
    u32 format = 0;
    u32 fullRow = WidestStatTag() * STAT_TAGS_PER_ROW + STAT_TAG_GAP * (STAT_TAGS_PER_ROW - 1);

    PARAMETRIZE { format = FORMAT_SINGLES; }
    PARAMETRIZE { format = FORMAT_DOUBLES; }
    PARAMETRIZE { format = FORMAT_DOUBLES_PLAYER_ALONE; }

    SetUpFormat(format);
    for (enum BattlerId battler = 0; battler < MAX_BATTLERS_COUNT; battler++)
    {
        if (!IsPresent(format, battler))
            continue;
        for (u32 row = 0; row < STAT_TAG_MAX_ROWS; row++)
        {
            struct Rect tags = StatTagRow(battler, row, fullRow);

            // Every present Pokemon has a place for every row.
            EXPECT_GT(tags.right, tags.left);
            EXPECT_GE(tags.left, 0);
            EXPECT_LE(tags.right, DISPLAY_WIDTH);
            EXPECT_GE(tags.top, 0);
            EXPECT_LE(tags.bottom, MOVE_MENU_TOP);

            for (enum BattlerId other = 0; other < MAX_BATTLERS_COUNT; other++)
            {
                if (!IsPresent(format, other))
                    continue;
                if (Overlaps(tags, HealthboxArt(other)) || Overlaps(tags, TypeIcons(format, other)))
                    Test_MgbaPrintf("battler %d row %d overlaps battler %d's box or icons", battler, row, other);
                EXPECT(!Overlaps(tags, HealthboxArt(other)));
                EXPECT(!Overlaps(tags, TypeIcons(format, other)));
                // Nor, with a full first row each, do two Pokemon's tags meet.
                if (row == 0 && other != battler)
                    EXPECT(!Overlaps(tags, StatTagRow(other, 0, fullRow)));
            }
        }
    }
}

// A singles battle with its HP boxes' sprites, the player's move menu open.
static void SetUpStatTagBattle(void)
{
    ResetSpriteData();
    FreeAllSpritePalettes();
    SetDefaultFontsPointer();
    SetUpFormat(FORMAT_SINGLES);
    gAbsentBattlerFlags = 0;
    for (enum BattlerId battler = 0; battler < gBattlersCount; battler++)
    {
        gBattleMons[battler].species = SPECIES_WOBBUFFET;
        gBattleMons[battler].hp = 1;
        ResetStages(battler);
        gHealthboxSpriteIds[battler] = CreateInvisibleSprite(SpriteCallbackDummy);
        gBattlerControllerFuncs[battler] = BattleControllerDummy;
    }
    gBattlerControllerFuncs[B_BATTLER_0] = HandleInputChooseMove;
}

static u32 SpritesWithStatTagTemplate(void)
{
    u32 count = 0;

    for (u32 i = 0; i < MAX_SPRITES; i++)
    {
        if (gSprites[i].inUse && gSprites[i].template != NULL && gSprites[i].template->paletteTag == 0x5A70)
            count++;
    }
    return count;
}

static u32 SpritesForRow(u32 width)
{
    return (width + 31) / 32;
}

TEST("Randolocke: stat tags come up with the move menu and go with it")
{
    u32 player, foe, spritesBefore = 0;

    SetUpStatTagBattle();
    gBattleMons[B_BATTLER_0].statStages[STAT_ATK] = DEFAULT_STAT_STAGE + 2;
    gBattleMons[B_BATTLER_0].statStages[STAT_SPEED] = DEFAULT_STAT_STAGE - 1;
    gBattleMons[B_BATTLER_1].statStages[STAT_DEF] = DEFAULT_STAT_STAGE - 1;
    player = SpritesForRow(RandolockeStatTagWidth(STAT_ATK, DEFAULT_STAT_STAGE + 2) + STAT_TAG_GAP
                         + RandolockeStatTagWidth(STAT_SPEED, DEFAULT_STAT_STAGE - 1));
    foe = SpritesForRow(RandolockeStatTagWidth(STAT_DEF, DEFAULT_STAT_STAGE - 1));
    for (u32 i = 0; i < MAX_SPRITES; i++)
        spritesBefore += gSprites[i].inUse;

    RandolockeLoadStatTags(B_BATTLER_0);
    EXPECT_EQ(RandolockeStatTagSpriteCount(), player + foe);
    // Each keeps a template that outlives the function that made it: code that walks every
    // sprite reads it, the type icons' clean-up among them.
    EXPECT_EQ(SpritesWithStatTagTemplate(), player + foe);

    // While the menu is open they stay, and opening it again does not add a second set.
    AnimateSprites();
    EXPECT_EQ(RandolockeStatTagSpriteCount(), player + foe);
    RandolockeLoadStatTags(B_BATTLER_0);
    EXPECT_EQ(RandolockeStatTagSpriteCount(), player + foe);

    // A move picked: every tag goes, and with them their tiles and palette.
    gBattlerControllerFuncs[B_BATTLER_0] = BattleControllerDummy;
    AnimateSprites();
    EXPECT_EQ(RandolockeStatTagSpriteCount(), 0);
    for (u32 i = 0; i < MAX_SPRITES; i++)
        spritesBefore -= gSprites[i].inUse;
    EXPECT_EQ(spritesBefore, 0);
    EXPECT_EQ(IndexOfSpritePaletteTag(0x5A70), 0xFF);
    EXPECT_EQ(GetSpriteTileStartByTag(0x5A71), 0xFFFF);
    EXPECT_EQ(GetSpriteTileStartByTag(0x5A72), 0xFFFF);

    ResetSpriteData();
    FreeAllSpritePalettes();
}

TEST("Randolocke: stat tags leak nothing over many turns")
{
    u32 sprites = 0;

    SetUpStatTagBattle();
    for (u32 stat = STAT_ATK; stat < NUM_BATTLE_STATS; stat++)
    {
        gBattleMons[B_BATTLER_0].statStages[stat] = MAX_STAT_STAGE;
        gBattleMons[B_BATTLER_1].statStages[stat] = MIN_STAT_STAGE;
    }
    for (u32 i = 0; i < MAX_SPRITES; i++)
        sprites += gSprites[i].inUse;

    for (u32 turn = 0; turn < 20; turn++)
    {
        gBattlerControllerFuncs[B_BATTLER_0] = HandleInputChooseMove;
        RandolockeLoadStatTags(B_BATTLER_0);
        // Seven tags each: three rows for both Pokemon.
        EXPECT_GT(RandolockeStatTagSpriteCount(), 0);
        gBattlerControllerFuncs[B_BATTLER_0] = BattleControllerDummy;
        AnimateSprites();
        EXPECT_EQ(RandolockeStatTagSpriteCount(), 0);
    }
    for (u32 i = 0; i < MAX_SPRITES; i++)
        sprites -= gSprites[i].inUse;
    EXPECT_EQ(sprites, 0);
    for (u32 tag = 0x5A71; tag <= 0x5A7C; tag++)
        EXPECT_EQ(GetSpriteTileStartByTag(tag), 0xFFFF);
    EXPECT_EQ(IndexOfSpritePaletteTag(0x5A70), 0xFF);

    ResetSpriteData();
    FreeAllSpritePalettes();
}

TEST("Randolocke: no stat tags while no stat has changed")
{
    SetUpStatTagBattle();
    RandolockeLoadStatTags(B_BATTLER_0);
    EXPECT_EQ(RandolockeStatTagSpriteCount(), 0);
    EXPECT_EQ(IndexOfSpritePaletteTag(0x5A70), 0xFF);

    ResetSpriteData();
    FreeAllSpritePalettes();
}
