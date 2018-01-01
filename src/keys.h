/*!
\file    keys.h
\brief   Game keys
\author  David Joffe

Copyright (C) 2001-2018 David Joffe

License: GNU GPL Version 2

09/2001
*/
#ifndef _KEYS_H_
#define _KEYS_H_

//! Game key indices
enum
{
	KEY_ACTION = 0,
	KEY_LEFT,
	KEY_RIGHT,
	KEY_JUMP,
	KEY_SHOOT,
	KEY_NUMKEYS
};

//! Game keys (e.g. SDLK_LEFT)
extern int g_anKeys[KEY_NUMKEYS];
//! Game key descriptions
extern const char *g_aszKeys[KEY_NUMKEYS];

//! Initialise game key system
extern void InitialiseGameKeySystem();

//! Return true if given key is an allowable game-play key
extern bool IsGameKey(int nKeyCode);

//! Store current game keys into the settings object
extern void StoreGameKeys();

//! Helper to check if the given (SDL) keycode has been assigned to a game action under 'redefine keys'
extern bool IsGameKeyAssigned(int nKeyCode);

#endif
