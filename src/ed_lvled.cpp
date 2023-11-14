// Level editor

#include "config.h"
#include "djfile.h"//djFile::dj_fopen
#include "ed_lvled.h"
#include "ed_common.h"
#include "ed_macros.h"
#include "djinput.h"
#include "graph.h"
#include "djstring.h"//djAppendPathS
#include "datadir.h"
#include "djtime.h"//djTimeGetTime() (for frame rate control in instructions screen loop)
#include "mission.h"		// g_pCurMission comes from here.
				// TODO: think of something to get
				// rid of that dependency on
				// g_pCurMission. Editors must
				// manage this by theirselves
#include "level.h"	// level_pointer() comes from here
#include "ed_DrawBoxContents.h"
//---------------------------------------------------------------------------
// Level stats stuff [dj2017-06]
#include <map>
#include <utility>//For std::pair
#include "block.h"//For names of block types for DN1
//---------------------------------------------------------------------------
extern int g_nLevel;//points to current level.//fixme dj2017-08 this is slightly gross but not as gross as having effectively two g_nLevel variables as it had been, caused problems
//---------------------------------------------------------------------------




// !!!!!!!!!!!
// If these change, change them in ed_DrawBocContents too!!!
#define POS_LEVELSPRITES_X 0
#define POS_LEVELSPRITES_Y 336
// !!!!!!!!!

#define POS_LEVELVIEW_X (390-16*7)
#define POS_LEVELVIEW_Y 2
#define LEVEL_VIEW_WIDTH 22
#define LEVEL_VIEW_HEIGHT 16



static int	levelview_x = 0;	// these two are for representing
static int	levelview_y = 0;	// a minimap.
static int	levelview_w = LEVEL_VIEW_WIDTH;	// these two are for representing
static int	levelview_h = LEVEL_VIEW_HEIGHT;	// a minimap.
static bool	bShowBack = true;	// toggles display of background blocks (the ones that could be trated as 'world', not the entities)
static bool	bLevelFore = true;//false;	// toggles drawing of foreground layer
static int	sprite0a = 0;
static int	sprite0b = 0;
static int	sprite1a = 0;
static int	sprite1b = 0;

// Unsaved changes flag [dj2017-06-27]
bool g_bDocumentDirty = false;


#define NUM_LEVEL_INSTRUCTIONS 17
static const char *level_instructions[NUM_LEVEL_INSTRUCTIONS] =
{
	"- Instructions: -----------",
	"1-9     Place macros 1-9",
	"Arrows  Move",
	"Alt+Clk MoveTo",
	"Ctl+Alt+Clk Start-Level-At",
	"F4      Sprite editor",
	"X       Toggle foreground",
	"Z       Toggle background",
	"F1      Save level",
	"F       Horizontal fill",
	"M       Next spriteset",
	"N       Previous spriteset",
	"Ctl+Alt+N New level",
	// These two now defunct:[dj2017-06]
	//"^C+M    Next level",
	//"^C+N    Previous level",
	"Ctl+F6  Level Statistics",
	"Ctl+F7  All Levels Overview",
	"ESC     Quit",
	"---------------------------"
};


bool HandleMouse();
//static void SetSprite ( int new_sprite );
static void MoveMinimap( int ox, int oy );
static void DrawMinimapRectangle();
static void RedrawView ();
static void DrawSprites ();
static void DrawGrid( int x, int y, int w, int h, int nx, int ny, const djColor& clr );
static void DrawLevelGrid();
static void DrawLevelname();
void ShowSettings();
static void DrawMinimap();
static void ShowInstructions();
static void DrawSpritesel ();
static void SetSpriteSelection( int a0, int b0, int a1, int b1 );
static void SelectLevel ( int i );
static void LevelFill( int ax, int ay );

// If calling this as an 'edit', should also call SetDocumentDirty() [dj2017-06]
void SetLevel( int x, int y, int a, int b, bool bforeground );


// <Unrefined>
//void level_fill( int ax, int ay );	// what does this do??
//void level_set( int x, int y, int a, int b, bool bforeground );		// // in ed.cpp
//void level_draw_spritesel();
//void level_set_spritesel( int a0, int b0, int a1, int b1 );
//void level_show_instructions();
// </Unrefined>

// New flickering stuff, more ugly globals dj2018-01
bool g_bFlashingIndicatorTimer=false;
bool g_bFlashingIndicatorEnabled=true;//User option to turn off flashing stuff
//a,b spriteset,spritenumber of the sprite the mouse is floating over, if any
unsigned char g_aMouseHighlighted = 0;
unsigned char g_bMouseHighlighted = 0;

#define LEVEL_GRIDSIZE (2)

#define DRAW_LEVEL_PIXEL(x, y) \
{\
	unsigned char a = *(level_pointer( 0, (x), (y) ) + 2);\
	unsigned char b = *(level_pointer( 0, (x), (y) ) + 3);\
	unsigned char fa = *(level_pointer( 0, (x), (y) ) + 0);\
	unsigned char fb = *(level_pointer( 0, (x), (y) ) + 1);\
	djgSetColorFore( pVisMain, ED_GetSpriteColor(  a, b ) );\
	djgDrawBox( pVisMain, (x)*LEVEL_GRIDSIZE, (y)*LEVEL_GRIDSIZE, LEVEL_GRIDSIZE, LEVEL_GRIDSIZE );\
	if ( g_bFlashingIndicatorTimer  &&  (sprite0a | sprite0b)!=0  &&\
	( ( a==sprite0a&&b==sprite0b) || (fa==sprite0a && fb==sprite0b) )\
	)\
	{\
		djgSetColorFore( pVisMain, djColor(255,255,0,150) );\
		djgDrawBox( pVisMain, (x)*LEVEL_GRIDSIZE, (y)*LEVEL_GRIDSIZE, 1, 1);\
	}\
	if ( g_bFlashingIndicatorTimer  &&  (g_aMouseHighlighted | g_bMouseHighlighted)!=0  &&\
	( (a==g_aMouseHighlighted&&b==g_bMouseHighlighted) || (fa==g_aMouseHighlighted&&fb==g_bMouseHighlighted) )\
	)\
	{\
		djgSetColorFore( pVisMain, djColor(0,255,255,180) );\
		djgDrawBox( pVisMain, 1+(x)*LEVEL_GRIDSIZE, (y)*LEVEL_GRIDSIZE, 1, 1);\
	}\
}
//[dj2018-01] Currently only used by the level overview stuff
#define DRAW_LEVEL_PIXEL2(nLevel, x, y,xOffset,yOffset) \
{\
	unsigned char a = *(level_pointer( (nLevel), (x), (y) ) + 2);\
	unsigned char b = *(level_pointer( (nLevel), (x), (y) ) + 3);\
	unsigned char fa = *(level_pointer( (nLevel), (x), (y) ) + 0);\
	unsigned char fb = *(level_pointer( (nLevel), (x), (y) ) + 1);\
	djgSetColorFore( pVisMain, ED_GetSpriteColor(  a, b ) );\
	djgDrawBox( pVisMain, (xOffset)+(x)*LEVEL_GRIDSIZE, (yOffset)+(y)*LEVEL_GRIDSIZE, LEVEL_GRIDSIZE, LEVEL_GRIDSIZE );\
	if ( g_bFlashingIndicatorTimer  &&  (sprite0a | sprite0b)!=0  &&\
	( ( a==sprite0a&&b==sprite0b) || (fa==sprite0a && fb==sprite0b) )\
	)\
	{\
		djgSetColorFore( pVisMain, djColor(255,255,0,150) );\
		djgDrawBox( pVisMain, (xOffset)+(x)*LEVEL_GRIDSIZE, (yOffset)+(y)*LEVEL_GRIDSIZE, 1, 1);\
	}\
	if ( g_bFlashingIndicatorTimer  &&  (g_aMouseHighlighted | g_bMouseHighlighted)!=0  &&\
	( (a==g_aMouseHighlighted&&b==g_bMouseHighlighted) || (fa==g_aMouseHighlighted&&fb==g_bMouseHighlighted) )\
	)\
	{\
		djgSetColorFore( pVisMain, djColor(0,255,255,180) );\
		djgDrawBox( pVisMain, (xOffset)+1+(x)*LEVEL_GRIDSIZE, (yOffset)+(y)*LEVEL_GRIDSIZE, 1, 1);\
	}\
}


//---------------------------------------------------------------------------
std::string GetFriendlyTypeName( int a, int b )
{
	std::string sType;
	int iType = ED_GetSpriteType( a, b );
	if (iType>0 && iType<TYPE_LASTONE)
	{
		sType = block_type_names[iType];

		// If it's a 'box', handle this as a special case to also show name of what's inside the box
		if (iType==TYPE_BOX)
		{
			int c = ED_GetSpriteExtra(a, b, 10);
			int d = ED_GetSpriteExtra(a, b, 11);
			int iTypeContents = ED_GetSpriteType( c,d );
			sType += " [";
			if (iTypeContents>=0 && iTypeContents<(int)TYPE_LASTONE)
				sType += block_type_names[iTypeContents];
			else
				sType += "(error: invalid box contents - check in spreditor!)";
			sType += "]";
		}
	}
	else if (iType>=TYPE_LASTONE)//<- dj2017-06 hm, that should probably be ">" as TYPE_LASTONE maybe reserved for possible use? Not sure anyway whatever
		sType += "(error: invalid type - check in spreditor!)";
	return sType;
}
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Level stats
class CLevelStats
{
public:
	CLevelStats();
	// The pair is the usual 'a,b' to denote sprite info (spritesetindex,spriteindex)
	std::map< std::pair< unsigned char, unsigned char >, int > SpriteCounts;	// Total count for (a,b)
	std::map< std::pair< unsigned char, unsigned char >, int > SpriteCountsF;	// Foreground instance count for (a,b)
	std::map< std::pair< unsigned char, unsigned char >, int > SpriteCountsB;	// Background instance count for (a,b)
	// Hm, shoudltry detect Invalid sprites

	//int m_nMaxCount;
};
CLevelStats::CLevelStats() //:
	//m_nMaxCount(0)
{
}
void CalculateLevelStats(CLevelStats& Stats, int iLevel=0)
{
	// Fill in with 0 by default?
	for ( int i=0; i<NUM_SPRITE_DATA; ++i)
	{
		const CSpriteData* pSpriteData = g_pCurMission->m_apSpriteData[i];
		// We MUST check for NULL as this silly way of doing it means there will be NULL items [dj2017-06]
		if (pSpriteData==NULL)continue;

		for ( int b=0; b<=127; ++b )
		{
			Stats.SpriteCounts [ std::make_pair((unsigned char)i,(unsigned char)b) ] = 0;
		}
	}

	//fixme ----- look for and show 'invalid' blocks???
	// This "shouldn't happen" but can if someone e.g. added a spriteset, edit level, then delete spriteset.
	// Or maybe, copy and paste a level from one 'mission' to another, but with fewer spritesets defined
	// in that mission.

	SLevelBlock Block;
	for ( int y=0; y<LEVEL_HEIGHT; ++y )
	{
		for ( int x=0; x<LEVEL_WIDTH; ++x )
		{
			Block = level_get_block( iLevel, x, y );//first param used to mean something i think but currently always 0 now, should maybe get rid of ultimately [dj2017-06]

			Stats.SpriteCounts [ std::make_pair(Block.aback,Block.bback) ]++;
			Stats.SpriteCounts [ std::make_pair(Block.afore,Block.bfore) ]++;
			Stats.SpriteCountsB[ std::make_pair(Block.aback,Block.bback) ]++;
			Stats.SpriteCountsF[ std::make_pair(Block.afore,Block.bfore) ]++;
		}
	}

	// Calculate largest counts (use this to show chart-like view) //?
	/*
	int nCountMax = 0;
	for ( std::map< std::pair< unsigned char, unsigned char >, int >::const_iterator iter=Stats.SpriteCounts.begin(); iter!=Stats.SpriteCounts.end(); ++iter )
	{
		// Don't count background / empty blocks toward the 'max'
		if (iter->first.first==0 && iter->first.second==0)
			continue;
		if (iter->second > nCountMax)
			nCountMax = iter->second;
	}
	Stats.m_nMaxCount = nCountMax;
	*/
}
void DisplayLevelStats( CLevelStats& Stats)
{
	bool	bRunning = true;
	int nPage = 0;
	int nRowsPerPage = (pVisMain->height / 16) - 2;
	//this+1is wrong, use ceil(),isok fornow,lowprio
	int nPages = (int)(Stats.SpriteCounts.size() / nRowsPerPage) + 1;
	while ( bRunning )
	{
		unsigned long delay = 80;
		SDL_Delay( delay );

		if (g_iKeys[DJKEY_HOME])
			nPage = 0;
		if (g_iKeys[DJKEY_END])
			nPage = nPages-1;
		if (g_iKeys[DJKEY_PGDN])
		{
			nPage++;
			if (nPage>=nPages) nPage = nPages;
		}
		if (g_iKeys[DJKEY_PGUP])
		{
			nPage--;
			if (nPage<0)nPage=0;
		}

		//RedrawView ();
		{
			// Total status
			ED_ClearScreen();
			char buf[2048]={0};
			//int nPages = //CEILINGStats.SpriteCounts.size() % (pVisMain->height / 16);
			int y=0,n=0,xleft=8;
			for ( std::map< std::pair< unsigned char, unsigned char >, int >::const_iterator iter=Stats.SpriteCounts.begin(); iter!=Stats.SpriteCounts.end(); ++iter )
			{
				++n;
				if (n<=nPage*nRowsPerPage)continue;
				if (y>=nRowsPerPage)
					continue;
				//_snprintf(


				djgSetColorFore( pVisMain, djColor(60,60,60) );
				djgDrawHLine( pVisMain, 0, y*16, pVisMain->width );

				ED_DrawSprite( xleft,
				y*16,
					iter->first.first, iter->first.second );

				//a,b pair
				const std::pair< unsigned char, unsigned char >& pair = iter->first;

				snprintf(buf,sizeof(buf), "%3d,%3d", (unsigned int)pair.first, (unsigned int)pair.second );
				ED_DrawString( xleft+16+8, y*16+4, buf );

				// Count
				if (iter->second!=0)
				{
					snprintf(buf,sizeof(buf), "%8d", (int)iter->second );
					ED_DrawString( xleft+16+8+10*8, y*16+4, buf );
				}

				// Count (background instances)
				if (Stats.SpriteCountsB[ pair ]!=0)
				{
					snprintf(buf,sizeof(buf), "%8d", Stats.SpriteCountsB[ pair ] );
					ED_DrawString( xleft+16+8 + 136, y*16+4, buf );
				}
				// Count (foreground instances)
				if (Stats.SpriteCountsF[ pair ]!=0)
				{
					snprintf(buf,sizeof(buf), "%8d", Stats.SpriteCountsF[ pair ] );
					ED_DrawString( xleft+16+8 + 136+9*8, y*16+4, buf );
				}

				// Sprite type 'friendly name' (from blocks.h)?
				std::string sType;
				sType = GetFriendlyTypeName( pair.first, pair.second );

				djgSetColorFore( pVisMain, djColor(60,60,60) );
				djgDrawVLine( pVisMain, xleft+16+8 + 136+9*8+9*8, y*16, 16 );
				if (!sType.empty())
				{
					ED_DrawString( xleft+16+8 + 136+9*8+9*8 + 1, y*16+4, sType.c_str() );
				}

				djgDrawVLine( pVisMain, pVisMain->width/2, y*16, 16 );
				if (iter->second>0)
				{
					// This 100 is a bit arbitrary
					int nCountValueCapped = djMIN(100,iter->second);
					float lfW = ((float)pVisMain->width/2.f);
					float lfWThis = (float)nCountValueCapped / 100.f;
					djgDrawBox( pVisMain, pVisMain->width/2, y*16, (int)(lfWThis * lfW), 16 );
				}

				++y;
			}

			snprintf(buf,sizeof(buf),"Level Statistics. Page %d/%d", nPage+1, nPages);
			ED_DrawString( 0, pVisMain->height-24, buf );

			const char * szfilename = g_pCurMission->GetLevel(g_nLevel)->GetFilename();
			snprintf(buf,sizeof(buf),"Level:%s",szfilename==NULL?"":szfilename);
			ED_DrawString( 0, pVisMain->height-16, buf );

			ED_DrawString( 0, pVisMain->height-8, "Help: Esc exit stats. PgUp/PgDn to scroll. Home/End 1st/last page. Counts are total,background,foreground" );

			ED_FlipBuffers ();
		}
		djiPoll();

		if (g_iKeys[DJKEY_ESC])
			bRunning = false;

		// dj2017-07 [fixmelow] this isn't quite right, it's the normal handlemouse so if you click on now invisible spriteset, it 'handles' it etc. even though we're in stats view, urgh
		if (!HandleMouse ())
			bRunning = false;

	} // while (bRunning)

	ED_ClearScreen();
	RedrawView();
	ED_FlipBuffers();//dj2017-07 Moving this out of RedrawView as we want it after handling keys etc. in level mainloop
}
//---------------------------------------------------------------------------
void DoAllLevelsOverview()
{
	const int PIXELSIZE = 2;


	// Analyze the sprite metadata to get the a,b ID pairs of certain
	// 'important' sprite types releveant to gameplay, e.g. powerboots etc.
	std::map< int, std::pair<int,int> > mapBoxTypes;
	std::map< int, std::pair<int,int> > mapTypes;
	{
		CSpriteData* pData=NULL;
		//iterates through the 256 possible ID's for spritesets
		for (int a=0;a<256;++a )
		{
			pData = g_pCurMission->GetSpriteData(a);
			if (pData)
			{
				for ( int b=0;b<SPRITES_PER_SPRITESHEET;++b )
				{
					int nType = ED_GetSpriteType( a, b );
					if (ED_GetSpriteType( a, b )==TYPE_BOX)
					{
						// Box contents are an 'extra'
						int c = ED_GetSpriteExtra(a, b, 10);
						int d = ED_GetSpriteExtra(a, b, 11);
						nType = ED_GetSpriteType( c, d );
						if (nType==TYPE_FIREPOWER) mapBoxTypes[TYPE_FIREPOWER] = std::make_pair( a,b );
						if (nType==TYPE_ACCESSCARD) mapBoxTypes[TYPE_ACCESSCARD] = std::make_pair( a,b );
						if (nType==TYPE_POWERBOOTS) mapBoxTypes[TYPE_POWERBOOTS] = std::make_pair( a,b );
						
					}
					
					if (nType==TYPE_FIREPOWER) mapTypes[TYPE_FIREPOWER] = std::make_pair( a,b );
					if (nType==TYPE_ACCESSCARD) mapTypes[TYPE_ACCESSCARD] = std::make_pair( a,b );
					if (nType==TYPE_POWERBOOTS) mapTypes[TYPE_POWERBOOTS] = std::make_pair( a,b );
					if (nType==TYPE_MAINCOMPUTER) mapTypes[TYPE_MAINCOMPUTER] = std::make_pair( a,b );
				}
			}
		}
	}




	// Load all levels into memory
	std::vector< unsigned char* > apLevels;
	std::vector< CLevelStats > aLevelStats;
	for ( int i=0; i<g_pCurMission->NumLevels(); ++i )
	{
		unsigned char* szLevel = level_load( i, g_pCurMission->GetLevel(i)->GetFilename() );
		apLevels.push_back( szLevel );

		CLevelStats Stats;
		CalculateLevelStats(Stats, i);
		aLevelStats.push_back( Stats );
	}


	bool bRunning = true;
	int nPage = 0;
	int nRowsPerPage = (pVisMain->height / ((LEVEL_HEIGHT+1)*PIXELSIZE));
	//this+1is wrong, use ceil(),isok fornow,lowprio
	int nPages = (int)(g_pCurMission->NumLevels() / nRowsPerPage) + 1;
	while ( bRunning )
	{
		unsigned long delay = 80;
		SDL_Delay( delay );

		g_bFlashingIndicatorTimer = g_bFlashingIndicatorEnabled && (((int)(djTimeGetTime()*2.0f) % 2)==0);

		if (g_iKeys[DJKEY_HOME])
			nPage = 0;
		if (g_iKeys[DJKEY_END])
			nPage = nPages-1;
		if (g_iKeys[DJKEY_PGDN])
		{
			nPage++;
			if (nPage>=nPages) nPage = nPages;
		}
		if (g_iKeys[DJKEY_PGUP])
		{
			nPage--;
			if (nPage<0)nPage=0;
		}

		{
			// Total status
			ED_ClearScreen();

			char buf[4096]={0};
			for ( int i=0;i<nRowsPerPage;++i )
			{
				int iLev = i + nPage*nRowsPerPage;
				if (iLev<0 || iLev>=g_pCurMission->NumLevels())
					continue;
				// The +1 is to put a blank row between each level
				int nLevelDispY = i*(LEVEL_HEIGHT+1)*PIXELSIZE;

				// Draw 'mini view' of the level
				//unsigned char* pLevel = apLevels[iLev];
				for ( int y=0; y<LEVEL_HEIGHT; ++y )
				{
					for ( int x=0; x<LEVEL_WIDTH; ++x )
					{
						DRAW_LEVEL_PIXEL2(iLev,x,y,0,nLevelDispY);
					}
				}

				// Show stats on 'important' pickups/objects etc., e.g. powerboots, firepower
				const int nTEXTHEIGHT = 8;
				int nTextY = 0;
				snprintf(buf,sizeof(buf),"LEVEL[%d/%d] %s", iLev+1,g_pCurMission->NumLevels(), g_pCurMission->GetLevel(iLev)->GetFilename() );
				ED_DrawString( PIXELSIZE*LEVEL_WIDTH + 8, nLevelDispY + (nTextY++)*nTEXTHEIGHT, buf );

				snprintf(buf,sizeof(buf),"Keys    : [1]%03d [2]%03d [3]%03d [4]%03d",
					aLevelStats[iLev].SpriteCounts[ std::make_pair(0,117) ],
					aLevelStats[iLev].SpriteCounts[ std::make_pair(0,118) ],
					aLevelStats[iLev].SpriteCounts[ std::make_pair(0,119) ],
					aLevelStats[iLev].SpriteCounts[ std::make_pair(0,120) ]
				);
				ED_DrawString( PIXELSIZE*LEVEL_WIDTH + 8, nLevelDispY + (nTextY++)*nTEXTHEIGHT, buf );

				snprintf(buf,sizeof(buf),"KeyLocks: [1]%03d [2]%03d [3]%03d [4]%03d",
					aLevelStats[iLev].SpriteCounts[ std::make_pair(0,121) ],
					aLevelStats[iLev].SpriteCounts[ std::make_pair(0,122) ],
					aLevelStats[iLev].SpriteCounts[ std::make_pair(0,123) ],
					aLevelStats[iLev].SpriteCounts[ std::make_pair(0,124) ]
				);
				ED_DrawString( PIXELSIZE*LEVEL_WIDTH + 8, nLevelDispY + (nTextY++)*nTEXTHEIGHT, buf );

				snprintf(buf,sizeof(buf),"Doors   : [1]%03d [2]%03d [3]%03d [4]%03d",
					aLevelStats[iLev].SpriteCounts[ std::make_pair(1, 0) ],
					aLevelStats[iLev].SpriteCounts[ std::make_pair(1,16) ],
					aLevelStats[iLev].SpriteCounts[ std::make_pair(1,32) ],
					aLevelStats[iLev].SpriteCounts[ std::make_pair(1,48) ]
				);
				ED_DrawString( PIXELSIZE*LEVEL_WIDTH + 8, nLevelDispY + (nTextY++)*nTEXTHEIGHT, buf );

				snprintf(buf,sizeof(buf),"Box[GNUKEM]: %-3d %-3d %-3d %-3d %-3d %-3d",
					aLevelStats[iLev].SpriteCounts[ std::make_pair(1, 117) ],
					aLevelStats[iLev].SpriteCounts[ std::make_pair(1, 118) ],
					aLevelStats[iLev].SpriteCounts[ std::make_pair(1, 119) ],
					aLevelStats[iLev].SpriteCounts[ std::make_pair(1, 120) ],
					aLevelStats[iLev].SpriteCounts[ std::make_pair(1, 121) ],
					aLevelStats[iLev].SpriteCounts[ std::make_pair(1, 122) ]
				);
				ED_DrawString( PIXELSIZE*LEVEL_WIDTH + 8, nLevelDispY + (nTextY++)*nTEXTHEIGHT, buf );
				snprintf(buf,sizeof(buf),"    GNUKEM : %-3d %-3d %-3d %-3d %-3d %-3d",
					aLevelStats[iLev].SpriteCounts[ std::make_pair(1, 58) ],
					aLevelStats[iLev].SpriteCounts[ std::make_pair(1, 59) ],
					aLevelStats[iLev].SpriteCounts[ std::make_pair(1, 60) ],
					aLevelStats[iLev].SpriteCounts[ std::make_pair(1, 61) ],
					aLevelStats[iLev].SpriteCounts[ std::make_pair(1, 62) ],
					aLevelStats[iLev].SpriteCounts[ std::make_pair(1, 63) ]
				);
				ED_DrawString( PIXELSIZE*LEVEL_WIDTH + 8, nLevelDispY + (nTextY++)*nTEXTHEIGHT, buf );

				int n1=0;
				int n2=0;
				int nType = TYPE_ACCESSCARD;
				n1 = 0; if (mapTypes.find(nType)!=mapTypes.end())		n1 = aLevelStats[iLev].SpriteCounts[ mapTypes[nType] ];
				n2 = 0; if (mapBoxTypes.find(nType)!=mapBoxTypes.end())	n2 = aLevelStats[iLev].SpriteCounts[ mapBoxTypes[nType] ];
				snprintf(buf,sizeof(buf),"[%s]=%d (Boxed=%d)", GetBlockTypeName((EBlockType)nType), n1, n2 );
				ED_DrawString( PIXELSIZE*LEVEL_WIDTH + 8, nLevelDispY + (nTextY++)*nTEXTHEIGHT, buf );

				nType = TYPE_FIREPOWER;
				n1 = 0; if (mapTypes.find(nType)!=mapTypes.end())		n1 = aLevelStats[iLev].SpriteCounts[ mapTypes[nType] ];
				n2 = 0; if (mapBoxTypes.find(nType)!=mapBoxTypes.end())	n2 = aLevelStats[iLev].SpriteCounts[ mapBoxTypes[nType] ];
				snprintf(buf,sizeof(buf),"[%s]=%d (Boxed=%d)", GetBlockTypeName((EBlockType)nType), n1, n2 );
				ED_DrawString( PIXELSIZE*LEVEL_WIDTH + 8, nLevelDispY + (nTextY++)*nTEXTHEIGHT, buf );

				nType = TYPE_MAINCOMPUTER;
				n1 = 0; if (mapTypes.find(nType)!=mapTypes.end())		n1 = aLevelStats[iLev].SpriteCounts[ mapTypes[nType] ];
				n2 = 0; if (mapBoxTypes.find(nType)!=mapBoxTypes.end())	n2 = aLevelStats[iLev].SpriteCounts[ mapBoxTypes[nType] ];
				snprintf(buf,sizeof(buf),"[%s]=%d (Boxed=%d)", GetBlockTypeName((EBlockType)nType), n1, n2 );
				ED_DrawString( PIXELSIZE*LEVEL_WIDTH + 8, nLevelDispY + (nTextY++)*nTEXTHEIGHT, buf );

				nType = TYPE_POWERBOOTS;
				n1 = 0; if (mapTypes.find(nType)!=mapTypes.end())		n1 = aLevelStats[iLev].SpriteCounts[ mapTypes[nType] ];
				n2 = 0; if (mapBoxTypes.find(nType)!=mapBoxTypes.end())	n2 = aLevelStats[iLev].SpriteCounts[ mapBoxTypes[nType] ];
				snprintf(buf,sizeof(buf),"[%s]=%d (Boxed=%d)", GetBlockTypeName((EBlockType)nType), n1, n2 );
				ED_DrawString( PIXELSIZE*LEVEL_WIDTH + 8, nLevelDispY + (nTextY++)*nTEXTHEIGHT, buf );

				snprintf(buf,sizeof(buf),"HIGHVOLTAGE: %-3d", aLevelStats[iLev].SpriteCounts[ std::make_pair(5, 31) ] );
				ED_DrawString( PIXELSIZE*LEVEL_WIDTH + 8, nLevelDispY + (nTextY++)*nTEXTHEIGHT, buf );
			}


			snprintf(buf,sizeof(buf),"Level overview. Page %d/%d", nPage+1, nPages);
			ED_DrawString( 0, pVisMain->height-24, buf );

			ED_DrawString( 0, pVisMain->height-8, "Help: Esc exit stats. PgUp/PgDn to scroll. Home/End 1st/last page." );

			ED_FlipBuffers ();
		}
		djiPoll();

		if (g_iKeys[DJKEY_ESC])
			bRunning = false;

		// [fixmelow] this isn't quite right, it's the normal handlemouse so if you click on now invisible spriteset, it 'handles' it etc. even though we're in stats view, urgh
		if (!HandleMouse ())
			bRunning = false;

	} // while (bRunning)


	// CLEAN UP
	for ( int i=0; i<g_pCurMission->NumLevels(); ++i )
	{
		level_delete( i );
	}

	// RE-LOAD THE CURRENT LEVEL THAT WAS LOADED AGAIN BEFORE WE WENT INTO OVERVIEW VIEW
	const char * szfilename = g_pCurMission->GetLevel( g_nLevel )->GetFilename( );
	level_load( 0, szfilename );


	ED_ClearScreen();
	RedrawView();
	ED_FlipBuffers();//Moving this out of RedrawView as we want it after handling keys etc. in level mainloop
}
//---------------------------------------------------------------------------
void SetDocumentDirty(bool bDirty)
{
	if (g_bDocumentDirty!=bDirty)
	{
		g_bDocumentDirty=bDirty;

		// Redraw the status indicator (for now, we're using the levelname area, so redraw levelname)
		DrawLevelname();
	}
}
//---------------------------------------------------------------------------

void LVLED_Init(int curr_level)
{
	g_bDocumentDirty=false;

	SelectLevel(curr_level);

	//load the level from disk to get a clean copy.
	const char * szfilename = g_pCurMission->GetLevel( g_nLevel )->GetFilename( );
	level_load( 0, szfilename );

	// [dj2016-10] Calculate how much screen area we can devote to minimap, so we can take advantage of
	// modern larger screens for a much friendlier level editing experience.
	levelview_w = (pVisMain->width - POS_LEVELVIEW_X) / 16;
	levelview_h = (pVisMain->height - (NUM_LEVEL_INSTRUCTIONS*8) - POS_LEVELVIEW_Y) / 16;
	// [dj2016-10] If we have a really high-res screen, theoretically this box could be larger than the whole level -
	// if that happens to happen, clip to the max level W/H (don't want this box being larger than the map):
	levelview_w = djMIN(levelview_w, LEVEL_WIDTH);
	levelview_h = djMIN(levelview_h, LEVEL_HEIGHT);
}



void LVLED_Kill()
{
	g_bDocumentDirty=false;
}

void DoCreateNewLevel()
{
	//fixmeLOW if unsaved changes, doens't ask or anythign. Should warn and ask are you sure before just wiping changes and creating new level.
	g_bDocumentDirty=false;

	//Generate a unique filename (using counter - if file exists, increment counter and try again)
	//fixLOW handle unicode in paths? Future? [dj2017-08]

	// hmm fixme if we're using e.g. "/usr/..." as datadir we'll have issues here?

	std::string sPath = djDataDirS();
	djAppendPathS(sPath, "levels");
	std::string sFilenameWithPath;
	std::string sFilenameWithoutPath;
	int n = 0;
	do
	{
		++n;
		char szBuf[8192]={0};//fixLOW MAX_PATH? Some issue with MAX_PATH I can't remember what right now [dj2017-08]
		snprintf(szBuf,sizeof(szBuf),"newlevel%03d.lev", n);
		sFilenameWithoutPath = szBuf;

		sFilenameWithPath = sPath;
		djAppendPathS(sFilenameWithPath, szBuf);
	} while (djFileExists(sFilenameWithPath.c_str()));

	// See config.h for explanation of the size here. Basically create a blank empty map datablock temporarily and save to the new file.
	unsigned char* szNewLevel = new unsigned char [ LEVEL_SIZE ];
	memset(szNewLevel, 0, LEVEL_SIZE);

	CLevel* pLevel = new CLevel;
	std::string sLevelRelativePath="levels";
	djAppendPathS(sLevelRelativePath, sFilenameWithoutPath.c_str());
	pLevel->SetFilename(sLevelRelativePath.c_str());
	pLevel->m_szName		= djStrDeepCopy("");
	pLevel->m_szBackground	= djStrDeepCopy("levels/bg1.tga");//fixme for now hardcoded
	pLevel->m_szAuthor		= djStrDeepCopy("");
	g_pCurMission->AddLevel(pLevel);

	//Mode wb = Write file in binary mode, MUST BE BINARY MODE, very important
	FILE* pOut = djFile::dj_fopen(sFilenameWithPath.c_str(), "wb");
	if (pOut)
	{
		fwrite(szNewLevel, LEVEL_SIZE, 1, pOut);
		fclose(pOut);
		pOut = NULL;
	}
	delete[] szNewLevel;

	// Select the new level we just added ...
	SelectLevel ( g_pCurMission->NumLevels() - 1 );

	// .. now 'actually' load it.
	level_load(0, sLevelRelativePath.c_str());

	// Refresh view.
	RedrawView();
}


switch_e LVLED_MainLoop ()
{
	//fixme hogs cpu ... but looking at below, maybe not trivial to fix without affecting usability? [dj2016-10]
	//int	i;
	bool	bRunning = true;
	while ( bRunning )
	{
		unsigned long delay = 10;

		//dj2017-08 Adding this framerate stuff to prevent the 100% CPU/core hogging,
		// but editing now feels slightly less snappy to me, might want to come back
		// to this again.
		// Aim for desired frame rate
		const float fTIMEFRAME = (1.0f / 300.0f);
		// Start out by being at next time
		float fTimeNext = djTimeGetTime();
		float fTimeNow = fTimeNext;
		
		{
			fTimeNow = djTimeGetTime();
			fTimeNext = fTimeNow + fTIMEFRAME;
			// Sleep a little to not hog CPU to cap menu update (frame rate) at approx 10Hz
			while (fTimeNow<fTimeNext)
			{
				SDL_Delay(1);
				fTimeNow = djTimeGetTime();
			}
		}

		RedrawView ();
		djiPoll();

		if (g_iKeys[DJKEY_ESC])
			bRunning = false;

		if (!HandleMouse ())
			bRunning = false;
		static bool bflagleft = false;
		static bool bflagright = false;
		static bool bflagup = false;
		static bool bflagdown = false;
		if (g_iKeys[DJKEY_LEFT])
		{
			bflagleft = true;
			MoveMinimap( levelview_x - 1, levelview_y );
			SDL_Delay( delay );
		}
		if ( (g_iKeys[DJKEY_LEFT] == 0) && (bflagleft == true) )
		{
			bflagleft = false;
			MoveMinimap( levelview_x - 1, levelview_y );
		}

		if (g_iKeys[DJKEY_RIGHT])
		{
			bflagright = true;
			MoveMinimap( levelview_x + 1, levelview_y );
			SDL_Delay( delay );
		}
		if ( (g_iKeys[DJKEY_RIGHT] == 0) && (bflagright == true) )
		{
			bflagright = false;
			MoveMinimap( levelview_x + 1, levelview_y );
		}

		if (g_iKeys[DJKEY_UP])
		{
			bflagup = true;
			MoveMinimap( levelview_x, levelview_y - 1 );
			SDL_Delay( delay );
		}
		if ( (g_iKeys[DJKEY_UP] == 0) && (bflagup == true) )
		{
			bflagup = false;
			MoveMinimap( levelview_x, levelview_y - 1 );
		}

		if (g_iKeys[DJKEY_DOWN])
		{
			bflagdown = true;
			MoveMinimap( levelview_x, levelview_y + 1 );
			SDL_Delay( delay );
		}
		if ( (g_iKeys[DJKEY_DOWN] == 0) && (bflagdown == true) )
		{
			bflagdown = false;
			MoveMinimap( levelview_x, levelview_y + 1);
		}



		if ( (djMouse::x >= 0) && (djMouse::y >= 0) )
		{
// TODO: no level filling at this point. Put it back here whenever possible.
			if (g_iKeys[DJKEY_F])
			{
				// if mouse in main level view
				if (INBOUNDS(djMouse::x, djMouse::y, 0, 0, LEVEL_WIDTH*LEVEL_GRIDSIZE - 1, LEVEL_HEIGHT*LEVEL_GRIDSIZE - 1))
				{
					LevelFill(djMouse::x / LEVEL_GRIDSIZE, djMouse::y / LEVEL_GRIDSIZE );
				}
				// if mouse in zoom level view
				else if (
						INBOUNDS(djMouse::x, djMouse::y,
					POS_LEVELVIEW_X, POS_LEVELVIEW_Y,
					POS_LEVELVIEW_X + levelview_w * 16 - 1,
					POS_LEVELVIEW_Y + levelview_h * 16 - 1 ))
				{
					LevelFill(
						levelview_x + (djMouse::x-POS_LEVELVIEW_X) / 16,
						levelview_y + (djMouse::y-POS_LEVELVIEW_Y) / 16 );
				}
			}
/**/
		}

		// If the mouse is inside the main level view, do some extra checking
		if (INBOUNDS(djMouse::x, djMouse::y,
			POS_LEVELVIEW_X, POS_LEVELVIEW_Y,
			POS_LEVELVIEW_X + levelview_w * 16 - 1,
			POS_LEVELVIEW_Y + levelview_h * 16 - 1 ))
		{
			int iLevelX = levelview_x + (djMouse::x-POS_LEVELVIEW_X) / 16;
			int iLevelY = levelview_y + (djMouse::y-POS_LEVELVIEW_Y) / 16;
			if (g_iKeys[DJKEY_1]) PlaceMacro( iLevelX, iLevelY, 0 );
			if (g_iKeys[DJKEY_2]) PlaceMacro( iLevelX, iLevelY, 1 );
			if (g_iKeys[DJKEY_3]) PlaceMacro( iLevelX, iLevelY, 2 );
			if (g_iKeys[DJKEY_4]) PlaceMacro( iLevelX, iLevelY, 3 );
			if (g_iKeys[DJKEY_5]) PlaceMacro( iLevelX, iLevelY, 4 );
			if (g_iKeys[DJKEY_6]) PlaceMacro( iLevelX, iLevelY, 5 );
			if (g_iKeys[DJKEY_7]) PlaceMacro( iLevelX, iLevelY, 6 );
			if (g_iKeys[DJKEY_8]) PlaceMacro( iLevelX, iLevelY, 7 );
			if (g_iKeys[DJKEY_9]) PlaceMacro( iLevelX, iLevelY, 8 );
		} // if (mouse inside main view)

		if (djiKeyPressed(DJKEY_F1))
		{
// TODO: no saving at this point. Reenable it as soon as possible.
			//for ( i=0; i<g_pCurMission->NumLevels(); i++ )
			//{
				const char * szfilename;
				szfilename = g_pCurMission->GetLevel(g_nLevel)->GetFilename();
				level_save( 0, szfilename );
			//}
			// Clear document dirty state on save
			SetDocumentDirty(false);
		}
		if (g_iKeys[DJKEY_F4])		// switch off the LVLED
		{
			return SWITCH_SPRED;
		}
		if (g_iKeys[DJKEY_F3])
		{
			g_bFlashingIndicatorEnabled = !g_bFlashingIndicatorEnabled;
			SDL_Delay(200);//HORRIBLE Ugly hack because i'm too lazy to detect keypress edges properly[dj2018-01] [to prevent toggling every frame rapidly effectively - give a bit of time to key-up]
		}
		if (djiKeyPressed(DJKEY_X))
		{
			bLevelFore = !bLevelFore;
			if (bLevelFore)
			{
				ED_DrawString( 48, 308, "[FOREGROUND]" );
			}
			else
			{
				ED_DrawStringClear( 48, 308, "[FOREGROUND]" );
			}
			DrawMinimap ();
		}
		//-- Z - Toggle display of background blocks
		if (djiKeyPressed(DJKEY_Z))
		{
			bShowBack = !bShowBack;
			DrawMinimap ();
		}
		if (g_iKeys[DJKEY_CTRL] && g_iKeys[DJKEY_ALT])
		{
			if (djiKeyPressed(DJKEY_N))
			{
				DoCreateNewLevel();
			}
		}
		else if (g_iKeys[DJKEY_CTRL])
		{
			//dj2017-06 Add basic level stats
			if (djiKeyPressed(DJKEY_F6))
			{
				CLevelStats Stats;
				CalculateLevelStats(Stats);
				DisplayLevelStats(Stats);
	//crudeworkaround to prevent double-escape-press handling since we do crappy keyhandling all over the palce in here .. really we should be detecting keyup/down events etc. .. but it's "just" the leveleditor so not a prio [dj2017-06]
	SDL_Delay(200);
			}
			else if (djiKeyPressed(DJKEY_F7))
			{
				DoAllLevelsOverview();
	//crudeworkaround to prevent double-escape-press handling since we do crappy keyhandling all over the palce in here .. really we should be detecting keyup/down events etc. .. but it's "just" the leveleditor so not a prio [dj2018-01]
	SDL_Delay(200);
			}

// TODO: no level selection at this point
/*			if (djiKeyPressed(DJKEY_N))
			{
				SelectLevel( g_nLevel - 1 );
			}
			if (djiKeyPressed(DJKEY_M))
			{
				SelectLevel( g_nLevel + 1 );
			}
*/
		}
		else
		{
			if (djiKeyPressed(DJKEY_N))
			{
				ED_SetCurrSpriteSet( ED_GetCurrSpriteSet() - 1 );
				if (ED_GetCurrSpriteSet() == -1)
					ED_SetCurrSpriteSet(255);
				while (g_pCurMission->GetSpriteData(ED_GetCurrSpriteSet()) == NULL)
				{
					if (ED_GetCurrSpriteSet() == -1)
						ED_SetCurrSpriteSet(255);
					else
						ED_SetCurrSpriteSet( ED_GetCurrSpriteSet() - 1 );
				}
			}
			if (djiKeyPressed(DJKEY_M))
			{
				ED_SetCurrSpriteSet( ED_GetCurrSpriteSet() + 1 );
				while (g_pCurMission->GetSpriteData(ED_GetCurrSpriteSet()) == NULL)
				{
					if (ED_GetCurrSpriteSet() == 255)
						ED_SetCurrSpriteSet(0);
					else
						ED_SetCurrSpriteSet( ED_GetCurrSpriteSet() + 1 );
				}
			}
		}

		ED_FlipBuffers();//dj2017-07 Moving this out of RedrawView as we want it after handling keys etc. in level mainloop
	} // while (bRunning)

	ED_ClearScreen();

	return SWITCH_EXIT;
}



void SetSprite ( int new_sprite )
{
	int	ox, oy;

	ox = POS_LEVELSPRITES_X;
	oy = POS_LEVELSPRITES_Y;

	ED_SetSprite ( new_sprite, ox, oy );
}


/*
=================
HandleMouse
=================
*/
bool HandleMouse ()
{
	int ax=0, ay=0;

	// sprites area
	g_aMouseHighlighted = 0;
	g_bMouseHighlighted = 0;
	if (INBOUNDS(djMouse::x, djMouse::y,
		POS_LEVELSPRITES_X, POS_LEVELSPRITES_Y,
		POS_LEVELSPRITES_X + SPRITESHEET_NUM_COLS * BLOCKW - 1,
		POS_LEVELSPRITES_Y + SPRITESHEET_NUM_ROWS * BLOCKH - 1 ))
	{
		//a,b spriteset,spritenumber that mouse is floating over
		ax = (djMouse::x - POS_LEVELSPRITES_X) / BLOCKW;
		ay = (djMouse::y - POS_LEVELSPRITES_Y) / BLOCKH;
		g_aMouseHighlighted = ED_GetCurrSpriteSet();
		g_bMouseHighlighted = ay * SPRITESHEET_NUM_COLS + ax;
		if (djMouse::b & 1)
			SetSpriteSelection( g_aMouseHighlighted, g_bMouseHighlighted, sprite1a, sprite1b );
		else if (djMouse::b & 2)
			SetSpriteSelection( sprite0a, sprite0b, g_aMouseHighlighted, g_bMouseHighlighted );

		// Rectangle around sprite under mouse
		//djgSetColorFore( pVisMain, djColor(127,127,127) );
		djgSetColorFore( pVisMain, djColor(0,255,255,150) );//[dj2018-01]cyan to match mouse-over sprite flicker stuff
		djgDrawRectangle( pVisMain,
			POS_LEVELSPRITES_X + ax*BLOCKW - 1,
			POS_LEVELSPRITES_Y + ay*BLOCKH - 1,
			BLOCKW+2,
			BLOCKH+2 );

		//dj2017-07 Show sprite type info
		std::string sType = GetFriendlyTypeName( ED_GetCurrSpriteSet(), ay * 16 + ax );
		if (!sType.empty())
			sType = std::string("Type:") + sType;
		if (!sType.empty())
			ED_DrawString( 0, POS_LEVELSPRITES_Y+BLOCKH*SPRITESHEET_NUM_ROWS, sType.c_str() );
	}
	// main editting area
	else if (INBOUNDS(djMouse::x, djMouse::y, 0, 0, LEVEL_WIDTH * LEVEL_GRIDSIZE - 1, LEVEL_HEIGHT * LEVEL_GRIDSIZE - 1))
	{
		ax = djMouse::x / LEVEL_GRIDSIZE;
		ay = djMouse::y / LEVEL_GRIDSIZE;
		if (g_iKeys[DJKEY_ALT])
		{
			if (djMouse::b & 1)
				MoveMinimap( ax - (levelview_w / 2),
//				level_move_view( ax - (levelview_w / 2),
				ay - (levelview_h / 2) );
		}
		else if (djMouse::b & 1)
		{
			SetLevel( ax, ay, sprite0a, sprite0b, bLevelFore );
			SetDocumentDirty();
		}
		else if (djMouse::b & 2)
		{
			SetLevel( ax, ay, sprite1a, sprite1b, bLevelFore );
			SetDocumentDirty();
		}
	}
	// zoomed view area
	else if (INBOUNDS(djMouse::x, djMouse::y,
		POS_LEVELVIEW_X, POS_LEVELVIEW_Y,
		POS_LEVELVIEW_X + levelview_w * BLOCKW - 1,
		POS_LEVELVIEW_Y + levelview_h * BLOCKH - 1 ))
	{
		ax = (djMouse::x - POS_LEVELVIEW_X) / BLOCKW;
		ay = (djMouse::y - POS_LEVELVIEW_Y) / BLOCKH;
		// dj2016-10: Hold in Ctrl+Alt and click with the mouse to automatically start level with hero 'dropped in' to the clicked position as starting position (to help with level editing / testing)
		if ((djMouse::b & 1)!=0 && g_iKeys[DJKEY_CTRL]!=0 && g_iKeys[DJKEY_ALT]!=0)
		{
			extern int g_nOverrideStartX;
			extern int g_nOverrideStartY;

			g_nOverrideStartX = levelview_x+ax;
			g_nOverrideStartY = levelview_y+ay;
			return false;
		}
		else if (djMouse::b & 1)
		{
			SetLevel( ax + levelview_x, ay + levelview_y,
				sprite0a, sprite0b, bLevelFore );
			SetDocumentDirty();
		}
		else if (djMouse::b & 2)
		{
			SetLevel( ax + levelview_x, ay + levelview_y,
				sprite1a, sprite1b, bLevelFore );
			SetDocumentDirty();
		}
	}
	return true;
}



/*
=================
MoveMinimap
=================
*/
static void MoveMinimap( int ox, int oy )
{
	if ( ( ox == levelview_x ) && ( oy == levelview_y ) ) // Hasn't moved?
		return;

	// Set new level view offset, checking bounds.
	levelview_x = djCLAMP(ox, 0, LEVEL_WIDTH - levelview_w);
	levelview_y = djCLAMP(oy, 0, LEVEL_HEIGHT - levelview_h);

	// Redraw the purple rectangle
	DrawMinimapRectangle();
}





void RedrawView ()
{
	// Slightly cheap n kludgy way of determining flickering on/off
	// (use time of day in seconds, %2, so every second second we're on, and every other
	// second we're off (but use a multiplying factor to speed that up) [dj2018-01-22]
	g_bFlashingIndicatorTimer = g_bFlashingIndicatorEnabled && (((int)(djTimeGetTime()*2.0f) % 2)==0);

	ED_ClearScreen();
	ShowInstructions();
	if (bLevelFore)
		ED_DrawString( 48, 308, "[FOREGROUND]" );
	else
		ED_DrawStringClear( 48, 308, "[FOREGROUND]" );
	DrawSprites();
	DrawGrid( 0, 0, LEVEL_GRIDSIZE, LEVEL_GRIDSIZE, LEVEL_WIDTH, LEVEL_HEIGHT, djColor(60,60,60) );
	DrawLevelGrid();
	DrawSpritesel();
	DrawMinimap();
	DrawLevelname();
	ShowSettings();
	ShowMacros();
	//ED_FlipBuffers ();
}



void DrawSprites ()
{
	int i=0;
	int ox=0, oy=0;
	int xoffset=0, yoffset=0;
	char buf[1024]={0};

//	djgSetColorFore( pVisMain, djColor(255,255,255) );

	snprintf(buf,sizeof(buf), "%d,%-15.15s", ED_GetCurrSpriteSet(), g_pCurMission->GetSpriteData(ED_GetCurrSpriteSet())->m_szImgFilename );

	ED_DrawString( 120, POS_LEVELSPRITES_Y - 8, buf );
	ox = POS_LEVELSPRITES_X;
	oy = POS_LEVELSPRITES_Y;

	for ( i=0; i<SPRITES_PER_SPRITESHEET; i++ )
	{
		//dj2019-07 hm should we have like a separate SPRITEW/SPRITEH here .. or something else .. hm. (thinking out loud)
		xoffset = (i % SPRITESHEET_NUM_COLS) * BLOCKW;
		yoffset = (i / SPRITESHEET_NUM_COLS) * BLOCKH;
		ED_DrawSprite( ox + xoffset, oy + yoffset, ED_GetCurrSpriteSet(), i );
	}

	// make it redraw the pointers to the current sprite
	SetSprite ( ED_GetCurrSprite () );
}


void DrawGrid( int x, int y, int w, int h, int nx, int ny, const djColor& clr )
{
	djgSetColorFore( pVisMain, clr );
	int i;
	for ( i=0; i<=ny; i++ )
	{
		djgDrawHLine( pVisMain, x, y+i*h, w*nx+1 );
	}
	for ( i=0; i<=nx; i++ )
	{
		djgDrawVLine( pVisMain, x+i*w, y, h*ny+1 );
	}
}



void DrawLevelGrid()
{
	for ( int y=0; y<LEVEL_HEIGHT; ++y )
	{
		for ( int x=0; x<LEVEL_WIDTH; ++x )
		{
			DRAW_LEVEL_PIXEL(x, y);
		}
	}
}


void DrawLevelname()
{
	std::string sLevelName;
	const char * szfilename = g_pCurMission->GetLevel(g_nLevel)->GetFilename();
	sLevelName += szfilename;

	if (g_bDocumentDirty)
	{
		djgSetColorFore(pVisMain, djColor(80,0,0));
		sLevelName += " * UNSAVED CHANGES";
	}
	else 
		djgSetColorFore(pVisMain, djColor(0,80,0));
	// Draw background
	djgDrawBox(pVisMain, 0, POS_LEVELVIEW_Y + 16 * levelview_h + 8, pVisMain->width/2, 8);
	//else
	/*ED_DrawStringClear( 0, POS_LEVELVIEW_Y + 16 * levelview_h + 8,
		"                           " );*/
	ED_DrawString( 0, POS_LEVELVIEW_Y + 16 * levelview_h + 8,
		sLevelName.c_str() );
}
void ShowSettings()
{
	char szBuf[4096]={0};
	snprintf(szBuf,sizeof(szBuf), "Selected Sprite Indicators: %s", g_bFlashingIndicatorEnabled ? "ON" : "OFF");
	// Clear background
	djgSetColorFore(pVisMain, djColor(0,0,80));//blue
	djgDrawBox(pVisMain, 0, POS_LEVELVIEW_Y + 16 * levelview_h + 8 + 8, pVisMain->width/2, 8);
	ED_DrawString( 0, POS_LEVELVIEW_Y + 16 * levelview_h + 8 + 8, szBuf );
}

void DrawMinimapRectangle()
{
	djgSetColorFore( pVisMain, djColor(255,0,255) );
	djgDrawRectangle( pVisMain,
		levelview_x * LEVEL_GRIDSIZE,
		levelview_y * LEVEL_GRIDSIZE,
		(levelview_w-1) * LEVEL_GRIDSIZE + LEVEL_GRIDSIZE,
		(levelview_h-1) * LEVEL_GRIDSIZE + LEVEL_GRIDSIZE );
}

void DrawMinimap()
{
	int i = 0, j = 0;
	int a = 0, b = 0;

	// Draw the purple rectangle indicating where your zoomed view is
	DrawMinimapRectangle();
	// Draw the zoomed view
	bool bHighlightBack = false;//Must be drawn after the foreground block's drawn, that's why we use this flag
	bool bHighlightBackMouseOver = false;//Must be drawn after the foreground block's drawn, that's why we use this flag
	for ( i=0; i<levelview_h; i++ )
	{
		for ( j=0; j<levelview_w; j++ )
		{
			a = *(level_pointer( 0, levelview_x + j, levelview_y + i) + 2);
			b = *(level_pointer( 0, levelview_x + j, levelview_y + i) + 3);
			bHighlightBack = false;//dj2018-01 Flickering display to highlight selected sprite
			bHighlightBackMouseOver = false;//dj2018-01 Flickering display to highlight mouse-over sprite
			// Draw a black block if ShowBack is disabled
			if (!bShowBack)
			{
				djgSetColorFore( pVisMain, djColor(0,0,0) );
				djgDrawBox( pVisMain, POS_LEVELVIEW_X + j*BLOCKW, POS_LEVELVIEW_Y + i*BLOCKH, BLOCKW, BLOCKH );
			}
			else
			{
				ED_DrawSprite( POS_LEVELVIEW_X + j * BLOCKW,
					POS_LEVELVIEW_Y + i * BLOCKH,
					a, b );
				if ( ((a) || (b)) )
				{
					if (a==sprite0a && b==sprite0b)
						bHighlightBack = true;
					
					if ((g_aMouseHighlighted | g_bMouseHighlighted) != 0 &&
						(a == g_aMouseHighlighted&&b == g_bMouseHighlighted))
						bHighlightBackMouseOver = true;
				}

			}
			if (bLevelFore)
			{
				a = *(level_pointer( 0, levelview_x + j, levelview_y + i) + 0);
				b = *(level_pointer( 0, levelview_x + j, levelview_y + i) + 1);
				if ((a) || (b))
				{
					ED_DrawSprite( POS_LEVELVIEW_X + j * BLOCKW,
					POS_LEVELVIEW_Y + i * BLOCKH,
					a, b );
					if (g_bFlashingIndicatorTimer)
					{
						const int nCROSSHAIRSIZE=8;
						if (a==sprite0a && b==sprite0b)
						{
							djgSetColorFore( pVisMain, djColor(0,0,0,200) );//black
							djgDrawBox( pVisMain, -1+POS_LEVELVIEW_X + j*BLOCKW, -2+POS_LEVELVIEW_Y + i*BLOCKH   ,18,4);
							djgDrawBox( pVisMain, -1+POS_LEVELVIEW_X + j*BLOCKW,    POS_LEVELVIEW_Y + i*BLOCKH+12,18,4);
							djgSetColorFore( pVisMain, djColor(255,255,0,200) );//yellow
							djgDrawBox( pVisMain, POS_LEVELVIEW_X + j*16, -2+POS_LEVELVIEW_Y + i*BLOCKH +1,16,2);
							djgDrawBox( pVisMain, POS_LEVELVIEW_X + j*16,    POS_LEVELVIEW_Y + i*BLOCKH+13,16,2);
						}

						if ( (g_aMouseHighlighted | g_bMouseHighlighted)!=0  &&
							(a==g_aMouseHighlighted&&b==g_bMouseHighlighted))
						{
							const int nCROSSHAIROFFSET=4;
							djgSetColorFore( pVisMain, djColor(0,255,255,200) );//cyan
							djgDrawBox( pVisMain, nCROSSHAIROFFSET +                    POS_LEVELVIEW_X + j*BLOCKW, nCROSSHAIROFFSET+(nCROSSHAIRSIZE/2)+POS_LEVELVIEW_Y + i*BLOCKH,nCROSSHAIRSIZE,1);
							djgDrawBox( pVisMain, nCROSSHAIROFFSET + (nCROSSHAIRSIZE/2)+POS_LEVELVIEW_X + j*BLOCKW, nCROSSHAIROFFSET+                   POS_LEVELVIEW_Y + i*BLOCKH,1,nCROSSHAIRSIZE);
						}
					}
				}
			}//bLevelFore
			const int nCROSSHAIRSIZE=5;
			if (g_bFlashingIndicatorTimer && bHighlightBack)
			{
				const int nCROSSHAIROFFSET=0;
				djgSetColorFore( pVisMain, djColor(255,255,0,150) );//yellow
				djgDrawBox( pVisMain, nCROSSHAIROFFSET +                    POS_LEVELVIEW_X + j*BLOCKW, nCROSSHAIROFFSET+(nCROSSHAIRSIZE/2)+POS_LEVELVIEW_Y + i*BLOCKH,nCROSSHAIRSIZE,1);
				djgDrawBox( pVisMain, nCROSSHAIROFFSET + (nCROSSHAIRSIZE/2)+POS_LEVELVIEW_X + j*BLOCKW, nCROSSHAIROFFSET+                   POS_LEVELVIEW_Y + i*BLOCKH,1,nCROSSHAIRSIZE);
			}
			if (g_bFlashingIndicatorTimer && bHighlightBackMouseOver)
			{
				const int nCROSSHAIROFFSET=8;
				djgSetColorFore( pVisMain, djColor(0,255,255,200) );//cyan
				djgDrawBox( pVisMain, nCROSSHAIROFFSET +                    POS_LEVELVIEW_X + j*BLOCKW, nCROSSHAIROFFSET+(nCROSSHAIRSIZE/2)+POS_LEVELVIEW_Y + i*BLOCKH,nCROSSHAIRSIZE,1);
				djgDrawBox( pVisMain, nCROSSHAIROFFSET + (nCROSSHAIRSIZE/2)+POS_LEVELVIEW_X + j*BLOCKW, nCROSSHAIROFFSET+                   POS_LEVELVIEW_Y + i*BLOCKH,1,nCROSSHAIRSIZE);
			}
		}
	}

	// If hold in Ctrl+Alt, draw hero overlay at mouse cursor position to indicate the new hero 'drop-in-level-here' functionality [dj2016-10]
	if (INBOUNDS(djMouse::x, djMouse::y,
		POS_LEVELVIEW_X, POS_LEVELVIEW_Y,
		POS_LEVELVIEW_X + levelview_w * BLOCKW - 1,
		POS_LEVELVIEW_Y + levelview_h * BLOCKH - 1 ))
	{
		int ax = (djMouse::x - POS_LEVELVIEW_X) / BLOCKW;
		int ay = (djMouse::y - POS_LEVELVIEW_Y) / BLOCKH;
		if (g_iKeys[DJKEY_CTRL]!=0 && g_iKeys[DJKEY_ALT]!=0)
		{
			// These sprite offsets etc. are horribly hardcoded [show hero sprite]
			if (ay>0)
			{
				ED_DrawSprite( POS_LEVELVIEW_X + (ax-1) * BLOCKW+8, POS_LEVELVIEW_Y + (ay-1) * BLOCKH, 4, 16 );
				ED_DrawSprite( POS_LEVELVIEW_X + (ax  ) * BLOCKW+8, POS_LEVELVIEW_Y + (ay-1) * BLOCKH, 4, 17 );
			}
			ED_DrawSprite( POS_LEVELVIEW_X + (ax-1) * BLOCKW+8, POS_LEVELVIEW_Y + ay * BLOCKH, 4, 18 );
			ED_DrawSprite( POS_LEVELVIEW_X + (ax  ) * BLOCKW+8, POS_LEVELVIEW_Y + ay * BLOCKH, 4, 19 );
		}
	}
}



void ShowInstructions()
{
	//[dj20126-10] fixme[low/future]: Font width/height should be gotten from the font object, not hardcoded all over the place
	const unsigned int FONT_HEIGHT = 8;
	const unsigned int FONT_WIDTH = 8;
	
	// [dj2016-10] Instructions in bottom right of screen
	unsigned int uMaxStrLen = 0;
	for ( int i=0; i<NUM_LEVEL_INSTRUCTIONS; ++i )//Get longest string width, to calculate this things width
	{
		uMaxStrLen = djMAX(uMaxStrLen, (unsigned int)strlen(level_instructions[i]));
	}
	unsigned int uOffsetX = pVisMain->width - uMaxStrLen * FONT_WIDTH;
	unsigned int uOffsetY = pVisMain->height - NUM_LEVEL_INSTRUCTIONS * FONT_HEIGHT;
	djgSetColorFore( pVisMain, djColor(42,57,112) );
	djgDrawBox( pVisMain, uOffsetX, uOffsetY, uMaxStrLen*FONT_WIDTH, NUM_LEVEL_INSTRUCTIONS*FONT_HEIGHT );
	for ( int i=0; i<NUM_LEVEL_INSTRUCTIONS; i++ )
	{
		ED_DrawString(
			uOffsetX,
			uOffsetY + i * FONT_HEIGHT,
			level_instructions[i] );
	}
}



void DrawSpritesel ()
{
	ED_DrawSprite(  4, 308, sprite0a, sprite0b );
	ED_DrawSprite( BLOCKW+8, 308, sprite1a, sprite1b );
	DBC_DrawBoxContents();
}



bool LVLED_GetLevelFore ()
{
	return bLevelFore;
}



void SetSpriteSelection( int a0, int b0, int a1, int b1 )
{
	sprite0a = a0;
	sprite0b = b0;
	sprite1a = a1;
	sprite1b = b1;

//	level_draw_spritesel();
	ED_DrawSprite(  4, 308, sprite0a, sprite0b );
	ED_DrawSprite( 24, 308, sprite1a, sprite1b );
	DBC_DrawBoxContents();
}



void SetLevel( int x, int y, int a, int b, bool bforeground )
{
	if (x<0 || y<0) return;
	if (x>=LEVEL_WIDTH || y>=LEVEL_HEIGHT) return;
	if (bforeground)
	{
		*(level_pointer(0, x, y) + 0) = a;
		*(level_pointer(0, x, y) + 1) = b;
	}
	else
	{
		*(level_pointer(0, x, y) + 2) = a;
		*(level_pointer(0, x, y) + 3) = b;

		DRAW_LEVEL_PIXEL(x, y);
		/*
		djgSetColorFore( pVis, sprite_get_color(a,b) );
		djgDrawBox( pVis,
			x*LEVEL_GRIDSIZE+1, y*LEVEL_GRIDSIZE+1,
			LEVEL_GRIDSIZE-1, LEVEL_GRIDSIZE-1 );
			*/
	}

	if ( ( x >= levelview_x )
		&& ( y >= levelview_y )
		&& ( x < levelview_x + levelview_w )
		&& ( y < levelview_y + levelview_h ) )
		ED_DrawSprite( POS_LEVELVIEW_X + (x - levelview_x) * BLOCKW,
		POS_LEVELVIEW_Y + (y - levelview_y) * BLOCKH,
		a, b );
}


void SelectLevel ( int i )
{
	if (i < 0)
		i = 0;
	if (i > g_pCurMission->NumLevels() - 1)
		i = g_pCurMission->NumLevels() - 1;

	g_nLevel = i;
	
	g_bDocumentDirty=false;

//	level_draw_view();
//	draw_level_grid();
//	level_draw_levelname();
}




void LevelFill( int ax, int ay )
{
	if ( (ax < 0) || (ay < 0) || (ax >= LEVEL_WIDTH) || (ay >= LEVEL_HEIGHT) )
		return;

	// background block
	int a, b; // block type to replace
	a = *(level_pointer( 0, ax, ay ) + 2);
	b = *(level_pointer( 0, ax, ay ) + 3);

	// Start at 'x', then dec one by one, filling in blocks until either we hit left side of level or a block that's different to the 'current' block
	int fillx = ax;
	while ( (a == *(level_pointer( 0, fillx, ay ) + 2)) &&
		(b == *(level_pointer( 0, fillx, ay ) + 3)) && (fillx >= 0) )
	{
		SetLevel( fillx, ay, sprite0a, sprite0b, false );
		fillx--;
		SetDocumentDirty();
	}
	// Start at 'x+1', then inc one by one, filling in blocks until either we hit right side of level or a block that's different to the 'current' block
	fillx = ax + 1;
	while ( (a == *(level_pointer( 0, fillx, ay ) + 2)) &&
		(b == *(level_pointer( 0, fillx, ay ) + 3)) && (fillx < LEVEL_WIDTH) )
	{
		SetLevel( fillx, ay, sprite0a, sprite0b, false );
		fillx++;
		SetDocumentDirty();
	}
}
