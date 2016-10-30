/*--------------------------------------------------------------------------*/
// thing.cpp
/*
Copyright (C) 2000-2001 David Joffe

License: GNU GPL Version 2 (*not* "later versions")
*/
/*--------------------------------------------------------------------------*/
#include "thing.h"
#include "djtypes.h"
#include "graph.h"
#include "mission.h" // FIXME: This dep. canprobly go
#include "block.h" // Class type definitions
#include "djlog.h"
#include "hero.h"
#include <assert.h>


#include "game.h"
#include "inventory.h"

SOUND_HANDLE CTeleporter::c_iSoundTeleport=SOUNDHANDLE_INVALID;

/*-----------------------------------------------------------*/
CThingFactory g_ThingFactory;

CThingFactory::CThingFactory()
{
}

// TODO: CThingFactory leaks a *lot*. Gotta do something about it.
CThingFactory::~CThingFactory()
{
	m_aDescriptors.clear();
}

int CThingFactory::Register(int nTypeID, THING_ALLOCATER pAllocateProc, THING_PERLEVELINIT pPerLevelInitProc)
{
	SDescriptor Desc;
	Desc.nTypeID = nTypeID;
	Desc.pAllocateProc = pAllocateProc;
	Desc.pPerLevelInitProc = pPerLevelInitProc;
	m_aDescriptors.push_back(Desc);
	return (int)m_aDescriptors.size() - 1;
}

CThing* CThingFactory::Allocate(int nTypeID)
{
	vector<SDescriptor>::const_iterator i;

	for ( i=m_aDescriptors.begin(); i!=m_aDescriptors.end(); ++i )
	{
		if ( nTypeID == (*i).nTypeID )
		{
			CThing *pThing = (*i).pAllocateProc();
			pThing->SetType(nTypeID);
			return pThing;
		}
	}
	return NULL;
/*
	for ( int i=0; i<(int)m_aDescriptors.size(); i++ )
	{
		if (m_aDescriptors[i].nTypeID==nTypeID)
		{
			CThing *pThing = m_aDescriptors[i].pAllocateProc();
			pThing->SetType(nTypeID);
			return pThing;
		}
	}
	return NULL;
*/
}

void CThingFactory::PerLevelInitialize()
{
	for ( int i=0; i<(int)m_aDescriptors.size(); i++ )
	{
		if (m_aDescriptors[i].pPerLevelInitProc!=NULL)
			m_aDescriptors[i].pPerLevelInitProc();
	}
}


void CLetterPerLevelInit()
{
	CLetter::c_nBonusIndex = 0;
}

void CCameraPerLevelInit()
{
	CCamera::c_nNumCameras = 0;
}

/*-----------------------------------------------------------*/
REGISTER_THING(CSpikeBall,     TYPE_SPIKEBALL, NULL);
REGISTER_THING(CTest,          TYPE_TEST, NULL);
REGISTER_THING(CExit,          TYPE_EXIT, NULL);
REGISTER_THING(CLetter,        TYPE_LETTER, CLetterPerLevelInit);
REGISTER_THING(CBox,           TYPE_BOX, NULL);
REGISTER_THING(CTeleporter,    TYPE_TELEPORTER, NULL);
REGISTER_THING(CDoor,          TYPE_DOOR, NULL);
REGISTER_THING(CKey,           TYPE_KEY, NULL);
REGISTER_THING(CDoorActivator, TYPE_DOORACTIVATOR, NULL);
REGISTER_THING(CSoftBlock,     TYPE_SOFTBLOCK, NULL);
REGISTER_THING(CCamera,        TYPE_CAMERA, CCameraPerLevelInit);
REGISTER_THING(CBanana,        TYPE_BANANA, NULL);
REGISTER_THING(CCoke,          TYPE_COKE, NULL);
REGISTER_THING(CCrawler,       TYPE_CRAWLER, NULL);
REGISTER_THING(CSpike,         TYPE_SPIKE, NULL);
REGISTER_THING(CBalloon,       TYPE_BALLOON, NULL);
REGISTER_THING(CAcme,          TYPE_ACME, NULL);
REGISTER_THING(CPickup,        TYPE_PICKUP, NULL);
REGISTER_THING(CBoots,         TYPE_POWERBOOTS, NULL);
REGISTER_THING(CRobot,         TYPE_ROBOT, NULL);
REGISTER_THING(CLift,          TYPE_LIFT, NULL);
REGISTER_THING(CConveyor,      TYPE_CONVEYOR, NULL);
REGISTER_THING(CFlameThrow,    TYPE_FLAMETHROW, NULL);
REGISTER_THING(CDynamite,      TYPE_DYNAMITE, NULL);
REGISTER_THING(CFan,           TYPE_FAN, NULL);
REGISTER_THING(CFirepower,     TYPE_FIREPOWER, NULL);
REGISTER_THING(CDust,          TYPE_DUST, NULL);

/*-----------------------------------------------------------*/
CThing::CThing()
{
	m_bFalls = false;
	m_x = m_y = m_xsmall = 0;
	m_bxsmall = false;
	m_a = 0;
	m_b = 0;
	m_xoffset = 0;
	m_yoffset = 0;
	m_width = 1;
	m_height = 1;
	m_iActionX1 = -1;
	m_iActionY1 = -1;
	m_iActionX2 = -1;
	m_iActionY2 = -1;
	m_iType = -1;
	m_eLayer = LAYER_BOTTOM;
	m_iID = 0;
	m_bSolid = false;
	m_iSolidX1 = -1;
	m_iSolidY1 = -1;
	m_iSolidX2 = -1;
	m_iSolidY2 = -1;
	m_iShootX1 = -1;
	m_iShootY1 = -1;
	m_iShootX2 = -1;
	m_iShootY2 = -1;
	m_bHeroInside = false;
	m_bShootable = false;
	m_iVisibleX1 = 0;
	m_iVisibleY1 = 0;
	m_iVisibleX2 = 15;
	m_iVisibleY2 = 15;
}

CThing::CThing( int x, int y, int xsmall, bool bxsmall )
{
	m_bFalls = false;
	m_x = x;
	m_y = y;
	m_xsmall = xsmall;
	m_bxsmall = bxsmall;

	m_a = 0;
	m_b = 0;
	m_xoffset = 0;
	m_yoffset = 0;
	m_width = 1;
	m_height = 1;
	m_iActionX1 = -1;
	m_iActionY1 = -1;
	m_iActionX2 = -1;
	m_iActionY2 = -1;
	m_iType = -1;
	m_eLayer = LAYER_BOTTOM;
	m_iID = 0;
	m_bSolid = false;
	m_iSolidX1 = -1;
	m_iSolidY1 = -1;
	m_iSolidX2 = -1;
	m_iSolidY2 = -1;
	m_iVisibleX1 = 0;
	m_iVisibleY1 = 0;
	m_iVisibleX2 = 15;
	m_iVisibleY2 = 15;
}

void CThing::SetLocation( int ix, int iy, int xoffset, int yoffset, int width, int height )
{
	m_x = ix;
	m_y = iy;
	m_xoffset = xoffset;
	m_yoffset = yoffset;
	m_width = width;
	m_height = height;
}

void CThing::SetSprite( int a, int b )
{
	m_a = a;
	m_b = b;
}

void CThing::SetActionBounds(int x1, int y1, int x2, int y2)
{
	m_iActionX1 = x1;
	m_iActionY1 = y1;
	m_iActionX2 = x2;
	m_iActionY2 = y2;
}

void CThing::SetSolidBounds(int x1, int y1, int x2, int y2)
{
	m_iSolidX1 = x1;
	m_iSolidY1 = y1;
	m_iSolidX2 = x2;
	m_iSolidY2 = y2;
}

void CThing::SetShootBounds(int x1, int y1, int x2, int y2)
{
	m_iShootX1 = x1;
	m_iShootY1 = y1;
	m_iShootX2 = x2;
	m_iShootY2 = y2;
}

void CThing::SetVisibleBounds(int x1, int y1, int x2, int y2)
{
	m_iVisibleX1 = x1;
	m_iVisibleY1 = y1;
	m_iVisibleX2 = x2;
	m_iVisibleY2 = y2;
}

void CThing::DrawActionBounds(const djColor &Color)
{
	djgSetColorFore(pVisView, Color);
	djgDrawRectangle(pVisView,
		CALC_XOFFSET(m_x,m_xsmall)+m_iActionX1,
		CALC_YOFFSET(m_y)+m_iActionY1,
		(m_iActionX2-m_iActionX1)+1,
		(m_iActionY2-m_iActionY1)+1);
}

bool CThing::OverlapsBounds(int x, int y)
{
	if (m_iActionX1==-1 && m_iActionY1==-1 && m_iActionX2==-1 && m_iActionY2==-1)
		return false;
	return OVERLAPS(
		x, y,
		x+15, y+31,
		m_x*16+m_iActionX1, m_y*16+m_iActionY1,
		m_x*16+m_iActionX2, m_y*16+m_iActionY2);
}

/*-----------------------------------------------------------*/
CTest::CTest()
{
	m_eLayer = LAYER_TOP;
	m_iActionX1 =  -8;
	m_iActionY1 =  -16 - 16;
	m_iActionX2 =  15+8;
	m_iActionY2 =  15 - 16;
	m_bSolid = true;
	m_iSolidX1 = 5;
	m_iSolidY1 = 5;
	m_iSolidX2 = 10;
	m_iSolidY2 = 10;
	m_bxsmall = true;
}

void CTest::HeroEnter()
{
	CThing::HeroEnter();
	update_health(-1);
	HeroSetHurting();
}

int CTest::Tick()
{
	return 0;
	static int nDir = -1;
	static int nCount = 0;
	nCount++;
	if (nCount<=8)
		return 0;
	nCount = 0;

	if (nDir==-1)
	{
		if (m_xsmall==1)
			m_xsmall = 0; // no need to check
		else
		{
			if (check_solid(m_x + nDir, m_y))
				nDir = -nDir;
			else
			{
				m_xsmall = 1;
				m_x--;
			}
		}
	}
	else
	{
		if (m_xsmall==1)
		{
			m_xsmall = 0;
			m_x++;
		}
		else
		{
			if (check_solid(m_x + 1, m_y))
				nDir = -nDir;
			else
			{
				m_xsmall = 1;
			}
		}
	}

	return 0;
	//m_bSolid = false; // Otherwise we check ourselves
	if (check_solid(m_x + nDir, m_y))
		nDir = -nDir;
	else
		m_x += nDir;
	//m_bSolid = true;
	return 0;
}

void CTest::Draw()
{
	DRAW_SPRITE16A(pVisView, m_a, m_b, CALC_XOFFSET(m_x,m_xsmall), CALC_YOFFSET(m_y));
}
/*-----------------------------------------------------------*/
CSpikeBall::CSpikeBall()
{
	SetActionBounds(0,0,15,15);
	m_nBounceIndex = -1;
	m_nYOffset = 0;
	m_nInitialOffset = 0;
	m_eType = TYPE_BOUNCING;
}

void CSpikeBall::Draw()
{
	DRAW_SPRITE16A(pVisView, m_a, m_b, CALC_XOFFSET(m_x,m_xsmall), CALC_YOFFSET(m_y) + m_nYOffset*2);
}

void CSpikeBall::OnAdded()
{
}

int CSpikeBall::Tick()
{
	if (m_eType==TYPE_BOUNCING)
	{
		static int anBounceOffsets[] = { -8, -6, -4, -2, -2, -1, 0, 0, 0, 1, 2, 4, 6, 10, 999 };
		while (m_nInitialOffset>0)
		{
			m_nInitialOffset--;
			m_nBounceIndex++;
			if (anBounceOffsets[m_nBounceIndex]==999)
				m_nBounceIndex = 0;
			m_nYOffset += anBounceOffsets[m_nBounceIndex];
		}
		m_nBounceIndex++;
		if (anBounceOffsets[m_nBounceIndex]==999)
			m_nBounceIndex = 0;
		m_nYOffset += anBounceOffsets[m_nBounceIndex];
	}
	m_iVisibleY1 = m_iActionY1 = m_nYOffset * 2;
	m_iVisibleY2 = m_iActionY2 = m_nYOffset * 2 + 15;

	return 0;
}

int CSpikeBall::HeroOverlaps()
{
	if (!HeroIsHurting())
	{
		update_health(-1);
		HeroSetHurting();
	}
	// If stationary spike, destroy self
	if (m_eType==TYPE_STATIONARY)
	{
		AddThing(CreateExplosion(m_x*16, m_y*16));
		return THING_DIE;
	}
	return 0;
}

void CSpikeBall::Initialize(int b0, int b1)
{
	m_nInitialOffset = GET_EXTRA(b0, b1, 0);
	m_eType = (EType)GET_EXTRA(b0, b1, 1);
}

/*-----------------------------------------------------------*/
CBox::CBox()
{
	// Default to an empty, invisible box
	SetContents( 0, 0 );
	SetSprite( 0, 0 );
	m_eLayer = LAYER_BOTTOM;
	m_bFalls = true;
	SetActionBounds(0, 0, 15, 15);
	SetShootBounds(0, 0, 15, 15);
	m_bShootable = true;
}

void CBox::SetContents( int iContentsA, int iContentsB )
{
	m_iContentsA = iContentsA;
	m_iContentsB = iContentsB;
}

void CBox::Initialize(int b0, int b1)
{
	SetContents( GET_EXTRA(b0, b1, 10), GET_EXTRA(b0, b1, 11) );
}

int CBox::OnHeroShot()
{
	AddThing(CreateExplosion(m_x*16, m_y*16));

	// Spawn whatever was inside the box
	sprite_factory( m_iContentsA, m_iContentsB, m_x, m_y, 0, false );

	return THING_DIE;
}

void CBox::Draw()
{
	if ( ( m_a | m_b ) != 0 )
		DRAW_SPRITE16(pVisView, m_a, m_b, CALC_XOFFSET(m_x,0), CALC_YOFFSET(m_y));
}
/*-----------------------------------------------------------*/
int CLetter::c_nBonusIndex = 0;

CLetter::CLetter()
{
	m_iNumber = -1;
	m_eLayer = LAYER_TOP;
	SetActionBounds(0,0,15,15);
}

void CLetter::Initialize(int b0, int b1)
{
	m_iNumber = GET_EXTRA(b0, b1, 11);
}

int CLetter::HeroOverlaps()
{
	update_score(500, m_x, m_y);
	if (c_nBonusIndex==m_iNumber)
	{
		c_nBonusIndex++;
		if (c_nBonusIndex==6)
		{
			update_score(10000, m_x, m_y-1);
		}
	}
	return THING_DIE;
}

void CLetter::Draw()
{
	DRAW_SPRITE16A(pVisView, m_a, m_b, CALC_XOFFSET(m_x,0), CALC_YOFFSET(m_y));
}
/*-----------------------------------------------------------*/
CLift::CLift()
{
	m_bBusy = false;
	m_eLayer = LAYER_MIDDLE; // FIXME: WHERE?
	m_bSolid = true;
	SetSolidBounds(0, 0, 15, 15);
	SetActionBounds(0, -16, 15, -1);
}

int CLift::Tick()
{
	if (m_bBusy)
	{
		// um, do nothing :)
	}
	else
	{
		// drop the lift a bit, it's no longer in use
		if (m_height > 1)
		{
			m_yoffset++;
			m_height--;
		}
	}
	SetActionBounds(0, -32 - (m_height-1)*16, 15, -1 - (m_height-1)*16);
	SetSolidBounds(0, 0 - (m_height-1)*16, 15, 15);
	SetVisibleBounds(0, 0 - (m_height-1)*16, 15, 15);
	return 0;
}

int CLift::Action()
{
	m_bBusy = true;
	if (move_hero(0, -1)==0)
	{
		m_yoffset--;
		m_height++;
		// We set these immediately, otherwise the hero may be outside the bounds,
		// causing the lift to drop.
		SetActionBounds(0, -32 - (m_height-1)*16, 15, -1 - (m_height-1)*16);
		SetSolidBounds(0, 0 - (m_height-1)*16, 15, 15);
		SetVisibleBounds(0, 0 - (m_height-1)*16, 15, 15);
	}
	return 0;
};

void CLift::Draw()
{
	int i;
	for ( i=0; i<m_height; i++ )
	{
		// if section of lift is on-screen
		if ( INBOUNDS( m_x + m_xoffset, m_y + m_yoffset + i,
			xo, yo, xo + VIEW_WIDTH + xo_small, yo + VIEW_HEIGHT ) )
		{
			if (i == 0)
			{
				DRAW_SPRITE16(pVisView, 1, 14, CALC_XOFFSET(m_x+m_xoffset,0), CALC_YOFFSET(m_y+m_yoffset));
			}
			else
			{
				DRAW_SPRITE16(pVisView, 1, 15, CALC_XOFFSET(m_x+m_xoffset,0), CALC_YOFFSET(m_y+m_yoffset+i));
			}
		}
	}
}

void CLift::HeroLeave()
{
	CThing::HeroLeave();
	m_bBusy = false;
}
/*-----------------------------------------------------------*/
CExplosion::CExplosion()
{
	m_countdown = 4;
	m_xoffset   = 0;
	m_yoffset   = 0;
	m_width     = 1;
	m_iType     = -1; // fixme
	m_eLayer    = LAYER_TOP;
	m_xOffset   = 0;
	m_yOffset   = 0;
}

int CExplosion::Tick()
{
	m_countdown--;
	if (m_countdown<0)
		return THING_DIE;
	return 0;
}

void CExplosion::Draw()
{
	DRAW_SPRITE16A(pVisView, 5, 16 + (3 - m_countdown), CALC_XOFFSET(m_x,m_xsmall) + m_xOffset, CALC_YOFFSET(m_y) + m_yOffset);
}
/*-----------------------------------------------------------*/
CExit::CExit()
{
	m_nActivated = -1;
	m_x = 2;
	m_y = 2;
	m_xoffset = -1;
	m_yoffset = -1;
	m_width   = 2;
	m_height  = 2;
	m_eLayer  = LAYER_BOTTOM;
	m_iType   = TYPE_EXIT;
	// Action key testing box and visible bounds
	m_iVisibleX1 = m_iActionX1 =   0;
	m_iVisibleY1 = m_iActionY1 = -16;
	m_iVisibleX2 = m_iActionX2 =  31;
	m_iVisibleY2 = m_iActionY2 =  15;
	m_nSecondFrameTick = 0;
}

void CExit::Draw()
{
	int nOffset;
	nOffset = (m_nActivated>=1 ? ((djMIN(5,m_nActivated)-1)%4)*2 : 0);
	DRAW_SPRITE16(pVisView, 5,  96 + nOffset, CALC_XOFFSET(m_x  ,0), CALC_YOFFSET(m_y-1));
	DRAW_SPRITE16(pVisView, 5,  97 + nOffset, CALC_XOFFSET(m_x+1,0), CALC_YOFFSET(m_y-1));
	DRAW_SPRITE16(pVisView, 5, 112 + nOffset, CALC_XOFFSET(m_x  ,0), CALC_YOFFSET(m_y  ));
	DRAW_SPRITE16(pVisView, 5, 113 + nOffset, CALC_XOFFSET(m_x+1,0), CALC_YOFFSET(m_y  ));
}

int CExit::Action()
{
	m_nActivated = 0;
	djSoundPlay( g_iSounds[SOUND_EXIT] );
	HeroFreeze(200);//<-This must just be longer or equal to the duration until m_nActivated goes past the threshold to do NextLevel()
	return 0;
}

int CExit::Tick()
{
	if (m_nActivated>=0)
	{
		m_nSecondFrameTick++;
		if (m_nSecondFrameTick==1)
		{
			m_nSecondFrameTick = 0;
			m_nActivated++;
			// We move the exit to the top layer, so as to create the appearance
			// that the hero has "entered" the door.
			if (m_nActivated>2)
				m_eLayer = LAYER_TOP;
			if (m_nActivated>25)//dj2016-10-28 changing this from 5 to 25 as we're adding a longer exit sound
			{
				NextLevel();
			}
		}
	}
	return 0;
}

/*-----------------------------------------------------------*/
CTeleporter::CTeleporter()
{
	m_xoffset = -2;
	m_yoffset = -1;
	m_width = 3;
	m_height = 2;
	// Action key testing box
	SetActionBounds ( -8, -16, 15+8,  15);
	SetVisibleBounds(-16, -32, 15+16, 15);

	m_iType = TYPE_TELEPORTER;
	m_iAnimationCount=0;
	m_eLayer = LAYER_TOP;

	if (c_iSoundTeleport==SOUNDHANDLE_INVALID)
	{
		c_iSoundTeleport = djSoundLoad( "data/sounds/lightmag.wav" );
	}
	m_bActivated = false;
	m_nTeleportX = 5;
	m_nTeleportY = 5;
}

void CTeleporter::Initialize(int b0, int b1)
{
	SetID(GET_EXTRA(b0, b1, 0));
}

int CTeleporter::Action()
{
	// Locate the other teleporter
	unsigned int j;
	for ( j=0; j<g_apThings.size(); j++ )
	{
		CThing *pThing2 = g_apThings[j];
		if (pThing2->GetTypeID() == TYPE_TELEPORTER) // If this is a teleporter
		{
			if ((CThing*)this != pThing2) // If this isn't the teleporter we're standing on
			{
				// If second teleporter ID is same as this one
				if (pThing2->ID() == ID())
				{
					m_nTeleportX = pThing2->m_x;
					m_nTeleportY = pThing2->m_y;

					// start animation of teleporter
					m_iAnimationCount = 12;
					// play sound
					djSoundPlay( c_iSoundTeleport );

					// Freeze hero for some frames. Doesn't matter what this is as
					// long as its longer than our countdown.
					HeroFreeze(12+10);

					m_bActivated = true;

					return 0;
				}
			}
		}
	} // j
	djMSG( "Warning: No matching teleport found for %d\n", ID() );

	return 0;
}

int CTeleporter::Tick()
{
	// Update animation
	if (m_iAnimationCount>0) m_iAnimationCount--;
	if (m_bActivated && m_iAnimationCount<=0)
	{
		HeroUnfreeze();
		relocate_hero(m_nTeleportX, m_nTeleportY);
		m_bActivated = false;
	}
	return 0;
}

void CTeleporter::Draw()
{
	int nOffset = (m_iAnimationCount % 3) * 3;
	DRAW_SPRITE16A(pVisView, 5,  48+nOffset, CALC_XOFFSET(m_x-1,0), CALC_YOFFSET(m_y-2));
	DRAW_SPRITE16A(pVisView, 5,  49+nOffset, CALC_XOFFSET(m_x  ,0), CALC_YOFFSET(m_y-2));
	DRAW_SPRITE16A(pVisView, 5,  50+nOffset, CALC_XOFFSET(m_x+1,0), CALC_YOFFSET(m_y-2));
	DRAW_SPRITE16A(pVisView, 5,  64, CALC_XOFFSET(m_x-1,0), CALC_YOFFSET(m_y-1));
	DRAW_SPRITE16A(pVisView, 5,  65, CALC_XOFFSET(m_x  ,0), CALC_YOFFSET(m_y-1));
	DRAW_SPRITE16A(pVisView, 5,  66, CALC_XOFFSET(m_x+1,0), CALC_YOFFSET(m_y-1));
	DRAW_SPRITE16A(pVisView, 5,  80, CALC_XOFFSET(m_x-1,0), CALC_YOFFSET(m_y  ));
	DRAW_SPRITE16A(pVisView, 5,  81, CALC_XOFFSET(m_x  ,0), CALC_YOFFSET(m_y  ));
	DRAW_SPRITE16A(pVisView, 5,  82, CALC_XOFFSET(m_x+1,0), CALC_YOFFSET(m_y  ));
}
/*-----------------------------------------------------------*/
CFloatingScore::CFloatingScore()
{
	m_bufferlength = 0;
	m_height = 1;
	m_eLayer = LAYER_TOP;
}

void CFloatingScore::SetScore( int score )
{
	m_height = 25; // fixme; was 24; tick first before first draw??? check
	sprintf( (char*)m_buffer, "%d", score );
	m_bufferlength = (int)strlen( (char*)m_buffer );
	for ( int i=0; i<m_bufferlength; i++ )
	{
		// Offset the digits to point to the beginning of the font djImage,
		// where the tiny font lives
		m_buffer[i] -= '0';
	}
}

int CFloatingScore::Tick()
{
	m_height--;
	if (m_height<=0)
		return THING_DIE;
	SetVisibleBounds(0, -48 + m_height*2 - 15, 32, -48 + m_height*2 + 15);
	return 0;
}

void CFloatingScore::Draw()
{
	for ( int i=0; i<m_bufferlength; i++ )
	{
		// 5x7 font
		djgDrawImageAlpha( pVisView, g_pFont8x8,
			((int)m_buffer[i]%32)*8,
			((int)m_buffer[i]/32)*8,
			i * 6 + 8 * ( -xo_small + 2 + (((m_x - xo) << 1))),
			i * -2 + 16 + (m_y - yo) * 16 - 48 + (m_height * 2),
			5,
			7 );
	}
}
/*-----------------------------------------------------------*/
CDoorRelatedType::CDoorRelatedType()
{
	m_nID = -1;
}

void CDoorRelatedType::Initialize(int b0, int b1)
{
	m_nID = GET_EXTRA(b0, b1, 0);
}
/*-----------------------------------------------------------*/
CDoor::CDoor()
{
	m_nOpenState = 0;
	m_bSolid = true; // <-- fixme, sucks
	SetSolidBounds(0,0,15,15);
}

int CDoor::Tick()
{
	if (m_nOpenState!=0)
	{
		m_nOpenState++;
		if (m_nOpenState==4)
		{
			m_bSolid = false;
			return THING_DIE;
		}
	}
	return 0;
}

void CDoor::Draw()
{
	DRAW_SPRITE16A(pVisView, m_a, m_b+m_nOpenState, CALC_XOFFSET(m_x,0), CALC_YOFFSET(m_y));
}
/*-----------------------------------------------------------*/
CKey::CKey()
{
	m_nID = -1;
	SetActionBounds(0,0,15,15);
}

int CKey::HeroOverlaps()
{
	// Try to add self to hero's inventory. If succeeded, remove from main list.
	if (InvAdd(this))
	{
		update_score(1000, m_x, m_y);
		djSoundPlay( g_iSounds[SOUND_KEY_PICKUP] );
		return THING_REMOVE;
	}
	// Inventory full? Don't pick up or do anything.
	return 0;
}

void CKey::Draw()
{
	DRAW_SPRITE16A(pVisView, m_a, m_b, CALC_XOFFSET(m_x,0), CALC_YOFFSET(m_y));
}
/*-----------------------------------------------------------*/
CDoorActivator::CDoorActivator()
{
	m_nID = -1;
	// Fixme, this seems a little hacky - we have to make the action bounds bigger, because
	// action bounds only support hero-completely-within, not just overlapping
	SetActionBounds(-8,0,15+8,31);
}

int CDoorActivator::Action()
{
	for ( int i=0; i<InvGetSize(); i++ )
	{
		CThing *pThing = InvGetItem(i);
		if (pThing->GetTypeID()==TYPE_KEY)
		{
			if (((CKey*)pThing)->GetID()==m_nID)
			{
				InvRemove(pThing);

				djSoundPlay( g_iSounds[SOUND_OPENDOOR] );

				// Find all doors belonging to this key and tell them to open.
				unsigned int j;
				for ( j=0; j<g_apThings.size(); j++ )
				{
					if (g_apThings[j]->GetTypeID()==TYPE_DOOR)
					{
						CDoor *pDoor = (CDoor*)g_apThings[j];
						if (pDoor->GetID()==m_nID)
							pDoor->OpenDoor();
					}
				}

				delete pThing; // Delete key
				return 0;
			}
		}
	}
	return 0;
}

void CDoorActivator::Draw()
{
	DRAW_SPRITE16A(pVisView, m_a, m_b, CALC_XOFFSET(m_x,0), CALC_YOFFSET(m_y));
}
/*-----------------------------------------------------------*/
CSoftBlock::CSoftBlock()
{
	m_bSolid = true;
	SetSolidBounds(0, 0, 15, 15);
	SetActionBounds(0, 0, 15, 15);
	SetShootBounds(0, 0, 15, 15);
	m_bShootable = true;
}
void CSoftBlock::Draw()
{
	DRAW_SPRITE16A(pVisView, m_a, m_b, CALC_XOFFSET(m_x,0), CALC_YOFFSET(m_y));
}

int CSoftBlock::OnHeroShot()
{
	AddThing(CreateExplosion(m_x*16, m_y*16));
	update_score(10);
	return THING_DIE;
}
/*-----------------------------------------------------------*/
int CCamera::c_nNumCameras = 0;

CCamera::CCamera()
{
	m_bShootable = true;
	SetActionBounds(0, 0, 15, 15);
	SetShootBounds(0, 0, 15, 15);
	c_nNumCameras++;
}

void CCamera::Draw()
{
	// "Turn" to face hero
	int nOffset = 0;
	if (x < m_x - 1)
		nOffset = -1;
	else if (x > m_x + 1)
		nOffset = 1;
	DRAW_SPRITE16A(pVisView, m_a, m_b + nOffset, CALC_XOFFSET(m_x,0), CALC_YOFFSET(m_y));
}

int CCamera::OnHeroShot()
{
	update_score(100, m_x, m_y);
	AddThing(CreateExplosion(m_x*16, m_y*16));
	c_nNumCameras--;
	if (c_nNumCameras==0) // Shot all cameras in level, issue a bonus!
	{
		update_score(10000, x, y);
	}
	return THING_DIE;
}
/*-----------------------------------------------------------*/
CBanana::CBanana()
{
	m_nState = 0;
	m_bFalls = true;
	m_bShootable = true;
	SetActionBounds(0, 0, 15, 15);
	SetShootBounds(0, 0, 15, 15);
}

void CBanana::Draw()
{
	DRAW_SPRITE16A(pVisView, m_a, m_b + m_nState, CALC_XOFFSET(m_x,0), CALC_YOFFSET(m_y));
}

int CBanana::OnHeroShot()
{
	if (m_nState==0)
	{
		m_nState = 1;
		m_bShootable = false;
	}
	return 0;
}

int CBanana::HeroOverlaps()
{
	if (m_nState==0)
	{
		update_score(100, m_x, m_y);
		update_health(1);
	}
	else
	{
		update_score(200, m_x, m_y);
		update_health(2);
	}
	djSoundPlay(g_iSounds[SOUND_PICKUP]);
	return THING_DIE;
}
/*-----------------------------------------------------------*/
CCoke::CCoke()
{
	m_nShotHeight = -1;
	m_nAnim = 0;
	m_bShootable = true;
	SetShootBounds(0,0,15,15);
}

void CCoke::Draw()
{
	if (IsShot())
	{
		DRAW_SPRITE16A(pVisView, 2, 48 + m_nAnim, CALC_XOFFSET(m_x, 0), CALC_YOFFSET(m_y) - m_nShotHeight);
		return;
	}
	CPickup::Draw();
}

int CCoke::OnHeroShot()
{
	m_nShotHeight = 0;
	m_bShootable = false;
	return 0;
}

int CCoke::HeroOverlaps()
{
	if (IsShot())
	{
		update_score(200, m_x, m_y - (m_nShotHeight/16));
		return THING_DIE;
	}
	return CPickup::HeroOverlaps();
}

int CCoke::Tick()
{
	if (IsShot())
	{
		m_nShotHeight += 12;
		m_nAnim = ((m_nAnim + 1) % 4);

		SetActionBounds (0, -m_nShotHeight, 15, -m_nShotHeight + 15);
		SetVisibleBounds(0, -m_nShotHeight, 15, -m_nShotHeight + 15);
		SetShootBounds  (0, -m_nShotHeight, 15, -m_nShotHeight + 15);

		// Check if hit ceiling or something
		if (CheckCollision(m_x*16+m_iVisibleX1, m_y*16+m_iVisibleY1, m_x*16+m_iVisibleX2, m_y*16+m_iVisibleY2))
		{
			//AddThing(CreateExplosion(m_x*16, 16*(m_y-1)-m_nShotHeight));
			return THING_DIE;
		}
	}
	return 0;
}
/*-----------------------------------------------------------*/
CCrawler::CCrawler()
{
	m_bShootable = true;
	SetActionBounds(0,0,15,15);
	m_nOffset = 0;
	m_nDir = -1; // direction
	m_nXDir = 0;
}

int CCrawler::Tick()
{
	if (m_nOffset==0)
	{
		if ((!check_solid(m_x + m_nXDir, m_y + m_nDir)) || (check_solid(m_x, m_y + m_nDir)))
		{
			m_nDir = -m_nDir;
			return 0;
		}
	}
	m_nOffset += m_nDir;
	if (m_nOffset<=-16)
	{
		m_nOffset = 0;
		m_y--;
	}
	else if (m_nOffset>=16)
	{
		m_nOffset = 0;
		m_y++;
	}
	SetActionBounds(0,m_nOffset,15,m_nOffset+15);
	SetVisibleBounds(0,m_nOffset,15,m_nOffset+15);
	SetShootBounds(0,m_nOffset,15,m_nOffset+15);
	return 0;
}

void CCrawler::Draw()
{
	DRAW_SPRITE16A(pVisView, m_a, SGN(m_nXDir)*m_nDir<0 ? m_b + 3 - anim4_count : m_b + anim4_count, CALC_XOFFSET(m_x,0), CALC_YOFFSET(m_y) + m_nOffset);
}

int CCrawler::OnHeroShot()
{
	update_score(100, m_x, m_y);
	AddThing(CreateExplosion(m_x*16, m_y*16));
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
	DRAW_SPRITE16A(pVisView, m_a, m_b + (m_nSpikePopupCount>0 ? 1 : 0), CALC_XOFFSET(m_x,m_xsmall), CALC_YOFFSET(m_y));
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
CBalloon::CBalloon()
{
	m_nHeight = 0;
	SetActionBounds (0,-16,15,15);
	SetVisibleBounds(0,-16,15,15);
	m_bShootable = true;
}

int CBalloon::HeroOverlaps()
{
	update_score(10000, m_x, m_y - 1 - (m_nHeight / 16));
	return THING_DIE;
}

void CBalloon::Draw()
{
	DRAW_SPRITE16A(pVisView, m_a, m_b,                         CALC_XOFFSET(m_x,m_xsmall), CALC_YOFFSET(m_y) - 16 - m_nHeight);
	DRAW_SPRITE16A(pVisView, m_a, m_b+16+16*((m_nHeight/4)%2), CALC_XOFFSET(m_x,m_xsmall), CALC_YOFFSET(m_y)      - m_nHeight);
}

int CBalloon::Tick()
{
	m_nHeight++;
	SetActionBounds (0, -16 - m_nHeight,15,-16 - m_nHeight + 31);
	SetVisibleBounds(0, -16 - m_nHeight,15,-16 - m_nHeight + 31);
	SetShootBounds  (0, -16 - m_nHeight,15,-16 - m_nHeight + 31);
	// Check if hit ceiling or something
	if (CheckCollision(m_x*16+m_iVisibleX1, m_y*16+m_iVisibleY1, m_x*16+m_iVisibleX2, m_y*16+m_iVisibleY2))
	{
		AddThing(CreateExplosion(m_x*16, 16*(m_y-1)-m_nHeight));
		return THING_DIE;
	}
	return 0;
}

int CBalloon::OnHeroShot()
{
	AddThing(CreateExplosion(m_x*16, 16*(m_y-1)));
	return THING_DIE;
}
/*-----------------------------------------------------------*/
CAcme::CAcme()
{
	m_nHeight = -1;
	m_eLayer = LAYER_TOP;
	SetActionBounds (0,0,31,15);
	SetVisibleBounds(0,0,31,15);
}

int CAcme::HeroOverlaps()
{
	// FIXME: Create some sort of explosion here
	update_health(-1);
	return THING_DIE;
}

void CAcme::Draw()
{
	if (m_nHeight>=0 && m_nHeight<16) // Shake up and down phase
	{
		int nOffset = (m_nHeight/4) % 2;
		DRAW_SPRITE16(pVisView, m_a, m_b,   CALC_XOFFSET(m_x  ,m_xsmall), CALC_YOFFSET(m_y) + (nOffset==0?-1:1));
		DRAW_SPRITE16(pVisView, m_a, m_b+1, CALC_XOFFSET(m_x+1,m_xsmall), CALC_YOFFSET(m_y) + (nOffset==0?-1:1));
	}
	else
	{
		// Falling phase
		DRAW_SPRITE16(pVisView, m_a, m_b,   CALC_XOFFSET(m_x  ,m_xsmall), CALC_YOFFSET(m_y) + MAX(0, m_nHeight));
		DRAW_SPRITE16(pVisView, m_a, m_b+1, CALC_XOFFSET(m_x+1,m_xsmall), CALC_YOFFSET(m_y) + MAX(0, m_nHeight));
	}
}

int CAcme::Tick()
{
	// FIXME: Implement some up-n-down etc
	if (m_nHeight==-1)
	{
		// Test if we are above hero. FIXME: Also test if within view bounds
		if ((x>=m_x) && (x<=m_x+1) && (y>m_y))
		{
			m_nHeight = 0; // Start falling
			m_bShootable = true;
		}
	}
	else if (m_nHeight<16)
	{
		m_nHeight += 2;
	}
	else
	{
		m_nHeight += 8;
//		if (m_nHeight>512) // FIXME. Implement proper behaviour
//			return THING_DIE;
		SetActionBounds (0,m_nHeight,31,15+m_nHeight);
		// Fixme, set always
		SetVisibleBounds(0,m_nHeight,31,15+m_nHeight);
		SetShootBounds  (0,m_nHeight,31,15+m_nHeight);

		if (CheckCollision(m_x*16+m_iVisibleX1, m_y*16+m_iVisibleY1, m_x*16+m_iVisibleX2, m_y*16+m_iVisibleY2))
			return THING_DIE;
	}
	return 0;
}

int CAcme::OnHeroShot()
{
	update_score(500, m_x, m_y);
	return THING_DIE;
}

/*-----------------------------------------------------------*/
CPickup::CPickup()
{
	m_nScoreDiff = 0;
	m_nHealthDiff = 0;
	m_nAnimationCount = -1;
	m_bInventoryItem = false;
	m_bPersistent = false;
	SetActionBounds(0,0,15,15);
}

int CPickup::HeroOverlaps()
{
	if (m_bInventoryItem)
	{
		// Try to add self to hero's inventory, then remove from main list.
		if (!InvAdd(this))
		{
			// Failed (inventory full?) In which case, don't pick up object at all.
			// FIXME: TODO: Play a sound here to indicate failure to pick up
			return 0;
		}
		return THING_REMOVE; // remove but don't delete
	}
	if (m_nScoreDiff!=0) update_score(m_nScoreDiff, m_x, m_y);
	if (m_nHealthDiff!=0) update_health(m_nHealthDiff);
	djSoundPlay(g_iSounds[SOUND_PICKUP]);
	return THING_DIE;
}

void CPickup::Draw()
{
	DRAW_SPRITE16A(pVisView, m_a, m_b + (m_nAnimationCount==-1?0:m_nAnimationCount), CALC_XOFFSET(m_x, m_xsmall), CALC_YOFFSET(m_y));
}

int CPickup::Tick()
{
	if (m_nAnimationCount!=-1)
		m_nAnimationCount = (m_nAnimationCount + 1) % 4;
	return 0;
}

void CPickup::Initialize(int b0, int b1)
{
	m_nScoreDiff = GET_EXTRA(b0, b1, EXTRA_SCOREHI) * 100 + GET_EXTRA(b0, b1, EXTRA_SCORELO);
	m_nHealthDiff = GET_EXTRA(b0, b1, 0);
	m_bFalls = (0!=CHECK_FLAG(b0, b1, FLAG_FALLS));
	if (CHECK_FLAG(b0, b1, FLAG_ANIMATED))
		m_nAnimationCount = 0;
	if (CHECK_FLAG(b0, b1, FLAG_INVENTORYITEM))
		m_bInventoryItem = true;
	if (m_bInventoryItem)
	{
		if (CHECK_FLAG(b0, b1, FLAG_PERSISTENT))
			m_bPersistent = true;
	}
}
/*-----------------------------------------------------------*/
CBoots::CBoots()
{
}

int CBoots::HeroOverlaps()
{
	int nRet = CPickup::HeroOverlaps();
	if (nRet==THING_REMOVE)
	{
		// Successfully picked up? Set hero's jump to higher.
		HeroSetJumpMode(JUMP_POWERBOOTS);
	}
	return nRet;
}

void CBoots::OnInventoryLoad()
{
	// If power-boots loaded, set hero jump mode to higher
	HeroSetJumpMode(JUMP_POWERBOOTS);
}
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
CConveyor::CConveyor()
{
	m_bSolid = true;
	SetActionBounds(-7, -32, 15+8, -1);
	SetSolidBounds(0, 0, 15, 15);
}

int CConveyor::Tick()
{
	if (m_bHeroInside)
	{
		move_hero(m_nDir, 0, false);
	}
	return 0;
}

void CConveyor::Draw()
{
	DRAW_SPRITE16A(pVisView, m_a, m_b + anim4_count, CALC_XOFFSET(m_x,0), CALC_YOFFSET(m_y));
}

void CConveyor::Initialize(int b0, int b1)
{
	m_nDir = (GET_EXTRA(b0,b1,0)==0 ? -1 : 1);
}
/*-----------------------------------------------------------*/
CFlameThrow::CFlameThrow()
{
	m_nDir = 0;
	m_nCount = 0;
	m_eLayer = LAYER_TOP;
}

int CFlameThrow::Tick()
{
	m_nCount = (m_nCount + 1) % 60;
	if (m_nCount < 32)
	{
		SetActionBounds(-1, -1, -1, -1);
	}
	else
	{
		if (m_nDir==-1)
			SetActionBounds(0-32, 0, 47-32, 15);
		else
			SetActionBounds(0, 0, 47, 15);
	}
	return 0;
}

void CFlameThrow::Draw()
{
	if (m_nCount<16)
	{
	}
	else if (m_nCount < 32)
	{
		// Flicker small flame
		if ((m_nCount % 4) < 2)
			DRAW_SPRITE16A(pVisView, m_a, m_b, CALC_XOFFSET(m_x,0), CALC_YOFFSET(m_y));
	}
	else
	{
		// Full flame
		if ((m_nCount % 4) < 2)
		{
			DRAW_SPRITE16A(pVisView, m_a, m_b + m_nDir*1, CALC_XOFFSET(m_x         ,0), CALC_YOFFSET(m_y));
			DRAW_SPRITE16A(pVisView, m_a, m_b + m_nDir*2, CALC_XOFFSET(m_x+m_nDir*1,0), CALC_YOFFSET(m_y));
			DRAW_SPRITE16A(pVisView, m_a, m_b + m_nDir*3, CALC_XOFFSET(m_x+m_nDir*2,0), CALC_YOFFSET(m_y));
		}
		else
		{
			DRAW_SPRITE16A(pVisView, m_a, m_b + m_nDir*2, CALC_XOFFSET(m_x         ,0), CALC_YOFFSET(m_y));
			DRAW_SPRITE16A(pVisView, m_a, m_b + m_nDir*1, CALC_XOFFSET(m_x+m_nDir*1,0), CALC_YOFFSET(m_y));
			DRAW_SPRITE16A(pVisView, m_a, m_b + m_nDir*3, CALC_XOFFSET(m_x+m_nDir*2,0), CALC_YOFFSET(m_y));
		}
	}

}

void CFlameThrow::Initialize(int b0, int b1)
{
	m_nDir = (GET_EXTRA(b0,b1,0)==0 ? -1 : 1);
	assert(m_nDir==-1 || m_nDir==1);
	if (m_nDir==-1)
		SetVisibleBounds(0-32, 0, 47-32, 15);
	else
		SetVisibleBounds(0, 0, 47, 15);
}

int CFlameThrow::HeroOverlaps()
{
	if (!HeroIsHurting())
	{
		update_health(-1);
		HeroSetHurting();
	}
	return 0;
}
/*-----------------------------------------------------------*/
#define EXPLODECOUNT (12)
//int nFoo=-1;
CDynamite::CDynamite()
{
	m_nCenter = -1;
	m_nX1Right = 0;
	m_nX2Right = 0;
	m_nX1Left = 0;
	m_nX2Left = 0;
	m_nCount = -1;
	m_eLayer = LAYER_TOP;
	m_bFalls = true;
	//nFoo=-1;
	SetVisibleBounds(0 - 64, 0, 15 + 64, 15);
}

int CDynamite::Tick()
{
	//nFoo++;if ((nFoo%4)!=0) return 0;
	if (m_nCount==(EXPLODECOUNT-6)) // fixme, fix sound so its explodecount
		djSoundPlay( g_iSounds[SOUND_EXPLODE] );
	m_nCount++;
	if (m_nCount >= EXPLODECOUNT)
		m_bFalls = false;
	if (m_nCount >= EXPLODECOUNT + 8*2)
		return THING_DIE;
	return 0;
}

void CDynamite::Draw()
{
	if (m_nCount < EXPLODECOUNT)
	{
		DRAW_SPRITE16A(pVisView, m_a, m_b + (m_nCount % 2), CALC_XOFFSET(m_x,0), CALC_YOFFSET(m_y));
		return;
	}
	int nStage = (m_nCount - EXPLODECOUNT) / 2;
	int nGraphicsOffset = (m_nCount % 2) * 4; // Either 0 or 4
	int nOffset = 0;
	int i, j;

	// Center
	if (nStage>=0 && nStage<=3)
		m_nCenter = nStage;
	else
		m_nCenter = -1;
	if (m_nCenter!=-1)
	{
		DRAW_SPRITE16A(pVisView, m_a, m_b + m_nCenter + 2, CALC_XOFFSET(m_x,0), CALC_YOFFSET(m_y));
	}

	// Right side
	m_nX1Right = 0;
	m_nX2Right = 0;
	bool bKeepGoing = true;
	for ( i=0; i<4 && bKeepGoing; i++ )
	{
		nOffset = -2 + i + nStage - 1;
		if (nOffset>1)
		{
			for ( j=1; j<nOffset; j++ )
			{
				if (!check_solid(m_x+j, m_y+1) || check_solid(m_x+j, m_y))
					bKeepGoing = false;
			}
		}
		if (bKeepGoing && nOffset>=1)
		{
			if (!check_solid(m_x+nOffset, m_y+1) || check_solid(m_x+nOffset, m_y))
				bKeepGoing = false;
		}
		if (nOffset>=1 && nOffset<=4 && bKeepGoing)
		{
			if (m_nX1Right==0)
				m_nX1Right = nOffset;
			DRAW_SPRITE16A(pVisView, m_a, m_b + nGraphicsOffset + 2 + (3-i), CALC_XOFFSET(m_x+nOffset,0), CALC_YOFFSET(m_y));
			m_nX2Right++;
		}
	}

	// Left side
	m_nX1Left = 0;
	m_nX2Left = 0;
	bKeepGoing = true;
	for ( i=0; i<4 && bKeepGoing; i++ )
	{
		nOffset = -2 + i + nStage - 1;
		if (nOffset>1)
		{
			for ( j=1; j<nOffset; j++ )
			{
				if (!check_solid(m_x-j, m_y+1) || check_solid(m_x-j, m_y))
					bKeepGoing = false;
			}
		}
		if (bKeepGoing && nOffset>=1)
		{
			if (!check_solid(m_x-nOffset, m_y+1) || check_solid(m_x-nOffset, m_y))
				bKeepGoing = false;
		}
		if (nOffset>=1 && nOffset<=4 && bKeepGoing)
		{
			if (m_nX1Left==0)
				m_nX1Left = nOffset;
			DRAW_SPRITE16A(pVisView, m_a, m_b + nGraphicsOffset + 2 + i, CALC_XOFFSET(m_x-nOffset,0), CALC_YOFFSET(m_y));
			m_nX2Left++;
		}
	}
}

int CDynamite::HeroOverlaps()
{
	if (!HeroIsHurting())
	{
		update_health(-1);
		HeroSetHurting();
	}
	return 0;
}

bool CDynamite::OverlapsBounds(int x, int y)
{
	int x1, x2, y1, y2;
	// Center
	if (m_nCenter!=-1)
	{
		x1 = m_x*16;
		x2 = x1 + 15;
		y1 = m_y*16;
		y2 = y1 + 15;
		if (OVERLAPS(x, y, x+15, y+31, x1, y1, x2, y2))
			return true;
	}
	// Right side of flames
	if (m_nX2Right>0)
	{
		x1 = m_x*16 + m_nX1Right*16;
		x2 = x1 + m_nX2Right*16 - 1;
		y1 = m_y*16;
		y2 = y1 + 15;
		if (OVERLAPS(x, y, x+15, y+31, x1, y1, x2, y2))
			return true;
	}
	// Left side of flames
	if (m_nX2Left>0)
	{
		x1 = m_x*16 - 16 * (m_nX1Left + m_nX2Left - 1);
		x2 = x1 + m_nX2Left*16 - 1;
		y1 = m_y*16;
		y2 = y1 + 15;
		if (OVERLAPS(x, y, x+15, y+31, x1, y1, x2, y2))
			return true;
	}
	return false;
}

void CDynamite::DrawActionBounds(const djColor &Color)
{
	djgSetColorFore(pVisView, Color);
	if (m_nCenter!=-1)
	{
		djgDrawRectangle(pVisView,
			CALC_XOFFSET(m_x,0),
			CALC_YOFFSET(m_y),
			16,
			16);
	}
	if (m_nX2Right>0)
	{
		djgDrawRectangle(pVisView,
			CALC_XOFFSET(m_x,0)+m_nX1Right*16,
			CALC_YOFFSET(m_y)+0,
			m_nX2Right*16,
			16);
	}
	if (m_nX2Left>0)
	{
		djgDrawRectangle(pVisView,
			CALC_XOFFSET(m_x,0) - 16 * (m_nX1Left + m_nX2Left - 1),
			CALC_YOFFSET(m_y)+0,
			m_nX2Left*16,
			16);
	}
}
/*-----------------------------------------------------------*/
CFan::CFan()
{
	m_nAnim = 0;
	m_nAnimTimer = 0;
	m_nSpeed = 0;
	m_eLayer = LAYER_TOP;
	m_nDir = 0;
	m_bStopped = false;
	m_bShootable = true;
	SetVisibleBounds(0, 0, 15, 31);
	SetActionBounds(0, 0, 4*16-1, 31);
	SetShootBounds(0, 0, 15, 31);
}

int CFan::Tick()
{
	if (m_nSpeed!=0)
	{
		m_nAnimTimer++;
		if (m_nAnimTimer>=m_nSpeed)
		{
			m_nAnimTimer = 0;
			m_nAnim = (m_nAnim + 1) % 4;

			if (m_bStopped && m_nSpeed<6)
			{
				m_nSpeed++;
				if (m_nSpeed>=6)
					m_nSpeed = 0;
			}
		}
	}
	if (!m_bStopped && m_bHeroInside && m_nSpeed!=0)
	{
		move_hero(m_nDir, 0, false);
	}
	return 0;
}

void CFan::Draw()
{
	DRAW_SPRITE16A(pVisView, m_a, m_b + m_nAnim   , CALC_XOFFSET(m_x,0), CALC_YOFFSET(m_y));
	DRAW_SPRITE16A(pVisView, m_a, m_b + m_nAnim+16, CALC_XOFFSET(m_x,0), CALC_YOFFSET(m_y+1));
}

void CFan::Initialize(int b0, int b1)
{
	m_nDir = (GET_EXTRA(b0, b1, 0)==0 ? -1 : 1);
	m_nSpeed = GET_EXTRA(b0, b1, 1);

	SetActionBounds(0 + (m_nDir==-1?-48:0), 0, 4*16-1 + (m_nDir==-1?-48:0), 31);
}

int CFan::OnHeroShot()
{
	// First time shot?
	if (!m_bStopped)
	{
		m_bStopped = true;
	}
	else // Not first time shot .. just spin
	{
		m_nSpeed = 1;
		m_nAnimTimer = m_nSpeed;
	}
	return 0;
}
/*-----------------------------------------------------------*/
CFirepower::CFirepower()
{
	SetActionBounds(0, 0, 15, 15);
	SetVisibleBounds(0, 0, 15, 15);
}

void CFirepower::Draw()
{
	DRAW_SPRITE16A(pVisView, m_a, m_b, CALC_XOFFSET(m_x,0), CALC_YOFFSET(m_y));
}

int CFirepower::HeroOverlaps()
{
	HeroModifyFirepower(1);
	return THING_DIE;
}
/*-----------------------------------------------------------*/
CDust::CDust()
{
	SetVisibleBounds(0, 0, 15, 15);
	int i;
	for ( i=0; i<4; i++ )
	{
		m_anAnim[i] = -1;
		m_anX[i] = -1;
		m_anY[i] = -1;
	}
	m_nPuff = -1;
}

int CDust::Tick()
{
	// FIXME:TEMP: This should be 4 puffs, I've made it 1 puff, because
	// I'm having horrible problems (pixie dust effect if hero jumps immediately
	// after landing, as dust clouds follow him into the air :)
	// ACTUALLY, I think its better with just 1, always. So FIXME: Delete support
	// for more than one.
	if (m_nPuff<=-1)
	{
		m_nPuff++;
		m_anX[m_nPuff] = x;
		m_anY[m_nPuff] = y;
	}
	int i;
	bool bAllDead = true;
	for ( i=0; i<=m_nPuff; i++ )
	{
		m_anAnim[i]++;
		if (m_anAnim[i]<=3)
			bAllDead = false;
	}
	if (bAllDead)
		return THING_DIE;
	return 0;
}

void CDust::Draw()
{
	int i;
	for ( i=0; i<=m_nPuff; i++ )
	{
		if (m_anAnim[i]<=3)
		{
			DRAW_SPRITE16A(pVisView, 5, 20 + m_anAnim[i], CALC_XOFFSET(m_anX[i],0), CALC_YOFFSET(m_anY[i]));
		}
	}
}
/*-----------------------------------------------------------*/
// Create helpers
/*-----------------------------------------------------------*/
CThing *CreateFloatingScore(int x, int y, int score)
{
	// A floating "0" would look a bit silly :)
	if ( score == 0 )
		return NULL;
	CFloatingScore * pScore;
	pScore = new CFloatingScore;
	pScore->SetPosition( x, y );
	pScore->SetScore( score );
	return pScore;
}

CThing *CreateExplosion(int nX, int nY)
{
	CExplosion *pExplosion = new CExplosion;
	pExplosion->SetPosition(nX/16, nY/16);
	pExplosion->m_xOffset = 0;//nX % 16;
	pExplosion->m_yOffset = nY % 16;
	djSoundPlay( g_iSounds[SOUND_SOFT_EXPLODE] );
	return pExplosion;
}

CThing *CreateDust(int nX, int nY)
{
	CDust *pDust = new CDust;
	pDust->SetPosition(nX, nY);
	pDust->SetType(TYPE_DUST);
	return pDust;
}
