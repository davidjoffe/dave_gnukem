// effect_viewportshadow.cpp
// Copyright (C) 1995-2022 David Joffe / Dave Gnukem project
//
// dj2022-11 just refactoring this recent new effect from game.cpp into separate .h/cpp files for neatness as game.cpp getting a bit long and cluttered
//

/*--------------------------------------------------------------------------*/
#include "config.h"
#include "effect_viewportshadow.h"
#ifdef djEFFECT_VIEWPORTSHADOW
//dj2022-11 adding this into official but still off by default for now as is a bit a bit beta and needs more testing and not sure if i like it yet .. dj2022-11:
#include <math.h>//sqrtf

//gross global
djEffectFauxShadowingAroundHero g_Effect;

djEffectFauxShadowingAroundHero::djEffectFauxShadowingAroundHero()
{
	//ApplyIntensity();
}
djEffectFauxShadowingAroundHero::~djEffectFauxShadowingAroundHero()
{
}

/*(virtual??)*/
void djEffectFauxShadowingAroundHero::DrawEffect(djVisual* pVis, const int nLevelBlockX, const int nLevelBlockY, const int nXOffset, const int nYOffset,
	// position in world space of top left of viewport (in pixels) (doesn't change over a single frame for all blocks)
	const int nViewportOriginX, const int nViewportOriginY,// const int nViewportPixelW, const int nViewportPixelH,
	const float fBlockWorldXStart, const float fBlockWorldYStart)
{
	// DARK effect: nHIGH=180, fFADEOVERNUMBLOCKS = 7.2f, fLOWRANGEEXTENT = 130.0f
	// SUBTLE, light effect:  nHIGH=262, fFADEOVERNUMBLOCKS = 7.2f, fLOWRANGEEXTENT = 110.0f
	// ALSO OK, light effect: nHIGH=262, fFADEOVERNUMBLOCKS = 7.2f, fLOWRANGEEXTENT = 160.0f

	//const int j = nLevelBlockX;
	//const int i = nLevelBlockY;

	const int nHIGH = g_Effect.nHIGH;// 180;
	const float fFADEOVERNUMBLOCKS = g_Effect.fFADEOVERNUMBLOCKS;// 7.2f;
	//const float fLOWRANGEEXTENT = 160.0f; // Higher numbers toward 250 make for very dark 'corners' .. lower numbers around 100 etc. make for more subtle effect
	const float fLOWRANGEEXTENT = g_Effect.fLOWRANGEEXTENT;// 130.0f; // Higher numbers toward 250 make for very dark 'corners' .. lower numbers around 100 etc. make for more subtle effect
	const int SHADSIZE = g_Effect.SHADSIZE;

	// SUBTLE, light effect:  const int nHIGH = 262; const float fFADEOVERNUMBLOCKS = 7.2f; const float fLOWRANGEEXTENT = 110.0f;

	//float fBlockWorldXStart = fBlockWorl;
	float fBlockWorldY = fBlockWorldYStart;
	//const float fHeroWorldX = (float)(HERO_PIXELX) + ((float)BLOCKW/2.0f);
	//const float fHeroWorldY = (float)(HERO_PIXELY) + (float)BLOCKH;
	extern int VIEW_WIDTH;
	extern int VIEW_HEIGHT;
	//const float fHeroWorldX = ((float)(g_Viewport.xo) + ((float)VIEW_WIDTH) / 2.0f) * (float)BLOCKW;
	//const float fHeroWorldY = ((float)(g_Viewport.yo) + ((float)VIEW_HEIGHT) / 2.0f) * (float)BLOCKH;
	//viewport centre (in world space in pixels)
	//const float fViewCenterWorldX = ((float)nViewportOriginX + (((float)VIEW_WIDTH) / 2.0f) * (float)BLOCKW);
	//const float fViewCenterWorldY = ((float)nViewportOriginY + (((float)VIEW_HEIGHT) / 2.0f) * (float)BLOCKH);
	const float fHeroWorldX = ((float)nViewportOriginX + (((float)VIEW_WIDTH) / 2.0f) * (float)BLOCKW);
	const float fHeroWorldY = ((float)nViewportOriginY + (((float)VIEW_HEIGHT) / 2.0f) * (float)BLOCKH);

	int nIntensity = 0;//0-255, 0 is darkness, 255 is fully lit (or rather, no darkness)
	float fDistance = 0.0f;

	SDL_Rect rectSrc;
	//rectSrc.x = (nIntensity % 16) * SHADSIZE;
	//rectSrc.y = (nIntensity / 16) * SHADSIZE;
	rectSrc.w = SHADSIZE;
	rectSrc.h = SHADSIZE;
	SDL_Rect rectDest;
	//rectDest.x = nXOffset;
	rectDest.y = nYOffset;
	rectDest.w = SHADSIZE;
	rectDest.h = SHADSIZE;
	//const float fFACTOR = 1.0f;
	///// dj2022-11 TODO could should this factor stuff should what, be table-driven for speed reasons? or is the blitting the slow part? etc.
	const float fFACTOR = 0.9f;//dj2022-11 added this 0.9f factor but can't remember exactly why, basically in my original  tests on the effect this was 1 then I changed it to 0.9 maybe later just try see if i can remember why and what value it should be
	for (int nY = 0; nY < BLOCKH; nY += SHADSIZE)
	{
		rectDest.x = nXOffset;
		float fBlockWorldX = fBlockWorldXStart;

		for (int nX = 0; nX < BLOCKW; nX += SHADSIZE)
		{
			// this could be table-driven:
			fDistance = sqrtf((fBlockWorldY - fHeroWorldY) * (fBlockWorldY - fHeroWorldY) + fFACTOR * (fBlockWorldX - fHeroWorldX) * fFACTOR * (fBlockWorldX - fHeroWorldX));
			nIntensity = nHIGH - (unsigned int)(((fDistance / (float)BLOCKW) / fFADEOVERNUMBLOCKS) * fLOWRANGEEXTENT);
			if (nIntensity < 0)nIntensity = 0;
			if (nIntensity < 255)
			{
				rectSrc.x = (nIntensity % 16) * SHADSIZE;
				rectSrc.y = (nIntensity / 16) * SHADSIZE;
				SDL_BlitSurface(m_pShadows, &rectSrc, pVis->pSurface, &rectDest);
			}

			rectDest.x += SHADSIZE;
			fBlockWorldX += (float)SHADSIZE;
		}

		rectDest.y += SHADSIZE;
		fBlockWorldY += (float)SHADSIZE;
	}
}

void djEffectFauxShadowingAroundHero::SetIntensity(unsigned int nIntensity)
{
	m_nEnabledIntensity = (nIntensity % 5);

	ApplyIntensity();
}

void djEffectFauxShadowingAroundHero::ApplyIntensity()
{
	// MORE TESTING NEEDED on these values this is beta stuff .. dj2022-11
	switch (m_nEnabledIntensity)
	{
	case 1://SUBTLE, light effect
		nHIGH = 267; fFADEOVERNUMBLOCKS = 7.2f; fLOWRANGEEXTENT = 105.0f;
		break;
	case 2://SUBTLE, light effect
		nHIGH = 262; fFADEOVERNUMBLOCKS = 7.2f; fLOWRANGEEXTENT = 110.0f;
		break;
	case 3://ALSO OK, light effect
		nHIGH = 262; fFADEOVERNUMBLOCKS = 7.2f; fLOWRANGEEXTENT = 160.0f;
		break;
	case 4:// DARK effect
		nHIGH = 180; fFADEOVERNUMBLOCKS = 7.2f; fLOWRANGEEXTENT = 130.0f;
		break;
	}
}

// Once-off setup during game init
void djEffectFauxShadowingAroundHero::InitEffect()
{
	if (m_pImgShadows != nullptr) return;

	// [Re]apply setting values [dj2022-11]
	ApplyIntensity();//SetIntensity(m_nEnabledIntensity);

	// Basically a black image with 0-255 'shades' of alpha transparency
	m_pImgShadows = new djImage(SHADSIZE * 16, SHADSIZE * 16, 32);
	int nAlpha = 255;//start with darkness at top left
	for (int y = 0; y < 16; ++y)
	{
		for (int x = 0; x < 16; ++x)
		{
			// Black pixel, with different alpha
			int nPixel = ((unsigned int)(nAlpha & 0xFF) << 24); //fixme which bits should be alpha?
			for (unsigned int i = 0; i < SHADSIZE; ++i)
			{
				for (unsigned int j = 0; j < SHADSIZE; ++j)
				{
					//slight and subtle random noise so it doesn't look too smooth
					int nRandNoise = (rand() % 3);
					m_pImgShadows->PutPixel(x * SHADSIZE + i, y * SHADSIZE + j, nPixel
						+ ((nRandNoise<<0) + (nRandNoise<<8) + (nRandNoise<<16))
					);
				}
			}
			--nAlpha;
		}
	}
	// Create corresponding hardware surface image
	m_pShadows = (SDL_Surface*)djCreateImageHWSurface(m_pImgShadows);
}

void djEffectFauxShadowingAroundHero::CleanupEffect()
{
	if (m_pImgShadows)
	{
		djDestroyImageHWSurface(m_pImgShadows);// Remove from cache we don't need anymore
		djDEL(m_pImgShadows);
	}
	m_pShadows = nullptr;
}

#endif//#idef djEFFECT_VIEWPORTSHADOW
