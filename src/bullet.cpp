/*
bullet.cpp

Copyright (C) 2002-2018 David Joffe

License: GNU GPL Version 2
*/

#include "config.h"
#include "bullet.h"
#include "djgraph.h"
#include "graph.h"
#include "mission.h"
#include "thing.h"
#ifdef EXPERIMENTAL_SPRITE_AUTO_DROPSHADOWS
#include "graph.h"//DRAW_SPRITEA_SHADOW
#endif

CBullet::CBullet()
{
	x = 0;
	y = 0;
	dx = 0;
	dy = 0;
	nAnim = 0;
	bDrawnOnce = false;
	eType = BULLET_HERO;
}

void CBullet::Tick()
{
	// NB, NOTE [dj2018-01] In theory the bullet move belongs in here,
	// but it's no longer in here due to refactoring - see the new
	// UpdateBullets() function. Could later try move things in here
	// but not a priority.
	nAnim = (nAnim+1)%4;
}

void CBullet::Draw()
{
	// Draw 'fire animation' if just shot, else draw bullet
	if (bDrawnOnce)
	{
		if (eType==BULLET_HERO)
		{
#ifdef EXPERIMENTAL_SPRITE_AUTO_DROPSHADOWS
			DRAW_SPRITEA_SHADOW(pVisView,
				5,
				nAnim,
				1 + WORLDX2VIEW(x),
				1 + WORLDY2VIEW(y)-4,
				16,16
			);
#endif
			djgDrawImageAlpha(pVisView,
				g_pCurMission->GetSpriteData(5)->m_pImage,
				/*bDrawnOnce ? */((nAnim)%16)*16,// : (dx<0 ? 4 : 5)*16,
				((0)/16)*16,
				WORLDX2VIEW(x),
				WORLDY2VIEW(y)-4,
				BLOCKW,
				BLOCKH);
		}
		else
		{
			djgDrawImageAlpha(pVisView,
				g_pCurMission->GetSpriteData(5)->m_pImage,
				/*bDrawnOnce ? */0,//  : (dx<0 ? 4 : 5)*16,
				/*bDrawnOnce ? */32,// : ((0)/16)*16,
				WORLDX2VIEW(x),
				WORLDY2VIEW(y)-4,
				BLOCKW,
				BLOCKH);
		}
	}
	else
	{
		if (eType==BULLET_HERO)
		{
			djgDrawImageAlpha(pVisView,
				g_pCurMission->GetSpriteData(5)->m_pImage,
				(dx<0 ? 4 : 5)*16,
				((0)/16)*16,
				WORLDX2VIEW(dx < 0 ? x - 16 : x + 16),
				WORLDY2VIEW(y)-4,
				BLOCKW,
				BLOCKH);
		}
		else
		{
			djgDrawImageAlpha(pVisView,
				g_pCurMission->GetSpriteData(5)->m_pImage,
				(dx<0 ? 4 : 5)*16,
				((0)/16)*16,
				WORLDX2VIEW(x),
				WORLDY2VIEW(y)-4,
				BLOCKW,
				BLOCKH);
		}
	}

	bDrawnOnce = true;
}
