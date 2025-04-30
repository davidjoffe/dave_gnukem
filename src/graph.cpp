/*--------------------------------------------------------------------------*/
// graph.cpp
// David Joffe 1998/12
// replacing the old djgega.cpp graphics interface
/*
Copyright (C) 1998-2024 David Joffe
*/
/*--------------------------------------------------------------------------*/

#include "config.h"//CFG_APPLICATION_RENDER_RES_W //dj2019-06
#include "djlang.h"
#include "djfile.h"//djFileExists
#ifdef WIN32
#include <Windows.h>//for workaround
#endif
#include <SDL3/SDL.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef djUNICODE_TTF
#include <SDL3_ttf/SDL_ttf.h>//TTF_Init

#include "djfonts.h"
#include "datadir.h"//LoadFont
#endif//#ifdef djUNICODE_TTF

#ifdef djUSE_SDLIMAGE//d2023-02
#include <SDL3_image/SDL_image.h>
#endif

#include "console.h"//dj2022-11 refactoring
#include "sys_log.h"//djLog helpers
#include "djstring.h"//djStrPrintf for only currenlty framerate display so might move out of here

#include "graph.h"

// These dependencies suck
#include "djlog.h"

#include "djtypes.h"
#include "djsprite.h"

#include "djutf8.h"// For GraphDrawStringUTF8 (dj2023-11) for utf8 iterate

djImage *g_pFont8x8=NULL;
djImage* djDefaultFont()
{
	extern djSprite* g_pFont2;
	//if (djLang::DoTranslations() && g_pFont2!=nullptr && g_pFont2->GetImage()!=nullptr)
	// Unicode-based (utf8)
	if (g_pFont2!=nullptr && g_pFont2->GetImage()!=nullptr)
		return g_pFont2->GetImage();
	// Old font.tga (not Unicode-based)
	return g_pFont8x8;
}
djSprite* djDefaultFontSprite()
{
	extern djSprite* g_pFont2;
	if (g_pFont2!=nullptr && g_pFont2->IsLoaded())
		return g_pFont2;
	// Hmm should we have a djSprite for the old font.tga? [low]
	return nullptr;
}
// Hm we need to know not just the font, but whether it's utf8 or that old font.tga thing?

djVisual *pVisMain = NULL;
djVisual *pVisBack = NULL;
djVisual *pVisView = NULL;

/*--------------------------------------------------------------------------*/

// Hrm, other default font for non-en languages ..	    (our old font.tag is not Unicode codepoint-wise)
#define DATAFILE_IMG_FONT_OTHERLANG "fonts/pixeloperator/PixelOperator8-raster.png"

//dj2022-11 new helpers refactoring to try fullscreen toggle. Load the image but not yet the hardware surface cache item (do that after GraphInit) so we can do fullscreen toggle (semi-experimental this stuff may change)
void djFontInit()
{
	if (g_pFont8x8 != nullptr) return;

	//--- Load 8x8 font bitmap (FIXME error check)
	if (NULL != (g_pFont8x8 = new djImage))
	{
		//std::string sLang = djGetLanguage();
		if (djLang::DoTranslations())//sLang!="en" && !sLang.empty())
			g_pFont8x8->Load(djDATAPATHc(DATAFILE_IMG_FONT_OTHERLANG));// FILE_IMG_FONT);
			//g_pFont8x8->Load(djDATAPATHc(DATAFILE_IMG_FONT));// FILE_IMG_FONT);
		else
			g_pFont8x8->Load(djDATAPATHc(DATAFILE_IMG_FONT));// FILE_IMG_FONT);
		//g_pFont8x8->Load(djDATAPATHc(DATAFILE_IMG_FONT2));// FILE_IMG_FONT);
	}
}
void djFontDone()
{
	if (g_pFont8x8 == nullptr) return;
	djDEL(g_pFont8x8);
}

// ScaleView should also be false for the level editor [why does that already work?]
void GraphFlip(bool bScaleView)
{
	const bool bShowFrameRate = false;//<- DEBUG? 'global' frame rate

	// in leveled i think pVisBack is NULL??? not sure [dj2016-10] don't want this in level editor
	SDL_Surface* pVisTemp = NULL;//<-cbsu/sbsu? create once not every frame? this is just to remember temporarily
	// what's 'behind' the console message and immediately restore it after, otherwise e.g. say subsequent
	// volume changes cause the console message to draw over its previous self in a sort of 'additive' way
	// leaving text looking messed up [dj2016-10]
	if (pVisBack!=NULL && g_pFont8x8!=NULL && (!djConsoleMessage::m_sMsg.empty() || bShowFrameRate))
	{
		pVisTemp = SDL_CreateSurface(CFG_APPLICATION_RENDER_RES_W, 8, pVisBack->pSurface->format);
		if (pVisTemp)
		{
			SDL_Rect Rect;
			Rect.x=0;
			Rect.y=0;
			Rect.w=CFG_APPLICATION_RENDER_RES_W;
			Rect.h=8;
			SDL_BlitSurface(pVisBack->pSurface, &Rect, pVisTemp, &Rect);
		}

		// crude 'console message' stuff .. should make this nicer someday
		if (!djConsoleMessage::m_sMsg.empty())
		{
			GraphDrawString(pVisBack, g_pFont8x8, 0, 0, (const unsigned char*)djConsoleMessage::m_sMsg.c_str());
		}


		//SDL_GetTicks "Get the number of milliseconds since the SDL library initialization". Note that SDL_GetTicks wraps at ~49days
		static unsigned int uTimeLast = 0;
		if (uTimeLast==0)
			uTimeLast = SDL_GetTicks();
		unsigned int uTimeNow = SDL_GetTicks();

		// should this frame rate stuff be here ?? [dj2017-08]
		if (bShowFrameRate)
		{
			// Simple global frame rate display [?]
			// Be careful to avoid divide by zero here
			std::string sFrameRate;//Note this is 'instantaneous' frame rate i.e. last-frame-only so can look a bit jumpy, no smoothing
			if (uTimeNow>=0 && uTimeNow>uTimeLast)
			{
				// Was %.2f but we moving from printf-style stuff ..
				sFrameRate = std::to_string(1000.f / static_cast<float>(uTimeNow - uTimeLast));
				GraphDrawString( pVisBack, g_pFont8x8, 150, 0, (const unsigned char*)sFrameRate.c_str() );
			}
		}

		uTimeLast = uTimeNow;
	}

	djgFlip( pVisMain, pVisBack, bScaleView );

	// Restore to back buffer what was underneath the message
	if (pVisTemp)
	{
		SDL_Rect Rect;
		Rect.x=0;
		Rect.y=0;
		Rect.w=CFG_APPLICATION_RENDER_RES_W;
		Rect.h=8;
		SDL_BlitSurface(pVisTemp, &Rect, pVisBack->pSurface, &Rect);

		// inefficient .. do this once not every frame ..
		SDL_DestroySurface(pVisTemp);
	}
}

int djGraphicsSystem::m_nW = 0;
int djGraphicsSystem::m_nH = 0;
int djGraphicsSystem::m_nForceScale = 1;
bool djGraphicsSystem::m_bFullscreen = false;
bool djGraphicsSystem::m_bInitialized = false;
void djGraphicsSystem::ToggleFullscreen()
{
#if defined(djCFG_FORCE_FULLSCREEN_ALWAYS) || defined(djCFG_FORCE_WINDOWED_ALWAYS)
	// do nothing here
#else
	if (!IsInitialized()) return;
	// Clear texture cache
	// fixme auto delete and recreate hardware texture cache here?
	djImageHardwareSurfaceCache::ClearHardwareSurfaces();
	djGraphicsSystem::GraphDone();
	m_bFullscreen = !m_bFullscreen;
	djGraphicsSystem::GraphInit(m_bFullscreen, m_nW, m_nH, m_nForceScale);
	djImageHardwareSurfaceCache::RecreateHardwareSurfaces();
#endif
}

bool djSDLInit()
{
	// Initialize graphics library
	SDL_Init(SDL_INIT_VIDEO);

#ifdef djUNICODE_TTF
	TTF_Init();//dj2022-11
#endif
	return true;
}
bool djSDLDone()
{
#ifdef djUNICODE_TTF
	TTF_Quit();//dj2022-11
#endif

	SDL_Quit();
	return true;
}

bool djGraphicsSystem::GraphInit( bool bFullScreen, int iWidth, int iHeight, int nForceScale )
{
	#ifdef djCFG_FORCE_FULLSCREEN_ALWAYS
	bFullScreen = true;//dj2022-11
	#endif
	#ifdef djCFG_FORCE_WINDOWED_ALWAYS
	bFullScreen = false;//dj2022-11
	#endif
	djGraphicsSystem::m_bInitialized = false;
	// Remember requested base settings for later fullscreen toggle [dj2022-11]
	djGraphicsSystem::m_bFullscreen = bFullScreen;
	djGraphicsSystem::m_nForceScale = nForceScale;
	djGraphicsSystem::m_nW = iWidth;
	djGraphicsSystem::m_nH = iHeight;

#ifdef PANDORA
	//[dj2018-03] Not sure if this is quite the right way to go, but this is based roughly on
	// diff patch of https://github.com/ptitseb for OpenPandora port - https://pyra-handheld.com/boards/threads/dave-gnukem.79533/ and http://repo.openpandora.org/?page=detail&app=davegnukem-magicsam
	iWidth = 320; iHeight = 200;
#else
	// [dj2016-10] Get the user's monitor resolution, and find (basically) largest multiple of 320x200 that fits in
	// that size, to make for 'largest possible' gameplay window, that also scales from 320x200 'proportionally' nicely
	// (i.e. square aspect ratio of pixels, etc.).
	// The game renders to a 320x200 game view buffer which is either blitted or scale-blitted
	// to the back(front?)buffer and then 'flipped' each frame. This allows the game to
	// be basically 320x200 but allows the integrated level editor to be much higher resolution.
	// (Note though in full-screen mode I think the above does not apply, it just
	// tries to set a 'true' 320x200 fullscreen display mode, IIRC - dj2017-08.)
	// No that doesn't seem to be what happens, press F5 when running with "-f" and see.
	// [low/future] - if 2 monitors, will this behave 'correct'
	auto dm = SDL_GetCurrentDisplayMode(0);
	int max_w = -1;
	int max_h = -1;
	if (dm)
	{
		// THIS MUST BE TESTED ON LINUX [dj2016-10]
		max_w = dm->w;
		max_h = dm->h;
		if (max_w>iWidth && max_h>iHeight)
		{
			int nMultiple = djMAX(1, djMIN( max_w / iWidth, max_h / iHeight ) );
			// If a forced scale factor has been passed in by commandline, try use that
			// what happens if pass in ginormous value? crash .. so limit to make 'not too big' e.g. nMultiple*2 max?
			if (nForceScale>=1)
			{
				if (nForceScale>nMultiple*2)
					nForceScale=nMultiple*2;
				nMultiple = nForceScale;
			}
			iWidth *= nMultiple;
			iHeight *= nMultiple;
		}
		djLog::LogFormatStr( "GraphInit(): DisplayResolution(%d,%d).\n", max_w, max_h );
	}
#endif


	// Hide mouse cursor
	SDL_HideCursor();

	std::string sIcon = djDATAPATHs("icon.bmp");
	#ifdef __APPLE__
	//dj2023 new hi-res icon (todo - Linux too? or still 32x32 for Linux?)
	if (djFileExists(djDATAPATHc("icon/Dave Gnukem.png")))
		sIcon = djDATAPATHs("icon/Dave Gnukem.png");
	#endif

	//--- (1) - Front buffer
	djLog::LogFormatStr( "GraphInit(): djgOpenVisual(w,h=%d,%d).\n", iWidth, iHeight );
	if (NULL == (pVisMain = djgOpenVisual( bFullScreen?"fullscreen":NULL, iWidth, iHeight, 32, false, "Dave Gnukem", sIcon.c_str())))
	{
		printf( "GraphInit(): COULDN'T CREATE MAIN WINDOW\n" );
		return false;
	}

	auto f = SDL_GetPixelFormatDetails(pVisMain->format);
	djLog::LogFormatStr( "GraphInit(): Display bytes per pixel %d\n", (int)f->bits_per_pixel) ;
	int imode = f->bits_per_pixel;

	// Set the 32<->16 pixel conversion atributes, so the
	// images would be displayed correctly with any pixel format
	SetPixelConversion ( pVisMain );

	//--- (2) - Back buffer
	if (NULL == (pVisBack = djgOpenVisual( "memory", iWidth, iHeight, imode )))
	{
		printf( "GraphInit(): COULDN'T OPEN GBACK\n" );
		return false;
	}

	//--- (3) - View buffer
	if (NULL == (pVisView = djgOpenVisual( "memory", iWidth, iHeight, imode )))
	{
		printf( "GraphInit(): COULDN'T OPEN GVIEW\n" );
		return false;
	}

	//--- (5) Create hardware surface for main 8x8 font bitmap (FIXME error check)
	djCreateImageHWSurface(g_pFont8x8);
	//SDL_Surface *pSurface = (SDL_Surface *)djCreateImageHWSurface(g_pFont8x8);
	//SDL_SetColorKey(pSurface, SDL_TRUE, SDL_MapRGB(pSurface->format, 0, 0, 0));

#ifdef WIN32
	// [Windows] Not sure if it's LibSDL or Windows but the window keeps getting created positioned so that the bottom portion of it is
	// off the bottom of the screen and you have to manually move the window to play every time, which is annoying ... attempting here
	// to get HWND of active window and try move it automatically if it's off the bottom of the screen .. [dj2016-10]
	//HANDLE hProc = ::GetCurrentProcess();
	HWND hWnd = ::GetActiveWindow();
	if (hWnd!=NULL)
	{
		// Want original X position [so when we call MoveWindow we keep Windows' X position - I'm not quite sure if this is good
		// or bad but I think it miiight be better in the case of multiple monitors, not sure though, this would need more testing [dj2016-10]]
		RECT rcWnd;
		memset(&rcWnd,0,sizeof(rcWnd));
		::GetWindowRect(hWnd,&rcWnd);

		RECT rc;
		memset(&rc,0,sizeof(rc));
		if (::GetClientRect(hWnd,&rc))//Client rect is the size of the game window area only, not including e.g. window border and title etc.
		{
			// [Note we need to factor in the size of the Windows taskbar bla bla]
			//if (max_h>0 && rc.top + iHeight >= max_h)
			{
			DWORD dwCurrentStyles = (DWORD)::GetWindowLongA(hWnd,GWL_STYLE);
			DWORD dwExStyle = (DWORD)::GetWindowLongA(hWnd,GWL_EXSTYLE);

			// MoveWindow will 'shrink' the window as it uses total size *including* window dressing, while GetWindowRect
			::AdjustWindowRectEx(&rc,dwCurrentStyles,FALSE,dwExStyle);//<-Uppercase FALSE as we're now in Win32-API-land [minor point]

			// For now just move window to top; later should try make his more 'intelligent' (or
			// maybe with LibSDL2 this is perhaps better?
			::MoveWindow(hWnd, rcWnd.left, 0, rc.right-rc.left, rc.bottom-rc.top, false);
			}
		}
	}
#endif


	djGraphicsSystem::m_bInitialized = true;
	djImageHardwareSurfaceCache::RecreateHardwareSurfaces();//<- check if images in cache that 'want' hardware surfaces? create now. This allows us to potentially e.g. add the font before or after GraphInit and it just 'figures it out'
	return true;
}

void djGraphicsSystem::GraphDone()
{
	// Why not just call this here? and init the font image separately
	djImageHardwareSurfaceCache::ClearHardwareSurfaces();
	//djDestroyImageHWSurface(g_pFont8x8);
	//djDEL(g_pFont8x8);

	djgCloseVisual( pVisView );
	djgCloseVisual( pVisBack );
	djgCloseVisual( pVisMain );

	djDEL(pVisView);
	djDEL(pVisBack);
	djDEL(pVisMain);

	djGraphicsSystem::m_bInitialized = false;
	//SDL_Quit();
}

// FIXME: , view_height? [dj2022-11 change the width/height to pixels instead of logical game blocks so we can have arbitrary viewport size not multiples of BLOCKW/BLOCKH etc. - more generic]
void GraphFlipView(int iViewWidthPixels, int iViewHeightPixels, int nXS, int nYS, int nXD, int nYD)
{
	//djgDrawVisual( pVisBack, pVisView, g_bLargeViewport?0:16, g_bLargeViewport?0:16, g_bLargeViewport?0:16, g_bLargeViewport?0:16, iViewWidth*16, iViewHeight*16 );
	djgDrawVisual(pVisBack, pVisView, nXD, nYD, nXS, nYS, iViewWidthPixels, iViewHeightPixels);
}

void GraphDrawStringUTF8( djVisual *pVis, djImage *pImg, int x, int y, int nCharW, int nCharH, const std::string& sText)
{
	if (sText.empty()) return;
	GraphDrawStringUTF8(pVis, pImg, x, y, nCharW, nCharH, sText.c_str(), sText.length());
}

void GraphDrawStringUTF8( djVisual *pVis, djImage *pImg, int x, int y, int nCharW, int nCharH, const char *szStr, int nStrLen )
{
	if (szStr == nullptr || szStr[0]==0) return;
	// FIXME: bounds check properly
	if (x<0 || y<0) return;

	// Draw each character in the string

	const char* szStart = (const char*)szStr;
	int c = -1;//codepoint in 32-bit
	size_t uOffset = 0;
	size_t uLen2 = (nStrLen<0 ? strlen(szStart) : (size_t)nStrLen);//If length passed in, avoid a strlen() for speed reasons
	// [dj2023] Must convert utf8 to 32-bit Unicode glyphs and iterate over string
	// Remember that utf8 is multi-byte and variable-width encoding so a single Unicode codepoint (i.e. one 32-bit value) could be maybe e.g. 1 byte or 2 bytes or 3 bytes or 4 bytes etc. in the utf8 string (but strlen returns the full number of bytes, not "Unicode Characters")
	//"Reads a single codepoint from the UTF-8 sequence being pointed to by str. The maximum number of bytes read is strlen, unless strlen is negative (in which case up to 4 bytes are read). If a valid codepoint could be read, it is stored in the variable pointed to by codepoint_ref, otherwise that variable will be set to -1. In case of success, the number of bytes read is returned; otherwise, a negative error code is returned."
	int ret = djutf8iterate(szStart + uOffset, uLen2, c);
	int x_=x;
	int y_=y;
	while (ret > 0)
	{
		uLen2 -= (size_t)ret;
		uOffset += (size_t)ret;

		// \n is 10, \r is 13 .. 
		if (c=='\n')//Newline?
		{
			y_ += nCharH;
			x_ = x;
		}
		else
		{
			int nXSrc = (c % (pImg->Width() / nCharW)) * nCharW;
			int nYSrc = (c / (pImg->Width() / nCharW)) * nCharH;
			//djgDrawImageAlpha( pVis, pImg, nXSrc, nYSrc, x_, y_, nCharW, nCharH);
			//sstd::string sDebug = std::to_string((int)c);
			//GraphDrawString(pVis, pImg, x_, y_, (const unsigned char*)sDebug.c_str());
			//x_ += nCharW * (int)sDebug.length();
			
			djgDrawImageAlpha( pVis, pImg, nXSrc, nYSrc, x_, y_, nCharW, nCharH);
			x_ += nCharW;
		}
		ret = djutf8iterate(szStart + uOffset, uLen2, c);
	}
}

void GraphDrawString( djVisual *pVis, djImage *pImg, int x, int y, const std::string& sText )
{
	//todo-future all these weird const unsigned char* should probably just be const char* [dj2024] I think there was some reason for it that may have been valid in the 90s but that's now lost to time
	if (sText.empty()) return;
	GraphDrawString( pVis, pImg, x, y, (const unsigned char *)(sText.c_str()) );
}

// FIXME: Currenetly assumes a 256-char 32x8 character 256x128 pixel alpha-mapped image
void GraphDrawString( djVisual *pVis, djImage *pImg, int x, int y, const unsigned char *szStr )
{
	if (szStr == nullptr) return;
	// FIXME: bounds check properly
	if (x<0 || y<0) return;

	const unsigned int W=8;
	const unsigned int H=8;

	// Draw each character in the string
	int x_=x;
	unsigned int uLen=strlen((char*)szStr);
	for ( unsigned int i=0; i<uLen; ++i )
	{
		// so um the idea is we want to start making this utf8-able-ish
		const int iIndex = (int)szStr[i];
		// \n is 10, \r is 13 .. 
		if (szStr[i]=='\n')//Newline? [dj2016-10-28]
		{
			y += H;
			x_ = x;
		}
		else
		{
			djgDrawImageAlpha( pVis, pImg, W*(iIndex%32), H*(iIndex/32), x_, y, W, H);
			x_ += W;
		}
	}
}

#ifdef djUNICODE_TTF

//! [New dj2022-11]
void djFontListInit()
{
}
//! [New dj2022-11]
void djFontListDone()
{
	///g_FontList.CleanupFonts();
}

//void DrawStringUnicodeHelper(djVisual* pVis, int x, int y, SDL_Color Color, const std::string& sTextUTF8)
void DrawStringUnicodeHelper(djVisual* pVis, int x, int y, SDL_Color Color, const char* szTextUTF8, const unsigned int uLen)
{
	if (szTextUTF8 == nullptr || uLen == 0) return;
	if (szTextUTF8[0] == 0) return;//non-null but empty string? do nothing

//#ifdef djTTF_HAVE_HARFBUZZ_EXTENSIONS
	// negative means right to left
	int nDir = djUnicodeTextHelpers::GuessDirection(szTextUTF8, uLen);
//#endif

	//fixme
	extern djFontList g_FontList;
	// Get best matching font [this needs work]
	//TTF_Font* pFont = djUnicodeFontHelpers::FindBestMatchingFontMostCharsStr(g_FontList.m_apFonts, sTextUTF8, sTextUTF8.length());
	TTF_Font* pFont = djUnicodeFontHelpers::FindBestMatchingFontMostCharsStr(&g_FontList, g_FontList.m_apFonts, szTextUTF8, uLen, nDir);
	if (!pFont)//<- old fallback for safety in case something went wrong and we have no matching fonts
	{
		if (g_pFont8x8)
			GraphDrawString(pVis, g_pFont8x8, x, y, (unsigned char*)szTextUTF8);
		return;
	}

#ifdef djTTF_HAVE_HARFBUZZ_EXTENSIONS
	//*
	if (nDir < 0)
	{
		TTF_SetFontDirection(pFont, TTF_DIRECTION_RTL);
		// hm could be Hebrew too but if Arabic we need the harfbuzz extensions .. should also detect language
		//TTF_SetFontScriptName(pFont, "Arab");
	}
	else
	{
		TTF_SetFontDirection(pFont, TTF_DIRECTION_LTR);
		// hmm what script name to set here?
	}
	//*/
#endif

	// first dropshadow

	//SDL_Surface* sur = TTF_RenderUNICODE_Blended_Wrapped(pFont, (const Uint16*)text.c_str(), SDL_Color{ 255, 255, 255, 255 }, CFG_APPLICATION_RENDER_RES_W - nXPOS);
	//SDL_Surface* sur = TTF_RenderUNICODE_Blended(pFont, (const Uint16*)text.c_str(), SDL_Color{ 255, 255, 255, 255 }, CFG_APPLICATION_RENDER_RES_W - nXPOS);
	//SDL_Surface* sur = TTF_RenderUTF8_Blended(pFont, sText.c_str(), SDL_Color{ 0, 0, 0, 255 });//black for +1 offset underline 'shadow' effect
	SDL_Surface* sur = TTF_RenderText_Solid(pFont, szTextUTF8, uLen, SDL_Color{ 0, 0, 0, 255 });//black for +1 offset underline 'shadow' effect
	//SDL_Texture* tex = SDL_CreateTextureFromSurface(pVis->pRenderer, sur);
	if (sur)
	{
		SDL_Rect rcDest{ x + 1, y + 1, pVis->width, pVis->height };
		CdjRect rcSrc(0, 0, sur->w, sur->h);
		SDL_BlitSurface(sur, &rcSrc, pVis->pSurface, &rcDest);//blit [is this best way?]
		SDL_BlitSurface(sur, &rcSrc, pVis->pSurface, &rcDest);//blit [is this best way?]
		//SDL_DestroySurface(sur);// why does this SDL_DestroySurface crash?SDL_DestroySurface(sur); [dj2022-11 fixme]
		sur = nullptr;
	}
	//sur = TTF_RenderUTF8_Blended(pFont, sText.c_str(), SDL_Color{ 255, 255, 255, 255 });
	sur = TTF_RenderText_Blended(pFont, szTextUTF8, uLen, Color);
	//SDL_Texture* tex = SDL_CreateTextureFromSurface(pVis->pRenderer, sur);
	if (sur)
	{
		SDL_Rect rcDest{ x, y, pVis->width, pVis->height };
		CdjRect rcSrc(0, 0, sur->w, sur->h);
		SDL_BlitSurface(sur, &rcSrc, pVis->pSurface, &rcDest);//blit [is this best way?]
		SDL_BlitSurface(sur, &rcSrc, pVis->pSurface, &rcDest);//blit [is this best way?]
		//SDL_BlitSurface(sur, &rcSrc, pVis->pSurface, &rcDest);//blit [is this best way?]
		//SDL_DestroySurface(sur);// why does this SDL_DestroySurface crash?SDL_DestroySurface(sur); [dj2022-11 fixme]
		sur = nullptr;
	}
	//SDL_RenderCopy(pVisBack->pRenderer, tex, NULL, &rect);
}
#endif
