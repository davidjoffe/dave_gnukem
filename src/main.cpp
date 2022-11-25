/*--------------------------------------------------------------------------*/
/* David Joffe '95/07/20 */
// Pre-1995 : something similar from QBasic or Pascal, I can't remember ...
// 1995 : original DOS/EGA version
// 1998/12/23 : begin attempted Linux port
// 1999/05 : begin attempted Win32 port, ack
// 1999/12 : re-begin attempted Win32 port
// 2001/05 : begin SDL port; doxygen comments
// 2016/10 : new github + livecoding 'era'
// 2018/04 : 'DG version 1' released
// ~2022 : Matteo Bini migration from SDL1 to SDL2

/*
Copyright (C) 1995-2022 David Joffe
*/

/*--------------------------------------------------------------------------*/

#include <time.h>   // for srand()

#include "graph.h"

#include "djinput.h"
#include "djtime.h"
#include "djstring.h"

#include "hiscores.h"
#include "menu.h"
#include "game.h"
#include "level.h"
#include "keys.h"
#include "settings.h"

#include "mission.h"

#include "credits.h"
#include "instructions.h"
#include "sys_log.h"
#include "sys_error.h"
#include "datadir.h"//DATA_DIR [might move later - dj2018-05]

#if defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__linux__) || defined(__unix__)
#include <unistd.h>//getcwd, chdir
#endif
#ifdef __APPLE__
#include <mach-o/dyld.h>//_NSGetExecutablePath
#include <sys/stat.h>//For djFolderExists stuff
#endif

// dj2022-11 Merging Andreas Peters OS2 commits https://github.com/davidjoffe/dave_gnukem/pull/128
#ifdef __OS2__
#define INCL_DOS
#include <os2.h>
#endif

#ifndef NOSOUND
#ifdef __OS2__
#include <SDL/SDL_mixer.h>
#else
#include <SDL_mixer.h>//For background music stuff
#endif
#endif

#include <map>
#include <string>

/*--------------------------------------------------------------------------*/
djImage *g_pImgMain = NULL;

int  DaveStartup(bool bFullScreen, bool b640, const std::map< std::string, std::string >& Parameters);	// Application initialization
void DaveCleanup();					// Application cleanup
void SelectMission();				// Select a mission
void RedefineKeys();				// Redefine keys

void DoMainMenu();					// Start main menu
void CheckHighScores( int score );	// check if high score table is beaten,
						// let user enter his name
						// and show the table after all
void InitMainMenu();
void KillMainMenu();

#ifdef __OS2__
void MorphToPM()
{
   PPIB pib;
   PTIB tib;

   DosGetInfoBlocks(&tib, &pib);

   // Change flag from VIO to PM:
   if (pib->pib_ultype==2) pib->pib_ultype = 3;
}
#endif

/*--------------------------------------------------------------------------*/
// Main menu [NB, warning, the handling code uses indexes :/ .. so if you add/remove items, must update there too - dj2016-10]
struct SMenuItem mainMenuItems[] =
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

unsigned char mainMenuCursor[] = { 128, 129, 130, 131, 0 };
unsigned char mainMenuCursorSkull[] = { 161, 162, 163, 164, 0 };
CMenu mainMenu ( "main.cpp:mainMenu" );



/*--------------------------------------------------------------------------*/
// This is the 'main' function. The big cheese.
int main ( int argc, char** argv )
{


#ifdef __OS2__
	MorphToPM();
#endif
	
	// Check commandline args
	bool bfullscreen = false;
	bool b640 = false;

	// parametername => value [dj2017-06-22]
	std::map< std::string, std::string > Parameters;

	if (argc > 1)
	{
		std::string sNextParamGetValue;
		for ( int i=1; i<argc; i++ )
		{
			if (!sNextParamGetValue.empty())
			{
				Parameters[sNextParamGetValue] = argv[i];
				sNextParamGetValue = "";
			}
			else
			{
				if (0 == strncmp( argv[i], "-f", 2 )) bfullscreen = true;
				if (0 == strncmp( argv[i], "-640", 4 )) b640 = true;
				if (0 == strncmp( argv[i], "-scale", 6 )) sNextParamGetValue = "scale";
			}
		}
	}
	else
	{
		// Show usage information
		printf( "---------------------------------------------------------\n" );
		printf( " Command-line options:\n" );
		printf( "   -f    Fullscreen mode\n" );
		printf( "   -640  640x480 mode\n" );
		printf( "   -scale N [Optional] Force window size multiple of base resolution (1=320x200)\n" );
		printf( "---------------------------------------------------------\n" );
	}

	// Initialize everything
	if (0 != DaveStartup(bfullscreen, b640, Parameters))
	{
		SYS_Error ("DaveStartup() failed.");
		return -1;
	}

	// Enter main menu
	DoMainMenu();

	// Cleanup
	DaveCleanup();

	SYS_Debug ( "\n" );

	return 0;
}

int DaveStartup(bool bFullScreen, bool b640, const std::map< std::string, std::string >& Parameters)
{
	// Note the order of things in here can be very important, eg InitLog saves stuff under userdatafolder, so that must be created first

	// Create user data folder (if doesn't exist) [dj2018-03]
	djEnsureFolderTreeExists(djGetFolderUserSettings().c_str());

	djEnsureFolderTreeExists(
		djAppendPathStr(djGetFolderUserSettings().c_str(), "logs").c_str()
	);
	InitLog ();

#ifdef WIN32
	//dj2022-11 Refactoring just moving this klunky looking Win32-specific DPI scaling fix from 2018 from main.cpp to its own file for neatness
	extern void djSetProcessDPIAwareHelper();
	djSetProcessDPIAwareHelper();
#endif//#ifdef WIN32

#ifdef __APPLE__
	// Basically what we want to do here is:
	// If the 'cwd' does NOT have a data folder under it, but
	// the 'executable path' does, then we want to *change* the
	// current 'working directory' to be the same as the 'executable
	// path' - I think. [dj2016-10 - trying to fix running straight
	// out of DMG doesn't work - though ideally users should install
	// or copy anyway ..]
	char cwd[8192]={0};//current working directory (not to be confused with the path the executable is in, though often they're the same, depending)
	if(getcwd(cwd,sizeof(cwd)))
	{
		//debug//printf("Current working directory:%s\n",cwd);
		// Check if data folder is present relative to cwd
		char szDataFile[8192]={0};
		strcpy(szDataFile,cwd);
		djAppendPath(szDataFile, DATA_DIR "missions.txt");//Some semi-'arb' Dave Gnukem data file someone is unlikely to have in say their user home folder or whatever
		//debug//printf("Checking for:%s\n",szDataFile);fflush(NULL);
		if (!djFileExists(szDataFile))
		{
			printf("Warning: Data folder not found, trying executable path fallback ...\n");fflush(NULL);
			char path[4096]={0};
			uint32_t size = sizeof(path);
			if (_NSGetExecutablePath(path, &size) == 0)
			{
				printf("Executable path is %s\n", path);fflush(NULL);

				// This path is e.g. /Blah/Foo/davegnukem
				// the last part is the executable, we want just the
				// folder, so find the last '/' and set the char
				// after it to 0 to NULL-terminate.
				char *szLast = strrchr(path,'/');
				if (szLast)
					*(szLast+1) = 0; // NULL-terminate

				strcpy(szDataFile,path);
				djAppendPath(szDataFile, DATA_DIR "missions.txt");
				if (djFileExists(szDataFile))
				{
					printf("Successfully found the data path :)\n");fflush(NULL);
					// Yay, we found the data folder by the executable -
					// change the 'working directory' to 'path'
					chdir(path);
				}
			}
//else
    //printf("buffer too small; need size %u\n", size);
		}
	}
#endif

	djLOGSTR( "\n================[ Starting Application Init ]===============\n" );

	// Check the data folder is present, and if not, try give the user some basic guidance as to how to address this. [dj2018-05]
	// This is pretty 'critical' in that we can't recover from it, but not really critical in that the cause is
	// likely simply that the data subfolder is either missing, or in a different path.
	// Better to offer the user a little guidance rather than just exiting with no clue at all, e.g. [dj2018-05] cf. https://github.com/davidjoffe/dave_gnukem/issues/114
	if (!djFolderExists( DATA_DIR ))
	{
		// dj2019-06 This whole business should be improved on Linux
		printf("Unable to find data folder '%s'. Please note this is in a separate repo - see the ReadMe.md for details.\n",DATA_DIR);
		printf("If you have the data folder, then you can generally fix this message by first changing your current\n");
		printf("directory to the folder in which the 'data' folder is contained, then running the application.\n");
		// (dj2022-11 Add the below line, hmm, not sure whether it really belongs in the help text here to mention things like git repo cloning (and also maybe the URL may change later) but for now
		// I think it's better to have 'more possibly helpful info for users' that may help them get up and running and maybe refine this later - dj2022-11)
		// We could also consider doing 'fancy' things like just exec'ing a git clone if the user wants or something .. and/or add some small little helper scripts to do things like below. Or even auto-downloading data. Anyway. Low priority for now.
		// Also to consider is doing it generically so this code could support more games (and/or a hypothetical 'DG version 2')
		// dj2022-11 One additional thought on the below is that the below may fetch a 'bleeding edge' version with unstable stuff in it in future - hmm - maybe this needs more thought. LOW prio though.
		//printf("You can also get it by running: git clone https://github.com/davidjoffe/gnukem_data.git %s\n", DATA_DIR);
		return -1;
	}

	g_Settings.Load(
		djAppendPathStr(djGetFolderUserSettings().c_str(), CONFIG_FILE).c_str()
		);	// Load saved settings
	// We need to first check the setting is *actually there*, not just call e.g. FindSettingInt(), otherwise
	// if volume setting has never been set/saved before, it will return a value of 0 which will set the volume to 0.
	// We need to distinguish between 'never been set', and 'actually set to 0'. [dj2016-10]
	if (g_Settings.FindSetting("Volume")!=NULL)
	{
		djSoundSetVolume( g_Settings.FindSettingInt("Volume"), false );//NB Don't "apply" the volume setting because sound library only init'd slightly further down
	}
	const char* szValue = g_Settings.FindSetting("SoundsOn");
	if (szValue!=NULL && std::string(szValue)=="OFF")//on by default unless specifically turned off
	{
		djSoundDisable();
		SetConsoleMessage("Sounds OFF (Ins)");
	}

	srand((unsigned int)time(NULL));				// Seed the random number generator

	djTimeInit();					// Initialise timer

	InitialiseGameKeySystem();		// Initialise game keys

	djFontInit();					// Initialize main font [dj2022-11]

	djSDLInit();					// Initialize SDL2 [dj2022-11 refactoring a bit to have possible live in-game fullscreen toggle support]

	//-- Initialize graphics
	//
	// NOTE: Use 640x480 if you want to use the built-in editor (F4/F5)
	// [dj2017-08] The above comment should in most situations no longer be relevant,
	// and actually probably worse than what you'd now get with the default behavior.
	// By default it now basically tries to create a window that is the largest
	// window (that's a 320x200 ratio), so e.g. on a 1920x1080 screen you might
	// by default get e.g. window that's a scale factor of 5, e.g. 1600x1000,
	// which is MUCH nicer for level editing than 640x400! The 640 is I think partly
	// a legacy stemming from the days where 640x480 was a standard/common video
	// mode ... but I'm not sure if it might still be relevant/helpful on some
	// platforms (OpenPandora?
	// e.g. see discussion thread here which I don't fully follow
	// https://pyra-handheld.com/boards/threads/dave-gnukem.79533/
	// Though note, 640x480 vs 640x400?
	// )
	// I think in theory -640 should behave the same as if passing "-scale 2" now
	// but my memory of this stuff is a little vague so this needs to be checked.
	djLOGSTR("DaveStartup(): Initializing graphics system ...\n");
	int w=CFG_APPLICATION_RENDER_RES_W;
	int h=CFG_APPLICATION_RENDER_RES_H;
	//dj2019 do we really need the b640 option anymore? not sure. Might be used by some ports for DG1? Think I vaguely recall seeing someone mentioning using it on a forum ... maybe Pandora? Or maybe not used anymore? don't know.
	if (b640 == true)
	{
		w = 640;
		h = 400;
	}
	// dj2017-06-22 Add option to pass in forced scale multiplier. Hm, would it be better to make this a 'setting' rather than command line param? Dunno.
	int nForceScale = -1;
	std::map< std::string, std::string >::const_iterator iter=Parameters.find("scale");
	if (iter!=Parameters.end())
	{
		nForceScale = atoi( iter->second.c_str() );
	}
	// [dj2016-10] Note this w/h is effectively now a 'hint' as it may not initialize to the exact requested size
	if (!djGraphicsSystem::GraphInit( bFullScreen, w, h, nForceScale ))
	{
		djLOGSTR( "DaveStartup(): Graphics initialization failed.\n" );
		return -1;
	}

	djSoundInit();				// Initialize sound

	g_pImgMain = new djImage;			// Load main skin (title screen)
	g_pImgMain->Load(DATA_DIR "main.tga");
	djCreateImageHWSurface( g_pImgMain );

	InitMissionSystem();

	// Load missions
	if (0 != LoadMissions(DATA_DIR "missions.txt"))
		return -1;
	djLog::LogFormatStr( "DaveStartup(): %d missions(s) found.\n", (int)g_apMissions.size() );//NB must convert .size() to int due to risk of 64-bit vs 32-bit mismatch on some platforms! very subtle bug/risk

	//-- Initialize input devices
	djLOGSTR("DaveStartup(): Initializing keys ..\n");
	if (!djiInit())
		return -1;

	InitMainMenu();			// fill the structures and load some stuff

	GameInitialSetup();		// Once-off setup for the game itself (e.g. create in-game menu etc)

	djLOGSTR( "================[ Application Init Complete ]===============\n\n" );

	return 0;
}

void DaveCleanup()
{
	djLOGSTR( "\n================[ Starting Application Kill ]===============\n" );

	// Save user volume setting
	g_Settings.SetSettingInt("Volume",djSoundGetVolume());
	g_Settings.SetSetting("SoundsOn",djSoundEnabled()?"ON":"OFF");

	KillMainMenu();
	djLOGSTR( "KillMainMenu() ok\n" );
	KillMissionSystem();
	djLOGSTR( "KillMissionSystem() ok\n" );
	KillCredits();
	djLOGSTR( "KillCredits() ok\n" );
	SaveHighScores();		// Save high scores
	djLOGSTR( "SaveHighScores() ok\n" );
	GameFinalCleanup();		// Game
	djLOGSTR( "GameFinalCleanup() ok\n" );
	djiDone();			// Input
	djLOGSTR( "djiDone() ok\n" );
	djSoundDone();			// Sound
	djLOGSTR( "djSoundDone() ok\n" );
	djDestroyImageHWSurface(g_pImgMain);
	djDEL(g_pImgMain);		// Delete main menu background image (title screen)
	djLOGSTR( "djDEL(g_pImgMain) ok\n" );
	djGraphicsSystem::GraphDone();			// Graphics
	djLOGSTR( "GraphDone() ok\n" );
	djFontDone();			// Font helper [dj2022-11]
	djLOGSTR("djFontDone() ok\n");
	djTimeDone();			// Timer stuff
	djLOGSTR( "djTimeDone() ok\n" );

	g_Settings.Save(
		djAppendPathStr(djGetFolderUserSettings().c_str(), CONFIG_FILE).c_str()
	);	// Save settings
	djLOGSTR( "g_Settings.Save(CONFIG_FILE) ok\n" );

	djSDLDone();

	djLOGSTR( "================[ Application Kill Complete ]===============\n\n" );

//	KillLog ();
}

extern int g_nSimulatedGraphics;
//dj2019-06 just-for-fun extra-retro simulated faux-EGA/CGA
void SettingsMenu()
{
	struct SMenuItem SettingsMenuItems[] =
	{
		// this looks very weird; need to fix menu code to handle this better, or wrap setttingsmenu in additional UI stuff
		{ false, " " },//(DJ2019-06 low prio, at some stage want to fix up menu code so we can space / layout this thing properly - either that, or change how it draws the background)
		{ false, " Retro user experience settings  " },
		//{ false, "                                     " },
		{ true,  "   Mild (Default graphics)          ", "settings/retro/default" },
		{ true,  "   Medium (EGA) (simulated 16-color)", "settings/retro/ega" },
		{ true,  "   High (CGA) (simulated 4-color)   ", "settings/retro/cga" },
		//{ false, "                                     " },
		{ false, "                                     " },
		{ false, NULL }
	};

	unsigned char MenuCursor[] = { 128, 129, 130, 131, 0 };

	CMenu Menu("SettingsMenu");
	Menu.setClrBack( djColor(48,66,128) );
	Menu.setSize(0);
	Menu.setItems(SettingsMenuItems);
	Menu.setMenuCursor(MenuCursor);
	Menu.setXOffset(-1); // Calculate menu position for us
	Menu.setYOffset(-1);

	//dj2022-11 refactoring
	int nMenuOption = do_menu( &Menu );
	std::string sSelectedMenuCommand;
	if (nMenuOption >= 0 && Menu.getItems()[nMenuOption].m_szRetVal != nullptr)
		sSelectedMenuCommand = Menu.getItems()[nMenuOption].m_szRetVal;
	
	if (sSelectedMenuCommand == "settings/retro/default")
		g_nSimulatedGraphics = 0;
	else if (sSelectedMenuCommand == "settings/retro/ega")
		g_nSimulatedGraphics = 1;
	else if (sSelectedMenuCommand == "settings/retro/cga")
		g_nSimulatedGraphics = 2;
}

void DoMainMenu()
{
	bool bRunning = true;

#ifndef NOSOUND
	//dj2016-10 adding background music to main menu, though have not put any real thought into what would
	// be the best track here so fixme todo maybe dig a bit more and find better choice here etc. [also for levels]
	Mix_Music* pMusic = Mix_LoadMUS(DATA_DIR "music/eric_matyas/8-Bit-Mayhem.ogg");
	if (pMusic!=NULL)
		Mix_FadeInMusic(pMusic, -1, 800);
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
		// 'version string history' here:
		// "v1.0 - 3 Apr 2018" [version 1]
		// "v1.0.1 - 25 Apr 2020"
		// "v1.0.2 - 19 Nov 2022" [<- last version on SDL1 - about to update to SDL2]
		// "v1.0.3 - 19 Nov 2022" [New version number for SDL2 version with Matteo Bini SDL2 commit merged in]
		const char* szVERSION = "v1.0.3 - 19 Nov 2022";
		GraphDrawString(pVisBack, g_pFont8x8, 0, CFG_APPLICATION_RENDER_RES_H - 8, (unsigned char*)szVERSION);
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

void AppendCharacter(char *szBuffer, char c, int nMaxLen)
{
	int nStrLen = strlen(szBuffer);
	if (nStrLen<nMaxLen)
	{
		szBuffer[nStrLen] = c;
		szBuffer[nStrLen+1] = 0;
	}
}

void DialogBoxEffect(int x1, int y1, int w, int h, bool bInverted = false)
{
	//dj2016-10 updating this to have broader smoother bevelled edges, a darker default background color, and some 'noise' to make it look a bit better

	// Background
	int nCOLORMID = 150;
	//djgSetColorFore( pVisBack, djColor(nCOLORMID,nCOLORMID,nCOLORMID) );
	//djgDrawBox( pVisBack, x1, y1, w, h );
	srand(123);//<- Must stay same each frame as this is drawing every frame? or what [dj2016-10]
	#define clrRANDOFFSET ( (rand() % 8) - 4 )
	for ( int x=0;x<w;++x )
	{
		for ( int y=0;y<h;++y )
		{
			djgSetColorFore( pVisBack, djColor(nCOLORMID+clrRANDOFFSET,nCOLORMID+clrRANDOFFSET,nCOLORMID+clrRANDOFFSET) );
			djgDrawBox( pVisBack, x1+x, y1+y, 1, 1 );
		}
	}
	// dark bottom/right edge
	//djgSetColorFore( pVisBack, bInverted ? djColor(230,230,230) : djColor(80,80,80) );/
	//djgDrawBox( pVisBack, x1, y1+h-1, w, 1);
	//djgDrawBox( pVisBack, x1+w-1, y1, 1, h);
	int N=(bInverted ? 1 : 5);
	for ( int n=0;n<N;n++ )
	{
		int nColorStep = (nCOLORMID-80)/N;
		int nColorStepHi = (230-nCOLORMID)/N;
		djgSetColorFore( pVisBack, bInverted ? djColor(230+clrRANDOFFSET-n*nColorStepHi,230+clrRANDOFFSET-n*nColorStepHi,230+clrRANDOFFSET-n*nColorStepHi) : djColor(80+n*nColorStep+clrRANDOFFSET,80+n*nColorStep+clrRANDOFFSET,80+n*nColorStep+clrRANDOFFSET) );
		djgDrawBox( pVisBack, x1+n, y1+h-n, w-n*2, 1);//bottom
		djgDrawBox( pVisBack, x1+w-(n+1), y1+n+1, 1, h-n*2);//right

		// white top/left edge
		djgSetColorFore( pVisBack, bInverted ? djColor(80+n*nColorStep+clrRANDOFFSET,80+n*nColorStep+clrRANDOFFSET,80+n*nColorStep+clrRANDOFFSET) : djColor(230+clrRANDOFFSET-n*nColorStepHi,230+clrRANDOFFSET-n*nColorStepHi,230+clrRANDOFFSET-n*nColorStepHi) );
		djgDrawBox( pVisBack, x1+n, y1+n, w-n*2, 1);//top
		djgDrawBox( pVisBack, x1+n, y1+n, 1, h-n*2);//left
	}
	//djgDrawBox( pVisBack, x1+w-1, y1, 1, h);

	// white top/left edge
	//djgSetColorFore( pVisBack, bInverted ? djColor(80,80,80) : djColor(230,230,230) );
	//djgDrawBox( pVisBack, x1, y1, w, 1);
	//djgDrawBox( pVisBack, x1, y1, 1, h);
}

// Helper for RedefineKeys to prevent assigning same key to two actions [dj2016-10]
bool IsKeyUsed(int* anKeys, int key)
{
	for ( unsigned int i=0; i<KEY_NUM_MAIN_REDEFINABLE_KEYS; ++i )
	{
		if (anKeys[i] == key)
			return true;
	}
	return false;
}
void RedefineKeys()
{
	int anKeys[KEY_NUM_MAIN_REDEFINABLE_KEYS] = {0};
	bool bLoop = true;
	bool bFinished = false;
	int nCurrent = 0;
	do
	{
		int nDX = 152*2;
		//dj2019-06 Either everything should be centered [future?] or everything 320-based [DG1]
		//int nXLeft = (CFG_APPLICATION_RENDER_RES_W/2) - (nDX/2);
		int nXLeft = (320/2) - (nDX/2);

		// Black background
		djgSetColorFore( pVisBack, djColor(0,0,0) );
		djgDrawBox( pVisBack, 0, 0, CFG_APPLICATION_RENDER_RES_W, CFG_APPLICATION_RENDER_RES_H );
		// Stupid cheesy boring dialog-border effect
		DialogBoxEffect(nXLeft, 32, nDX, 128);

		djiPollBegin();
		//SDLMod ModState = SDL_GetModState();
		SDL_Event Event;
		while (djiPollEvents(Event))
		{
			switch (Event.type)
			{
			//case SDL_KEYDOWN:
				//break;
			case SDL_KEYDOWN://UP:
				switch (Event.key.keysym.sym)
				{
				case SDLK_ESCAPE:
					bLoop = false;
					break;
				default:
					if (bFinished)
					{
						if (Event.key.keysym.sym==SDLK_RETURN)
						{
							bLoop = false;
							// Commit changes
							int j;
							for ( j=0; j<KEY_NUM_MAIN_REDEFINABLE_KEYS; j++ )
							{
								g_anKeys[j] = anKeys[j];
							}
							// Save into the settings object so it gets saved to disk
							StoreGameKeys();
						}
					}
					else if (IsGameKey(Event.key.keysym.sym) && !IsKeyUsed(anKeys, Event.key.keysym.sym))
					{
						anKeys[nCurrent] = Event.key.keysym.sym;
						nCurrent++;
						if (nCurrent==KEY_NUM_MAIN_REDEFINABLE_KEYS)
						{
							bFinished = true;
						}
					}
					break;
				}
				break;
			case SDL_QUIT:
				bLoop = false;
				break;
			}
		}
		djiPollEnd();


		//DialogBoxEffect(nXLeft-4, 100, nDX+8, 16, true);
		int i;
		for ( i=0; i<KEY_NUM_MAIN_REDEFINABLE_KEYS; i++ )
		{
			int j;
			GraphDrawString( pVisBack, g_pFont8x8, 64, 64+i*16, (unsigned char*)g_aszKeys[i]);
			for ( j=strlen(g_aszKeys[i])+1; j<14; j++ )
			{
				GraphDrawString( pVisBack, g_pFont8x8, 64+j*8, 64+i*16, (unsigned char*)".");
			}
			if (i==nCurrent)
			{
				GraphDrawString( pVisBack, g_pFont8x8, 64+14*8, 64+i*16, (unsigned char*)"( )" );
				if ((SDL_GetTicks() % 700) < 400) // Draw flashing cursor thingy
				{
					GraphDrawString( pVisBack, g_pFont8x8, 64+14*8+8, 64+i*16, (unsigned char*)"_" );
					GraphDrawString( pVisBack, g_pFont8x8, 48, 64+nCurrent*16, (unsigned char*)"=>");
				}
			}
			else if (anKeys[i]!=0)
			{
				// Show new key
				char szBuf[2048] = {0};
				snprintf(szBuf, sizeof(szBuf), "(%s)", GetKeyString(anKeys[i]));
				GraphDrawString( pVisBack, g_pFont8x8, 64+14*8, 64+i*16, (unsigned char*)szBuf );
			}
			// Show previous key
			{
				char szBuf[2048] = {0};
				snprintf(szBuf, sizeof(szBuf), "(%s)", GetKeyString(g_anKeys[i]));
				GraphDrawString( pVisBack, g_pFont8x8, 64+22*8, 64+i*16, (unsigned char*)szBuf );
			}
		}

		GraphDrawString(pVisBack, g_pFont8x8, 8, 186, (unsigned char*)"'Action' is used for e.g. opening doors ");
		GraphDrawString(pVisBack, g_pFont8x8, 8, 192, (unsigned char*)"with keys or activating teleporters.");

		if (bFinished)
			GraphDrawString(pVisBack, g_pFont8x8, 48, 164, (unsigned char*)"Enter accepts, escape cancels");
		else
			GraphDrawString(pVisBack, g_pFont8x8, 48, 164, (unsigned char*)"Escape cancels");

		GraphFlip(true);

		//Prevent CPU hogging or it eats up a full core here [dj2016-10]
		SDL_Delay(20);
	} while (bLoop);
}

bool GetHighScoreUserName(std::string& sReturnString)
{
	#define MAX_HIGHSCORE_LEN 256
	#define WIDTH_INPUTBOX 34

	#ifdef djUNICODE_SUPPORT
	//SDL_EnableUNICODE(1);
	std::string sInput;
	#else
	char szBuffer[2048] = {0};//temp phase out?
	#endif

	bool bRet = true; // Return false if user selected quit/close or something
	bool bLoop = true;
	do
	{
		int nDX = WIDTH_INPUTBOX*8;
		//dj2019-07 for now just stick to 320; genericize better later re CFG_APPLICATION_RENDER_RES_W stuff ..
		//int nXLeft = (CFG_APPLICATION_RENDER_RES_W/2) - (nDX / 2);
		int nXLeft = (320/2) - (nDX / 2);

		// Black background
		djgSetColorFore( pVisBack, djColor(0,0,0) );
		djgDrawBox( pVisBack, 0, 0, CFG_APPLICATION_RENDER_RES_W, CFG_APPLICATION_RENDER_RES_H );
		// Stupid cheesy boring dialog-border effect
		DialogBoxEffect(nXLeft-12, 64, nDX+24, 64);

		djiPollBegin();
		SDL_Keymod ModState = SDL_GetModState();
		SDL_Event Event;
		while (djiPollEvents(Event))
		{
			switch (Event.type)
			{
#ifdef djUNICODE_SUPPORT
			case SDL_TEXTINPUT://dj2022-11 NB for Unicode input (what platforms are supported here?)
				sInput += Event.text.text;
				break;
			case SDL_KEYDOWN:
			{
				if (((Event.key.keysym.mod & KMOD_LSHIFT) == 0) && 
					((Event.key.keysym.mod & KMOD_RSHIFT) == 0) &&
					((Event.key.keysym.mod & KMOD_LCTRL) != 0) || (Event.key.keysym.mod & KMOD_RCTRL) != 0)
				{
					// Ctrl+V paste text?
					if (Event.key.keysym.sym == SDLK_v && SDL_HasClipboardText())
					{
						char* sz = SDL_GetClipboardText();
						if (sz)
						{
							// Hm what if it's crazy long .. put some reasonable limit in case someone pastes GBs of text .. this is a bit arb:
							if (strlen(sz) > 10000)
							{
								std::string s;
								for (int z = 0; z < 10000; ++z)
									s += sz[z];
								sInput += s;
							}
							else
								sInput += sz;
							SDL_free(sz);
						}
					}
				}


				switch (Event.key.keysym.sym)
				{
				case SDLK_BACKSPACE: // Backspace is slightly non-trivial to handle if we're dealing with utf8 strings but technically we can probably use 
					if (!sInput.empty())
					{
						/*
						Code point  UTF - 8 conversion
						First code point	Last code point	Byte 1	Byte 2	Byte 3	Byte 4
						U + 0000	U + 007F	0xxxxxxx
						U + 0080	U + 07FF	110xxxxx	10xxxxxx
						U + 0800	U + FFFF	1110xxxx	10xxxxxx	10xxxxxx
						U + 10000[nb 2]U + 10FFFF	11110xxx	10xxxxxx	10xxxxxx	10xxxxxx
						*/
						// Basically if:
						// (1) it's "0xxxxxxx" we can delete just that one char
						// (2) it starts with "10" in high bits we can delete UNTIL we hit something starting with "11" in high bits
						// We seriously need to double check this
						char cLast = sInput.back();
						if ((cLast & 0x80) == 0)//ascii
							sInput = sInput.substr(0, sInput.size() - 1);
						else if ((cLast & 0xC0) == 0x80)
						{
							while (!sInput.empty() && (cLast & 0xC0) == 0x80)
							{
								sInput = sInput.substr(0, sInput.size() - 1);
								if (!sInput.empty())
									cLast = sInput.back();
							}
							if (!sInput.empty())
								sInput = sInput.substr(0, sInput.size() - 1);
						}
					}
				}
			}
			break;
#else
			case SDL_KEYDOWN:
				if (Event.key.keysym.sym>=SDLK_a && Event.key.keysym.sym<=SDLK_z)
				{
					// I'm assuming these constants are linearly increasing, hopefully they are
					AppendCharacter(szBuffer, ((char)Event.key.keysym.sym - SDLK_a) + ((ModState & KMOD_SHIFT) ? 'A' : 'a'), MAX_HIGHSCORE_LEN);
				}
				else if (Event.key.keysym.sym>=SDLK_0 && Event.key.keysym.sym<=SDLK_9)
				{
					const char* acShifted = ")!@#$%^&*(";
					if (ModState & KMOD_SHIFT)
						AppendCharacter(szBuffer, acShifted[(char)Event.key.keysym.sym - SDLK_0], MAX_HIGHSCORE_LEN);
					else
						AppendCharacter(szBuffer, ((char)Event.key.keysym.sym - SDLK_0) + '0', MAX_HIGHSCORE_LEN);
				}
				else
				{
					switch (Event.key.keysym.sym)
					{
					case SDLK_SPACE:	AppendCharacter(szBuffer, ' ', MAX_HIGHSCORE_LEN); break;
					case SDLK_PLUS:		AppendCharacter(szBuffer, '+', MAX_HIGHSCORE_LEN); break;
					case SDLK_MINUS:	AppendCharacter(szBuffer, '-', MAX_HIGHSCORE_LEN); break;
					case SDLK_COMMA:	AppendCharacter(szBuffer, ',', MAX_HIGHSCORE_LEN); break;
					case SDLK_BACKSPACE:
						if (strlen(szBuffer)>0)
							szBuffer[strlen(szBuffer) - 1] = 0;
						break;
					}
				}
				break;
			case SDL_KEYUP:
				break;
#endif
			}
		}
		djiPollEnd();

		if (g_iKeys[DJKEY_ESC])
		{
			bLoop = false;
			bRet = false;
		}
		// Handle this when user lets go of enter key, not when presses down, otherwise
		// event transfers to main menu and starts a gnu game immediately.
		if (!g_iKeys[DJKEY_ENTER] && g_iKeysLast[DJKEY_ENTER])
		{
			bLoop = false;
		}

#ifdef djUNICODE_SUPPORT
		DialogBoxEffect(nXLeft-4, 100, nDX+8, 22, true);
#else
		DialogBoxEffect(nXLeft-4, 100, nDX+8, 16, true);
#endif
		GraphDrawString( pVisBack, g_pFont8x8, 100,  72, (unsigned char*)"New high score!");
		GraphDrawString( pVisBack, g_pFont8x8,  96,  88, (unsigned char*)"Enter your name:" );

#ifdef djUNICODE_SUPPORT
		extern void DrawUnicodeHelper(djVisual * pVis, int x, int y, SDL_Color Color, const std::string & sText);
		std::string sFakeCursor;
		if ((SDL_GetTicks() % 700) < 400) // Draw flashing cursor
			sFakeCursor = "|";
		DrawUnicodeHelper(pVisBack, nXLeft - 2, 104, SDL_Color{ 255, 255, 255, 255 }, sInput + sFakeCursor);
#else
		GraphDrawString( pVisBack, g_pFont8x8, nXLeft-2, 104, (unsigned char*)szBuffer );
		if ((SDL_GetTicks() % 700) < 400) // Draw flashing cursor
		{
			unsigned char szCursor[2] = { 254, 0 };
			GraphDrawString( pVisBack, g_pFont8x8, (nXLeft-2) + 8*strlen(szBuffer), 104, szCursor );
		}
#endif


		GraphFlip(true);

		//Prevent CPU hogging or it eats up a full core here [dj2019-07] (A little simplistic but it'll do)
		SDL_Delay(20);

	} while (bLoop);

#ifdef djUNICODE_SUPPORT
	sReturnString = sInput;
	//SDL_EnableUNICODE(0);
#else
	sReturnString = szBuffer;
#endif

	return bRet;
}

void CheckHighScores( int score )
{
	if (IsNewHighScore(score))
	{
		char szUserName[1024] = {0};
		std::string sUserName;
		if (GetHighScoreUserName(sUserName))
		{
			AddHighScore(sUserName.c_str(), score);
			SaveHighScores(); // Save high scores immediately, in case Windows crashes
		}

		ShowHighScores();
	}
}

void SelectMission()
{
	int i=0, iret=0;
	SMenuItem* pMenuItems=NULL;
	unsigned char menuMissionCursor[] = { 128, 129, 130, 131, 0 };
	CMenu menuMission ("main.pp:SelectMission()");
	menuMission.setClrBack ( djColor(48,66,128) );
	menuMission.setSize ( 0 );
	menuMission.setMenuCursor( menuMissionCursor );
	menuMission.setXOffset ( -1 );
	menuMission.setYOffset ( -1 );
	menuMission.setItems ( 0 );

	// Build the menu, adding mission names as entries
	pMenuItems = new SMenuItem[g_apMissions.size() + 3];
	for ( i=0; i<(int)g_apMissions.size(); i++ )
	{
		pMenuItems[i+1].m_bitem = true;
		char* szText = new char[512];
		//fixme ^ leaks
		snprintf( szText, 512, "   %-31.31s  ", g_apMissions[i]->GetName() );//snprintf( szText, 512, "|  %-31.31s |", g_apMissions[i]->GetName() );
		pMenuItems[i+1].m_szText = szText;//<- a bit gross [a bit you say]
	}
	// Top and bottom menu entries, the borders
	pMenuItems[0].m_bitem = false;
	pMenuItems[0].m_szText = djStrDeepCopy( "                                    " );//pMenuItems[0].m_szText = djStrDeepCopy( "{~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~}" );

	pMenuItems[g_apMissions.size()+1].m_bitem = false;
	pMenuItems[g_apMissions.size()+1].m_szText = djStrDeepCopy("                                    ");//djStrDeepCopy("[~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~]");
	pMenuItems[g_apMissions.size()+2].m_bitem = false; // Null-terminator
	pMenuItems[g_apMissions.size()+2].m_szText = NULL;

	menuMission.setItems ( pMenuItems );

	// Do menu
	iret = do_menu( &menuMission );
	if (iret != -1)
	{
		g_pCurMission = g_apMissions[iret - 1];
	}

	// Unallocate menu
	for ( i=0; i<(int)g_apMissions.size() + 2; i++ )
	{
		if (pMenuItems[i].m_szText)
		{
			djDELV(pMenuItems[i].m_szText);
		}
	}

	djDELV(pMenuItems);
}

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

	mainMenu.setSoundMove ( djSoundLoad( DATA_DIR "sounds/cardflip.wav" ) );
}

void KillMainMenu()
{
	// TODO
}
