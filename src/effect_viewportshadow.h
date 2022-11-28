// effect_viewportshadow.h
// Copyright (C) 1995-2022 David Joffe / Dave Gnukem project
//
// dj2022-11 just refactoring this recent new effect from game.cpp into separate .h/cpp files for neatness as game.cpp getting a bit long and cluttered
//

/*--------------------------------------------------------------------------*/
#ifndef _dj_EFFECT_VIEWPORTSHADOW_H_
#define _dj_EFFECT_VIEWPORTSHADOW_H_

#include "config.h"

//dj2022-11 adding this into official but still off by default for now as is a bit a bit beta and needs more testing and not sure if i like it yet .. dj2022-11:
#define djEFFECT_VIEWPORTSHADOW

#ifdef djEFFECT_VIEWPORTSHADOW

#include "djgraph.h"

class djImage;

// Slightly experimental 'faux shadowing around hero' shadowing effect - not quite sure if I like this or not.
// Sometimes, sometimes not, some things about it I like, some things maybe not .. could maybe be tweaked and improved .. [dj2022-11]
// [Ideally at some stage want to try experimental more 'proper' lightmap effect also across level - someday - low prior - dj2022]
// 
// THESE EFFECTS SHOULD LIVE IN SEPARATE CPP/H FILES
// dj2020-07 Playing around with a new visual effect to try improve the graphics slightly ...
// subtle 'shadowing' that is focused around hero as he moves around viewport .. so walls behind
// hero aren't perfectly uniform, and get slightly darker toward corners, given a crude 'pretend effect'
// of lighting in the room. Not sure if this sucks? Will maybe add it as an option and let users decide.
// I think I overall prefer it on, to off. But it needs more testing before I add it to the real game.
class djEffectFauxShadowingAroundHero
{
public:
	djEffectFauxShadowingAroundHero();
	virtual ~djEffectFauxShadowingAroundHero();

	djImage* m_pImgShadows = nullptr;

	// NOT OUR POINTER WE DON'T DELETE IT (CACHE DOES) we just want a copy but it can change sometimes maybe [dj2022 ingame fullscreen toggle? not sure]
	SDL_Surface* m_pShadows = nullptr;
	void OnDrawFrameStart()
	{
		// NB even if we already have m_pShadows non-null the point is to re-get it (in case the 'cache' HW surface 'manager' thing flushed the surface and we need a new one) [dj2022-11 see notes around ingame fullscreen toggle etc.]
		// Get from cache
		if (m_pImgShadows)
		{
			// This name djCreate.. is now misleading most the time (if not all) it's not creating it just getting it from cache
			m_pShadows = (SDL_Surface*)djCreateImageHWSurface(m_pImgShadows);
		}
	}
	/*(virtual??)*/ void DrawEffect(djVisual* pVis, const int nLevelBlockX, const int nLevelBlockY,
		// hmm these values precise meaning etc. anmd how they're passed in could maybe use a re-think if/when we start abstracting away for more different games .. ideally we don't want *generic* effect code to "know about" crufty stuff like Dave Gnukem 1 specific "xo_small" viewport scrolling stuff (which itself is specifically meant to be there for the Duke Nukem 1 retro vibe)
		const int nXOffset, const int nYOffset,
		// position in world space of top left of viewport (in pixels) (doesn't change over a single frame for all blocks)
		const int nViewportOriginX, const int nViewportOriginY,
		// 
		const float fBlockWorldXStart, const float fBlockWorldYStart
	);

	// we shouldn't keep a pointer to the surface if it can change on fullscreentoggle? hm SDL_Surface* m_pShadows = nullptr;
	// either we must get again after fullscreen toggle or get before using [via map] but that's not ideal speedwise ..
	//void OnFullscreenToggle();
	//void OnStartDraw();

	// USER SETTING. 0=off, or 1, 2, 3 (currently .. this is very beta-ish) [or fo rnow just off/on .. later add more]
	unsigned int m_nEnabledIntensity = 1;// 0;
	void ApplyIntensity();

	// dj2022 this is slighty untidy sorry, was still playing around with this effect .. 

	int nHIGH = 180;
	float fFADEOVERNUMBLOCKS = 7.2f;
	float fLOWRANGEEXTENT = 130.0f;
	//unsigned int SHADSIZE = 4;// 4;2//should be an even number .. low numbers like 2 look good and smooth, 4 looks blocky but probably is faster for slow computer .. not sure what would be best as on my computer it's all fast enough [dj2022-11]
	const unsigned int SHADSIZE = 2;// 4;2//should be an even number .. low numbers like 2 look good and smooth, 4 looks blocky but probably is faster for slow computer .. not sure what would be best as on my computer it's all fast enough .. should maybe be a setting and/or have graphics 'profiles' e.g. fast or 'highest qualty' [dj2022-11]

	/*
	//dj2022-11 some quick n crude subjective values from quick testing .. needs more thought and testing but let's run with this for now:
	// DARK effect: nHIGH=180, fFADEOVERNUMBLOCKS = 7.2f, fLOWRANGEEXTENT = 130.0f
	// SUBTLE, light effect:  nHIGH=262, fFADEOVERNUMBLOCKS = 7.2f, fLOWRANGEEXTENT = 110.0f
	// ALSO OK, light effect: nHIGH=262, fFADEOVERNUMBLOCKS = 7.2f, fLOWRANGEEXTENT = 160.0f

	const float fLOWRANGEEXTENT = 130.0f; // Higher numbers toward 250 make for very dark 'corners' .. lower numbers around 100 etc. make for more subtle effect
	*/
	void SetIntensity(unsigned int nIntensity);

	// Once-off setup on start playing game
	virtual void InitEffect();
	// Cleanup on stop playing game
	virtual void CleanupEffect();
};

//gross global
extern djEffectFauxShadowingAroundHero g_Effect;

#endif//#ifdef djEFFECT_VIEWPORTSHADOW

#endif//_dj_EFFECT_VIEWPORTSHADOW_H_
