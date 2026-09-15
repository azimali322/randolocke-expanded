#ifndef GUARD_CONSTANTS_RANDOLOCKE_H
#define GUARD_CONSTANTS_RANDOLOCKE_H

// Bytes of SaveBlock1 given over to the nuzlocke's "already caught here" bits, one bit
// per wild-encounter area, keyed by region map section. 50 bytes is 400 areas; MAPSEC_COUNT
// is 210, so there is room to spare. Carved out of the old dex-flag filler, and the filler
// beside it absorbs whatever this does not use -- so changing it moves no other field and
// does not invalidate a save. src/randolocke_nuzlocke.c asserts the table still fits.
#define RANDOLOCKE_AREA_BYTES   50
#define RANDOLOCKE_MAX_AREAS    (RANDOLOCKE_AREA_BYTES * 8)

// Emerald's move tutors. See gRandolockeTutorMoves.
#define RANDOLOCKE_TUTOR_COUNT  10

#endif // GUARD_CONSTANTS_RANDOLOCKE_H
