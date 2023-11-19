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
// Is there really a good reason for this function to exist? [dj2023-11]
// I don't remember exactly why/what etc. but it looks like it's used during redefining keys
// But, looks like a good way to possibly exclude potentially important (and maybe new) keycodes for niche platforms e.g. retro handhelds
// Why not just accept any keycode during redefining? This could at best be used to check for supported ones to display correctly or something
// and then have some fallback code e.g. if some platform returns a new 'unknown' keycode just show the keycode and allow it to be defined and used in the game?
// but 
// I mean, then instead of a vector, maybe an unordered_map (unordered for faster lookups) that just map keycode to name
// and then we don't exclude any key accepted during redefining OR playing, but just do something like:
// if (map.find(nkeyCode)!=map.end())
//    sKeyName = map[nKeyCode];
// else
//    sKeyName = std::string("<") + std::to_string(nkeyCode) + ">";
// e.g. if you press Up then it displays "Up" but if an unknown keycode comes in from some future or niche device still accept it
// but just display e.g. "<143>" or whatever the internal code is.
// Or am I remembering wrong why this is here? I wrote this really long ago
// The above would also be more 'future-proof' e.g. say in a few years new keyboards come out with lots of new keycode, or
// different languages or something - or odd joysticks or mice that map arbitrary keycodes - make this 'future-proof' so
// it automatically supports anything, and not have a moving target we have to chase to keep adding so-called 'valid' keys.
// If SDL_input stuff sends a keycode it's surely "valid" whether we have it this list
// If there are keycodes we should explicitly not allow redefining in a game (are there? I don't think so) then rather
// have an 'IsInvalidGameKey' with an exclusion list (or just take this out entirely and allow anything, unless it causes problems)


//! Store current game keys into the settings object
extern void StoreGameKeys();

//! Helper to check if the given (SDL) keycode has been assigned to a game action under 'redefine keys'
extern bool IsGameKeyAssigned(int nKeyCode);

#endif
