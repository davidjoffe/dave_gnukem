/*--------------------------------------------------------------------------*/
// graph.cpp
// David Joffe 1998/12
// replacing the old djgega.cpp graphics interface
/*
Copyright (C) 1998-2001 David Joffe

License: GNU GPL Version 2 (*not* "later versions")
*/
/*--------------------------------------------------------------------------*/

#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "SDL.h"

#include "graph.h"

// These dependencies suck
#include "djlog.h"

#include "djtypes.h"


djImage *g_pFont8x8=NULL;

djVisual *pVisMain = NULL;
djVisual *pVisBack = NULL;
djVisual *pVisView = NULL;

void GraphFlip()
{
	djgFlip( pVisMain, pVisBack );
}

bool GraphInit( bool bFullScreen, int iWidth, int iHeight )
{
	int imode;

	// Initialize graphics library
	SDL_Init(SDL_INIT_VIDEO);

	SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);

	// Window dressing crap
	SDL_WM_SetCaption("Dave Gnukem", NULL);
	SDL_WM_SetIcon(SDL_LoadBMP("data/icon.bmp"), NULL);
	// Hide mouse cursor
	SDL_ShowCursor(0);

	//--- (1) - Front buffer
	if (NULL == (pVisMain = djgOpenVisual( bFullScreen?"fullscreen":NULL, iWidth, iHeight )))
	{
		printf( "GraphInit(): COULDNT OPEN GMAIN\n" );
		return false;
	}
	imode = pVisMain->bpp;

	// Set the 32<->16 pixel conversion atributes, so the
	// images would be displayed correctly with any pixel format
	SetPixelConversion ( pVisMain );

	//--- (2) - Back buffer
	if (NULL == (pVisBack = djgOpenVisual( "memory", iWidth, iHeight, imode )))
	{
		printf( "GraphInit(): COULDNT OPEN GBACK\n" );
		return false;
	}

	//--- (3) - View buffer
	if (NULL == (pVisView = djgOpenVisual( "memory", iWidth, iHeight, imode )))
	{
		printf( "GraphInit(): COULDNT OPEN GVIEW\n" );
		return false;
	}

	//--- (5) - Load 8x8 font bitmap (FIXME error check)
	if (NULL != (g_pFont8x8 = new djImage))
		g_pFont8x8->Load( FILE_IMG_FONT );

	return true;
}

void GraphDone()
{
	djDEL(g_pFont8x8);

	djgCloseVisual( pVisView );
	djgCloseVisual( pVisBack );
	djgCloseVisual( pVisMain );

	djDEL(pVisView);
	djDEL(pVisBack);
	djDEL(pVisMain);

	SDL_Quit();
}

// FIXME: , view_height?
void GraphFlipView( int iViewWidth )
{
	djgDrawVisual( pVisBack, pVisView, 16, 16, 16, 16, iViewWidth*16, 10*16 );
}

// FIXME: Currenetly assumes a 256-char 32x8 character 256x128 pixel alpha-mapped image
void GraphDrawString( djVisual *pVis, djImage *pImg, int x, int y, const unsigned char *szStr )
{
	// FIXME: bounds check properyl
	if (x<0 || y<0) return;

	// Draw each character in the string
	for ( unsigned int i=0; i<strlen((char*)szStr); i++ )
	{
		int iIndex = (int)szStr[i];
		djgDrawImageAlpha( pVis, pImg, 8*(iIndex%32), 8*(iIndex/32), x+i*8, y, 8, 8 );
	}
}

