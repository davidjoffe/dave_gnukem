/*!
\file    game.h
\brief   The control of the game itself
\author  David Joffe

Copyright (C) 1995-2020 David Joffe
*/
/*--------------------------------------------------------------------------*/
/* [File created] David Joffe '95/07/28 */
/* Game playing stuff for djg */
/*--------------------------------------------------------------------------*/
#ifndef _GAME_H_
#define _GAME_H_

class CBullet;

#include "config.h"
#include <cstddef>//NULL
#include <vector>

//! Visible 'blocks' on X axis in game viewport
extern int VIEW_WIDTH;
//! Visible 'blocks' on Y axis in game viewport
extern int VIEW_HEIGHT;
extern int g_nViewOffsetX;//Top left of game viewport in pixels (X)
extern int g_nViewOffsetY;//Top left of game viewport in pixels (Y)

//! Tests if rectangle overlaps portion of world visible on screen (pixel coordinates)
#define OVERLAPS_VIEW(x1,y1,x2,y2) (OVERLAPS(x1,y1,x2,y2,BLOCKW*g_Viewport.xo,BLOCKH*g_Viewport.yo,BLOCKW*(g_Viewport.xo+VIEW_WIDTH)+(HALFBLOCKW*g_Viewport.xo_small),BLOCKH*(g_Viewport.yo+VIEW_HEIGHT)))
//! Tests if point is directly inside portion of world visible on screen (pixel coordinates)
#define IN_VIEW(x,y) (INBOUNDS(x,y,g_Viewport.xo*BLOCKW,g_Viewport.yo*BLOCKH,BLOCKW*(g_Viewport.xo+VIEW_WIDTH)+(HALFBLOCKW*g_Viewport.xo_small),BLOCKH*(g_Viewport.yo+VIEW_HEIGHT)))


class CThing;
extern std::vector<CThing *> g_apThings;

//! Add a \ref CThing to the current array of things in the level.
extern void AddThing(CThing *pThing);


#include "djsound.h"//SOUND_HANDLE
enum EdjGameSounds
{
	SOUND_PICKUP=0,
	SOUND_SHOOT,
	SOUND_EXIT,
	SOUND_OPENDOOR,
	SOUND_EXPLODE,
	SOUND_SHOOT2,
	SOUND_JUMP,
	SOUND_JUMP_LANDING,
	SOUND_SOFT_EXPLODE,
	SOUND_KEY_PICKUP,
	SOUND_MAX
};
extern SOUND_HANDLE g_iSounds[SOUND_MAX];

//! Global 4-animation-frame sprite animation count. This name must also change.
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
extern void GameHeartBeat(float fDT);

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

// [dj2016-10-28: bLoadGame is true if and only if loading game from *main menu* .. long story short, this is because
// the LoadGame() function assumes game being loaded *in-game* (as you can also load game from in-game menu) - this
// feels a bit spaghetti-ish to me, but will do for now.
extern int game_startup(bool bLoadGame=false);
extern void parse_level();
extern void sprite_factory( unsigned char a, unsigned char b, int ix, int iy, int ifore, bool bfromlevel );

//! Set current score. On-screen score display is redrawn automatically.
extern void SetScore(int nScore);
//! Set current health. On-screen score display is redrawn automatically
extern void SetHealth(int nHealth);
//! Modify current score by given amount, optionally creating a "floating score" display
extern void update_score(int score_diff, int nFloatingScoreXBlockUnits=-1, int nFloatingScoreYBlockUnits=-1);
extern void update_health(int health_diff);
extern void DrawHealth();
extern void DrawScore();

//! Set the hero state to "hurting", unless already hurting, unless bReset is set to true
extern void HeroSetHurting(bool bReset=false);
//! Return true if hero is "hurting" (flashing)
extern bool HeroIsHurting();

extern void ShowEndGameSequence();

#endif
