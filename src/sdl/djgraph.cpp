// SDL-based graphics routines
/*
djgraph.cpp

Copyright (C) 1997-2001 David Joffe

License: GNU GPL Version 2 (*not* "later versions")
*/


// The reason I can't use precompiled headers through stdafx.h seems
// to be some sort of stupid visual c++ compiler bug that makes:
//  #ifdef WIN32
//  #include "stdafx.h"
//  #endif
// not work.

#include "../djgraph.h"
#include "../sys_log.h"
#include "SDL.h"

#include <string.h>


int	rMask, gMask, bMask, aMask;
int	rShift, gShift, bShift, aShift;



unsigned int djgMapColor( djVisual * pVis, djColor color )
{
	unsigned int ret = 0;

	switch ( pVis->bpp )
	{
	case 16:
		// FIXME: assume 5-6-5 = wrong
// DEBUG: changed by rtfb

		ret = ((color.r/8)<<11) | ((color.g/4)<<5) | (color.b/8);
/*/
		ret = ((color.r/8)<<10) | ((color.g/4)<<5) | (color.b/8);
/**/
		break;
	case 24:
	case 32:
		ret = 0xFF000000 | (color.r<<16) | (color.g<<8) | color.b;
		break;
	}

	return ret;
}

djVisual* djgOpenVisual( const char *vistype, int w, int h, int bpp, bool bBackbuffer )
{
	// Create a djVisual
	djVisual * pVis;
	pVis = new djVisual;

	pVis->m_bFullscreen = false;

	// Create a default visual, just a plain non-resizing window
	static SDL_Surface *p = NULL;
	if (NULL == vistype)
	{
		p = pVis->pSurface = SDL_SetVideoMode(w, h, bpp, SDL_HWSURFACE|(bBackbuffer?SDL_DOUBLEBUF:0));
	}
	else if (0 == strcmp( vistype, "memory" ))
	{
		SDL_Surface *pSurface = SDL_CreateRGBSurface(SDL_HWSURFACE, w, h,
			bpp,
			0xFF000000,
			0x00FF0000,
			0x0000FF00,
			0x000000FF);
		if (pSurface!=NULL)
		{
			//SDL_SetColorKey(pSurface, SDL_SRCCOLORKEY|SDL_RLEACCEL, SDL_MapRGB(pSurface->format, 255, 0, 255));
			pVis->pSurface = SDL_DisplayFormat(pSurface);
			SDL_FreeSurface(pSurface);
		}
	}
	else if (0 == strcmp( vistype, "fullscreen" ))
	{
		pVis->pSurface = SDL_SetVideoMode(w, h, bpp, SDL_HWSURFACE|(bBackbuffer?SDL_DOUBLEBUF:0)|SDL_FULLSCREEN);
		pVis->m_bFullscreen = true;
	}

	pVis->bpp = pVis->pSurface->format->BitsPerPixel;
	pVis->width = w;
	pVis->height = h;
	pVis->stride = w * (bpp/8);
	switch (pVis->bpp)
	{
	case  8: pVis->pixwidth = 1; break;
	case 16: pVis->pixwidth = 2; break;
	case 24: pVis->pixwidth = 3; break; // FIXME: 4?
	case 32: pVis->pixwidth = 4; break;
	}

	return pVis;
}

void djgCloseVisual( djVisual * pVis )
{
	SDL_FreeSurface(pVis->pSurface);
}

void djgLock( djVisual * pVis )
{
	SDL_LockSurface(pVis->pSurface);
}

void djgUnlock( djVisual * pVis )
{
	SDL_UnlockSurface(pVis->pSurface);
}


void djgFlush( djVisual * /*pVis*/)
{
	//pVis = pVis;		// shut up the "unused" warning
}

void djgFlip( djVisual * pVisDest, djVisual * pVisSrc )
{
	if (pVisSrc==NULL)
		SDL_Flip(pVisDest->pSurface);
	else
	{
		CdjRect rcSrc(0, 0, 320, 200);
		CdjRect rcDest(0, 0, 320, 200);
		SDL_BlitSurface(pVisSrc->pSurface, &rcSrc, pVisDest->pSurface, &rcDest);
		SDL_Flip(pVisDest->pSurface);
	}
}

void djgClear( djVisual * pVis )
{
	SDL_Rect rc;
	rc.x = 0;
	rc.y = 0;
	rc.w = pVis->width;
	rc.h = pVis->height;
	SDL_FillRect(pVis->pSurface, &rc, djgMapColor(pVis, djColor(0,0,0)));
}

void djgPutPixel( djVisual * pVis, int x, int y, int r, int g, int b )
{
	djColor       col;
	unsigned int pixel;
	col = djColor( r, g, b );
	pixel = djgMapColor( pVis, col );
	SDL_Rect rc;
	rc.x = x;
	rc.y = y;
	rc.w = 1;
	rc.h = 1;
	SDL_FillRect(pVis->pSurface, &rc, pixel);
}

void djgPutPixel( djVisual * pVis, int x, int y, djColor color )
{
	djgPutPixel(pVis, x, y, color.r, color.g, color.b);
}

void djgSetColorFore( djVisual * pVis, djColor color )
{
	pVis->colorfore.r = color.r;
	pVis->colorfore.g = color.g;
	pVis->colorfore.b = color.b;
}

void djgSetColor( djVisual *pVis, djColor clrFore, djColor clrBack )
{
	djgSetColorFore( pVis, clrFore );
	djgSetColorBack( pVis, clrBack );
}

void djgSetColorBack( djVisual * pVis, djColor color )
{
	pVis->colorback.r = color.r;
	pVis->colorback.g = color.g;
	pVis->colorback.b = color.b;
}

void djgDrawRectangle( djVisual * pVis, int x, int y, int w, int h )
{
	// FIXME: TEST THESE BOUNDS CHECKING
//	if (x<0) x=0;
//	if (y<0) y=0;
//	if (x+w>=pVis->width+1) w = pVis->width - x;
//	if (y+h>=pVis->height+1) h = pVis->height - y;
	djgDrawHLine( pVis, x  , y  , w );
	djgDrawHLine( pVis, x  , y+h-1, w );
	djgDrawVLine( pVis, x  , y  , h );
	djgDrawVLine( pVis, x+w-1, y  , h );
}

void djgDrawBox( djVisual * pVis, int x, int y, int w, int h )
{
	unsigned int pixel = djgMapColor( pVis, pVis->colorfore );
	CdjRect rc(x, y, w, h);
	SDL_FillRect(pVis->pSurface, &rc, pixel);
}

void djgDrawHLine( djVisual * pVis, int x, int y, int n )
{
	if (y<0 || y>=pVis->height) return;
	if (x<0) { n=n+x; if (n<=0) return; x=0; }
	if (x+n>pVis->width) { n=pVis->width-x; }
	if (n<=0) return;

	unsigned int pixel = djgMapColor( pVis, pVis->colorfore );

	djgLock( pVis );
	for ( int i=0; i<n; i++ )
		memcpy( (char*)pVis->pSurface->pixels + y*pVis->stride + (x+i)*pVis->pixwidth, &pixel, pVis->pixwidth );
	djgUnlock( pVis );

}

void djgDrawVLine( djVisual * pVis, int x, int y, int n )
{
	if (x<0 || x>=pVis->width) return;
	if (y<0) { n=n+y; if (n<=0) return; y=0; }
	if (y+n>pVis->height) { n=pVis->height-y; }
	if (n<=0) return;

	unsigned int pixel = djgMapColor( pVis, pVis->colorfore );

	djgLock( pVis );
	for ( int i=0; i<n; i++ )
		memcpy( (char*)pVis->pSurface->pixels + (y+i)*pVis->stride + x*pVis->pixwidth, &pixel, pVis->pixwidth );
	djgUnlock( pVis );

}

void djgDrawImage( djVisual *pVis, djImage *pImage, int x, int y, int w, int h )
{
	djgDrawImage( pVis, pImage, 0, 0, x, y, w, h );
}

void djgDrawImage( djVisual *pVis, djImage *pImage, int xS, int yS, int xD, int yD, int w, int h )
{
	if (pImage==NULL) return;

	if (xS<0 || yS<0) return;

	// clipping
	if (xD>=pVis->width || yD>=pVis->height) return;
	if (xD+w>pVis->width)  { w = pVis->width-xD; }
	if (yD+h>pVis->height) { h = pVis->height-yD; }
	if (xD<0) { w=w+xD; if (w<=0) return; xD=0; }
	if (yD<0) { h=h+yD; if (h<=0) return; yD=0; }

	// If same pixel formats, just copy
	if (pVis->bpp==pImage->BPP())
	{
		djgLock(pVis);

		for ( int i=0; i<h; i++ )
		{
			memcpy(
				(char*)pVis->pSurface->pixels + (yD+i)*pVis->stride + xD*pVis->pixwidth,
				pImage->Data() + (yS+i)*pImage->Pitch()+xS*pVis->pixwidth,
				w * pVis->pixwidth );
		}

		djgUnlock(pVis);

		return;
	}

	// 32-bit, no transparency
	if (pVis->bpp==32)
	{
		if (pImage->BPP()==16)
		{
		}
	}
	if (pVis->bpp==16)
	{
		// 32-bit, no transparency
		if (pImage->BPP()==32)
		{
			djgLock( pVis );
			for ( int i=0; i<h; i++ )
			{
				int iOffsetSrc = (yS+i)*pImage->Pitch() + xS*4;
				int iOffsetDest = (yD+i)*pVis->stride + xD*2;
				unsigned int   *pSrc = (unsigned int*)((char*)pImage->Data() + iOffsetSrc);
				unsigned short *pDest = (unsigned short*)((char*)pVis->pSurface->pixels + iOffsetDest);
				for ( int j=0; j<w; j++ )
				{
					*pDest = PIXEL32TO16(*pSrc);
					pSrc++;
					pDest++;
				}
			}
			djgUnlock( pVis );
			return;
		}
	}
}




void djgDrawImage2( djVisual *v, djImage *img, int xS, int yS, int xD, int yD, int w, int h )
{
	djgLock( v );
	for ( int i=0; i<h; i++ )
	{
		int iOffsetSrc = (yS+i)*img->Pitch() + xS*4;
		int iOffsetDest = (yD+i)*v->stride + xD*2;
		unsigned int   *src = (unsigned int*)((char*)img->Data() + iOffsetSrc);
		unsigned short *dst = (unsigned short*)((char*)v->pSurface->pixels + iOffsetDest);
		for ( int j=0; j<w; j++ )
		{
			*dst = PIXEL32TO16(*src);
			src++;
			dst++;
		}
	}
/**/
	djgUnlock( v );
	return;
}




void djgDrawImageAlpha( djVisual *pVis, djImage *pImage, int xS, int yS, int xD, int yD, int w, int h )
{
	if (pImage==NULL) return;

	// clipping
	if (xD>=pVis->width || yD>=pVis->height) return;
	if (xD+w>pVis->width)  { w = pVis->width-xD; }
	if (yD+h>pVis->height) { h = pVis->height-yD; }
	if (xD<0) { w=w+xD; if (w<=0) return; xD=0; }
	if (yD<0) { h=h+yD; if (h<=0) return; yD=0; }

	// 32-bit, alpha map
	if (pVis->bpp==32)
	{
		if (pImage->BPP()==32)
		{
			djgLock( pVis );

			for ( int i=0; i<h; i++ )
			{
				int iOffsetSrc = (yS+i)*pImage->Pitch() + xS*4;
				int iOffsetDest = (yD+i)*pVis->stride + xD*4;
				unsigned int *pSrc = (unsigned int*)((char*)pImage->Data() + iOffsetSrc);
				unsigned int *pDest = (unsigned int*)((char*)pVis->pSurface->pixels + iOffsetDest);
				for ( int j=0; j<w; j++ )
				{
					unsigned int pixel = *pSrc;
					// If alpha value non-zero
					if (pixel & 0xFF000000)
					{
						*pDest = pixel;
					}
					pSrc++;
					pDest++;
				}
			}

			djgUnlock( pVis );
			return;
		}
	}
	if (pVis->bpp==16)
	{
		if (pImage->BPP()==32)
		{
			djgLock( pVis );

			for ( int i=0; i<h; i++ )
			{
				int iOffsetSrc = (yS+i)*pImage->Pitch() + xS*4;
				int iOffsetDest = (yD+i)*pVis->stride + xD*2;
				unsigned int   *pSrc = (unsigned int*)((char*)pImage->Data() + iOffsetSrc);
				unsigned short *pDest = (unsigned short*)((char*)pVis->pSurface->pixels + iOffsetDest);
				for ( int j=0; j<w; j++ )
				{
					unsigned int pixel = *pSrc;
					// If alpha value non-zero
					if (pixel & 0xFF000000)
					{
						*pDest = PIXEL32TO16(pixel);
					}
					pSrc++;
					pDest++;
				}
			}

			djgUnlock( pVis );
			return;
		}
	}
}

void djgDrawVisual( djVisual *pDest, djVisual *pSrc, int xD, int yD, int xS, int yS, int w, int h )
{
	CdjRect rcSrc(xS, yS, w, h );
	CdjRect rcDest(xD, yD, w, h );

	SDL_BlitSurface(pSrc->pSurface, &rcSrc, pDest->pSurface, &rcDest);
}

void SetPixelConversion ( djVisual *vis )
{
	SDL_PixelFormat *f = vis->pSurface->format;

	Log ( "Setting up pixel conversion attributes...:\n" );
	rMask = f->Rmask;
	gMask = f->Gmask;
	bMask = f->Bmask;
	aMask = f->Amask;
	rShift = f-> Rshift;
	gShift = f-> Gshift;
	bShift = f-> Bshift;
	aShift = f-> Ashift;

	Log ( "\t[RGBA]Mask: 0x%x; 0x%x; 0x%x 0x%x\n", rMask, gMask, bMask, aMask );
	Log ( "\t[RGBA]Shift: 0x%x; 0x%x; 0x%x 0x%x\n", rShift, gShift, bShift, aShift );
}
