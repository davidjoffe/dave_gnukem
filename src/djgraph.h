/*!
\file    djgraph.h
\brief   Graphics functionality
\author  David Joffe

Copyright (C) 1998-2018 David Joffe
*/
/*--------------------------------------------------------------------------*/
#ifndef _DJGRAPH_H_
#define _DJGRAPH_H_

#ifdef __OS2__
#include <SDL/SDL.h>
#else
#include "SDL.h"
#endif
#include "mmgr/mmgr.h"

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



class djVisual
{
public:
	djVisual()
	{
		pSurface = NULL;
		colorfore = djColor(255,255,255);
		colorback = djColor(0,0,0);
		width    = 0;
		height   = 0;
		stride   = 0;
		bpp      = 0;
		pixwidth = 0;
		m_bFullscreen = false;
	}
	SDL_Surface *pSurface;
	djColor                  colorfore;
	djColor                  colorback;
	int                      width;
	int                      height;
	int                      stride;
	int                      bpp;         // 32 ... 24? (pixsize) pixdepth=24, pixsize=32 width=4
	int                      pixwidth;    // 4
	bool                     m_bFullscreen;
};

/*--------------------------------------------------------------------------*/
// Color management

//! Create a 32-bit unsigned int representing the given colour as it should be written to the given display surface.
extern unsigned int djgMapColor( djVisual *pVis, const djColor& color );
/*--------------------------------------------------------------------------*/
// Visual management

//! Create a visual
extern djVisual *djgOpenVisual( const char * vistype, int w, int h, int bpp = 0, bool bBackbuffer=false);
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
//[dj2016-10] experimenting here with attempted HW surface cache, needs work [started as fixing wrong colors on Linux VNC]
extern void* djCreateImageHWSurface( djImage* pImage/*, djVisual* pVisDisplayBuffer*/ );
extern void djDestroyImageHWSurface( djImage* pImage );//dj2017-06-20 Added the 'destroy'
/*--------------------------------------------------------------------------*/

#endif

