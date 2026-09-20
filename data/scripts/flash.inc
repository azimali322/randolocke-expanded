EventScript_UseFlash::
	animateflash 1
	setflashlevel 1
	releaseall
	end

@ randolocke: Flash in the Sealed Chamber's inner room stands in for the Braille puzzle,
@ which needs a Wailord and a Relicanth a randomized run may never see. Same presentation
@ as SealedChamber_InnerRoom_EventScript_BrailleBackWall -- the rumble, the three doors,
@ the message -- minus that check. Reached from the party menu through
@ RandolockeOpenRegiDoors, where the releaseall at the end is also what hands the player
@ back: a field callback runs with the player's controls locked.
RandolockeEventScript_FlashOpensRegiDoors::
	lockall
	fadeoutbgm 0
	playse SE_TRUCK_MOVE
	special DoSealedChamberShakingEffect_Long
	delay 40
	special DoSealedChamberShakingEffect_Short
	playse SE_DOOR
	delay 40
	special DoSealedChamberShakingEffect_Short
	playse SE_DOOR
	delay 40
	special DoSealedChamberShakingEffect_Short
	playse SE_DOOR
	delay 40
	msgbox gText_DoorOpenedFarAway, MSGBOX_DEFAULT
	closemessage
	fadeinbgm 0
	setflag FLAG_REGI_DOORS_OPENED
	releaseall
	end
