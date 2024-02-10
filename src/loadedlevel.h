/*!
\file    loadedlevel.h
\brief   Some data and metadata of current level being played or loaded for editor
\author  David Joffe

Copyright (C) 1995-2024 David Joffe
*/
/*--------------------------------------------------------------------------*/
// Refactoring some stuff from game.h and extending ..
/*--------------------------------------------------------------------------*/
#ifndef _LOADEDLEVEL_H_
#define _LOADEDLEVEL_H_

#include <vector>
#include "djrect.h" // Include the rectangle template class
//#include "config.h"

// Forward declarations (don't include actual headers here for compile speed reasons etc.)
class djImage;
class djSprite;

struct tsLoadedLevel
{
	//djSprite* pSkinGame; // Main game view skin (while playing)
	//djSprite* pBackground = nullptr; // Level background image
	// Main game backing sprite (while playing) - new [dj2023-11]
	djSprite* pBack1 = nullptr;
	djSprite* Back1() const { return pBack1; }

	// Vector of rectangles (specific case of polygons, but slightly faster collision detection if we do it this way) for collision detection [dj2024]
	std::vector<djRectD> collisionRects;
	//std::vector<djRectD> collisionPolys;

	// For speed reasons an extra copy of pBack1->GetImage() (if loaded) or nullptr if not
	djImage* pImgBack1 = nullptr;
	//djImage* ImgBack1() const { return pBack1!=nullptr && pBack1->IsLoaded() ? pBack1->GetImage() : nullptr; }
	djImage* ImgBack1() const { return pImgBack1; }
};

//---------------------------------------------------------------------------
extern tsLoadedLevel g_Level;
//---------------------------------------------------------------------------

#endif
