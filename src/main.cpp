/*--------------------------------------------------------------------------*/
/* David Joffe '95/07/20 */
// Pre-1995 : something similar from QBasic or Pascal, I can't remember ...
// 1995 : original DOS/EGA version
// 1998/12/23 : begin attempted Linux port
// 1999/05 : begin attempted Win32 port, ack
// 1999/12 : re-begin attempted Win32 port
// 2001/05 : begin SDL port; doxygen comments
// 2016/10 : new github + livecoding 'era'

/*
Copyright (C) 1995-2001 David Joffe

License: GNU GPL Version 2 (*not* "later versions")
*/

/*--------------------------------------------------------------------------*/

#include "mmgr/mmgr.h"

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

#ifdef __APPLE__
#include <mach-o/dyld.h>//_NSGetExecutablePath
#include <sys/stat.h>//For djFolderExists stuff
#endif

#include <SDL_mixer.h>//For background music stuff

/*--------------------------------------------------------------------------*/
djImage *g_pImgMain = NULL;

int  DaveStartup(bool bFullScreen, bool b640);	// Application initialization
void DaveCleanup();					// Application cleanup
void SelectMission();				// Select a mission
void RedefineKeys();				// Redefine keys

void DoMainMenu();					// Start main menu
void CheckHighScores( int score );	// check if high score table is beaten,
						// let user enter his name
						// and show the table after all
void InitMainMenu();
void KillMainMenu();

/*--------------------------------------------------------------------------*/
// Main menu [NB, warning, the handling code uses indexes :/ .. so if you add/remove items, must update there too - dj2016-10]
struct SMenuItem mainMenuItems[] =
{
	/*{ false, "{~~~~~~~~~~~~~~~~~}" },//dj2016-10 try without the big borders ..
	{ true,  "|  Start gnu game |" },
	{ true,  "|  Select Mission |" },
	{ true,  "|  Restore game   |" },
	{ true,  "|  Ordering info  |" },
	{ true,  "|   (not!)        |" },
	{ true,  "|  Instructions   |" },
	{ true,  "|  Redefine keys  |" },
	{ true,  "|  High scores    |" },
	{ true,  "|  Credits        |" },
	{ true,  "|  Quit           |" },
	{ false, "[~~~~~~~~~~~~~~~~~]" },*/
	{ false, "                   " },
	{ true,  "   Start gnu game  " },
	{ true,  "   Select Mission  " },
	{ true,  "   Restore game    " },
	{ true,  "   Ordering info   " },
	{ true,  "    (not!)         " },
	{ true,  "   Instructions    " },
	{ true,  "   Redefine keys   " },
	{ true,  "   High scores     " },
	{ true,  "   Credits         " },
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
	// Check commandline args
	bool bfullscreen = false;
	bool b640 = false;

	if (argc > 1)
	{
		for ( int i=1; i<argc; i++ )
		{
			if (0 == strncmp( argv[i], "-f", 2 )) bfullscreen = true;
			if (0 == strncmp( argv[i], "-640", 4 )) b640 = true;
		}
	}
	else
	{
		// Show usage information
		printf( "---------------------------------------------------------\n" );
		printf( " Command-line options:\n" );
		printf( "   -f    Fullscreen mode\n" );
		printf( "   -640  640x480 mode\n" );
		printf( "---------------------------------------------------------\n" );
	}

	// Initialize everything
	if (0 != DaveStartup(bfullscreen, b640))
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

#ifdef __APPLE__
// Append a folder to existing path, 'intelligently' handling
// the trailing slash worries for us.
void djAppendPath(char* szPath,char* szAppend)
{
	if (szPath==NULL)return;
	if (szAppend==NULL||szAppend[0]==0)return;

	// If doesn't have trailing slash, add one (unless szPath is empty string)
	if (strlen(szPath)>0)
	{
		char cLast = szPath[ strlen(szPath)-1 ];
		if (cLast!='/' && cLast!='\\')
		{
			strcat(szPath,"/");
		}
	}
	strcat(szPath,szAppend);
}
/*bool djFolderExists(const char* szPath)
{
	struct stat sb;
	if (stat(szPath, &sb) == 0 && S_ISDIR(sb.st_mode))
	{
		return true;
	}
	return false;
}*/
bool djFileExists(const char* szPath)
{
	struct stat sb;
	if (stat(szPath, &sb) == 0 && S_ISREG(sb.st_mode))
	{
		return true;
	}
	return false;
}
#endif

int DaveStartup(bool bFullScreen, bool b640)
{
	InitLog ();

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
		djAppendPath(szDataFile,"data/missions.txt");//Some semi-'arb' Dave Gnukem data file someone is unlikely to have in say their user home folder or whatever
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
				djAppendPath(szDataFile,"data/missions.txt");
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

	Log ( "\n================[ Starting Application Init ]===============\n" );

	g_Settings.Load(CONFIG_FILE);	// Load saved settings
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

	srand(time(NULL));				// Seed the random number generator

	djTimeInit();					// Initialise timer

	InitialiseGameKeySystem();		// Initialise game keys

	//-- Initialize graphics
	//
	// NOTE: Use 640x480 if you want to use the built-in editor (F4/F5)
	//
	Log ("DaveStartup(): Initializing graphics system ...\n");
	int w=320;
	int h=200;
	if (b640 == true)
	{
		w = 640;
		h = 400;
	}
	// [dj2016-10] Note this w/h is effectively now a 'hint' as it may not initialize to the exact requested size
	if (!GraphInit( bFullScreen, w, h ))
	{
		Log( "DaveStartup(): Graphics initialization failed.\n" );
		return -1;
	}

	djSoundInit();				// Initialize sound

	g_pImgMain = new djImage;			// Load main skin (title screen)
	g_pImgMain->Load("data/main.tga");
	djCreateImageHWSurface( g_pImgMain );

	InitMissionSystem();

	// Load missions
	if (0 != LoadMissions("data/missions.txt"))
		return -1;
	Log ( "DaveStartup(): %d missions(s) found.\n", g_apMissions.size() );

	//-- Initialize input devices
	Log ("DaveStartup(): Initializing keys ..\n");
	if (!djiInit( pVisMain, INPUT_KEYDOWN|INPUT_KEYUP|INPUT_KEYREPEAT ))
		return -1;

	InitMainMenu();			// fill the structures and load some stuff

	GameInitialSetup();		// Once-off setup for the game itself (e.g. create in-game menu etc)

	Log ( "================[ Application Init Complete ]===============\n\n" );

	return 0;
}

void DaveCleanup()
{
	Log ( "\n================[ Starting Application Kill ]===============\n" );

	// Save user volume setting
	g_Settings.SetSettingInt("Volume",djSoundGetVolume());
	g_Settings.SetSetting("SoundsOn",djSoundEnabled()?"ON":"OFF");

	KillMainMenu();
	Log ( "KillMainMenu() ok\n" );
	KillMissionSystem();
	Log ( "KillMissionSystem() ok\n" );
	KillCredits();
	Log ( "KillCredits() ok\n" );
	SaveHighScores();		// Save high scores
	Log ( "SaveHighScores() ok\n" );
	GameFinalCleanup();		// Game
	Log ( "GameFinalCleanup() ok\n" );
	djiDone();			// Input
	Log ( "djiDone() ok\n" );
	djSoundDone();			// Sound
	Log ( "djSoundDone() ok\n" );
	djDEL(g_pImgMain);		// Delete main menu background image (title screen)
	Log ( "djDEL(g_pImgMain) ok\n" );
	GraphDone();			// Graphics
	Log ( "GraphDone() ok\n" );
	djTimeDone();			// Timer stuff
	Log ( "djTimeDone() ok\n" );

	g_Settings.Save(CONFIG_FILE);	// Save settings
	Log ( "g_Settings.Save(CONFIG_FILE) ok\n" );

	Log ( "================[ Application Kill Complete ]===============\n\n" );

//	KillLog ();
}

void DoMainMenu()
{
	bool bRunning = true;

	//dj2016-10 adding background music to main menu, though have not put any real thought into what would
	// be the best track here so fixme todo maybe dig a bit more and find better choice here etc. [also for levels]
	Mix_Music* pMusic = Mix_LoadMUS("data/music/eric_matyas/8-Bit-Mayhem.ogg");
	if (pMusic!=NULL)
		Mix_FadeInMusic(pMusic, -1, 800);

	do
	{
		// Load main menu background image
		if (g_pImgMain)
			djgDrawImage( pVisBack, g_pImgMain, 0, 0, g_pImgMain->Width(), g_pImgMain->Height() );
		char sz[100]={0};
		sprintf(sz,"%s","v0.65b (23 Oct 2016)");
		GraphDrawString(pVisBack, g_pFont8x8, 320 - strlen(sz)*8, 200 - 8*2, (unsigned char*)sz);
		sprintf(sz,"%s","http://djoffe.com/");
		GraphDrawString(pVisBack, g_pFont8x8, 320 - strlen(sz)*8, 200 - 8, (unsigned char*)sz);

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
			// Game levels start their own music, so when come out of game and back to main menu, restart main menu music
			if (pMusic!=NULL)
				Mix_FadeInMusic(pMusic, -1, 800);
			break;
		}
		case 2: // select mission
			SelectMission();
			break;
		case 3: // restore game [dj2016-10 adding implementation for this - it did nothing before]
			{
				int score = game_startup(true);
				CheckHighScores( score );
				// Game levels start their own music, so when come out of game and back to main menu, restart main menu music
				if (pMusic!=NULL)
					Mix_FadeInMusic(pMusic, -1, 800);
			}
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
		case -1: // escape
		case 10: // quit
			bRunning = false;
			break;
		}
	} while (bRunning);

	if (pMusic)
	{
		Mix_FreeMusic(pMusic);
		pMusic = NULL;
	}
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
	for ( unsigned int i=0; i<KEY_NUMKEYS; ++i )
	{
		if (anKeys[i] == key)
			return true;
	}
	return false;
}
void RedefineKeys()
{
	int anKeys[KEY_NUMKEYS] = {0};
	bool bLoop = true;
	bool bFinished = false;
	int nCurrent = 0;
	do
	{
		int nDX = 152*2;
		int nXLeft = (320/2) - (nDX/2);

		// Black background
		djgSetColorFore( pVisBack, djColor(0,0,0) );
		djgDrawBox( pVisBack, 0, 0, 320, 200 );
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
							for ( j=0; j<KEY_NUMKEYS; j++ )
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
						if (nCurrent==KEY_NUMKEYS)
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
		for ( i=0; i<KEY_NUMKEYS; i++ )
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
				char szBuf[1024] = {0};
				sprintf(szBuf, "(%s)", GetKeyString(anKeys[i]));
				GraphDrawString( pVisBack, g_pFont8x8, 64+14*8, 64+i*16, (unsigned char*)szBuf );
			}
			// Show previous key
			{
				char szBuf[1024] = {0};
				sprintf(szBuf, "(%s)", GetKeyString(g_anKeys[i]));
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

bool GetHighScoreUserName(char *szBuffer)
{
#define MAX_HIGHSCORE_LEN 22
	bool bRet = true; // Return false if user selected quit/close or something
	bool bLoop = true;
	do
	{
		int nDX = MAX_HIGHSCORE_LEN*8;
		int nXLeft = (320/2) - (nDX / 2);

		// Black background
		djgSetColorFore( pVisBack, djColor(0,0,0) );
		djgDrawBox( pVisBack, 0, 0, 320, 200 );
		// Stupid cheesy boring dialog-border effect
		DialogBoxEffect(nXLeft-12, 64, nDX+24, 64);

		djiPollBegin();
		SDLMod ModState = SDL_GetModState();
		SDL_Event Event;
		while (djiPollEvents(Event))
		{
			switch (Event.type)
			{
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
				/*
			case SDL_QUIT:
				bLoop = false;
				bRet = false;
				break;
				*/
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

		DialogBoxEffect(nXLeft-4, 100, nDX+8, 16, true);
		GraphDrawString( pVisBack, g_pFont8x8, 100,  72, (unsigned char*)"New high score!");
		GraphDrawString( pVisBack, g_pFont8x8,  96,  88, (unsigned char*)"Enter your name:" );
		GraphDrawString( pVisBack, g_pFont8x8, nXLeft-2, 104, (unsigned char*)szBuffer );
		if ((SDL_GetTicks() % 700) < 400) // Draw flashing cursor
		{
			unsigned char szCursor[2] = { 254, 0 };
			GraphDrawString( pVisBack, g_pFont8x8, (nXLeft-2) + 8*strlen(szBuffer), 104, szCursor );
		}


		GraphFlip(true);
	} while (bLoop);
	return bRet;
}

void CheckHighScores( int score )
{
	if (IsNewHighScore(score))
	{
		char szUserName[1024] = {0};
		if (GetHighScoreUserName(szUserName))
		{
			AddHighScore(szUserName, score);
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
		char* szText = new char[256];
		//fixme ^ leaks
		sprintf( szText, "   %-31.31s  ", g_apMissions[i]->GetName() );//sprintf( szText, "|  %-31.31s |", g_apMissions[i]->GetName() );
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
	mainMenu.setSoundMove ( djSoundLoad( "data/sounds/cardflip.wav" ) );
}

void KillMainMenu()
{
	// TODO
}
