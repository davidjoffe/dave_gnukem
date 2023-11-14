// SDL-based graphics routines
/*
djgraph.cpp

Copyright (C) 1997-2023 David Joffe
*/


// The reason I can't use precompiled headers through stdafx.h seems
// to be some sort of stupid visual c++ compiler bug that makes:
//  #ifdef WIN32
//  #include "stdafx.h"
//  #endif
// not work.

#include "../config.h"//[For CFG_APPLICATION_RENDER_RES_W etc. dj2019-06 slightly ugly dependency direction, conceptually, but not the biggest thing in the world to worry about now, maybe later.]
#include "../djgraph.h"
#include "../djtypes.h"
#include "../sys_log.h"
#ifdef __OS2__
#include <SDL/SDL.h>
#else
#include "SDL.h"
#endif

#include <cstdint>//for uint32_t

#include <string.h>

// [dj2016-10] For 'texture' manager'
#include <map>

#ifdef _DEBUG
#include <cassert>//assert
#endif

// dj2022-12 one small additional note on the below whether unordered_map is faster also may I think depend on factors like size? best may be to actually check and do speed tests BUT also maybwe re-think whether we can just do away with the map entirely rather, there are some possible ways to solve that.
// However, it might not be worth doing because this game has relatively few images .. and premature optimization as they say is the root of all evil - we should do profile-driven etc.
// And also if I uncap the framerate we're at about 240fps on my dev machine which while isn't some incredible record it's seemingly pretty fast arleady.
// however design if we create a struct like say with an image and surface (which may be null or may change etc.) .. hmm think about

//fixme[dj2020] low priority, should ideally be sped up:
// 1. A map is not really the most efficient way to do this as it must do a lookup for every blit
// 2. std::map is probably not the fastest map for this either .. unordered_map may be (we don't need correct sorting and we're happy with slower inserts for faster lookups)
// The more correct way requires some re-coding all over the codebase - return some kind of 'handle' here which could be a void* to either a struct with a pointer to the SDL_surface or the SDL_surface pointer as a void* or something ... then each 'user' must keep this 'texture handle' for when it does blits. Then, no map lookup.
// I started adding that so it can be used already if desired [dj2020-07]
// On modern hardware for a small 320x200 game this is probably not the worst bottleneck in the code ... if we wanted a more generic engine with much more intense graphics it may become worth it to optimize here.
std::map< djImage*, SDL_Surface *> g_SurfaceMap;

int	rMask=0, gMask=0, bMask=0, aMask=0;
int	rShift=0, gShift=0, bShift=0, aShift=0;

// 0=default
// 1=EGA (Enhanced Graphics Adapter - simulate retro old 16 color display mode)
// 2=CGA (simulated retro)
int g_nSimulatedGraphics = 0;

// CGA palette 1 (4 colors) cyan magenta black white
const djColor djPALETTE_CGA[4] = {
	djColor(0,0,0),//black
	djColor(0x55,0xFF,0xFF),//bright cyan
	djColor(0xFF,0x55,0xFF),//bright magenta
	djColor(0xFF,0xFF,0xFF)//bright white
	// DITHERTEST
	//djColor(0x55,0x55,0x55)//dark grey as white/black dither
};
// EGA standard 16-colors
const djColor djPALETTE_EGA[16] = {
	djColor(0,0,0),//black
	djColor(0,0,0xAA),//blue
	djColor(0,0xAA,0),//green
	djColor(0,0xAA,0xAA),//cyan
	djColor(0xAA,0,0),//red
	djColor(0xAA,0,0xAA),//magenta
	djColor(0xAA,0x55,0),//brown
	djColor(0xAA,0xAA,0xAA),//light grey
	djColor(0x55,0x55,0x55),//dark grey
	djColor(0x55,0x55,0xFF),//bright blue
	djColor(0x55,0xFF,0x55),//bright green
	djColor(0x55,0xFF,0xFF),//bright cyan
	djColor(0xFF,0x55,0x55),//bright red
	djColor(0xFF,0x55,0xFF),//bright magenta
	djColor(0xFF,0xFF,0x55),//bright yellow
	djColor(0xFF,0xFF,0xFF)//bright white
};

djVisual* djgOpenVisual( const char *vistype, int w, int h, int bpp, bool bBackbuffer, const char* szWindowTitle, const char* szWindowIconFile)
{
	// Create a djVisual
	djVisual * pVis;
	pVis = new djVisual;

	pVis->m_bFullscreen = NULL != vistype && 0 == strcmp ( vistype, "fullscreen" ) ? true : false;


	//dj2016-10 fixmelow_tocheck i am wondering if the below is doing speed optimal way of doing everything as this hasn't been looked at in years

	// Create a default visual: a resizable window or fullscreen
	//static SDL_Surface *p = NULL;
	if (NULL == vistype || pVis->m_bFullscreen)
	{
		// dj2022-11 slightly changing Matteo Bini's recent SDL2 code changes here to pass in the window title and bitmap as parameters (so this codebase could be more generically used for other games)
		SDL_Window* win = SDL_CreateWindow(szWindowTitle == nullptr ? "Window" : szWindowTitle, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, w, h,
			pVis->m_bFullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : SDL_WINDOW_RESIZABLE);
		pVis->pWindow = win;//<- dj2022-11 added storing this pWindow pointer so we can cleanup with corresponding SDL_DestroyWindow (for in-game fullscreen toggle etc.)
		if (szWindowIconFile != nullptr)
			SDL_SetWindowIcon(win, SDL_LoadBMP(szWindowIconFile));
		pVis->pRenderer = SDL_CreateRenderer(win, -1, 0);
		SDL_RenderSetLogicalSize(pVis->pRenderer, CFG_APPLICATION_RENDER_RES_W, CFG_APPLICATION_RENDER_RES_H);
		SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");
		pVis->pSurface = SDL_CreateRGBSurface(0, CFG_APPLICATION_RENDER_RES_W, CFG_APPLICATION_RENDER_RES_H, bpp,
			0, 0, 0, 0);
		pVis->pTexture = SDL_CreateTextureFromSurface(pVis->pRenderer, pVis->pSurface);
		// to check/merge emsdk-build changes ->
		//// 			pVis->pSurface = SDL_DisplayFormat(pSurface); =>
		//			pVis->pSurface = SDL_DisplayFormatAlpha(pSurface);

		pVis->width = w;
		pVis->height = h;
		pVis->stride = w * (bpp/8);
	}
	else if (0 == strcmp( vistype, "memory" ))
	{
		// [dj2022-11] Small dev note here: In previous SDL1 version this used to create at size w,h so there's a slight behaviour change here with SDL2 version, so e.g.
		// previously if we were at say 'scale 2' then the game base resolution is e.g. 320x200 but w,h would be e.g. 640x400 at scale 2 (and so on for higher scale values) -
		// .. the below by Matto Bini seems probably more 'correct' (and probably better performance in some aspects) but just adding a correcton here to also set
		// "pVis->width = CFG_APPLICATION_RENDER_RES_W" etc. otherwise the actual pSurface resolution (320x200) doesn't match what pVis reports which cause menu rendering funnies and possible crasshes this should fix.
		pVis->pSurface = SDL_CreateRGBSurface(0, CFG_APPLICATION_RENDER_RES_W, CFG_APPLICATION_RENDER_RES_H, bpp,
			0, 0, 0, 0);
		pVis->width = CFG_APPLICATION_RENDER_RES_W;
		pVis->height = CFG_APPLICATION_RENDER_RES_H;
		pVis->stride = CFG_APPLICATION_RENDER_RES_W * (bpp/8);
	}

	pVis->bpp = pVis->pSurface->format->BitsPerPixel;
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
	pVis->pSurface = NULL;//?<- dj2017-01 not sure if this is all that's necessary for cleanup here ...

	//dj2022-11 adding this SDL_DestroyWindow here as I think it may help fix the in-game fullscreen toggle issues
	// QUESTION[dj2022-11] must we also destroy pRenderer manually here to double check
	if (pVis->pWindow)
	{
		SDL_DestroyWindow(pVis->pWindow);
		pVis->pWindow = nullptr;
	}
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

void djgFlip( djVisual * pVisDest, djVisual * pVisSrc, bool bScaleView )
{
	if (pVisSrc!=NULL)//<- Not level editor etc.
	{
		CdjRect rcSrc(0, 0, CFG_APPLICATION_RENDER_RES_W, CFG_APPLICATION_RENDER_RES_H);//E.g. 320x200 for DG1
		CdjRect rcDest(0, 0, CFG_APPLICATION_RENDER_RES_W, CFG_APPLICATION_RENDER_RES_H);
#if __cplusplus>=202002L // c++20?
		SDL_Rect rc = { .w = 1, .h = 1 };
#else
		SDL_Rect rc;
		rc.w = 1;
		rc.h = 1;
#endif

		//fixme this won't work bigendian
		#if SDL_BYTEORDER==SDL_BIG_ENDIAN
		// Not yet supported for big-endian platforms (dj2019-06)
		if (false);
		#else
		if (g_nSimulatedGraphics>0) //'Simulate' CGA/EGA
		{
			djgLock(pVisSrc);
			djgLock(pVisDest);
			unsigned int uBPP = pVisSrc->pSurface->format->BytesPerPixel;
			unsigned int *pSurfaceMem = (unsigned int*)pVisSrc->pSurface->pixels;
			unsigned int uMemOffsetRow = 0;


				// Select target simulated-graphics palette
				const unsigned int NUMCOLORS = (g_nSimulatedGraphics==1?16:4);
				const djColor* pPalette = (g_nSimulatedGraphics==1 ? djPALETTE_EGA : djPALETTE_CGA);
				
				// [dj2022-01] Removing "register" hint here on these four variables as appears to cause issues with c++17 on arch e.g. see https://github.com/davidjoffe/dave_gnukem/issues/132
				// "register" is not important anyway here - it's just a hint for compiler optimization and the compiler usually does a decent job, this codepath's for an unimportant feature (pseudo fake retro display modes which will be hardly used and our resolution usually low, I doubt its removal will make a material difference to anyone's lives, if it does cause bottlenecks someday we can revisit this)
				int nPixel;
				// For finding closest-matching pixel in target simulated mode palette
				int nDistance=0;
				int nDistanceMin=-1;
				int nClosest = 0;//black
				
				rc.y=0;
				for ( unsigned int y=0; y<CFG_APPLICATION_RENDER_RES_H; ++y )
				{
					uMemOffsetRow = (y * (pVisSrc->pSurface->pitch/uBPP));
					pSurfaceMem = ((unsigned int*)pVisSrc->pSurface->pixels) + uMemOffsetRow;
					// Note we must be careful here, pVisSrc->pSurface->pitch is in *bytes*, pSurfaceMem is a pointer to unsigned int* so pointer 'math' in multiples of 4
					rc.x = 0;
					for ( unsigned int x=0; x<CFG_APPLICATION_RENDER_RES_W; ++x )
					{
						// Getpixel color from source
						nPixel = *pSurfaceMem;
						djColor Color(
							(nPixel & 0xFF0000) >> 16,//red
							(nPixel & 0xFF00) >> 8,//green
							(nPixel & 0xFF));//blue

						// Look for closest approximate match in simulated palette
						nDistance=0;
						nDistanceMin=-1;
						nClosest = 0;//black
						for (unsigned int z=0;z<NUMCOLORS;++z)
						{
							nDistance =
								(int)djABS((int)Color.r - (int)(pPalette[z]).r) +
								(int)djABS((int)Color.g - (int)(pPalette[z]).g) +
								(int)djABS((int)Color.b - (int)(pPalette[z]).b);
							if (nDistanceMin==-1 || nDistance<nDistanceMin)
							{
								nDistanceMin = nDistance;
								nClosest = z;
							}
						}
						//cga dark gray
						/*
						if (g_nSimulatedGraphics==2 && nClosest==4)
						{
							if ((x%2) ^ (y%2))
								nClosest = 0;//dither alternating black
							else
								nClosest = 3;//dither alternating white
						}
						*/

						nPixel = SDL_MapRGB(pVisDest->pSurface->format,pPalette[nClosest].r,pPalette[nClosest].g,pPalette[nClosest].b);
						SDL_FillRect(pVisDest->pSurface, &rc, nPixel);
						++pSurfaceMem;
						++rc.x;
					}//x
					++rc.y;
				}//y
			djgUnlock(pVisDest);
			djgUnlock(pVisSrc);
		}
		#endif
		else
		{
			CdjRect rcSrc(0, 0, pVisSrc->width, pVisSrc->height);
			//Non-scaling blit [faster, but can only do if src same size as dest]
			SDL_BlitSurface(pVisSrc->pSurface, &rcSrc, pVisDest->pSurface, &rcDest);
		}
	}
	SDL_UpdateTexture(pVisDest->pTexture, NULL, pVisDest->pSurface->pixels, pVisDest->pSurface->pitch);
	SDL_RenderClear(pVisDest->pRenderer);
	SDL_RenderCopy(pVisDest->pRenderer, pVisDest->pTexture, NULL, NULL);
	SDL_RenderPresent(pVisDest->pRenderer);
}

void djgClear( djVisual * pVis )
{
	SDL_Rect rc;
	rc.x = 0;
	rc.y = 0;
	rc.w = pVis->width;
	rc.h = pVis->height;
	SDL_FillRect(pVis->pSurface, &rc, SDL_MapRGB(pVis->pSurface->format, 0, 0, 0));//black
}

void djgPutPixel( djVisual * pVis, int x, int y, int r, int g, int b )
{
	const uint32_t pixel = SDL_MapRGB(pVis->pSurface->format, r, g, b);
	SDL_Rect rc;
	rc.x = x;
	rc.y = y;
	rc.w = 1;
	rc.h = 1;
	SDL_FillRect(pVis->pSurface, &rc, pixel);
}

//[low prio] these types of functions could/should ideally be sped up with inlining? [dj2022-11] [and/or avoid for sensitive performance bottleneck areas]
void djgPutPixel( djVisual * pVis, int x, int y, const djColor& color )
{
	djgPutPixel(pVis, x, y, color.r, color.g, color.b);
}

void djgSetColorFore( djVisual * pVis, const djColor& color )
{
	pVis->colorfore.r = color.r;
	pVis->colorfore.g = color.g;
	pVis->colorfore.b = color.b;
}

void djgSetColor( djVisual *pVis, const djColor& clrFore, const djColor& clrBack )
{
	djgSetColorFore( pVis, clrFore );
	djgSetColorBack( pVis, clrBack );
}

void djgSetColorBack( djVisual * pVis, const djColor& color )
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
	const uint32_t pixel = SDL_MapRGB(pVis->pSurface->format, pVis->colorfore.r, pVis->colorfore.g, pVis->colorfore.b);
	CdjRect rc(x, y, w, h);
	SDL_FillRect(pVis->pSurface, &rc, pixel);
}

void djgDrawHLine( djVisual * pVis, int x, int y, int n )
{
	if (y<0 || y>=pVis->height) return;
	if (x<0) { n=n+x; if (n<=0) return; x=0; }
	if (x+n>pVis->width) { n=pVis->width-x; }
	if (n<=0) return;

	const uint32_t pixel = SDL_MapRGB(pVis->pSurface->format, pVis->colorfore.r, pVis->colorfore.g, pVis->colorfore.b);

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

	const uint32_t pixel = SDL_MapRGB(pVis->pSurface->format, pVis->colorfore.r, pVis->colorfore.g, pVis->colorfore.b);

	djgLock( pVis );
	for ( int i=0; i<n; i++ )
		memcpy( (char*)pVis->pSurface->pixels + (y+i)*pVis->stride + x*pVis->pixwidth, &pixel, pVis->pixwidth );
	djgUnlock( pVis );

}

void djgDrawImage( djVisual *pVis, djImage *pImage, int x, int y, int w, int h )
{
	djgDrawImage( pVis, pImage, 0, 0, x, y, w, h );
}

 void djgDrawImageStretchBlit( djVisual *pVis, djImage *pImage, int xS, int yS, int xD, int yD, int wS, int hS, int wD, int hD )
 {
	if (pImage==NULL) return;

	if (xS<0 || yS<0) return;

	// IF HARDWARE SURFACE FOR THIS IMAGE, USE IT
	SDL_Surface* pHWSurface = NULL;
	std::map< djImage*, SDL_Surface * >::const_iterator iter = g_SurfaceMap.find( pImage );
	if (iter != g_SurfaceMap.end())
	{
		pHWSurface = iter->second;
	}
	if (pHWSurface)
	{
		//BLIT
		SDL_Rect rectSrc;
		rectSrc.x = xS;
		rectSrc.y = yS;
		rectSrc.w = wS;
		rectSrc.h = hS;
		SDL_Rect rectDest;
		rectDest.x = xD;
		rectDest.y = yD;
		rectDest.w = wD;
		rectDest.h = hD;
		// if dimensions are the same shouldn't use scaled blit for speed ..
		if (wS==wD && hS==hD)
			SDL_BlitSurface(pHWSurface, &rectSrc, pVis->pSurface, &rectDest);
		else
			SDL_BlitScaled(pHWSurface, &rectSrc, pVis->pSurface, &rectDest);
		return;
	}


 }
void djgDrawImage( djVisual *pVis, djImage *pImage, int xS, int yS, int xD, int yD, int w, int h )
{
	if (pImage==NULL) return;

	if (xS<0 || yS<0) return;


	// IF HARDWARE SURFACE FOR THIS IMAGE, USE IT
	SDL_Surface* pHWSurface = NULL;
	std::map< djImage*, SDL_Surface * >::const_iterator iter = g_SurfaceMap.find( pImage );
	if (iter != g_SurfaceMap.end())
	{
		pHWSurface = iter->second;
	}
	if (pHWSurface)
	{
		//BLIT
		SDL_Rect rectSrc;
		rectSrc.x = xS;
		rectSrc.y = yS;
		rectSrc.w = w;
		rectSrc.h = h;
		SDL_Rect rectDest;
		rectDest.x = xD;
		rectDest.y = yD;
		rectDest.w = w;
		rectDest.h = h;
		SDL_BlitSurface(pHWSurface, &rectSrc, pVis->pSurface, &rectDest);
		return;
	}



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

	// IF HARDWARE SURFACE FOR THIS IMAGE, USE IT
	SDL_Surface* pHWSurface = NULL;
	std::map< djImage*, SDL_Surface * >::const_iterator iter = g_SurfaceMap.find( img );
	if (iter != g_SurfaceMap.end())
	{
		pHWSurface = iter->second;
	}
	if (pHWSurface)
	{
		//BLIT
		SDL_Rect rectSrc;
		rectSrc.x = xS;
		rectSrc.y = yS;
		rectSrc.w = w;
		rectSrc.h = h;
		SDL_Rect rectDest;
		rectDest.x = xD;
		rectDest.y = yD;
		rectDest.w = w;
		rectDest.h = h;
		SDL_BlitSurface(pHWSurface, &rectSrc, v->pSurface, &rectDest);
		return;
	}

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

	// IF HARDWARE SURFACE FOR THIS IMAGE, USE IT
	SDL_Surface* pHWSurface = NULL;
	std::map< djImage*, SDL_Surface * >::const_iterator iter = g_SurfaceMap.find( pImage );
	if (iter != g_SurfaceMap.end())
	{
		pHWSurface = iter->second;
	}
	if (pHWSurface)
	{
		//BLIT
		SDL_Rect rectSrc;
		rectSrc.x = xS;
		rectSrc.y = yS;
		rectSrc.w = w;
		rectSrc.h = h;
		SDL_Rect rectDest;
		rectDest.x = xD;
		rectDest.y = yD;
		rectDest.w = w;
		rectDest.h = h;
		SDL_BlitSurface(pHWSurface, &rectSrc, pVis->pSurface, &rectDest);
		return;
	}
				//dj2018-01 in theory the code from this point below should 'never'
				// activate anymore? I think? Or are there platforms where it might?
	// i.e. ideally we just want to stick to doing hardware blits (or "proper correct" software fall-back in worst case - not our crappy software blit, basically :D)
				//fixmeMedium
	/* comment comment dj2018-01-13
	djgDrawImageAlpha: Tentatively remove 'fallback' old sprite drawing code - in theory this could break some platforms if the new g_SurfaceMap stuff fails - if that happens I think we should discover that through testing, and handle it on ad hoc basis - e.g. either add the fallback code back (not ideal, for multiple reasons, including speed, and lack of proper support for semi-transparent alpha, possible lack of support for all pixel formats etc.), or figure out why the new g_SurfaceMap stuff isn't working on whatever new platform, and fix it (more ideal).
	*/
				return;

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

	djLOGSTR( "Setting up pixel conversion attributes...:\n" );
	rMask = f->Rmask;
	gMask = f->Gmask;
	bMask = f->Bmask;
	aMask = f->Amask;
	rShift = f-> Rshift;
	gShift = f-> Gshift;
	bShift = f-> Bshift;
	aShift = f-> Ashift;

	djLog::LogFormatStr( "\t[RGBA]Mask: 0x%x; 0x%x; 0x%x 0x%x\n", (int)rMask, (int)gMask, (int)bMask, (int)aMask );
	djLog::LogFormatStr( "\t[RGBA]Shift: 0x%x; 0x%x; 0x%x 0x%x\n", (int)rShift, (int)gShift, (int)bShift, (int)aShift );
}


//----------------------------
void* djImageHardwareSurfaceCache::CreateImageHWSurface(djImage* pImage/*, djVisual* pVisDisplayBuffer*/)
{
	return djCreateImageHWSurface(pImage);
}
void djImageHardwareSurfaceCache::DestroyImageHWSurface(djImage* pImage)
{
	djDestroyImageHWSurface(pImage);
}
void djImageHardwareSurfaceCache::ClearHardwareSurfaces()
{
	for (auto& iter : g_SurfaceMap)
	{
		djImage* pImage = iter.first;
		// Destroy the hardware surface but DON'T remove the image from cache!
		SDL_Surface* pHardwareSurface = iter.second;
		if (pHardwareSurface)
		{
			SDL_FreeSurface(pHardwareSurface);
			// fixme check if the above leaks?
			g_SurfaceMap[pImage] = nullptr;
		}
	}
}
void djImageHardwareSurfaceCache::RecreateHardwareSurfaces()
{
	for (auto& iter : g_SurfaceMap)
	{
		djImage* pImage = iter.first;
		djCreateImageHWSurface(pImage);
	}
}
//----------------------------
void djDestroyImageHWSurface( djImage* pImage )
{
	if (pImage==NULL) return;

	std::map< djImage*, SDL_Surface *>::const_iterator iter = g_SurfaceMap.find( pImage );
	if (iter == g_SurfaceMap.end())
		return; // Hardware texture for this image not found

	// Delete the associated hardware surface
	SDL_Surface* pHardwareSurface = iter->second;
	if (pHardwareSurface)
	{
		SDL_FreeSurface(pHardwareSurface);
		//? ?delete pHardwareSurface;
		pHardwareSurface = NULL;
	}

	// Remove from 'map'
	g_SurfaceMap.erase( pImage );
}
// Create corresponding hardware surface image (or return existing if already present)
// dj2020 Adding that this returns a void* as a sort of 'handle', which is in fact the SDL_Surface*, so it can be used as such which is faster than the map, see comments at top of file .. later this might return SDL_Surface* or some sort of 'handle' or struct that includes the SDL_Surface*.
// dj2022 Why bother returning a void* I think that's just confusing now .. was trying to be 'generic' but at this point maybe can just as well return SDL_Surface* for clarity? unless there's some chance it might change drastically in a way that needs genericization (unlikely)
void* djCreateImageHWSurface( djImage* pImage/*, djVisual* pVisDisplayBuffer*/ )
{
	//extern djVisual* pVisView;
	//djVisual* pVisDisplayBuffer = pVisView;
	if (pImage==NULL) return nullptr;//false;

	SDL_Surface* pSurfaceHardware = nullptr;

	// [dj2022-11] Here we check if graphics 'system' not actually initialized and if not just add an item for the image but set the surface to null ... during GraphInit we could just hypothetically instantiate hardware surfaces for any not present? This code also helps facilitate fullscreen toggle
	// fixme low to check is it sufficient for SDL_init to be called or do we need the GraphInit to be done? not sure [dj2022-11]
	//if (!djGraphicsSystem::IsInitialized())
		//return (void*)pSurfaceHardware;

	// Check if already have a hardware surface in the cache for the requested image and return it if we do [dj2022-11 part of changes to help with potential fullscreen toggle ability]
	const std::map< djImage*, SDL_Surface*>::const_iterator iter = g_SurfaceMap.find(pImage);
	if (iter != g_SurfaceMap.end())
	{
		pSurfaceHardware = iter->second;
		if (pSurfaceHardware!=nullptr)
			return (void*)pSurfaceHardware;
	}

	//fixme to check are these actually hardware surfaces

	/*
	dj2019-06: There may still be something else wrong here re SDL_BIG_ENDIAN patch for issue 100 below.
	macppc OpenBSD users reported having to seemingly (as I interpret it) BeWorld2018 fix for issue 100,
	to get it working on OpenBSD macppc, or they get blank screen ... see here for more info:

	http://openbsd-archive.7691.n7.nabble.com/NEW-games-gnukem-td365426.html

	NB: (If I'm reading that Raphael Graf patch correctly, I can't incorporate that OpenBSD macppc patch 'blindly'
	here without breaking it again on MorphOS - so this needs further investigation as to what exactly is going on,
	and the most 'correct' way to fix it. If macpcc has SDL_BYTEORDER==SDL_BIG_ENDIAN then why the blank screen,
	why must we pass little-endian bitmasks, could it be something like graphics hardware byte order different from
	CPU byte order? Not sure. Or could it be an issue with libsdl1.2 on macppc itself (less likely).
	Unfortunately I cannot test on PPC MorphOS which makes it difficult to just try things out as I have no way to see
	how they impact that configuration.
	Or could it be something to do with how we load images into RAM.
	See also:
	https://github.com/davidjoffe/dave_gnukem/issues/100
	https://wiki.libsdl.org/CategoryEndian
	https://wiki.libsdl.org/SDL_CreateRGBSurface
	*/

	// dj2023-02 our loaded PNGs may ahve different byte order it seems .. this now all needs more testing .. either we must convert image files on load,
	// or generically handle more cases here .. or both I suppose, to help make sure this stuff works on all platforms etc.
	#ifdef djUSE_SDLIMAGE
	//pImage->
	pSurfaceHardware = ::SDL_CreateRGBSurfaceFrom(
		pImage->Data(),
		pImage->Width(),
		pImage->Height(),
		pImage->BPP(),
		pImage->Pitch(),
		// R,G,B,A masks (dj2018-03 specify different masks here on PPC etc. - see https://github.com/davidjoffe/dave_gnukem/issues/100 - thanks to @BeWorld2018 for report and patch suggestion)
		#if SDL_BYTEORDER==SDL_BIG_ENDIAN
		pImage->m_Rmask,
		pImage->m_Gmask,
		pImage->m_Bmask,
		pImage->m_Amask
		#else
		pImage->m_Rmask,
		pImage->m_Gmask,
		pImage->m_Bmask,
		pImage->m_Amask
		#endif
	);
	#else
	pSurfaceHardware = ::SDL_CreateRGBSurfaceFrom(
		pImage->Data(),
		pImage->Width(),
		pImage->Height(),
		32,
		pImage->Pitch(),
		// R,G,B,A masks (dj2018-03 specify different masks here on PPC etc. - see https://github.com/davidjoffe/dave_gnukem/issues/100 - thanks to @BeWorld2018 for report and patch suggestion)
		#if SDL_BYTEORDER==SDL_BIG_ENDIAN
		0x0000FF00, 0X00FF0000, 0xFF000000, 0x000000FF
		#else
		0xFF0000,
		0xFF00,
		0xFF,
		0xFF000000
		#endif
	);
	#endif
	//fixme should be sped up:
	// 1. A map is not really the most efficient way to do this as it must do a lookup for every blit
	// 2. std::map is probably not the fastest map for this either .. unordered_map may be (we don't need correct sorting and we're happy with slower inserts for faster lookups)
	// The more correct way requires some re-coding all over the codebase - return some kind of 'handle' here which could be a void* to either a struct with a pointer to the SDL_surface or the SDL_surface pointer as a void* or something ... then each 'user' must keep this 'texture handle' for when it does blits. Then, no map lookup.
	g_SurfaceMap[ pImage ] = pSurfaceHardware;

	return (void*)pSurfaceHardware;
					

	/*
								
								
								// TO CHECK - if fail to create in hardware, will it automatically created in SW or must we do that ourselves (!?)s
	SDL_Surface* pSurface = ::SDL_CreateRGBSurface(
SDL_SWSURFACE,
		pImage->Width(),
		pImage->Height(),
		32,//pVisDisplayBuffer->pSurface->format->BitsPerPixel,
		0xFF0000,//pVisDisplayBuffer->pSurface->format->Rmask,
		0xFF00,//pVisDisplayBuffer->pSurface->format->Gmask,
		0xFF,//pVisDisplayBuffer->pSurface->format->Bmask,
		0xFF000000);//pVisDisplayBuffer->pSurface->format->Amask);
	if (!pSurface)
		return nullptr;
	g_SurfaceMap[ pImage ] = pSurface;
	SDL_Surface* pSurfaceImg = ::SDL_CreateRGBSurfaceFrom(
		pImage->Data(),
		pImage->Width(),
		pImage->Height(),
		32,
		pImage->Pitch(),
		0xFF0000,
		0xFF00,
		0xFF,
		0xFF000000);

	if (pSurfaceImg)
	{
		//SDL_LockSurface(pSurfaceImg);
		//SDL_LockSurface(pSurface);
		//SDL_SetAlpha(pSurfaceImg, SDL_SRCALPHA, 255);
		//SDL_SetAlpha(pSurfaceImg, SDL_SRCALPHA, 100);

		SDL_Rect rectSrc;
		rectSrc.x = 0;
		rectSrc.y = 0;
		rectSrc.w = pImage->Width();
		rectSrc.h = pImage->Height();
		SDL_Rect rectDest;
		rectDest.x = 0;
		rectDest.y = 0;
		rectDest.w = pImage->Width();
		rectDest.h = pImage->Height();
		SDL_BlitSurface(pSurfaceImg, &rectSrc, pSurface, &rectDest);


		SDL_LockSurface(pSurface);
		SDL_LockSurface(pSurfaceImg);
		unsigned int *pSurfaceMemImg = (unsigned int*)pSurfaceImg->pixels;
		unsigned int *pSurfaceMem = (unsigned int*)pSurface->pixels;
		//unsigned int *pSurfaceMem2 = (unsigned int*)pSurfaceImg->pixels;
		for ( int y=0; y<pImage->Height(); ++y )
		{
			for ( int i=0; i<pImage->Width(); ++i )
			{
				if ((y%3)==0)
				{
					unsigned int vOrig = *(pSurfaceMemImg + i + ((y*pSurfaceImg->pitch)/4));
					if ((vOrig & 0xFF000000)==0)
					*(pSurfaceMem + i + ((y*pSurface->pitch)/4)) = *(pSurfaceMem + i + ((y*pSurface->pitch)/4))  &  0x00FFFFFF;
				}
			}
		}
		SDL_UnlockSurface(pSurfaceImg);
		SDL_UnlockSurface(pSurface);
SDL_SetSurfaceAlphaMod(pSurface, 0);
		//SDL_UnlockSurface(pSurfaceImg);

		//SDL_FreeSurface(pSurfaceImg);
		//delete pSurface;//?
	}
	return (void*)pSurface;

	//*/
}

