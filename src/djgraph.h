/*!
\file    djgraph.h
\brief   Graphics functionality
\author  David Joffe

Copyright (C) 1998-2001 David Joffe

License: GNU GPL Version 2 (*not* "later versions")
*/
/*--------------------------------------------------------------------------*/
#ifndef _DJGRAPH_H_
#define _DJGRAPH_H_

#include "SDL.h"
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
#ifdef USESDL
		pSurface = NULL;
		colorfore = djColor(255,255,255);
		colorback = djColor(0,0,0);
#endif
		width    = 0;
		height   = 0;
		stride   = 0;
		bpp      = 0;
		pixwidth = 0;
		m_bFullscreen = false;
	}
#ifdef USESDL
	SDL_Surface *pSurface;
	djColor                  colorfore;
	djColor                  colorback;
#endif
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
extern unsigned int djgMapColor( djVisual *pVis, djColor color );
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
extern void       djgSetColor( djVisual *pVis, djColor clrFore, djColor clrBack );
//! set foreground color
extern void       djgSetColorFore( djVisual * pVis, djColor color );
//! set background color
extern void       djgSetColorBack( djVisual * pVis, djColor color );

//-- Drawing primitives

//! clear a visual
extern void       djgClear( djVisual * pVis );
//! Put pixel
extern void       djgPutPixel( djVisual * pVis, int x, int y, int r, int g, int b );
//! Put pixel
extern void       djgPutPixel( djVisual * pVis, int x, int y, djColor color );
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
extern bool djCreateImageHWSurface( djImage* pImage/*, djVisual* pVisDisplayBuffer*/ );
/*--------------------------------------------------------------------------*/

#endif

