// David Joffe, 1999/12 GNU etc.
/*
instructions.cpp

Copyright (C) 1999-2017 David Joffe

License: GNU GPL Version 2
*/
#include "djtypes.h"
#include "instructions.h"
#include "menu.h"
#include "graph.h"
#include <vector>
#include <string>

// I don't quite like these two dependencies, but because keys are re-definable, the
// instructions screen 'must' 'know about' them. Hm, a slightly better design might
// be more MVC-ish where e.g. a controller class that 'knows about' both redefinable keys
// and instructions screens could 'pre-configure' the instruction text or something..
// but that's extremely low, bottom of priority list of things worth worrying about.[dj2017-08]
#include "keys.h"
#include"djinput.h"//GetKeyString()

struct SMenuItem instructionsMenuItems[] =
{
   { false, "        " },
   { true,  "   OK   " },
   { false, "        " },
   { false, NULL }
};
unsigned char instructionsMenuCursor[] = { 128, 129, 130, 131, 0 };
CMenu instructionsMenu ( "instructions.cpp:instructionsMenu" );

//-------------------------------------------------------
void ShowInstructions()
{
	std::vector<std::string> asText;
	asText.push_back("INSTRUCTIONS:");
	// This plotline needs a bit more work (fixme dj2017-06-22)
	asText.push_back(" Find the exit in each level, while");
	asText.push_back(" dodging or shooting monsters. You");
	asText.push_back(" must save the world from the evil");
	asText.push_back(" SystemD.");
	asText.push_back("");
	asText.push_back("KEYS:");

	// Show new key
	char szBuf[1024] = {0};
	sprintf(szBuf, "%s", GetKeyString(g_anKeys[KEY_LEFT]));
	asText.push_back(std::string(" Left:   ") + szBuf);
	sprintf(szBuf, "%s", GetKeyString(g_anKeys[KEY_RIGHT]));
	asText.push_back(std::string(" Right:  ") + szBuf);
	sprintf(szBuf, "%s", GetKeyString(g_anKeys[KEY_SHOOT]));
	asText.push_back(std::string(" Shoot:  ") + szBuf);
	sprintf(szBuf, "%s", GetKeyString(g_anKeys[KEY_JUMP]));
	asText.push_back(std::string(" Jump:   ") + szBuf);
	sprintf(szBuf, "%s", GetKeyString(g_anKeys[KEY_ACTION]));
	asText.push_back(std::string(" Action: ") + szBuf + std::string(" (Activate doors,"));
	asText.push_back("  exits, lifts, teleporters etc.)");
	asText.push_back("");
	// Hrm, these may be platform specific, fixmeLOW:
	asText.push_back(" Esc: In-game menu: Save/Restore/Quit");
	asText.push_back(" PgUp/PgDn: Volume up/down");
	asText.push_back(" Ins: Toggle sounds");
	asText.push_back(" F4,F5: Sprite/Level editors");
	//asText.push_back("Use the action key to open doors, activate lifts, teleporters, etc.");
	asText.push_back("");
	asText.push_back("BONUSES:");
	asText.push_back("-Shoot all security cameras in level");
	asText.push_back("-Collect letters G,N,U,K,E,M in order");

	const int X = 8;
	const int Y = 8;
	const int H = (int)asText.size() + 1;
	// Calculate widest string, use that for 'dialog' size
	int W = (320/8)-2;//5;
	/*for ( int i=0; i<(int)asText.size(); ++i )
	{
		if (W < (int)asText[i].length())
			W = (int)asText[i].length();
	}*/

	// Draw 'blank' underneath text (pseudo-dialogue-background and border)
	djImage* pImgBackground = NULL;
	if (pImgBackground==NULL)
	{
		pImgBackground = new djImage;
		if (pImgBackground->Load( "data/menucharbackground.tga" )>=0)
		{
			djCreateImageHWSurface( pImgBackground );
		}
	}
	// This is slightly crass .. clear background
	for ( int i=0; i<H; ++i )
	{
		for ( int j=0; j<W; ++j )
		{
			djgDrawImage( pVisBack, pImgBackground, X+j*8, Y+i*8, 8, 8 );
		}
	}
	if (pImgBackground!=NULL)
	{
		djDestroyImageHWSurface( pImgBackground );
		djDEL(pImgBackground);
	}

	//left+top 'light' lines
	djgSetColorFore(pVisBack,djColor(80,80,80));
	djgDrawRectangle( pVisBack,
		X,
		Y,
		1,
		H*8);
	//top
	djgDrawRectangle( pVisBack,
		X,
		Y,
		W*8,
		1
		);
	//bottom+right 'dark' lines
	djgSetColorFore(pVisBack,djColor(35,35,35));
	djgDrawRectangle( pVisBack,
		X+2,
		Y+H*8,
		W*8-2,
		1);
	//right
	djgDrawRectangle( pVisBack,
		X+W*8,
		Y+2,
		1,
		H*8-1
		);


	// Draw lines of text
	for ( int i=0; i<(int)asText.size(); ++i )
	{
		GraphDrawString( pVisBack, g_pFont8x8, X+4, (Y+4)+i*8, (unsigned char*)asText[i].c_str() );
	}


	// OK 'button' (we 'abuse' the menu code to make an OK button that's basically
	// a menu with one option, a bit gross but does the job for now.)
	struct SMenuItem menuItemsOK[] =
	{
	   { false, "        " },
	   { true,  "   OK   " },
	   { false, "        " },
	   { false, NULL }
	};
	const unsigned char MenuCursor[] = { 128, 129, 130, 131, 0 };
	CMenu Menu ( "OK" );
	Menu.setSize(0);
	Menu.setItems(menuItemsOK);
	Menu.setMenuCursor(MenuCursor);
	Menu.setClrBack(djColor(48,66,128));
	Menu.setXOffset(320 - (8*8 + 4));
	Menu.setYOffset(52);
	//Menu.setXOffset(320/2 - (8*4));
	//Menu.setYOffset(200 - (8*4) + 4);
	do_menu( &Menu );
}
