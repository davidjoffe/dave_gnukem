/*!
\file    level.h
\brief   Level stuff
\author  David Joffe

Copyright (C) 1995-2023 David Joffe
*/
/*--------------------------------------------------------------------------*/
#ifndef _LEVEL_H_
#define _LEVEL_H_
/*--------------------------------------------------------------------------*/
#include <vector>

#include "config.h"
/*--------------------------------------------------------------------------*/
// [dj2022-11 LOW prio consider refactoring this to be for level editor only and having something separate for ingame? ingame currently we just load at position 0! abit weird some of this harkens back to 1993/4 design thoughts on old 286 with tiny memory]
extern std::vector<unsigned char * > apLevels;
/*--------------------------------------------------------------------------*/
//! Each 'block' position in the level has two blocks: A foreground block and a background block. Each block is described by a pair of unsigned chars, denoted by convention in this codebase as (a,b). 'a' denotes the 0-based spriteset index, and 'b' denotes a [0..127] offset into that spriteset.
struct SLevelBlock
{
	unsigned char afore;
	unsigned char bfore;
	unsigned char aback;
	unsigned char bback;
};

void InitLevelSystem ();
void KillLevelSystem ();
//Returns NULL if bad filename, but still allocates a default empty level, which will be loaded into apLevels[0]. dj2017-06 First parameter is level 'slot' which I think is now always 0 and thus not used, I think some older version of the code used to load level N datablock in slot N. Now only one level's datablock is actually loaded at any time.
extern unsigned char * level_load( int i, const char * szfilename );
int                    level_save( int i, const char * szfilename );

// dj2022-11 for clarity I'm wondering if we should have different helper functions for level editor and for gameplay for this level pointer stuff.
// The game only has one level loaded at a time typically. The level editor currently also, except in certain circumstances (e.g. the stats function) where it suddenly loads them all at once.
// So at most times this 'i' is unused except for the level editor stats function. But there may be other advantages to having the level editor in future load all levels at once.
// HOWEVER for gameplay we just want the fastest access possible (so it shouldn't even be a function I think but a macro that just gets offset into memory block of current level - with no or limited safety checking for speed reasons except maybe in debug build -
// we e.g. already have such helpers (e.g. LEVCHAR_FOREA etc.) .. so these are meant for *gameplay* fast access to *current level* where we don't want to slow down things with any
// unnecessary parameters like the 'i' here .. but for level editor we don't care about the tiny performance hit that's negligible for editing purposes .. we rather want safety and more functionality eg having all levels loaded at once sometimes
// So maybe these should be split or renamed etc. i.e. separate helpers for editor or gameplay that are labelled differently to make it clear which to use (fast macros with no 'i' for in-game level data reads).
// Anyway, not important at this point - I was just trying to remember more exactly the nature and use of these functions.

// also first parameter not used? it could be an inline class member? globals gross
//cbsu/sbsu this might be worth inlining (LOW prio - dj2019-07)
extern unsigned char * level_pointer( int i, int x, int y );
//! This is 'friendlier' than using level_pointer() but slightly slower so where speed is sensitive don't use it [dj2017-06]
extern SLevelBlock     level_get_block( int i, int x, int y );
extern void            level_delete( int i );

/*--------------------------------------------------------------------------*/
// 'Fast' macro helpers for in-game level data checking of 'currently loaded level that the player is playing' (macros to 'force' inline for speed reasons):
// though in future things like the LEVEL_BYTESPERROW and LEVEL_BYTESPERBLOCK should become more easily configurable, both per-game and per-level etc. [low prio -dj2022-11]

#define LEVCHAR_FOREA(x,y) (*( g_pLevel + (y) * LEVEL_BYTESPERROW + (x) * LEVEL_BYTESPERBLOCK + 0 ))
#define LEVCHAR_FOREB(x,y) (*( g_pLevel + (y) * LEVEL_BYTESPERROW + (x) * LEVEL_BYTESPERBLOCK + 1 ))
#define LEVCHAR_BACKA(x,y) (*( g_pLevel + (y) * LEVEL_BYTESPERROW + (x) * LEVEL_BYTESPERBLOCK + 2 ))
#define LEVCHAR_BACKB(x,y) (*( g_pLevel + (y) * LEVEL_BYTESPERROW + (x) * LEVEL_BYTESPERBLOCK + 3 ))

#define CHECK_SOLID(a,b)  ( GET_EXTRA( (a), (b), EXTRA_FLAGS ) & FLAG_SOLID )

extern unsigned char *g_pLevel;
/*--------------------------------------------------------------------------*/

#endif

