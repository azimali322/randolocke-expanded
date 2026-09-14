#ifndef GUARD_CONSTANTS_RANDOLOCKE_H
#define GUARD_CONSTANTS_RANDOLOCKE_H

// Bytes of SaveBlock1 given over to the nuzlocke's "already caught here" bits, one bit
// per wild-encounter area. 50 bytes is 400 areas; there are 394. It is carved out of the
// old dex-flag filler, so raising it costs save space that filler currently holds and
// lowering it is free. src/randolocke_nuzlocke.c asserts the table still fits.
#define RANDOLOCKE_AREA_BYTES   50
#define RANDOLOCKE_MAX_AREAS    (RANDOLOCKE_AREA_BYTES * 8)

#endif // GUARD_CONSTANTS_RANDOLOCKE_H
