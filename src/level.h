/*!
\file    level.h
\brief   Level stuff
\author  David Joffe

Copyright (C) 1995-2017 David Joffe

License: GNU GPL Version 2
*/
/*--------------------------------------------------------------------------*/
#ifndef _LEVEL_H_
#define _LEVEL_H_
/*--------------------------------------------------------------------------*/
#include <vector>
using namespace std;
#include "config.h"
/*--------------------------------------------------------------------------*/
extern vector<unsigned char * > apLevels;
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
extern unsigned char * level_pointer( int i, int x, int y );
//! This is 'friendlier' than using level_pointer() but slightly slower so where speed is sensitive don't use it [dj2017-06]
extern SLevelBlock     level_get_block( int i, int x, int y );
extern void            level_delete( int i );

/*--------------------------------------------------------------------------*/
#define LEVCHAR_FOREA(x,y) (*( g_pLevel + (y) * 512 + (x) * 4 + 0 ))
#define LEVCHAR_FOREB(x,y) (*( g_pLevel + (y) * 512 + (x) * 4 + 1 ))
#define LEVCHAR_BACKA(x,y) (*( g_pLevel + (y) * 512 + (x) * 4 + 2 ))
#define LEVCHAR_BACKB(x,y) (*( g_pLevel + (y) * 512 + (x) * 4 + 3 ))

#define CHECK_SOLID(a,b)  ( GET_EXTRA( (a), (b), EXTRA_FLAGS ) & FLAG_SOLID )

extern unsigned char *g_pLevel;
/*--------------------------------------------------------------------------*/

#endif

