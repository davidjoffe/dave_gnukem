//
// game.cpp
//
// Created 1995/07/28
//
// Copyright (C) 1995-2024 David Joffe
//
/*--------------------------------------------------------------------------*/

#include "config.h"
#include "djfile.h"//dj2022-11
#include "djtypes.h"
#include "loadedlevel.h"//dj2023
#include "localization/djgettext.h"//pgettext
#include "djlang.h"
#include "effect_viewportshadow.h"//dj2022
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

#include <string>

#ifdef WIN32
#else
#include <unistd.h>
#endif

#ifdef djMAP_AUTO_DROPSHADOWS
#include <map>//For map auto-shadows' [dj2018-01]
#endif

#include "console.h"//SetConsoleMessage [dj2022-11 refactoring]
#include "mission.h"
#include "hero.h"
#include "inventory.h"
#include "thing.h"
#include "thing_monsters.h"//CDrProton
#include "graph.h"
#include "game.h"
#include "graph.h"
#include "block.h"
#include "level.h"
#include "djlog.h"
#include "djtime.h"
#include "djinput.h"
#include "djsprite.h"
#include "menu.h"
#include "keys.h"
#include "ed.h"
#include "bullet.h"
#include "hiscores.h"
#include "sys_log.h"
#include "djstring.h"//djStrPrintf
#include "instructions.h"//Slightly don't like this dependency. For ShowInstructions() from in-game menu. [dj2017-08]

#ifndef NOSOUND
#include "sdl/djinclude_sdlmixer.h"

Mix_Music* g_pInGameMusic=NULL;
#endif

// See comments at viewport vertical auto-scrolling. DN1 vertical viewport auto-re-centering has this subtle feel of almost taking a frame or three to start 'catching up' to jumping upwards etc. ... this variable helps implement that effect. [dj2017-06-29]
int g_nRecentlyFallingOrJumping = 0;
int g_nNoShootCounter = 0;

// The original Duke Nukem 1 has a 13x10 'blocks' viewport, though in future we could use this
// to either allow larger game viewport for this game, or have 'derived' games using this 'engine' with
// larger viewports [dj2016-10]
#ifdef LARGE_VIEWPORT_TEST
//! 'Large viewport mode' NOT TO BE CONFUSED WITH g_bBigViewportMode! TOTALLY DIFFERENT THINGS.
bool g_bLargeViewport = false;//true;
#endif
// Width/height of gameplay-area viewport (in number of game 'blocks'). The real Duke Nukem 1 was 13x10.
// These used to be constants, that's why they're all uppercase, but should be renamed according to normal variable naming conventions [dj2017-08]
int VIEW_WIDTH = -1;
int VIEW_HEIGHT = -1;
// Viewport drawing 'rectangle' in pixels (firstly offset in pixels from relative to top left of screen or gameview display buffer, then a width/height in pixels):
int g_nViewOffsetX = 0;//Top left of game viewport in pixels (X)
int g_nViewOffsetY = 0;//Top left of game viewport in pixels (Y)
int g_nViewportPixelW = 0;//dj2022-11
int g_nViewportPixelH = 0;


//[dj2016-10-10]
bool g_bBigViewportMode=false;

//[dj2016-10-10 [livecoding-streamed]] Trying to fix "Dying in the pungee sticks will often cause a crash"
// Problem in short was:
// * Game main loop iterating over 'things' and checking hero interaction with them through HeroOverlaps
// * CSpike::HeroOverlaps triggers 'update health (-1)
// * SetHealth on health reaching 0 would call Die()
// * Die() in turn would try restart the level [immediately], which would cause all g_apThings to be deleted [and new ones created etc.] - and this is the bad part - while we're still in the loop checking HeroOverlaps() with those things
// Solution: Die 'asynchronously' - i.e. just set a flag that you've died, and then (still immediately) but once 'safely' past iterating through all
// the g_apThings update ticks etc., *then* actually restartlevel.
bool g_bDied = false;

/*--------------------------------------------------------------------------*/
//
// Game cheats system (useful for development/testing)
//

#define DAVEGNUKEM_CHEATS_ENABLED

#ifdef DAVEGNUKEM_CHEATS_ENABLED
bool g_bGodMode = false;
#endif

/*--------------------------------------------------------------------------*/
//
// Forward declarations
//

// Draw hero firepower
void GameDrawFirepower();
// Bullet system
void DrawBullets(float fDeltaTime_ms);
// Draw debug info
void DrawDebugInfo();

void IngameMenu();

/*--------------------------------------------------------------------------*/
//
// Sound files
//

// These live under DATA_DIR but [dj2022-11] that's now prepended with djDATAPATHc at load time .. so removng the DATA_DIR prefixes again from here ..
const char *g_szSoundFiles[SOUND_MAX] =
{
	"sounds/pickup.wav",
	"sounds/shoot_cg1_modified.wav",//<- Hero shoot sound
	"sounds/exit.ogg",//<-dj2016-10-28 New proper exit sound (is "PowerUp13.mp3" by Eric Matyas http://soundimage.org/)
	"sounds/wooeep.wav",
	"sounds/explode.wav",
	"sounds/sfx_weapon_singleshot7.wav"//<- Monster shoot sound
	,"sounds/jump.wav"//dj2016-10-30
	,"sounds/jump_landing.wav"//dj2016-10-30
	,"sounds/soft_explode.wav"//dj2016-10-30
	,"sounds/key_pickup.wav"//dj2016-10-30
};
SOUND_HANDLE g_iSounds[SOUND_MAX]={0};

/*--------------------------------------------------------------------------*/

// Positions of various elements on screen

#define SCORE_X (28*8+8)
#define SCORE_Y (3*8)

#define HEALTH_X (28*8+8)
#define HEALTH_Y (7*8)

#define FIREPOWER_X (224+8)
#define FIREPOWER_Y (96)


// some game constants

// dj2022-11 comments: A question I sometime see is 'Why 18 frames per second' (and sometimes criticism) and of course many players feel that sucks by modern standards, the reason is twofold:
// (1) I want to mimic Duke Nukem 'look and feel' of gameplay
// (2) I always subjectively felt like DN1 was running 'around' 18Hz but never literally never confirmed this in any technical (I suspect they may have used the old IBM PC timer chip stuff which was 18Hz .. e.g. if you run DN1 in DosBox it doesn't run superfast it still runs 'correctly' at about the same speed it did in the old days)
// In theory though we can uncap the frame rate here for much higher frame rates (and potential smooth scrolling) a lot of other parts of the code need to be refactored a bit to handle that more correctly/differently, as currently if you uncap it it just everything runs super-fast but with the same "klunky" 8-pixel or 16-pixel 'jumps' in scrolling etc. (because it's a retro nostalgic experience mimicking DN1)

// Yeah its pretty low but thats what I was aiming for back in the EGA days
// and I don't feel like changing it right now. I might still though.
float g_fFrameRate=18.0f;
//float g_fFrameRate=18.2f;
//float g_fFrameRate = 36.f;
// Hmm https://retrocomputing.stackexchange.com/questions/1428/why-is-the-8254s-default-rate-18-2-hz should it be 18.2 not 18?
// [low history/background info: See also https://github.com/davidjoffe/dave_gnukem/discussions/161 "Notes on the 18Hz framerate"]

#define MAX_HEALTH (10)
#define HEALTH_INITIAL MAX_HEALTH

#define MAX_FIREPOWER (5)

unsigned char* g_pLevelShadowMap = NULL;
unsigned char *g_pLevel = NULL;
int g_nLevel = 0;
bool bShowDebugInfo = false;
bool g_bEnableDebugStuff = false;//dj2017-08-13 Enable 'cheats'/debug-settings at run-time (later might want to distinguish between 'cheats' and 'debug stuff' but for now treat the same - dj2018)
std::vector<CThing *> g_apThings;

std::string g_sGameMessage;
int g_nGameMessageCount = -1;

std::vector<CBullet*> g_apBullets;
std::vector<CBullet*> g_apBulletsDeleted;//This is perhaps slightly kludgy but the purpose of this is to draw bullets 'one last frame' just as/after they've been destroyed when they collide with something - looks better visually I think - dj2018-01-12/13 (see livestream of same date) ... in theory these could even 'look different' later but that's very low prio

void DestroyBullets(std::vector<CBullet*>& apBullets)
{
	for ( unsigned int i=0; i<apBullets.size(); ++i )
	{
		delete apBullets[i];
	}
	apBullets.clear();
}
// Completely cleanup/destroy all active bullets (for level init/re-init/cleanup). [dj2018-03-31] Add this to fix bug: Bullets that you fire persist and keep going and shooting things in the loaded game if you fire bullets, then do a loadgame while they're in the air
void DestroyAllBullets()
{
	DestroyBullets(g_apBullets);
	DestroyBullets(g_apBulletsDeleted);
}

//dj2018-03-31
void DestroyAllThings()
{
	for ( unsigned int i=0; i<g_apThings.size(); ++i )
	{
		delete g_apThings[i];
	}
	g_apThings.clear();
}


// Return number of bullets in bullet system that were fired by hero
int CountHeroBullets()
{
	unsigned int i;
	int nCount = 0;
	for ( i=0; i<g_apBullets.size(); ++i )
	{
		if (g_apBullets[i]->eType==CBullet::BULLET_HERO)
			++nCount;
	}
	return nCount;
}

std::vector<float> afTimeTaken;
#define MAX_DEBUGGRAPH 128

const char *DATAFILE_GAMESKIN = "gameskin.tga";
djImage *pSkinGame        = NULL; // Main game view skin (while playing)
djImage *pBackground      = NULL; // Level background image

// Game effects [dj2018-01]
// 'Map auto-shadows' [dj2018-01]
const char* DATAFILE_SHADOWS = "shadows.tga";
djImage* g_pImgShadows = NULL;

// Use config options to turn on/off specific effects if need be or desired (ideally these shouldn't be ugly globals, should have some sort of generic properties type thing - LOW prio - dj2018)
bool g_bAutoShadows = true;
bool g_bSpriteDropShadows = true;

/*--------------------------------------------------------------------------*/

//global 4-block sprite animation offset counter for various things
int anim4_count = 0;
// ^ dj2022-11 this slightly gross global stems from the earliest earliest code .. it should be slightly compartmentalized into 'DG1 specific' stuff as this is one of the reasons we can't switch meaningfully to smooth scrolling and fast frame rates right this second.




//game-play
unsigned int g_nScore = 0;
int  g_nHealth = 0;
bool g_bGameRunning = false;
int  key_action= 0;
int  key_left  = 0;
int  key_right = 0;
int  key_jump  = 0;
int  key_shoot = 0;

int g_nFirepower = 1;
int g_nFirepowerOld = 1;
int g_nScoreOld = 0;
int g_nHealthOld = 0;


/*-----------------------------------------------------------*/
#if defined(djINGAME_FULLSCREEN_TOGGLE) && defined(djDEV_STRESSTESTS)
//dj2022-11 add quick n dirty stress-tester to help test in-game fullscreen toggle robustness  [todo lwo move stress tests to seprate .cpp files?]
bool djStressTestInGameFullscreenToggle(float fDT)
{
	static bool g_bStressTestFullscreenToggle = true;
	static float g_fStressTestFullscreenToggleTimer = 0.f;
	if (g_bStressTestFullscreenToggle)
	{
		g_fStressTestFullscreenToggleTimer += fDT;
		if (g_fStressTestFullscreenToggleTimer >= 2.f)
		{
			g_fStressTestFullscreenToggleTimer = 0.f;
			//dj2022-11 experimental toggle fullscreen probably going to crash a lot
			djGraphicsSystem::ToggleFullscreen();
			return true;
		}
	}
	return false;
}
#endif
/*-----------------------------------------------------------*/
//
// GameHeartBeat() helpers
//

//! GameHeartBeat() helper. These params seem a bit weird to me, design-wise[low][dj2018-01]
void GameViewportAutoscroll(bool bFalling, bool bFallingPrev)
{
	// If we're right at the end of the game tackling Dr Proton, and he starts
	// escaping, then we want the viewport to briefly 'center around' / follow
	// Dr Proton as he flies upwards to escape (as per original DN1). So we
	// check here. Else, we center around the hero as per normal. [dj2017-08-09]
	// (Not sure I'm mad about this dependency but it'll probably do, there
	// are worse things to worry about in life [dj2017-08])
	if (CDrProton::GameEnding())
	{
		CDrProton* pDrProton = CDrProton::GetDrProton();
		if (g_Viewport.yo + 2 > pDrProton->m_y)
			--g_Viewport.yo;
	}
	else
	{

		// Viewport auto-scrolling (horizontal)

		//[was:onmoveright]
		//if (
		if (g_Player.x >= g_Viewport.xo + VIEW_WIDTH)//Totally at/off right side of view?
		{
			// Snap to it
			g_Viewport.xo = g_Player.x - VIEW_WIDTH;
			g_Viewport.xo_small = g_Player.x_small != 0 ? 1 : 0;
		}
		if ((g_Player.x - g_Viewport.xo) >= VIEW_WIDTH - 5)
		{
			// This stuff relates to trying to get similar 'retro' viewport scrolling behavior to DN1
			// Not sure it's 100% right but seems sorta 'close enough' at this point (dj2019)
			// This code's a bit ugly as it's based on my really old/early code.
			// xo_offset = 'Viewport offset by 8 pixels?'
			// One must look at DN1's scrolling behavior to see the intention here also.
			// Baaasically the DN1 blocks were 16 pixels but the horizontal *viewport offset* (as
			// well as the hero horizontal position) could be offset by either a further 8 pixels,
			// or no further offset, i.e. aligned to the 16-pixel block positions.)
			// If we want to use the dave_gnukem code for more generic purposes,
			// eg totally smooth-animating and/or higher-resolution platforms, this is one of
			// the parts of the code we'd first want to change/genericize etc.

			//bool bEven = (((x-xo)%2)==0);

			/*if (!bEven & (x_small==0))
			{
			++xo;
			xo_small = 0;
			}
			*/
			if (((g_Player.x - g_Viewport.xo) == VIEW_WIDTH - 5) & (g_Player.x_small)) {
				g_Viewport.xo_small = 1;
			}
			if ((g_Player.x - g_Viewport.xo) >= VIEW_WIDTH - 4) {
				++g_Viewport.xo;
				g_Viewport.xo_small = 0;
			}
			/*else
			{
			++g_Viewport.xo;
			g_Viewport.xo_small = 1;
			}*/

			// (//dj2019-07) hm fixmeHIGH i suspect for low LEVEL_WIDTH we may have issues here with xo being negative, or is that OK
			if ( (g_Viewport.xo + g_Viewport.xo_small) > LEVEL_WIDTH - VIEW_WIDTH )
			{
				g_Viewport.xo = LEVEL_WIDTH - VIEW_WIDTH;
				g_Viewport.xo_small = 0;
			}
		}

		//[was:onmoveleft]
		// If our x position is left of the viewport X origin (i.e. hero entirely outside viewport and would thus be invisible), 'force'/'snap'/reset viewport origin as a-few-blocks-left of hero
		if (g_Player.x <= g_Viewport.xo)
		{
			g_Viewport.xo = g_Player.x - 4;
			g_Viewport.xo_small = 0;
		}
		// If our hero is close to left of viewport and we maybe need to adjust the horizontal scrolling
		else if (((g_Player.x - g_Viewport.xo) <= 4))
		{
			bool bEven = (((g_Player.x - g_Viewport.xo) % 2) == 0);
			if (bEven & (!(g_Player.x_small))) {
				g_Viewport.xo_small = 0;
			}
			// dj2019-06 NB: This was "if (!bEven && (x_small))"; changing it based on a compiler warning from Ubuntu. I don't even know anymore (as some of this code is 20+ years old) if the intention was to do this bitwise or int-wise etc. but I don't think it matters, I think end result is the same. Nonetheless, if we suddenly have strange viewport scrolling behavior after this change, change it back or come back to this.
			if ((!bEven) && (g_Player.x_small!=0)) {
				g_Viewport.xo--;
				g_Viewport.xo_small = 1;
			}
			if (g_Viewport.xo < 0)
			{
				g_Viewport.xo = 0;
				g_Viewport.xo_small = 0;
			}
		}


		// Viewport auto-scrolling (vertical).
		// Try auto-scroll viewport if we're going too high/low .. the actual original DN1 behaviour seems to be quite well fine-tuned.
		// The precisely 'correct' behavior is actually pretty subtle, hard to explain exactly what the below's trying to achieve, must play
		// original DN1 or check livestreaming dev video from 29 June 2017 where this was mainly done. [dj2017-06-29]
		// Note also here we must keep in mind cases like where you jump up and almost immediately hit your head on the roof -
		// the jump 'cancels' but simultaneously we're also not falling - this creates one frame where we're neither jumping nor falling
		// as hero's head hits roof, and without this g_nRecentlyFallingOrJumping "buffer" the vertical offset auto-scrolling incorrectly kicks in.
		{
			// 'Avoid' scroll yo (unless 'necessary' e.g. if right at top) up if busy jumping up ... likewise for downward movement
			if (g_Player.hero_mode == MODE_JUMPING)
			{
				if (g_Player.y - g_Viewport.yo < 2) g_Viewport.yo--;
				g_nRecentlyFallingOrJumping = 2;
			}
			else
			{
				bool bIsFalling = bFalling || bFallingPrev || g_Player.hero_mode==MODE_JUMPING;
				if (bIsFalling)
				{
					g_nRecentlyFallingOrJumping = 2;
					if (g_Player.y - g_Viewport.yo >= 9) g_Viewport.yo++;
				}
				else
				{
					if (g_nRecentlyFallingOrJumping>0)
					{
						--g_nRecentlyFallingOrJumping;
					}
					else
					{
						if (g_Player.y - g_Viewport.yo < 7)
						{
							g_Viewport.yo--;
						}
					}
					if (g_Player.y - g_Viewport.yo >= 7)
					{
						g_Viewport.yo++;
					}
				}
			}
			if ( g_Viewport.yo < 0 )
				g_Viewport.yo = 0;
			if ( g_Viewport.yo > LEVEL_HEIGHT - 10 )
				g_Viewport.yo = LEVEL_HEIGHT - 10;
		}
	}
}
//! GameHeartBeat() helper
void InteractWithThings()
{
	CThing* pThing=NULL;
	for ( int i=0; i<(int)g_apThings.size(); ++i )
	{
		pThing = g_apThings[i];
		if (pThing->OverlapsBounds(HERO_PIXELX, HERO_PIXELY))
		{
			// [dj2016-10-10] Note that if inside HeroOverlaps(), it can cause you to die, e.g. if you've interacted with
			// spikes .. so be aware you may be dead after calling that .. thats g_bDied, which causes level restart below.
			int nRet = pThing->HeroOverlaps();
			if (nRet==THING_DIE)
			{
				delete pThing;
				g_apThings.erase(g_apThings.begin() + i);
				i--;
				pThing = NULL;
			}
			else if (nRet==THING_REMOVE)
			{
				g_apThings.erase(g_apThings.begin() + i);
				i--;
				pThing = NULL;
			}
		}
		// If thing wasn't deleted on HeroOverlaps, check bounds enter/leave
		if (pThing!=NULL)
		{
			// Test if entering or leaving action bounds box
			if (pThing->HeroInsideActionBounds(HERO_PIXELX, HERO_PIXELY))
			{
				if (!pThing->IsHeroInside())
					pThing->HeroEnter();
			}
			else // not in bounds
			{
				if (pThing->IsHeroInside())
					pThing->HeroLeave();
			}
		}
	}
}
//! GameHeartBeat() helper
void TickAllThings(float fDeltaTime_ms)
{
	CThing* pThing = NULL;
	for ( int i=0; i<(int)g_apThings.size(); ++i )
	{
		pThing = g_apThings[i];
		// FIXME: THING_REMOVE?
		if (pThing->Tick(fDeltaTime_ms)==THING_DIE)
		{
			// Delete this
			delete pThing;
			g_apThings.erase(g_apThings.begin() + i);
			i--;
		}
	}
}
//! GameHeartBeat() helper
void DropFallableThings()
{
	CThing* pThing = NULL;
	// Note i may decrement during the loop (if thing deleted)
	for ( int i=0; i<(int)g_apThings.size(); ++i )
	{
		pThing = g_apThings[i];
		// if (object falls) && (nothing below it) && (inview)
		if (pThing->Falls())
		{
			// If thing is in visible view
			// FIXME: Small border around view?
			if (pThing->IsInView())
			{
				// if (nothing below it)
				if ( !check_solid( pThing->m_x, pThing->m_y + 1 ) )
				{
					pThing->m_y += 1;
					// if object falls off bottom of level
					if (pThing->m_y >= LEVEL_HEIGHT)
					{
						// delete this object!!!
						delete pThing;
						g_apThings.erase(g_apThings.begin() + i);
						i--;
					}
				}
			}
		}
	}
}
//! GameHeartBeat() helper
void CheckForBulletsOutOfView()
{
	CBullet* pBullet=NULL;
	// Check for bullets that have gone out of the view
	for ( int i=0; i<(int)g_apBullets.size(); ++i )
	{
		pBullet = g_apBullets[i];
		if (!OVERLAPS_VIEW(
			pBullet->x,
			pBullet->y,
			pBullet->x + BULLET_WIDTH-1,
			pBullet->y + BULLET_HEIGHT-1))
		{
			djDEL(pBullet);
			g_apBullets.erase(g_apBullets.begin() + i);
			i--;
		}
	}
}
//! GameHeartBeat() helper
void CheckIfHeroShooting()
{
	if (key_shoot)
	{
		// [dj2017-12] Completely change the firepower behaviour:
		// In short, the logic is, we can't shoot for N frames,
		// where it's a higher number of frames to wait if we
		// have less firepower. There's an exception, if we run
		// out of active bullets then it resets to (basically 3) frames
		// to wait so 'almost immediately' ... this is tweaked to
		// be nearly in line with DN1 behaviour [see stream
		// of 31 December 2017]. For future games, we might want to
		// deviate more from this DN1 style behaviour - for version 1
		// though I want to try stick close to the look n feel of DN1.
		// This may still need some slight tweaking.
		if (g_nNoShootCounter==0)
		{
			#define HERO_BULLET_SPEED (16)


			//dj2019-07 this is to multiply the (silly(?)) offset if we're using blocksizes larger than 16x16 .. this is a bit crude. The offset itself is a bit DG1/DN1 specific, which was 'supposed to be' 16x16 only, but for fun we're doing 32x32 etc., and to make source more generic.
			int nMultiple=1;
			if (BLOCKW>16)
				nMultiple = (BLOCKW/16);

			// The start X position of the bullet may be a bit imperfect here :/ .. but now that version 1's been released, probably best not to mess with it (at least for DG v1) - dj2020-06
			HeroShoot(
				g_Player.x * BLOCKW /*+ (g_Player.hero_dir==1 ? BLOCKW : -BLOCKW)*/ + g_Player.x_small*HALFBLOCKW,
				(g_Player.y-1)*BLOCKH + (11*nMultiple),
				// If hero facing left, start bullet speed in negative X axis direction (shooting left), else positive (shooting right)
				(g_Player.hero_dir==0 ? -HERO_BULLET_SPEED : HERO_BULLET_SPEED)
			);

			// RESET COUNTER [fixme this must reset between games also]
			g_nNoShootCounter = 3 + (MAX_FIREPOWER - g_nFirepower);
		}
		// dj2017-12-31 I'm not quite sure if this quite right.
		// I realized that one big difference with DN1 is that they only
		// shoot on-key-down ..... I am not quite sure I want to change
		// Dave Gnukem to be shoot-on-key-down-only at this late stage,
		// actually I don't want to, I think I want to deviate from the
		// original in that regard, as it's been like this for 20+ years.
		if (CountHeroBullets()==0 && g_nNoShootCounter>2)
			g_nNoShootCounter = 3;

		if (g_nNoShootCounter<0) g_nNoShootCounter = 0;

	}
	if (g_nNoShootCounter != 0)
		g_nNoShootCounter--;
}
//! GameHeartBeat() helper
void UpdateBullets(float fDeltaTime_ms)
{
	CBullet* pBullet=NULL;



	for ( int i=0; i<(int)g_apBullets.size(); ++i )//I think we must use signed here because we do --i in the loop
	{
		pBullet = g_apBullets[i];
		pBullet->Tick(fDeltaTime_ms);

		// NB This only handles simple-case 'horizontal-moving' bullets -
		// for now that's fine, but if we ever make this a generic
		// 'engine' we'll probably have to handle more complex cases,
		// e.g. vertical bullets, or even arbitrarily-angled bullets ...
		// (that last case would probably resemble a 'line algorithm') [dj2018-01]
		int nXPixelDelta = pBullet->dx < 0 ? -1 : 1;
		int nY = pBullet->y;
		int nXNumPixelsToMove = djABS(pBullet->dx);
		for ( int n=0; n<nXNumPixelsToMove; ++n )
		{
			// [dj2018-01]
			// The idea is sort of 'try' move a pixel at a time in the directon the
			// bullet is moving (without drawing at every single pixel movement of
			// course - just move 'in the model') and do collision detection at every
			// pixel movement to determine the 'exact pixel' at which we 'should'
			// collide with something like a wall. This means that we can in theory
			// even create highly fast-moving bullets (e.g. that move 100 pixels
			// in a frame even though 16 pixels wide) and they should e.g. 'correctly'
			// not 'skip over' a small object it collides with, and also if e.g.
			// colliding 40 pixels on, should show the visuals etc. for collision
			// at 40 pixels along. To move it 1 pixel at a time is of course slightly
			// slower but I doubt this is a concern on modern machines. [dj2018-01-12]
			int nXOld = pBullet->x;
			// Try move bullet one pixel horizontally
			pBullet->x += nXPixelDelta;




			bool bBulletDeleted = false;
			// Check for bullet collisions with things (e.g. shootable boxes, monsters etc).
			// Also we check for monster bullet collisions against hero.
			CThing* pThing=NULL;
			if (pBullet->eType==CBullet::BULLET_HERO)
			{
				for ( int k=0; k<(int)g_apThings.size(); ++k )
				{
					pThing = g_apThings[k];
					if (pThing->IsShootable())
					{
						//fixmeHIGH this + 8 makes NO SENSE to me what the hell is
						// it doing here!?!? [dj2018-01]
						int x1 = pBullet->x;//pBullet->dx<0 ? pBullet->x + 8 : pBullet->x;
						if (pThing->OverlapsShootArea(
							x1,
							pBullet->y,
							x1 + BULLET_WIDTH-1,//fixmeHIGH this makes no sense shoudl be + 15?? Shoudl be, BULLET_WIDTH?
									// Why the F is it 7? It's possible the sprite used to be smaller, or perhaps
									// it has something to do  with that + 8 above...
							pBullet->y+BULLET_HEIGHT-1))
						{
							int nRet = pThing->OnHeroShot();
							if (nRet==THING_DIE)
							{
								delete pThing;
								g_apThings.erase(g_apThings.begin() + k);
								k--;
							}
							else if (nRet==THING_REMOVE)
							{
								g_apThings.erase(g_apThings.begin() + k);
								k--;
							}

							// delete bullet i
							bBulletDeleted=true;
							g_apBulletsDeleted.push_back(pBullet);//delete g_apBullets[i];
							g_apBullets.erase(g_apBullets.begin() + i);
							i--;
							goto NextBullet1b;
						}
					}
				} // k
			}
			else if (pBullet->eType==CBullet::BULLET_MONSTER)
			{
				// Check if monster bullet overlaps with hero
				if (OVERLAPS(
					HERO_PIXELX,
					// fixmehigh2019-07 this actually looks like a bug! A possibly imporrant but why not add hero y_offset here!??
					g_Player.y*BLOCKH-BLOCKH,// FIXMEHIGH2019-07 WHY NOT HERO_PIXELY which includes the y_offset??
					HERO_PIXELX + (HEROW_COLLISION-1),
					(g_Player.y*BLOCKH-BLOCKH) + (HEROH_COLLISION-1),
					pBullet->x,
					pBullet->y,
					pBullet->x+(BLOCKW-1),
					// fixme is this +15(BLOCKW-1) right? looks too big .. (dj2019-07)
					// fixme is this +15 right? looks too big .. (dj2019-07)
					// fixme is this +15 right? looks too big .. (dj2019-07)
					// fixme is this +15 right? looks too big .. (dj2019-07)
					// fixme is this +15 right? looks too big .. (dj2019-07)
					pBullet->y+(BLOCKH-1)))
				{
					bBulletDeleted=true;
					g_apBulletsDeleted.push_back(pBullet);//delete g_apBullets[i];
					g_apBullets.erase(g_apBullets.begin() + i);
					i--;
					if (!HeroIsHurting())
					{
						update_health(-1);
						HeroSetHurting();
					}
				}
			}

		NextBullet1b:
			;



			// Break out of 'n' loop if bullet 'i' was deleted due to collision
			if (bBulletDeleted)
				break;








			// Check if bullet would touch anything solid at new position
			int x1 = pBullet->x;//pBullet->dx<0 ? pBullet->x + 8 : pBullet->x;
			if (CheckCollision(
				x1,
				nY,
				x1+BULLET_WIDTH -1,
				nY+BULLET_HEIGHT-1, pBullet))
			{
				// [dj2018-01] This if check is so we create the explosion centred around the 'tip' of where the bullet collided
				if (pBullet->dx<0)
				{
					AddThing(CreateExplosion(nXOld - HALFBLOCKW, nY-4,
						// Make the hero's bullet slightly larger than smallest explosion
						g_apBullets[i]->eType==CBullet::BULLET_HERO ? 1 : 0
					));
				}
				else
				{
					AddThing(CreateExplosion(nXOld + BLOCKW - HALFBLOCKW, nY-4,
						// Make the hero's bullet slightly larger than smallest explosion
						g_apBullets[i]->eType==CBullet::BULLET_HERO ? 1 : 0
					));
				}
				g_apBulletsDeleted.push_back(pBullet);
				g_apBullets.erase(g_apBullets.begin() + i);
				--i;
				break;//NB, break out of the 'n' loop now as bullet is dangling
			}











		}//n
	}//i(bullets)
}
/*-----------------------------------------------------------*/
// Redraw everything that needs to be redrawn, as larger viewport will have obliterated right side with score etc.
void RedrawEverythingHelper()
{
	//fixLOW[dj2017-08-13] really not 100% if all this is exactly correct but anyway

	//dj2019 Clear the back buffer when flipping between viewportmodes eg g_bBigViewportMode/g_bLargeViewport stuff so we don't in some cases potentially leave junky stuff from the previous mode, if the new mode only renders to a sub-portion of the backbuffer
	//if (g_bLargeViewport || g_bBigViewportMode)
	{
		// Clear back buffer [dj2019]
		djgSetColorFore(pVisBack, djColor(0, 0, 0));
		djgDrawBox(pVisBack, 0, 0, pVisBack->width, pVisBack->height);
	}

	GameDrawSkin();
	//dj2019-06 Move DrawHealth(),score,firepower,inventory before GraphFlipView for big/large viewport modes to work correctly, not sure if that's right.
	DrawHealth();
	DrawScore();
	GameDrawFirepower();
	InvDraw();
	GraphFlipView(g_nViewportPixelW, g_nViewportPixelH, g_nViewOffsetX, g_nViewOffsetY, g_nViewOffsetX, g_nViewOffsetY);
	GraphFlip(!g_bBigViewportMode);
}
/*-----------------------------------------------------------*/
void ReInitGameViewport()
{
	// NB keep in mind 'big viewport mode' and 'large viewport' mode are
	// two totally different things, not to be confused with one another.
	// 'Big viewport mode' runs the game in a large HIGH RESOLUTION
	// window, keeping 1:1 pixel ratio for sprites (so sprites draw tiny
	// and you see a lot more game area); large/wide viewport mode
	// still scales the game viewport so it's effectively 320x200,
	// but just uses up more area of the screen (i.e. instead of having
	// the big right-bar area for score and health etc. and the border
	// around the gameplay viewport, that space is used for gameplay area).

	if (g_bBigViewportMode)
	{
		//dj2019-07 should refine later but for now make it, use all pixels except a ring around the viewport one 'gameblock' size ..
		VIEW_WIDTH = (pVisView->width / BLOCKW) - 2;// - 10;
		VIEW_HEIGHT = (pVisView->height / BLOCKH) - 2;// - 5*16) / 16;
		//Top left of game viewport in pixels:
		g_nViewOffsetX=BLOCKW;//?? or should these be 0, probably [dj2017-08 ??]
		g_nViewOffsetY=BLOCKH;//??
	}
	else if (g_bLargeViewport)
	{
		// This is basically 320x200 divided by the game blocksize
		VIEW_WIDTH = 20;
		VIEW_HEIGHT = 13;//<- Note last block will be only half
		//Top left of game viewport in pixels:
		g_nViewOffsetX=0;
		g_nViewOffsetY=0;
	}
	else
	{
		// The real Duke Nukem 1 was 13x10 (in theory a larger viewport might be 'more fun' but we want to stick to the spirit of the original? Debatable .. dj2017-08):
		VIEW_WIDTH = 13;
		VIEW_HEIGHT = 10;
		//Top left of game viewport in pixels:
		g_nViewOffsetX=16;
		g_nViewOffsetY=16;

		// NB, TODO, we actually need to also need to redraw score etc. here (though since this is just a dev/editing mode, not a real game mode, it doesn't have to be perfect)

		// When going out of 'big viewport' mode, hero might now be off the (now-tiny) 'viewport' :/ .. so must also 're-center' viewport around hero
		//if (x>xo+VIEW_WIDTH/2) xo = x-VIEW_WIDTH/2;
		//if (y>yo+VIEW_HEIGHT/2) yo = y-VIEW_HEIGHT/2;
	}

	//dj2019-MMtest//VIEW_WIDTH = 32;
	//dj2019-MMtest//VIEW_HEIGHT = 16;
	//Top left of game viewport in pixels:
	//dj2019-MMtest//g_nViewOffsetX=0;
	//dj2019-MMtest//g_nViewOffsetY=0;

	// If very high resolution then in theory VIEW_WIDTH could be wider than the level, we don't want that or bad things will happen, so clamp to level dimensions:
	if (VIEW_WIDTH > LEVEL_WIDTH) VIEW_WIDTH = LEVEL_WIDTH;
	if (VIEW_HEIGHT > LEVEL_HEIGHT) VIEW_HEIGHT = LEVEL_HEIGHT;

	//dj2022-11 experimenting with increasing flexibility of viewport dimensions (changing to pixels from game block units)
	//g_nViewportPixelW = VIEW_WIDTH * BLOCKW - (BLOCKW - nExtraPartialBlockPixelsX);
	//g_nViewportPixelH = VIEW_HEIGHT * BLOCKH - (BLOCKH - nExtraPartialBlockPixelsY);
	g_nViewportPixelW = VIEW_WIDTH * BLOCKW;
	g_nViewportPixelH = VIEW_HEIGHT * BLOCKH;
}
/*-----------------------------------------------------------*/


/*-----------------------------------------------------------*/
// Game functions
/*-----------------------------------------------------------*/
// Once off initialization stuff
void GameInitialSetup()
{
	SYS_Debug( "GameInitialSetup()\n" );
	
	// Not quite sure where this should go. Does need to be at least after pVisView created. [dj2017-08]
	ReInitGameViewport();

	InitHighScores();
	// Prepend usersettings folder
	std::string sFilenameHighScores = djAppendPathStr(djGetFolderUserSettings().c_str(), USERFILE_HIGHSCORES);
	LoadHighScores(sFilenameHighScores.c_str());		// Load high scores

	SYS_Debug ( "GameInitialSetup(): loading sounds\n" );
	// Load the game sounds
	int i;
	for ( i=0; i<SOUND_MAX; i++ )
	{
		g_iSounds[i] = djSoundLoad(djDATAPATHc(g_szSoundFiles[i]) );
	}

	// Load main game skin
	if (!pSkinGame)
	{
		pSkinGame = new djImage;
		pSkinGame->Load(djDATAPATHc(DATAFILE_GAMESKIN));
		djCreateImageHWSurface( pSkinGame );
	}

	// Load map shadow effects sprite [new/experimental dj2018-01]
	if (!g_pImgShadows)
	{
		g_pImgShadows = new djImage;
		g_pImgShadows->Load(djDATAPATHc(DATAFILE_SHADOWS));
		djCreateImageHWSurface( g_pImgShadows );
	}

#ifdef djEFFECT_VIEWPORTSHADOW
	g_Effect.InitEffect();
#endif

	// Register the "thing"'s that need to be registered dynamically at runtime [dj2017-07-29]
	RegisterThings_Monsters();
}

// Final cleanup
void GameFinalCleanup()
{
	SYS_Debug( "GameFinalCleanup()\n" );

#ifdef djEFFECT_VIEWPORTSHADOW
	g_Effect.CleanupEffect();
#endif

	djDestroyImageHWSurface(g_pImgShadows);
	djDEL(g_pImgShadows);

	djDestroyImageHWSurface(pSkinGame);
	djDEL(pSkinGame);

	// Unload the game sounds (FIXME)

	KillHighScores();
}


// Per-game initialization
void PerGameSetup()
{
	djLOGSTR("PerGameSetup(): InitLevelSystem()\n");
	InitLevelSystem();

	g_nHealth = HEALTH_INITIAL; // Initial health
	HeroSetJumpMode(JUMP_NORMAL);

	g_nLevel = 0;
	g_bDied = false;

	g_nScore = 0;
	g_nFirepower = 1;

	djMSG( "PerGameSetup(): Loading sprites\n" );
	if (0 != GameLoadSprites())
	{
		djMSG( "PerGameSetup(): error loading sprites.\n" );
		return;
	}
}

void PerLevelSetup()
{
	djLOGSTR( "PerLevelSetup()\n" );

	g_nRecentlyFallingOrJumping=0;
	g_nNoShootCounter = 0;

	// Start per-level background music
	std::vector< std::string > asMusicFiles;

	/*//these are sorted smallest to largest .. for now just selected all except the smaller ones as the shorter ones will may drive you too crazy with the looping..
	// not yet sure if even the longer ones will be too repetitive [dj2016-10]
	asMusicFiles.push_back("Dont-Mess-with-the-8-Bit-Knight.ogg");
	asMusicFiles.push_back("Futureopolis.ogg");
	asMusicFiles.push_back("Farty-McSty.ogg");
	asMusicFiles.push_back("Attack-of-the-8-Bit-Hyper-Cranks.ogg");
	asMusicFiles.push_back("The-8-bit-Princess.ogg");
	asMusicFiles.push_back("Mister-Snarkypants.ogg");
	asMusicFiles.push_back("Classy-8-Bit.ogg");
	asMusicFiles.push_back("80s-Space-Game-Loop_v001.ogg");
	asMusicFiles.push_back("Good-Morning-Doctor-Weird.ogg");
	asMusicFiles.push_back("Crazy-Candy-Highway-2.ogg");
	asMusicFiles.push_back("Cyber-Dream-Loop.ogg");
	asMusicFiles.push_back("The-Furry-Monsters-Laboratory.ogg");
	asMusicFiles.push_back("World-of-Automatons_Looping.ogg");
	asMusicFiles.push_back("Castle-8-Bit-Stein.ogg");*/
	asMusicFiles.push_back("Insane-Gameplay_Looping.ogg");
	asMusicFiles.push_back("Dystopic-Mayhem.ogg");
	asMusicFiles.push_back("Mad-Scientist_Looping.ogg");
	asMusicFiles.push_back("Monkey-Drama.ogg");
	//[used in main menu, though am totally opening to changing that]asMusicFiles.push_back("8-Bit-Mayhem.ogg");
	asMusicFiles.push_back("The-Darkness-Below_Looping.ogg");
	asMusicFiles.push_back("Techno-Caper.ogg");
	asMusicFiles.push_back("Funky-Gameplay_Looping.ogg");
	asMusicFiles.push_back("Escape_Looping.ogg");
	asMusicFiles.push_back("Monster-Street-Fighters.ogg");
	asMusicFiles.push_back("Monsters-in-Bell-Bottoms_Looping.ogg");
	asMusicFiles.push_back("Retro-Frantic_V001_Looping.ogg");
	asMusicFiles.push_back("Techno-Gameplay_Looping.ogg");
#ifndef NOSOUND
	// This is somewhat gross quick n dirty simplistic for now - should rather have ability to assign music file in the level file format [dj2016-10]
	int nMusicFile = (g_nLevel % asMusicFiles.size());
	std::string sBasePath = djDATAPATH("music/eric_matyas/");
	if (g_pInGameMusic!=NULL)
	{
		Mix_FreeMusic(g_pInGameMusic);
		g_pInGameMusic = NULL;
	}
	g_pInGameMusic = Mix_LoadMUS((sBasePath + asMusicFiles[nMusicFile]).c_str());
	if (g_pInGameMusic!=NULL)
	{
		Mix_FadeInMusicPos(g_pInGameMusic, -1, 500, 0);
	}
#endif

	// Save current score and firepower - these must be restored if we die.
	g_nScoreOld = g_nScore;
	g_nFirepowerOld = g_nFirepower;
	g_nHealthOld = g_nHealth;

	g_bDied = false;

	// (1) Initialize all relevant variables for entering a new level

	// Key states variables
	key_action= 0;
	key_left  = 0;
	key_right = 0;
	key_jump  = 0;
	key_shoot = 0;

	g_nHealth = HEALTH_INITIAL;

	HeroReset(); // Reset hero

	anim4_count=0; // animation count 0
	hero_picoffs=0;
	g_nHeroJustFiredWeaponCounter = 0;
	// just in case level doesn't contain a starting block ..
	g_Viewport.xo = 0;
	g_Viewport.yo = 0;
	g_Viewport.xo_small = 0; // view not half-block offset
	relocate_hero( LEVEL_WIDTH/2, LEVEL_HEIGHT/2 );
	g_Player.hero_dir = 1;

	DestroyAllThings();// clear list of "things"
	DestroyAllBullets();//Make sure no bullets, for good measure [dj2018-03]

	// (2) Load the currently selected level
	const char * szfilename = g_pCurMission->GetLevel( g_nLevel )->GetFilename( );

	// always keep current level loaded at slot 0
	SYS_Debug ("PerLevelSetup(): level_load( %s )\n", szfilename );
	if (NULL == level_load( 0, szfilename ))
	{
		djMSG("PerLevelSetup(): error loading level %s.\n", szfilename );
		//dj2019-07 This should be just a warning but it should let you play, with some default placement position.
		ShowGameMessage("BAD FILENAME FOR LEVEL", 1000);
		relocate_hero( LEVEL_WIDTH/2, LEVEL_HEIGHT/2 );
		//return;
	}
	g_pLevel = apLevels[0];

#ifdef djMAP_AUTO_DROPSHADOWS
	/*
	HOW THIS WORKS:
	[See also streams of 12&13 Jan 2018 where this was implemented. - dj2018-01]

	S = Solid block/floor
	The numbers are 0-based indexes into shadows.tga
	So e.g. if we just have one single solid above us,
	use the (0-based) 4th sprite (which shows slight
	tapering of the shadows on both sides).
	Basically, 32 (actually 16) 'combinations' here,
	so we use a 'table' (in the form of a map of pairs)
	to get the shadow sprite index, based on the 'pair'
	of values (bitflags above us, bitflags adjacent to us)

	00000S00000000
	00000400000000

	0000S0S0000000
	00004040000000

	000S0   0S000
	00S30   01S00

	0SS00  0SS00   0SS00
	0S300  002S0   01S00

	0000SSSSSS0000
	00001222230000

	0000SSSSSSS000
	0000122222S000

	000SSSSSSS000
	000S222223000

	0000SSSSSS000
	000S222223000
	*/

	std::map< std::pair< int,int >, unsigned char > mapShadowCases;

	mapShadowCases[ std::make_pair(0,0) ] = 0;
	mapShadowCases[ std::make_pair(0,1) ] = 0;
	mapShadowCases[ std::make_pair(0,4) ] = 0;
	mapShadowCases[ std::make_pair(0,5) ] = 0;

	//mapShadowCases[ std::make_pair(1,0) ] = 0;
	//mapShadowCases[ std::make_pair(1,1) ] = 0;
	//mapShadowCases[ std::make_pair(1,4) ] = 0;
	//mapShadowCases[ std::make_pair(1,5) ] = 0;

	mapShadowCases[ std::make_pair(2,0) ] = 4;
	mapShadowCases[ std::make_pair(2,1) ] = 1;
	mapShadowCases[ std::make_pair(2,4) ] = 3;
	mapShadowCases[ std::make_pair(2,5) ] = 2;

	mapShadowCases[ std::make_pair(3,0) ] = 1;
	mapShadowCases[ std::make_pair(3,1) ] = 1;
	mapShadowCases[ std::make_pair(3,4) ] = 2;
	mapShadowCases[ std::make_pair(3,5) ] = 2;

	//mapShadowCases[ std::make_pair(4,0) ] = 0;
	//mapShadowCases[ std::make_pair(4,1) ] = 0;
	//mapShadowCases[ std::make_pair(4,4) ] = 0;
	//mapShadowCases[ std::make_pair(4,5) ] = 0;

	//mapShadowCases[ std::make_pair(5,0) ] = 0;
	//mapShadowCases[ std::make_pair(5,1) ] = 0;
	//mapShadowCases[ std::make_pair(5,4) ] = 0;
	//mapShadowCases[ std::make_pair(5,5) ] = 0;

	mapShadowCases[ std::make_pair(6,0) ] = 3;
	mapShadowCases[ std::make_pair(6,1) ] = 2;
	mapShadowCases[ std::make_pair(6,4) ] = 3;
	mapShadowCases[ std::make_pair(6,5) ] = 2;

	mapShadowCases[ std::make_pair(7,0) ] = 2;
	mapShadowCases[ std::make_pair(7,1) ] = 2;
	mapShadowCases[ std::make_pair(7,4) ] = 2;
	mapShadowCases[ std::make_pair(7,5) ] = 2;

	g_pLevelShadowMap = new unsigned char[LEVEL_WIDTH * LEVEL_HEIGHT];
	memset(g_pLevelShadowMap,0,LEVEL_WIDTH * LEVEL_HEIGHT);
	for ( int y=0;y<LEVEL_HEIGHT;++y )
	{
		for ( int x=0;x<LEVEL_WIDTH;++x )
		{
			// If 'this' x,y block is a solid block then we don't have auto-shadows on it
			bool bThisSolid = (CHECK_SOLID( LEVCHAR_BACKA(x,y),   LEVCHAR_BACKB(x,y) ));
			if (y>0 && !bThisSolid)
			{
				int nSolidFlagsAbove=0;//bitflags [4|2|1] representing solidness of blocks above 'this' x,y block
				int nSolidFlagsAdjac=0;//bitflags [4|  1] representing solidness of blocks adjacent to 'this' x,y block
				// Check solidness of two blocks adjacent to us, fill in bitflags
				if (x<=0             || (CHECK_SOLID( LEVCHAR_BACKA(x-1,y  ), LEVCHAR_BACKB(x-1,y  ) ))) nSolidFlagsAdjac |= 4;
				if (x>=LEVEL_WIDTH-1 || (CHECK_SOLID( LEVCHAR_BACKA(x+1,y  ), LEVCHAR_BACKB(x+1,y  ) ))) nSolidFlagsAdjac |= 1;
				// Check solidness of three blocks above us, fill in bitflags
				if (x<=0             || (CHECK_SOLID( LEVCHAR_BACKA(x-1,y-1), LEVCHAR_BACKB(x-1,y-1) ))) nSolidFlagsAbove |= 4;
				if (                    (CHECK_SOLID( LEVCHAR_BACKA(x  ,y-1), LEVCHAR_BACKB(x  ,y-1) ))) nSolidFlagsAbove |= 2;
				if (x>=LEVEL_WIDTH-1 || (CHECK_SOLID( LEVCHAR_BACKA(x+1,y-1), LEVCHAR_BACKB(x+1,y-1) ))) nSolidFlagsAbove |= 1;

				// Look up the correct shadow sprite index using the table,
				// based on bitflags representing solidness above us [4|2|1], and adjacent to us [4| |1].
				*(g_pLevelShadowMap + (y*LEVEL_WIDTH) + x)
					 = mapShadowCases[
						 std::make_pair(nSolidFlagsAbove,nSolidFlagsAdjac)
					 ];
			}
		}
	}
#endif

	// loaded new back1 stuff [dj2023]
	std::string sFile = djDATAPATHs(g_pCurMission->GetLevel(g_nLevel)->m_sBack1);
	sFile = djDATAPATHs(std::string(g_pCurMission->GetLevel(g_nLevel)->GetFilename()) + "_back1.png");
	g_Level.pBack1 = nullptr;
	g_Level.pImgBack1 = nullptr;
	if (!sFile.empty() && djFileExists(sFile.c_str()))
	{
		extern djSprite* LoadSpriteHelper(const char* szPath, int nW, int nH);
		g_Level.pBack1 = LoadSpriteHelper(sFile.c_str(), 16, 16);
		if (g_Level.pBack1!=nullptr && g_Level.pBack1->IsLoaded())
			g_Level.pImgBack1 = g_Level.pBack1->GetImage();
	}

	//dj2022-11 changing this from DATA_DIR to djDataDir() .. should keep an eye here to make sure no problems introduced ..
	// Load map background image
	pBackground = new djImage;
	char *bg = (char *) malloc((strlen(djDataDir()) + strlen(g_pCurMission->GetLevel(g_nLevel)->m_szBackground) + 1) * sizeof(char));
	strcpy(bg, djDataDir());
	strcat(bg, g_pCurMission->GetLevel(g_nLevel)->m_szBackground);
	if (0!=pBackground->Load(bg))
	{
		// [dj2022-11] not quite sure myself even anymore these 'what log helpers to use when' .. should consolidate and simplify the log maybe slightly ..
		printf("Warning: Background load failed: %s\n", bg);
		djLOGSTR("Warning: Background load failed: ");djLOGSTR(bg);
		djConsoleMessage::SetConsoleMessage("Warning: Background load failed");
		djDEL(pBackground);
	}
	free(bg);
	djCreateImageHWSurface( pBackground );

	// Clear out inventory
	InvClear();//<- Note that this takes care of the fact that some inventory items are 'persistent' between levels eg powerboots
	InvDraw();

	g_ThingFactory.PerLevelInitialize();

	parse_level();
}

// Per-game cleanup
void PerGameCleanup()
{
	PerLevelCleanup();//dj2016-10 adding this, not quite sure where it 'should' be called
	
	// Empty the inventory completely
	InvEmpty();
	// Delete game background image
	djDestroyImageHWSurface(pBackground);
	djDEL(pBackground);
	// Delete all levels
	KillLevelSystem();
	djLOGSTR( "KillLevelSystem() ok\n" );
}

void PerLevelCleanup()
{
	//Not quite sure if correct place for this [dj2018-03]
	DestroyAllThings();// clear list of "things"
	DestroyAllBullets();//Make sure no bullets, for good measure [dj2018-03]

#ifndef NOSOUND
	if (g_pInGameMusic!=NULL)
	{
		Mix_FreeMusic(g_pInGameMusic);
		g_pInGameMusic = NULL;
	}
#endif

	if (g_pLevelShadowMap)
	{
		djDELV(g_pLevelShadowMap);
	}
}

// we have something like this:
// initialize
//  initialsetup
//   per-game setup
//    per-level setup
//    per-level cleanup
//   per-game cleanup
//  finalcleanup
// cleanup
/*-----------------------------------------------------------*/

int game_startup(bool bLoadGame)
{
	// FIXME: Where to determine this?
	TRACE( "game_startup()\n" );
	TRACE( "game_startup(): Playing game [%s]\n", g_pCurMission->GetName() );

	// Per game setup
	PerGameSetup();

	TRACE( "game_startup(): GameDrawSkin()\n" );
	GameDrawSkin();
	GraphFlip(!g_bBigViewportMode);

	// Per level setup (fixme, should this get called from withing per-game setup?
	PerLevelSetup();

	TRACE("game_startup(): GameDrawView()\n");
	// FIXME: Is this necessary?
	GameDrawView(0.f);

	g_bGameRunning = true;

	//dj2016-10-28 Used if doing 'Restore Game' from *main* game menu. [This is perhaps slightly spaghetti-ish, it's done this way as LoadGame() has been originally
	// written under the assumption of doing *in-game* loading of a savegame.
	if (bLoadGame)
	{
		LoadGame();
	}

	// Draw inventory
	InvDraw();
	TRACE("game_startup(): DrawScore()\n");
	DrawScore();
	TRACE("game_startup(): DrawHealth()\n");
	DrawHealth();
	GameDrawFirepower();// This must come after LoadGame() else draws 'out of date' default firepower instead of value from loadgame [dj2018-03-25]

	GraphFlip(!g_bBigViewportMode);

	// try maintain a specific frame rate
	/*const */float fTIMEFRAME = (1.0f / g_fFrameRate);

	float fTimeFirst = djTimeGetTime();

	// Start out by being at next time
	float fTimeNext = djTimeGetTime();
	float fTimeNow = fTimeNext;
	int iFrameCount=0;
	int anKeyState[KEY_NUM_MAIN_REDEFINABLE_KEYS] = { 0 };
	int i;
	while (g_bGameRunning)
	{
		static std::string g_sAutoScreenshotFolder;//<- If !empty it's busy recording into this folder [dj2018-04]
		static int g_nScreenshotNumber=0;

		//debug//printf("{");
		fTimeNow = djTimeGetTime();
		bool bForceUpdate = false;
		// If we're already behind, just force us to get there
		if (fTimeNext < fTimeNow)
		{
			//printf( "slow frame\n" );
			fTimeNext = fTimeNow;
			bForceUpdate = true;
		}

		int iEscape=0;
		while (fTimeNow<fTimeNext || bForceUpdate) // until we reach next frames time
		{
			// Try to prevent this from hogging the CPU, which can adversely affect other processes
			//dj this #ifdef etc. a bit gross/clunky should we wrap 'SDL_delay' in a simple wrapper functino to deal with emscripted in there in one place application-wide, or what better way? Hm
			#ifdef __EMSCRIPTEN__
			/*SDL_Delay(1);*/
			#else
			SDL_Delay(1);
			#endif

			// poll keys
			djiPollBegin();
			//djiPoll();
			// only register key presses here, so that key presses aren't missed.
			SDL_Event Event;
			//debug//printf("P");
			// dj2017-06-18:
			// Detect the 'edge' i.e. the moment keystate changes from 0 to 1
			// We must do this so that if we get a down/up within the same poll *before*
			// the heartbeat, we still process it (once) to e.g. move one block in the
			// immediate subsequent 'heartbeat'.
			// (This fixes the loooooong-outstanding annoying issue "Key polling behavior is
			// subtly incorrect")
			int key_down_edge[KEY_NUM_MAIN_REDEFINABLE_KEYS] = {0};
			while (djiPollEvents(Event))
			{
				switch (Event.type)
				{
				case SDL_KEYDOWN:
					for ( i=0; i<KEY_NUM_MAIN_REDEFINABLE_KEYS; i++ )
					{
						if (Event.key.keysym.sym==g_anKeys[i])
						{
							//debug//printf("KEY_DOWN[%d]",i);
							anKeyState[i] = 1;
							
							if (i==KEY_LEFT  && key_left==0)	key_down_edge[KEY_LEFT] = 1;
							if (i==KEY_RIGHT && key_right==0)	key_down_edge[KEY_RIGHT] = 1;
							if (i==KEY_ACTION&& key_action==0)	key_down_edge[KEY_ACTION] = 1;
							if (i==KEY_JUMP  && key_jump==0)	key_down_edge[KEY_JUMP] = 1;
							if (i==KEY_SHOOT && key_shoot==0)	key_down_edge[KEY_SHOOT] = 1;
						}
					}

					// Shift + F6/F7: Dec/Inc speed (framerate).
					// Not sure if this really makes sense in 'production' game,
					// but note the original DN1 had equivalent speec dec/inc ('<' and
					// '>' keys) [dj2017-06-24]
					if ( (Event.key.keysym.mod & KMOD_LSHIFT)!=0 ||
					     (Event.key.keysym.mod & KMOD_RSHIFT)!=0)
					{
						// Ctrl+Shift?
						if ( (Event.key.keysym.mod & KMOD_LCTRL)!=0 ||
							 (Event.key.keysym.mod & KMOD_RCTRL)!=0)
						{
							// Ctrl+Shift+G: Enable debug commands (e.g. H for health self-damage etc.)
							if (Event.key.keysym.sym==SDLK_g)
							{
								g_bEnableDebugStuff = true;
								ShowGameMessage("DebugStuff Enabled", 64);
							}
#ifdef djINGAME_FULLSCREEN_TOGGLE//dj2022-11
//dj2022 do we really need to waste yet another key shortcut etc. just for live fullscreen toggle we can do from the ingame menu? what if porters using 'f' and its causing conflict?
/*							else if (Event.key.keysym.sym == SDLK_f)
							{
								//dj2022-11 experimental toggle fullscreen probably going to crash a lot
								djGraphicsSystem::ToggleFullscreen();
								RedrawEverythingHelper();
								bForceUpdate = true;
								//return;
							}*/
#endif//djINGAME_FULLSCREEN_TOGGLE
#ifdef DAVEGNUKEM_CHEATS_ENABLED
							else if (Event.key.keysym.sym==SDLK_w)
							{
								// For now regard large-viewport-mode as a cheat but think about it [dj2017-08]
								if (g_bEnableDebugStuff)
								{
									g_bLargeViewport = !g_bLargeViewport;
									g_bBigViewportMode = false;//Can't have both bigviewport and largeviewport
									ReInitGameViewport();
									RedrawEverythingHelper();
								}
							}
#endif
						}
						else
						{
						/*{
						char buf[1024]={0};
						snprintf(buf,sizeof(buf),"%08x,%08x,%08x",(int)Event.key.keysym.sym, (int)Event.key.keysym.mod, (int)Event.key.keysym.scancode);
						ShowGameMessage(buf, 32);
						}*/
						if (Event.key.keysym.sym==SDLK_F6)
						{
							//ShowEndGameSequence();
							//RedrawEverythingHelper();

							g_fFrameRate -= 1.0f;
							if (g_fFrameRate<1.f)
								g_fFrameRate = 1.f;
							fTIMEFRAME = (1.0f / g_fFrameRate);
							char buf[1024]={0};
							snprintf(buf,sizeof(buf),"Dec framerate %.2f",g_fFrameRate);
							ShowGameMessage(buf, 32);
						}
						else if (Event.key.keysym.sym==SDLK_F7)
						{
							g_fFrameRate += 1.0f;
							fTIMEFRAME = (1.0f / g_fFrameRate);
							char buf[1024]={0};
							snprintf(buf,sizeof(buf),"Inc framerate %.2f",g_fFrameRate);
							ShowGameMessage(buf, 32);
						}
						else if (Event.key.keysym.sym==SDLK_F8)
						{
							g_bAutoShadows = !g_bAutoShadows;
						}
						else if (Event.key.keysym.sym==SDLK_F9)
						{
							g_bSpriteDropShadows = !g_bSpriteDropShadows;
						}

						}
					}//shift


					// F10? Screenshot and auto-screenshot stuff
					if (Event.key.keysym.sym==SDLK_F10)
					{
					// Ctrl+Shift+F10? Start/stop screenshot recording [dj2018-04-01]
					if (((Event.key.keysym.mod & KMOD_LSHIFT)!=0 ||
						(Event.key.keysym.mod & KMOD_RSHIFT)!=0)/*
						&&
						((Event.key.keysym.mod & KMOD_LCTRL)!=0 ||
						(Event.key.keysym.mod & KMOD_RCTRL)!=0)*/)
					{
						if (!g_sAutoScreenshotFolder.empty())
						{
							g_sAutoScreenshotFolder.clear();//Stop recording
							g_nScreenshotNumber=0;
						}
						else
						{
							// Start recording
							std::string sBasePath = djGetFolderUserSettings();//<- for now use this, not really the right place, should be under user Documents or something [todo-future-lowprio - dj2018-03]
							djAppendPathS(sBasePath, "screenshots");
							djEnsureFolderTreeExists(sBasePath.c_str());
							std::string sFilenameWithPath;
							int n = 0;
							do
							{
								++n;
								char szBuf[8192]={0};//fixLOW MAX_PATH? Some issue with MAX_PATH I can't remember what right now [dj2017-08]
								snprintf(szBuf, sizeof(szBuf), "gnukem_recording_%03d", n);
								sFilenameWithPath = djAppendPathStr(sBasePath.c_str(),szBuf);
							} while (djFolderExists(sFilenameWithPath.c_str()));
							djEnsureFolderTreeExists(sFilenameWithPath.c_str());
							if (djFolderExists(sFilenameWithPath.c_str()))
							{
								// Start recording
								g_sAutoScreenshotFolder = sFilenameWithPath;
								g_nScreenshotNumber=1;
							}
						}
					}
					else//dj2018-03-30 F10 saves a screenshot
					{
						//Generate a unique filename (using counter - if file exists, increment counter and try again)
						//fixLOW handle unicode in paths? Future? [dj2017-08]
						std::string sPath = djGetFolderUserSettings();//<- for now use this, not really the right place, should be under user Documents or something [todo-future-lowprio - dj2018-03]
						djAppendPathS(sPath, "screenshots");
						djEnsureFolderTreeExists(sPath.c_str());
						std::string sFilenameWithPath;
						int n = 0;
						do
						{
							++n;
							const std::string sBuf = std::string("gnukem_screenshot_") + std::to_string(n) + ".bmp";

							sFilenameWithPath = djAppendPathStr(sPath.c_str(),sBuf.c_str());
						} while (djFileExists(sFilenameWithPath.c_str()));
						SDL_SaveBMP(pVisMain->pSurface, sFilenameWithPath.c_str());
						ShowGameMessage("Screenshot saved", 7);
					}
					}//if(F10)

					
					// [dj2017-06] DEBUG/CHEAT/DEV KEYS
					/*
					if (Event.key.keysym.sym==SDLK_F8)
					{
						g_bSmoothVerticalMovementTest = !g_bSmoothVerticalMovementTest;
						y_offset = 0;
						if (g_bSmoothVerticalMovementTest)
							ShowGameMessage("Smooth vertical movement ON", 32);
						else
							ShowGameMessage("Smooth vertical movement OFF", 32);
					}
					else
					*/
					// 'Global' shortcut keys for adjusting volume [dj2016-10]
					// [dj2018-03-30] Changing from PgUp/PgDn for two reasons:
					// 1. Some keyboards - like my Asus laptop - don't seem to have regular PgUp/PgDn
					// 2. It conflicts with keyboards like OpenPandora where PgUp/PgDn are mapped to important game keys
					// (Not mad about the 6/7 choice might change that in future, or make it configurable or something.)
					{
					// Get localized string for "Volume"
					const std::string sVolume = pgettext("sound", "Volume");
					if (Event.key.keysym.sym==SDLK_7)//SDLK_PAGEUP)
					{
						djSoundAdjustVolume(4);
						djConsoleMessage::SetConsoleMessage(sVolume + ": " + std::to_string(static_cast<int>((100.f * (static_cast<float>(djSoundGetVolume()) / 128.f)))));
					}
					else if (Event.key.keysym.sym==SDLK_6)//SDLK_PAGEDOWN)
					{
						djSoundAdjustVolume(-4);
						djConsoleMessage::SetConsoleMessage(sVolume + ": " + std::to_string(static_cast<int>((100.f * (static_cast<float>(djSoundGetVolume()) / 128.f)))));
					}
					else if (Event.key.keysym.sym==SDLK_INSERT)
					{
						if (djSoundEnabled())
							djSoundDisable();
						else
							djSoundEnable();
						djConsoleMessage::SetConsoleMessage( djSoundEnabled() ? "Sounds ON (Ins)" : "Sounds OFF (Ins)" );
					}
					}
					break;
				case SDL_KEYUP:
					for ( i=0; i<KEY_NUM_MAIN_REDEFINABLE_KEYS; i++ )
					{
						if (Event.key.keysym.sym==g_anKeys[i])
						{
							//debug//printf("KEY_UP[%d]",i);
							anKeyState[i] = 0;
							
							// If e.g. key_left is '1' it means we've already 'processed/handled' the 'down' edge event in the 'heartbeat' - so squash the value to 0 now to ensure we don't e.g. do the double-move
							if (i==KEY_LEFT && key_left==1)		key_left = 0;
							if (i==KEY_RIGHT && key_right==1)	key_right = 0;
							if (i==KEY_ACTION && key_action==1)	key_action = 0;
							if (i==KEY_SHOOT && key_shoot==1)	key_shoot = 0;
							if (i==KEY_JUMP && key_jump==1)		key_jump = 0;
								
						}
					}
					break;
				case SDL_QUIT:
					//If user clicks Windows 'X' with mouse in-game, not quite sure
					// what behavior makes the most sense, but for now just pop up
					// in-game menu. I don't think it should immediately exit, in
					// case you're in the middle of a game and press by mistake.[dj2017-08]
					iEscape = 1;
					break;
				}
			}// while (pollevents)

			if (key_down_edge[KEY_ACTION]) key_action = 1;
			if (key_down_edge[KEY_LEFT])   key_left = 1;
			if (key_down_edge[KEY_RIGHT])  key_right = 1;
			if (key_down_edge[KEY_JUMP])   key_jump = 1;
			if (key_down_edge[KEY_SHOOT])  key_shoot = 1;
			//if (g_iKeys[DJKEY_UP])		key_action = 1;
			//if (g_iKeys[DJKEY_LEFT])	key_left = 1;
			//if (g_iKeys[DJKEY_RIGHT])	key_right = 1;

			//dj2022-11 it's debatable whether to do below as it 'interferes' with e.g. some shortcut keys			
			// We allow ctrl as a sort of 'default' fallback jump if (and only if) it isn't assigned/redefined to anything
			if (!IsGameKeyAssigned(SDLK_RCTRL))
			{
				if (g_iKeys[DJKEY_CTRL])	key_jump = 1;
			}
			//[dj2016-10 don't think it really makes sense to have P as jump - if anything, pause??[LOW]](g_iKeys[DJKEY_P])		key_jump = 1;
			if (g_iKeys[DJKEY_ESC])		iEscape = 1;
			// oh dear now can't get out of menu if do this ?? if (g_iKeys[DJKEY_F1])		iEscape = 1;//dj2018-03

			if (g_bEnableDebugStuff)
			{

#ifdef DAVEGNUKEM_CHEATS_ENABLED
				// CHEAT KEYS
				if (g_iKeys[DJKEY_BACKSPACE])
				{
					// BACKSPACE + G: God mode
					if (g_iKeys[DJKEY_G])
					{
						ShowGameMessage("CHEAT: GOD MODE", 96);
						g_bGodMode = true;
					}
					// BACKSPACE + B: Toggle 'big viewport mode' [dj2016-10-10]
					// It seems to be difficult to toggle just once ... so we detect key up/down 'edge' and only toggle on that
					static bool g_bBKeyLast=false;
					bool bBKey = (g_iKeys[DJKEY_B]!=0);
					if (bBKey && !g_bBKeyLast)// Detect keydown 'edge'
					{
						g_bBigViewportMode = !g_bBigViewportMode;
						g_bLargeViewport = false;//Can't have both bigviewport and largeviewport
						ReInitGameViewport();
						// Redraw everything that needs to be redrawn, as larger viewport will have obliterated right side with score etc.
						RedrawEverythingHelper();
					}
					g_bBKeyLast = bBKey;

					// BACKSPACE + P: Powerboots
					if (g_iKeys[DJKEY_P])
					{
						HeroSetJumpMode(JUMP_POWERBOOTS);
						ShowGameMessage("CHEAT: POWERBOOTS", 96);
						//dj2017-08 Adding boots to inventory- this is slightly gross
						bool bHave = false;
						for ( i=0; i<InvGetSize(); i++ )
						{
							if (InvGetItem(i)->GetTypeID()==TYPE_POWERBOOTS)
							{
								bHave = true;
								break;
							}
						}
						if (!bHave)
						{
							CBoots* pThing = new CBoots;
							pThing->SetType(TYPE_POWERBOOTS);
							pThing->SetSprite(1, 128-32);//GROSS HARDCODED - if we move to different sprite in metadata, this will break
							InvAdd(pThing);
						}

					}
					// BACKSPACE + PGDN: All power-ups
					if (g_iKeys[DJKEY_PGDN])
					{
						ShowGameMessage("CHEAT: HealthKeysFirepower", 96);
#ifdef __EMSCRIPTEN__
						/*SDL_Delay(100);*///<-'wrong' workaround for, it adds 6 access cards [dj2017-06]
#else
						SDL_Delay(100);//<-'wrong' workaround for, it adds 6 access cards [dj2017-06]
#endif
						// Full health
						SetHealth(MAX_HEALTH);

						// All keys
						std::vector<int> anKeysHave;
						for ( i=0; i<InvGetSize(); i++ )
						{
							if (InvGetItem(i)->GetTypeID()==TYPE_KEY
								||InvGetItem(i)->GetTypeID()==TYPE_ACCESSCARD)//dj2017-06 adding access card
							{
								CKey *pKey = (CKey*)InvGetItem(i);
								anKeysHave.push_back(pKey->GetID());
							}
						}
						for ( i=1; i<=4; i++)
						{
							unsigned int j;
							bool bHave = false;
							for ( j=0; j<anKeysHave.size(); j++ )
							{
								if (i==anKeysHave[j])
									bHave = true;
							}
							if (!bHave)
							{
								CKey *pKey = new CKey;
								pKey->SetType(TYPE_KEY);
								pKey->SetSprite(0, 116+i);
								pKey->Initialize(0, 116+i);
								InvAdd(pKey);
							}
						}
						{
							//dj2017-06 Adding access card - this is gross, it's hardcoded here that '5' is its key/door number. Oh well, not going to lose sleep over it.
							bool bHave = false;
							for ( unsigned int j=0; j<anKeysHave.size(); j++ )
							{
								if (5==anKeysHave[j])//<- [LOW PRIO] this detectioh isn't working correctly [see workaround note above 2017-06]
									bHave = true;
							}
							if (!bHave)
							{
								CKey *pKey = new CAccessCard;
								pKey->SetType(TYPE_ACCESSCARD);
								pKey->SetID(5);
								pKey->SetSprite(1, 97);
								pKey->Initialize(1, 97);
								InvAdd(pKey);
							}
						}
						{
							//dj2017-08 Adding antivirus - this is gross, it's hardcoded here that '6' is its key/door number. Oh well, not going to lose sleep over it.
							bool bHave = false;
							for ( unsigned int j=0; j<anKeysHave.size(); j++ )
							{
								if (5==anKeysHave[j])//<- [LOW PRIO] this detectioh isn't working correctly [see workaround note above 2017-06]
									bHave = true;
							}
							if (!bHave)
							{
								CAntivirus *pKey = new CAntivirus;
								pKey->SetType(TYPE_ANTIVIRUS);
								pKey->SetID(6);
								pKey->SetSprite(1, 98);
								pKey->Initialize(1, 98);
								InvAdd(pKey);
							}
						}

						//dj2022-11
						InvMakeAllPersistent();

						// Full firepower
						HeroSetFirepower(MAX_FIREPOWER);
					}
					
					// BACKSPACE + PGUP: Increase firepower by one [dj2017-12]
					if (g_iKeys[DJKEY_PGUP])
					{
						// Note this function does clamping to MAX_FIREPOWER so we don't need to check here
						HeroSetFirepower(g_nFirepower+1);
#ifdef __EMSCRIPTEN__
						/*SDL_Delay(200);*///<-'wrong' workaround for, it immediately adds a lot
#else
						SDL_Delay(200);//<-'wrong' workaround for, it immediately adds a lot
#endif
					}
				}
#endif//#ifdef DAVEGNUKEM_CHEATS_ENABLED

				// This is for debugging/testing: hurt self. This definitely
				// shouldn't be enabled by default in the real game, so we
				// put it behind the g_bEnableDebugStuff setting. [dj2017-08]
				// This is of course not a 'cheat', lol .. literally just useful for gamedev/testing.
				static bool b = false;
				bool bOld = b;
				if (g_iKeys[DJKEY_H]) b = true; else b = false;
				if (b && !bOld)
					update_health(-1);

				// 'D' for Debug: toggle display of debug info
				if (g_iKeys[DJKEY_D] && !g_iKeysLast[DJKEY_D]) bShowDebugInfo = !bShowDebugInfo;
			
			}//if (g_bEnableDebugStuff)


			fTimeNow = djTimeGetTime();
			bForceUpdate = false;
		}
		// FIXME: time next should be calculated more absolutely, not relatively.
//		fTimeNext = fTimeNow + fTIMEFRAME;
		fTimeNext = fTimeNext + fTIMEFRAME;

		//-- ESC - Pop up the in-game menu
		if (iEscape==1)
		{
			IngameMenu();

			// WORKAROUND: If we are holding eg left when entering ingamemenu, we 'miss' the gameloop's KeyUp event as it takes over input polling; this leads to potentially "stuck" keystates in anKeyState when exiting TL;DR CLEAR THE KEYSTATES HERE (even if the keys really are down on exit). I am not mad about this solution, all feels a little wobbly/workaround-y, but should do for now. [dj2017-08-13]
			// Workaround for: "If press direction key then immediately Escape in-game, then Esc again, doesn't "detect" direction key was released while in game menu again (hero keeps moving)"
			memset( anKeyState, 0, sizeof(anKeyState) );

			// Redraw everything that needs to be redrawn
			RedrawEverythingHelper();
		}


		// Make a simple FPS (Frames per Second) display for debug purposes
		//fixme_notworking://in largeviewportmode
		const float fTimeRun = fTimeNow - fTimeFirst;
		iFrameCount++;
		static char szBufFPS[1024]={0};
		snprintf( szBufFPS, sizeof(szBufFPS), "%.2f", (float)iFrameCount / fTimeRun );
		//snprintf( sbuf, sizeof(sbuf), "%.2f %d %d", (float)iFrameCount / fTimeRun ,HERO_PIXELX,HERO_PIXELY);
		if (iFrameCount==60)
		{
			iFrameCount /= 2;
			fTimeFirst += (fTimeRun/2);
		}
		if (!g_bLargeViewport)
			djgDrawImage( pVisBack, pSkinGame, 12, 4, 12, 4, 196, 8 );
		extern djSprite* g_pFont2;
		if (g_pFont2 && g_pFont2->IsLoaded())
			GraphDrawString( pVisBack, g_pFont2->GetImage(), 12, 4, (unsigned char*)szBufFPS );
		else
		GraphDrawString( pVisBack, g_pFont8x8, 12, 4, (unsigned char*)szBufFPS );

		/////////////////////////////
		// UPDATE

		// [dj2022-11] Simple calculate of delta-time since last frame at this point (not currently used for much but in theory a lot of game update stuff 'should' be driven more by something like this .. should perhaps in future be passed to CThing::Tick or something like that)
		static uint64_t uTicksLast = 0;
		uint64_t uTicksNow = djTimeGetTicks64();
		if (uTicksLast == 0) uTicksLast = uTicksNow;//<- hmm .. better idea? first tick will be a bit odd [dj2022-11 low prio]
		const float fDT = (float)(uTicksNow - uTicksLast);//<- delatime (milliseconds) since last frame ... to think about - double?
		uTicksLast = uTicksNow;//<- save for next frame to calculate delmtatime

		//dj2022-11 add quick n dirty stress-tester to help test in-game fullscreen toggle robustness
#if defined(djINGAME_FULLSCREEN_TOGGLE) && defined(djDEV_STRESSTESTS)
		if (djStressTestInGameFullscreenToggle(fDT))
		{
			// If toggled
			RedrawEverythingHelper();
			bForceUpdate = true;
		}
#endif
		djConsoleMessage::Update(fDT);//dj2022-11 refactoring message stuff

		GameHeartBeat(fDT);
		
		if (!g_sAutoScreenshotFolder.empty())
		{
			const std::string sAppName="gnukem";
			std::string sFilename = sAppName + "_" + std::to_string(g_nScreenshotNumber) + ".bmp";
			std::string sPath = djAppendPathStr(g_sAutoScreenshotFolder.c_str(), sFilename.c_str());
			SDL_SaveBMP(pVisMain->pSurface, sPath.c_str());//"c:\\dj\\DelmeTestMain.bmp");
			++g_nScreenshotNumber;
			//SDL_SaveBMP(pVisMain->pSurface, szFilename);//"c:\\dj\\DelmeTestMain.bmp");

			//static int nFrameCounter=0;
			//char szFilename[4096]={0};
			//snprintf(szFilename,sizeof(szFilename),"c:\\dj\\rectest\\dave_gnukem_%08d.bmp",nFrameCounter);
			//SDL_SaveBMP(pVisMain->pSurface, szFilename);//"c:\\dj\\DelmeTestMain.bmp");
			//nFrameCounter++;
		}

		// to see this frame timing graph currently do firstly Ctrl+Shift+G, then press D to enable onscreen debug info
		uint64_t uTicksAfterHeartbeat = djTimeGetTicks64();
		afTimeTaken.push_back(float(uTicksAfterHeartbeat - uTicksNow));
		if (afTimeTaken.size()>MAX_DEBUGGRAPH)
			afTimeTaken.erase(afTimeTaken.begin());

		key_action = anKeyState[KEY_ACTION];
		key_left   = anKeyState[KEY_LEFT];
		key_right  = anKeyState[KEY_RIGHT];
		key_jump   = anKeyState[KEY_JUMP];
		key_shoot  = anKeyState[KEY_SHOOT];

		// We allow ctrl as a sort of 'default' fallback jump if (and only if) it isn't assigned/redefined to anything
		//fixmeLOW/MED - This functionality seems to appear twice - not sure what that's about but will probably have
		// to come back to that [perhaps as part of looking into the story of keypolling behavior being 'subtly incorrect'] [dj2016-10-16]
		if (!IsGameKeyAssigned(SDLK_RCTRL))
		{
			key_jump   = g_iKeys[DJKEY_CTRL];
		}
		//key_jump  |= g_iKeys[DJKEY_P];

		// ensure we don't leave the borders of the level
		// fixme; is this still necessary what with the (other functions)
		g_Player.x = djMAX( djMIN(g_Player.x,126), 1 );
		g_Player.y = djMAX( djMIN(g_Player.y, 99), 2 );
		//debug//printf("}");

#ifdef DAVEGNUKEM_CHEATS_ENABLED
		if (g_bEnableDebugStuff && key_action && g_iKeys[DJKEY_L])
		{
			// NB, if we are holding a key down when entering sprite/level editor, we 'miss' the gameloop's KeyUp event as the integrated editor takes over input polling; this leads to potentially "stuck" keystates in anKeyState when exiting, which in turn causes problems like e.g. if you press 'up' before entering the level editor then drop yourself out of the level editor over a teleporter, the game freezes as it keeps re-activating the teleporter since the action key is effectively behaving as if stuck down. TL;DR CLEAR THE KEYSTATES HERE (even if the keys really are down on exit editor). I am not mad about this solution, all feels a little wobbly/workaround-y, but should do for now. [dj2017-06-22]
			memset( anKeyState, 0, sizeof(anKeyState) );
			NextLevel();
			//return;
		}
		else
#endif
		// "integrated" sprite / level editors [dj2017-06-20 moving these to bottom of this loop, just in case we have any issues comparable to the pungee sticks crash bug, e.g interacting with dangling objects or something in the one single heartbeat update that occurs after exiting level editor]
		if (g_bEnableDebugStuff && g_iKeys[DJKEY_F4])
		{
			SwitchMode ( SWITCH_SPRED );
			ED_Main ();

			// NB, if we are holding a key down when entering sprite/level editor, we 'miss' the gameloop's KeyUp event as the integrated editor takes over input polling; this leads to potentially "stuck" keystates in anKeyState when exiting, which in turn causes problems like e.g. if you press 'up' before entering the level editor then drop yourself out of the level editor over a teleporter, the game freezes as it keeps re-activating the teleporter since the action key is effectively behaving as if stuck down. TL;DR CLEAR THE KEYSTATES HERE (even if the keys really are down on exit editor). I am not mad about this solution, all feels a little wobbly/workaround-y, but should do for now. [dj2017-06-22]
			memset( anKeyState, 0, sizeof(anKeyState) );
			
			RestartLevel();
		}
		else if (g_bEnableDebugStuff && g_iKeys[DJKEY_F5])
		{
			SwitchMode ( SWITCH_LVLED );
			ED_Main ();

			// NB, if we are holding a key down when entering sprite/level editor, we 'miss' the gameloop's KeyUp event as the integrated editor takes over input polling; this leads to potentially "stuck" keystates in anKeyState when exiting, which in turn causes problems like e.g. if you press 'up' before entering the level editor then drop yourself out of the level editor over a teleporter, the game freezes as it keeps re-activating the teleporter since the action key is effectively behaving as if stuck down. TL;DR CLEAR THE KEYSTATES HERE (even if the keys really are down on exit editor). I am not mad about this solution, all feels a little wobbly/workaround-y, but should do for now. [dj2017-06-22]
			memset( anKeyState, 0, sizeof(anKeyState) );

			RestartLevel(); // [dj2017-06-20] This replaces PerLevelSetup() call that was after LVLED_Kill(), not 100% sure but suspect this slightly more 'correct'
		}
		//fixme maybe add F1 check to show menu like in DN1? so ppl who play original
		// can have that behaviour, even tho still don't bake 'F1' into gameskin. [dj2018-03]
		// NB, not all platforms will have an F1 key. The original DN1 said on the screen
		// 'press F1 for help' but we can't 'bake that in' to the images etc.
		/*else if (g_iKeys[DJKEY_F1])
		{
			ShowInstructions();
			g_iKeys[DJKEY_F1]=0;//????
			// NB, if we are holding a key down when entering sprite/level editor, we 'miss' the gameloop's KeyUp event as the integrated editor takes over input polling; this leads to potentially "stuck" keystates in anKeyState when exiting, which in turn causes problems like e.g. if you press 'up' before entering the level editor then drop yourself out of the level editor over a teleporter, the game freezes as it keeps re-activating the teleporter since the action key is effectively behaving as if stuck down. TL;DR CLEAR THE KEYSTATES HERE (even if the keys really are down on exit editor). I am not mad about this solution, all feels a little wobbly/workaround-y, but should do for now. [dj2017-06-22]
			//memset( anKeyState, 0, sizeof(anKeyState) );
		}*/

		// If game is ending, and Dr Proton's escaping off the top,
		// activate the end-game sequence and then move on to next
		// level (which should end the game as Dr Proton should be
		// placed on last level only).
		if (CDrProton::GameEnding())
		{
			if (CDrProton::GetDrProton()->m_y<=5)
			{
				//ShowInstructions();
				ShowEndGameSequence();

				// Redraw everything that needs to be redrawn, otherwise still see parts of endgame window over right side etc. [dj2017-08-13]
				RedrawEverythingHelper();

				// ['In theory' the following workaround shouldn't be necessary as boss is supposed to be last level but sometimes during testing isn't, so handle this anyway .. otherwise eg. keys 'stuck down' on start next level]
				// If we are holding a key down when entering endgamesequence, we 'miss' the gameloop's KeyUp event as it takes over input polling; this leads to potentially "stuck" keystates in anKeyState when exiting, which in turn causes problems like e.g. if you press 'up' before entering the level editor then drop yourself out of the level editor over a teleporter, the game freezes as it keeps re-activating the teleporter since the action key is effectively behaving as if stuck down. TL;DR CLEAR THE KEYSTATES HERE (even if the keys really are down on exit editor). I am not mad about this solution, all feels a little wobbly/workaround-y, but should do for now. [dj2017-08-13]
				memset( anKeyState, 0, sizeof(anKeyState) );
				
				// Next-level is maybe slightly weird, but since this 'should be'
				// last level, it should at this point just pop out to the main
				// game menu.
				// (fixmeLOW I think DN1 had a 'room' between each level showing
				// the bonuses you got etc. Not sure, don't think we really need to have that.) [dj2017-08]
				NextLevel();
			}
		}

	} // while (game running)

	TRACE("game_startup(): main game loop exited.\n");

	PerGameCleanup();
	return g_nScore;
}

/*-----------------------------------------------------------*/
void GameHeartBeat(float fDeltaTime_ms)
{
	// Update hero basic stuff
	HeroUpdate();

	// Update global animation counter
	++anim4_count;
	if (anim4_count>3)
		anim4_count = 0;

	//nSlowDownHeroWalkAnimationCounter ^= 1;
	// Above line should have same effect and be faster, but is less understandable
	nSlowDownHeroWalkAnimationCounter++;
	if (nSlowDownHeroWalkAnimationCounter>1)
		nSlowDownHeroWalkAnimationCounter = 0;

	//not jumping but about to be, then dont left/right move
	if (!((key_jump) && (g_Player.hero_mode != MODE_JUMPING))) {
		if (key_left)
		{
			//debug//printf("L");
			key_left = 0;
			move_hero(-1,0);
		}
		if (key_right)
		{
			//debug//printf("R");
			key_right = 0;
			move_hero(1,0);
		}
	}


	static bool bFallingPrev = false;
	bool bFalling = false;

	//mode-specific handling
	int n=0;
	switch (g_Player.hero_mode)
	{
	case MODE_NORMAL:
		//fall:

		n = move_hero(0,1);
		{
			bFalling = (n==0);
			if (bFalling)
			{
				if (!bFallingPrev) // <- just started falling?
				{
					g_Player.m_nFalltime = 0;
				}
				++g_Player.m_nFalltime;
			}
			else
				g_Player.m_nFalltime = 0;
			if (bFallingPrev && !bFalling) // <- just stopped falling
			{
				// Kick up some dust ..
				AddThing(CreateDust(g_Player.x, g_Player.y, g_Player.x_small*HALFBLOCKW,g_Player.y_offset));
				djSoundPlay( g_iSounds[SOUND_JUMP_LANDING] );
			}
			bFallingPrev = bFalling;
		}

		// standing still and pressing 'up': (just pressing up?)
		if (key_action)
		{
			key_action = 0; // huh?
			// dj2017-06-22 I think that "huh?" might have something to do with issue encountered of 'freezing on entering teleporter' issue, or else it's just redundant/old code, not sure, as it re-sets key_action anyway from anKeyStates[KEY_ACTION] each heartbeat

#ifdef DAVEGNUKEM_CHEATS_ENABLED
			// Go-to-exit cheat key
			if (g_bEnableDebugStuff && g_iKeys[DJKEY_I])
			{
				for ( int i=0; i<(int)g_apThings.size(); ++i )
				{
					if (g_apThings[i]->GetTypeID()==TYPE_EXIT)
					{
						relocate_hero(g_apThings[i]->m_x, g_apThings[i]->m_y);
						HeroFreeze(5);
						return;
					}
				}
			}
#endif

			// Check if you're on anything funny, like an exit
			CThing* pThing=NULL;
			for ( int i=0; i<(int)g_apThings.size(); ++i )
			{
				pThing = g_apThings[i];
				if (!HeroIsFrozen() && pThing->HeroInsideActionBounds(HERO_PIXELX, HERO_PIXELY))
				{
//					int iRet = pThing->Action();
					pThing->Action();
					// FIXME: Handle return codes
				}
				// If hero has been "frozen" by some action (e.g. teleport, exit), return
				if (HeroIsFrozen())
					return;
			} // i

		}

		if (key_jump)
		{
			key_jump = 0;
			if (n) { // if hero wasnt busy falling
				HeroStartJump();
				hero_picoffs=1;
			}
		}
		break;

	case MODE_JUMPING:
		HeroUpdateJump();

		key_jump = 0;
		break;
	}//switch (hero_mode)


	// Game viewport auto-scrolling
	GameViewportAutoscroll(bFalling, bFallingPrev);


	// Update bullets
	UpdateBullets(fDeltaTime_ms);
	CheckForBulletsOutOfView();

	// Check if hero is shooting and if must create new hero bullets
	CheckIfHeroShooting();

	// Interact with "things", e.g. check if you're on anything funny, like an exit
	InteractWithThings();

	// Drop all objects that can fall
	DropFallableThings();

	// "Tick" (update) all objects
	TickAllThings(fDeltaTime_ms);

	if (g_bDied)
	{
		// Reset to beginning of current level
		RestartLevel();
		g_bDied = false;
	}

	// Redraw the screen according to the current game state
	GameDrawView(fDeltaTime_ms);

	// Clear the to-be-deleted bullets that have just hit something (after
	// drawing them one last time) [dj2018-01-13] [This is a 'kludge' for effective visual effect of drawing these one last frame after they've hit something]
	DestroyBullets(g_apBulletsDeleted);

	if ( g_Player.nHurtCounter > 0 )
		g_Player.nHurtCounter--;

	// Show on-screen message
	if (g_nGameMessageCount>=0)
	{
		g_nGameMessageCount--;
		GraphDrawString( pVisBack, g_pFont8x8, 16, 160, (unsigned char*)g_sGameMessage.c_str() );
	}

	// Flip the back buffer onto the front
	GraphFlip(!g_bBigViewportMode);

	//debug//printf("]");
}

void Die()
{
	// [dj2016-10] Just set a flag here that we've died, which we process 'asynchronously' (to fix the 'dying in the pungee sticks will often cause a crash' issue)
	g_bDied = true;
}

void HeroShoot(int nX, int nY, int nXDiff, int nYDiff)
{
	CBullet *pBullet = new CBullet;
	pBullet->x = nX;
	pBullet->y = nY;
	pBullet->dx = nXDiff;
	pBullet->dy = nYDiff;
	pBullet->eType = CBullet::BULLET_HERO;
	g_apBullets.push_back(pBullet);
	djSoundPlay( g_iSounds[SOUND_SHOOT] );

	// [DN1 style] Change hero draw slightly right after shooting to show sort of 'kickback' effect
	g_nHeroJustFiredWeaponCounter = 2;
}

void MonsterShoot(int nX, int nY, int nXDiff, int nYDiff)
{
	CBullet *pBullet = new CBullet;
	pBullet->x = nX;
	pBullet->y = nY;
	pBullet->dx = nXDiff;
	pBullet->dy = nYDiff;
	pBullet->eType = CBullet::BULLET_MONSTER;
	g_apBullets.push_back(pBullet);
	djSoundPlay( g_iSounds[SOUND_SHOOT2] );
}

void HeroSetHurting(bool bReset)
{
	// [dj2020-06] This 16 is NOT the 'block width or height' 16, it's not in pixels, it's in num-frames ... so when we genericize away the BLOCKW/BLOCKH stuff, leave this one at 16
	if (bReset || g_Player.nHurtCounter==0)
		g_Player.nHurtCounter = 16;
}

bool HeroIsHurting()
{
	return (g_Player.nHurtCounter!=0);
}

void DrawHealth()
{
	// Build a string representing health bars (which are in the 8x8 font)
	/*
	unsigned char szHealth[MAX_HEALTH+1]={0};
	for ( unsigned int i=0; i<MAX_HEALTH; ++i )
	{
		// [dj2023-11] UNHARDCODING and doing more nicely generically. The below are ugly old hardcoded offsets into main old font.tga, we want to be able to not even load that and everythign else must work, so, separating small things like this into separate new png's [dj2023-11] so we can do French etc.
		// 170 = health; 169 = not health
		szHealth[MAX_HEALTH-1-i] = ((int)i<g_nHealth?170:169);
	}
	szHealth[MAX_HEALTH] = 0;
	if (g_bLargeViewport || g_bBigViewportMode)
		GraphDrawString( pVisView, g_pFont8x8, g_nViewOffsetX+g_nViewportPixelW-MAX_HEALTH*8, g_nViewOffsetY, (unsigned char*)szHealth );
	else
		GraphDrawString( pVisBack, g_pFont8x8, HEALTH_X, HEALTH_Y, (unsigned char*)szHealth );
	//*/

	// [dj2023-11] UNHARDCODING and doing more nicely generically. The below are ugly old hardcoded offsets into main old font.tga, we want to be able to not even load that and everythign else must work, so, separating small things like this into separate new png's [dj2023-11] so we can do French etc.
	extern djSprite* g_pBars;
	if (g_pBars!=nullptr && g_pBars->IsLoaded())
	{
		djImage* pImg=g_pBars->GetImage();
		int nW = g_pBars->GetSpriteW();
		int nH = g_pBars->GetSpriteH();
		for ( unsigned int i=0; i<MAX_HEALTH; ++i )
		{
			const int nSpriteOffset = ((int)i < g_nHealth ? 1 : 0);// unhealthy vs healthy bar sprite
			// X position offset:
			int nXOffset = ((MAX_HEALTH-1)*nW) - (i*nW);//<- this is the offset from the right side of the screen (or right side of healthbar)
			if (g_bLargeViewport || g_bBigViewportMode)
				djgDrawImageAlpha( pVisView, pImg, nSpriteOffset*nW, 0, g_nViewOffsetX+g_nViewportPixelW-MAX_HEALTH*nW + nXOffset, g_nViewOffsetY, nW, nH );
			else
				djgDrawImageAlpha( pVisBack, pImg, nSpriteOffset*nW, 0, HEALTH_X + nXOffset, HEALTH_Y, nW, nH );
		}
	}
}

void SetHealth(int nHealth)
{
	g_nHealth = djCLAMP(nHealth, 0, MAX_HEALTH);
	if (g_nHealth==0)
	{
#ifdef DAVEGNUKEM_CHEATS_ENABLED
		if (g_bGodMode)
		{
			g_nHealth = MAX_HEALTH;
			ShowGameMessage("GODMODE: Death ignored", 72);
		}
		else
			Die();
#else
		Die();
#endif
	}

	DrawHealth();
}

void update_health(int health_diff)
{
	// [dj2016-10-28] Keep health static if hero 'frozen' (e.g. going through teleporters or exits), otherwise
	// say a monster could kill you after you've already gone through an exit.
	if (HeroIsFrozen())
		return;
	SetHealth(g_nHealth + health_diff);
	// If busy jumping up and something hurts hero, stop the jump
	if (health_diff<0 && g_Player.hero_mode==MODE_JUMPING)
	{
		HeroCancelJump();
	}
}

void SetScore(int nScore)
{
	g_nScore = nScore;

	DrawScore();
}

void DrawScore()
{
	char score_buf[128]={0};
	snprintf( score_buf, sizeof(score_buf), "%10d", (int)g_nScore );
	// Display score
	if (g_bLargeViewport || g_bBigViewportMode)
	{
		// Don't need to clear behind as the game viewport is redrawn every frame underneath us
		GraphDrawString( pVisView, g_pFont8x8, (g_nViewOffsetX+g_nViewportPixelW) - 10*8, g_nViewOffsetY + 8/* +8 is to put it below health */, (unsigned char*)score_buf );
	}
	else
	{
		// Clear behind the score with part of the game skin
		if (pSkinGame)
			djgDrawImage( pVisBack, pSkinGame, SCORE_X, SCORE_Y, SCORE_X, SCORE_Y, 10*8, 8 );
		GraphDrawString( pVisBack, g_pFont8x8, SCORE_X, SCORE_Y, (unsigned char*)score_buf );
	}
}

void update_score(int score_diff, int nFloatingScoreXBlockUnits, int nFloatingScoreYBlockUnits)
{
	SetScore(g_nScore + score_diff);

	// If requested, create a "floating score" display
	if (nFloatingScoreXBlockUnits!=-1 && nFloatingScoreYBlockUnits!=-1)
		AddThing(CreateFloatingScore(nFloatingScoreXBlockUnits, nFloatingScoreYBlockUnits, score_diff));
}
/*-----------------------------------------------------------*/
void DrawThingsAtLayer(EdjLayer eLayer, float fDeltaTime_ms)
{
	CThing *pThing;
	unsigned int i;
	for ( i=0; i<g_apThings.size(); i++ )
	{
		pThing = g_apThings[i];
		if (pThing->Layer()==eLayer && pThing->IsInView())
			pThing->Draw(fDeltaTime_ms);
	}
}

void GameDrawView(float fDeltaTime_ms)
{
	int i=0,j=0,a=0,b=0,nXOffset=0;
	int anim_offset = 0;
	unsigned char *pLevelBlockPointer=NULL;

#ifdef djEFFECT_VIEWPORTSHADOW
	// dj2022-11 This function OnDrawFrameStart() for issue of e.g. hardware surface correspodnign to our image possibly needing to be re-fetched etc.
	g_Effect.OnDrawFrameStart();
#endif

	// Draw view background
	if (pBackground)
		djgDrawImage(pVisView, pBackground, 0, 0, g_nViewOffsetX, g_nViewOffsetY, g_nViewportPixelW, g_nViewportPixelH);

	// Clear viewport background before starting to draw game view in there
	// (If we don't, then the background doesn't clear where there are 'bg' (background) sprites)
	// (We don't want to draw the actual sprite as it has a little 'BG' on it to help with level editing)
	if (g_bBigViewportMode || pBackground==NULL)//<- The (only) reason we don't 'need' to do this if not in 'big viewport mode' is because of the pBackground image draw right above, effectively clears the viewport 'anyway' already for that section where there is background image
	{
		SDL_Rect rect;
		rect.x = g_nViewOffsetX;
		rect.y = g_nViewOffsetY;
		rect.w = g_nViewportPixelW;
		rect.h = g_nViewportPixelH;
		SDL_FillRect(pVisView->pSurface, &rect, SDL_MapRGB(pVisView->pSurface->format, 0, 0, 0));
		//djgClear(pVisView);
	}

#ifdef djEFFECT_VIEWPORTSHADOW
	// dj2022-11 These new values currently only used for this effectg but could maybe in future be used for more things if need be
	// It looks a bit complex but isn't really, it's just the position in world space of top left of viewport (in pixels) (doesn't change over drawing a single frame for all blocks)

	const int nViewportOriginX = g_Viewport.xo * BLOCKW + (g_Viewport.xo_small == 0 ? 0 : BLOCKW / 2);
	const int nViewportOriginY = g_Viewport.yo * BLOCKH;
#endif

	// todo ... more work on this. is this in front or behind pBackground?
	if (g_Level.ImgBack1()!=nullptr)
	{
		djgDrawImage(pVisView, g_Level.ImgBack1(), nViewportOriginX, nViewportOriginY, g_nViewOffsetX, g_nViewOffsetY, g_nViewportPixelW, g_nViewportPixelH);
	}

	//dj2019-07 Re this "10 seconds got to just after coke can, purple lab" comment: I don't know anymore what I meant with that (possibly something timing/benchmark-related),
	// but that comment was written in the 1990s, as the 'purple lab' was a computer lab at University of Pretoria where I studied .. for some reason I think of this comment often still when I think about this game so I want to leave this here:
	//(10 seconds got to just after coke can, purple lab)
	pLevelBlockPointer = (unsigned char *)(g_pLevel) + g_Viewport.yo*LEVEL_BYTESPERROW + (g_Viewport.xo*LEVEL_BYTESPERBLOCK);//was:(g_pLevel) + yo*512+(xo<<2);
	//const unsigned int uLevelPixelW = LEVEL_WIDTH*BLOCKW;
	//const unsigned int uLevelPixelH = LEVEL_HEIGHT*BLOCKH;

	int nYOffset = g_nViewOffsetY;
	for ( i=0; i<VIEW_HEIGHT; ++i )
	{
		nXOffset = -g_Viewport.xo_small + (g_bLargeViewport ? 0 : 2);
		nXOffset *= HALFBLOCKW;
		for ( j=0; j<VIEW_WIDTH+g_Viewport.xo_small; ++j )
		{
			// Bounds-checks to not 'buffer overflow' etc. by going past bottom (or right) of level [dj2016-10]
			if (g_Viewport.yo+i>=LEVEL_HEIGHT || g_Viewport.xo+j>=LEVEL_WIDTH)
			{
				// do nothing .. leave black
			}
			else
			{
				// BLOCK[2,3] -> background block
				a = *(pLevelBlockPointer+2);
				b = *(pLevelBlockPointer+3);
				// Animated block?
				anim_offset = (GET_EXTRA( a, b, 4 ) & FLAG_ANIMATED) ? anim4_count : 0;

				// draw background block
				//djgDrawImage( pVisView, g_pCurMission->GetSpriteData(a)->m_pImage, ((b+anim_offset)%16)*16, ((b+anim_offset)/16)*16, nXOffset*8,16+i*16,16,16 );
				if ((a | b) != 0)//<- This if prevents background clearing of 'bg' background block .. etiher we need to clear entire viewport before start drawing map, or must draw a black square here 'manually' .. not sure which is more efficient ultimately
				{
					DRAW_SPRITE16A(pVisView, a, b+anim_offset, nXOffset, nYOffset);

					// We include this in the above 'if' because it's not
					// strictly correct to have drop-shadows falling onto
					// the 'background skin' (i.e. when background sprite is 0)
					// as the background skin may include backdrops like
					// cityscapes etc. [dj2018-01]
					if (g_bAutoShadows && g_pLevelShadowMap!=NULL)
					{
						// Note several things could be slightly sped up here should this ever be a performance bottleneck
						unsigned char uShadowVal = *(g_pLevelShadowMap + ((g_Viewport.yo+i)*LEVEL_WIDTH) + (g_Viewport.xo+j));
						if (uShadowVal)
						{
							djgDrawImageAlpha( pVisView,
								g_pImgShadows,
								(uShadowVal % 16) * BLOCKW,
								(uShadowVal / 16) * BLOCKH,
								nXOffset, nYOffset,
								BLOCKW,BLOCKH );
						}
					}

#ifdef djEFFECT_VIEWPORTSHADOW
					// These shadowings look crap in simulated EGA/CGA retro so don't do if g_nSimulatedGraphics!=0
					//const bool bBackgroundSolid = CHECK_SOLID( a, b );
					extern int g_nSimulatedGraphics;
					if (g_Effect.m_nEnabledIntensity!=0 && g_nSimulatedGraphics==0 && g_Effect.m_pShadows!=nullptr && CHECK_SOLID(a, b)==0)//<-only non-solid blocks
					{
//dj2022-11 hm this is maybe not the most efficient .. could probably improve efficiency here slightly
						// [in pixels] hmm these values precise meaning etc. anmd how they're passed in could maybe use a re-think if/when we start abstracting away for more different games .. ideally we don't want *generic* effect code to "know about" crufty stuff like Dave Gnukem 1 specific "xo_small" viewport scrolling stuff (which itself is specifically meant to be there for the Duke Nukem 1 retro vibe)
						const float fBlockWorldXStart = (float)(j + g_Viewport.xo) * (float)BLOCKW;
		//					- (g_Viewport.xo_small ? ((float)BLOCKW / 2.f) : 0.f); //<- must compensate for extra 8-pixel viewport offset or the whole effect kinda looks like it 'wobbles' positionally slightly on the x axis as we walk left/right (this is purely an effect of DN1 scroling behaviour)
						const float fBlockWorldYStart = (float)(i + g_Viewport.yo) * (float)BLOCKH;

						g_Effect.DrawEffect(pVisView, j, i, nXOffset, nYOffset, nViewportOriginX, nViewportOriginY, fBlockWorldXStart, fBlockWorldYStart);
					}
#endif//#ifdef djEFFECT_VIEWPORTSHADOW
				}

				// BLOCK[0,1] -> foreground block
				a = *(pLevelBlockPointer);
				b = *(pLevelBlockPointer+1);
				// Animated block?
				anim_offset = (GET_EXTRA( a, b, 4 ) & FLAG_ANIMATED) ? anim4_count : 0;

				// draw foreground block, unless its (0,0)
				if ((a | b) != 0)
				{
					DRAW_SPRITE16A(pVisView, a, b+anim_offset, nXOffset, nYOffset);
				}
			}
			nXOffset += BLOCKW;
			pLevelBlockPointer += LEVEL_BYTESPERBLOCK;// <- 4 bytes per level 'block' so advance pointer 4 bytes, see comments at definition of LEVEL_SIZE etc.
		}
		nYOffset += BLOCKH;
		// The reason xo_small comes into it if advancing level pointer to next row, is that
		// if xo_small is 1, we literally actually effectively have a 1-block wider game viewport (as two 'halves' on left/right side of viewport) (keep in mind xo_small is either 0 or 1, IIRC) [dj2017-08]
		pLevelBlockPointer += (LEVEL_BYTESPERROW - ((VIEW_WIDTH+g_Viewport.xo_small) * LEVEL_BYTESPERBLOCK));//was:pLevelBlockPointer += (512 - ((VIEW_WIDTH+g_Viewport.xo_small)<<2));
	}

	// Draw pre-hero layers, then draw hero, then draw post-hero layers.
	DrawThingsAtLayer(LAYER_BOTTOM, fDeltaTime_ms);
	DrawThingsAtLayer(LAYER_2, fDeltaTime_ms);
	DrawThingsAtLayer(LAYER_MIDDLE, fDeltaTime_ms);
	// draw hero, but flash if he is currently hurt
	int yoff=0;
	if ((g_Player.nHurtCounter == 0) || (g_Player.nHurtCounter%3 != 0))
	{
		// no human being can really understand what this code was meant to be doing, surely [dj2017-12]
		// commenting out bits of it now to try clean it up a bit
		//xoff = (x_small - xo_small)+1 + ((x-xo)<<1);
		//yoff = 200+16+(y-yo-1)*16;

		//xoff = ((x_small - xo_small)+1)*8 + (x-xo) * 16;
		yoff = g_nViewOffsetY + (g_Player.y - g_Viewport.yo - 1) * BLOCKH;

		int xoff = (g_Player.x_small - g_Viewport.xo_small) + 1 + ((g_Player.x - g_Viewport.xo) << 1);
		xoff *= HALFBLOCKW;
		if (g_bLargeViewport) xoff -= BLOCKW;
		/*
		if (g_Player.hero_dir>0)
		{
			//tuxtest [dj2017-07 want to make hero sprite simpler to work on ultimately]
			DRAW_SPRITE16A(pVisView,4,   96+hero_picoffs*2  ,xoff   ,yoff   +y_offset);
			DRAW_SPRITE16A(pVisView,4,   96+hero_picoffs*2+1,xoff+16,yoff   +y_offset);
			DRAW_SPRITE16A(pVisView,4,16+96+hero_picoffs*2  ,xoff   ,yoff+16+y_offset);
			DRAW_SPRITE16A(pVisView,4,16+96+hero_picoffs*2+1,xoff+16,yoff+16+y_offset);
		}
		else
		//*/
		// See comments at g_nHeroJustFiredWeaponCounter declaration for what this is about
		// In short:
		// Immediately after firing weapon, the hero sprite is drawn slightly differently
		// briefly, which gives almost a slight 'recoil/kickback' visual effect, this ugly
		// global variable is to simulate that (this is based on the animation behaviour in
		// DN1, where it does that).
		if (g_nHeroJustFiredWeaponCounter>0)
		{
			--g_nHeroJustFiredWeaponCounter;
			int nOffs = (hero_picoffs+1)%4;
#ifdef djSPRITE_AUTO_DROPSHADOWS
			// 'Sprite auto dropshadows' effect
			DRAW_SPRITEA_SHADOW(pVisView,4,  g_Player.hero_dir*16+nOffs*4,1+xoff       ,1+yoff       +g_Player.y_offset,BLOCKW,BLOCKH);
			DRAW_SPRITEA_SHADOW(pVisView,4,2+g_Player.hero_dir*16+nOffs*4,1+xoff       ,1+yoff+BLOCKH+g_Player.y_offset,BLOCKW,BLOCKH-1);
			DRAW_SPRITEA_SHADOW(pVisView,4,1+g_Player.hero_dir*16+nOffs*4,1+xoff+BLOCKW,1+yoff       +g_Player.y_offset,BLOCKW,BLOCKH);
			DRAW_SPRITEA_SHADOW(pVisView,4,3+g_Player.hero_dir*16+nOffs*4,1+xoff+BLOCKW,1+yoff+BLOCKH+g_Player.y_offset,BLOCKW,BLOCKH-1);
#endif
			DRAW_SPRITE16A(pVisView,4,  g_Player.hero_dir*16+nOffs*4,xoff       ,yoff       +g_Player.y_offset);
			DRAW_SPRITE16A(pVisView,4,2+g_Player.hero_dir*16+nOffs*4,xoff       ,yoff+BLOCKH+g_Player.y_offset);
			DRAW_SPRITE16A(pVisView,4,1+g_Player.hero_dir*16+nOffs*4,xoff+BLOCKW,yoff       +g_Player.y_offset);
			DRAW_SPRITE16A(pVisView,4,3+g_Player.hero_dir*16+nOffs*4,xoff+BLOCKW,yoff+BLOCKH+g_Player.y_offset);
		}
		else
		{
#ifdef djSPRITE_AUTO_DROPSHADOWS
			// 'Sprite auto dropshadows' effect
			DRAW_SPRITEA_SHADOW(pVisView,4,  g_Player.hero_dir*16+hero_picoffs*4,1+xoff       ,1+yoff       +g_Player.y_offset,BLOCKW,BLOCKH);
			DRAW_SPRITEA_SHADOW(pVisView,4,2+g_Player.hero_dir*16+hero_picoffs*4,1+xoff       ,1+yoff+BLOCKH+g_Player.y_offset,BLOCKW,BLOCKH-1);
			DRAW_SPRITEA_SHADOW(pVisView,4,1+g_Player.hero_dir*16+hero_picoffs*4,1+xoff+BLOCKW,1+yoff       +g_Player.y_offset,BLOCKW,BLOCKH);
			DRAW_SPRITEA_SHADOW(pVisView,4,3+g_Player.hero_dir*16+hero_picoffs*4,1+xoff+BLOCKW,1+yoff+BLOCKH+g_Player.y_offset,BLOCKW,BLOCKH-1);
#endif
			DRAW_SPRITE16A(pVisView,4,  g_Player.hero_dir*16+hero_picoffs*4,xoff       ,yoff       +g_Player.y_offset);
			DRAW_SPRITE16A(pVisView,4,2+g_Player.hero_dir*16+hero_picoffs*4,xoff       ,yoff+BLOCKH+g_Player.y_offset);
			DRAW_SPRITE16A(pVisView,4,1+g_Player.hero_dir*16+hero_picoffs*4,xoff+BLOCKW,yoff       +g_Player.y_offset);
			DRAW_SPRITE16A(pVisView,4,3+g_Player.hero_dir*16+hero_picoffs*4,xoff+BLOCKW,yoff+BLOCKH+g_Player.y_offset);
		}
		if (bShowDebugInfo)
		{
			// Light blue box shows hero collision bounding box
			djgSetColorFore(pVisView,djColor(5,50,200));
			djgDrawRectangle(pVisView,
				xoff + HALFBLOCKW,
				yoff + g_Player.y_offset,
				HEROW_COLLISION,
				HEROH_COLLISION);
		}
	}
	DrawThingsAtLayer(LAYER_4, fDeltaTime_ms);
	DrawThingsAtLayer(LAYER_TOP, fDeltaTime_ms);

	// Draw bullets
	DrawBullets(fDeltaTime_ms);

#ifdef DAVEGNUKEM_CHEATS_ENABLED
	// God mode status display
	if (g_bGodMode) GraphDrawString(pVisView, g_pFont8x8, 32, 16, (unsigned char*)"GODMODE");
#endif

	// Draw debug info
	if (bShowDebugInfo) DrawDebugInfo();

	// In g_bLargeViewport mode, the health etc. are overlays, so must be drawn every frame.
	// In DN1-gameviewport mode, they don't need to be drawn every frame.
	if (g_bLargeViewport || g_bBigViewportMode)
	{
		DrawHealth();
		DrawScore();
		InvDraw();
		GameDrawFirepower();
	}

	// Flip the off-screen world viewport onto the backbuffer
	GraphFlipView(g_nViewportPixelW, g_nViewportPixelH, g_nViewOffsetX, g_nViewOffsetY, g_nViewOffsetX, g_nViewOffsetY);
}

// [dj2016-10] [fixme don't like these globals just floating here]
// Level Editor: New feature: Hold in Ctrl+Alt and click with the mouse to automatically start level with hero 'dropped in' to the clicked position as starting position (to help with level editing / testing)
int g_nOverrideStartX=-1;
int g_nOverrideStartY=-1;
void parse_level(void)
{
	int i, j;
	// parse the level (for doors, keys, hero starting position etc.)
	for ( i=0; i<LEVEL_HEIGHT; ++i )
	{
		for ( j=0; j<LEVEL_WIDTH; ++j )
		{
			sprite_factory( 0, 0, j, i, 0, true );
			sprite_factory( 0, 0, j, i, 1, true );
		}
	}
	
	// For level editor 'click to start hero here' function, added 2016-10:
	if (g_nOverrideStartX>=0 && g_nOverrideStartY>=0)
	{
		relocate_hero(g_nOverrideStartX,g_nOverrideStartY);
	}
	g_nOverrideStartX=-1;
	g_nOverrideStartY=-1;
}

void sprite_factory( unsigned char a, unsigned char b, int ix, int iy, int ifore, bool bfromlevel )
{
	unsigned char * plevel;
	unsigned char   b0, b1;
	bool            bWipeSprite = false;

	plevel = g_pLevel + LEVEL_BYTESPERBLOCK * (iy * LEVEL_WIDTH + ix) + (ifore * 2);

	if ( bfromlevel )
	{
		b0 = *(plevel    );
		b1 = *(plevel + 1);
	}
	else
	{
		b0 = a;
		b1 = b;
	}

	// Check if this block is a valid sprite
	if (g_pCurMission->GetSpriteData( b0 ) == NULL)
	{
		djMSG( "WARNING: INVALID SPRITESET ID FOUND (%d,%d) AT (%d,%d,%d)\n",
			b0, b1, ix, iy, ifore );
		if ( bfromlevel )
		{
			*(plevel    ) = 0;
			*(plevel + 1) = 0;
		}
		return;
	}

	// First attempt to allocate from "CThing" factory
	int nTypeID = g_pCurMission->GetSpriteData(b0)->m_type[b1];
	CThing *pThing = g_ThingFactory.Allocate(nTypeID);
	if (pThing)
	{
		pThing->SetLocation(ix, iy, 0, 0, 1, 1);
		pThing->SetSprite(b0, b1);
		pThing->Initialize(b0, b1);
		AddThing(pThing);
		bWipeSprite = true;
	}
	else // Special types
	{
		switch (nTypeID)
		{
		case TYPE_HEROSTART:
			// hero starting position
			relocate_hero( ix, iy );

			// By default start looking right, unless the start-looking-left is used [dj2017-08]
			g_Player.hero_dir = 1;//Right
			// Check the sprite metadata, the first
			if (GET_EXTRA(b0, b1, 0) == 0)
				g_Player.hero_dir = 0;//Left


			bWipeSprite = true;
			break;
		} //switch (block type)
	}

	// Remove from physical level data, as this has been converted
	// to a thing, or some other action taken
	if ( (bWipeSprite == true) && (bfromlevel == true) )
	{
		*(plevel    ) = 0;
		*(plevel + 1) = 0;
	}

}

void GameDrawSkin()
{
	if (g_bLargeViewport) return;
	// Draw the game skin
	if (pSkinGame)
		djgDrawImage( pVisBack, pSkinGame, 0, 0, CFG_APPLICATION_RENDER_RES_W, CFG_APPLICATION_RENDER_RES_H );
	//dj2023-11
	const std::string sLang=djGetLanguage();
	if (djLang::DoTranslations())
	{
		djSprite* spr = djDefaultFontSprite();
		//
		// Localized labels for "Health" etc.
		int x = HEALTH_X - 4;
		int y = 2+HEALTH_Y - 16;
		std::string sText = pgettext("ingame", "Health");
		GraphDrawStringUTF8(pVisBack, djDefaultFont(), x, y, spr->GetSpriteW(), spr->GetSpriteH(), sText.c_str(), sText.length());
		sText = pgettext("ingame", "Score");
		GraphDrawStringUTF8(pVisBack, djDefaultFont(), x, y - 32, spr->GetSpriteW(), spr->GetSpriteH(), sText.c_str(), sText.length());
		sText = pgettext("ingame", "Firepower");
		GraphDrawStringUTF8(pVisBack, djDefaultFont(), x, y + 40, spr->GetSpriteW(), spr->GetSpriteH(), sText.c_str(), sText.length());
		sText = pgettext("ingame", "Inventory");
		GraphDrawStringUTF8(pVisBack, djDefaultFont(), x, y + 80, spr->GetSpriteW(), spr->GetSpriteH(), sText.c_str(), sText.length());
	}
}

int GetCurrentLevel()
{
	return g_nLevel;
}

void SetLevel(int nLevel)
{
	g_nLevel = nLevel;
	djiWaitForKeyUp( DJKEY_L );
	if ( g_nLevel >= g_pCurMission->NumLevels() )
	{
		// You have finished the game!!! Woohoo!!!!
		djMSG( "-----------------------------------------\n" );
		djMSG( "You have finished the game!!! Woohoo!!!!.\n" );
		djMSG( "-----------------------------------------\n" );
		g_bGameRunning = false;
		return;
	}
	PerLevelSetup();
}

EJump g_eSaveJumpModeAtStartOfLevel = JUMP_NORMAL;//fixLOW ugly global, one day we'll make a class, one day. [dj2017-08]

void NextLevel()
{
	// Mark non-persistent inventory items as persistent
	InvMakeAllPersistent();
	// Go to next level
	SetLevel(GetCurrentLevel()+1);

	// We need to I think remember the current jumpheight here when we STARTED *THIS* level
	// So e.g. if you go into level editor and come out to RestartLevel() then your
	// jumpheight is what it was when you started that level. This is necessary to make
	// the behaviour work relatively 'correctly' re powerboots, if the powerboots are in
	// that level (or in a level preceding this one, say). [dj2017-08-09]
	g_eSaveJumpModeAtStartOfLevel = HeroGetJumpMode();
}

void RestartLevel()
{
	// Restore firepower and score to what they were when we started this level.
	SetScore(g_nScoreOld);
	HeroSetFirepower(g_nFirepowerOld);

	// Make sure health is full and redrawn as full [dj2017-07-29]
	SetHealth(MAX_HEALTH);

	// Set level to current level (restarts the level)
	SetLevel(g_nLevel);

	HeroSetJumpMode(g_eSaveJumpModeAtStartOfLevel);
}

void DrawDebugInfo()
{
	int i;
	djgSetColorFore(pVisView,djColor(255,50,255));
	for ( i=0; i<(int)g_apBullets.size(); i++ )
	{
		CBullet *pBullet = g_apBullets[i];
		if (OVERLAPS_VIEW(pBullet->x, pBullet->y, pBullet->x+8, pBullet->y+8))
		{
			djgDrawRectangle(pVisView,
				WORLDX2VIEW(pBullet->x),
				WORLDY2VIEW(pBullet->y),
				BULLET_WIDTH,
				BULLET_HEIGHT);
		}
	}

	// Time taken graph
	float fScale = 2.0f;
	djgSetColorFore(pVisView,djColor(150,150,150));
	for ( float fAxis=0; fAxis<=50.0f; fAxis += 10.0f )
	{
		djgDrawRectangle(pVisView,
			10, 170 - (int)(fAxis * fScale),
			MAX_DEBUGGRAPH, 1);
	}
	djgSetColorFore(pVisView,djColor(0,255,0));
	for ( i=0; i<(int)afTimeTaken.size(); i++ )
	{
		float f = afTimeTaken[i];
		djgDrawRectangle(pVisView,
			10 + i,170 - (int)(f * fScale),
			1, (int)(f * fScale));
	}

	int nNumVisible = 0;
	// DEBUG: Draw "action" bounding boxes
	CThing *pThing;
	for ( i=0; i<(int)g_apThings.size(); i++ )
	{
		pThing = g_apThings[i];

		// Draw 'shoot bounds' if applicable (grey?)
		djgSetColorFore(pVisView,djColor(128,128,128));
		if (pThing->IsShootable() && pThing->IsInView())
		{
			djgDrawRectangle( pVisView,
				CALC_XOFFSET(pThing->m_x) + pThing->m_iShootX1 + pThing->m_xoffset,
				CALC_YOFFSET(pThing->m_y) + pThing->m_iShootY1 + pThing->m_yoffset,
				(pThing->m_iShootX2 - pThing->m_iShootX1)+1,
				(pThing->m_iShootY2 - pThing->m_iShootY1)+1 );
		}

		// Draw "visible" bounds (red)
		// "Visible" box (red)
		djgSetColorFore(pVisView,djColor(255,0,0));
		if (pThing->IsInView())
		{
			nNumVisible++;
			// The following is um 'wrong' the red one. The purple one more 'correct', incorporates m_xoffset,m_yoffset (NB note lowercase) [dj2017-07-28]
			// red = 'wrong', ie not taking m_xoffset into account
			djgDrawRectangle(pVisView,
				CALC_XOFFSET(pThing->m_x)+pThing->m_iVisibleX1,
				CALC_YOFFSET(pThing->m_y)+pThing->m_iVisibleY1,
				(pThing->m_iVisibleX2-pThing->m_iVisibleX1)+1,
				(pThing->m_iVisibleY2-pThing->m_iVisibleY1)+1);
			// New purple box [dj2017-07] takes m_xoffset/m_yoffset into account
			djgSetColorFore(pVisView,djColor(200,0,200));//purple
			djgDrawRectangle(pVisView,
				1 + (CALC_XOFFSET(pThing->m_x)+pThing->m_iVisibleX1 + pThing->m_xoffset),
				1 + (CALC_YOFFSET(pThing->m_y)+pThing->m_iVisibleY1                  + pThing->m_yoffset),
				(pThing->m_iVisibleX2 - pThing->m_iVisibleX1)-1,
				(pThing->m_iVisibleY2 - pThing->m_iVisibleY1)-1);
		}

		// Draw action bounds (cyan=overlapping, white=inside, yellow=not interacting)
		if (pThing->OverlapsBounds(HERO_PIXELX, HERO_PIXELY))
		{
			if (pThing->IsHeroInside())
				pThing->DrawActionBounds(djColor(255,255,255));//white
			else
				pThing->DrawActionBounds(djColor(0,255,255));//cyan
		}
		else
			pThing->DrawActionBounds(djColor(255,255,0));//yellow
		// Draw solid bounds (green)
		if (pThing->m_bSolid)
		{
			djgSetColorFore( pVisView, djColor(0,255,0) );//green
			djgDrawRectangle( pVisView,
				CALC_XOFFSET(pThing->m_x) + pThing->m_iSolidX1 + pThing->m_xoffset,
				CALC_YOFFSET(pThing->m_y) + pThing->m_iSolidY1 + pThing->m_yoffset,
				(pThing->m_iSolidX2 - pThing->m_iSolidX1)+1,
				(pThing->m_iSolidY2 - pThing->m_iSolidY1)+1 );
		}
	}

	GraphDrawString(pVisView, g_pFont8x8, 32, 16, (unsigned char*)"Debug info on (D)" );
	char buf[256]={0};
	snprintf(buf,sizeof(buf), "%d things", (int)g_apThings.size());// VERY NB that we convert .size() to 'int' because on some platforms size() is 64-bit but int 32-bit so can crash otherwise if not careful!
	GraphDrawString(pVisView, g_pFont8x8, 32, 24, (unsigned char*)buf );
	snprintf(buf,sizeof(buf), "%d visible", nNumVisible);
	GraphDrawString(pVisView, g_pFont8x8, 32, 32, (unsigned char*)buf );
	snprintf(buf,sizeof(buf), "[%d,%d] [%d firepower]", g_Player.x, g_Player.y, g_nFirepower);
	GraphDrawString(pVisView, g_pFont8x8, 32, 40, (unsigned char*)buf );

	if (HeroIsFrozen())
	{
		snprintf(buf,sizeof(buf), "[FROZEN]");
		GraphDrawString(pVisView, g_pFont8x8, 32, 48, (unsigned char*)buf );
	}

	extern int nFrozenCount;
	snprintf(buf,sizeof(buf), "frozecount=%d", nFrozenCount);
	GraphDrawString(pVisView, g_pFont8x8, 32, 56, (unsigned char*)buf );
	//snprintf(buf,sizeof(buf), "[%d,%d,%d,%d]", x,y,x_small,y_offset);
	//GraphDrawString(pVisView, g_pFont8x8, 32, 56+8, (unsigned char*)buf );
	//snprintf(buf,sizeof(buf), "hero_mode=%d", hero_mode);
	//GraphDrawString(pVisView, g_pFont8x8, 32, 62, (unsigned char*)buf );

	//snprintf(buf,sizeof(buf), "xo,yo=%d,%d", xo, yo);
	//GraphDrawString(pVisView, g_pFont8x8, 32, 70, (unsigned char*)buf );
	// todo, add: endianness indication, RGBA bitmasks etc.? also new localization info.

	std::string s = djDATAPATHs(std::string(g_pCurMission->GetLevel(g_nLevel)->GetFilename()) + "_back1.png");
	if (!djFileExists(s.c_str())) s="N:" + s; else s="Y:" + s; 
	GraphDrawStringUTF8(pVisView, djDefaultFont(), 32, 80, 8, 8, s.c_str());
	if (g_Level.Back1())//2023 back1 stuff (debug info)
	{
		s = std::to_string(g_Level.Back1()->GetSpriteW()) + " "
			+ std::to_string(g_Level.Back1()->GetSpriteH()) + " "
			+ std::to_string(g_Level.Back1()->GetNumSpritesX()) + " "
			+ std::to_string(g_Level.Back1()->GetNumSpritesY());
		if (g_Level.Back1()->GetImage())
		{
			s += " " + std::to_string(g_Level.Back1()->GetImage()->Width()) + " "
				+ std::to_string(g_Level.Back1()->GetImage()->Height()) + " ";
		}
		GraphDrawStringUTF8(pVisView, djDefaultFont(), 32, 88, 8, 8, s.c_str());
	}
}

void DrawBullets(float fDeltaTime_ms)
{
	unsigned int i;
	CBullet* pBullet=NULL;
	for ( i=0; i<g_apBulletsDeleted.size(); ++i )
	{
		pBullet = g_apBulletsDeleted[i];
		if (OVERLAPS_VIEW(pBullet->x, pBullet->y, pBullet->x+BULLET_WIDTH, pBullet->y+BULLET_HEIGHT))
		{
			pBullet->Draw(fDeltaTime_ms);
		}
	}
	for ( i=0; i<g_apBullets.size(); ++i )
	{
		pBullet = g_apBullets[i];
		if (OVERLAPS_VIEW(pBullet->x, pBullet->y, pBullet->x+BULLET_WIDTH, pBullet->y+BULLET_HEIGHT))
		{
			pBullet->Draw(fDeltaTime_ms);
		}
	}
}

bool check_solid( int ix, int iy, bool bCheckThings )
{
	int i;

	// Create an invisible "border" around the level. Handy catch-all for things going out of bounds.
	//(dj2019-07 LOWPrio It's debatable here whether we might actually want this to go all the way to the edges perhaps,
	// e.g. test if ix<0 or ix>LEVEL_WIDTH .. maybe for other games .. must make sure no crashing issues etc. .. behaviour should be game-dependent)
//#ifdef tBUILD_DAVEGNUKEM1
	if ( ix<1 || iy<1 || ix>(LEVEL_WIDTH-2) || iy>(LEVEL_HEIGHT-2) ) return true;
	//if ( ix<0 || iy<-2 || ix>(LEVEL_WIDTH) || iy>(LEVEL_HEIGHT+5) ) return true;
//#else
//#endif

	// FIXME: This is not speed-optimal (does it really matter?)

	// If foreground block is solid
	if (CHECK_SOLID( LEVCHAR_FOREA(ix,iy), LEVCHAR_FOREB(ix,iy) )) return true;
	// If background block is solid
	if (CHECK_SOLID( LEVCHAR_BACKA(ix,iy), LEVCHAR_BACKB(ix,iy) )) return true;

	// Check "things"
	if (bCheckThings)
	{
		CThing *pThing = NULL;
		for ( i=0; i<(int)g_apThings.size(); i++ )
		{
			pThing = g_apThings[i];
			if (pThing->m_bSolid)
			{
				//fixmeHIGH 'In theory' we shoudl add m_xoffset etc. but not sure
				// that's the correct thing to do as e.g. cf. this is the code
				// that checks a.o. whether things should fall ... could things
				// end up falling when they're not supposed to or vice versa if
				// we suddenly change that now, since it's been seemingly working
				// for so long?? If we change this now a lot of things
				// must be carefully re-tested .. [dj2018-01-12]
				if (OVERLAPS(
					ix*BLOCKW, iy*BLOCKH, ix*BLOCKW+(BLOCKW-1), iy*BLOCKH+(BLOCKH-1),
					pThing->m_x*BLOCKW + pThing->m_iSolidX1,
					pThing->m_y*BLOCKH + pThing->m_iSolidY1,
					pThing->m_x*BLOCKW + pThing->m_iSolidX2,
					pThing->m_y*BLOCKH + pThing->m_iSolidY2))
				{
					return true;
				}
			}
			//if (pThing->CheckSolid(ix, iy))
			//	return true;
		}
	}

	return false;
}

bool CheckCollision(int x1, int y1, int x2, int y2, CBullet *pBullet)
{
	int i, j;
	int nX1 = x1 / BLOCKW;
	int nY1 = y1 / BLOCKH;
	int nX2 = x2 / BLOCKW;
#ifdef tBUILD_DAVEGNUKEM1
	//dj2018-03 Hacky - technically "wrong" but emulates DN1 behavior, and I quite like it because it squares the unfairness in some situations eg flyingrobot shoot over barrel cf 25 Mar 2018 issue. Basically in DN1 your monsters fly over solid single block in front of you like barrel(yellow can), but still hit boxes [which are same position/size etc.] ... so technically that aspect of the physics "doesn't make sense" but this setting nY2 to nY1 emulates the not-making-sense for solid-blocks (check_solid() function) but NOT for CThing's (which boxes are) - so boxes remain shootable, but we can 'shoot over' barrels :)
	// This is um basically 'very Duke-Nukem-1-specific', probably, I think [dj2018-03] - if we ever make this more generic engine for other games, may want nY2 = y2 / 16; rather
	int nY2 = nY1;
#else
	int nY2 = y2 / BLOCKH;
#endif
	for ( i=nX1; i<=nX2; ++i )
	{
		for ( j=nY1; j<=nY2; ++j )
		{
			if (check_solid(i, j, false))
			{
				if (OVERLAPS(x1, y1, x2, y2, i*BLOCKW, j*BLOCKH, i*BLOCKW+(BLOCKW-1), j*BLOCKH+(BLOCKH-1)))
					return true;
			}
		}
	}
	CThing *pThing;
	for ( i=0; i<(int)g_apThings.size(); ++i )
	{
		pThing = g_apThings[i];
		if (pThing->m_bSolid)
		{
			//fixmeHIGH need m_xoffset etc. also here?
			if (pThing->OverlapsShootArea(x1,y1,x2,y2))
			{
				if (pBullet!=NULL && pBullet->eType==CBullet::BULLET_HERO && pThing->IsShootable())
				{
					int nRet = pThing->OnHeroShot();
					if (nRet==THING_DIE)
					{
						delete pThing;
						g_apThings.erase(g_apThings.begin() + i);
					}
					else if (nRet==THING_REMOVE)
					{
						g_apThings.erase(g_apThings.begin() + i);
					}
				}
				return true;
			}
		}
	}

	return false;
}


int GameLoadSprites()
{
	int i;

	// Load the raw sprite data
	i = g_pCurMission->LoadSprites();
	if ( i != 0 )
	{
		djMSG( "GameLoadSprites(): Unable to load raw sprite data\n" );
		return i;
	}

	// Convert sprite data to format useful for ggi
	// FIXME: This doesn't happen at the moment. It currently doesn't
	// need to do anything of the sort, but should in future to take
	// advantage of (for example) hardware acceleration, but at the
	// moment it just draws the sprites straight out of the djImage.

	// i iterates through the 256 possible ID's for spritesets
	for ( i=0; i<256; i++ )
	{
		CSpriteData * pSpriteData;

		pSpriteData = g_pCurMission->GetSpriteData( i );
		if (pSpriteData != NULL)
		{
			// The sprite data must now be converted to a format useful for ggi
		}
	}

	return 0;
}

void GameDrawFirepower()
{
	if (g_bLargeViewport || g_bBigViewportMode)
	{
		// Draw firepower
		int nX = g_nViewOffsetX;
		int nY = (g_nViewOffsetY+g_nViewportPixelH) - BLOCKH;

		for ( int i=0; i<g_nFirepower; ++i )
		{
			DRAW_SPRITE16A( pVisView, 5, 0, nX + i*BLOCKW, nY );
		}
	}
	else
	{
		// First clear firepower display area with game skin
		djgDrawImage( pVisBack, pSkinGame, FIREPOWER_X, FIREPOWER_Y, FIREPOWER_X, FIREPOWER_Y, BLOCKW*5, BLOCKH );
		// Draw firepower
		for ( int i=0; i<g_nFirepower; ++i )
		{
			DRAW_SPRITE16A( pVisBack, 5, 0, FIREPOWER_X + i*BLOCKW, FIREPOWER_Y );
		}
	}
}

void AddThing(CThing *pThing)
{
	if (pThing!=NULL) // This can be NULL in some cases
	{
		g_apThings.push_back(pThing);
		pThing->OnAdded();
	}
}

void HeroModifyFirepower(int nDiff)
{
	HeroSetFirepower(g_nFirepower + nDiff);
}

void HeroSetFirepower(int nFirepower)
{
	g_nFirepower = djCLAMP(nFirepower, 1, MAX_FIREPOWER);
	GameDrawFirepower();
}

bool SaveGame()
{
	std::string s = djAppendPathStr(djGetFolderUserSettings().c_str(), USERFILE_SAVEGAME);
	FILE *pOut = djFile::dj_fopen(s.c_str(), "w");
	if (pOut==NULL)
		return false;

	//From 2016-10-28 save a 'format version', to make it easier if we change the file format in future to keep things sane
	fprintf(pOut, "fileversion=2\n");
	//dj2016-10-28 Add load/save of current 'mission' (otherwise if we save game as say level 4 of 3rd mission then do save game from mission 1 it loads incorrectly to level 4 of mission 1). Note though, want to try keep this file format though 'backward-compatible' i.e. want older versions to still load
	// We try use the filename to 'know' which 'mission' to load again (as this is probably the least likely to change)
	fprintf(pOut, "mission=%s\n",g_pCurMission!=NULL ? g_pCurMission->GetFilename().c_str() : "");

	fprintf(pOut, "%d\n", g_nLevel);
	// We use the "old" values because we must save the values we had when we *started* this level.
	// Same with health.
	fprintf(pOut, "%d\n", g_nScoreOld);
	fprintf(pOut, "%d\n", g_nFirepowerOld);
	fprintf(pOut, "%d\n", g_nHealthOld);
	InvSave(pOut); // Save inventory

	fclose(pOut);
	return true;
}

bool LoadGame()
{
	std::string s = djAppendPathStr(djGetFolderUserSettings().c_str(), USERFILE_SAVEGAME);

	if (!djFileExists(s.c_str()))// [dj2022-11]
		return false;

	FILE *pIn = djFile::dj_fopen(s.c_str(), "r");
	if (pIn==NULL)
		return false;
	int nLevel=0, nScore=0, nFirepower=0, nHealth=0;

	//dj2016-10-28 Add load/save of current 'mission' (otherwise if we save game as say level 4 of 3rd mission then do save game from mission 1 it loads incorrectly to level 4 of mission 1). Note though, want to try keep this file format though 'backward-compatible' i.e. want older versions to still load
	// Note here if loading a file saved with a build prior to 28 October 2016, then at this point we're already 'at the end' of the file, in which case we can't know the correct 'mission', however, we can assume it's probably most likely to have been the default game, so just use that as default.
	std::string sMissionFilename = "default.gam";

	// Check for 'fileversion=', which was only added 2016-10-28 .. if not present, then the first line is the level number.
	char szLine[4096]={0};
	if (dj_fscanf_line(pIn, szLine) <= 0) { SYS_Error("Error loading game"); return false; }
	if (strncmp(szLine,"fileversion=",12)==0)
	{
		if (dj_fscanf_line(pIn, szLine) <= 0) { SYS_Error("Error loading game"); return false; }

		// Fileversion 2
		//char szFilename[4096]={0};//gross
		//[LOW] Wonder if we might have cross-platform issues here? E.g. savegame on Windows with CR+LF & try load on e.g. a LF-only platform say?

		//if (djFSCANF(pIn,"mission=%s\n",szFilename, 4096) <= 0) { SYS_Error("Error loading game"); return false; }
		if (strncmp(szLine, "mission=", 8) == 0)
		{
			//strncat(szFilename, szLine+8, sizeof(szLine)-10);
			//if (szFilename[0] != 0)
			//{
				sMissionFilename = (szLine + 8);
			//}
		}
		else
		{
			SYS_Error("Error loading game");
			return false;
		}

		if (dj_fscanf_intline(pIn, nLevel) <= 0) { SYS_Error("Error loading game"); return false; }
	}
	else
	{
		// Fileversion 1
		nLevel = atoi(szLine);
	}
	djMSG("LOADGAME: Mission=%s\n", sMissionFilename.c_str());
	// Find the mission from the list of missions
	CMission* pMission = g_apMissions[0];//<-Default to the first one
	bool bFound = false;
	for ( std::vector<CMission * >::const_iterator iter=g_apMissions.begin(); iter!=g_apMissions.end(); ++iter )
	{
		if (sMissionFilename==(*iter)->GetFilename())
		{
			pMission = *iter;
			bFound = true;
			break;
		}
	}
	if (!bFound)
	{
		// warning ? failed to find mission, falling back to default one
		//SYS_Warn("Error loading game [unknown mission]");
		//PerGameCleanup();
		//return false;
	}
	if (pMission != g_pCurMission)
	{
		PerGameCleanup();

		g_pCurMission = pMission;

		PerGameSetup();
	}

	//fscanf(pIn, "%d\n", &nLevel);
	if (dj_fscanf_intline(pIn, nScore) <= 0) { SYS_Error("Error loading game [score]"); return false; }
	if (dj_fscanf_intline(pIn, nFirepower) <= 0) { SYS_Error("Error loading game [firepower]"); return false; }
	if (dj_fscanf_intline(pIn, nHealth) <= 0) { SYS_Error("Error loading game [health]"); return false; }
	djMSG("LOADGAME: Level=%d Score=%d Firepower=%d Health=%d\n", nLevel, nScore, nFirepower, nHealth);
	SetLevel(nLevel);
	SetScore(nScore);
	SetHealth(nHealth);
	g_nFirepower = nFirepower;
	// Load inventory. We do this after level setup stuff, otherwise it gets cleared in per-level setup..
	if (!InvLoad(pIn))
	{
		SYS_Error("Error loading game inventory");
		//fclose(pIn);
		//return false;
	}

	fclose(pIn);
	// Set current "old" values, otherwise if save a game immediately after loading one, the existing
	// game's values (and not the loaded games values) get saved
	g_nScoreOld = g_nScore;
	g_nFirepowerOld = g_nFirepower;
	g_nHealthOld = g_nHealth;
	return true;
}

void ShowGameMessage(const char *szMessage, int nFrames)
{
	g_sGameMessage = szMessage;
	g_nGameMessageCount = nFrames;
}
//-------------------------------------------------------
void IngameMenu()
{
	//dj2022-11 Just made gameMenuItems stuff non-global and function scope so I can more easily add 'on/off' info .. but should think about all that
	/*--------------------------------------------------------------------------*/
#ifdef djEFFECT_VIEWPORTSHADOW
	const std::string sViewportShadows = (g_Effect.m_nEnabledIntensity == 0 ? "   Viewport shadow: off" : std::string("   Viewport shadows: ") + std::to_string(g_Effect.m_nEnabledIntensity) + "  ");
#endif
	const std::string sAutoShadows = (g_bAutoShadows ? "   Map auto shadows: on   " : "   Map auto shadows: off  ");
#ifdef djSPRITE_AUTO_DROPSHADOWS
	const std::string sSpriteShadows = (g_bSpriteDropShadows ? "   Sprite shadows: on  " : "   Sprite shadows: off ");
#endif
	struct SMenuItem gameMenuItems[] =
	{
		{ false, "                       " },
		{ true,  pgettext("ingamemenu", "Continue") },
		{ true,  pgettext("ingamemenu", "Save Game") },
		{ true,  pgettext("ingamemenu", "Restore Game") },
		{ true,  pgettext("ingamemenu", "Instructions") },
		{ true,  pgettext("ingamemenu", "Retro Settings"), "show_retrosettings_menu" },//dj2019-06 new
		#ifdef djEFFECT_VIEWPORTSHADOW
		{ true,  sViewportShadows.c_str(), "setting/betashadoweffect" },
		#endif
		{ true,  sAutoShadows.c_str(), "setting/autoshadows" },
		#ifdef djSPRITE_AUTO_DROPSHADOWS
		{ true,  sSpriteShadows.c_str(), "setting/spriteshadows" },
		#endif
		#if defined(djCFG_FORCE_FULLSCREEN_ALWAYS) || defined(djCFG_FORCE_WINDOWED_ALWAYS)
		#else
		#ifdef djINGAME_FULLSCREEN_TOGGLE//dj2022-11 will think about these names, hmm
		{ true,  pgettext("ingamemenu", "Fullscreen"), "ingame/toggle_fullscreen" },//Hmm note some consoles/ports might only work in fullscreen mode or something? should have 
		#endif
		#endif
		{ true,  pgettext("ingamemenu", "Abort Game"), "ingame/abort_game" },
		{ false, "                       " },
		{ false, "" }//NB slightly old-fashioned indication of final 'null' item, without this bad things happen [someday to refactor nicer]
	};
	const unsigned char gameMenuCursor[] = { 128, 129, 130, 131, 0 };
	CMenu gameMenu("game.cpp:gameMenu");

	// Set up the in-game menu
	gameMenu.setClrBack(djColor(48, 66, 128));
	gameMenu.setSize(0);
	gameMenu.setItems(gameMenuItems);
	gameMenu.setMenuCursor(gameMenuCursor);
	gameMenu.setXOffset(-1); // Calculate menu position for us
	gameMenu.setYOffset(-1);
	/*--------------------------------------------------------------------------*/



	int nMenuOption = do_menu( &gameMenu );
	//dj2022-11 testing moving away from gross hardcoded numbers for menu return values but only a few implemented so far ..
	// hm should maybe make a new small wrapper for this
	std::string sSelectedMenuCommand;
	if (nMenuOption >= 0 && !gameMenu.getItems()[nMenuOption].GetRetVal().empty())
		sSelectedMenuCommand = gameMenu.getItems()[nMenuOption].GetRetVal();
#ifdef djEFFECT_VIEWPORTSHADOW
	//dj2022-11 testing new beta shadow effect
	if (sSelectedMenuCommand == "setting/betashadoweffect")
	{
		g_Effect.SetIntensity(g_Effect.m_nEnabledIntensity + 1);
		std::string sMsg = "beta shadow effect toggled to: ";
		if (g_Effect.m_nEnabledIntensity == 0) sMsg += "OFF";
		else
			sMsg += std::to_string(static_cast<int>(g_Effect.m_nEnabledIntensity));
		djConsoleMessage::SetConsoleMessage(sMsg);
		return;
	}
#endif

	// 'auto dropshadows' effect (on level parts)
	if (sSelectedMenuCommand == "setting/autoshadows")
	{
		g_bAutoShadows = !g_bAutoShadows;
		std::string sMsg = "Auto shadow effect setting turned ";
		if (g_bAutoShadows) sMsg += "OFF"; else sMsg += "ON";
		djConsoleMessage::SetConsoleMessage(sMsg);
		return;
	}

#ifdef djSPRITE_AUTO_DROPSHADOWS
	// 'Sprite auto dropshadows' effect
	if (sSelectedMenuCommand == "setting/spriteshadows")
	{
		g_bSpriteDropShadows = !g_bSpriteDropShadows;
		std::string sMsg = "Sprite shadow effect setting turned ";
		if (g_bSpriteDropShadows) sMsg += "OFF"; else sMsg += "ON";
		djConsoleMessage::SetConsoleMessage(sMsg);
		return;
	}
#endif

#ifdef djINGAME_FULLSCREEN_TOGGLE
	if (sSelectedMenuCommand == "ingame/toggle_fullscreen")
	{
		//dj2022-11 experimental toggle fullscreen probably going to crash a lot
		djGraphicsSystem::ToggleFullscreen();
		//ReInitGameViewport();
		RedrawEverythingHelper();
		//bForceUpdate = true;
		return;
	}
#endif
	if (sSelectedMenuCommand == "ingame/abort_game")
	{
		g_bGameRunning = false;
		return;
	}
	else if (sSelectedMenuCommand == "show_retrosettings_menu")//dj2019-06 just-for-fun extra-retro simulated faux-EGA/CGA
	{
		extern void SettingsMenu();
		SettingsMenu();
		return;
	}

	switch (nMenuOption)
	{
	case 2:
		if (SaveGame())
			ShowGameMessage("Saved checkpoint at\nlast-completed level", 32);
		break;
	case 3:
		if (!LoadGame())
			ShowGameMessage("Game load failed", 32);
		break;
	case 4://dj2017-06-22 Add in-game help screen
		{
			ShowInstructions();
		}
		break;
	}
}
