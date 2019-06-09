/*--------------------------------------------------------------------------*/
// thing.cpp
/*
Copyright (C) 2000-2018 David Joffe
*/
/*--------------------------------------------------------------------------*/
#include "thing.h"
#include "djtypes.h"
#include "graph.h"
#include "mission.h"//For spriteset data, not sure if mad about this dependency [low]
#include "block.h" // Class type definitions
#include "djlog.h"
#include "hero.h"
#include <assert.h>
#include "game.h"
#include "level.h"//For LEVCHAR_FOREA etc. for CRocket
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
REGISTER_THING(CExit,          TYPE_EXIT, NULL);
REGISTER_THING(CLetter,        TYPE_LETTER, CLetterPerLevelInit);
REGISTER_THING(CBox,           TYPE_BOX, NULL);
REGISTER_THING(CTeleporter,    TYPE_TELEPORTER, NULL);
REGISTER_THING(CRocket,        TYPE_ROCKET, NULL);
REGISTER_THING(CDoor,          TYPE_DOOR, NULL);
REGISTER_THING(CKey,           TYPE_KEY, NULL);
REGISTER_THING(CAccessCard,    TYPE_ACCESSCARD, NULL);
REGISTER_THING(CAntivirus,     TYPE_ANTIVIRUS, NULL);
REGISTER_THING(CMasterComputer,TYPE_MASTERCOMPUTER, NULL);
REGISTER_THING(CDoorActivator, TYPE_DOORACTIVATOR, NULL);
REGISTER_THING(CSoftBlock,     TYPE_SOFTBLOCK, NULL);
REGISTER_THING(CCamera,        TYPE_CAMERA, CCameraPerLevelInit);
REGISTER_THING(CBanana,        TYPE_BANANA, NULL);
REGISTER_THING(CSoda,          TYPE_SODACAN, NULL);
REGISTER_THING(CFullHealth,    TYPE_FULLHEALTH, NULL);
REGISTER_THING(CBalloon,       TYPE_BALLOON, NULL);
REGISTER_THING(CAcme,          TYPE_ACME, NULL);
REGISTER_THING(CPickup,        TYPE_PICKUP, NULL);
REGISTER_THING(CBoots,         TYPE_POWERBOOTS, NULL);
REGISTER_THING(CLift,          TYPE_LIFT, NULL);
REGISTER_THING(CCrumblingFloor,TYPE_CRUMBLINGFLOOR, NULL);
REGISTER_THING(CConveyor,      TYPE_CONVEYOR, NULL);
REGISTER_THING(CFlameThrow,    TYPE_FLAMETHROW, NULL);
REGISTER_THING(CDynamite,      TYPE_DYNAMITE, NULL);
REGISTER_THING(CFan,           TYPE_FAN, NULL);
REGISTER_THING(CFirepower,     TYPE_FIREPOWER, NULL);
REGISTER_THING(CDust,          TYPE_DUST, NULL);
REGISTER_THING(CWater,         TYPE_WATER, NULL);
REGISTER_THING(CBlock,         TYPE_BLOCK, NULL);

/*-----------------------------------------------------------*/
CThing::CThing()
{
	m_bFalls = false;
	m_x = m_y = 0;
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
		CALC_XOFFSET(m_x)+m_iActionX1 + m_xoffset,
		CALC_YOFFSET(m_y)+m_iActionY1 + m_yoffset,
		(m_iActionX2 - m_iActionX1)+1,
		(m_iActionY2 - m_iActionY1)+1);
}

bool CThing::OverlapsBounds(int x, int y)
{
	if (m_iActionX1<0 && m_iActionY1<0 && m_iActionX2<0 && m_iActionY2<0)
		return false;
	return OVERLAPS(
		x, y,
		x+15, y+31,
		m_x*16+m_iActionX1+m_xoffset, m_y*16+m_iActionY1+m_yoffset,
		m_x*16+m_iActionX2+m_xoffset, m_y*16+m_iActionY2+m_yoffset);
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
#ifdef EXPERIMENTAL_SPRITE_AUTO_DROPSHADOWS
	DRAW_SPRITEA_SHADOW(pVisView, m_a, m_b, 1+CALC_XOFFSET(m_x), 1+CALC_YOFFSET(m_y) + m_nYOffset*2,16,16);
#endif
	DRAW_SPRITE16A(pVisView, m_a, m_b, CALC_XOFFSET(m_x), CALC_YOFFSET(m_y) + m_nYOffset*2);
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
		AddThing(CreateExplosion(PIXELX, PIXELY, 1));
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
	SetShootBounds(4, 0, 12, 15);
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
	AddThing(CreateExplosion(PIXELX, PIXELY));

	// Spawn whatever was inside the box
	sprite_factory( m_iContentsA, m_iContentsB, m_x, m_y, 0, false );

	return THING_DIE;
}

void CBox::Draw()
{
	if ( ( m_a | m_b ) != 0 )
		DRAW_SPRITE16(pVisView, m_a, m_b, CALC_XOFFSET(m_x), CALC_YOFFSET(m_y));
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
	DRAW_SPRITE16A(pVisView, m_a, m_b, CALC_XOFFSET(m_x), CALC_YOFFSET(m_y));
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
		m_height++;
		// We set these immediately, otherwise the hero may be outside the bounds,
		// causing the lift to drop.
		//SetActionBounds(0, -32 - (m_height-1)*16, 15, -1 - (m_height-1)*16);
		SetActionBounds(0, -32 - (m_height-1)*16, 15, -1 - (m_height-1)*16);
		SetSolidBounds(0, 0 - (m_height-1)*16, 15, 15);
		SetVisibleBounds(0, 0 - (m_height-1)*16, 15, 15);
	}
	return 0;
};

void CLift::Draw()
{
	for ( int i=0; i<m_height; ++i )
	{
		int nLiftBlockY = m_y-m_height+i+1;
		// if section of lift is on-screen
		if ( INBOUNDS( m_x, nLiftBlockY,
			xo, yo, xo + VIEW_WIDTH + xo_small, yo + VIEW_HEIGHT ) )
		{
			if (i == 0)
			{
				DRAW_SPRITE16(pVisView, 1, 14, CALC_XOFFSET(m_x), CALC_YOFFSET(nLiftBlockY));
			}
			else
			{
				DRAW_SPRITE16(pVisView, 1, 15, CALC_XOFFSET(m_x), CALC_YOFFSET(nLiftBlockY));
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
	m_countdown = 6;
	m_xoffset   = 0;
	m_yoffset   = 0;
	m_width     = 1;
	m_iType     = -1; // fixme
	m_eLayer    = LAYER_TOP;
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
	if (m_iType==1)
	{
		DRAW_SPRITE16A(pVisView,
			16, 16 - (m_countdown+1),//Top right corner of new spriteset '16' [dj2017-08] .. 6 frames of animation
			CALC_XOFFSET(m_x) + m_xoffset, CALC_YOFFSET(m_y) + m_yoffset);
	}
	else
	{
		DRAW_SPRITE16A(pVisView, 5, 16 + (3 - ((m_countdown+2)%4)), CALC_XOFFSET(m_x) + m_xoffset, CALC_YOFFSET(m_y) + m_yoffset);
	}
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
	DRAW_SPRITE16(pVisView, 5,  96 + nOffset, CALC_XOFFSET(m_x  ), CALC_YOFFSET(m_y-1));
	DRAW_SPRITE16(pVisView, 5,  97 + nOffset, CALC_XOFFSET(m_x+1), CALC_YOFFSET(m_y-1));
	DRAW_SPRITE16(pVisView, 5, 112 + nOffset, CALC_XOFFSET(m_x  ), CALC_YOFFSET(m_y  ));
	DRAW_SPRITE16(pVisView, 5, 113 + nOffset, CALC_XOFFSET(m_x+1), CALC_YOFFSET(m_y  ));
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
	// Action key testing box
	SetActionBounds ( -8, -16, 15+8,  15);
	SetVisibleBounds(-16, -32, 15+16, 15);
}

int CTeleporter::Action()
{
	// If already activated, don't re-activative, this prevents us sitting in a freeze-loop if user sits holding in action key on the teleporter [dj2017-06-22]
	if (m_bActivated)
		return 0;
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
	DRAW_SPRITE16A(pVisView, 5,  48+nOffset, CALC_XOFFSET(m_x-1), CALC_YOFFSET(m_y-2));
	DRAW_SPRITE16A(pVisView, 5,  49+nOffset, CALC_XOFFSET(m_x  ), CALC_YOFFSET(m_y-2));
	DRAW_SPRITE16A(pVisView, 5,  50+nOffset, CALC_XOFFSET(m_x+1), CALC_YOFFSET(m_y-2));
	DRAW_SPRITE16A(pVisView, 5,  64, CALC_XOFFSET(m_x-1), CALC_YOFFSET(m_y-1));
	DRAW_SPRITE16A(pVisView, 5,  65, CALC_XOFFSET(m_x  ), CALC_YOFFSET(m_y-1));
	DRAW_SPRITE16A(pVisView, 5,  66, CALC_XOFFSET(m_x+1), CALC_YOFFSET(m_y-1));
	DRAW_SPRITE16A(pVisView, 5,  80, CALC_XOFFSET(m_x-1), CALC_YOFFSET(m_y  ));
	DRAW_SPRITE16A(pVisView, 5,  81, CALC_XOFFSET(m_x  ), CALC_YOFFSET(m_y  ));
	DRAW_SPRITE16A(pVisView, 5,  82, CALC_XOFFSET(m_x+1), CALC_YOFFSET(m_y  ));
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
	m_bShootable = true;//Set shootable flag, but don't do anything 'special' other than STOP the bullet.
	SetSolidBounds(0,0,15,15);
	SetShootBounds(0,0,15,15);
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
	// Adding ability for door sprite to be animated, as access card is a 'special case' door but is animated [dj2017-06]
	if (CHECK_FLAG(m_a, m_b, FLAG_ANIMATED))
	{
	extern int anim4_count;//[dj2017-06-26 gross, not mad about this 'global' but will do for now]
	DRAW_SPRITE16A(pVisView, m_a, m_b+anim4_count, CALC_XOFFSET(m_x), CALC_YOFFSET(m_y));
	}
	else
	{
	DRAW_SPRITE16A(pVisView, m_a, m_b+m_nOpenState, CALC_XOFFSET(m_x), CALC_YOFFSET(m_y));
	}
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
		//dj2018-03 Use the sprite metadata score for the key's pickup score value
		int nScoreDiff = GET_EXTRA(m_a, m_b, EXTRA_SCOREHI) * 100 + GET_EXTRA(m_a, m_b, EXTRA_SCORELO);
		if (nScoreDiff<0) nScoreDiff = 0;
		if (nScoreDiff>0)
			update_score(nScoreDiff, m_x, m_y);
		djSoundPlay( g_iSounds[SOUND_KEY_PICKUP] );
		return THING_REMOVE;
	}
	// Inventory full? Don't pick up or do anything.
	return 0;
}

void CKey::Draw()
{
	DRAW_SPRITE16A(pVisView, m_a, m_b, CALC_XOFFSET(m_x), CALC_YOFFSET(m_y));
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
		if (pThing->GetTypeID()==TYPE_KEY
			|| pThing->GetTypeID()==TYPE_ACCESSCARD//fixLOW "IsKindOf" would be more appropriate but we don't have that type of RTTI info [yet] [dj2017-06]
			|| pThing->GetTypeID()==TYPE_ANTIVIRUS//fixLOW "IsKindOf" would be more appropriate but we don't have that type of RTTI info [yet] [dj2017-08]
			)
		{
			if (((CKey*)pThing)->GetID()==m_nID)
			{
				InvRemove(pThing);

				djSoundPlay( g_iSounds[SOUND_OPENDOOR] );

				// Find all doors belonging to this key and tell them to open.
				unsigned int j;
				for ( j=0; j<g_apThings.size(); j++ )
				{
					if (g_apThings[j]->GetTypeID()==TYPE_DOOR)//Should be "IsKindOf(TYPE_DOOR)" [low]
					{
						CDoor *pDoor = (CDoor*)g_apThings[j];
						if (pDoor->GetID()==m_nID)
							pDoor->OpenDoor();
					}
				}

				OnActivated();

				delete pThing; // Delete key
				return 0;
			}
		}
	}
	return 0;
}

void CDoorActivator::Draw()
{
	// Adding ability for door sprite to be animated, as access card is a 'special case' door but is animated [dj2017-06]
	if (CHECK_FLAG(m_a, m_b, FLAG_ANIMATED))
	{
	extern int anim4_count;//[dj2017-06-26 gross, not mad about this 'global' but will do for now]
	DRAW_SPRITE16A(pVisView, m_a, m_b+anim4_count, CALC_XOFFSET(m_x), CALC_YOFFSET(m_y));
	}
	else
	{
	DRAW_SPRITE16A(pVisView, m_a, m_b, CALC_XOFFSET(m_x), CALC_YOFFSET(m_y));
	}
}
/*-----------------------------------------------------------*/
CMasterComputer::CMasterComputer()
{
	SetActionBounds(-8,-16,31+8,15);
	SetVisibleBounds(0,-16,31,15);
}
void CMasterComputer::OnActivated()
{
	// The parent class is used to check if activated, meaning, if hero inserted antivirus
	// floppy into drive. When this happens, the OnActivated() virtual function is called,
	// so we can do special handling here not relevant to other types of 'activated' key/door type stuff.

	// OK, right here, you've more or less 'won the game' (at least that's the idea),
	// you've just saved the world. Should do something here slightly less unspectacular.

	// [TODO] What exactly to do over here? You've just saved the world.
	update_score(10000, m_x, m_y - 1);
}
void CMasterComputer::Draw()
{
	DRAW_SPRITEA(pVisView, m_a, m_b-16, CALC_XOFFSET(m_x), CALC_YOFFSET(m_y-1),32,32);
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
	DRAW_SPRITE16A(pVisView, m_a, m_b, CALC_XOFFSET(m_x), CALC_YOFFSET(m_y));
}

int CSoftBlock::OnHeroShot()
{
	AddThing(CreateExplosion(PIXELX, PIXELY));
	update_score(10);
	return THING_DIE;
}
/*-----------------------------------------------------------*/
int CCamera::c_nNumCameras = 0;

CCamera::CCamera()
{
	m_bShootable = true;
	SetActionBounds(0, 0, 15, 15);
	SetShootBounds(7, 0, 8, 15);//dj2018-01-12 (To fix "the banana problem",) make shoot bounds 'thin vertical line' so that the visual effect of rendering bullet 'one last frame' that collided with us, shows us visually with that bullet impacting near the center, looks a bit odd otherwise
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

#ifdef EXPERIMENTAL_SPRITE_AUTO_DROPSHADOWS
	DRAW_SPRITEA_SHADOW(pVisView, m_a, m_b + nOffset, CALC_XOFFSET(m_x)+1, CALC_YOFFSET(m_y)+1,16,16);
#endif
	DRAW_SPRITE16A(pVisView, m_a, m_b + nOffset, CALC_XOFFSET(m_x), CALC_YOFFSET(m_y));
}

int CCamera::OnHeroShot()
{
	update_score(100, m_x, m_y);
	AddThing(CreateExplosion(PIXELX, PIXELY));
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
	SetShootBounds(7, 0, 8, 15);//dj2018-01-12 (To fix "the banana problem",) make shoot bounds 'thin vertical line' so that the visual effect of rendering bullet 'one last frame' that collided with us, shows us visually with that bullet impacting near the center, looks a bit odd otherwise
}

void CBanana::Draw()
{
#ifdef EXPERIMENTAL_SPRITE_AUTO_DROPSHADOWS
	DRAW_SPRITEA_SHADOW(pVisView, m_a, m_b + m_nState, CALC_XOFFSET(m_x), CALC_YOFFSET(m_y),16,16);
	DRAW_SPRITE16A(pVisView, m_a, m_b + m_nState, CALC_XOFFSET(m_x)-1, CALC_YOFFSET(m_y)-1);
#else
	DRAW_SPRITE16A(pVisView, m_a, m_b + m_nState, CALC_XOFFSET(m_x), CALC_YOFFSET(m_y));
#endif
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
CSoda::CSoda()
{
	m_nShotHeight = -1;
	m_nAnim = 0;
	m_bShootable = true;
	SetShootBounds(7, 0, 8, 15);//dj2018-01-12 (To fix "the banana problem",) make shoot bounds 'thin vertical line' so that the visual effect of rendering bullet 'one last frame' that collided with us, shows us visually with that bullet impacting near the center, looks a bit odd otherwise
}

void CSoda::Draw()
{
	if (IsShot())
	{
		DRAW_SPRITE16A(pVisView, 2, 48 + m_nAnim, CALC_XOFFSET(m_x), CALC_YOFFSET(m_y) - m_nShotHeight);
		return;
	}
	CPickup::Draw();
}

int CSoda::OnHeroShot()
{
	m_nShotHeight = 0;
	m_bShootable = false;
	return 0;
}

int CSoda::HeroOverlaps()
{
	if (IsShot())
	{
		update_score(200, m_x, m_y - (m_nShotHeight/16));
		return THING_DIE;
	}
	return CPickup::HeroOverlaps();
}

int CSoda::Tick()
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
	return CPickup::Tick();//<- This is important for animation currently as CPickup::Tick() handles animation count [dj2017-06-24]
}
/*-----------------------------------------------------------*/
CFullHealth::CFullHealth()
{
	m_bShootable = false;
	SetShootBounds(0,0,15,15);
}

int CFullHealth::HeroOverlaps()
{
	// Give full health here
	update_health(100000);//<- This is arbitrary but any very big number should do as long as it's bigger than MAX_HEALTH
	// Create floating score
	update_score(1000, m_x, m_y);
	//ShowGameMessage("Full Health!", 32);
	// Should we maybe flash the health display or something here to show full health restored
	// TODO: Play special sound?
	return CPickup::HeroOverlaps();
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
	DRAW_SPRITE16A(pVisView, m_a, m_b,                         CALC_XOFFSET(m_x), CALC_YOFFSET(m_y) - 16 - m_nHeight);
	DRAW_SPRITE16A(pVisView, m_a, m_b+16+16*((m_nHeight/4)%2), CALC_XOFFSET(m_x), CALC_YOFFSET(m_y)      - m_nHeight);
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
		AddThing(CreateExplosion(PIXELX, 16*(m_y-1)-m_nHeight));
		return THING_DIE;
	}
	return 0;
}

int CBalloon::OnHeroShot()
{
	AddThing(CreateExplosion(PIXELX, 16*(m_y-1)));
	return THING_DIE;
}
/*-----------------------------------------------------------*/
CAcme::CAcme() :
	m_nState(0),
	m_nCounter(0)
{
	m_eLayer = LAYER_TOP;
	SetActionBounds (0,0,BLOCKW*2-1,BLOCKH-1);
	SetVisibleBounds(0,0,BLOCKW*2-1,BLOCKH-1);
	SetShootBounds  (0,0,BLOCKW*2-1,BLOCKH-1);
	//[dj2018-03] It's very important we NOT be shootable until after we start falling, otherwise there's a bug where you can shoot the acme by shooting just above it along the floor it's attached to, from the side
	m_bShootable = false;
}

int CAcme::HeroOverlaps()
{
	// If walking in a tight passage only 2 blocks in height then it 'immediately'
	// touches you while shaking still ... so make it only pops on contact if
	// already past the m_nCounter to get to the 'falling' state ... otherwise
	// it immediately pops, when in fact you should be able to walk past it entirely.
	// (If unsure what I mean, see 8/9 Aug 2017 stream 03:03:00) [dj2017-08]
	// Not sure if this is quite the best way .. alternatively could fiddle
	// with the bounding boxes. The disadvantage of this approach is if it's overhanging
	// or floating in mid-air, you won't collide with it until it's falling, which is
	// 'wrong' (HOWEVER you're not supposed to ever place them that way in the level editor)
	if (m_nState>=3)
	{
		update_health(-1);
		CreateOnDestroyedEffects();
		return THING_DIE;
	}
	return 0;
}

void CAcme::Draw()
{
#ifdef EXPERIMENTAL_SPRITE_AUTO_DROPSHADOWS
	DRAW_SPRITEA_SHADOW(pVisView, m_a, m_b, 1+CALC_XOFFSET(m_x) + m_xoffset, 1+CALC_YOFFSET(m_y) + m_yoffset, 32, 16);
#endif
	DRAW_SPRITEA(pVisView, m_a, m_b, CALC_XOFFSET(m_x) + m_xoffset, CALC_YOFFSET(m_y) + m_yoffset, 32, 16);
}

int CAcme::Tick()
{
	switch (m_nState)
	{
	case 0:// Normal default state
		if (IsInView())
		{
		// Test if we are above hero.
// FIXME I think this doesn't work for half-block under - it should
// FIXME Also use check_solid for problem of if solid above us but below Acme, then shouldn't fall (I think (??))
		if ((x>=m_x) && (x<=m_x+1) && (y>m_y))
		{
			// Sort of 'cast a ray downwards' from us, to hero's Y position, to see if
			// there are eg solid floors BETWEEN us and the hero .. if there are, don't
			// drop. [dj2017-08 - todo maybe confirm if DN1 behaves like this, or else just decide how we want it]

			int nY = m_y+1;
			bool bClear = true;
			while (bClear && nY<y)
			{
				if (check_solid(m_x,nY) || check_solid(m_x+1,nY))
					bClear = false;
				++nY;
			};

			if (bClear)
			{
				// Start breaking off (first shake up & down)
				++m_nState;
			}
		}
		}
		break;
	case 1:// Shake up and down briefly when activated
		switch (m_nCounter % 4)//Modulo 4
		{
		case 0: m_yoffset = -1; break;
		case 1: m_yoffset = -1; break;
		case 2: m_yoffset =  1; break;
		case 3: m_yoffset =  1; break;
		}
		++m_nCounter;
		if (m_nCounter>20)
		{
			m_bShootable = true;//dj2018-03-30 Fix bug that you could shoot this from the side [sort of above] by shooting over the top of the floor we're attached to - make shootable only once we're falling
			++m_nState;
			m_yoffset = 0;
		}
		break;
	case 2://Falling (initial little bit) - Don't start collision detection until we've fallen at least past the block we're attached to, otherwise it immediately explodes
		m_yoffset += 4;
		if (m_yoffset>=BLOCKH)
		{
			m_yoffset -= BLOCKH;
			++m_y;
			++m_nState;
		}
		break;
	case 3://Falling
		m_yoffset += 8;
		while (m_yoffset > BLOCKH)
		{
			m_yoffset -= BLOCKH;
			++m_y;
		}

		// If hit bottom of level or collide with solid, explode
		if (m_y>=LEVEL_HEIGHT-1 || CheckCollision(m_x*16+m_iVisibleX1, PIXELY+m_iVisibleY1, m_x*16+m_iVisibleX2, PIXELY+m_iVisibleY2))
		{
			CreateOnDestroyedEffects();
			return THING_DIE;
		}

		break;
	}
	
	return 0;
}

void CAcme::CreateOnDestroyedEffects()
{
	// [dj2018-01] Create several 'intermixed' smallest-type and next-biggest-type
	// explosions, intermixed, with a little random variation in x,y position.
	// Note only 1 plays the sound (or actually 2, combine both sounds? tweak / play around with this later? want a sound similar to 'big' explosion but more 'immediate' [LOW prio] - dj2018-01)
	AddThing(CreateExplosion(((rand()%4)-2) + m_x*BLOCKW           , PIXELY + ((rand()%6)-3),0, 0));
	AddThing(CreateExplosion(((rand()%4)-2) + m_x*BLOCKW           , PIXELY + ((rand()%6)-3),1, 1));
	AddThing(CreateExplosion(((rand()%4)-2) + m_x*BLOCKW+ BLOCKW   , PIXELY + ((rand()%6)-3),0,-1));
	AddThing(CreateExplosion(((rand()%4)-2) + m_x*BLOCKW+ BLOCKW   , PIXELY + ((rand()%6)-3),1,-1));
	AddThing(CreateExplosion(((rand()%4)-2) + m_x*BLOCKW+(BLOCKW/2), PIXELY + ((rand()%6)-3),0,-1));
	AddThing(CreateExplosion(((rand()%4)-2) + m_x*BLOCKW+(BLOCKW/2), PIXELY + ((rand()%6)-3),1,-1));
}

int CAcme::OnHeroShot()
{
	//dj2018-03-30 This only becomes shootable once it starts falling (otherwise there's a bug where we can shoot it by shooting just along the top of the floor we're attached to)
	if (m_bShootable)
	{
		update_score(500, m_x, m_y/*+m_yoffset*/);
		CreateOnDestroyedEffects();
		return THING_DIE;
	}
	return 0;
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

	// [dj2017-12] Moving these to before the m_bInventoryItem test - I think
	// that's more correct as otherwise inventory items like powerboots the
	// score etc. doesn't work
	if (m_nScoreDiff!=0) update_score(m_nScoreDiff, m_x, m_y);
	if (m_nHealthDiff!=0) update_health(m_nHealthDiff);
	djSoundPlay(g_iSounds[SOUND_PICKUP]);

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
	return THING_DIE;
}

void CPickup::Draw()
{
#ifdef EXPERIMENTAL_SPRITE_AUTO_DROPSHADOWS
	// Note the 15 height here, this is debatable, some things look better with 16
	// some better with 15, e.g. rugby ball looks better with 15
	DRAW_SPRITEA_SHADOW(pVisView, m_a, m_b + (m_nAnimationCount==-1?0:m_nAnimationCount), CALC_XOFFSET(m_x)+1, CALC_YOFFSET(m_y)+1,16,15);
#endif
	DRAW_SPRITE16A(pVisView, m_a, m_b + (m_nAnimationCount==-1?0:m_nAnimationCount), CALC_XOFFSET(m_x), CALC_YOFFSET(m_y));
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
	// I think we can't set this in the constructor as CPickup::Initialize will
	// later override it.
	//m_nScoreDiff = 1000;
}

void CBoots::Initialize(int b0, int b1)
{
	CPickup::Initialize(b0,b1);
	// NB, set this after base class Initialize or that will override this.
	m_nScoreDiff = 1000;
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
CCrumblingFloor::CCrumblingFloor() :
	m_nStrength(2),
	m_bHeroTouchingPrev(false),
	m_nWidth(1)
{
	m_bSolid = true;
	SetActionBounds(0, -5, 15, 15);
	SetSolidBounds(0, 0, 15, 15);
}
int CCrumblingFloor::Tick()
{
	bool bHeroTouching = OverlapsBounds(x*16+x_small*8, y*16+y_offset-16);
	//Detect 'edge' when just start touching.
	if (bHeroTouching && !m_bHeroTouchingPrev)
	{
		--m_nStrength;
	}
	m_bHeroTouchingPrev = bHeroTouching;

	if (m_nStrength<=0)
	{
		for ( int i=0; i<m_nWidth; ++i )
		{
			// Every 2nd one to look slightly less silly (fixmeLOW todo later make nicer / fancier explosion visuals) [dj2017-07]
			if ((i%2)==0)
			{
				AddThing(CreateExplosion((m_x+i)*16, m_y*16));
			}
		}
		return THING_DIE;
	}
	return CThing::Tick();
}
void CCrumblingFloor::Draw()
{
	for ( int i=0; i<m_nWidth; ++i )
	{
		DRAW_SPRITE16A(pVisView, m_a, m_b+(i%2), CALC_XOFFSET(m_x+i), CALC_YOFFSET(m_y));
	}
}
void CCrumblingFloor::Initialize(int b0, int b1)
{
	// Expand sideways right/left until we hit solid
	while (m_x+m_nWidth<LEVEL_WIDTH && !check_solid(m_x+m_nWidth,m_y,false))
	{
		++m_nWidth;
	}
	while (m_x>0 && !check_solid(m_x-1,m_y,false))
	{
		--m_x;
		++m_nWidth;
	}
	SetActionBounds(0, -5, m_nWidth*16-1, 15);
	SetSolidBounds(0, 0, m_nWidth*16-1, 15);
	SetVisibleBounds(0, 0, m_nWidth*16-1, 15);

	CThing::Initialize(b0,b1);
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
#ifdef EXPERIMENTAL_SPRITE_AUTO_DROPSHADOWS
	DRAW_SPRITEA_SHADOW(pVisView, m_a, m_b + anim4_count, 1+CALC_XOFFSET(m_x), 1+CALC_YOFFSET(m_y),16,16);
#endif
	DRAW_SPRITE16A(pVisView, m_a, m_b + anim4_count, CALC_XOFFSET(m_x), CALC_YOFFSET(m_y));
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
		if (m_nDir<0)
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
			DRAW_SPRITE16A(pVisView, m_a, m_b, CALC_XOFFSET(m_x), CALC_YOFFSET(m_y));
	}
	else
	{
		// Full flame
		if ((m_nCount % 4) < 2)
		{
			DRAW_SPRITE16A(pVisView, m_a, m_b + m_nDir*1, CALC_XOFFSET(m_x         ), CALC_YOFFSET(m_y));
			DRAW_SPRITE16A(pVisView, m_a, m_b + m_nDir*2, CALC_XOFFSET(m_x+m_nDir*1), CALC_YOFFSET(m_y));
			DRAW_SPRITE16A(pVisView, m_a, m_b + m_nDir*3, CALC_XOFFSET(m_x+m_nDir*2), CALC_YOFFSET(m_y));
		}
		else
		{
			DRAW_SPRITE16A(pVisView, m_a, m_b + m_nDir*2, CALC_XOFFSET(m_x         ), CALC_YOFFSET(m_y));
			DRAW_SPRITE16A(pVisView, m_a, m_b + m_nDir*1, CALC_XOFFSET(m_x+m_nDir*1), CALC_YOFFSET(m_y));
			DRAW_SPRITE16A(pVisView, m_a, m_b + m_nDir*3, CALC_XOFFSET(m_x+m_nDir*2), CALC_YOFFSET(m_y));
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
		DRAW_SPRITE16A(pVisView, m_a, m_b + (m_nCount % 2), CALC_XOFFSET(m_x), CALC_YOFFSET(m_y));
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
		DRAW_SPRITE16A(pVisView, m_a, m_b + m_nCenter + 2, CALC_XOFFSET(m_x), CALC_YOFFSET(m_y));
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
			DRAW_SPRITE16A(pVisView, m_a, m_b + nGraphicsOffset + 2 + (3-i), CALC_XOFFSET(m_x+nOffset), CALC_YOFFSET(m_y));
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
			DRAW_SPRITE16A(pVisView, m_a, m_b + nGraphicsOffset + 2 + i, CALC_XOFFSET(m_x-nOffset), CALC_YOFFSET(m_y));
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
			CALC_XOFFSET(m_x),
			CALC_YOFFSET(m_y),
			16,
			16);
	}
	if (m_nX2Right>0)
	{
		djgDrawRectangle(pVisView,
			CALC_XOFFSET(m_x)+m_nX1Right*16,
			CALC_YOFFSET(m_y),
			m_nX2Right*16,
			16);
	}
	if (m_nX2Left>0)
	{
		djgDrawRectangle(pVisView,
			CALC_XOFFSET(m_x) - 16 * (m_nX1Left + m_nX2Left - 1),
			CALC_YOFFSET(m_y),
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
	DRAW_SPRITE16A(pVisView, m_a, m_b + m_nAnim   , CALC_XOFFSET(m_x), CALC_YOFFSET(m_y));
	DRAW_SPRITE16A(pVisView, m_a, m_b + m_nAnim+16, CALC_XOFFSET(m_x), CALC_YOFFSET(m_y+1));
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
CRocket::CRocket() :
	m_nStrength(1),
	m_bTakingOff(false),
	m_fVelocity(0.f),
	m_fAccel(0.f),
	m_fDeltaAccel(0.02f),
	m_fHeight(0.f),
	m_nYStart(0),
	m_nFireAnim(0)
{
}

int CRocket::Tick()
{
	if (m_bTakingOff)
	{
		m_fAccel += m_fDeltaAccel;
		m_fVelocity += m_fAccel;
		if (m_fVelocity>=0.f)
		{
			m_fHeight += m_fVelocity;
			m_y = m_nYStart;
			m_yoffset = 0;
			m_yoffset = -(int)(m_fHeight);
			while (m_yoffset<-BLOCKH)
			{
				m_yoffset += BLOCKH;
				m_y--;
			}
			if (m_y<=0)
				return THING_DIE;
		}
	}
	return 0;
}

void CRocket::Draw()
{
	// Hm, probably one single big blit is more efficient, but would leave 'wasted' space in sprite
	DRAW_SPRITE16A(pVisView, 16, 1   , CALC_XOFFSET(m_x), CALC_YOFFSET(m_y-3) + m_yoffset);
	DRAW_SPRITE16A(pVisView, 16, 1+16, CALC_XOFFSET(m_x), CALC_YOFFSET(m_y-2) + m_yoffset);
	DRAW_SPRITE16A(pVisView, 16, 1+32, CALC_XOFFSET(m_x), CALC_YOFFSET(m_y-1) + m_yoffset);
	DRAW_SPRITEA(pVisView, 16, 1+48-1, CALC_XOFFSET(m_x-1), CALC_YOFFSET(m_y  ) + m_yoffset, 16*3,16);
	if (m_bTakingOff)
	{
		// Mod 6 so we do, size0, size1, size2, size3, size 2, size1, and loop
		switch (m_nFireAnim)
		{
		case 0:
			DRAW_SPRITE16A(pVisView, 16, 1+64  , CALC_XOFFSET(m_x), CALC_YOFFSET(m_y+1) + m_yoffset + (rand()%2));
			break;
		case 1:
		case 5:
			DRAW_SPRITE16A(pVisView, 16, 1+64+1, CALC_XOFFSET(m_x), CALC_YOFFSET(m_y+1) + m_yoffset + (rand()%2));
			break;
		case 2:
		case 4:
			DRAW_SPRITEA(pVisView, 16, 1+64+1+1, CALC_XOFFSET(m_x)-8, CALC_YOFFSET(m_y+1) + m_yoffset + (rand()%2), 32, 32);
			break;
		case 3:
			DRAW_SPRITEA(pVisView, 16, 1+64+1+1+2, CALC_XOFFSET(m_x)-8, CALC_YOFFSET(m_y+1) + m_yoffset + (rand()%2), 32, 32);
			break;
		}

		++m_nFireAnim;
		m_nFireAnim = (m_nFireAnim % 6);
	}
}

void CRocket::Initialize(int a, int b)
{
	CThing::Initialize(a, b);
	SetVisibleBounds(-16,-48,31,15);
	SetActionBounds (0,-32,15,15);
	SetShootBounds  (0,-32,15,15);
	SetSolidBounds  (0,-48,15,15);
	m_nStrength = 1;
	m_bSolid = true;
	m_bFalls = false;
	m_bShootable = true;
	SetLayer(LAYER_4);
	m_fVelocity=-1.f;//Hack: Start velocity negative, this is just a hack, to make it sit on the ground for a second or two before it starts
}

int CRocket::OnHeroShot()
{
	if (m_nStrength>0)
	{
		--m_nStrength;
		if (m_nStrength==0)
		{
			// START FLYING
			m_bTakingOff = true;
			m_nYStart = m_y;
			// Increase visiblebounds for fire below
			SetVisibleBounds(-16,-48,31,15 + 32);

			// In the original DN1, rockets standing on solid block which disappears on takeoff. Do same.
			// Not sure exactly the 'right' way, but let's say, I think, um, if *solid* block underneath
			// us in *foreground* layer, remove it. (What about background layer? Think about) [dj2017-08]
			if (m_y<LEVEL_HEIGHT-1)
			{
				// Clear block
				unsigned char *pBlock = level_pointer( 0, m_x, m_y+1 );
				*(pBlock + 0) = 0;
				*(pBlock + 1) = 0;
			}
			if (m_y<LEVEL_HEIGHT-2)
			{
				// Clear block
				unsigned char *pBlock = level_pointer( 0, m_x, m_y+2 );
				*(pBlock + 0) = 0;
				*(pBlock + 1) = 0;
			}
		}
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
	DRAW_SPRITE16A(pVisView, m_a, m_b, CALC_XOFFSET(m_x), CALC_YOFFSET(m_y));
}

int CFirepower::HeroOverlaps()
{
	HeroModifyFirepower(1);
	update_score(1000,m_x,m_y);
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
			DRAW_SPRITE16A(pVisView, 5, 20 + m_anAnim[i], CALC_XOFFSET(m_anX[i])+m_xoffset, CALC_YOFFSET(m_anY[i])+m_yoffset);
		}
	}
}
/*-----------------------------------------------------------*/
CWater::CWater() :
	m_nAnimationCount(0)
{
}
int CWater::Tick()
{
	++m_nAnimationCount;
	if (m_nAnimationCount>=4*4)
		m_nAnimationCount = 0;
	return CThing::Tick();
}
void CWater::Draw()
{
	DRAW_SPRITE16A(pVisView,
		m_a, m_b + (m_nAnimationCount/4),
		CALC_XOFFSET(m_x) + m_xoffset, CALC_YOFFSET(m_y));
}
void CWater::Initialize(int a, int b)
{
	CThing::Initialize(a,b);

	// There is 'background water' layer, and a 'foreground water' layer,
	// to create the sort of visual effect that he's "in" the water ...
	// so we assign the extra metadata in sprite editor and use that
	// to determine which layer we render in during level initalize.
	EdjLayer eLayer = (EdjLayer)(GET_SPRITE_EXTRA_METADATA(a,b,0)==0 ? LAYER_TOP : LAYER_BOTTOM);

	// Want to draw in front hero, so hero looks 'submerged' in water
	SetLayer(eLayer);
	SetVisibleBounds(0,0,BLOCKW-1,BLOCKH-1);
	SetActionBounds (0,0,BLOCKW-1,BLOCKH-1);
}
/*-----------------------------------------------------------*/
CBlock::CBlock()
{
}
void CBlock::Draw()
{
	DRAW_SPRITE16A(pVisView,
		m_a, m_b,// + anim4_count,
		CALC_XOFFSET(m_x) + m_xoffset, CALC_YOFFSET(m_y));
}
void CBlock::Initialize(int a, int b)
{
	CThing::Initialize(a,b);

	// Can maybe add some of these later, dunno [dj2017-08]
	m_bSolid = false;
	m_bFalls = false;
	m_bShootable = false;

	SetVisibleBounds(0,0,BLOCKW-1,BLOCKH-1);

	// Specify the layer in the first sprite metadata 'extra' value
	EdjLayer eLayer = (EdjLayer)(GET_SPRITE_EXTRA_METADATA(a,b,0));
	SetLayer(eLayer);
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

CThing *CreateExplosion(int nX, int nY, int nType, int nSoundIntensity)
{
	CExplosion *pExplosion = new CExplosion;
	pExplosion->SetType(nType);//added dj2017-08-12
	// Note we're passed in pixel coordinates, so use modulo operator to 'backcalculate' again the block x/y and pixel x/y offset..
	pExplosion->SetLocation( nX/BLOCKW, nY/BLOCKH, nX % BLOCKW, nY % BLOCKH, BLOCKW, BLOCKH );
	pExplosion->SetVisibleBounds( 0, 0, BLOCKW-1, BLOCKH-1 );
	if (nSoundIntensity<0)
	{
		// Don't play any sound
	}
	else if (nSoundIntensity==1)
		djSoundPlay( g_iSounds[SOUND_EXPLODE] );
	else
		djSoundPlay( g_iSounds[SOUND_SOFT_EXPLODE] );
	return pExplosion;
}

CThing *CreateDust(int nX, int nY, int nOffsetX, int nOffsetY)
{
	CDust *pDust = new CDust;
	//pDust->SetPosition(nX, nY);
	pDust->SetLocation(nX, nY, nOffsetX, nOffsetY, 1, 1);
	pDust->SetType(TYPE_DUST);
	return pDust;
}
