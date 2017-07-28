/*!
\file    thing_monsters.h
\brief   Monster-type things (CThing-derived game entities), e.g. robots

Copyright (C) 1999-2017 David Joffe

License: GNU GPL Version 2
*/
/*--------------------------------------------------------------------------*/
/* Created dj 2017/07/29 - moving monster-type things into own file(s) */
/*--------------------------------------------------------------------------*/
#ifndef _THING_MONSTERS_H_
#define _THING_MONSTERS_H_

#include "thing.h"

/*-----------------------------------------------------------*/
/*!
\class CRobot
\nosubgrouping

Monster - dumb robot
*/
class CRobot : public CThing
{
public:
	CRobot();
	virtual int HeroOverlaps();
	virtual void Draw();
	virtual int Tick();
	virtual int OnHeroShot();
	virtual void Initialize(int b0, int b1);
protected:
	int m_nXOffset;
	int m_nDX; // Either -1 or 1.
	int m_nType; // Currently 0=normal robot, 1=fireball thingy
	int m_nHeightOffset; // 0 for robot, -15 for fireball
	int m_nNoShootCounter;
};
/*-----------------------------------------------------------*/
class CFlyingRobot : public CRobot
{
public:
	CFlyingRobot();
protected:
	virtual int HeroOverlaps();
	virtual int OnHeroShot();
	virtual void Draw();
	virtual int Tick();
	int m_nStrength;
	int m_nDieAnim;
	//int m_nDir;//Direction (-1 or 1)
	int m_nXOffset;
	int m_nYOffset;
	int m_nMoveEveryNthFrameX;
	int m_nMoveEveryNthFrameY;
	int m_nDieAnimLength;
	bool IsDying() const { return m_nDieAnim>0; }
};
/*-----------------------------------------------------------*/

#endif
