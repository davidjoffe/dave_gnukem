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
	#include <map>
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
djSprite* LoadSpriteHelper(const char* szPath, int nW, int nH)
{
	if (szPath==nullptr||szPath[0]==0)
		return nullptr;
	printf("LoadSpriteHelper\n");
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


	// MAIN MENU LOCALIZATION ...
	std::map<std::string, std::map<std::string, std::string>> map;

	// NB NB NB!!! THE BELOW ARE AI-DONE TRANSLATINOS TO HELP WITH DEV AND TESTING
	// NOT human translations yet and likely have mistakes:

	// Afrikaans translations
	/*
	map["af"]["Start gnu game"] = "Begin gnu-spel";
	map["af"]["Restore game"] = "Herstel spel";
	map["af"]["Select Mission"] = "Kies Missie";
	map["af"]["Ordering info"] = "Bestelinligting";
	map["af"]["(not!)"] = "(nie!)";
	map["af"]["Instructions"] = "Instruksies";
	map["af"]["Redefine keys"] = "Herkies sleutels";
	map["af"]["High scores"] = "Hoë tellings";
	map["af"]["Credits"] = "Krediete";
	map["af"]["About"] = "Oor";
	map["af"]["Retro Settings"] = "Retro-instellings";
	map["af"]["Don't quit"] = "Moenie ophou nie";
	map["af"]["Quit"] = "Uitgaan";
	*/

	// French translations
	//map["fr"]["Start gnu game"] = "*Démarrer";
	/*map["fr"]["Start gnu game"] = "Démarrer le jeu gnu";
	map["fr"]["Restore game"] = "Restaurer le jeu";
	map["fr"]["Select Mission"] = "Sélectionner la mission";
	map["fr"]["Ordering info"] = "Information de commande";
	map["fr"]["(not!)"] = "(pas vrai!)";
	map["fr"]["Instructions"] = "Instructions";
	map["fr"]["Redefine keys"] = "Redéfinir les touches";
	map["fr"]["High scores"] = "Scores élevés";
	*/
	//map["fr"]["Credits"] = "Crédits";
	//map["fr"]["About"] = "À propos";
	//map["fr"]["Retro Settings"] = "Paramètres rétro";
	//map["fr"]["Don't quit"] = "Ne quittez pas";
	//map["fr"]["Quit"] = "Quitter";

	// German translations
	/*
	map["de"]["Start gnu game"] = "Neues Spiel starten";
	map["de"]["Restore game"] = "Spiel wiederherstellen";
	map["de"]["Select Mission"] = "Mission auswählen";
	map["de"]["Ordering info"] = "Bestellinformationen";
	map["de"]["(not!)"] = "(nicht!)";
	map["de"]["Instructions"] = "Anleitung";
	map["de"]["Redefine keys"] = "Tasten neu belegen";
	map["de"]["High scores"] = "Bestenliste";
	map["de"]["Credits"] = "Credits";
	map["de"]["About"] = "Über";
	map["de"]["Retro Settings"] = "Retro-Einstellungen";
	map["de"]["Don't quit"] = "Nicht beenden";
	map["de"]["Quit"] = "Beenden";

	// Spanish translations
	map["es"]["Start gnu game"] = "Iniciar juego gnu";
	map["es"]["Restore game"] = "Restaurar juego";
	map["es"]["Select Mission"] = "Seleccionar Misión";
	map["es"]["Ordering info"] = "Información de pedido";
	map["es"]["(not!)"] = "(¡no!)";
	map["es"]["Instructions"] = "Instrucciones";
	map["es"]["Redefine keys"] = "Redefinir teclas";
	map["es"]["High scores"] = "Puntuaciones altas";
	map["es"]["Credits"] = "Créditos";
	map["es"]["About"] = "Acerca de";
	map["es"]["Retro Settings"] = "Configuraciones Retro";
	map["es"]["Don't quit"] = "No salir";
	map["es"]["Quit"] = "Salir";
	*/

	std::string sLang = djGetLanguage();
	if (!sLang.empty() && sLang!="en")
	{
		// Translate the menu items
		size_t uCount = 0;
		const SMenuItem *pItem = &mainMenuItems[0];
		while (pItem->m_szText!=nullptr)
		{
			//g_pMainMenuItems[pItem-mainMenuItems] = *pItem;

			pItem++;
			++uCount;
		}
		SMenuItem* pMenu = new SMenuItem[uCount+1];//+1?
		unsigned int uIndex = 0;
		pItem = &mainMenuItems[0];
		while (pItem->m_szText!=nullptr)
		{
			std::string sItem = pItem->m_szText;
			// First copy it
			pMenu[uIndex] = mainMenuItems[uIndex];

			pMenu[uIndex].SetText("");

			std::string sNew = mainMenuItems[uIndex].m_szText;
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
			pMenu[uIndex].SetText(nullptr);
			// Store new copies on the heap of the translated strings
			{
				char *sz = new char[sNew.size()+1];
				strcpy(sz, sNew.c_str());
				pMenu[uIndex].SetText(sz);
			}

			pItem++;
			++uIndex;
		}
		// Do the old-fashioned nullptr-terminator thing ..
		pMenu[uIndex].m_szText = nullptr;

		// Fixme leaks
		mainMenu.setItems ( pMenu );
	}



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

		// Language code selected for localization
		const std::string sLang = djGetLanguage();
		if (!sLang.empty() && sLang!="en")
			GraphDrawString(pVisBack, g_pFont2->GetImage(), CFG_APPLICATION_RENDER_RES_W - sLang.length()*8, CFG_APPLICATION_RENDER_RES_H - 16, (unsigned char*)sLang.c_str());

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
