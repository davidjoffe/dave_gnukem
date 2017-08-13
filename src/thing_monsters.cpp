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

// We define everything's position in blockx,blocky (m_x,m_y), plus a pixel offset relative to that (m_xoffset,m_yoffset).
// This implies there's more than one way to describe the same ultimate pixel coordinate, e.g.
// if blocksize is 16 pixels, then e.g.
// m_x 10 + m_xoffset 20
// is the same position as
// m_x 11 + m_xoffset 4 (i.e. 20 - 16)
// is also the same as
// m_x  9 + m_xoffset 20+16
// etc.
// This helper is just to help 'normalize' it to the smallest/simplest case.
// (Maybe a silly system, I don't know, but that's how it evolved, as initially everything was block units. dj2017-08)
#define NORMALIZEX\
	while (m_xoffset>=BLOCKW)\
	{\
		m_xoffset -= BLOCKW;\
		++m_x;\
	}\
	while (m_xoffset<=-BLOCKW)\
	{\
		m_xoffset += BLOCKW;\
		--m_x;\
	}
#define NORMALIZEX2(x,xoffset)\
	while ((xoffset)>=BLOCKW)\
	{\
		xoffset -= BLOCKW;\
		++(x);\
	}\
	while ((xoffset)<=-BLOCKW)\
	{\
		xoffset += BLOCKW;\
		--(x);\
	}
#define NORMALIZEY\
	while (m_yoffset>=BLOCKH)\
	{\
		m_yoffset -= BLOCKH;\
		++m_y;\
	}\
	while (m_yoffset<=-BLOCKH)\
	{\
		m_yoffset += BLOCKH;\
		--m_y;\
	}

/*-----------------------------------------------------------*/
//! Macro that "thing"s can use (in the .cpp file) to register handlers in the thing factory
#define ALLOCATE_FUNCTION(ClassName)	\
	CThing *ALLOCATE##ClassName() \
	{ \
		return new ClassName; \
	}
#define REGISTER_THING2(ClassName, nTypeID)	g_ThingFactory.Register(nTypeID, ALLOCATE##ClassName, NULL)
/*-----------------------------------------------------------*/
// Allocate functions for object factory
ALLOCATE_FUNCTION(CRobot);
ALLOCATE_FUNCTION(CFlyingRobot);
ALLOCATE_FUNCTION(CRabbit);
ALLOCATE_FUNCTION(CHighVoltage);
ALLOCATE_FUNCTION(CCannon);
ALLOCATE_FUNCTION(CJumpingMonster);
ALLOCATE_FUNCTION(CDrProton);
ALLOCATE_FUNCTION(CCrawler);
ALLOCATE_FUNCTION(CSpike);

// Register these classes in the object factory, for creating instances on level load etc.
// Register these at runtime (can't use REGISTER_THING as it seems to not work on Windows/MSVC, I think because it gets called before the object factory's constructor gets called - dj2017-07)
void RegisterThings_Monsters()
{
	REGISTER_THING2(CRobot,			TYPE_ROBOT);
	REGISTER_THING2(CFlyingRobot,	TYPE_FLYINGROBOT);
	REGISTER_THING2(CRabbit,		TYPE_RABBIT);
	REGISTER_THING2(CHighVoltage,	TYPE_HIGHVOLTAGE);
	REGISTER_THING2(CCannon,		TYPE_CANNON);
	REGISTER_THING2(CJumpingMonster,TYPE_JUMPINGMONSTER);
	REGISTER_THING2(CDrProton,		TYPE_DRPROTON);
	REGISTER_THING2(CCrawler,       TYPE_CRAWLER);
	REGISTER_THING2(CSpike,         TYPE_SPIKE);
}
/*-----------------------------------------------------------*/

/*-----------------------------------------------------------*/
CMonster::CMonster() :
	m_nStrength(1),
	m_nXDir(1),
	m_nNoShootCounter(1),
	m_nFlickerCounter(0)
{
}
void CMonster::Initialize(int a, int b)
{
	m_bShootable = true;
	m_bFalls = true;
	SetLayer(LAYER_4);
}

int CMonster::OnKilled()
{
	return 0;//Return THING_DIE if want to immediately die (but most will start some sort of 'death animation', and only thereafter die)
}
int CMonster::Tick()
{
	if (m_nFlickerCounter>0)
	{
		--m_nFlickerCounter;
	}
	return CThing::Tick();
}
int CMonster::OnHeroShot()
{
	if (m_nStrength>0)
	{
		--m_nStrength;

		m_nFlickerCounter = 4;//Hm, a bit weird, always set even if unused [low dj2017-08 not worth worrying about]

		if (m_nStrength==0)
		{
			return OnKilled();
		}
	}
	return 0;
}

/*-----------------------------------------------------------*/
CRobot::CRobot()
{
	m_bShootable = true;
	m_nXDir = 0;
	m_nType = 0;
	m_nHeightOffset = 0;
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
		DRAW_SPRITE16A(pVisView, m_a, 32/*m_b*/ + anim4_count, CALC_XOFFSET(m_x) + m_xoffset, CALC_YOFFSET(m_y)+m_yoffset);
	}
	else if (m_nType==1) // Fireball?
	{
		DRAW_SPRITEA(pVisView, m_a, (m_nXDir>0 ? 16 : 20) + anim4_count - 16, CALC_XOFFSET(m_x) + m_xoffset, CALC_YOFFSET(m_y-1)+m_yoffset, 16, 32);
	}
}

int CRobot::Tick()
{
	// NOTE: Say the robot is on shootable soft-solid blocks, but with m_xoffset partial e.g.:
	//
	//    [robot_]
	// [solid][solid]
	// [solid][solid]
	//
	// then if i shoot the top-left solid, I probably don't want it to immediately fall -
	// probably it should stay on the solid (unless its m_xoffset is 0 ie directly on top).
	// So this testing code looks at the value of m_xoffset, if 0, checks only the single
	// block below us; if non-zero, checks both blocks - if 'partially' solid below us,
	// regard basically as solid (i.e. don't fall).

	// FIRST TRY SEE IF WE SHOULD FALL?
	bool bSolidBelowUs = false;
	if (m_xoffset==0)
		bSolidBelowUs = check_solid(m_x,m_y+1);
	else if (m_xoffset<0)
		bSolidBelowUs = ( check_solid(m_x,m_y+1) || check_solid(m_x-1,m_y+1) );
	else
		bSolidBelowUs = ( check_solid(m_x,m_y+1) || check_solid(m_x+1,m_y+1) );
	bool bFalling = false;
	if (!bSolidBelowUs)
	{
		const int nFALLSPEED=4;
		m_yoffset += nFALLSPEED;
		NORMALIZEY;
		bFalling = true;
	}

	// DON'T DO LEFT/RIGHT MOVEMENT IN THIS TICK() IF WE JUST DID ANY FALLING THIS ROUND
	if (!bFalling)
	{
		// Flame guy speed is faster than robot [I really don't like the fact
		// designwise (old gross code) that robot 'is also' other things depending
		// on the m_nType]
		const int nSPEED = (m_nType==0 ? 3 : 5);
		int nXNew = m_x;
		int nXOffset = m_xoffset;
		nXOffset += m_nXDir * nSPEED;
		NORMALIZEX2(nXNew,nXOffset);

		if ((check_solid(nXNew+m_nXDir, m_y)) || (!check_solid(nXNew+m_nXDir, m_y + 1)))
		{
			// At ends of floor etc. re-"line-up" with block units ie set m_xoffset to 0
			m_nXDir = -m_nXDir;
			m_xoffset = 0;
			nXOffset = 0;
			m_x = nXNew;
			return 0;
		}
		// 'Apply' the move
		m_x = nXNew;
		m_xoffset = nXOffset;
	}

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
				if ((ABS(y-m_y)<3) && ((m_nXDir<0 && x<=m_x) || (m_nXDir>0 && x>=m_x)))
				{
					if ((rand()%100)<=3)
					{
						MonsterShoot(PIXELX, PIXELY, m_nXDir<0?-12:12);
						m_nNoShootCounter = 36;// fixme, should be time, not frame count
					}
				}
			}
		}
	}

	return 0;
}

int CRobot::OnKilled()
{
	update_score(100, m_x, m_y);
	AddThing(CreateExplosion(PIXELX,PIXELY,1));
	return THING_DIE;
}

void CRobot::Initialize(int a, int b)
{
	CMonster::Initialize(a,b);

	SetActionBounds(0,0,15,15);
	SetVisibleBounds(0,0,15,15);
	SetShootBounds  (0,0,15,15);

	m_bFalls = false;//The robot does fall, but we do our own falilng code [for now?] [dj2017-08-13]

	m_nXDir = (GET_EXTRA(a,b,0)==0 ? -1 : 1);
	m_nType = GET_EXTRA(a,b,2);
	if (m_nType==1) // Only robots can be shot, not fireballs
	{
		m_bShootable = false;
		m_nHeightOffset = -15;
	}
}
/*-----------------------------------------------------------*/
CFlyingRobot::CFlyingRobot() :
	CRobot(),
	m_nDieAnim(-1),
	m_nMoveEveryNthFrameX(0),
	m_nMoveEveryNthFrameY(0),
	m_nDieAnimLength(20)
{
	SetLayer(LAYER_4);
	m_nNoShootCounter = 6;//<- see notes in Tick
	m_nXDir = 1;
}
int CFlyingRobot::HeroOverlaps()
{
	// NB, since we don't 'die' immediately (first show death 'animation'), we only
	// hurt hero if not dead during HeroOverlaps() (i.e. if in death animation we're
	// not harmful.)
	if (!IsDying())
	{
		if (!HeroIsHurting())
		{
			update_health(-1);
			HeroSetHurting();
		}
		
		// We initiate dying if hero touches us
		update_score(100, m_x, m_y);
		AddThing(CreateExplosion(m_x*16, m_y*16));
		m_nDieAnim = m_nDieAnimLength;
	}
	return 0;
}
int CFlyingRobot::OnKilled()
{
	if (m_nDieAnim<0)//If we've 'just been killed' for the first time (um, as if we can be killed more than once)
	{
		update_score(100, m_x, m_y);
		AddThing(CreateExplosion(m_x*16, m_y*16));
		// Start the 'dying' animation 'countdown'
		m_nDieAnim = m_nDieAnimLength;
	}
	return 0;
}
void CFlyingRobot::Draw()
{
	if (IsDying())
	{
		DRAW_SPRITE16A(pVisView,
			m_a, m_b + anim4_count + 4,
			CALC_XOFFSET(m_x) + m_xoffset,
			CALC_YOFFSET(m_y) + m_yoffset
			);
	}
	else
	{
		// Spriteset has 8 sprites in a row: First 4 is facing right, next 4 facing left
		if (m_nXDir>0)//Facing right?
		{
			DRAW_SPRITE16A(pVisView, m_a, m_b + anim4_count    , CALC_XOFFSET(m_x) + m_xoffset, CALC_YOFFSET(m_y) + m_yoffset);
		}
		else//Facing left
		{
			DRAW_SPRITE16A(pVisView, m_a, m_b + anim4_count + 4, CALC_XOFFSET(m_x) + m_xoffset, CALC_YOFFSET(m_y) + m_yoffset);
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

		// Sort of falls off to the left and downwards, off the screen
		m_xoffset = -12 - (m_nDieAnimLength-m_nDieAnim)*4;
		m_yoffset += (((m_nDieAnimLength-m_nDieAnim)*(m_nDieAnimLength-m_nDieAnim))/2);

		return CThing::Tick();//Should this be CMonster::Tick()? CRobot::Tick()?
	}

	// Normal ('alive') tick/update (which we only do if in view)
	if (!IsInView())
		return CThing::Tick();

	// Turn to face (and move in) direction of hero
	if (x < m_x - 1)
		m_nXDir = -1;
	else if (x > m_x + 1)
		m_nXDir = 1;

	// Move slowly in direction of hero along both axes (but slower in y axis)
	//if (++m_nMoveEveryNthFrameX>1)
	{
		bool bSolidInFrontOfUsXAxis = false;
		if (m_xoffset==0)
		{
			if (m_yoffset>0)
				bSolidInFrontOfUsXAxis = check_solid(m_x+m_nXDir, m_y  ) || check_solid(m_x+m_nXDir, m_y+1);
			else if (m_yoffset<0)
				bSolidInFrontOfUsXAxis = check_solid(m_x+m_nXDir, m_y-1) || check_solid(m_x+m_nXDir, m_y  );
			else
				bSolidInFrontOfUsXAxis = check_solid(m_x+m_nXDir, m_y);
		}

		if (!bSolidInFrontOfUsXAxis)
		{
			//m_nMoveEveryNthFrameX = 0;
			m_xoffset += m_nXDir;
			NORMALIZEX;
		}
	}
	if (++m_nMoveEveryNthFrameY>4)
	{
		m_nMoveEveryNthFrameY = 0;

		// Work in pixel units here, simplifies things slightly. Compare our center-line, to hero center-line, vertically.
		int nOurYPixels = PIXELY + (BLOCKH/2);//Add half as we use our center-line vs hero center-line
		int nHeroYPixels = (y*BLOCKH + y_offset);
		int nYDiffDir = 0;
		if (nHeroYPixels < nOurYPixels)
			nYDiffDir = -1;
		else if (nHeroYPixels > nOurYPixels)
			nYDiffDir = 1;

		if (nYDiffDir!=0)
		{
			bool bSolidInFrontOfUsYAxis = false;
			if (m_xoffset>0)
				bSolidInFrontOfUsYAxis = check_solid(m_x, m_y+nYDiffDir) || check_solid(m_x+1, m_y+nYDiffDir);
			else if (m_xoffset<0)
				bSolidInFrontOfUsYAxis = check_solid(m_x, m_y+nYDiffDir) || check_solid(m_x-1, m_y+nYDiffDir);
			else
				bSolidInFrontOfUsYAxis = check_solid(m_x, m_y+nYDiffDir);

			if (!bSolidInFrontOfUsYAxis)
			{
				m_yoffset += nYDiffDir;
				NORMALIZEY;
			}
		}
	}

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
			if ((ABS(y-m_y)<3) && ((m_nXDir<0 && x<=m_x) || (m_nXDir>0 && x>=m_x)))
			{
				if ((rand()%50)<=2)
				{
					MonsterShoot(PIXELX + (m_nXDir*16), PIXELY, m_nXDir<0?-12:12);
					m_nNoShootCounter = 24;// fixme, should be time, not frame count
				}
			}
			else
			{
				// If we don't do this, then we have slightly really annoying situation
				// where you sorta jump up, and they immediately shoot when you as soon
				// as you're at the same level (while jumping) .. this gives you a little
				// chance.
				m_nNoShootCounter = 6;//Make lower (or 0) to increase difficulty
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

	return CMonster::Tick();
}
void CFlyingRobot::Initialize(int a, int b)
{
	CRobot::Initialize(a,b);
	// This is a flying robot, so of course it doesn't fall
	m_bFalls = false;
}

/*-----------------------------------------------------------*/
CRabbit::CRabbit() :
	m_nWalkAnimOffset(0),
	m_nWalkAnimOffsetUpdateCounter(0)
{
}
void CRabbit::Initialize(int a, int b)
{
	CMonster::Initialize(a,b);
	// So if you have a bunch, they aren't necessarily all walking exactly in sync:
	m_nWalkAnimOffset = rand()%4;
	SetVisibleBounds(0,-BLOCKH,BLOCKW*2,BLOCKH-1);
	SetShootBounds  (8,-BLOCKH,BLOCKW*2-8,BLOCKH-1);
	SetActionBounds (8,-BLOCKH,BLOCKW*2-8,BLOCKH-1);
	SetLayer(LAYER_4);
	m_bShootable = true;
}
void CRabbit::Draw()
{
	const int a=6;//Spriteset number
	const int b=(m_nXDir>0 ? 6*16 : 6*16 + 8);//Sprite number within spriteset
	int x = CALC_XOFFSET(m_x) + m_xoffset;
	int y = CALC_YOFFSET(m_y) + m_yoffset;
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
	
	return CMonster::Tick();
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
	AddThing(CreateExplosion(m_x*BLOCKW + 8, m_y*BLOCKH - 8, 1));
	return THING_DIE;
}
int CRabbit::OnHeroShot()
{
	// Start death animation [TODO]
	update_score(200, m_x, m_y);// [fixmeLOW] Should get more points for shooting than stupidly walking into it? [dj2017]
	AddThing(CreateExplosion(m_x*BLOCKW + 8, m_y*BLOCKH - 8, 1));
	return THING_DIE;
}
/*-----------------------------------------------------------*/
CHighVoltage::CHighVoltage() :
	m_nHeight(1)
{
	m_nStrength = 10;
}
void CHighVoltage::Initialize(int a, int b)
{
	CMonster::Initialize(a,b);

	m_bFalls = false;

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
		int x = CALC_XOFFSET(m_x  );
		int y = CALC_YOFFSET(m_y+i);
		DRAW_SPRITE16A(pVisView,a,b,x,y);
	}
}
int CHighVoltage::Tick()
{
	return CMonster::Tick();
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
CCannon::CCannon()
{
}
void CCannon::Initialize(int a, int b)
{
	CMonster::Initialize(a,b);
	m_nStrength = 2;
	m_bShootable = true;
	SetVisibleBounds(0,0,BLOCKW*2-1,BLOCKH-1);
	SetShootBounds  (0,0,BLOCKW*2-1,BLOCKH-1);
	SetActionBounds (0,0,BLOCKW*2-1,BLOCKH-1);
}
int CCannon::Tick()
{
	// IF BUMP INTO SOMETHING SOLID, REVERSE DIRECTION
	//fixmeTHIS and also the rabbit i think might not be 100% correct, check these
	// Remember we're 2x1 block-units in size
	bool bCollidedOrEndOfFloor = false;
	if (m_nXDir<0)
	{
		bCollidedOrEndOfFloor =
			(check_solid(m_x + m_nXDir, m_y))//solid to left of us?
		|| (!check_solid(m_x + m_nXDir, m_y + 1));//nothing below & just in front of us?
	}
	else
	{
		bCollidedOrEndOfFloor =
			(check_solid(m_x + m_nXDir + 1, m_y))//solid to right of us?
		|| (!check_solid(m_x + m_nXDir + 1, m_y + 1));//nothing below & just in front of us?
	}
	if (bCollidedOrEndOfFloor)
	{
		m_nXDir = -m_nXDir;
		m_xoffset = 0;
	}

	// Move
	m_xoffset += m_nXDir*2;
	// Moving left?
	if (m_nXDir<0)
	{
		while (m_xoffset < -BLOCKW)
		{
			m_xoffset += BLOCKW;
			--m_x;
		}
	}
	// Moving right?
	else if (m_nXDir>0)
	{
		while (m_xoffset > BLOCKW)
		{
			m_xoffset -= BLOCKW;
			++m_x;
		}
	}


	// SHOOT AT HERO
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
			if ((ABS(y-m_y)<3) && ((m_nXDir<0 && x<=m_x) || (m_nXDir>0 && x>=m_x)))
			{
				if ((rand()%50)<=2)
				{
					const int nBULLETSPEEDX=10;
					if (m_nXDir<0)//Facing left?
						MonsterShoot(PIXELX-BLOCKW  , m_y*16, m_nXDir*nBULLETSPEEDX,0);
					else
						MonsterShoot(PIXELX+BLOCKW*2, m_y*16, m_nXDir*nBULLETSPEEDX,0);
					// FIXME PLAY SOUND HERE?
					m_nNoShootCounter = 12;
				}
			}
		}
	}

	return CMonster::Tick();
}
void CCannon::Draw()
{
	// Draw sprite
	int a=m_a;//Spriteset number
	int b=(m_nXDir>0 ? m_b+4+(anim4_count%2)*2 : m_b+(anim4_count%2)*2);//Sprite number within spriteset
	int x = CALC_XOFFSET(m_x) + m_xoffset;
	int y = CALC_YOFFSET(m_y) + m_yoffset;
	DRAW_SPRITEA(pVisView, a, b, x, y, 32, 16);

	// Note that after it's been shot once, it 'smokes', to show it's injured [cf. DN1] [dj2017-08]
	if (m_nStrength<2)
	{
		// Draw 'smoke'
		a=5;//Spriteset number
		b=36+anim4_count;//Sprite number within spriteset
		if (m_nXDir<0)
			x = CALC_XOFFSET(m_x) + m_xoffset + BLOCKW*2 - 6;
		else
			x = CALC_XOFFSET(m_x) + m_xoffset - BLOCKW + 6;
		y = CALC_YOFFSET(m_y) + m_yoffset;
		DRAW_SPRITE16A(pVisView, a, b, x, y);
	}
}
int CCannon::HeroOverlaps()
{
	if (!HeroIsHurting())
	{
		update_health(-1);
		HeroSetHurting();
	}
	return 0;
}
int CCannon::OnHeroShot()
{
	if (m_nStrength>0)
	{
		AddThing(CreateExplosion(m_x*16+8, m_y*16));
	}
	return CMonster::OnHeroShot();
}
int CCannon::OnKilled()
{
	update_score(400, m_x, m_y);
	//AddThing(CreateExplosion(m_x*16+8+m_xoffset, m_y*BLOCKH,1));
	// Use two explosions as quick n dirty / cheap way to make explosion seem slightly bigger
	AddThing(CreateExplosion(PIXELX+(BLOCKW/2)-(rand()%4),PIXELY+((rand()%3)-1),1));
	AddThing(CreateExplosion(PIXELX+(BLOCKW/2)+(rand()%4),PIXELY+((rand()%3)-1),1,-1));
	//[TODO?] later could add a 'dying' animation here [dj2017-08]
	return THING_DIE;
}
/*-----------------------------------------------------------*/
CCrawler::CCrawler()
{
	m_bShootable = true;
	m_nDir = -1; // direction
	m_nXDir = 0;
}

int CCrawler::Tick()
{
	if (m_yoffset==0)
	{
		if ((!check_solid(m_x + m_nXDir, m_y + m_nDir)) || (check_solid(m_x, m_y + m_nDir)))
		{
			m_nDir = -m_nDir;
			return 0;
		}
	}
	m_yoffset += m_nDir;
	if (m_yoffset<=-16)
	{
		m_yoffset = 0;
		m_y--;
	}
	else if (m_yoffset>=16)
	{
		m_yoffset = 0;
		m_y++;
	}
	return 0;
}

void CCrawler::Draw()
{
	DRAW_SPRITE16A(pVisView, m_a, SGN(m_nXDir)*m_nDir<0 ? m_b + 3 - anim4_count : m_b + anim4_count, CALC_XOFFSET(m_x), CALC_YOFFSET(m_y) + m_yoffset);
}

int CCrawler::OnHeroShot()
{
	update_score(100, m_x, m_y);
	AddThing(CreateExplosion(PIXELX,PIXELY));
	return THING_DIE;
}

int CCrawler::HeroOverlaps()
{
	if (!HeroIsHurting())
	{
		update_health(-1);
		HeroSetHurting();
	}
	return 0;
}

void CCrawler::Initialize(int b0, int b1)
{
	m_nXDir = (GET_EXTRA(b0, b1, 0)==0 ? -1 : 1);
	SetActionBounds(0,0,15,15);
	SetVisibleBounds(0,0,15,15);
	SetShootBounds(0,0,15,15);
}
/*-----------------------------------------------------------*/
CSpike::CSpike()
{
	m_nType = 0;
	m_nSpikePopupCount = 0;
	SetActionBounds(0,0,15,15);
}

int CSpike::Tick()
{
	if (m_nSpikePopupCount>0)
		m_nSpikePopupCount--;
	return 0;
}

void CSpike::Draw()
{
	DRAW_SPRITE16A(pVisView, m_a, m_b + (m_nSpikePopupCount>0 ? 1 : 0), CALC_XOFFSET(m_x), CALC_YOFFSET(m_y));
}

int CSpike::HeroOverlaps()
{
	if (!HeroIsHurting())
	{
		update_health(-1);
		HeroSetHurting();
	}
	if (m_nType==1) // Pop-up type
	{
		m_nSpikePopupCount = 10;
	}
	return 0;
}

void CSpike::Initialize(int b0, int b1)
{
	m_nType = GET_EXTRA(b0, b1, 0);
	if (m_nType==1)
	{
		SetActionBounds(4,4,11,15);
		SetVisibleBounds(4,4,11,15);
	}
}
/*-----------------------------------------------------------*/
// NOTE: the jumping-monster in 'resting' / 'shooting' state
// is 2x2 'blocks' ... while jumping / falling, it's 2x3 ..
// that's why on certain state changes, we re-set the various
// bounding boxes.
CJumpingMonster::CJumpingMonster() :
	m_nJumpingIndex(-1),
	m_bFalling(false),
	m_bLinedUpToShoot(false)
{
}
int CJumpingMonster::Tick()
{
	// Turn to face (and move in) direction of hero
	if (x < m_x - 1)
		m_nXDir = -1;
	else if (x > m_x + 1)
		m_nXDir = 1;

	if (IsInView())
	{
		if(m_nJumpingIndex<0 && !m_bFalling)
		{
			// Likelihood of jumping less if lined up to shoot, as he'd rather try shoot you
			if (
				( m_bLinedUpToShoot &&  ((rand()%1000)<25) )
				|| 
				( !m_bLinedUpToShoot && ((rand()%100)<4) )
				)
			//if ((rand()%100)<4)
			{
				// Go to 2x3 size state
				m_nJumpingIndex = 0;//<- start jump
				SetVisibleBounds(0,-32,31,15);
				SetActionBounds (0,-32,31,15);
				SetShootBounds  (0,-32,31,15);
			}
		}
	}

	// dj2017-08 This if condition is a bit un-obvious - basically,
	// things should fall by default only if in view, including monsters
	// (eg if walk such that monster enters view, want to see it drop if it was placed in air) [NOT 100% sure of that but doesn't really matter - LOW]
	// BUT, if we did actually jump, then we definitely fall. It would be
	// silly if falling only if in view in that case, as eg we'd see it jump, we
	// go out of view, wait a while, come back, and then it only finishes falling.
	if (m_bFalling || ( IsInView() && !IsJumping()) )
	{
		bool bSolidBelowUsYAxis = false;
		if (m_xoffset>0)
			bSolidBelowUsYAxis = ( check_solid(m_x  ,m_y+1) || check_solid(m_x+1,m_y+1) || check_solid(m_x+2,m_y+1) );
		else if (m_xoffset<0)
			bSolidBelowUsYAxis = ( check_solid(m_x-1,m_y+1) || check_solid(m_x  ,m_y+1) || check_solid(m_x+1,m_y+1) );
		else
			bSolidBelowUsYAxis = ( check_solid(m_x  ,m_y+1) || check_solid(m_x+1,m_y+1) );

		if ( bSolidBelowUsYAxis )
		{
			// If *just* landed, set noshootcounter so we don't immediately shoot upon landing (give slight chance to hero)
			if (m_bFalling)
				m_nNoShootCounter = 4;//<- Make this lower to make monster more difficult
			m_bFalling = false;
			// make sure we go back to 2x2 square resting state/size
			SetVisibleBounds(0,-16,31,15);
			SetActionBounds (0,-16,31,15);
			SetShootBounds  (0,-16,31,15);
			m_yoffset = 0;
		}
		else
		{
			const int FALLSPEED = 4;//pixels
			m_yoffset += FALLSPEED;//anJumpOffsetsY[m_nJumpingIndex];
			NORMALIZEY;
		}
	}
	if (IsJumping() || m_bFalling)
	{
		// x-axis collision detection
		const int nSPEEDX = 1;//<- Could try experiment with increasing this to make him sort of 'get to us faster' in terms of X direction
		bool bSolidInFrontOfUsXAxis = false;
		//if (m_xoffset==0)
		{
			// First translate to world pixel coordinates
			// Then add our xoffset diff
			// Then re-translate back to block coordinate is 'in' a solid
			// If it's not, it means we can move.
			int nXPixel = PIXELX;
			int nNewDesiredPixelX = nXPixel + m_nXDir*nSPEEDX;
			int nNewDesiredBlockX = nNewDesiredPixelX / BLOCKW;

			if (m_nXDir>0)
				bSolidInFrontOfUsXAxis = check_solid(nNewDesiredBlockX+2, m_y) || check_solid(nNewDesiredBlockX+2, m_y-1) || check_solid(nNewDesiredBlockX+2, m_y-2);
			else if (m_nXDir<0)
				bSolidInFrontOfUsXAxis = check_solid(nNewDesiredBlockX, m_y) || check_solid(nNewDesiredBlockX, m_y-1) || check_solid(nNewDesiredBlockX, m_y-2);
		}
		if (!bSolidInFrontOfUsXAxis)
		{
			m_xoffset += m_nXDir*nSPEEDX;
			NORMALIZEX;
		}
	}

	// If jumping, apply jump movements
	if (IsJumping())
	{
		const int anJUMPOFFSETSY[] = { -8,-8,-6,-4,-4,-4,-4,-2,-2,-2,-1,0,0,0,99999 };

		int nYSave = m_y;
		int nYOffsetSave = m_yoffset;

		m_yoffset += anJUMPOFFSETSY[m_nJumpingIndex];
		NORMALIZEY;

		++m_nJumpingIndex;
		if (anJUMPOFFSETSY[m_nJumpingIndex]==99999)
		{
			m_nJumpingIndex = -1;
			m_bFalling = true;
		}

		// If we busy jumping UP, then must collision-detect above us

		bool bSolidAboveUsYAxis = false;
		if (m_xoffset>0)
			bSolidAboveUsYAxis = ( check_solid(m_x  ,m_y-3) || check_solid(m_x+1,m_y-3) || check_solid(m_x+2,m_y-3) );
		else if (m_xoffset<0)
			bSolidAboveUsYAxis = ( check_solid(m_x-1,m_y-3) || check_solid(m_x  ,m_y-3) || check_solid(m_x+1,m_y-3) );
		else
			bSolidAboveUsYAxis = ( check_solid(m_x  ,m_y-3) || check_solid(m_x+1,m_y-3) );

		if (bSolidAboveUsYAxis)//check_solid(m_x,m_y-3,true) || check_solid(m_x+1,m_y-3,true))
		{
			// 'Disallow'/'revert' the movement itself
			m_y = nYSave;
			m_yoffset = nYOffsetSave;
			//IMMEDIATELY stop jumping, go into falling state
			m_nJumpingIndex = -1;
			m_bFalling = true;
		}
	}

	// If in view, and NOT jumping OR falling, AND sort of 'in line' with hero, then sometimes shoot
	m_bLinedUpToShoot = false;
	if (IsInView() && !m_bFalling && !IsJumping())
	{
		// Check we're facing hero, and hero is more or less at same height ..
		if ((ABS(y-m_y)<3) && ((m_nXDir<0 && x<=m_x) || (m_nXDir>0 && x>=m_x)))
		{
			m_bLinedUpToShoot = true;
			if (m_nNoShootCounter>0)
			{
				m_nNoShootCounter--;
			}
			else
			{
				const int SHOOTPERCENT=10;//<- Make this higher to increase difficulty
				if ((rand()%100)<=SHOOTPERCENT)
				{
					const int nBULLETSPEEDX=10;
					if (m_nXDir<0)//Facing left?
						MonsterShoot(PIXELX-BLOCKW  , m_y*16, m_nXDir*nBULLETSPEEDX,0);
					else
						MonsterShoot(PIXELX+BLOCKW*2, m_y*16, m_nXDir*nBULLETSPEEDX,0);
					// FIXME PLAY SOUND HERE?
					m_nNoShootCounter = 12;//<- Make this lower to increase difficulty
				}
			}
		}
	}

	return CMonster::Tick();
}
void CJumpingMonster::Draw()
{
	if (m_nFlickerCounter==0 || ((m_nFlickerCounter%2)==0))
	{
		if (IsJumping() || m_bFalling)
		{
			const int a=m_a;//Spriteset number
			const int b=(m_nXDir>0 ? m_b+16-4 : m_b+16-4+2);//Sprite number within spriteset
			int x = CALC_XOFFSET(m_x) + m_xoffset;
			int y = CALC_YOFFSET(m_y-2) + m_yoffset;
			DRAW_SPRITEA(pVisView,
				a   ,b,//+m_nWalkAnimOffset*2,
				x   ,y,
				32  ,  48);
		}
		else
		{
			const int a=m_a;//Spriteset number
			const int b=(m_nXDir>0 ? m_b-16 : m_b-16+2);//Sprite number within spriteset
			int x = CALC_XOFFSET(m_x) + m_xoffset;
			int y = CALC_YOFFSET(m_y-1) + m_yoffset;
			DRAW_SPRITEA(pVisView,
				a   ,b,//+m_nWalkAnimOffset*2,
				x   ,y,
				32  ,  32);
		}
	}
}
void CJumpingMonster::Initialize(int a, int b)
{
	CMonster::Initialize(a,b);
	m_bFalls = false;//<- Don't use 'default' fall stuff, since doesn't fall if not in view. We want to fall IF jumped if was in view
	SetVisibleBounds(0,-16,31,15);
	SetActionBounds (0,-16,31,15);
	SetShootBounds  (0,-16,31,15);
	m_nStrength = 3;//fixme chekc is this 3 or 4?
	m_nJumpingIndex = -1;
	m_bFalling = false;
	m_nNoShootCounter = 12;
}
int CJumpingMonster::HeroOverlaps()
{
	if (!HeroIsHurting())
	{
		update_health(-1);
		HeroSetHurting();
	}
	return 0;
}
int CJumpingMonster::OnKilled()
{
	update_score(150, m_x, m_y);
	// dj2017-08 Quick n dirty way to make it seem like we have a bigger
	// explosion effect than we really do for this monster type ... note
	// in this case only one should produce a sound .. this is all a bit hacky.
	AddThing(CreateExplosion(PIXELX-(rand()%7)+8,PIXELY-(rand()%6) - 12,1,1));
	AddThing(CreateExplosion(PIXELX  +8,PIXELY   - 12,1,-1));
	AddThing(CreateExplosion(PIXELX+(rand()%6)+8,PIXELY+(rand()%6) - 12,1,-1));
	AddThing(CreateExplosion(PIXELX+(rand()%9)+8,PIXELY-(rand()%6) - 12,1,-1));

	AddThing(CreateExplosion(PIXELX-(rand()%9)+8,PIXELY-(rand()%6) -  5,1,-1));
	AddThing(CreateExplosion(PIXELX+(rand()%9)+8,PIXELY-(rand()%6) -  5,1,-1));
	return THING_DIE;
}
/*-----------------------------------------------------------*/
CDrProton* CDrProton::g_pGameEnding=NULL;//This is a little weird
CDrProton::CDrProton() :
	m_bEscaping(false),
	m_nOrigX(-1)
{
}
CDrProton::~CDrProton()
{
	g_pGameEnding = NULL;
}
int CDrProton::Tick()
{
	if (m_nOrigX<0)
		m_nOrigX = m_x;
	if (m_bEscaping)
	{
		// FLY AWAY! Dr Proton escapes, so we can put him a 'version 2' [dj2017-08]
		//m_yoffset -= BLOCKH;
		m_yoffset = 0;
		--m_y;
		NORMALIZEY;
		//if (m_y<=2)//<- Hit top of level?
		if (m_y<=2)// || m_y<y-20)
		{
			m_y = 2;
			//yo = m_y;
			// UHM DO SOMETHING COOL HERE
			// Activate end-game sequence
			//ActivateEndGameSequence();
			//return THING_DIE;
		}
	}
	else if (IsInView())
	{
		if (ABS(m_nOrigX - m_x) < 3)
		{
			if (m_x<x)
			{
				++m_xoffset;
			}
			else if (m_x >= x+1)
			{
				--m_xoffset;
			}
			// THIS IS IMPORTANT:
			NORMALIZEX;
		}


		if (m_y<y - 2)
		{
			++m_yoffset;
		}
		else if (m_y > y + 1)
		{
			--m_yoffset;
		}
		NORMALIZEY;

		// Turn to face direction of hero
		if (x < m_x - 1)
			m_nXDir = -1;
		else if (x > m_x + 1)
			m_nXDir = 1;


		if (IsInView())// && !m_bFalling && !IsJumping())
		{
			// Check we're facing hero, and hero is more or less at same height ..
			if ((ABS(y-m_y)<3) && ((m_nXDir<0 && x<=m_x) || (m_nXDir>0 && x>=m_x)))
			{
				//m_bLinedUpToShoot = true;
				if (m_nNoShootCounter>0)
				{
					m_nNoShootCounter--;
				}
				else
				{
					const int SHOOTPERCENT=10;//<- Make this higher to increase difficulty
					if ((rand()%100)<=SHOOTPERCENT)
					{
						const int nBULLETSPEEDX=10;
						if (m_nXDir<0)//Facing left?
							MonsterShoot(PIXELX-BLOCKW  , m_y*16, m_nXDir*nBULLETSPEEDX,0);
						else
							MonsterShoot(PIXELX+BLOCKW*3, m_y*16, m_nXDir*nBULLETSPEEDX,0);
						// FIXME PLAY SOUND HERE?
						m_nNoShootCounter = 12;//<- Make this lower to increase difficulty
					}
				}
			}
		}

	}

	return CMonster::Tick();
}
void CDrProton::Draw()
{
	// If flickering-from-hurting, skip every 2nd frame
	if (m_nFlickerCounter==0 || ((m_nFlickerCounter%2)==0))
	{
		const int a=m_a;//Spriteset number
		const int b=(m_nXDir>0 ? ((anim4_count % 2)*3) : ((anim4_count % 2)*3) + 6);
		int x = CALC_XOFFSET(m_x) + m_xoffset;
		int y = CALC_YOFFSET(m_y-2) + m_yoffset;//Topleft, hence subtract from m_y
		DRAW_SPRITEA(pVisView,
			a   ,b,
			x   ,y,
			48  ,48);
	}
}
void CDrProton::Initialize(int a, int b)
{
	CMonster::Initialize(a,b);
	m_bFalls = false;
	m_nStrength = 15;//?fixmeLOW what should Dr Proton strength be
	m_nNoShootCounter = 5;
	// This might be slightly too big
	SetVisibleBounds(0,-31,47,15);
	SetActionBounds (0,-31,47,15);
	SetShootBounds  (0,-31,47,15);
}
int CDrProton::HeroOverlaps()
{
	// If in 'escaping' mode then we don't hurt hero, i.e. while he's flying away
	if (!m_bEscaping && !HeroIsHurting())
	{
		update_health(-1);
		HeroSetHurting();
	}
	return 0;
}
int CDrProton::OnHeroShot()
{
	int nPrevStrength = m_nStrength;
	int nRet = CMonster::OnHeroShot();
	if (nPrevStrength>0 && m_nStrength<=0)
	{
		update_score(20000, m_x, m_y);// [fixmeLOW] Should get more points for shooting than stupidly walking into it? [dj2017]
		m_bEscaping = true;
		g_pGameEnding = this;
	}
	return nRet;
}
/*-----------------------------------------------------------*/
