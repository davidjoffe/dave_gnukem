/*
bullet.cpp

Copyright (C) 2002-2023 David Joffe
*/

#include "config.h"
#include "bullet.h"
#include "djgraph.h"
#include "graph.h"
#include "mission.h"
#include "thing.h"
#ifdef djSPRITE_AUTO_DROPSHADOWS
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

CBullet::~CBullet()
{
}

void CBullet::Tick(float fDeltaTime_ms)
{
	// NB, NOTE [dj2018-01] In theory the bullet move belongs in here,
	// but it's no longer in here due to refactoring - see the new
	// UpdateBullets() function. Could later try move things in here
	// but not a priority.
	nAnim = (nAnim+1)%4;
}

void CBullet::Draw(float fDeltaTime_ms)
{
	// Draw 'fire animation' if just shot, else draw bullet
	if (bDrawnOnce)
	{
		//dj2019-07: The simplest/quickest/only way to understand all these offsets etc. for the sprites is to look at the sprite image:
		if (eType==BULLET_HERO)
		{
#ifdef djSPRITE_AUTO_DROPSHADOWS
			DRAW_SPRITEA_SHADOW(pVisView,
				5,
				nAnim,
				1 + WORLDX2VIEW(x),
				1 + WORLDY2VIEW(y)-4,
				BLOCKW,BLOCKH
			);
#endif
			djgDrawImageAlpha(pVisView,
				g_pCurMission->GetSpriteData(5)->m_pImage,
				/*bDrawnOnce ? */((nAnim)%16)*BLOCKW,// : (dx<0 ? 4 : 5)*16,
				0,
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
				/*bDrawnOnce ? */BLOCKH*2,// : ((0)/16)*16,
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
				(dx<0 ? 4 : 5)*BLOCKW,
				0,
				//? why the offset here?(dj2019-07)
				WORLDX2VIEW(dx < 0 ? x - 16 : x + 16),
				// why -4? seems a fudgy offset to fudge-offset another offset that's wrong elsewhere?  not sure - check - dj2019-07
				WORLDY2VIEW(y)-4,
				BLOCKW,
				BLOCKH);
		}
		else
		{
			djgDrawImageAlpha(pVisView,
				g_pCurMission->GetSpriteData(5)->m_pImage,
				(dx<0 ? 4 : 5)*BLOCKW,
				0,
				WORLDX2VIEW(x),
				WORLDY2VIEW(y)-4,
				BLOCKW,
				BLOCKH);
		}
	}

	bDrawnOnce = true;
}
