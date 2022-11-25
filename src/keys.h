/*!
\file    keys.h
\brief   Game keys
\author  David Joffe

Copyright (C) 2001-2022 David Joffe

We want these headers as small and light as possible, not including lots of stuff, for fast compile speeds.

09/2001
*/
#ifndef _KEYS_H_
#define _KEYS_H_

//! Game key indices. NB NB if you add more here, also change g_aszKeys!
enum
{
	KEY_ACTION = 0,
	KEY_LEFT,
	KEY_RIGHT,
	KEY_JUMP,
	KEY_SHOOT,
	KEY_NUM_MAIN_REDEFINABLE_KEYS
};

//! Main redefinable game keys (e.g. SDLK_LEFT)
extern int g_anKeys[KEY_NUM_MAIN_REDEFINABLE_KEYS];
//! Game key descriptions
extern const char *g_aszKeys[KEY_NUM_MAIN_REDEFINABLE_KEYS];

//! Initialise game key system
extern void InitialiseGameKeySystem();


// dj2022-11 A few off-the-cuff thoughts on "IsGameKey()" and "std::vector<int> g_anValidGameKeys":
// (1) I don't remember exactly the thinking behind having some specific closed list of so-called 'valid game keys' and I wonder if it's really a
// good idea to have this 'restriction' in the code as it may make porting to special platforms (e.g. maybe consoles with special custom scancodes
// for keys) a bit more difficult? But maybe there was some good reason - I don't know remember this was written long ago.
// I guess some keys we may want to avoid using but not sure what's the best way (though maybe porters to other platforms could have a configurable file with custom scancodes or something they could add somewhere etc.)
// (2) A vector is 'std::vector<int> g_anValidGameKeys' is a bad performance choice, but, it seems it's "only" used in redefining the keys so not a priority


//! Return true if given key is an allowable game-play key
extern bool IsGameKey(int nKeyCode);

//! Store current game keys into the settings object
extern void StoreGameKeys();

//! Helper to check if the given (SDL) keycode has been assigned to a game action under 'redefine keys'
extern bool IsGameKeyAssigned(int nKeyCode);

#endif
