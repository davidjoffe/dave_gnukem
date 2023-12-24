/*!
\file    mission.h
\brief   Level sets, campaigns, missions, whatever you want to call 'em
\author  David Joffe

Copyright (C) 1998-2023 David Joffe
*/
/*--------------------------------------------------------------------------*/
/* David Joffe '99/02 */
/* Level sets, campaigns, missions, whatever you want to call 'em */
/*--------------------------------------------------------------------------*/
#ifndef _MISSION_H_
#define _MISSION_H_
/*--------------------------------------------------------------------------*/
#include "config.h"
#include "mixins.h"
#include "djtypes.h"
#include "djimage.h"

#include <vector>

#include <string>
/*--------------------------------------------------------------------------*/
class CMission;
class CLevel;
class CSpriteData;
/*--------------------------------------------------------------------------*/
// Level access macros
// Macro should be faster than the actual function
// FIXME: OPTIMIZE ME! OPTIMIZE ME! (Is it worth it? Real bottleneck is graphics ...)
// Fixme again, I don't think this needs optimizing anymore, I think it is only
// used in initialization (or is supposed to be only)
#define GET_EXTRA(a,b,i) ( g_pCurMission->GetSpriteData((a))->m_extras[(b)][(i)] )
// Same as GET_EXTRA but I think the name is more self-explanatory than 'extra' [dj2017-08]
#define GET_SPRITE_EXTRA_METADATA(a,b,i) ( GET_EXTRA((a),(b),(i)) )
#define CHECK_FLAG(a,b,f) ( GET_EXTRA( (a), (b), EXTRA_FLAGS ) & (f) )

#define FLAG_SOLID			1
#define FLAG_ANIMATED		2
#define FLAG_FALLS			4
#define FLAG_INVENTORYITEM	8
#define FLAG_PERSISTENT		16

#define EXTRA_0        0 // healthdiff, key/door num, monster type etc
#define EXTRA_SCORELO  1
#define EXTRA_SCOREHI  2
#define EXTRA_3        3
#define EXTRA_FLAGS    4

/*--------------------------------------------------------------------------*/
extern std::vector<CMission * > g_apMissions;
extern CMission * g_pCurMission;
extern int LoadMissions( const char * szfilename );
extern void InitMissionSystem();
extern void KillMissionSystem();
/*--------------------------------------------------------------------------*/
/*!
\class CMission
\nosubgrouping

A "mission" consists of a set of levels to be played, it contains an array of
CSpriteData's and the ID numbers (0-255?) associated with them.
Each CSpriteData consists of a djImage, which is normally the .TGA file with
all the sprites on them, as well as all the data associated with the sprite
file. Each TGA is 256x128 (for historical reasons, it made sense back in the
original DOS EGA version :) and thus each CSpriteData represents 128 16x16 sprites.

*/
#define NUM_SPRITE_DATA (256)
class CMission : public CNamed
{
public:
	// Constructor(s) and destructor
	CMission();
	~CMission();

	// Methods

	int		Load( const char * szfilename );

	void	AddLevel( CLevel * pLevel );
	CLevel*	GetLevel( int i );
	int		NumLevels() const;

	void			AddSpriteData( int iID, CSpriteData * pSprites );
	CSpriteData*	GetSpriteData( int iID );

	// FIXME: Do we want this here?
	int		LoadSprites();
	int		SaveSprites();

	// Attributes

	std::vector<CLevel *>	m_apLevels;
	CSpriteData			*m_apSpriteData[NUM_SPRITE_DATA];

	std::string GetFilename() const { return m_sFilename; }
protected:
	std::string m_sFilename;//Store the filename so we can remember it for load/save game purposes [dj2016-10 - hm, conceptually it doesn't quite feel right having this here, but, there are probably worse things in the codebase to worry about right now]
};
/*--------------------------------------------------------------------------*/
/*!
\class CLevel
\nosubgrouping

Level descriptor (e.g. information such as filename). Not to be confused with a particular LOADED level while in-game (which is a large memory block currently).
This should probably be renamed to LevelDescriptor
*/
class CLevel : public CNamed
{
public:
	CLevel();
	~CLevel();

	void SetFilename( const char *szFilename );
	const char *GetFilename() const { return m_szFilename; }

	char *m_szFilename = nullptr;
	char *m_szName = nullptr;
	char *m_szAuthor = nullptr;
	char *m_szBackground = nullptr; // background image filename

	std::string m_sBack1 = "";//"levels/back1.png";;
};
/*--------------------------------------------------------------------------*/
// This 128 has nothing to do with the levelwidth 128, not to be confused with it (dj2019-07):
// Note that the file format is currently 128 also exactly of these .. so if you change this it will break existing metadata files .. ew.
// OH NO IT WONT I see I thought of that, the file format has this valu ein it.
#define SPRITES_PER_SPRITESHEET (128)
//dj2019-07 adding these helpers .. in theory SPRITESHEET_NUM_COLS * SPRITESHEET_NUM_ROWS should = SPRITES_PER_SPRITESHEET
// So for DG1 BLOCKW/BLOCKH=16x16, but each of our spriteset images has 16x8 (128) sprites on it .. 16 across, 8 down. That 16 has nothing do with BLOCK/BLOCK 16, that's coincidence.
// So the pixel image size/resolution of a single spriteset image should be: width=BLOCKW * SPRITESHEET_NUM_COLS and height=BLOCKH * SPRITESHEET_NUM_ROWS
// [//dj2019-07] All this is kinda gross, of course, for a proper generic engine we want to be able to throw arbitrary sprite sizes at it, it will simplify things.
#define SPRITESHEET_NUM_COLS (16)
#define SPRITESHEET_NUM_ROWS (8)
/*--------------------------------------------------------------------------*/
class CSpriteData
{
public:
	CSpriteData();
	~CSpriteData();

	// Load/save the data (extras, type, color)
	int LoadData( const char *szFilename );
	int SaveData( const char *szFilename );

	// Load the sprite set image
	int LoadSpriteImage();

	int          m_iID;

	djImage *m_pImage;
#ifdef djSPRITE_AUTO_DROPSHADOWS
	djImage *m_pImageAutoShadow;//!< Sprite auto-dropshadow effect
#endif
	char * m_szImgFilename;

	// not sure if this is used currently(dj2019-07):
	//int          GetID() const { return m_iID; }

	char *       m_szFilenameData;

	// These should probably be in a struct with one array of struct members or something.
	int          m_extras[SPRITES_PER_SPRITESHEET][12];   // extras values (sprite metadata)
	int          m_type[SPRITES_PER_SPRITESHEET];         // block type
	djColor      m_Color[SPRITES_PER_SPRITESHEET];        // block color
};
/*--------------------------------------------------------------------------*/
#endif
