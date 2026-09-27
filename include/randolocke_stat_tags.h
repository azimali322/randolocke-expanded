#ifndef GUARD_RANDOLOCKE_STAT_TAGS_H
#define GUARD_RANDOLOCKE_STAT_TAGS_H

// Showdown-style tags beside each Pokemon's HP box while a move is chosen: one for each stat
// whose stage is not 0, reading "1.5× Atk" or "0.67× Spe". See RANDOLOCKE_STAT_STAGE_TAGS in
// include/config/randolocke.h.

#define STAT_TAG_MAX            7   // Atk, Def, SpA, SpD, Spe, Acc and Eva
#define STAT_TAGS_PER_ROW       3
#define STAT_TAG_MAX_ROWS       ((STAT_TAG_MAX + STAT_TAGS_PER_ROW - 1) / STAT_TAGS_PER_ROW)
#define STAT_TAG_TEXT_LENGTH    12  // "0.67× SpA" and its EOS, with room to spare
#define STAT_TAG_HEIGHT         10  // one pill, its text included
#define STAT_TAG_GAP            1   // between two pills in a row
#define STAT_TAG_ROW_WIDTH      128 // three of the widest pills and their gaps fit

struct StatTag
{
    enum Stat stat; // STAT_ATK to STAT_EVASION
    u8 stage;       // MIN_STAT_STAGE to MAX_STAT_STAGE, never DEFAULT_STAT_STAGE
};

// The multiplier the engine applies at this stage, x100 and rounded: 150 for +1 Attack, 67 for
// -1, 133 for +1 accuracy. Accuracy and evasion have a table of their own.
u32 RandolockeStatStageMultiplier100(enum Stat stat, u32 stage);
// The tag's text, "1.5× Atk". dst must hold STAT_TAG_TEXT_LENGTH bytes; returns its EOS.
u8 *RandolockeStatTagText(u8 *dst, enum Stat stat, u32 stage);
// The width of the tag in pixels, pill and text.
u32 RandolockeStatTagWidth(enum Stat stat, u32 stage);
// The battler's changed stats in Showdown's order: Atk, Def, SpA, SpD, Spe, Acc, Eva. Returns
// how many there are.
u32 RandolockeGetStatTags(enum BattlerId battler, struct StatTag *tags);
// Where row `row` of the battler's tags goes, from where its HP box is: x is the row's left
// edge, or when alignRight its right edge plus one; y is the top of its pills. FALSE if the
// battler's position has no place for them.
bool32 RandolockeStatTagRowPos(enum BattlerId battler, u32 row, s32 *x, s32 *y, bool32 *alignRight);

// Puts up every battler's tags as the move menu opens for `battler`. They come down again
// when it closes, by the rule that takes the type icons down.
void RandolockeLoadStatTags(enum BattlerId battler);
void RandolockeDestroyStatTags(void);
// How many tag sprites are up now.
u32 RandolockeStatTagSpriteCount(void);

#endif // GUARD_RANDOLOCKE_STAT_TAGS_H
