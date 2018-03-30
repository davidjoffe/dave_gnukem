/*--------------------------------------------------------------------------*/
/* David Joffe '95/07/20 */
/*
menu.cpp

Copyright (C) 1995-2018 David Joffe

License: GNU GPL Version 2
*/

#include "graph.h"

#include <string.h>

#include "menu.h"

#include "djstring.h"
#include "djinput.h"
#include "djtime.h"
#include "sys_error.h"

djImage* g_pImgMenuBackground8x8 = NULL;//dj2016-10 background 'noise' image experiment

void menu_move( CMenu *pMenu, int& option, int diff, unsigned char cCursor )
{
	//djgSetColorFore( pVisBack, pMenu->getClrBack() );
	//djgDrawBox( pVisBack, pMenu->getXOffset()+8, pMenu->getYOffset()+option*8, 8, 8 );
	djgDrawImage( pVisBack, g_pImgMenuBackground8x8, pMenu->getXOffset()+8, pMenu->getYOffset()+option*8, 8, 8 );

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
}
/*--------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------*/
int do_menu( CMenu *pMenu )
{
	int i=0;
	int size=0;

	//dj2016-10-28 trying background image with 'noise' instead of solid background color for menu ..
	if (g_pImgMenuBackground8x8==NULL)
	{
		g_pImgMenuBackground8x8 = new djImage;
		if (g_pImgMenuBackground8x8->Load( "data/menucharbackground.tga" )>=0)
		{
			djCreateImageHWSurface( g_pImgMenuBackground8x8 );
			//bLoaded = true;
		}
	}

	// Initialize cursor animation
	const unsigned char *szCursor = pMenu->getMenuCursor();

	// set default option
	int option = 1;

	// calculate size of menu
	for ( size=0; pMenu->getItems()[size].m_szText != NULL; size++ )
		;

	pMenu->setSize ( size );
	if (size==0)return -1;//error[dj2018-03]

	// Calculate position of menu on screen, if "asked" to do so (== -1)
	if ( pMenu->getXOffset() == -1 )
		pMenu->setXOffset ( 8 * (20 - (strlen(pMenu->getItems()[0].m_szText) / 2)));
	if ( pMenu->getYOffset() == -1 )
		pMenu->setYOffset( 8 * (12 - (size / 2)) );

	// Draw dropshadows [dj2018-03-30]
	const unsigned char szTR[2]={(unsigned char)251,0};//top right
	const unsigned char szR [2]={(unsigned char)252,0};//right
	const unsigned char szBR[2]={(unsigned char)250,0};//bottom right
	const unsigned char szB [2]={(unsigned char)249,0};//bottom
	const unsigned char szBL[2]={(unsigned char)248,0};//bottom left
	unsigned int uWidth = strlen(pMenu->getItems()[0].m_szText);//[dj2018-03] *Assumes* (true for v1) that strlen the same for all items
	for ( i=0; i<size; ++i )
	{
		GraphDrawString( pVisBack, g_pFont8x8, pMenu->getXOffset()+(strlen(pMenu->getItems()[i].m_szText))*8, pMenu->getYOffset()+i*8, i==0?szTR:szR );
	}
	for ( i=1; i<uWidth; ++i )
	{
		GraphDrawString( pVisBack, g_pFont8x8, pMenu->getXOffset()+i*8, pMenu->getYOffset()+size*8, szB );
	}
	GraphDrawString( pVisBack, g_pFont8x8, pMenu->getXOffset()+uWidth*8, pMenu->getYOffset()+size*8, szBR );
	GraphDrawString( pVisBack, g_pFont8x8, pMenu->getXOffset(), pMenu->getYOffset()+size*8, szBL );

	// draw menu
	for ( i=0; i<size; i++ )
	{
		// Draw blank underneath menu (FIXME: Somehow should be able to be image underneath)

		//djgSetColorFore( pVisBack, pMenu->getClrBack() );
		//djgDrawBox( pVisBack, pMenu->getXOffset(), pMenu->getYOffset()+i*8, 8 * strlen(pMenu->getItems()[i].m_szText), 8 );
		//static bool bLoaded=false;
		//if (bLoaded)
		{
			for ( unsigned int j=0; j<strlen(pMenu->getItems()[i].m_szText); ++j )
			{
				djgDrawImage( pVisBack, g_pImgMenuBackground8x8, pMenu->getXOffset()+j*8, pMenu->getYOffset()+i*8, 8, 8 );
			}

			//left+top 'light' lines
			djgSetColorFore(pVisBack,djColor(80,80,80));
			djgDrawRectangle( pVisBack,
				pMenu->getXOffset(),
				pMenu->getYOffset(),
				1,
				size*8);
			//top
			djgDrawRectangle( pVisBack,
				pMenu->getXOffset(),
				pMenu->getYOffset(),
				strlen(pMenu->getItems()[0].m_szText)*8,
				1
				);

			//bottom+right 'dark' lines
			djgSetColorFore(pVisBack,djColor(35,35,35));
			djgDrawRectangle( pVisBack,
				pMenu->getXOffset()+2,
				pMenu->getYOffset()+size*8,
				strlen(pMenu->getItems()[0].m_szText)*8-2,
				1);
			//right
			djgDrawRectangle( pVisBack,
				pMenu->getXOffset()+strlen(pMenu->getItems()[0].m_szText)*8,
				pMenu->getYOffset()+2,
				1,
				size*8-1
				);
		}


		// Draw menu text
		GraphDrawString( pVisBack, g_pFont8x8, pMenu->getXOffset(), pMenu->getYOffset()+i*8, (unsigned char*)pMenu->getItems()[i].m_szText );
	}
	menu_move( pMenu, option, 0, *szCursor );



	// Wait for user to let go of escape, if he is pressing it
	djiWaitForKeyUp( DJKEY_ESC );



	// We want to maintain 10 fps on menu cursor animations
	const float fTimeFrame = (1.0f / 10.0f);

	// Start out by being at next time
	float fTimeNext = djTimeGetTime();
	float fTimeNow = fTimeNext;

	bool bmenurunning = true;

	do
	{
		fTimeNow = djTimeGetTime();
		fTimeNext = fTimeNow + fTimeFrame;
		
		// Sleep a little to not hog CPU to cap menu update (frame rate) at approx 10Hz
		while (fTimeNow<fTimeNext)
		{
			SDL_Delay(5);
			fTimeNow = djTimeGetTime();
		}

		// [dj2016-10] Re-implementing this to do own djiPollBegin/djiPollEnd in menu instead of calling djiPoll()
		// because of issue whereby key events get 'entirely' missed if up/down even within one 'frame'.
		djiPollBegin();
		SDL_Event Event;
		while (SDL_PollEvent(&Event))
		{
			switch (Event.type)
			{
			case SDL_KEYDOWN:

				// 'Global' shortcut keys for adjusting volume [dj2016-10]
				if (Event.key.keysym.sym==SDLK_7)//SDLK_PAGEUP)
				{
					djSoundAdjustVolume(4);
					SetConsoleMessage( djStrPrintf( "Volume: %d%%", (int) ( 100.f * ( (float)djSoundGetVolume()/128.f ) ) ) );
				}
				else if (Event.key.keysym.sym==SDLK_6)//SDLK_PAGEDOWN)
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

				// up arrow
				else if (Event.key.keysym.sym==SDLK_UP)
					menu_move( pMenu, option, -1, *szCursor );

				// down arrow
				else if (Event.key.keysym.sym==SDLK_DOWN)
					menu_move( pMenu, option, 1, *szCursor );

				// home key
				else if (Event.key.keysym.sym==SDLK_HOME)//g_iKeys[DJKEY_HOME])
					menu_move( pMenu, option, -option + pMenu->getSize() - 1, *szCursor );

				// end key
				else if (Event.key.keysym.sym==SDLK_END)//if (g_iKeys[DJKEY_END])
					menu_move( pMenu, option, -option, *szCursor );

				// enter
				else if (Event.key.keysym.sym==SDLK_RETURN)//if (g_iKeys[DJKEY_ENTER])
					bmenurunning = 0;

				// escape
				else if (Event.key.keysym.sym==SDLK_ESCAPE)//if (g_iKeys[DJKEY_ESC])
				{
					option = -1;
					bmenurunning = 0;
				}

				break;
			case SDL_KEYUP:
				break;
			case SDL_QUIT:
				bmenurunning=0;
				option = -1;//Exit
				break;
			}
		}
		djiPollEnd();

		// [dj2016-10] this if seems silly here to me but if i take it out, then as you press Esc on menu,
		// it draws some 'wrong' stuff for one frame .. whatever, just adding this if back again
		if (bmenurunning)
		{
			// Animate cursor [note this is unfortunately currently a bit 'tied' to the 10Hz frame rate limit ...
			// if want to e.g. increase menu frame rate in future to say 20Hz or whatever, then the cursor will
			// animate two times too fast (say) .. if do that in future then must just make this update slightly
			// 'smarter' on the animation - not a priority now at all. dj2016-10]
			szCursor++;
			if (*szCursor == 0)
				szCursor = pMenu->getMenuCursor ();

			menu_move( pMenu, option, 0, *szCursor );//Force redraw of cursor for animation purposes
		}

		GraphFlip(true);
	} while (bmenurunning);

	// Wait for user to let go of escape or enter
	if (option == -1)
		djiWaitForKeyUp(DJKEY_ESC);
	else
		//this isn't working [anymore?] for redefine keys???
		djiWaitForKeyUp(DJKEY_ENTER);

	//Mix_FadeOutChannel(1, 1000);


	if (g_pImgMenuBackground8x8!=NULL)
	{
		djDestroyImageHWSurface( g_pImgMenuBackground8x8 );
		djDEL(g_pImgMenuBackground8x8);
	}

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
