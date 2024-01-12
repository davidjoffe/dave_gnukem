/*!
\file    thing_monsters.h
\brief   Monster-type things (CThing-derived game entities), e.g. robots

Copyright (C) 1999-2024 David Joffe
*/
/*--------------------------------------------------------------------------*/
/* Created dj 2017/07/29 - moving monster-type things into own file(s) */
/*--------------------------------------------------------------------------*/
#ifndef _THING_MONSTERS_H_
#define _THING_MONSTERS_H_

#include "config.h"
#include "thing.h"

/*-----------------------------------------------------------*/
class CMonster : public CThing
{
public:
	CMonster();
	virtual void Initialize(int a, int b) override;
	virtual int Tick(float fDeltaTime_ms) override;
	virtual int OnHeroShot() override;

	// hmm this is a monster thing? or should all things have a 'killable' flag? so we don't ahve override flag here [yet] unless in future we add to CThing
	virtual int OnKilled();
protected:
	int m_nStrength;//"Health" of the monster
	int m_nXDir; // Direction on x axis: -1=face left, 1=face right
	int m_nNoShootCounter;// This ideally shouldn't really be in the base
	// class because not all monsters shoot, there are better ways design-wise
	// to do this, but maybe not worth it for small game like this [dj2017-08]
	int m_nFlickerCounter;//Flicker if hurt [some monsters only, if effect desired]
};
/*-----------------------------------------------------------*/
/*!
\class CRobot
\nosubgrouping

Monster - dumb robot
*/
class CRobot : public CMonster
{
public:
	CRobot();
	virtual int HeroOverlaps() override;
	virtual void Draw(float fDeltaTime) override;
	virtual int Tick(float fDeltaTime) override;
	virtual void Initialize(int a, int b) override;
	virtual int OnKilled() override;
protected:
	int m_nType; // Currently 0=normal robot, 1=fireball thingy
	int m_nHeightOffset; // 0 for robot, -15 for fireball
};
/*-----------------------------------------------------------*/
class CFlyingRobot : public CRobot
{
public:
	CFlyingRobot();
	virtual int HeroOverlaps() override;
	virtual void Draw(float fDeltaTime_ms) override;
	virtual int Tick(float fDeltaTime) override;
	virtual int OnKilled() override;
	virtual void Initialize(int a, int b) override;
protected:
	int m_nDieAnim;
	//int m_nDir;//Direction (-1 or 1)
	int m_nMoveEveryNthFrameX;
	int m_nMoveEveryNthFrameY;
	int m_nDieAnimLength;
	bool IsDying() const { return m_nDieAnim>0; }
};
/*-----------------------------------------------------------*/
/*!
\class CRabbit
\nosubgrouping

This constitutes the approximate equivalent of DN1 rabbits (for now, or maybe permanently, this is just a sort of 'evil Tux')
*/
class CRabbit : public CMonster
{
public:
	CRabbit();
	virtual void Initialize(int a, int b) override;
	virtual void Draw(float fDeltaTime_ms) override;
	virtual int Tick(float fDeltaTime) override;
	virtual int HeroOverlaps() override;
	virtual int OnHeroShot() override;
protected:
	int m_nWalkAnimOffset;
	int m_nWalkAnimOffsetUpdateCounter;
};
/*-----------------------------------------------------------*/
/*!
\class CHighVoltage
\nosubgrouping

High-voltage "barrier" that must be shot multiple times to destroy before hero can pass through. Touching it results in immediate death.
*/
class CHighVoltage : public CMonster
{
public:
	CHighVoltage();
	virtual void Initialize(int a, int b) override;
	virtual void Draw(float fDeltaTime_ms) override;
	virtual int Tick(float fDeltaTime) override;
	virtual int HeroOverlaps() override;
	virtual int OnHeroShot() override;
protected:
	int m_nHeight;//Height (in game blocks) - this is comparable to the width of the crumbling floors (same principle, just vertical instead)
};
/*-----------------------------------------------------------*/
class CCannon : public CMonster
{
public:
	CCannon();
	virtual void Initialize(int a, int b) override;
	virtual int Tick(float fDeltaTime) override;
	virtual void Draw(float fDeltaTime_ms) override;
	virtual int HeroOverlaps() override;
	virtual int OnHeroShot() override;
	virtual int OnKilled() override;
};
/*-----------------------------------------------------------*/
/*!
\class CCrawler
\nosubgrouping

Green crawler monster, clings to wall going up and down.
*/
class CCrawler : public CThing
{
public:
	CCrawler();
	virtual int Tick(float fDeltaTime) override;
	virtual void Draw(float fDeltaTime_ms) override;
	virtual int OnHeroShot() override;
	virtual int HeroOverlaps() override;
	virtual void Initialize(int b0, int b1) override;
protected:
	int m_nDir; // direction
	int m_nXDir; // -1=face left, 1=face right
};
/*-----------------------------------------------------------*/
/*!
\class CSpike
\nosubgrouping

Spike in ground, hurts hero.

Two types; static (extra[0]=0), and pop-up (extra[0]=1)
*/
class CSpike : public CThing
{
public:
	CSpike();
	virtual int Tick(float fDeltaTime_ms) override;
	virtual void Draw(float fDeltaTime_ms) override;
	virtual int HeroOverlaps() override;
	virtual void Initialize(int b0, int b1) override;
protected:
	int m_nType;
	int m_nSpikePopupCount;
};
/*-----------------------------------------------------------*/
class CJumpingMonster : public CMonster
{
public:
	CJumpingMonster();
	virtual int Tick(float fDeltaTime_ms) override;
	virtual void Draw(float fDeltaTime_ms) override;
	virtual void Initialize(int a, int b) override;
	virtual int HeroOverlaps() override;
	virtual int OnKilled() override;
protected:
	int m_nJumpingIndex;
	bool m_bFalling;
	bool IsJumping() const { return m_nJumpingIndex>=0; }
	bool m_bLinedUpToShoot;
};
/*-----------------------------------------------------------*/
/*!
\class CDrProton
\nosubgrouping

The main 'boss' / baddie etc., Dr Proton. In our game, this is Dr Proetton.

Note we don't override OnKilled() because you specifically don't kill him .. he gets away. So we can have him in a sequel of course. [dj2017-08]
*/
class CDrProton : public CMonster
{
public:
	CDrProton();
	virtual ~CDrProton();
	virtual int Tick(float fDeltaTime_ms) override;
	virtual void Draw(float fDeltaTime_ms) override;
	virtual void Initialize(int a, int b) override;
	virtual int HeroOverlaps() override;
	virtual int OnHeroShot() override;

	static bool GameEnding() { return (g_pGameEnding!=NULL); }
	static CDrProton* GetDrProton() { return g_pGameEnding; }

protected:
	int m_bEscaping;
	//int m_nOrigX;//Stay close to original X
	int m_nDesiredXRandomVariation;
	int m_nDesiredYRandomVariation;
	bool m_bActivated;	
	static CDrProton* g_pGameEnding;
};
/*-----------------------------------------------------------*/
// a few ideas for things from DN1 that didn't quite make it into DG v1: [dj2022-11] (helicopter is self-explanatory, there are helicopters in DN1 but can't remember what loop and spinning things are)
/*
class CLoopThing : public CMonster
{
public:
};
class CSpinningThing : public CMonster
{
public:
};
class CHelicopter : public CMonster
{
public:
};
*/

#endif
