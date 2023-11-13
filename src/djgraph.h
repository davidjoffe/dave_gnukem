/*!
\file    djgraph.h
\brief   Graphics functionality
\author  David Joffe

Copyright (C) 1998-2022 David Joffe
*/
/*--------------------------------------------------------------------------*/
#ifndef _DJGRAPH_H_
#define _DJGRAPH_H_

#ifdef __OS2__
#include <SDL/SDL.h>
#else
//emsdk ..
#include <SDL/SDL.h>
#endif

#include "djimage.h"
#include "djtypes.h"
/*--------------------------------------------------------------------------*/

// DEBUG: changed by rtfb
/*
#define PIXEL32TO16(i) ( ((((i&0xFF0000)>>16) / 8) << 11) |   ((((i&  0xFF00)>> 8) / 4) <<  5) |   ((((i&    0xFF)    ) / 8)      ) )
/*/
//#define PIXEL32TO16(i) (((((i&0xff0000)>>16)/8)<<0xa) | ((((i&0xFF00)>>8)/8)<<0x5) | (((i&0xFF)/8)))
/**/

extern int	rMask, gMask, bMask, aMask;
extern int	rShift, gShift, bShift, aShift;


inline unsigned short PIXEL32TO16 ( unsigned long p )
{
	return (unsigned short)
		(
		((((p&0xff0000)>>16)/8)<<rShift) |
		((((p&0xFF00)>>8)/8)<<gShift) |
		(((p&0xFF)/8)<<bShift)
		);
}



//! Roughly, in this source code there are three main of these 'visuals' - one for the main window, a main backbuffer ('pVisBack' - probably not used for editor), and a main 'gameview render' buffer ('pVisView') ... this is perhaps slightly confusing sometimes but has reasons, though not to say it's the best or ideal for any and every game, it's just how this evolved over the years as we changed backends etc. - dj2022
class djVisual
{
public:
	djVisual()
	{
		colorfore = djColor(255,255,255);
		colorback = djColor(0,0,0);
		width    = 0;
		height   = 0;
		stride   = 0;
		bpp      = 0;
		pixwidth = 0;
		m_bFullscreen = false;
	}
	SDL_Window* pWindow = nullptr;//dj2022-11 for in-game fullscreen toggle stuff (so we can clean up with SDL_DestroyWindow)
	SDL_Surface *pSurface = nullptr;
	SDL_Renderer *pRenderer = nullptr;
	SDL_Texture *pTexture = nullptr;
	djColor                  colorfore;
	djColor                  colorback;
	int                      width;
	int                      height;
	int                      stride;
	int                      bpp;         // 32 ... 24? (pixsize) pixdepth=24, pixsize=32 width=4
	int                      pixwidth;    // 4
	bool                     m_bFullscreen;
};

//dj2022-11 refactoring to help with potential fullscreen toggle capability
extern bool djSDLInit();
extern bool djSDLDone();

/*--------------------------------------------------------------------------*/
// Visual management

//! Create a visual (if vistype is null or if fullscreen this creates a main window, if "memory" it creates on offscreen buffer e.g. extra game backbuffer or viewbuffer)
extern djVisual *djgOpenVisual( const char * vistype, int w, int h, int bpp = 0, bool bBackbuffer=false, const char* szWindowTitle="", const char* szWindowIconFile = nullptr);
//! Destroy a visual
extern void      djgCloseVisual( djVisual * pVis );
//! Lock a visual for surface buffer drawing
extern void      djgLock( djVisual * pVis );
//! Unlock a visual
extern void      djgUnlock( djVisual * pVis );
//! Flush any pending drawing operations to visual buffer
extern void      djgFlush( djVisual * pVis );
/*--------------------------------------------------------------------------*/
// 2D drawing functions

//-- Color functions

//! set foreground and background colors
extern void       djgSetColor( djVisual *pVis, const djColor& clrFore, const djColor& clrBack );
//! set foreground color
extern void       djgSetColorFore( djVisual * pVis, const djColor& color );
//! set background color
extern void       djgSetColorBack( djVisual * pVis, const djColor& color );

//-- Drawing primitives

//! clear a visual
extern void       djgClear( djVisual * pVis );
//! Put pixel
extern void       djgPutPixel( djVisual * pVis, int x, int y, int r, int g, int b );
//! Put pixel
extern void       djgPutPixel( djVisual * pVis, int x, int y, const djColor& color );
//! draw rectangle in current color
extern void       djgDrawRectangle( djVisual * pVis, int x, int y, int w, int h );
//! draw filled box in current color
extern void       djgDrawBox( djVisual * pVis, int x, int y, int w, int h );
//! draw horizontal line in current color
extern void       djgDrawHLine( djVisual * pVis, int x, int y, int n );
//! draw vertical line in current color
extern void       djgDrawVLine( djVisual * pVis, int x, int y, int n );

//-- Image blitting

//! Draw (w,h) pixels of an image at (x,y)
extern void       djgDrawImage( djVisual *pVis, djImage *pImage, int x, int y, int w, int h );
//! Draw (w,h) pixels of an image at (xD,yD) from (xS,yS) on image
extern void       djgDrawImage( djVisual *pVis, djImage *pImage, int xS, int yS, int xD, int yD, int w, int h );
extern void       djgDrawImageStretchBlit( djVisual *pVis, djImage *pImage, int xS, int yS, int xD, int yD, int wS, int hS, int wD, int hD );
//! Draw (w,h) pixels of an image at (xD,yD) from (xS,yS) on image with transparency
extern void       djgDrawImageAlpha( djVisual *pVis, djImage *pImage, int xS, int yS, int xD, int yD, int w, int h );
//! Draw (w,h) pixels from a visual at (xD,yD) from (xS,yS)
extern void       djgDrawVisual( djVisual *pDest, djVisual *pSrc, int xD, int yD, int xS, int yS, int w, int h );
//! flip one visual to another
extern void       djgFlip( djVisual * pVisDest, djVisual * pVisSrc, bool bScaleView );


// DEBUG by rtfb
void djgDrawImage2( djVisual *v, djImage *img, int xS, int yS, int xD, int yD, int w, int h );


void SetPixelConversion ( djVisual *vis );


//! This class inherits from an SDL_rect. Its only purpose is to add a
//! constructor, for convenience.
class CdjRect : public SDL_Rect
{
public:
	CdjRect(int nX, int nY, int nW, int nH)
	{
		x = nX;
		y = nY;
		w = nW;
		h = nH;
	}
};


/*--------------------------------------------------------------------------*/
// Create corresponding hardware surface image (or return existing if already present)
//[dj2016-10] experimenting here with attempted HW surface cache, needs work [started as fixing wrong colors on Linux VNC]
extern void* djCreateImageHWSurface( djImage* pImage/*, djVisual* pVisDisplayBuffer*/ );

// ["dj2020 Adding that this returns a void* as a sort of 'handle'"] -> dj2022-11 NOTE Am now thinking why bother returning a void* I think that's just confusing now .. was trying to be 'generic' but at this point maybe can just as well return SDL_Surface* for clarity? unless there's some chance it might change drastically in a way that needs genericization (unlikely)
extern void djDestroyImageHWSurface( djImage* pImage );//dj2017-06-20 Added the 'destroy'
/*--------------------------------------------------------------------------*/
class djImageHardwareSurfaceCache
{
public:
	// Add image to cache and try create corresponding hardware surface (dj2022-11 this should maybe replace djCreateImageHWSurface global)
	static void* CreateImageHWSurface(djImage* pImage/*, djVisual* pVisDisplayBuffer*/);
	// Destroy and entirely remove this particular image and its corresponding hardware surface (if any) from the hardware image cache (dj2022-11 this should maybe replace djDestroyImageHWSurface)
	static void DestroyImageHWSurface(djImage* pImage);//dj2017-06-20 Added the 'destroy'

	// Clear all hardware surfaces but do NOT remove the image from the cache - we want to potentially recreate them with RecreateHardwareSurfaces [dj2022-11 toward possible fullscreen toggle etc.]
	static void ClearHardwareSurfaces();
	static void RecreateHardwareSurfaces();
};
/*--------------------------------------------------------------------------*/

#endif
