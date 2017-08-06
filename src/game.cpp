//
// game.cpp
//
// 1995/07/28
/*
Copyright (C) 1995-2017 David Joffe

License: GNU GPL Version 2
*/
/*--------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

#include "mmgr/nommgr.h"
#include <string>
using namespace std;
#include "mmgr/mmgr.h"

#ifdef WIN32
#else
#include <unistd.h>
#endif


#include "mission.h"
#include "hero.h"
#include "inventory.h"
#include "thing.h"
#include "graph.h"
#include "game.h"
#include "graph.h"
#include "block.h"
#include "level.h"
#include "djgamelib.h"
#include "menu.h"
#include "keys.h"
#include "ed.h"
#include "bullet.h"
#include "hiscores.h"
#include "sys_log.h"
#include "djstring.h"//djStrPrintf
#include "instructions.h"//Slightly don't like this dependency. For ShowInstructions() from in-game menu. [dj2017-08]

#ifndef NOSOUND
#include <SDL_mixer.h>//For background music stuff
Mix_Music* g_pGameMusic=NULL;
#endif

// See comments at viewport vertical auto-scrolling. DN1 vertical viewport auto-re-centering has this subtle feel of almost taking a frame or three to start 'catching up' to jumping upwards etc. ... this variable helps implement that effect. [dj2017-06-29]
int g_nRecentlyFallingOrJumping = 0;

// The original Duke Nukem 1 has a 13x10 'blocks' viewport, though in future we could use this
// to either allow larger game viewport for this game, or have 'derived' games using this 'engine' with
// larger viewports [dj2016-10]
const int VIEW_WIDTH_DEFAULT=13;//In number of game 'blocks'
int VIEW_WIDTH = VIEW_WIDTH_DEFAULT;
int VIEW_HEIGHT = 10;

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
void DrawBullets();
// Draw debug info
void DrawDebugInfo();

void IngameMenu();

/*--------------------------------------------------------------------------*/
//
// Sound files
//

const char *g_szSoundFiles[SOUND_MAX] =
{
	"data/sounds/pickup.wav",
	"data/sounds/shoot_cg1_modified.wav",//<- Hero shoot sound
	"data/sounds/exit.ogg",//<-dj2016-10-28 New proper exit sound (is "PowerUp13.mp3" by Eric Matyas http://soundimage.org/)
	"data/sounds/wooeep.wav",
	"data/sounds/explode.wav",
	"data/sounds/sfx_weapon_singleshot7.wav"//<- Monster shoot sound
	,"data/sounds/jump.wav"//dj2016-10-30
	,"data/sounds/jump_landing.wav"//dj2016-10-30
	,"data/sounds/soft_explode.wav"//dj2016-10-30
	,"data/sounds/key_pickup.wav"//dj2016-10-30
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

// Yeah its pretty low but thats what I was aiming for back in the EGA days
// and I don't feel like changing it right now. I might still though.
float g_fFrameRate=18.0f;

#define MAX_HEALTH (10)
#define HEALTH_INITIAL MAX_HEALTH

#define MAX_FIREPOWER (5)

unsigned char *g_pLevel = NULL;
int g_nLevel = 0;
bool bShowDebugInfo = false;
vector<CThing *> g_apThings;

string g_sGameMessage;
int g_nGameMessageCount = -1;

vector<CBullet*> g_apBullets;

// Return number of bullets in bullet system that were fired by hero
int CountHeroBullets()
{
	unsigned int i;
	int nCount = 0;
	for ( i=0; i<g_apBullets.size(); i++ )
	{
		if (g_apBullets[i]->eType==CBullet::BULLET_HERO)
			nCount++;
	}
	return nCount;
}

vector<float> afTimeTaken;
#define MAX_DEBUGGRAPH 128

const char *FILE_GAMESKIN = "data/gameskin.tga";
djImage *pSkinGame        = NULL; // Main game view skin (while playing)
djImage *pBackground      = NULL; // Level background image

/*--------------------------------------------------------------------------*/
struct SMenuItem gameMenuItems[] =
{
	{ false, "                   " },
	{ true,  "   Continue        " },
	{ true,  "   Save Game       " },
	{ true,  "   Restore Game    " },
	{ true,  "   Instructions    " },
	{ true,  "   Abort Game      " },
	{ false, "                   " },
	{ false, NULL }
};
unsigned char gameMenuCursor[] = { 128, 129, 130, 131, 0 };
CMenu gameMenu ( "game.cpp:gameMenu" );
/*--------------------------------------------------------------------------*/

//animation
int anim4_count = 0;
int nSlowDownHeroWalkAnimationCounter = 0;




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
// Game functions
/*-----------------------------------------------------------*/
// Once off initialization stuff
void GameInitialSetup()
{
	SYS_Debug( "GameInitialSetup()\n" );

	InitHighScores();
	LoadHighScores();		// Load high scores

	// Set up the in-game menu
	gameMenu.setClrBack( djColor(48,66,128) );
	gameMenu.setSize(0);
	gameMenu.setItems(gameMenuItems);
	gameMenu.setMenuCursor(gameMenuCursor);
	gameMenu.setXOffset(-1); // Calculate menu position for us
	gameMenu.setYOffset(-1);

	SYS_Debug ( "GameInitialSetup(): loading sounds\n" );
	// Load the game sounds
	int i;
	for ( i=0; i<SOUND_MAX; i++ )
	{
		g_iSounds[i] = djSoundLoad( g_szSoundFiles[i] );
	}

	// Load main game skin
	if (!pSkinGame)
	{
		pSkinGame = new djImage;
		pSkinGame->Load( FILE_GAMESKIN );
		djCreateImageHWSurface( pSkinGame );
	}

	// Register the "thing"'s that need to be registered dynamically at runtime [dj2017-07-29]
	RegisterThings_Monsters();
}

// Final cleanup
void GameFinalCleanup()
{
	SYS_Debug( "GameFinalCleanup()\n" );

	djDestroyImageHWSurface(pSkinGame);
	djDEL(pSkinGame);

	// Unload the game sounds (FIXME)

	KillHighScores();
}


// Per-game initialization
void PerGameSetup()
{
	Log("PerGameSetup(): InitLevelSystem()\n");
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
	int i;

	Log ( "PerLevelSetup()\n" );

	g_nRecentlyFallingOrJumping=0;

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
	// This is somewhat gross quick n dirty simplistic for now - should rather have ability to assign music file in the level file format [dj2016-10]
	int nMusicFile = (g_nLevel % asMusicFiles.size());
	std::string sBasePath = "data/music/eric_matyas/";
	if (g_pGameMusic!=NULL)
	{
		Mix_FreeMusic(g_pGameMusic);
		g_pGameMusic = NULL;
	}
	g_pGameMusic = Mix_LoadMUS((sBasePath + asMusicFiles[nMusicFile]).c_str());
	if (g_pGameMusic!=NULL)
	{
		Mix_FadeInMusic(g_pGameMusic, -1, 500);
	}

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
	x_small = 0; // hero not half-block offset
	xo_small = 0; // view not half-block offset
	y_offset = 0;
	// just in case level doesn't contain a starting block ..
	xo = 0;
	yo = 0;
	relocate_hero( 20, 20 );
	hero_dir = 1;

	// clear list of "things"
	for ( i=0; i<(int)g_apThings.size(); i++ )
	{
		delete g_apThings[i];
	}
	g_apThings.clear();

	// (2) Load the currently selected level
	const char * szfilename = g_pCurMission->GetLevel( g_nLevel )->GetFilename( );

	// always keep current level loaded at slot 0
	SYS_Debug ("PerLevelSetup(): level_load( %s )\n", szfilename );
	if (NULL == level_load( 0, szfilename ))
	{
		djMSG("PerLevelSetup(): error loading level %s.\n", szfilename );
		ShowGameMessage("BAD FILENAME FOR LEVEL", 1000);
		//return;
	}
	g_pLevel = apLevels[0];

	// Load map background image
	pBackground = new djImage;
	if (0!=pBackground->Load(g_pCurMission->GetLevel(g_nLevel)->m_szBackground))
	{
		djDEL(pBackground);
	}
	djCreateImageHWSurface( pBackground );

	// Clear out inventory
	InvClear();
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
	Log ( "KillLevelSystem() ok\n" );
}

void PerLevelCleanup()
{
	if (g_pGameMusic!=NULL)
	{
		Mix_FreeMusic(g_pGameMusic);
		g_pGameMusic = NULL;
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
	GameDrawView();
	// Draw inventory
	InvDraw();
	TRACE("game_startup(): update_score()\n");
	update_score(0);
	TRACE("game_startup(): draw_health()\n");
	update_health(0);

	g_bGameRunning = true;

	GameDrawFirepower();

	//dj2016-10-28 Used if doing 'Restore Game' from *main* game menu. [This is perhaps slightly spaghetti-ish, it's done this way as LoadGame() has been originally
	// written under the assumption of doing *in-game* loading of a savegame.
	if (bLoadGame)
	{
		LoadGame();
	}

	GraphFlip(!g_bBigViewportMode);

	// try maintain a specific frame rate
	/*const */float fTIMEFRAME = (1.0f / g_fFrameRate);

	float fTimeFirst = djTimeGetTime();

	// Start out by being at next time
	float fTimeNext = djTimeGetTime();
	float fTimeNow = fTimeNext;
	int iFrameCount=0;
	int anKeyState[KEY_NUMKEYS] = { 0 };
	int i;
	while (g_bGameRunning)
	{

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
			SDL_Delay(1);

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
			int key_down_edge[KEY_NUMKEYS] = {0};
			while (djiPollEvents(Event))
			{
				switch (Event.type)
				{
				case SDL_KEYDOWN:
					for ( i=0; i<KEY_NUMKEYS; i++ )
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
						/*{
						char buf[1024]={0};
						sprintf(buf,"%08x,%08x,%08x",(int)Event.key.keysym.sym, (int)Event.key.keysym.mod, (int)Event.key.keysym.scancode);
						ShowGameMessage(buf, 32);
						}*/
						if (Event.key.keysym.sym==SDLK_F6)
						{
							g_fFrameRate -= 1.0f;
							if (g_fFrameRate<1.f)
								g_fFrameRate = 1.f;
							fTIMEFRAME = (1.0f / g_fFrameRate);
							char buf[1024]={0};
							sprintf(buf,"Dec framerate %.2f",g_fFrameRate);
							ShowGameMessage(buf, 32);
						}
						else if (Event.key.keysym.sym==SDLK_F7)
						{
							g_fFrameRate += 1.0f;
							fTIMEFRAME = (1.0f / g_fFrameRate);
							char buf[1024]={0};
							sprintf(buf,"Inc framerate %.2f",g_fFrameRate);
							ShowGameMessage(buf, 32);
						}
					}
					
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
					if (Event.key.keysym.sym==SDLK_PAGEUP)
					{
						djSoundAdjustVolume(4);
						SetConsoleMessage( djStrPrintf( "Volume: %d%%", (int) ( 100.f * ( (float)djSoundGetVolume()/128.f ) ) ) );
					}
					else if (Event.key.keysym.sym==SDLK_PAGEDOWN)
					{
						djSoundAdjustVolume(-4);
						SetConsoleMessage( djStrPrintf( "Volume: %d%%", (int) ( 100.f * ( (float)djSoundGetVolume()/128.f ) ) ) );
					}
					else if (Event.key.keysym.sym==SDLK_INSERT)
					{
						if (djSoundEnabled())
							djSoundDisable();
						else
							djSoundEnable();
						SetConsoleMessage( djSoundEnabled() ? "Sounds ON (Ins)" : "Sounds OFF (Ins)" );
					}
					break;
				case SDL_KEYUP:
					for ( i=0; i<KEY_NUMKEYS; i++ )
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
				}
			}

			if (key_down_edge[KEY_ACTION]) key_action = 1;
			if (key_down_edge[KEY_LEFT])   key_left = 1;
			if (key_down_edge[KEY_RIGHT])  key_right = 1;
			if (key_down_edge[KEY_JUMP])   key_jump = 1;
			if (key_down_edge[KEY_SHOOT])  key_shoot = 1;
			//if (g_iKeys[DJKEY_UP])		key_action = 1;
			//if (g_iKeys[DJKEY_LEFT])	key_left = 1;
			//if (g_iKeys[DJKEY_RIGHT])	key_right = 1;
			
			// We allow ctrl as a sort of 'default' fallback jump if (and only if) it isn't assigned/redefined to anything
			if (!IsGameKeyAssigned(SDLK_RCTRL))
			{
				if (g_iKeys[DJKEY_CTRL])	key_jump = 1;
			}
			//if (g_iKeys[DJKEY_ALT])		key_shoot = 1;
			//[dj2016-10 don't think it really makes sense to have P as jump - if anything, pause??[LOW]](g_iKeys[DJKEY_P])		key_jump = 1;
			if (g_iKeys[DJKEY_ESC])		iEscape = 1;

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
					if (g_bBigViewportMode)
					{
						VIEW_WIDTH = (pVisView->width / 16) - 10;
						VIEW_HEIGHT = (pVisView->height - 5*16) / 16;
						if (VIEW_HEIGHT>=100)VIEW_HEIGHT=100;
						if (VIEW_WIDTH>=128)VIEW_WIDTH=128;
					}
					else
					{
						VIEW_WIDTH = VIEW_WIDTH_DEFAULT;
						VIEW_HEIGHT = 10;

						// NB, TODO, we actually need to also need to redraw score etc. here (though since this is just a dev/editing mode, not a real game mode, it doesn't have to be perfect)

						// When going out of 'big viewport' mode, hero might now be off the (now-tiny) 'viewport' :/ .. so must also 're-center' viewport around hero
						if (x>xo+VIEW_WIDTH/2) xo = x-VIEW_WIDTH/2;
						if (y>yo+VIEW_HEIGHT/2) yo = y-VIEW_HEIGHT/2;

						// Redraw everything that needs to be redrawn, as larger viewport will have obliterated right side with score etc.
						GameDrawSkin();
						GraphFlipView( VIEW_WIDTH, VIEW_HEIGHT );
						update_health( 0 );
						update_score( 0 );
						GameDrawFirepower();
						InvDraw();
						GraphFlip(!g_bBigViewportMode);//Flip
					}
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
SDL_Delay(100);//<-'wrong' workaround for, it adds 6 access cards [dj2017-06]
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

					// Full firepower
					HeroSetFirepower(MAX_FIREPOWER);
				}
			}
#endif

//this shouldn't be in 'default' game or something .. ?
			// Debug: hurt self
			static bool b = false;
			bool bOld = b;
			if (g_iKeys[DJKEY_H]) b = true; else b = false;
			if (b && !bOld)
				update_health(-1);

			// Debug: toggle display of debug info
			if (g_iKeys[DJKEY_D] && !g_iKeysLast[DJKEY_D]) bShowDebugInfo = !bShowDebugInfo;


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

			// Redraw everything that needs to be redrawn
			GameDrawSkin();
			GraphFlipView( VIEW_WIDTH, VIEW_HEIGHT );
			update_health( 0 );
			update_score( 0 );
			GameDrawFirepower();
			InvDraw();
			GraphFlip(!g_bBigViewportMode);
		}


		// Make a simple FPS display for debug purposes
		float fTimeRun;
		fTimeRun = fTimeNow - fTimeFirst;
		iFrameCount++;
		static char sbuf[1024]={0};
		sprintf( sbuf, "%.2f", (float)iFrameCount / fTimeRun );
		if (iFrameCount==60)
		{
			iFrameCount /= 2;
			fTimeFirst += (fTimeRun/2);
		}
		djgDrawImage( pVisBack, pSkinGame, 0, 8, 0, 8, 196, 8 );
		GraphDrawString( pVisBack, g_pFont8x8, 0, 8, (unsigned char*)sbuf );

		// update
		float fT1 = djTimeGetTime();
		GameHeartBeat();
		float fT2 = djTimeGetTime();
		afTimeTaken.push_back((fT2 - fT1)*1000.0f);
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
		x = MAX( MIN(x,126), 1 );
		y = MAX( MIN(y, 99), 2 );
		//debug//printf("}");

#ifdef DAVEGNUKEM_CHEATS_ENABLED
		if (key_action && g_iKeys[DJKEY_L])
		{
			// NB, if we are holding a key down when entering sprite/level editor, we 'miss' the gameloop's KeyUp event as the integrated editor takes over input polling; this leads to potentially "stuck" keystates in anKeyState when exiting, which in turn causes problems like e.g. if you press 'up' before entering the level editor then drop yourself out of the level editor over a teleporter, the game freezes as it keeps re-activating the teleporter since the action key is effectively behaving as if stuck down. TL;DR CLEAR THE KEYSTATES HERE (even if the keys really are down on exit editor). I am not mad about this solution, all feels a little wobbly/workaround-y, but should do for now. [dj2017-06-22]
			memset( anKeyState, 0, sizeof(anKeyState) );
			NextLevel();
			//return;
		}
		else
#endif
		// "integrated" sprite / level editors [dj2017-06-20 moving these to bottom of this loop, just in case we have any issues comparable to the pungee sticks crash bug, e.g interacting with dangling objects or something in the one single heartbeat update that occurs after exiting level editor]
		if (g_iKeys[DJKEY_F4])
		{
			SwitchMode ( SWITCH_SPRED );
			ED_Main ();

			// NB, if we are holding a key down when entering sprite/level editor, we 'miss' the gameloop's KeyUp event as the integrated editor takes over input polling; this leads to potentially "stuck" keystates in anKeyState when exiting, which in turn causes problems like e.g. if you press 'up' before entering the level editor then drop yourself out of the level editor over a teleporter, the game freezes as it keeps re-activating the teleporter since the action key is effectively behaving as if stuck down. TL;DR CLEAR THE KEYSTATES HERE (even if the keys really are down on exit editor). I am not mad about this solution, all feels a little wobbly/workaround-y, but should do for now. [dj2017-06-22]
			memset( anKeyState, 0, sizeof(anKeyState) );
			
			RestartLevel();
		}
		else if (g_iKeys[DJKEY_F5])
		{
			SwitchMode ( SWITCH_LVLED );
			ED_Main ();

			// NB, if we are holding a key down when entering sprite/level editor, we 'miss' the gameloop's KeyUp event as the integrated editor takes over input polling; this leads to potentially "stuck" keystates in anKeyState when exiting, which in turn causes problems like e.g. if you press 'up' before entering the level editor then drop yourself out of the level editor over a teleporter, the game freezes as it keeps re-activating the teleporter since the action key is effectively behaving as if stuck down. TL;DR CLEAR THE KEYSTATES HERE (even if the keys really are down on exit editor). I am not mad about this solution, all feels a little wobbly/workaround-y, but should do for now. [dj2017-06-22]
			memset( anKeyState, 0, sizeof(anKeyState) );

			RestartLevel(); // [dj2017-06-20] This replaces PerLevelSetup() call that was after LVLED_Kill(), not 100% sure but suspect this slightly more 'correct'
		}
		// NB, not all platforms will have an F1 key. The original DN1 said on the screen
		// 'press F1 for help' but we can't 'bake that in' to the images etc.
		/*else if (g_iKeys[DJKEY_F1])
		{
			ShowInstructions();
			g_iKeys[DJKEY_F1]=0;//????
			// NB, if we are holding a key down when entering sprite/level editor, we 'miss' the gameloop's KeyUp event as the integrated editor takes over input polling; this leads to potentially "stuck" keystates in anKeyState when exiting, which in turn causes problems like e.g. if you press 'up' before entering the level editor then drop yourself out of the level editor over a teleporter, the game freezes as it keeps re-activating the teleporter since the action key is effectively behaving as if stuck down. TL;DR CLEAR THE KEYSTATES HERE (even if the keys really are down on exit editor). I am not mad about this solution, all feels a little wobbly/workaround-y, but should do for now. [dj2017-06-22]
			//memset( anKeyState, 0, sizeof(anKeyState) );
		}*/

	} // while (game running)

	TRACE("game_startup(): main game loop exited.\n");

	PerGameCleanup();
	return g_nScore;
}

/*-----------------------------------------------------------*/
void GameHeartBeat()
{
	//debug//printf("HEARTBEAT[");
	CThing * pThing = NULL;
	int n=0, i=0, j=0;
	//int ifoo = key_action;

	// Update hero basic stuff
	HeroUpdate();

	//update animation counts:
	anim4_count++;
	if (anim4_count>3)
		anim4_count = 0;

	//nSlowDownHeroWalkAnimationCounter ^= 1;
	// Above line should have same effect and be faster, but is less understandable
	nSlowDownHeroWalkAnimationCounter++;
	if (nSlowDownHeroWalkAnimationCounter>1)
		nSlowDownHeroWalkAnimationCounter = 0;

	//not jumping but about to be, then dont left/right move
	if (!((key_jump) && (hero_mode != MODE_JUMPING))) {
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
	switch (hero_mode)
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
					g_nFalltime = 0;
				}
				++g_nFalltime;
			}
			else
				g_nFalltime = 0;
			if (bFallingPrev && !bFalling) // <- just stopped falling
			{
				// Kick up some dust ..
				AddThing(CreateDust(x, y, x_small*8,0));
				djSoundPlay( g_iSounds[SOUND_JUMP_LANDING] );
			}
			bFallingPrev = bFalling;
		}

		// standing still and pressing 'up': (just pressing up?)
		if (key_action)
		{
			key_action = 0; // huh?
			// dj2017-06-22 I think that "huh?" might have something to do with issue encountered of 'freezing on entering teleporter' issue, or else it's just redundant/old code, not sure, as it re-sets key_action anyway from anKeyStates[KEY_ACTION] each heartbeat

			// Go-to-exit cheat key
			if (g_iKeys[DJKEY_I])
			{
				for ( i=0; i<(int)g_apThings.size(); i++ )
				{
					if (g_apThings[i]->GetTypeID()==TYPE_EXIT)
					{
						relocate_hero(g_apThings[i]->m_x, g_apThings[i]->m_y);
						HeroFreeze(5);
						return;
					}
				}
			}

			// Check if you're on anything funny, like an exit
			for ( i=0; i<(int)g_apThings.size(); i++ )
			{
				CThing *pThing = g_apThings[i];
				if (!HeroIsFrozen() && pThing->HeroInsideActionBounds(x*16+x_small*8, y*16-16+y_offset))
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
		if (hero_mode == MODE_JUMPING)
		{
			if (y-yo<2) yo--;
			g_nRecentlyFallingOrJumping = 2;
		}
		else
		{
			bool bIsFalling = bFalling || bFallingPrev || hero_mode==MODE_JUMPING;
			if (bIsFalling)
			{
				g_nRecentlyFallingOrJumping = 2;
				if (y-yo>=9) yo++;
			}
			else
			{
				if (g_nRecentlyFallingOrJumping>0)
				{
					--g_nRecentlyFallingOrJumping;
				}
				else
				{
					if (y-yo<7)
					{
						yo--;
					}
				}
				if (y-yo>=7)
				{
					yo++;
				}
			}
		}
		if ( yo < 0 )
			yo = 0;
		if ( yo > LEVEL_HEIGHT - 10 )
			yo = LEVEL_HEIGHT - 10;
	}


	// Check for bullets that have gone out of the view
	for ( i=0; i<(int)g_apBullets.size(); i++ )
	{
		CBullet *pBullet = g_apBullets[i];
		int x1 = pBullet->dx<0 ? pBullet->x + 8 : pBullet->x;
		if (!OVERLAPS_VIEW(
			x1,
			pBullet->y,
			x1 + 7,
			pBullet->y+BULLET_HEIGHT-1))
		{
			djDEL(pBullet);
			g_apBullets.erase(g_apBullets.begin() + i);
			i--;
		}
	}

	// Check for bullet collisions with things (e.g. shootable boxes, monsters etc).
	// Also we check for monster bullet collisions against hero.
	for ( i=0; i<(int)g_apBullets.size(); ++i )
	{
		CBullet *pBullet = g_apBullets[i];
		if (pBullet->eType==CBullet::BULLET_HERO)
		{
			for ( j=0; j<(int)g_apThings.size(); ++j )
			{
				CThing *pThing = g_apThings[j];
				if (pThing->IsShootable())
				{
					int x1 = pBullet->dx<0 ? pBullet->x + 8 : pBullet->x;
					if (pThing->OverlapsShootArea(
						x1,
						pBullet->y,
						x1 + 7,
						pBullet->y+BULLET_HEIGHT-1))
						/*
						if (pThing->OverlapsBounds(
						pBullet->x,
						pBullet->y,
						pBullet->x+BULLET_WIDTH-1,
						pBullet->y+BULLET_HEIGHT-1))
						*/
					{
						int nRet = pThing->OnHeroShot();
						if (nRet==THING_DIE)
						{
							delete pThing;
							g_apThings.erase(g_apThings.begin() + j);
							j--;
						}
						else if (nRet==THING_REMOVE)
						{
							g_apThings.erase(g_apThings.begin() + j);
							j--;
						}

						// delete bullet i
						delete g_apBullets[i];
						g_apBullets.erase(g_apBullets.begin() + i);
						i--;
						goto NextBullet1;
					}
				}
			} // j
		}
		else if (pBullet->eType==CBullet::BULLET_MONSTER)
		{
			if (OVERLAPS(
				x*16+x_small*8,
				y*16-16,
				(x*16+x_small*8) + 15,
				(y*16-16) + 31,
				pBullet->x,
				pBullet->y,
				pBullet->x+15,
				pBullet->y+15))
			{
				delete g_apBullets[i];
				g_apBullets.erase(g_apBullets.begin() + i);
				i--;
				if (!HeroIsHurting())
				{
					update_health(-1);
					HeroSetHurting();
				}
			}
		}

NextBullet1:
		;

	}

	// Create new bullets
	static int nNoShootCounter = 0;
	if (key_shoot)
	{
		if (nNoShootCounter==0 && CountHeroBullets()<g_nFirepower)
		{
			nNoShootCounter = 4; // can't shoot for 4 frames
			HeroShoot(
				x * 16 + (hero_dir==1 ? 16 : -16) + x_small*8,
				y * 16 - 2,
				(hero_dir==0 ? -16 : 16)
				);
		}
	}
	if (nNoShootCounter != 0)
		nNoShootCounter--;


	// Update bullets
	for ( i=0; i<(int)g_apBullets.size(); i++ )
	{
		CBullet *pBullet = g_apBullets[i];
		int nXOld = pBullet->x;
		int nYOld = pBullet->y;
		// Update bullet
		pBullet->Tick();

		// Check if bullet touching anything solid
		int x1 = pBullet->dx<0 ? pBullet->x + 8 : pBullet->x;
		if (CheckCollision(
			x1,
			pBullet->y,
			x1 + 7,
			pBullet->y+BULLET_HEIGHT-1, pBullet))
		{
			AddThing(CreateExplosion((nXOld + (pBullet->dx<0 ? 0 : 0)), nYOld-4));
			g_apBullets.erase(g_apBullets.begin() + i);
			i--;
			goto NextBullet3;
		}


NextBullet3:
		;

	}


	// Interact with "things"
	// Check if you're on anything funny, like an exit
	for ( i=0; i<(int)g_apThings.size(); ++i )
	{
		CThing *pThing = g_apThings[i];
		if (pThing->OverlapsBounds(x*16+x_small*8, y*16+y_offset-16))
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
			if (pThing->HeroInsideActionBounds(x*16+x_small*8, y*16+y_offset-16))
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


	// Drop all objects that can fall
	for ( i=0; i<(int)g_apThings.size(); ++i )
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
					if (pThing->m_y >= 100)
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


	// "Tick" (update) all objects
	for ( i=0; i<(int)g_apThings.size(); ++i )
	{
		pThing = g_apThings[i];
		// FIXME: THING_REMOVE?
		if (pThing->Tick()==THING_DIE)
		{
			// Delete this
			delete pThing;
			g_apThings.erase(g_apThings.begin() + i);
			i--;
		}
	}



	if (g_bDied)
	{
		// Reset to beginning of current level
		RestartLevel();
		g_bDied = false;
	}



	// Redraw the screen according to the current game state
	GameDrawView();

	if ( nHurtCounter > 0 )
		nHurtCounter--;

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
	if (bReset || nHurtCounter==0)
		nHurtCounter = 16;
}

bool HeroIsHurting()
{
	return nHurtCounter!=0;
}

void SetHealth(int nHealth)
{
	int i;
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

	// Build a string representing health bars (which are in the 8x8 font)
	char szHealth[MAX_HEALTH+1]={0};
	for ( i=0; i<MAX_HEALTH; ++i )
	{
		// 170 = health; 169 = not health
		szHealth[MAX_HEALTH-1-i] = (i<g_nHealth?170:169);
	}
	szHealth[MAX_HEALTH] = 0;
	GraphDrawString( pVisBack, g_pFont8x8, HEALTH_X, HEALTH_Y, (unsigned char*)szHealth );
}

void update_health(int health_diff)
{
	// [dj2016-10-28] Keep health static if hero 'frozen' (e.g. going through teleporters or exits), otherwise
	// say a monster could kill you after you've already gone through an exit.
	if (HeroIsFrozen())
		return;
	SetHealth(g_nHealth + health_diff);
	// If busy jumping up and something hurts hero, stop the jump
	if (health_diff<0 && hero_mode==MODE_JUMPING)
	{
		HeroCancelJump();
	}
}

void SetScore(int nScore)
{
	g_nScore = nScore;

	char score_buf[32]={0};
	sprintf( score_buf, "%10d", (int)g_nScore );
	// Clear behind the score with part of the game skin
	if (pSkinGame)
		djgDrawImage( pVisBack, pSkinGame, SCORE_X, SCORE_Y, SCORE_X, SCORE_Y, 10*8, 8 );
	// Display score
	GraphDrawString( pVisBack, g_pFont8x8, SCORE_X, SCORE_Y, (unsigned char*)score_buf );
}

void update_score(int score_diff, int nFloatingScoreXBlockUnits, int nFloatingScoreYBlockUnits)
{
	SetScore(g_nScore + score_diff);

	// If requested, create a "floating score" display
	if (nFloatingScoreXBlockUnits!=-1 && nFloatingScoreYBlockUnits!=-1)
		AddThing(CreateFloatingScore(nFloatingScoreXBlockUnits, nFloatingScoreYBlockUnits, score_diff));
}
/*-----------------------------------------------------------*/
void DrawThingsAtLayer(EdjLayer eLayer)
{
	CThing *pThing;
	unsigned int i;
	for ( i=0; i<g_apThings.size(); i++ )
	{
		pThing = g_apThings[i];
		if (pThing->Layer()==eLayer && pThing->IsInView())
			pThing->Draw();
	}
}

void GameDrawView()
{
	int i=0,j=0,a=0,b=0,xoff=0,yoff=0;
	int anim_offset = 0;
	unsigned char *tempptr=NULL;

	// Draw view background
	if (pBackground)
		djgDrawImage(pVisView, pBackground, 0, 0, 16, 16, VIEW_WIDTH*16, VIEW_HEIGHT*16);

	// Clear viewport background before starting to draw game view in there
	// (If we don't, then the background doesn't clear where there are 'bg' (background) sprites)
	// (We don't want to draw the actual sprite as it has a little 'BG' on it to help with level editing)
	if (g_bBigViewportMode || pBackground==NULL)//<- The (only) reason we don't 'need' to do this if not in 'big viewport mode' is because of the pBackground image draw right above, effectively clears the viewport 'anyway' already for that section where there is background image
	{
		SDL_Rect rect;
		rect.x = 16;
		rect.y = 16;
		rect.w = VIEW_WIDTH*16;
		rect.h = VIEW_HEIGHT*16;
		SDL_FillRect(pVisView->pSurface, &rect, SDL_MapRGB(pVisView->pSurface->format, 0, 0, 0));
		//djgClear(pVisView);
	}

	//(10 seconds got to just after coke can, purple lab)
	tempptr = (unsigned char *)(g_pLevel) + yo*512+(xo<<2);
	yoff = 200+16;
	//  c=2;
	//const unsigned int uLevelPixelW = 128*16;
	//const unsigned int uLevelPixelH = 100*16;


	for ( i=0; i<VIEW_HEIGHT; ++i )
	{
		//  d=24;
		xoff = -xo_small+2;
		for ( j=0; j<VIEW_WIDTH+xo_small; ++j )
		{
			// Bounds-checks to not 'buffer overflow' etc. by going past bottom (or right) of level [dj2016-10]
			if (yo+i>=LEVEL_HEIGHT || xo+j>=LEVEL_WIDTH)
			{
				// do nothing .. leave black
			}
			else
			{
				// BLOCK[2,3] -> background block
				a = *(tempptr+2);
				b = *(tempptr+3);
				// Animated block?
				anim_offset = (GET_EXTRA( a, b, 4 ) & FLAG_ANIMATED) ? anim4_count : 0;

				// draw background block
				//djgDrawImage( pVisView, g_pCurMission->GetSpriteData(a)->m_pImage, ((b+anim_offset)%16)*16, ((b+anim_offset)/16)*16, xoff*8,16+i*16,16,16 );
				if ((a | b) != 0)//<- This if prevents background clearing of 'bg' background block .. etiher we need to clear entire viewport before start drawing map, or must draw a black square here 'manually' .. not sure which is more efficient ultimately
				{
					DRAW_SPRITE16A(pVisView, a, b+anim_offset, xoff*8, 16+i*16);
				}

				// BLOCK[0,1] -> foreground block
				a = *(tempptr);
				b = *(tempptr+1);
				// Animated block?
				anim_offset = (GET_EXTRA( a, b, 4 ) & FLAG_ANIMATED) ? anim4_count : 0;

				// draw foreground block, unless its (0,0)
				if ((a | b) != 0)
					DRAW_SPRITE16A(pVisView, a, b+anim_offset, xoff*8, 16+i*16);
			}
			xoff+=2;
			tempptr += 4;
		}
		yoff += 16;
		tempptr += (512 - ((VIEW_WIDTH+xo_small)<<2));
	}

	// Draw pre-hero layers, then draw hero, then draw post-hero layers.
	DrawThingsAtLayer(LAYER_BOTTOM);
	DrawThingsAtLayer(LAYER_2);
	DrawThingsAtLayer(LAYER_MIDDLE);
	// draw hero, but flash if he is currently hurt
	if ((nHurtCounter == 0) || (nHurtCounter%3 != 0))
	{
		xoff = (x_small - xo_small)+1 + ((x-xo)<<1);
		yoff = 200+16+(y-yo-1)*16;

		xoff = ((x_small - xo_small)+1)*8 + (x-xo) * 16;
		yoff = 16 + (y-yo-1) * 16;

		xoff = (x_small - xo_small)+1 + ((x-xo)<<1);
		xoff *= 8;
		/*
		if (hero_dir>0)
		{
			//tuxtest [dj2017-07 want to make hero sprite simpler to work on ultimately]
			DRAW_SPRITE16A(pVisView,4,   96+hero_picoffs*2  ,xoff   ,yoff   +y_offset);
			DRAW_SPRITE16A(pVisView,4,   96+hero_picoffs*2+1,xoff+16,yoff   +y_offset);
			DRAW_SPRITE16A(pVisView,4,16+96+hero_picoffs*2  ,xoff   ,yoff+16+y_offset);
			DRAW_SPRITE16A(pVisView,4,16+96+hero_picoffs*2+1,xoff+16,yoff+16+y_offset);
		}
		else
		//*/
		{
			DRAW_SPRITE16A(pVisView,4,  hero_dir*16+hero_picoffs*4,xoff   ,yoff   +y_offset);
			DRAW_SPRITE16A(pVisView,4,2+hero_dir*16+hero_picoffs*4,xoff   ,yoff+16+y_offset);
			DRAW_SPRITE16A(pVisView,4,1+hero_dir*16+hero_picoffs*4,xoff+16,yoff   +y_offset);
			DRAW_SPRITE16A(pVisView,4,3+hero_dir*16+hero_picoffs*4,xoff+16,yoff+16+y_offset);
		}
		if (bShowDebugInfo)
		{
			// Light blue box shows hero collision bounding box
			djgSetColorFore(pVisView,djColor(5,50,200));
			djgDrawRectangle(pVisView,
				xoff+8,
				yoff+y_offset,
				16,
				32);
		}
	}
	DrawThingsAtLayer(LAYER_4);
	DrawThingsAtLayer(LAYER_TOP);

	// Draw bullets
	DrawBullets();

#ifdef DAVEGNUKEM_CHEATS_ENABLED
	// God mode status display
	if (g_bGodMode) GraphDrawString(pVisView, g_pFont8x8, 32, 16, (unsigned char*)"GODMODE");
#endif

	// Draw debug info
	if (bShowDebugInfo) DrawDebugInfo();

	// Flip the off-screen world viewport onto the backbuffer
	GraphFlipView( VIEW_WIDTH, VIEW_HEIGHT );
}

// [dj2016-10] [fixme don't like these globals just floating here]
// Level Editor: New feature: Hold in Ctrl+Alt and click with the mouse to automatically start level with hero 'dropped in' to the clicked position as starting position (to help with level editing / testing)
int g_nOverrideStartX=-1;
int g_nOverrideStartY=-1;
void parse_level(void)
{
	int i, j;
	// parse the level (for doors, keys, hero starting position etc.)
	for ( i=0; i<100; ++i )
	{
		for ( j=0; j<128; ++j )
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

	plevel = g_pLevel + 4 * (iy * 128 + ix) + (ifore * 2);

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

			hero_dir = 1;
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
	// Draw the game skin
	if (pSkinGame)
		djgDrawImage( pVisBack, pSkinGame, 0, 0, 320, 200 );
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

void NextLevel()
{
	// Mark non-persistent inventory items as persistent
	InvMakeAllPersistent();
	// Go to next level
	SetLevel(GetCurrentLevel()+1);
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
		if (pThing->OverlapsBounds(x*16+x_small*8, y*16+y_offset-16))
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
	char buf[128]={0};
	sprintf(buf, "%d things", (int)g_apThings.size());
	GraphDrawString(pVisView, g_pFont8x8, 32, 24, (unsigned char*)buf );
	sprintf(buf, "%d visible", nNumVisible);
	GraphDrawString(pVisView, g_pFont8x8, 32, 32, (unsigned char*)buf );
	sprintf(buf, "[%d,%d] [%d firepower]", x, y, g_nFirepower);
	GraphDrawString(pVisView, g_pFont8x8, 32, 40, (unsigned char*)buf );

	if (HeroIsFrozen())
	{
		sprintf(buf, "[FROZEN]");
		GraphDrawString(pVisView, g_pFont8x8, 32, 48, (unsigned char*)buf );
	}

	extern int nFrozenCount;
	sprintf(buf, "frozecount=%d", nFrozenCount);
	GraphDrawString(pVisView, g_pFont8x8, 32, 56, (unsigned char*)buf );
	//sprintf(buf, "[%d,%d,%d,%d]", x,y,x_small,y_offset);
	//GraphDrawString(pVisView, g_pFont8x8, 32, 56+8, (unsigned char*)buf );
	//sprintf(buf, "hero_mode=%d", hero_mode);
	//GraphDrawString(pVisView, g_pFont8x8, 32, 62, (unsigned char*)buf );
}

void DrawBullets()
{
	unsigned int i;
	for ( i=0; i<g_apBullets.size(); ++i )
	{
		CBullet *pBullet = g_apBullets[i];
		if (OVERLAPS_VIEW(pBullet->x, pBullet->y, pBullet->x+BULLET_WIDTH, pBullet->y+BULLET_HEIGHT))
		{
			pBullet->Draw();
		}
	}
}

bool check_solid( int ix, int iy, bool bCheckThings )
{
	int i;

	// Create an invisible "border" around the level. Handy catch-all for things going out of bounds.
	if ( ix<1 || iy<1 || ix>126 || iy>98 ) return true;

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
				if (OVERLAPS(
					ix*16, iy*16, ix*16+15, iy*16+15,
					pThing->m_x*16 + pThing->m_iSolidX1,
					pThing->m_y*16 + pThing->m_iSolidY1,
					pThing->m_x*16 + pThing->m_iSolidX2,
					pThing->m_y*16 + pThing->m_iSolidY2))
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
	CThing *pThing;
	int nX1 = x1 / 16;
	int nY1 = y1 / 16;
	int nX2 = x2 / 16;
	int nY2 = y2 / 16;
	for ( i=nX1; i<=nX2; i++ )
	{
		for ( j=nY1; j<=nY2; j++ )
		{
			if (check_solid(i, j, false))
			{
				if (OVERLAPS(x1, y1, x2, y2, i*16, j*16, i*16+15, j*16+15))
					return true;
			}
		}
	}
	for ( i=0; i<(int)g_apThings.size(); i++ )
	{
		pThing = g_apThings[i];
		if (pThing->m_bSolid)
		{
			if (OVERLAPS(
				x1, y1, x2, y2,
				pThing->m_x*16 + pThing->m_iShootX1,
				pThing->m_y*16 + pThing->m_iShootY1,
				pThing->m_x*16 + pThing->m_iShootX2,
				pThing->m_y*16 + pThing->m_iShootY2))
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
	int i;
	// First clear firepower display area with game skin
	djgDrawImage( pVisBack, pSkinGame, FIREPOWER_X, FIREPOWER_Y, FIREPOWER_X, FIREPOWER_Y, 16*5, 16 );
	// Draw firepower
	for ( i=0; i<g_nFirepower; i++ )
	{
		DRAW_SPRITE16A( pVisBack, 5, 0, FIREPOWER_X + i*16, FIREPOWER_Y );
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
	FILE *pOut = fopen("savegame.gnukem", "w");
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
	FILE *pIn = fopen("savegame.gnukem", "r");
	if (pIn==NULL)
		return false;
	int nLevel=0, nScore=0, nFirepower=0, nHealth=0;

	//dj2016-10-28 Add load/save of current 'mission' (otherwise if we save game as say level 4 of 3rd mission then do save game from mission 1 it loads incorrectly to level 4 of mission 1). Note though, want to try keep this file format though 'backward-compatible' i.e. want older versions to still load
	// Note here if loading a file saved with a build prior to 28 October 2016, then at this point we're already 'at the end' of the file, in which case we can't know the correct 'mission', however, we can assume it's probably most likely to have been the default game, so just use that as default.
	std::string sMissionFilename = "default.gam";

	// Check for 'fileversion=', which was only added 2016-10-28 .. if not present, then the first line is the level number.
	char szFirstLine[1024]={0};
	fscanf(pIn,"%s\n",szFirstLine);
	if (strncmp(szFirstLine,"fileversion=",12)==0)
	{
		// Fileversion 2
		char szFilename[4096]={0};//gross
		fscanf(pIn,"mission=%s\n",szFilename);//[LOW] Wonder if we might have cross-platform issues here? E.g. savegame on Windows with CR+LF & try load on e.g. a LF-only platform say?
		if (szFilename[0]!=0)
		{
			sMissionFilename = szFilename;
		}

		fscanf(pIn, "%d\n", &nLevel);
	}
	else
	{
		// Fileversion 1
		nLevel = atoi(szFirstLine);
	}
	djMSG("LOADGAME: Mission=%s\n", sMissionFilename.c_str());
	// Find the mission from the list of missions
	CMission* pMission = g_apMissions[0];//<-Default to the first one
	for ( std::vector<CMission * >::const_iterator iter=g_apMissions.begin(); iter!=g_apMissions.end(); ++iter )
	{
		if (sMissionFilename==(*iter)->GetFilename())
		{
			pMission = *iter;
			break;
		}
	}
	if (pMission != g_pCurMission)
	{
		PerGameCleanup();

		g_pCurMission = pMission;

		PerGameSetup();
	}

	//fscanf(pIn, "%d\n", &nLevel);
	fscanf(pIn, "%d\n", &nScore);
	fscanf(pIn, "%d\n", &nFirepower);
	fscanf(pIn, "%d\n", &nHealth);
	djMSG("LOADGAME: Level=%d Score=%d Firepower=%d Health=%d\n", nLevel, nScore, nFirepower, nHealth);
	SetLevel(nLevel);
	SetScore(nScore);
	SetHealth(nHealth);
	g_nFirepower = nFirepower;
	// Load inventory. We do this after level setup stuff, otherwise it gets cleared in per-level setup..
	InvLoad(pIn);

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
	int nMenuOption = do_menu( &gameMenu );
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
	case 5:
		g_bGameRunning = false;
		break;
	}
}
