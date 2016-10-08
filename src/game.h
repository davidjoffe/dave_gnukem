/*!
\file    game.h
\brief   The control of the game itself
\author  David Joffe

Copyright (C) 1995-2001 David Joffe

License: GNU GPL Version 2 (*not* "later versions")
*/
/*--------------------------------------------------------------------------*/
/* David Joffe '95/07/28 */
/* Game playing stuff for djg */
/*--------------------------------------------------------------------------*/
#ifndef _GAME_H_
#define _GAME_H_

class CBullet;

#include <vector>
using namespace std;

//! Visible blocks on X axis in game viewport
#define VIEW_WIDTH (12)
//! Visible blocks on Y axis in game viewport
#define VIEW_HEIGHT (10)

//! Tests if rectangle overlaps portion of world visible on screen (pixel coordinates)
#define OVERLAPS_VIEW(x1,y1,x2,y2) (OVERLAPS(x1,y1,x2,y2,16*xo,16*yo,16*(xo+VIEW_WIDTH)+(8*xo_small),16*(yo+VIEW_HEIGHT)))
//! Tests if point is directly inside portion of world visible on screen (pixel coordinates)
#define IN_VIEW(x,y) (INBOUNDS(x,y,xo*16,yo*16,16*(xo+VIEW_WIDTH)+(8*xo_small),16*(yo+VIEW_HEIGHT)))


class CThing;
extern vector<CThing *> g_apThings;

//! Add a \ref CThing to the current array of things in the level.
extern void AddThing(CThing *pThing);


#include "djsound.h"
enum EdjGameSounds
{
	SOUND_PICKUP=0,
	SOUND_SHOOT,
	SOUND_EXIT,
	SOUND_OPENDOOR,
	SOUND_EXPLODE,
	SOUND_MAX
};
extern SOUND_HANDLE g_iSounds[SOUND_MAX];

//! Global 4-animation-frame animation count. This name must also change.
extern int anim4_count;

// rtfb
extern int g_nLevel;

//----- Game control

//! Advance to next level
extern void NextLevel();
//! Restart current level
extern void RestartLevel();
//! Get current level (0-based index)
extern int GetCurrentLevel();
//! Lose a life
extern void Die();
//! The main game "tick" routine
extern void GameHeartBeat();

//----- Game screen redrawing

//! Draw the main game skin
extern void GameDrawSkin();
//! Draw the main game view
extern void GameDrawView();

//! Once off initialization stuff
extern void GameInitialSetup();
	//! Per-game initialization ("start gnu game")
	extern void PerGameSetup();
		//! Per-level initialization (i.e. for each map)
		extern void PerLevelSetup();
		//! Per-level cleanup
		extern void PerLevelCleanup();
	//! Per-game cleanup
	extern void PerGameCleanup();
//! Final cleanup
extern void GameFinalCleanup();

//! Add some value to hero firepower
extern void HeroModifyFirepower(int nDiff);
//! Set hero firepower
extern void HeroSetFirepower(int nFirepower);

//! Hero shoot. Creates a bullet.
extern void HeroShoot(int nX, int nY, int nXDiff, int nYDiff=0);
//! Monster shoot. Creates a monster bullet.
extern void MonsterShoot(int nX, int nY, int nXDiff, int nYDiff=0);


extern int GameLoadSprites(); // fixme , move into per-game init?

//! Save game (FIXME: Somewhat rudimentary at the moment. Needs more advanced, e.g. savegame slots, naming etc)
extern bool SaveGame();
//! Load game (FIXME: See SaveGame for comments)
extern bool LoadGame();

//! Display a message on-screen during gameplay for next nFrames frames (FIXME: Make this time)
extern void ShowGameMessage(const char *szMessage, int nFrames);


//! Return true if solid. bCheckThings is an ugly hack which (fixme) should be fixed
extern bool check_solid( int ix, int iy, bool bCheckThings=true );

//! Return true if given rectangle [pixel coordinates] is colliding with anything solid
extern bool CheckCollision(int x1, int y1, int x2, int y2, CBullet *pBullet=NULL);


extern int game_startup();
extern void parse_level();
extern void sprite_factory( unsigned char a, unsigned char b, int ix, int iy, int ifore, bool bfromlevel );

//! Set current score. On-screen score display is redrawn automatically.
extern void SetScore(int nScore);
//! Set current health. On-screen score display is redrawn automatically
extern void SetHealth(int nHealth);
//! Modify current score by given amount, optionally creating a "floating score" display
extern void update_score(int score_diff, int nFloatingScoreX=-1, int nFloatingScoreY=-1);
extern void update_health(int health_diff);

//! Set the hero state to "hurting", unless already hurting, unless bReset is set to true
extern void HeroSetHurting(bool bReset=false);
//! Return true if hero is "hurting" (flashing)
extern bool HeroIsHurting();

#endif
