/*!
\file    block.h
\brief   Level block types
\author  David Joffe

Copyright (C) 2000-2024 David Joffe
*/
#ifndef _BLOCK_H_
#define _BLOCK_H_

#include <string>

//! Block type enumeration (NB: If add more here, remember to also extend \ref block_type_names)
enum EBlockType
{
	TYPE_NOTHING = 0,
	TYPE_BLOCK,//[dj2018-01-22]
	TYPE_2,
	//! 03 Solid, but can come up from underneath
	TYPE_SOFTSOLID,
	TYPE_4,
	//! 05 Simple water effect [dj2017-08]
	TYPE_WATER,
	//! 06 Dust
	TYPE_DUST,
	//! 07 Lift
	TYPE_LIFT,
	//! 08 Teleporter
	TYPE_TELEPORTER,
	//! 09 Rocket [dj2017-08]
	TYPE_ROCKET,
	TYPE_10,
	//! 11 Firepower
	TYPE_FIREPOWER,
	//! 12 Crumbling floor (crumbles slightly explosively after hero walks on it a couple of times) [dj2017-07]
	TYPE_CRUMBLINGFLOOR,
	//! 13 Conveyor belt
	TYPE_CONVEYOR,
	//! 14 Letter (G,N,U,K,E,M)
	TYPE_LETTER,
	//! 15 Box that opens when shot
	TYPE_BOX,
	TYPE_16,
	//! 17 Generic pickup (food, points etc)
	TYPE_PICKUP,
	TYPE_18,
	//! 19 Power boots
	TYPE_POWERBOOTS,
	//! 20 Soda can
	TYPE_SODACAN,
	//! 21 Full health pickup (DN1 molecule) [dj2017-06-26]
	TYPE_FULLHEALTH,
	TYPE_22,
	//! 23 Dynamite
	TYPE_DYNAMITE,
	//! 24 Flame throw thingy
	TYPE_FLAMETHROW,
	//! 25 Key. Extra[0]==ID
	TYPE_KEY,
	//! 26 Door. Extra[0]==ID
	TYPE_DOOR,
	//! 27 Door activator. Extra[0]==ID
	TYPE_DOORACTIVATOR,
	//! 28 Fan
	TYPE_FAN,
	//! 29 Access card
	TYPE_ACCESSCARD,
	//! 30 Antivirus
	TYPE_ANTIVIRUS,
	//! 31 Level exit
	TYPE_EXIT,
	//! 32 Hero starting position
	TYPE_HEROSTART,
	//! 33 Main Computer
	TYPE_MAINCOMPUTER,
	TYPE_34,
	//! 35 Acme falling block
	TYPE_ACME,
	//! 36 Balloon
	TYPE_BALLOON,
	//! 37 Spike
	TYPE_SPIKE,
	//! 38 Monster - Green crawly thing
	TYPE_CRAWLER,
	//! 39 Banana
	TYPE_BANANA,
	//! 40 Security camera
	TYPE_CAMERA,
	//! 41 Solid block which disappears when gets shot
	TYPE_SOFTBLOCK,
	//! 42 Unused [This used to be TYPE_TEST (CTest) - reclaiming it and making it unused - dj2017-08]
	TYPE_42,
	//! 43 Bouncing spiked ball
	TYPE_SPIKEBALL,
	//! 44 Monster - Dumb robot
	TYPE_ROBOT,
	//! 45 Monster - Small flying robot that follows you slowly-ish [dj2017-06-30]
	TYPE_FLYINGROBOT,
	//! 46 Rabbit-equivalent (disclaimer: may or may not be a rabbit) [dj2017-07-29]
	TYPE_RABBIT,
	//! 47 High-voltage "barrier" that must be shot multiple times to destroy before hero can pass through. Touching it results in immediate death.
	TYPE_HIGHVOLTAGE,
	//! 48 Cannon-on-wheels-ish-type-thing
	TYPE_CANNON,
	//! 49 Jumping monster [dj2017-08]
	TYPE_JUMPINGMONSTER,
	//! 50 Dr Proton [dj2017-08]
	TYPE_DRPROTON,
	//! Highest used block value (UNUSED VALUE)
	TYPE_LASTONE
};

//! Friendly strings for block types \ref EBlockType, used by editor.
extern const char * block_type_names[TYPE_LASTONE+1];

//! Friendly strings for block types \ref EBlockType, used by editor. Gets the block_type_names value, but safer, as returns empty string if out of bounds.
extern std::string GetBlockTypeName(EBlockType eType);

#endif
