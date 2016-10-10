/*--------------------------------------------------------------------------*/
/* David Joffe '95/07/20 */
/*
menu.cpp

Copyright (C) 1995-2001 David Joffe

License: GNU GPL Version 2 (*not* "later versions")
*/

#include "graph.h"

#include <string.h>

#include "menu.h"

#include "djstring.h"
#include "djinput.h"
#include "djtime.h"
#include "sys_error.h"


int option;

void menu_move( CMenu *pMenu, int& option, int diff, unsigned char cCursor )
{
	djgSetColorFore( pVisBack, pMenu->getClrBack() );
	djgDrawBox( pVisBack, pMenu->getXOffset()+8, pMenu->getYOffset()+option*8, 8, 8 );

	int iOptionPrev = option;
	option += diff;
	if (option < 1)                    option = pMenu->getSize() - 2;
	if (option > (pMenu->getSize() - 2)) option = 1;

	// Play a sound
	if (option != iOptionPrev)
	{
		djSoundPlay( pMenu->getSoundMove () );
	}

	// Redraw the menu cursor
	djgDrawImageAlpha( pVisBack, g_pFont8x8, ((int)cCursor%32)*8, ((int)cCursor/32)*8, pMenu->getXOffset()+8, pMenu->getYOffset()+8*option, 8, 8 );

	GraphFlip(true);
}
/*--------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------*/
int do_menu( CMenu *pMenu )
{
	int option;
	int i;
	int size;
	const unsigned char *szCursor;

	// Initialize cursor animation
	szCursor = pMenu->getMenuCursor();

	// set default option
	option = 1;

	// calculate size of menu
	for ( size=0; pMenu->getItems()[size].m_szText != NULL; size++ )
		;

	pMenu->setSize ( size );

	// Calculate position of menu on screen, if "asked" to do so (== -1)
	if ( pMenu->getXOffset() == -1 )
		pMenu->setXOffset ( 8 * (20 - (strlen(pMenu->getItems()[0].m_szText) / 2)));
	if ( pMenu->getYOffset() == -1 )
		pMenu->setYOffset( 8 * (12 - (size / 2)) );

	// draw menu
	for ( i=0; i<size; i++ )
	{
		// Draw blank underneath menu (FIXME: Somehow should be able to be image underneath)

		djgSetColorFore( pVisBack, pMenu->getClrBack() );
		djgDrawBox( pVisBack, pMenu->getXOffset(), pMenu->getYOffset()+i*8, 8 * strlen(pMenu->getItems()[i].m_szText), 8 );

		// Draw menu text
		GraphDrawString( pVisBack, g_pFont8x8, pMenu->getXOffset(), pMenu->getYOffset()+i*8, (unsigned char*)pMenu->getItems()[i].m_szText );
	}
	menu_move( pMenu, option, 0, *szCursor );



	// Wait for user to let go of escape, if he is pressing it
	djiWaitForKeyUp( DJKEY_ESC );



	// We want to maintain 10 fps on menu cursor animations
	float fTimeFrame = (1.0f / 10.0f);

	// Start out by being at next time
	float fTimeNext = djTimeGetTime();
	float fTimeNow = fTimeNext;

	bool bmenurunning = true;

	do
	{
		fTimeNow = djTimeGetTime();
		bool bForceUpdate = false;
		// If we're already behind, just force us to get there
		if (fTimeNext < fTimeNow)
		{
			//printf( "slow\n" );
			fTimeNext = fTimeNow;
			bForceUpdate = true;
		}

		while (bmenurunning && (fTimeNow<fTimeNext || bForceUpdate)) // until we reach next frames time
		{
			SDL_Delay(10);

			// poll keys
			djiPoll();

			// up arrow
			static bool bkeyup = false;
			if ( (g_iKeys[DJKEY_UP]) && (bkeyup == false) )
			{
				menu_move( pMenu, option, -1, *szCursor );
			}
			bkeyup = (bool)(g_iKeys[DJKEY_UP]!=0);

			// down arrow
			static bool bkeydown = false;
			if ( (g_iKeys[DJKEY_DOWN]) && (bkeydown == false) )
			{
				menu_move( pMenu, option, 1, *szCursor );
			}
			bkeydown = (bool)(g_iKeys[DJKEY_DOWN] != 0);

			// home key
			if (g_iKeys[DJKEY_HOME])
				menu_move( pMenu, option, -option + pMenu->getSize() - 1, *szCursor );

			// end key
			if (g_iKeys[DJKEY_END])
				menu_move( pMenu, option, -option, *szCursor );

			// enter
			if (g_iKeys[DJKEY_ENTER])
				bmenurunning = 0;

			// escape
			if (g_iKeys[DJKEY_ESC])
			{
				option = -1;
				bmenurunning = 0;
			}


			fTimeNow = djTimeGetTime();
			bForceUpdate = false;
		}
		fTimeNext = fTimeNow + fTimeFrame;


		if (bmenurunning)
		{
			// Animate cursor
			szCursor++;
			if (*szCursor == 0)
				szCursor = pMenu->getMenuCursor ();

			menu_move( pMenu, option, 0, *szCursor );
		}

	} while (bmenurunning);

	// Wait for user to let go of escape or enter
	if (option == -1)
		djiWaitForKeyUp(DJKEY_ESC);
	else
		djiWaitForKeyUp(DJKEY_ENTER);


	return option;
}
/*--------------------------------------------------------------------------*/

CMenu::CMenu(const char *idstr) :
	m_items(NULL),
	m_szCursor(NULL),
	m_xOffset(-1),
	m_yOffset(-1),
	m_iSize(-1),
	m_clrBack(djColor(0, 0, 0)),
	m_iSoundMove(SOUNDHANDLE_INVALID)
{
	idstring = djStrDeepCopy(idstr);
}

CMenu::~CMenu()
{
	SYS_Debug ( "CMenu::~CMenu (%s)\n", idstring );

	djDELV(idstring);

	// Do NOT delete m_szCursor or m_items. At the moment they're pointers to globals ... bad old code.
	// This should change later, and be consistent.

	SYS_Debug ( "CMenu::~CMenu ()ok\n" );
	SYS_Debug ( "TODO: items and cursor are not deleted b/c in most cases the yare static strings. May need some changes later\n" );
}
