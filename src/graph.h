/*!
\file    graph.h
\brief   Graphics helpers
\author  David Joffe

Copyright (C) 1998-2018 David Joffe
*/
/*--------------------------------------------------------------------------*/
// graph.h
// David Joffe 1998/12
// replacing the old djgega.cpp graphics interface
/*--------------------------------------------------------------------------*/
#ifndef _GRAPH_H_
#define _GRAPH_H_

#include "datadir.h"
#include "djgraph.h"
#include "djimage.h"
#include <string>

//! Image file to use for font
#define FILE_IMG_FONT DATA_DIR "font.tga"

//! Main visual
extern djVisual *pVisMain;
//! Back buffer
extern djVisual *pVisBack;
//! View visual - 3rd buffer for game play display
extern djVisual *pVisView;

extern djImage *g_pFont8x8;

//! Convenience macro to call the sprite draw function for 16x16 sprite b in sprite set a
#define DRAW_SPRITE16(vis,a,b,x,y) djgDrawImage( vis, g_pCurMission->GetSpriteData(a)->m_pImage, ((b)%SPRITESHEET_NUM_COLS)*BLOCKW,((b)/SPRITESHEET_NUM_COLS)*BLOCKH, (x),(y), BLOCKW,BLOCKH )

#ifdef EXPERIMENTAL_SPRITE_AUTO_DROPSHADOWS
extern bool g_bSpriteDropShadows;
#define DRAW_SPRITEA_SHADOW(vis,a,b,x,y,w,h) if (g_bSpriteDropShadows) djgDrawImageAlpha( vis, g_pCurMission->GetSpriteData(a)->m_pImageShadow, ((b)%SPRITESHEET_NUM_COLS)*BLOCKH,((b)/SPRITESHEET_NUM_COLS)*BLOCKH, (x),(y), (w),(h) )
#endif

//! Same as \ref DRAW_SPRITE16 but also uses alpha map for transparency masking
#define DRAW_SPRITE16A(vis,a,b,x,y) djgDrawImageAlpha( vis, g_pCurMission->GetSpriteData(a)->m_pImage, ((b)%SPRITESHEET_NUM_COLS)*BLOCKW,((b)/SPRITESHEET_NUM_COLS)*BLOCKH, (x),(y), BLOCKW,BLOCKH )

#define DRAW_SPRITEA(vis,a,b,x,y,w,h) djgDrawImageAlpha( vis, g_pCurMission->GetSpriteData(a)->m_pImage, ((b)%SPRITESHEET_NUM_COLS)*BLOCKW,((b)/SPRITESHEET_NUM_COLS)*BLOCKH, (x),(y), (w),(h) )

//! Initialize the graphics system for the game
extern bool GraphInit( bool bFullScreen, int iWidth, int iHeight, int nForceScale=-1 );

//! Shut down the graphics system
extern void GraphDone();

//! Flip the back buffer to the front buffer
extern void GraphFlip(bool bScaleView);

//! Flip the view buffer into the back buffer
extern void GraphFlipView( int iViewWidth, int iViewHeight, int nXS, int nYS, int nXD, int nYD );

//! Draw a string of characters from a font bitmap
extern void GraphDrawString( djVisual *pVis, djImage *pImg, int x, int y, const unsigned char *szStr );

//! Very simple pseudo 'console message' .. not exactly sure where this 'belongs' dependency-wise [dj2016-10]
extern void SetConsoleMessage( const std::string& sMsg );

#endif
