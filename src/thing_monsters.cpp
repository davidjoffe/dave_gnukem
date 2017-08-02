/*--------------------------------------------------------------------------*/
// thing_monsters.cpp
/*
Copyright (C) 2000-2017 David Joffe

License: GNU GPL Version 2
*/
/*--------------------------------------------------------------------------*/
#include "thing_monsters.h"
#include "block.h"
#include "mission.h"//GET_EXTRA
#include "graph.h"//pVisView etc.

/*-----------------------------------------------------------*/
//! Macro that "thing"s can use (in the .cpp file) to register handlers in the thing factory
#define ALLOCATE_FUNCTION(ClassName)	\
	CThing *ALLOCATE##ClassName() \
	{ \
		return new ClassName; \
	}
#define REGISTER_THING2(ClassName, nTypeID)	g_ThingFactory.Register(nTypeID, ALLOCATE##ClassName, NULL)
/*-----------------------------------------------------------*/
ALLOCATE_FUNCTION(CRobot);
ALLOCATE_FUNCTION(CFlyingRobot);
ALLOCATE_FUNCTION(CRabbit);
ALLOCATE_FUNCTION(CHighVoltage);
/*-----------------------------------------------------------*/
// Register these classes in the object factory, for creating instances on level load etc.
// Register these at runtime (can't use REGISTER_THING as it seems to not work on Windows/MSVC, I think because it gets called before the object factory's constructor gets called - dj2017-07)
void RegisterThings_Monsters()
{
	REGISTER_THING2(CRobot,			TYPE_ROBOT);
	REGISTER_THING2(CFlyingRobot,	TYPE_FLYINGROBOT);
	REGISTER_THING2(CRabbit,		TYPE_RABBIT);
	REGISTER_THING2(CHighVoltage,	TYPE_HIGHVOLTAGE);
}
/*-----------------------------------------------------------*/

/*-----------------------------------------------------------*/
CRobot::CRobot()
{
	m_bShootable = true;
	m_nDX = 0;
	m_nXOffset = 0;
	SetActionBounds(0,0,15,15);
	m_bFalls = true;
	m_nType = 0;
	m_nHeightOffset = 0;
	m_nNoShootCounter = 0;
	SetLayer(LAYER_4);
}

int CRobot::HeroOverlaps()
{
	if (!HeroIsHurting())
	{
		update_health(-1);
		HeroSetHurting();
	}
	return 0;
}

void CRobot::Draw()
{
	if (m_nType==0) // Robot?
	{
		DRAW_SPRITE16A(pVisView, m_a, 32/*m_b*/ + anim4_count, CALC_XOFFSET(m_x,0) + m_nXOffset, CALC_YOFFSET(m_y));
	}
	else if (m_nType==1) // Fireball?
	{
		DRAW_SPRITE16A(pVisView, m_a, (m_nDX>0 ? 16 : 20) + anim4_count - 16, CALC_XOFFSET(m_x,0) + m_nXOffset, CALC_YOFFSET(m_y-1));
		DRAW_SPRITE16A(pVisView, m_a, (m_nDX>0 ? 16 : 20) + anim4_count,      CALC_XOFFSET(m_x,0) + m_nXOffset, CALC_YOFFSET(m_y));
	}
}

int CRobot::Tick()
{
	if (m_nXOffset==0)
	{
		if ((check_solid(m_x + m_nDX, m_y)) || (!check_solid(m_x + m_nDX, m_y + 1)))
		{
			m_nDX = -m_nDX;
			return 0;
		}
		// Don't move left/right if nothing below us (i.e. we're busy falling)
		if (!check_solid(m_x, m_y+1))
			return 0;
	}

	m_nXOffset += m_nDX * 4;//(m_nType==0 ? 4 : 2);
	if (ABS(m_nXOffset)>=16)
	{
		m_nXOffset = 0;
		m_x += m_nDX;
	}
	SetActionBounds (m_nXOffset,m_nHeightOffset,m_nXOffset+15,15);
	SetVisibleBounds(m_nXOffset,m_nHeightOffset,m_nXOffset+15,15);
	SetShootBounds  (m_nXOffset,m_nHeightOffset,m_nXOffset+15,15);

	if (m_nType==0) // The robot shoots, not the fireball
	{
		if (m_nNoShootCounter>0)
		{
			m_nNoShootCounter--;
		}
		else
		{
			// Only shoot if we're in the view
			if (IsInView())
			{
				// Check we're facing hero, and hero is more or less at same height ..
				if ((ABS(y-m_y)<3) && ((m_nDX<0 && x<=m_x) || (m_nDX>0 && x>=m_x)))
				{
					if ((rand()%50)<=2)
					{
						MonsterShoot(m_x*16+m_nXOffset, m_y*16, m_nDX<0?-16:16);
						m_nNoShootCounter = 36;// fixme, should be time, not frame count
					}
				}
			}
		}
	}

	return 0;
}

int CRobot::OnHeroShot()
{
	update_score(100, m_x, m_y);
	AddThing(CreateExplosion(m_x*16, m_y*16));
	return THING_DIE;
}

void CRobot::Initialize(int b0, int b1)
{
	m_nDX = (GET_EXTRA(b0,b1,0)==0 ? -1 : 1);
	m_nType = GET_EXTRA(b0,b1,2);
	if (m_nType==1) // Only robots can be shot, not fireballs
	{
		m_bShootable = false;
		m_nHeightOffset = -15;
	}
}
/*-----------------------------------------------------------*/
CFlyingRobot::CFlyingRobot() :
	CRobot(),
	m_nStrength(1),
	m_nDieAnim(-1),
	m_nXOffset(0),
	m_nYOffset(0),
	m_nMoveEveryNthFrameX(0),
	m_nMoveEveryNthFrameY(0),
	m_nDieAnimLength(20)
{
	m_bFalls = false;
	SetLayer(LAYER_4);
	m_nNoShootCounter = 6;//<- see notes in Tick
	m_nDX = 1;

	//fixmeMED Bullets go through doors (monster bullets do, hero's bullets don't :/ .. either none should (I think) or both)
}
int CFlyingRobot::HeroOverlaps()
{
	// NB, since we don't 'die' immediately (first show death 'animation'), we only
	// hurt hero if not dead during HeroOverlaps() (i.e. if in death animation we're
	// not harmful.)
	if (!IsDying() && !HeroIsHurting())
	{
		update_health(-1);
		HeroSetHurting();
		
		// We initiate dying if hero touches us
		update_score(100, m_x, m_y);
		AddThing(CreateExplosion(m_x*16, m_y*16));
		m_nDieAnim = m_nDieAnimLength;
	}
	return 0;
}
int CFlyingRobot::OnHeroShot()
{
	if (m_nDieAnim<0)
	{
		update_score(100, m_x, m_y);
		AddThing(CreateExplosion(m_x*16, m_y*16));
		m_nDieAnim = m_nDieAnimLength;
	}
	return 0;
}
void CFlyingRobot::Draw()
{
	if (IsDying())
	{
		DRAW_SPRITE16A(pVisView, m_a, m_b + anim4_count + 4, -12 + CALC_XOFFSET(m_x,0) + m_nXOffset - (m_nDieAnimLength-m_nDieAnim)*3, 12 + CALC_YOFFSET(m_y) + (((m_nDieAnimLength-m_nDieAnim)*(m_nDieAnimLength-m_nDieAnim))/2) + m_nYOffset);
	}
	else
	{
		// Spriteset has 8 sprites in a row: First 4 is facing right, next 4 facing left
		if (m_nDX>0)//Facing right?
		{
			DRAW_SPRITE16A(pVisView, m_a, m_b + anim4_count    , CALC_XOFFSET(m_x,0) + m_nXOffset, CALC_YOFFSET(m_y) + m_nYOffset);
		}
		else//Facing left
		{
			DRAW_SPRITE16A(pVisView, m_a, m_b + anim4_count + 4, CALC_XOFFSET(m_x,0) + m_nXOffset, CALC_YOFFSET(m_y) + m_nYOffset);
		}
	}
}
int CFlyingRobot::Tick()
{
	// Busy dying? Show death animation
	if (IsDying())
	{
		// Increment animation offset towards final offset, when this reaches 0 we actually die (as a CThing)
		if (--m_nDieAnim<=0)
			return THING_DIE;
		return CThing::Tick();
	}

	// Normal ('alive') tick/update (which we only do if in view)
	if (!IsInView())
		return CThing::Tick();

	// Turn to face (and move in) direction of hero
	if (x < m_x - 1)
		m_nDX = -1;
	else if (x > m_x + 1)
		m_nDX = 1;

	// Move slowly in direction of hero along both axes (but slower in y axis)
	//if (++m_nMoveEveryNthFrameX>1)
	{

		if (m_nXOffset==0//<- This seems dubious, surely should check solid if YOffset!=0 [dj2017-06]
			&& check_solid(m_x+m_nDX, m_y)
			)
		{
			//Do nothing
		}
		else
		{
			//m_nMoveEveryNthFrameX = 0;
			m_nXOffset += m_nDX;
			if (ABS(m_nXOffset)>=16)
			{
				m_nXOffset = 0;
				m_x += m_nDX;
			}
		}
	}
	if (++m_nMoveEveryNthFrameY>4)
	{
		m_nMoveEveryNthFrameY = 0;

		int nYDiffDir = 0;
		if (y < m_y - 1)
			nYDiffDir = -1;
		else if (y > m_y + 1)
			nYDiffDir = 1;

		if (nYDiffDir!=0)
		{
			if (m_nYOffset==0//<- This seems dubious, surely should check solid if YOffset!=0 [dj2017-06]
				&& check_solid(m_x, m_y+nYDiffDir)
				)
			{
				//Do nothing
			}
			else
			{
				m_nYOffset += nYDiffDir;
				if (ABS(m_nYOffset)>=16)
				{
					m_nYOffset = 0;
					m_y += nYDiffDir;
				}
			}
		}
	}


	SetActionBounds (m_nXOffset,m_nYOffset,m_nXOffset+15,m_nYOffset+15);
	SetVisibleBounds(m_nXOffset,m_nYOffset,m_nXOffset+15,m_nYOffset+15);
	SetShootBounds  (m_nXOffset,m_nYOffset,m_nXOffset+15,m_nYOffset+15);

	if (m_nNoShootCounter>0)
	{
		m_nNoShootCounter--;
	}
	else
	{
		// Only shoot if we're in the view
		if (IsInView())
		{
			// Check we're facing hero, and hero is more or less at same height ..
			if ((ABS(y-m_y)<3) && ((m_nDX<0 && x<=m_x) || (m_nDX>0 && x>=m_x)))
			{
				if ((rand()%50)<=2)
				{
					MonsterShoot(m_x*16+m_nXOffset + (m_nDX*16), m_y*16+m_nYOffset, m_nDX<0?-16:16);
					m_nNoShootCounter = 24;// fixme, should be time, not frame count
				}
			}
		}
		else
			m_nNoShootCounter = 6;//<- It feels slightly 'unfair' if you walk hurriedly off left/right
		// into one of these and get shot .. create a few frames 'buffer' when we come into view before
		// we'll actually shoot? Making this number higher should make these flying robots slightly
		// 'easier' in this respect, make it lower to make them slightly more difficult. Or maybe might
		// decide after more testing not to do this at all in future. [dj2017-06-30]
		// NB note this also implies we set to same in *constructor*.
	}

	return CThing::Tick();
}
/*-----------------------------------------------------------*/
CRabbit::CRabbit() :
	m_nXDir(1),
	m_nWalkAnimOffset(0),
	m_nWalkAnimOffsetUpdateCounter(0)
{
}
void CRabbit::Initialize(int a, int b)
{
	CThing::Initialize(a,b);
	// So if you have a bunch, they aren't necessarily all walking exactly in sync:
	m_nWalkAnimOffset = rand()%4;
	SetVisibleBounds(0,-BLOCKH,BLOCKW*2,BLOCKH-1);
	SetShootBounds  (0,-BLOCKH,BLOCKW*2,BLOCKH-1);
	SetActionBounds (0,-BLOCKH,BLOCKW*2,BLOCKH-1);
	SetLayer(LAYER_4);
	m_bShootable = true;
}
void CRabbit::Draw()
{
	const int a=6;//Spriteset number
	const int b=(m_nXDir>0 ? 6*16 : 6*16 + 8);//Sprite number within spriteset
	int x = CALC_XOFFSET(m_x,0) + m_xoffset;
	int y = CALC_YOFFSET(m_y  ) + m_yoffset;
	DRAW_SPRITEA(pVisView,
		a   ,b+m_nWalkAnimOffset*2,
		x   ,y-16,
		32  ,  32);
}
int CRabbit::Tick()
{
	if (++m_nWalkAnimOffsetUpdateCounter>=2)
	{
		++m_nWalkAnimOffset;
		m_nWalkAnimOffset = m_nWalkAnimOffset % 4;
		m_nWalkAnimOffsetUpdateCounter = 0;

		// IF BUMP INTO SOMETHING SOLID, REVERSE DIRECTION
		if ((check_solid(m_x + m_nXDir, m_y)) || (!check_solid(m_x + m_nXDir, m_y + 1)))
		{
			m_nXDir = -m_nXDir;
			m_xoffset = 0;
		}

		// Walking left?
		if (m_nXDir<0)
		{
			m_xoffset += m_nXDir*2;
			while (m_xoffset < -BLOCKW)
			{
				m_xoffset += BLOCKW;
				--m_x;
			}
		}
		// Walking right?
		else if (m_nXDir>0)
		{
			m_xoffset += m_nXDir*2;
			while (m_xoffset > BLOCKW)
			{
				m_xoffset -= BLOCKW;
				++m_x;
			}
		}
	}
	
	return CThing::Tick();
}
int CRabbit::HeroOverlaps()
{
	if (!HeroIsHurting())
	{
		update_health(-1);
		HeroSetHurting();
	}
	// Start death animation [TODO]
	update_score(100, m_x, m_y);
	AddThing(CreateExplosion(m_x*BLOCKW + 8, m_y*BLOCKH - 8));
	return THING_DIE;
}
int CRabbit::OnHeroShot()
{
	// Start death animation [TODO]
	update_score(200, m_x, m_y);// [fixmeLOW] Should get more points for shooting than stupidly walking into it? [dj2017]
	AddThing(CreateExplosion(m_x*BLOCKW + 8, m_y*BLOCKH - 8));
	return THING_DIE;
}
/*-----------------------------------------------------------*/
CHighVoltage::CHighVoltage() :
	m_nStrength(10),
	m_nHeight(1)
{
}
void CHighVoltage::Initialize(int a, int b)
{
	CThing::Initialize(a,b);
	SetLayer(LAYER_4);
	m_bShootable = true;

	// Expand downwards until we hit solid
	m_nHeight = 1;
	while (m_y+m_nHeight<LEVEL_HEIGHT&& !check_solid(m_x,m_y+m_nHeight,false))
	{
		++m_nHeight;
	}
	SetVisibleBounds(0,0,BLOCKW-1,BLOCKH*m_nHeight-1);
	SetShootBounds  (0,0,BLOCKW-1,BLOCKH*m_nHeight-1);
	SetActionBounds (0,0,BLOCKW-1,BLOCKH*m_nHeight-1);
}
void CHighVoltage::Draw()
{
	for ( int i=0; i<m_nHeight; ++i )
	{
		int a=m_a;//Spriteset number
		// This animation is very crude, might need something better here
		//int b=m_b + ((anim4_count+m_nHeight)%4)*16;//Sprite number within spriteset
		int b=m_b + ((i+anim4_count)%4)*16;//Sprite number within spriteset
		int x = CALC_XOFFSET(m_x,0);
		int y = CALC_YOFFSET(m_y+i);
		DRAW_SPRITE16A(pVisView,a,b,x,y);
	}
}
int CHighVoltage::Tick()
{
	return CThing::Tick();
}
int CHighVoltage::HeroOverlaps()
{
	update_health(-1000);//Kill hero immediately if touch us
	return 0;
}
int CHighVoltage::OnHeroShot()
{
	if (--m_nStrength<=0)
	{
		// Actually we want to start death *animation* here [TODO]
		for ( int i=0; i<m_nHeight; ++i )
		{
			// These effects are a little unspectacular
			// This is very crude death animation, TODO, something fancier/nicer etc.
			if ( (i%2)==0 )
			{
				AddThing(CreateExplosion(m_x*BLOCKW,(m_y+i)*BLOCKH));
			}
		}
		djSoundPlay( g_iSounds[SOUND_EXPLODE] );
		return THING_DIE;
	}
	return 0;
}
/*-----------------------------------------------------------*/
