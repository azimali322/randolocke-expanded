#include "global.h"
#include "config/randolocke.h"
#include "battle.h"
#include "battle_controllers.h"
#include "battle_interface.h"
#include "battle_script_commands.h"
#include "pokemon.h"
#include "sprite.h"
#include "string_util.h"
#include "text.h"
#include "type_icons.h"
#include "randolocke_stat_tags.h"
#include "constants/characters.h"
#include "constants/rgb.h"

// Showdown's statbar tags for this engine: a pill per stat whose stage is not 0, green for a
// boost and red for a drop, reading the multiplier the damage, speed and accuracy formulas
// actually use. Stages are all that is shown -- a Choice Band, a burn or Tailwind is not a
// stage, and Showdown's boost tags leave them out too.
//
// Each row of pills is a chain of 32x16 sprites drawn with the sprite text printer. The printer
// writes a glyph's full 15-row cell, and a cell that runs past the bottom of a sprite goes into
// the sprite below it, so a row gets sprites of its own rather than sharing a taller one.

#define STAT_TAG_FONT               FONT_SMALL_NARROWER
#define STAT_TAG_TEXT_X             2   // the text's left edge inside its pill
#define STAT_TAG_PADDING            4   // a pill is its text's width plus this
#define STAT_TAG_SPRITE_WIDTH       32
#define STAT_TAG_SPRITE_TILES       8   // 32x16
#define STAT_TAG_SPRITES_PER_ROW    (STAT_TAG_ROW_WIDTH / STAT_TAG_SPRITE_WIDTH)
// The pill's first row in its sprite. Text printed at the top of the sprite has its first ink
// on row 4 and its descenders on row 12, so rows 3 to 12 frame it.
#define STAT_TAG_PILL_TOP           3
#define MAX_STAT_TAG_ROWS           (MAX_BATTLERS_COUNT * STAT_TAG_MAX_ROWS)
#define MAX_STAT_TAG_SPRITES        (MAX_STAT_TAG_ROWS * STAT_TAG_SPRITES_PER_ROW)

#define TAG_STAT_TAGS_PAL           0x5A70
#define TAG_STAT_TAGS_TILES         0x5A71  // one sheet per row, 0x5A71 to 0x5A7C

#define sChoosingBattler            data[0]
#define sOwner                      data[5]

enum
{
    STAT_TAG_COLOR_BOOST = 1,
    STAT_TAG_COLOR_DROP,
    STAT_TAG_COLOR_TEXT,
    STAT_TAG_COLOR_BOOST_SHADOW,
    STAT_TAG_COLOR_DROP_SHADOW,
};

// From the HP box's centre, each row's left edge (or right edge + 1 when right-aligned) and the
// top of its pills.
struct StatTagPlace
{
    s8 x[STAT_TAG_MAX_ROWS];
    s8 y[STAT_TAG_MAX_ROWS];
    bool8 alignRight;
    bool8 used;
};

struct StatTagSprites
{
    u8 spriteIds[MAX_STAT_TAG_SPRITES];
    u8 count;
    u8 rowsLoaded;  // sheets TAG_STAT_TAGS_TILES onwards
};

static void SpriteCB_StatTag(struct Sprite *sprite);

static const u8 sText_Atk[] = _("Atk");
static const u8 sText_Def[] = _("Def");
static const u8 sText_SpA[] = _("SpA");
static const u8 sText_SpD[] = _("SpD");
static const u8 sText_Spe[] = _("Spe");
static const u8 sText_Acc[] = _("Acc");
static const u8 sText_Eva[] = _("Eva");

static const u8 *const sStatTagNames[NUM_BATTLE_STATS] =
{
    [STAT_ATK]     = sText_Atk,
    [STAT_DEF]     = sText_Def,
    [STAT_SPATK]   = sText_SpA,
    [STAT_SPDEF]   = sText_SpD,
    [STAT_SPEED]   = sText_Spe,
    [STAT_ACC]     = sText_Acc,
    [STAT_EVASION] = sText_Eva,
};

// Showdown's order, not the engine's: Speed comes after the special stats.
static const enum Stat sStatTagOrder[STAT_TAG_MAX] =
{
    STAT_ATK, STAT_DEF, STAT_SPATK, STAT_SPDEF, STAT_SPEED, STAT_ACC, STAT_EVASION,
};

// Every row is clear of all four HP boxes and the type icons beside them, and of the move menu;
// test/randolocke_stat_tags.c holds them to it. What cannot be kept clear are the Pokemon, as
// with the ability pop-up. Rows of different Pokemon meet only when both have most of their
// stats changed.
static const struct StatTagPlace sStatTagPlaces[BATTLE_COORDS_COUNT][MAX_BATTLERS_COUNT] =
{
    [BATTLE_COORDS_SINGLES] =
    {
        // The first row under the box, between the EXP bar and the move menu, stopping short
        // of the type icons. The menu leaves room for one row; the rest go above the box,
        // against the screen's edge so they miss the foe's box.
        [B_POSITION_PLAYER_LEFT]   = { .x = { 68, 82, 82 }, .y = { 7, -41, -53 }, .alignRight = TRUE, .used = TRUE },
        // Under the box, where Showdown puts them, from under the type icons so that three to
        // a row end before the player's box.
        [B_POSITION_OPPONENT_LEFT] = { .x = { -36, -36, -36 }, .y = { 15, 26, 37 }, .used = TRUE },
    },
    [BATTLE_COORDS_DOUBLES] =
    {
        // Beside each box, towards the middle of the screen, beyond the type icons. The player's
        // two end at the same place, so the lower one's third row, above it, misses the upper box.
        [B_POSITION_PLAYER_LEFT]    = { .x = { -31, -31, -31 }, .y = { -13, -2, 9 }, .alignRight = TRUE, .used = TRUE },
        [B_POSITION_OPPONENT_LEFT]  = { .x = { 69, 69, 69 }, .y = { -14, -3, 8 }, .used = TRUE },
        [B_POSITION_PLAYER_RIGHT]   = { .x = { -43, -43, -43 }, .y = { -13, -2, -24 }, .alignRight = TRUE, .used = TRUE },
        // Past the upper foe's box and type icons as well as its own. Its third row goes above
        // its first: below, it would reach a lone player's box, which sits where it does in
        // singles.
        [B_POSITION_OPPONENT_RIGHT] = { .x = { 81, 81, 81 }, .y = { -14, -3, -25 }, .used = TRUE },
    },
};

static const u16 sStatTagPaletteData[16] =
{
    [STAT_TAG_COLOR_BOOST]        = RGB(7, 21, 10),
    [STAT_TAG_COLOR_DROP]         = RGB(27, 9, 9),
    [STAT_TAG_COLOR_TEXT]         = RGB_WHITE,
    [STAT_TAG_COLOR_BOOST_SHADOW] = RGB(3, 12, 5),
    [STAT_TAG_COLOR_DROP_SHADOW]  = RGB(16, 4, 4),
};

static const struct SpritePalette sStatTagPalette =
{
    .data = sStatTagPaletteData,
    .tag = TAG_STAT_TAGS_PAL,
};

// A row starts blank; the pills and their text are drawn into it.
static const u32 sStatTagBlankRow[STAT_TAG_SPRITES_PER_ROW * STAT_TAG_SPRITE_TILES * TILE_SIZE_4BPP / sizeof(u32)] = {0};

static const struct OamData sOamData_StatTag =
{
    .affineMode = ST_OAM_AFFINE_OFF,
    .objMode = ST_OAM_OBJ_NORMAL,
    .shape = SPRITE_SHAPE(32x16),
    .size = SPRITE_SIZE(32x16),
    .priority = 1,
};

// One per row, each with that row's sheet. A sprite keeps a pointer to its template, and code
// that walks every sprite reads it (the type icons do, to free their own), so these cannot be
// copies on the stack.
#define STAT_TAG_TEMPLATE(row)                          \
{                                                       \
    .tileTag = TAG_STAT_TAGS_TILES + (row),             \
    .paletteTag = TAG_STAT_TAGS_PAL,                    \
    .oam = &sOamData_StatTag,                           \
    .anims = gDummySpriteAnimTable,                     \
    .images = NULL,                                     \
    .affineAnims = gDummySpriteAffineAnimTable,         \
    .callback = SpriteCB_StatTag,                       \
}

static const struct SpriteTemplate sStatTagTemplates[] =
{
    STAT_TAG_TEMPLATE(0), STAT_TAG_TEMPLATE(1), STAT_TAG_TEMPLATE(2),  STAT_TAG_TEMPLATE(3),
    STAT_TAG_TEMPLATE(4), STAT_TAG_TEMPLATE(5), STAT_TAG_TEMPLATE(6),  STAT_TAG_TEMPLATE(7),
    STAT_TAG_TEMPLATE(8), STAT_TAG_TEMPLATE(9), STAT_TAG_TEMPLATE(10), STAT_TAG_TEMPLATE(11),
};
STATIC_ASSERT(ARRAY_COUNT(sStatTagTemplates) == MAX_STAT_TAG_ROWS, StatTagTemplateForEveryRow);

static EWRAM_DATA struct StatTagSprites sStatTags = {0};

u32 RandolockeStatStageMultiplier100(enum Stat stat, u32 stage)
{
    u32 dividend, divisor;

    if (stage > MAX_STAT_STAGE)
        stage = MAX_STAT_STAGE;

    if (stat == STAT_ACC || stat == STAT_EVASION)
    {
        dividend = gAccuracyStageRatios[stage].dividend;
        divisor = gAccuracyStageRatios[stage].divisor;
    }
    else
    {
        dividend = gStatStageRatios[stage][0];
        divisor = gStatStageRatios[stage][1];
    }
    return (dividend * 100 + divisor / 2) / divisor;
}

u8 *RandolockeStatTagText(u8 *dst, enum Stat stat, u32 stage)
{
    u32 value = RandolockeStatStageMultiplier100(stat, stage);
    u32 hundredths = value % 100;

    dst = ConvertIntToDecimalStringN(dst, value / 100, STR_CONV_MODE_LEFT_ALIGN, 1);
    if (hundredths != 0)
    {
        *dst++ = CHAR_PERIOD;
        *dst++ = CHAR_0 + hundredths / 10;
        if (hundredths % 10 != 0)
            *dst++ = CHAR_0 + hundredths % 10;
    }
    *dst++ = CHAR_MULT_SIGN;
    *dst++ = CHAR_SPACE;
    return StringCopy(dst, sStatTagNames[stat]);
}

u32 RandolockeStatTagWidth(enum Stat stat, u32 stage)
{
    u8 text[STAT_TAG_TEXT_LENGTH];

    RandolockeStatTagText(text, stat, stage);
    return GetStringWidth(STAT_TAG_FONT, text, 0) + STAT_TAG_PADDING;
}

u32 RandolockeGetStatTags(enum BattlerId battler, struct StatTag *tags)
{
    u32 i, count = 0;

    for (i = 0; i < STAT_TAG_MAX; i++)
    {
        enum Stat stat = sStatTagOrder[i];
        u32 stage = gBattleMons[battler].statStages[stat];

        if (stage == DEFAULT_STAT_STAGE || stage > MAX_STAT_STAGE)
            continue;
        tags[count].stat = stat;
        tags[count].stage = stage;
        count++;
    }
    return count;
}

bool32 RandolockeStatTagRowPos(enum BattlerId battler, u32 row, s32 *x, s32 *y, bool32 *alignRight)
{
    const struct StatTagPlace *place = &sStatTagPlaces[GetBattlerCoordsIndex(battler)][GetBattlerPosition(battler)];
    s16 boxX, boxY;

    if (!place->used || row >= STAT_TAG_MAX_ROWS)
        return FALSE;

    GetBattlerHealthboxCoords(battler, &boxX, &boxY);
    *x = boxX + place->x[row];
    *y = boxY + place->y[row];
    *alignRight = place->alignRight;
    return TRUE;
}

u32 RandolockeStatTagSpriteCount(void)
{
    u32 i, count = 0;

    for (i = 0; i < sStatTags.count; i++)
    {
        if (gSprites[sStatTags.spriteIds[i]].inUse && gSprites[sStatTags.spriteIds[i]].callback == SpriteCB_StatTag)
            count++;
    }
    return count;
}

void RandolockeDestroyStatTags(void)
{
    u32 i;

    for (i = 0; i < sStatTags.count; i++)
    {
        struct Sprite *sprite = &gSprites[sStatTags.spriteIds[i]];

        // The battle ending resets every sprite, and the slot may be someone else's by now.
        if (sprite->inUse && sprite->callback == SpriteCB_StatTag)
            DestroySprite(sprite);
    }
    for (i = 0; i < sStatTags.rowsLoaded; i++)
        FreeSpriteTilesByTag(TAG_STAT_TAGS_TILES + i);
    FreeSpritePaletteByTag(TAG_STAT_TAGS_PAL);
    sStatTags.count = 0;
    sStatTags.rowsLoaded = 0;
}

static void SpriteCB_StatTag(struct Sprite *sprite)
{
    // The same rule as the type icons: up while this battler's move menu is.
    if (!IsBattlerChoosingMove(sprite->sChoosingBattler))
    {
        RandolockeDestroyStatTags();
        return;
    }
    // The chosen battler's HP box bobs while its menu is open; its tags go with it.
    sprite->y2 = gSprites[gHealthboxSpriteIds[sprite->sOwner]].y2;
}

static void ClearStatTagPixel(const u8 *spriteIds, u32 x, u32 y)
{
    FillSpriteRectColor(spriteIds[x / STAT_TAG_SPRITE_WIDTH], x % STAT_TAG_SPRITE_WIDTH, y, 1, 1, 0);
}

// One pill at `x` in its row: filled, its corners rounded off, and its text on top.
static void DrawStatTag(const u8 *spriteIds, u32 x, u32 width, const struct StatTag *tag)
{
    bool32 boost = tag->stage > DEFAULT_STAT_STAGE;
    u32 bottom = STAT_TAG_PILL_TOP + STAT_TAG_HEIGHT - 1;
    union TextColor color = {
        .background = 0, // not drawn, so the pill shows through
        .foreground = STAT_TAG_COLOR_TEXT,
        .shadow = boost ? STAT_TAG_COLOR_BOOST_SHADOW : STAT_TAG_COLOR_DROP_SHADOW,
        .accent = 0,
    };
    u8 text[STAT_TAG_TEXT_LENGTH];
    u32 textX = x + STAT_TAG_TEXT_X;

    FillSpriteRectColor(spriteIds[x / STAT_TAG_SPRITE_WIDTH], x % STAT_TAG_SPRITE_WIDTH, STAT_TAG_PILL_TOP,
                        width, STAT_TAG_HEIGHT, boost ? STAT_TAG_COLOR_BOOST : STAT_TAG_COLOR_DROP);
    ClearStatTagPixel(spriteIds, x, STAT_TAG_PILL_TOP);
    ClearStatTagPixel(spriteIds, x + width - 1, STAT_TAG_PILL_TOP);
    ClearStatTagPixel(spriteIds, x, bottom);
    ClearStatTagPixel(spriteIds, x + width - 1, bottom);

    RandolockeStatTagText(text, tag->stat, tag->stage);
    // The printer has to start in the sprite that holds the first letter.
    AddSpriteTextPrinterParameterized6(spriteIds[textX / STAT_TAG_SPRITE_WIDTH], STAT_TAG_FONT,
                                       textX % STAT_TAG_SPRITE_WIDTH, 0, 0, 0, color, TEXT_SKIP_DRAW, text);
}

static void CreateStatTagRow(enum BattlerId choosingBattler, enum BattlerId owner, u32 row,
                             const struct StatTag *tags, u32 tagCount)
{
    u8 spriteIds[STAT_TAG_SPRITES_PER_ROW];
    u32 widths[STAT_TAGS_PER_ROW];
    u32 i, rowWidth = 0, spriteCount, tagX;
    s32 x, y;
    bool32 alignRight;
    struct SpriteSheet sheet;

    if (!RandolockeStatTagRowPos(owner, row, &x, &y, &alignRight))
        return;

    for (i = 0; i < tagCount; i++)
    {
        widths[i] = RandolockeStatTagWidth(tags[i].stat, tags[i].stage);
        rowWidth += widths[i] + (i != 0 ? STAT_TAG_GAP : 0);
    }
    if (rowWidth > STAT_TAG_ROW_WIDTH)
        rowWidth = STAT_TAG_ROW_WIDTH;
    if (alignRight)
        x -= rowWidth;

    spriteCount = (rowWidth + STAT_TAG_SPRITE_WIDTH - 1) / STAT_TAG_SPRITE_WIDTH;
    if (sStatTags.rowsLoaded >= MAX_STAT_TAG_ROWS || sStatTags.count + spriteCount > MAX_STAT_TAG_SPRITES)
        return;

    sheet.data = sStatTagBlankRow;
    sheet.size = spriteCount * STAT_TAG_SPRITE_TILES * TILE_SIZE_4BPP;
    sheet.tag = TAG_STAT_TAGS_TILES + sStatTags.rowsLoaded;
    // Tile 0 is a place like any other, so LoadSpriteSheet's result cannot say it failed.
    if (!CanAllocSpriteTiles(sheet.size / TILE_SIZE_4BPP))
        return;
    LoadSpriteSheet(&sheet);
    if (GetSpriteTileStartByTag(sheet.tag) == TAG_NONE)
        return;
    sStatTags.rowsLoaded++;

    for (i = 0; i < spriteCount; i++)
    {
        spriteIds[i] = CreateSprite(&sStatTagTemplates[sStatTags.rowsLoaded - 1],
                                    x + i * STAT_TAG_SPRITE_WIDTH + STAT_TAG_SPRITE_WIDTH / 2,
                                    y - STAT_TAG_PILL_TOP + 8, 0);
        if (spriteIds[i] == MAX_SPRITES)
        {
            // Out of sprites: take this row's back down rather than draw half of it.
            while (i-- != 0)
            {
                DestroySprite(&gSprites[spriteIds[i]]);
                sStatTags.count--;
            }
            return;
        }
        gSprites[spriteIds[i]].oam.tileNum += i * STAT_TAG_SPRITE_TILES;
        sStatTags.spriteIds[sStatTags.count++] = spriteIds[i];
    }

    SetupSpritesForTextPrinting(spriteIds, NULL, spriteCount, 1);
    for (i = 0, tagX = 0; i < tagCount && tagX + widths[i] <= rowWidth; i++)
    {
        DrawStatTag(spriteIds, tagX, widths[i], &tags[i]);
        tagX += widths[i] + STAT_TAG_GAP;
    }

    // Set after drawing: the printer keeps its own links in the sprites' data while it works.
    for (i = 0; i < spriteCount; i++)
    {
        gSprites[spriteIds[i]].sChoosingBattler = choosingBattler;
        gSprites[spriteIds[i]].sOwner = owner;
    }
}

void RandolockeLoadStatTags(enum BattlerId battler)
{
    struct StatTag tags[STAT_TAG_MAX];
    u32 position, count, row;

    RandolockeDestroyStatTags();
    if (LoadSpritePalette(&sStatTagPalette) == 0xFF)
        return;

    for (position = 0; position < gBattlersCount; position++)
    {
        enum BattlerId owner = GetBattlerAtPosition(position);

        if (!IsBattlerAlive(owner))
            continue;
        count = RandolockeGetStatTags(owner, tags);
        for (row = 0; row * STAT_TAGS_PER_ROW < count; row++)
        {
            u32 first = row * STAT_TAGS_PER_ROW;

            CreateStatTagRow(battler, owner, row, &tags[first], min(count - first, STAT_TAGS_PER_ROW));
        }
    }

    if (sStatTags.count == 0)
        RandolockeDestroyStatTags();
}
