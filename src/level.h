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
/*--------------------------------------------------------------------------*/
// These should be dynamic, to allow any size level. I initially made
// restrictions because of the memory limitations in the DOS world.
#define LEVEL_WIDTH  128
#define LEVEL_HEIGHT 100
#define LEVEL_SIZE   (4 * LEVEL_WIDTH * LEVEL_HEIGHT)
#define MAX_LEVELS   20
/*--------------------------------------------------------------------------*/
extern vector<unsigned char * > apLevels;
/*--------------------------------------------------------------------------*/
struct SLevelBlock
{
	unsigned char afore;
	unsigned char bfore;
	unsigned char aback;
	unsigned char bback;
};

void InitLevelSystem ();
void KillLevelSystem ();
extern unsigned char * level_load( int i, const char * szfilename );
int                    level_save( int i, const char * szfilename );
extern unsigned char * level_pointer( int i, int x, int y );
extern SLevelBlock     level_get_block( int i, int x, int y );
extern void            level_delete( int i );

#endif

