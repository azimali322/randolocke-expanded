#ifndef GUARD_BRAILLE_PUZZLES_H
#define GUARD_BRAILLE_PUZZLES_H

bool8 ShouldDoBrailleRegisteelEffect(void);
bool8 ShouldDoBrailleRegirockEffect(void);
bool8 ShouldDoBrailleDigEffect(void);
void DoBrailleDigEffect(void);
void SetUpPuzzleEffectRegisteel(void);
void SetUpPuzzleEffectRegirock(void);


bool8 RandolockeFlashOpensRegirock(void);
bool8 RandolockeFlashOpensRegice(void);
bool8 RandolockeFlashOpensSealedOuter(void);
bool8 RandolockeFlashOpensRegiDoors(void);
void RandolockeOpenRegiceWall(void);
void RandolockeOpenSealedChamberDoor(void);
void RandolockeOpenRegiDoors(void);

#endif // GUARD_BRAILLE_PUZZLES_H
