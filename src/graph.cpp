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
#include "sys_log.h"//Log

#include "graph.h"

// These dependencies suck
#include "djlog.h"

#include "djtypes.h"


djImage *g_pFont8x8=NULL;

djVisual *pVisMain = NULL;
djVisual *pVisBack = NULL;
djVisual *pVisView = NULL;

// ScaleView should also be false for the level editor [why does that already work?]
void GraphFlip(bool bScaleView)
{
	djgFlip( pVisMain, pVisBack, bScaleView );
}

bool GraphInit( bool bFullScreen, int iWidth, int iHeight )
{
	int imode;

	// Initialize graphics library
	SDL_Init(SDL_INIT_VIDEO);

	// [dj2016-10] Get the user's monitor resolution, and find (basically) largest multiple of 320x200 that fits in
	// that size, to make for 'largest possible' gameplay window, that also scales from 320x200 'proportionally' nicely
	// (i.e. square aspect ratio of pixels, etc.)
	// [low/future] - if 2 monitors, will this behave 'correct'
	const SDL_VideoInfo* vidinfo = SDL_GetVideoInfo();
	if (vidinfo)
	{
		// THIS MUST BE TESTED ON LINUX [dj2016-10]
		int max_w = vidinfo->current_w;
		int max_h = vidinfo->current_h;
		if (max_w>iWidth && max_h>iHeight)
		{
			int nMultiple = djMAX(1, djMIN( vidinfo->current_w / iWidth, vidinfo->current_h / iHeight ) );
			iWidth *= nMultiple;
			iHeight *= nMultiple;
		}

		Log( "DaveStartup(): DisplayResolution(%d,%d).\n", max_w, max_h );
	}

	SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);

	// Window dressing crap
	SDL_WM_SetCaption("Dave Gnukem", NULL);
	SDL_WM_SetIcon(SDL_LoadBMP("data/icon.bmp"), NULL);
	// Hide mouse cursor
	SDL_ShowCursor(0);

	//--- (1) - Front buffer
	Log( "DaveStartup(): djgOpenVisual(w,h=%d,%d).\n", iWidth, iHeight );
	if (NULL == (pVisMain = djgOpenVisual( bFullScreen?"fullscreen":NULL, iWidth, iHeight )))
	{
		printf( "GraphInit(): COULDNT OPEN GMAIN\n" );
		return false;
	}
	Log( "DaveStartup(): Display bytes per pixel %d\n", pVisMain->bpp) ;
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
	{
		g_pFont8x8->Load( FILE_IMG_FONT );
		djCreateImageHWSurface( g_pFont8x8 );
	}

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
void GraphFlipView( int iViewWidth, int iViewHeight )
{
	djgDrawVisual( pVisBack, pVisView, 16, 16, 16, 16, iViewWidth*16, iViewHeight*16 );
}

// FIXME: Currenetly assumes a 256-char 32x8 character 256x128 pixel alpha-mapped image
void GraphDrawString( djVisual *pVis, djImage *pImg, int x, int y, const unsigned char *szStr )
{
	// FIXME: bounds check properyl
	if (x<0 || y<0) return;

	// Draw each character in the string
	for ( unsigned int i=0; i<strlen((char*)szStr); ++i )
	{
		int iIndex = (int)szStr[i];
		djgDrawImageAlpha( pVis, pImg, 8*(iIndex%32), 8*(iIndex/32), x+i*8, y, 8, 8 );
	}
}

