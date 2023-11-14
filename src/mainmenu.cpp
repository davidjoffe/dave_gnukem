//Copyright (C) 1995-2022 David Joffe
//
// dj2022-12 Just refactoring old main menu code out of main.cpp (and in prep toward some localization stuff)

/*--------------------------------------------------------------------------*/
#include "config.h"
#include "version.h"//dj2022-11 for new VERSION
#include "mainmenu.h"
#include "djimage.h"
#include "djgraph.h"
#include "djsound.h"
#include "djstring.h"//djAppendPathStr

#include "game.h"//game_startup etc.
#include "menu.h"
#include "hiscores.h"
#include "credits.h"
#include "datadir.h"
#include "graph.h"
#include "instructions.h"//ShowInstructions()

// For Mix_Music but not sure I'm mad about that but it's not the most serious thing to worry about .. dj2022
#ifndef NOSOUND
//dj2022-12 hm just a thought but isn't a difference like that resolvable by passing the paths differently in build system for OS2? no idea how OS/2 version is built tho. low prio.
#ifdef __OS2__
	#include <SDL/SDL_mixer.h>
#else
	#include <SDL_mixer.h>//For background music stuff
#endif
#endif

/*--------------------------------------------------------------------------*/
#define DATAFILE_MAINMENUBACKGROUND "main.tga"
//! Background image *behind* main menu (e.g. grim cityscape for Dave Gnukem)
djImage *g_pImgMain = NULL;

/*--------------------------------------------------------------------------*/
// dj2022-12 these shoiuld probably not extern/forwards but refactored into also serpate cpps
extern void SelectMission();				// Select a mission
extern void RedefineKeys();				// Redefine keys
/*--------------------------------------------------------------------------*/
// Main menu [NB, warning, the handling code uses indexes :/ .. so if you add/remove items, must update there too - dj2016-10]
const struct SMenuItem mainMenuItems[] =
{
	{ false, "                   " },
	{ true,  "   Start gnu game  " },
	{ true,  "   Restore game    " },
	{ true,  "   Select Mission  " },
	{ true,  "   Ordering info   " },
	{ true,  "    (not!)         " },
	{ true,  "   Instructions    " },
	{ true,  "   Redefine keys   " },
	{ true,  "   High scores     " },
	{ true,  "   Credits         " },
	{ true,  "   About           " },
	{ true,  "   Retro Settings  " },
	{ true,  "   Don't quit      " },
	{ true,  "   Quit            " },
	{ false, "                   " },
	{ false, NULL }
};

const unsigned char mainMenuCursor[] = { 128, 129, 130, 131, 0 };
const unsigned char mainMenuCursorSkull[] = { 161, 162, 163, 164, 0 };
CMenu mainMenu ( "main.cpp:mainMenu" );
/*--------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------*/
// 'controller' ?

//fixmerefaxtor:
extern bool djGetTextInput(std::string& sReturnString, int nMaxLen, unsigned int uPixelW, const char* szLabel);

bool GetHighScoreUserName(std::string& sReturnString)
{
	#define MAX_HIGHSCORE_LEN 256
	#define WIDTH_INPUTBOX 34

	//fixmelocalize:
	std::string s = "New high score!";
	s += (char)10;//linefeed
	s += (char)10;//linefeed
	s += "Enter your name:";

	return djGetTextInput(sReturnString, MAX_HIGHSCORE_LEN, WIDTH_INPUTBOX*8, s.c_str());
}

// check if high score table is beaten,
// let user enter his name
// and show the table after all
// dj2022 not quite sure where this belongs .. some overall game 'controller'? or in hiscores.h/cpp? I think hiscores.h/cpp should probably just be the core 'model' of high score not the stuff like this that 'knows about' e.g. user interface stuff like getting text input (thinking of model/view/controller type of design)
void CheckHighScores( int score )
{
	if (IsNewHighScore(score))
	{
		std::string sUserName;
		if (GetHighScoreUserName(sUserName))
		{
			AddHighScore(sUserName.c_str(), score);

			extern std::string djGetFolderUserSettings();//<-todo make new better header locatino for this function .. [low prio]

			std::string s = djAppendPathStr(djGetFolderUserSettings().c_str(), USERFILE_HIGHSCORES);
			SaveHighScores(s.c_str()); // Save high scores immediately, in case Windows crashes
		}

		ShowHighScores();
	}
}
/*--------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------*/
void InitMainMenu()
{
	mainMenu.setClrBack ( djColor(70,70,80)/*djColor(42,57,112)*/ ); //mainMenu.setClrBack ( djColor(10,40,150) ); // Crap colour. Need something better, like a bitmap
	//mainMenu.m_clrBack = djColor(129,60,129);
	mainMenu.setSize ( 0 );
	mainMenu.setItems ( mainMenuItems );
	mainMenu.setMenuCursor ( mainMenuCursor );
	mainMenu.setXOffset (-1);
	mainMenu.setYOffset (-1);
	//dj2018-04-01 make the Y position sightly higher by 4 pixel than the default, looks slightly better with new city background
	mainMenu.setYOffset( 8 * (12 - (13 / 2)) - 4 );//13 = num items

	mainMenu.setSoundMove(djSoundLoad(djDATAPATHc("sounds/cardflip.wav")));


	// Main menu background image
	g_pImgMain = new djImage();
	if (g_pImgMain->Load(djDATAPATHc(DATAFILE_MAINMENUBACKGROUND)) < 0)
	{
		printf("Error: Image load failed: %s\n", DATAFILE_MAINMENUBACKGROUND);
	}
	djCreateImageHWSurface( g_pImgMain );
}

// hmm [dj2022-11] it's maybe sligtly debatable whether main menu 'calls' game or main returns control to some 'game state controller' which launches game -
// I'm inclined to think the latter is more correct .. tho for simple game overkill but keep moving design in more 'correct' direction..
void KillMainMenu()
{
	// TODO
    if (g_pImgMain)
    {
	    djDestroyImageHWSurface(g_pImgMain);
	    djDEL(g_pImgMain);		// Delete main menu background image (title screen)
    	//djLOGSTR( "djDEL(g_pImgMain) ok\n" );
    }
}
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
void DoMainMenu()
{
	bool bRunning = true;

#ifndef NOSOUND
	//dj2016-10 adding background music to main menu, though have not put any real thought into what would
	// be the best track here so fixme todo maybe dig a bit more and find better choice here etc. [also for levels]
	Mix_Music* pMusic = Mix_LoadMUS(djDATAPATHc("music/eric_matyas/8-Bit-Mayhem.ogg"));
	if (pMusic!=NULL)
		Mix_FadeInMusic(pMusic, -1, 800);
	else
	{
		//'debugassert' / trap / exception type of thing?
		printf("Warning: Failed to load main menu music\n");
	}
#endif

	do
	{
		// Clear back buffer [dj2019-06 .. adding this just to make as if we drop out of bigviewportmode and the menu skin doesn't cover full size, then there may be junk drawn at right or bottom]
		djgSetColorFore(pVisBack, djColor(0, 0, 0));
		djgDrawBox(pVisBack, 0, 0, pVisBack->width, pVisBack->height);

		// Load main menu background image
		if (g_pImgMain)
		{
			// Simple 1 to 1 blit .. later it might be worthwhile doing a stretch blit if size doesn't match resolution? [LOW - dj2019]
			djgDrawImage( pVisBack, g_pImgMain, 0, 0, g_pImgMain->Width(), g_pImgMain->Height() );
		}
		GraphDrawString(pVisBack, g_pFont8x8, 0, CFG_APPLICATION_RENDER_RES_H - 8, (unsigned char*)VERSION);
		const char* szURL = "djoffe.com";
		GraphDrawString(pVisBack, g_pFont8x8, CFG_APPLICATION_RENDER_RES_W - strlen(szURL)*8, CFG_APPLICATION_RENDER_RES_H - 8, (unsigned char*)szURL);

		GraphFlip(true);

		// Random select menu cursor, either hearts or skulls
		mainMenu.setMenuCursor ( (rand()%4==0 ? mainMenuCursorSkull : mainMenuCursor) );

		int menu_option = do_menu( &mainMenu );

		switch (menu_option)
		{
		case 1:		/* rtfb's vision of this branch :)*/
		{
			//int score = PlayGame ();
			int score = game_startup();
			CheckHighScores( score );
#ifndef NOSOUND
			// Game levels start their own music, so when come out of game and back to main menu, restart main menu music
			if (pMusic!=NULL)
				Mix_FadeInMusic(pMusic, -1, 800);
#endif
			break;
		}
		case 2: // restore game [dj2016-10 adding implementation for this - it did nothing before]
			{
				int score = game_startup(true);
				CheckHighScores( score );
#ifndef NOSOUND
				// Game levels start their own music, so when come out of game and back to main menu, restart main menu music
				if (pMusic!=NULL)
					Mix_FadeInMusic(pMusic, -1, 800);
#endif
			}
			break;
		case 3: // select mission
			SelectMission();
			break;
		case 6: // instructions
			ShowInstructions();
			break;
		case 7:
			RedefineKeys();
			break;
		case 8:
			ShowHighScores();
			break;
		case 9: // credits
			ShowCredits();
			break;
		case 10: // about
			ShowAbout();
			break;
		case 11://dj2019-06 just-for-fun extra-retro simulated faux-EGA/CGA
    		extern void SettingsMenu();
			SettingsMenu();
			break;
		case 12://Don't quit
			break;
		case -1: // escape
		case 13: // quit
			bRunning = false;
			break;
		}
	} while (bRunning);

#ifndef NOSOUND
	if (pMusic)
	{
		Mix_FreeMusic(pMusic);
		pMusic = NULL;
	}
#endif
}
/*--------------------------------------------------------------------------*/
