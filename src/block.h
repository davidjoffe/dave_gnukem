/*!
\file    block.h
\brief   Level block types
\author  David Joffe

Copyright (C) 2000-2017 David Joffe

License: GNU GPL Version 2
*/
#ifndef _BLOCK_H_
#define _BLOCK_H_

//! Block type enumeration
enum EBlockType
{
	TYPE_NOTHING = 0,
	TYPE_1,
	TYPE_2,
	//! 03 Solid, but can come up from underneath
	TYPE_SOFTSOLID,
	TYPE_4,
	TYPE_5,
	//! 06 Dust
	TYPE_DUST,
	//! 07 Lift
	TYPE_LIFT,
	//! 08 Teleporter
	TYPE_TELEPORTER,
	TYPE_09,
	TYPE_10,
	//! 11 Firepower
	TYPE_FIREPOWER,
	TYPE_12,
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
	//! 20 Coke can
	TYPE_COKE,
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
	TYPE_30,
	//! 31 Level exit
	TYPE_EXIT,
	//! 32 Hero starting position
	TYPE_HEROSTART,
	TYPE_33,
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
	//! 42 Debug block, used for testing stuff out
	TYPE_TEST,
	//! 43 Bouncing spiked ball
	TYPE_SPIKEBALL,
	//! 44 Monster - Dumb robot
	TYPE_ROBOT,
	//! Highest used block value
	TYPE_LASTONE
};

//! Friendly strings for block types \ref EBlockType, used by editor.
extern const char * block_type_names[TYPE_LASTONE+1];

#endif
