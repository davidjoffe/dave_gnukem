/*
bullet.cpp

Copyright (C) 2002-2018 David Joffe

License: GNU GPL Version 2
*/

#include "bullet.h"
#include "djgraph.h"
#include "graph.h"
#include "mission.h"
#include "thing.h"

CBullet::CBullet()
{
	x = 0;
	y = 0;
	dx = 0;
	dy = 0;
	nAnim = 0;
	bMovedOnce = false;
	bDrawnOnce = false;
	eType = BULLET_HERO;
}

void CBullet::Tick()
{
	if (bMovedOnce)
	{
		x += dx;
		y += dy;
		nAnim = (nAnim+1)%4;
	}
	bMovedOnce = true;
}

void CBullet::Draw()
{
	if (eType==BULLET_HERO)
	{
		djgDrawImageAlpha(pVisView,
			g_pCurMission->GetSpriteData(5)->m_pImage,
			bDrawnOnce ? ((nAnim)%16)*16 : (dx<0 ? 4 : 5)*16,
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
			bDrawnOnce ? 0  : (dx<0 ? 4 : 5)*16,
			bDrawnOnce ? 32 : ((0)/16)*16,
			WORLDX2VIEW(x),
			WORLDY2VIEW(y)-4,
			BLOCKW,
			BLOCKH);
	}
	bDrawnOnce = true;
}
