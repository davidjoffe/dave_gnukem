//Copyright (C) 1995-2023 David Joffe
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
#include "djlang.h"//djGetLanguage()
#include "localization/djgettext.h"//dj2023 for localizations e.g. French Dave Gnukem
#include "djsprite.h"
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
	//For background music stuff
	#include "sdl/djinclude_sdlmixer.h"
#endif//#ifndef NOSOUND

/*--------------------------------------------------------------------------*/
#define DATAFILE_MAINMENUBACKGROUND "main.tga"
//#define DATAFILE_MAINMENUBACKGROUND "tlex.png"
//! Background image *behind* main menu (e.g. grim cityscape for Dave Gnukem)
djImage *g_pImgMain = NULL;

/*--------------------------------------------------------------------------*/
// dj2022-12 these shoiuld probably not extern/forwards but refactored into also serpate cpps
extern void SelectMission();				// Select a mission
extern void RedefineKeys();				// Redefine keys
/*--------------------------------------------------------------------------*/
// Main menu [NB, warning, the handling code uses indexes :/ .. so if you add/remove items, must update there too - dj2016-10]
// TODO: The old const struct globals may represent a problem with the pgettext call because we need to have already initialized the .po loading etc. by the time this gets created
// Todo refactor all the menus for localization to be created dynamically now rather at runtime like this so that pgettext can do its thing.
const struct SMenuItem* CreateMenuItems_MainMenu()
{
	struct SMenuItem mainMenuItems[] =
	{
	{ false, "                   " },
	// TRANSLATORS: This is a pun on "Start new game". "Gnu" is an open-source reference. 
	// If a similar pun works in your language, feel free to use it. Otherwise, 
	// translate as "Start new game".
	{ true,  pgettext("mainmenu/game/start", "Start gnu game") },
	//mainmenu.addmenuitem(pgettext("mainmenu", "Start gnu game"));
	{ true,  pgettext("mainmenu/game/restore", "Restore game") },
	{ true,  pgettext("mainmenu", "Select Mission") },
	{ true,  pgettext("mainmenu/orderinfo-info", "Ordering info") },
	// TRANSLATORS: This is a negation of 'ordering info' (a joke menu item since it's open source) but also another reference to 90's 'not!', while the joke 'Ordering info' menu item is there in the first place as a parody reference to the Shareware games like the original Duke Nukem we're parodying here, which had 'ordering info'
	{ true,  pgettext("mainmenu/not", "(not!)"), "", 10 },//10 pixels x offset indentation
	{ true,  pgettext("mainmenu/instructions", "Instructions") },
	{ true,  pgettext("mainmenu/redefine-keys", "Redefine keys") },
	{ true,  pgettext("mainmenu/highscores", "High scores") },
	{ true,  pgettext("mainmenu/credits", "Credits") },
	{ true,  pgettext("mainmenu/about", "About") },
	{ true,  pgettext("mainmenu/settings-retro", "Retro Settings") },
	// TRANSLATORS: Note this is/was meant to mean "Don't quit the game" i.e. it's a humor joke menu item that does nothing
	{ true,  pgettext("mainmenu/dontquit", "Don't quit") },
	{ true,  pgettext("mainmenu/quit", "Quit") },
	{ false, "                   " },
	{ false, "" }//Terminal (if you don't have this last empty one bad things will happen)
	};

	//{ true,  pgettext("mainmenu", "Select language") }, //?todo add 'select language'?
	//{ true,  pgettext("mainmenu", "Settings") },//todo make general Settings?

	// Urgh, get count due to old-fashioned null-terminator stuff ...
	size_t uCount = 0;
	const SMenuItem *pItem = &mainMenuItems[0];
	while (!pItem->IsTerminal())
	{
		pItem++;
		++uCount;
	}
	struct SMenuItem* pMenuItemsRet = new SMenuItem[uCount+1];
	// Copy and return (um this this is gross there are constant string char* pointers in there to above, will it even work?)
	// Probably .. the mainMenuItems may be on the stack, but the constant strings 'should' exist for the lifetime of the program
	// Still, this is icky, we should probably rather use std::string for menu stuff
	for ( size_t i=0; i<uCount; ++i )
	{
		pMenuItemsRet[i] = mainMenuItems[i];
	}
	pMenuItemsRet[uCount].SetTerminal();//last one
	return pMenuItemsRet;
}

// [dj2023-11] For localization purposes I need to more genericize text rendering and font stuff, which means I need to refactor the skull-cursor stuff after over 20 years of it being done like this to have these menu cursors be in their own separate new sprite images (not be in, and re-use, the main old game font.tga) so that we can toggle to e.g. e.g. pixel operator as UI font if loading French interface etc.
// These are/were ugly hardcoded offsets into main.tga where these which now will become meaningless, and done more nicely/generically
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

//funny skull (slight parody homage to old Doom menu)
djSprite* g_pCursor2=nullptr;
djSprite* g_pShadow=nullptr;
djSprite* g_pBars=nullptr;
djSprite* g_pFont2=nullptr;
djSprite* g_pFontNumbers=nullptr;
djSprite* g_pCharBackground=nullptr;
djSprite* LoadSpriteHelper(const char* szPath, int nW, int nH)
{
	if (szPath==nullptr||szPath[0]==0)
		return nullptr;
	printf("LoadSprite:");
	djSprite* pSprite = new djSprite;
	if (pSprite->LoadSpriteImage(szPath, nW, nH))
	{
		djCreateImageHWSurface( pSprite->GetImage() );
		return pSprite;
	}
	else
	{
		delete pSprite;
	}
	return nullptr;
}
void InitMainMenu()
{
	std::string sLang = djGetLanguage();
	
	//djDEL(g_pDefaultMenuCursor);
	if (g_pDefaultMenuCursor==nullptr)
	{
		g_pDefaultMenuCursor = new djMenuCursorSprite();
		g_pDefaultMenuCursor->m_pSprite = LoadSpriteHelper(djDATAPATHc("menucursor/cursor1-8x8.png"), 8, 8);
	}
	//djDEL(g_pFont2);
		g_pFont2 = LoadSpriteHelper(djDATAPATHc("fonts/pixeloperator/PixelOperator8-raster.png"), 8, 8);
		g_pFontNumbers = LoadSpriteHelper(djDATAPATHc("fonts/numbers.png"), 8, 8);
	//funny skull (slight parody homage to old Doom menu)
	g_pCursor2 = LoadSpriteHelper(djDATAPATHc("menucursor/cursor2-8x8.png"), 8, 8);
	g_pShadow = LoadSpriteHelper(djDATAPATHc("ui/dropshadow.png"), 8, 8);
	g_pBars = LoadSpriteHelper(djDATAPATHc("ui/bars.png"), 8, 8);
	g_pCharBackground = LoadSpriteHelper(djDATAPATHc("menucharbackground.tga"), 8, 8);

	const struct SMenuItem* mainMenuItems = CreateMenuItems_MainMenu();

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



/*
	std::string sLang = djGetLanguage();
	if (!sLang.empty() && sLang!="en")
	{
		// Translate the menu items
		size_t uCount = 0;
		const SMenuItem *pItem = &mainMenuItems[0];
		while (!pItem->IsTerminal())
		{
			pItem++;
			++uCount;
		}
		SMenuItem* pMenu = new SMenuItem[uCount+1];//+1?
		unsigned int uIndex = 0;
		pItem = &mainMenuItems[0];
		while (!pItem->IsTerminal())
		{
			std::string sItem = pItem->GetTextStr();
			// First copy it
			pMenu[uIndex] = mainMenuItems[uIndex];

			//pMenu[uIndex].SetText("");

			std::string sNew = mainMenuItems[uIndex].GetTextStr();
			if (sNew.empty())
			{
				++uIndex;
				continue;
			}

			std::string sOrigL;
			//std::string sOrigR;
			// But now we need our own copy of the string (with translation perhaps)
			if (!sItem.empty())
			{
				while (sItem[0]==' ') { sOrigL += ' '; sItem = sItem.substr(1); }//left-trim spaces, although it's gross we still need them for now
				while (sItem.back()==' ') { sItem = sItem.substr(0, sItem.size()-1); } //right-trim spaces, that was always gross
				if (!sItem.empty())
				{
					if (map[sLang].find(sItem)!=map[sLang].end())
						sNew = sOrigL + map[sLang][sItem];// + sOrigR;
				}
			}
			// Store new copies on the heap of the translated strings
			pMenu[uIndex].SetText(sNew.c_str());

			pItem++;
			++uIndex;
		}
		// Do the old-fashioned nullptr-terminator thing ..
		pMenu[uIndex].SetTerminal();

		// Fixme leaks
		mainMenu.setItems ( pMenu );
	}
	*/

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
		GraphDrawString(pVisBack, djDefaultFont(), 0, CFG_APPLICATION_RENDER_RES_H - 8, (unsigned char*)VERSION);
		const char* szURL = "djoffe.com";
		GraphDrawString(pVisBack, djDefaultFont(), CFG_APPLICATION_RENDER_RES_W - strlen(szURL)*8, CFG_APPLICATION_RENDER_RES_H - 8, (unsigned char*)szURL);

		// Language code selected for localization
		const std::string sLang = djGetLanguage();
		//if (djLang::DoTranslations())//!sLang.empty() && sLang!="en")
		GraphDrawString(pVisBack, djDefaultFont(), CFG_APPLICATION_RENDER_RES_W - sLang.length()*8, CFG_APPLICATION_RENDER_RES_H - 16, (unsigned char*)sLang.c_str());

		GraphFlip(true);

		// Random select menu cursor, either hearts or skulls
		mainMenu.setMenuCursor ( (rand()%4==0 ? mainMenuCursorSkull : mainMenuCursor) );

		if (((rand()%4)==0) && g_pCursor2!=nullptr && g_pCursor2->IsLoaded())
			//funny skull (slight parody homage to old Doom menu)
			mainMenu.SetMenuCursor(g_pCursor2);
		else
			mainMenu.SetMenuCursor(nullptr);//Use default menu cursor

		// Old hardcoded cursor (to deprecate font.tga stuff with hardcoded offsets)
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

		mainMenu.SetMenuCursor(nullptr);
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
