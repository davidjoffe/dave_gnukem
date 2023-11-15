/*!
\file    graph.h
\brief   Graphics helpers
\author  David Joffe

Copyright (C) 1998-2023 David Joffe
*/
/*--------------------------------------------------------------------------*/
// graph.h
// David Joffe 1998/12
// replacing the old djgega.cpp graphics interface
/*--------------------------------------------------------------------------*/
#ifndef _GRAPH_H_
#define _GRAPH_H_

#include "config.h"
#include "datadir.h"
#include "djgraph.h"
#include "djimage.h"
//#include <string>
#ifdef djUNICODE_TTF
//#include "djfonts.h"//dj2022-11 may refactor this may move to another file
#endif

//! Image file to use for font
//#define FILE_IMG_FONT DATA_DIR "font.tga"
#define DATAFILE_IMG_FONT "font.tga"
//#define DATAFILE_IMG_FONT "fonts/pixeloperator/PixelOperator8-raster.png"

//! Main visual
extern djVisual *pVisMain;
//! Back buffer
extern djVisual *pVisBack;
//! View visual - 3rd buffer for game play display
extern djVisual *pVisView;

// Main game 8x8 font
extern djImage *g_pFont8x8;
extern void djFontInit();
extern void djFontDone();
/*--------------------------------------------------------------------------*/
#ifdef djUNICODE_TTF

//dj2022-11 basics of new Unicode TTF/OTF font stuff
//extern djFontList g_FontList;

//! Initialize [New dj2022-11]
extern void djFontListInit();
//! [New dj2022-11]
extern void djFontListDone();
#endif
/*--------------------------------------------------------------------------*/

//! Convenience macro to call the sprite draw function for 16x16 sprite b in sprite set a
#define DRAW_SPRITE16(vis,a,b,x,y) djgDrawImage( vis, g_pCurMission->GetSpriteData(a)->m_pImage, ((b)%SPRITESHEET_NUM_COLS)*BLOCKW,((b)/SPRITESHEET_NUM_COLS)*BLOCKH, (x),(y), BLOCKW,BLOCKH )

#ifdef djSPRITE_AUTO_DROPSHADOWS
// 'Sprite auto dropshadows' effect
extern bool g_bSpriteDropShadows;
#define DRAW_SPRITEA_SHADOW(vis,a,b,x,y,w,h) if (g_bSpriteDropShadows) djgDrawImageAlpha( vis, g_pCurMission->GetSpriteData(a)->m_pImageAutoShadow, ((b)%SPRITESHEET_NUM_COLS)*BLOCKH,((b)/SPRITESHEET_NUM_COLS)*BLOCKH, (x),(y), (w),(h) )
#endif

//! Same as \ref DRAW_SPRITE16 but also uses alpha map for transparency masking
#define DRAW_SPRITE16A(vis,a,b,x,y) djgDrawImageAlpha( vis, g_pCurMission->GetSpriteData(a)->m_pImage, ((b)%SPRITESHEET_NUM_COLS)*BLOCKW,((b)/SPRITESHEET_NUM_COLS)*BLOCKH, (x),(y), BLOCKW,BLOCKH )

#define DRAW_SPRITEA(vis,a,b,x,y,w,h) djgDrawImageAlpha( vis, g_pCurMission->GetSpriteData(a)->m_pImage, ((b)%SPRITESHEET_NUM_COLS)*BLOCKW,((b)/SPRITESHEET_NUM_COLS)*BLOCKH, (x),(y), (w),(h) )

//dj2022-11 start refactoring this stuff to think of things like potential fullscreen toggle (and possibly later we might use derived classes perhaps to change the instantiation behaviour for different games?)
class djGraphicsSystem
{
public:

	//! Initialize the graphics system for the game
	static bool GraphInit(bool bFullScreen, int iWidth, int iHeight, int nForceScale = -1);

	//! Shut down the graphics system
	static void GraphDone();

	static bool IsInitialized() { return m_bInitialized; }

	//dj2022-11 experimental live fullscreen toggle? this ought to bring out some bugs
	static void ToggleFullscreen();

	static int m_nW;
	static int m_nH;
	static int m_nForceScale;
	static bool m_bFullscreen;
	static bool m_bInitialized;
};


//! Flip the back buffer to the front buffer
extern void GraphFlip(bool bScaleView);

//! Flip the view buffer into the back buffer
extern void GraphFlipView(int iViewWidthPixels, int iViewHeightPixels, int nXS, int nYS, int nXD, int nYD);

//! Draw a string of characters from a font bitmap
extern void GraphDrawString( djVisual *pVis, djImage *pImg, int x, int y, const unsigned char *szStr );
extern void GraphDrawStringUTF8( djVisual *pVis, djImage *pImg, int x, int y, int nCharW, int nCharH, const unsigned char *szStr, int nStrLen=-1 );

#ifdef djUNICODE_TTF
//dj2022-11 basics of new Unicode TTF/OTF font stuff
extern void DrawStringUnicodeHelper(djVisual* pVis, int x, int y, SDL_Color Color, const char* szTextUTF8, const unsigned int uLen);
/*
extern void DrawStringUnicodeHelper(djVisual* pVis, int x, int y, SDL_Color Color, const std::string& sTextUTF8);
*/
#endif

#endif
