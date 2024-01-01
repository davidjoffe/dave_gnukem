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
// [dj] A few off-the-cuff thoughts on text rendering and language direction and localization and right-to-left support:
// It's slightly debatable how best to handle rendering text in a game that is, say, just a numerical score, which should remain LTR - each piece of text
// we may need to do some determining of the text direction and maybe in future handle mixed text in a fancy way but for this simple game we don't need to be too fancy
// We could also include support for some basic Unicode modifiers to help during text rendering, that could give e.g. user interface some power and control
// if a text is rendering in wrong direction - e.g. let's say we had some text that's one Hebrew character followed by 10 directioon-ambiguous or Latin alphabet characters or Roman numerals
// and our mixed-text rendering engine decides to render it wrong, then maybe we could allow translators to e.g. use Unicode special direction symbols to help correct it in these edge cases
//
// In general when coding and designing for localization, also think about these issues to try keep localization simpler, e.g.:
// Better: "Score: [N]"
// Worse: "Your score is [N] point(s)"
// The former is much, much easier to just localize "Score", while the latter creates various translation issues in many different languages (from conjugation issues to different representations of plural to possible mixed-text rendering direction issues and so on)
/*--------------------------------------------------------------------------*/

#ifndef _GRAPH_H_
#define _GRAPH_H_

#include "config.h"
#include "datadir.h"
#include "djgraph.h"
#include "djimage.h"
#include <string>
#ifdef djUNICODE_TTF
//#include "djfonts.h"//dj2022-11 may refactor this may move to another file
#endif

//! Image file to use for font
//#define FILE_IMG_FONT DATA_DIR "font.tga"
#define DATAFILE_IMG_FONT "font.tga"
//#define DATAFILE_IMG_FONT "fonts/pixeloperator/PixelOperator8-raster.png"
extern djImage* djDefaultFont();
class djSprite;
extern djSprite* djDefaultFontSprite();

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
extern void GraphDrawString( djVisual *pVis, djImage *pImg, int x, int y, const std::string& sText );
extern void GraphDrawStringUTF8( djVisual *pVis, djImage *pImg, int x, int y, int nCharW, int nCharH, const char *szStr, int nStrLen=-1 );
extern void GraphDrawStringUTF8( djVisual *pVis, djImage *pImg, int x, int y, int nCharW, int nCharH, const std::string& sText);

#ifdef djUNICODE_TTF
//dj2022-11 basics of new Unicode TTF/OTF font stuff
extern void DrawStringUnicodeHelper(djVisual* pVis, int x, int y, SDL_Color Color, const char* szTextUTF8, const unsigned int uLen);
/*
extern void DrawStringUnicodeHelper(djVisual* pVis, int x, int y, SDL_Color Color, const std::string& sTextUTF8);
*/
#endif

#endif
