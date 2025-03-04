//Copyright (C) 1995-2025 David Joffe
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

#include <map>//dj2023-11 menu stuff
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
	// Menu internal command IDs
	// We must start with the vector to preserve order for display in the menu to human
	// THESE MAY CHANGE because some of these also appear elsewhere on the in-game menu
	std::vector<std::string> asItemCommands = {
		//"<text>:Heading",
		"mainmenu/start_new_game",
		"mainmenu/restore_game",
		"mainmenu/select_mission",
		"mainmenu/order_info",
		"mainmenu/not",
		"mainmenu/instructions",
		"mainmenu/redefine_keys",
		"mainmenu/high_scores",
		"mainmenu/credits",
		"mainmenu/about",
		//"mainmenu/language",
		//"mainmenu/crash",
		//"mainmenu/settings",
		"mainmenu/settings_retro",
		"mainmenu/dont_quit",
		"mainmenu/quit"
	};

	//{ "<text>:Heading", "A heading" },

	// Menu display strings
	std::map<std::string, std::string> map = {

		// TRANSLATORS: This is a pun on "Start new game". "Gnu" is an open-source reference. 
		// If a similar pun works in your language, feel free to use it. Otherwise, 
		// preferably translate as "Start new game", which has the advantage of being potentially more re-usable for other games
		{ "mainmenu/start_new_game",	pgettext("mainmenu/start_new_game", "Start gnu game") },
		{ "mainmenu/restore_game",		pgettext("mainmenu/restore_game", "Restore game") },
		{ "mainmenu/select_mission",	pgettext("mainmenu/select_mission", "Select Mission") },
		// Humor menu item (but might not be if this is re-use in future for orderable games)
		{ "mainmenu/order_info",		pgettext("mainmenu/order_info", "Ordering info") },
		// Humor menu item. TRANSLATORS: This is a negation of 'ordering info' (a joke menu item since it's open source) but also since it's a retro game, it's another retro 90s reference to 90's 'not!', while the joke 'Ordering info' menu item is there in the first place as a parody reference to the Shareware games like the original Duke Nukem we're parodying here, which had 'ordering info'
		{ "mainmenu/not",				pgettext("mainmenu/not", "(not!)") },
		{ "mainmenu/instructions",		pgettext("mainmenu/instructions", "Instructions") },
		{ "mainmenu/redefine_keys",		pgettext("mainmenu/redefine_keys", "Redefine keys") },
		{ "mainmenu/high_scores",		pgettext("mainmenu/high_scores", "High scores") },
		{ "mainmenu/credits",			pgettext("mainmenu/credits", "Credits") },
		{ "mainmenu/about",				pgettext("mainmenu/about", "About") },
		{ "mainmenu/language",			pgettext("mainmenu/language", "Choose language") },
		{ "mainmenu/settings",			pgettext("mainmenu/settings", "Settings") },
		{ "mainmenu/settings_retro",	pgettext("mainmenu/settings_retro", "Retro Settings") },
		// Humor menu item. TRANSLATORS: Note this is/was meant to mean "Don't quit the game" (or "Don't exit the game") (not "Don't give up") i.e. it's a humor joke menu item that does nothing
		{ "mainmenu/dont_quit",			pgettext("mainmenu/dont_quit", "Don't quit") },
		// Quit/exit the game
		{ "mainmenu/quit",				pgettext("mainmenu/quit", "Quit") }
		};
	std::map<std::string, int> mapX = {
		{ "mainmenu/not", 8 }
	};

	// todo should we link the callbacks here or closer to when we call?

	// +3 for leading spacing string, following spacing string and 'IsTerminal' terminator to indicate last one (ideally all those things should be refactored)
	const size_t uNewSize = asItemCommands.size() + 3;

	std::vector<SMenuItem> aItems;
	aItems.reserve(uNewSize);
	aItems.push_back(SMenuItem( false, "                   " ));

	// 'paMenuItems' means 'pointer to array of menu items'
	for ( unsigned int i=0; i<asItemCommands.size(); ++i )
	{
		const std::string sItemCommand = asItemCommands[i];
		std::string sItemText;// = asItemCommands[i];
		if (map.find(asItemCommands[i]) != map.end())
			sItemText = map[asItemCommands[i]];
		else
			sItemText = asItemCommands[i];
		
		// If it's only spaces use 'false' or overtly a text-only type eg retro settings submenu heading
		bool bIsSelectable = true;
		if (sItemCommand.substr(0,6)=="<text>")
			bIsSelectable = false;

		//paMenuItems[i+1] = SMenuItem( bIsSelectable, sItemText, sItemCommand );
		aItems.push_back(SMenuItem( bIsSelectable, sItemText, sItemCommand ));

		if (mapX.find(sItemCommand)!=mapX.end()){
			//paMenuItems[i+1].m_Pos.x = mapX[sItemCommand];
			aItems.back().m_Pos.x = mapX[sItemCommand];
		}
	}

	aItems.push_back(SMenuItem( false, "                   " ));
	aItems.push_back(SMenuItem( false, "" ));//Terminal (if you don't have this last empty one bad things will happen)

	// Copy vector from std::vector to 'pointer to array'
	SMenuItem* paItems = new SMenuItem[uNewSize];
	for ( size_t i=0; i<uNewSize; ++i )
	{
		paItems[i] = aItems[i];
	}
	return paItems;
}

// [dj2023-11] For localization purposes I need to more genericize text rendering and font stuff, which means I need to refactor the skull-cursor stuff after over 20 years of it being done like this to have these menu cursors be in their own separate new sprite images (not be in, and re-use, the main old game font.tga) so that we can toggle to e.g. e.g. pixel operator as UI font if loading French interface etc.
// These are/were ugly hardcoded offsets into main.tga where these which now will become meaningless, and done more nicely/generically
const unsigned char mainMenuCursor[] = { 128, 129, 130, 131, 0 };
//const unsigned char mainMenuCursorSkull[] = { 161, 162, 163, 164, 0 };
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

	mainMenu.setClrBack ( djColor(70,70,80) );
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
class djMenu
{
public:
	bool bRunning = true;
#ifndef NOSOUND
	Mix_Music* pMusic = nullptr; 
#endif
};
djMenu g_Menu;
void DoMainMenu()
{
	g_Menu.bRunning = true;

#ifndef NOSOUND
	//dj2016-10 adding background music to main menu, though have not put any real thought into what would
	// be the best track here so fixme todo maybe dig a bit more and find better choice here etc. [also for levels]
	g_Menu.pMusic = Mix_LoadMUS(djDATAPATHc("music/eric_matyas/8-Bit-Mayhem.ogg"));
	if (g_Menu.pMusic!=NULL)
		Mix_FadeInMusicPos(g_Menu.pMusic, -1, 800, 0);
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
		std::string sLang = djGetLanguage();
		if (djLang::GetCurLangDirection() < 0)
		{
			sLang+="(RTL)";// If we're actually in RTL this might confusingly display as 'LTR' if our text renderer just blindly reverses it
		}
		//if (djLang::DoTranslations())//!sLang.empty() && sLang!="en")
		GraphDrawString(pVisBack, djDefaultFont(), CFG_APPLICATION_RENDER_RES_W - sLang.length()*8, CFG_APPLICATION_RENDER_RES_H - 16, (unsigned char*)sLang.c_str());
		//Right to Left eg Hebrew?
		/*
		int nDirection = djLang::GetCurLangDirection();
		if (nDirection<0)
		{
			std::string sDirection = "RTL";
			GraphDrawString(pVisBack, djDefaultFont(), CFG_APPLICATION_RENDER_RES_W - sDirection.length()*8, CFG_APPLICATION_RENDER_RES_H - 24, (unsigned char*)sDirection.c_str());
		}
		//*/

		GraphFlip(true);

		// Random select menu cursor, either hearts or skulls
		//mainMenu.setMenuCursor ( (rand()%4==0 ? mainMenuCursorSkull : mainMenuCursor) );
		if (((rand()%4)==0) && g_pCursor2!=nullptr && g_pCursor2->IsLoaded())
			//funny skull (slight parody homage to old Doom menu)
			mainMenu.SetMenuCursor(g_pCursor2);
		else
			mainMenu.SetMenuCursor(nullptr);//Use default menu cursor


		/*
		int menu_option = do_menu( &mainMenu );
		*/
		typedef void (*djCALLBACK)();
		// ACTUALLY 'DO MENU'. Note we want to refactor here from returning an int to returning and using a command ID (and/or callback)
		std::map<std::string, djCALLBACK> mapCallbacks =
		{
		{ "mainmenu/start_new_game", []() {
			//int score = PlayGame ();
			int score = game_startup();
			CheckHighScores( score );
#ifndef NOSOUND
			// Game levels start their own music, so when come out of game and back to main menu, restart main menu music
			if (g_Menu.pMusic!=NULL)
				Mix_FadeInMusic(g_Menu.pMusic, -1, 800);
#endif
			}
		},
		{ "mainmenu/restore_game",		[]() // restore game [dj2016-10 adding implementation for this - it did nothing before]
				{
				int score = game_startup(true);
				CheckHighScores( score );
#ifndef NOSOUND
				// Game levels start their own music, so when come out of game and back to main menu, restart main menu music
				if (g_Menu.pMusic!=NULL)
					Mix_FadeInMusicPos(g_Menu.pMusic, -1, 800, 0);
#endif
			}
		},
		{ "mainmenu/select_mission", []() { // select mission
			SelectMission();
		}},
		{ "mainmenu/instructions", []() {
			ShowInstructions();
		}},
		{ "mainmenu/redefine_keys", []() {
			RedefineKeys();
		}},
		{ "mainmenu/high_scores", []() {
			ShowHighScores();
		}},
		{ "mainmenu/credits", []() {
			ShowCredits();
		}},
		{ "mainmenu/about", []() {
			ShowAbout();
		}},
		{ "mainmenu/settings_retro", []() {//dj2019-06 just-for-fun extra-retro simulated faux-EGA/CGA
    		extern void SettingsMenu();
			SettingsMenu();
		}},
		{ "mainmenu/dont_quit", []() {//Don't quit
			// Do nothing
		}},
		//}},{ "mainmenu/",		[]() {//		case -1: // escape
		{ "mainmenu/quit",		[]() {// quit
			g_Menu.bRunning = false;
		}
		}
		};//mapCallbacks

		
		int menu_option = do_menu( &mainMenu );
		if (menu_option<0)//Escape
			g_Menu.bRunning = false;

		std::string sCommand;
		if (menu_option >= 0 && !mainMenu.getItems()[menu_option].GetRetVal().empty())
			sCommand = mainMenu.getItems()[menu_option].GetRetVal();
		if (!sCommand.empty())
		{
			if (mapCallbacks.find(sCommand)!=mapCallbacks.end())
			{
				// If menu callback defined, call it
				mapCallbacks[sCommand]();
			}
		}


		mainMenu.SetMenuCursor(nullptr);
	} while (g_Menu.bRunning);

#ifndef NOSOUND
	if (g_Menu.pMusic)
	{
		Mix_FreeMusic(g_Menu.pMusic);
		g_Menu.pMusic = NULL;
	}
#endif
}
/*--------------------------------------------------------------------------*/
