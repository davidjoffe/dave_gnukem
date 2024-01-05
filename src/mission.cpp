/*--------------------------------------------------------------------------*/
/* David Joffe '99/02 */
/* Level sets, campaigns, missions, whatever you want to call 'em */

/*
mission.cpp

Copyright (C) 1999-2024 David Joffe
*/
/*--------------------------------------------------------------------------*/
#include "config.h"
#include "djfile.h"
#include "datadir.h"
#include "mission.h"
#include "djlog.h"
#include "djstring.h"
#include "graph.h"//djCreateImageHWSurface
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <string>
#include <fstream>


std::vector<CMission * > g_apMissions;
// FIXME: Does this stuff really belong here? Probably not.
CMission * g_pCurMission = NULL;
/*--------------------------------------------------------------------------*/
int LoadMissions( const char * szfilename )
{
	if (szfilename==NULL) return -1; // NULL string
	if (szfilename[0]==0) return -2; // empty string
	FILE * fin=NULL;
	char buf[4096]={0};

	// open file
	if ( NULL == ( fin = djFile::dj_fopen( szfilename, "r" ) ) )
	{
		djMSG("ERROR: LoadMissions(%s): Unable to open file.\n", szfilename);
		return -3;
	}

	//" If the End-of-File is encountered and no characters have been read, the contents of str remain unchanged and a null pointer is returned."
	#define djREADLINE() buf[0]=0; if ((fgets(buf, sizeof(buf), fin) == NULL) && ferror(fin)) goto error; djStripCRLF(buf)

	// Read the list of available mission filenames from the "mission registry"
	//djREADLINE();
	while (!feof(fin))
	{
		djREADLINE();
		if ( strlen(buf) > 1 )
		{
			CMission* pMission = new CMission;

			// Load this mission and add to list (FIXME: ERROR CHECK)
			if (pMission->Load(buf) < 0)
			{
				djMSG("ERROR: Error loading mission\n");
			}
			// Maybe it partially loaded so maybe go on anyway
			g_apMissions.push_back(pMission);
		}
	}

	fclose( fin );

	if (g_apMissions.size()<1)
	{
		djMSG("ERROR: LoadMissions(%s): No valid missions loaded from file.\n", szfilename);
		return -1;
	}

	// Set current mission by default to the first one in the list
	g_pCurMission = g_apMissions[0];
	return 0;

error:
	djMSG("ERROR: LoadMissions(%s): File loading error.\n", szfilename);
	fclose(fin);
	return -1;
}
/*--------------------------------------------------------------------------*/
CMission::CMission()
{
	int i;
	for ( i=0; i<NUM_SPRITE_DATA; i++ )
	{
		m_apSpriteData[i] = NULL;
	}
}

CMission::~CMission()
{
	for ( unsigned int i=0; i<m_apLevels.size(); i++ )
	{
		djDEL(m_apLevels[i]);
	}
	m_apLevels.clear();

	for ( unsigned int i=0; i<NUM_SPRITE_DATA; i++ )
	{
		djDEL(m_apSpriteData[i]);
	}
}

int CMission::Load( const char * szfilename )
{
	if (szfilename==NULL) return -1; // NULL string
	if (szfilename[0]==0) return -2; // empty string

	std::ifstream	fin;
	std::string		line;
	int			state=0;
	std::string sFilename;

	SYS_Debug ( "CMission::Load( %s ): Loading ...\n", szfilename );

	sFilename = djDATAPATHs(szfilename);
	//snprintf( filename, sizeof(filename), "%s%s", DATA_DIR, szfilename );
	// open file
	fin.open (sFilename.c_str());
	if ( !fin.is_open() )
	{
		SYS_Error ( "CMission::Load( %s ) Unable to open file.\n", sFilename.c_str());
		return -3;
	}

	state = 0;
	while ( getline ( fin, line ) )
	{
		//dj2017-06 If file has DOS/Windows style newlines then on Linux these get parsed into the string,
		// which causes problems loading e.g. sprite data because we get a sprite data filename that ends
		// in a newline (or CR, I'm not sure, doesn't matter, both are wrong) which then causes file to fail to load.
		// I don't think it's ever desirable for the read 'line' to end in newline or CR so let's just
		// chop it off when it happens.
		while (line.length()>0 && 
			( line[line.length()-1]=='\r' || line[line.length()-1]=='\n' )
			)
		{
			line = line.substr( 0, line.length() - 1 );
		}

		switch (state)
		{
			///////////////////////////////////////////////////////////////
		case 0: // name
			SetName( line.c_str() );
			SYS_Debug ( "CMission::Load(): Name: %s\n", GetName() );
			state = 1;
			SYS_Debug ( "CMission::Load(): Description:\n", line.c_str() );
			break;
			///////////////////////////////////////////////////////////////
		case 1: // description
			if ( 0 == strcmp( line.c_str(), "~" ) || 0 == strcmp( line.c_str(), "~\r" ))
			{
				state = 2;
				SYS_Debug ( "-------------\n" );
			}
			else
			{
				// add this part of string to description
				SYS_Debug ( "%s\n", line.c_str() );
			}
			break;
			///////////////////////////////////////////////////////////////
		case 2: // level filenames, names, authors
			if ( 0 == strcmp( line.c_str(), "~" ) || 0 == strcmp( line.c_str(), "~\r" ))
			{
				state = 3;
			}
			else
			{
				CLevel * pLevel;
				pLevel = new CLevel;
				char *szFilename, *szName, *szAuthor, *szBackground;
				// Interpret line of level information

				szFilename   = djStrPart( line.c_str(), 1, "," );
				szBackground = djStrPart( line.c_str(), 2, "," );
				szName       = djStrPart( line.c_str(), 3, "," );
				szAuthor     = djStrPart( line.c_str(), 4, "," );

				SYS_Debug ( "CMission::Load(): Level: [%s][%s][%s]\n", szFilename, szName, szAuthor );

				pLevel->SetFilename(szFilename);
				pLevel->m_szName		= djStrDeepCopy(szName);
				pLevel->m_szBackground	= djStrDeepCopy(szBackground);
				pLevel->m_szAuthor		= djStrDeepCopy(szAuthor);
				AddLevel( pLevel );

				djDELV(szFilename);
				djDELV(szName);
				djDELV(szAuthor);
				djDELV(szBackground);
			}
			break;
			///////////////////////////////////////////////////////////////
		case 3: // sprites
			if ( 0 == strcmp( line.c_str(), "~" ) || 0 == strcmp( line.c_str(), "~\r" ))
			{
				state = 4;
			}
			else
			{
				char * szID, * szDatafile, *szImgFilename;
				CSpriteData * pSpriteData;

				pSpriteData = new CSpriteData;

				szID          = djStrPart( line.c_str(), 1, "," );
				szImgFilename = djStrPart( line.c_str(), 2, "," );
				szDatafile    = djStrPart( line.c_str(), 3, "," );

				SYS_Debug ( "CMission::Load(): Sprites: [%s][%s][%s]\n", szID, szImgFilename, szDatafile );
				// Should this happen? NO! LATER! Then must remember names!
				//	       pSprites->LoadSpriteImage(
				pSpriteData->m_iID            = atoi( szID );
				pSpriteData->m_szImgFilename  = djStrDeepCopy( szImgFilename );
				pSpriteData->m_szFilenameData = djStrDeepCopy( szDatafile );
				AddSpriteData( pSpriteData->m_iID, pSpriteData );

				djDELV( szID );
				djDELV( szImgFilename );
				djDELV( szDatafile );
			}
			break;
		}
	} // while ( getline() )

	if ( 4 != state )
	{
		SYS_Error( "CMission::Load( %s ): Bad end state %d\n", sFilename.c_str(), state );
	}

	// Remember the filename for load/save game purposes (to know which 'mission' to load/save) [dj2016-10]
	// NOTE this must is stored without the "data/" prepended as that may be user or port- configurable and gets prepended during load
	// fixme test this!
	m_sFilename = szfilename;

	fin.close ();

	return 0;
}

void CMission::AddLevel( CLevel * pLevel )
{
	m_apLevels.push_back( pLevel );
}

CLevel *CMission::GetLevel( int i )
{
	return m_apLevels[i];
}

int CMission::NumLevels() const
{
	return m_apLevels.size();
}

void CMission::AddSpriteData( int iID, CSpriteData * pSprites )
{
	TRACE( "CMission::AddSpriteData( %d )\n", iID );
	if ( m_apSpriteData[ iID ] != NULL )
	{
		delete m_apSpriteData[ iID ];
	}
	m_apSpriteData[ iID ] = pSprites;
}

CSpriteData * CMission::GetSpriteData( int iID )
{
	// FIXME: Error check here
	return m_apSpriteData[ iID ];
	//printf("Done\n");
}

int CMission::LoadSprites()
{
	int i;

	// i iterates through the 256 possible ID's for spritesets
	for ( i=0; i<256; i++ )
	{
		CSpriteData * pSpriteData;

		pSpriteData = g_pCurMission->GetSpriteData( i );
		if ( pSpriteData != NULL )
		{
			// FIXME: WHERE IS OUR ERROR CHECKING!?!?!?
			// Load spriteset (the sprite image)
			pSpriteData->LoadSpriteImage();

			// Load data (flags etc associated with sprites)
			std::string sFilename = djDATAPATHs(pSpriteData->m_szFilenameData);
			if (pSpriteData->LoadData(sFilename.c_str()) < 0)
			{
				printf("WARNING: Error loading sprite data: %s\n", sFilename.c_str());
				SYS_Debug("WARNING: Error loading sprite data: ");
			}

		} // if
	} // i

	return 0;
}

int CMission::SaveSprites()
{
	int i=0;
	int nRet = 0;

	// i iterates through the 256 possible ID's for spritesets
	for ( i=0; i<256; i++ )
	{
		CSpriteData * pSpriteData;

		pSpriteData = g_pCurMission->GetSpriteData( i );
		if ( pSpriteData != NULL ) // It *can* be NULL
		{
			// Save sprite data file
			std::string sFilename = djDATAPATHs(pSpriteData->m_szFilenameData);
			/*
			char szFilename[4096]={0};
			// Save sprite data file
#ifdef DATA_DIR
			snprintf( szFilename, sizeof(szFilename), "%s%s", DATA_DIR, pSpriteData->m_szFilenameData );
#else
			snprintf( szFilename, sizeof(szFilename), "%s", pSpriteData->m_szFilenameData );
#endif
			*/
			// dj2022-11 Re this "if (nRet>=0)" I don't quite see the logic of why if saving fails for a spriteset it should seemingly stop saving further ones? I can't recall if that makes sense but commenting it out for now but keep an eye here if side effects
			//if (nRet>=0)
				nRet = pSpriteData->SaveData( sFilename.c_str() );
				if (nRet < 0)
				{
					printf("WARNING: Error saving sprite data: %s\n", sFilename.c_str());
					SYS_Debug("WARNING: Error saving sprite data: ");
					//SYS_Debug("%s", sFilename.c_str());
				}

		} // if
	}

	return nRet;
}


void InitMissionSystem()
{
}

void KillMissionSystem()
{
	std::vector<CMission*>::iterator	i;

	for ( i=g_apMissions.begin(); i!=g_apMissions.end(); ++i )
	{
		if ( NULL != *i )
			delete *i;
	}
	g_apMissions.clear();
}

/*--------------------------------------------------------------------------*/
CLevel::CLevel()
{
	m_szFilename = NULL;
	m_szName = NULL;
	m_szAuthor = NULL;
	m_szBackground = NULL;
}

CLevel::~CLevel()
{
	djDELV(m_szFilename);
	djDELV(m_szName);
	djDELV(m_szAuthor);
	djDELV(m_szBackground);
}

void CLevel::SetFilename( const char *szFilename )
{
	djDELV(m_szFilename);
	m_szFilename = djStrDeepCopy( szFilename );
}

/*--------------------------------------------------------------------------*/
CSpriteData::CSpriteData()
{
	m_iID = -1;
	m_pImage = NULL;
#ifdef djSPRITE_AUTO_DROPSHADOWS
	m_pImageAutoShadow = NULL;
#endif
	m_szImgFilename = NULL;
	m_szFilenameData = NULL;
	memset(m_extras, 0, sizeof(m_extras));
	memset(m_type, 0, sizeof(m_type));
}

CSpriteData::~CSpriteData()
{
#ifdef djSPRITE_AUTO_DROPSHADOWS
	djDestroyImageHWSurface(m_pImageAutoShadow);
	djDEL(m_pImageAutoShadow);
#endif
	djDestroyImageHWSurface(m_pImage);
	djDEL(m_pImage);
	djDELV(m_szImgFilename);
	djDELV(m_szFilenameData);
}

int CSpriteData::LoadData( const char *szFilename )
{
	if (szFilename == nullptr) return -1;
	FILE	*fin=NULL;
	int		i=0, j=0;
	int		temp=0;

	SYS_Debug ( "CSpriteData::LoadData( %s ): Loading ...\n", szFilename );

	// open the file
	// [dj2022-11 hm this seems to first try without DATA_DIR which fails then tries with DATA_DIR ..simplify?]
	if ((!djFileExists(szFilename)) || (NULL == (fin = djFile::dj_fopen( szFilename, "r" ))))
	{
		//dj2022-11 moving this DATA_DIR prepend to the CALLER .. not quite which is more 'correct' but I think probably the caller
		/*
#ifdef DATA_DIR
		// safety? we want to be able to handle VERY long paths elegantly all over the code (and also Unicode filenames) .. maybe make some file helpers etc.
		char buf[8192]={0};
		snprintf(buf,sizeof(buf), "%s%s", DATA_DIR, szFilename );
		if (NULL == (fin = djFile::dj_fopen( buf, "r" )))
#endif
		*/
		{
			SYS_Error ( "CSpriteData::LoadData( %s ): Error opening file\n", szFilename );
			return -1;
		}
	}

	// Read "128"
	if (dj_fscanf_int(fin, temp) <= 0) goto error;

	for ( i=0; i<SPRITES_PER_SPRITESHEET; i++ )
	{
		// Read type
		if (dj_fscanf_int(fin, temp) <= 0) goto error;
		if (feof(fin))
			goto error;

		m_type[i] = temp;

		// Read extras
		for ( j=0; j<11; j++ )
		{
			if (dj_fscanf(fin, "%i,", &temp) <= 0) goto error;
			m_extras[i][j] = temp;
		}
		if (dj_fscanf_int(fin, temp) <= 0) goto error;
		m_extras[i][11] = temp;

		// FIXME: FOLLOWING OLD COLOR OBSOLETE
		// Read block color (2019-06 THIS IS NOW DEPRECATED/OBSOLETE, this was for the old m_color variable, but we still load/save a dummy value to keep compatibility with existing sprite files.)
		if (dj_fscanf_int(fin, temp) <= 0) goto error;
		if (feof(fin))
			goto error;
		//(deprecated)m_color[i] = temp;


		// Calculate color from sprite by averaging the 16x16 array of pixels (or whatever the dimensions are - dj2019-07 extend to handle other size sprites)
		if (m_pImage)
		{
			int iIndexX = (i%SPRITESHEET_NUM_COLS)*BLOCKW; // x offset into image
			int iIndexY = (i/SPRITESHEET_NUM_COLS)*BLOCKH; // y offset into image
			int r=0,g=0,b=0;
			for ( j=0; j<BLOCKH; ++j )
			{
				for ( int k=0; k<BLOCKW; ++k )
				{
					const djColor& clr = m_pImage->GetPixelColor(k+iIndexX, j+iIndexY);
					r += (int)clr.r;
					g += (int)clr.g;
					b += (int)clr.b;
				}
			}
			r /= (BLOCKW*BLOCKH);
			g /= (BLOCKW*BLOCKH);
			b /= (BLOCKW*BLOCKH);
			m_Color[i] = djColor((unsigned char)r,(unsigned char)g,(unsigned char)b);
		}
	}

	fclose( fin );
	return 0;

error:
	fclose( fin );
	SYS_Error ( "CSpriteData::LoadData( %s ): Error reading data.\n", szFilename );
	return -2;
}

int CSpriteData::SaveData( const char *szFilename )
{
	if (szFilename == nullptr) return -1;
	//assert(

	TRACE( "CSpriteData::SaveData( %s ): Saving ...\n", szFilename );

	FILE* fout = nullptr;
	if (NULL == (fout = djFile::dj_fopen( szFilename, "w" )))
	{
		djMSG( "CSpriteData::SaveData( %s ): unable to open file.\n", szFilename );
		return -1;
	}

	// Print the number of images in the file. Not used yet, but maybe in
	// the future.
	fprintf( fout, "%d\n", SPRITES_PER_SPRITESHEET );
	for ( int i=0; i<SPRITES_PER_SPRITESHEET; i++ )
	{
		// output block type
		fprintf( fout, "%d\n", m_type[i] );

		// output "extras" values
		for (int j=0; j<12; j++ )
		{
			if ( j<11 )
				fprintf( fout, "%d,", m_extras[i][j] );
			else
				fprintf( fout, "%d\n", m_extras[i][j] );
		} // j

		// output block color (for level editor) (DEPRECATED! Changed 2019-07 from m_color[i] to just output a zero.)
		fprintf( fout, "%d\n", 0);//Deprecated, but still save to file as it's part of the file format! Otherwise file loading won't work.//m_color[i] );
	} // i

	fclose( fout );

	return 0;
}

int CSpriteData::LoadSpriteImage()
{
	if (m_szImgFilename == nullptr)
	{
		SYS_Debug("CSpriteData::LoadSpriteImage failed: NULL image");
		return -1;
	}
	if (m_szImgFilename[0] == 0)
	{
		// Hm can this happen?
		SYS_Debug("CSpriteData::LoadSpriteImage failed: empty image filename");
#ifdef _DEBUG
		//assert(m_szImgFilename[0] != 0);
#endif
		return -1;
	}

	SYS_Debug ( "CSpriteData::LoadSpriteImage( %s ): Loading ...\n", m_szImgFilename );

	// If image already exists, delete it
	if (m_pImage)
	{
		djDestroyImageHWSurface(m_pImage);
		djDEL(m_pImage);
	}
#ifdef djSPRITE_AUTO_DROPSHADOWS
	if (m_pImageAutoShadow)
	{
		djDestroyImageHWSurface(m_pImageAutoShadow);
		djDEL(m_pImageAutoShadow);
	}
#endif

	int iRet = 0;

	// Attempt to load image file
	m_pImage = new djImage;
	if (m_pImage==NULL)
	{
		SYS_Debug("CSpriteData::LoadSpriteImage failed: Alloc image failed");// This is pretty unlikely to happen .. 'new' rarely fails
		return -1;
	}

		/*
		char buf[4096]={0};
#ifdef DATA_DIR
		snprintf(buf,sizeof(buf), "%s%s", DATA_DIR, m_szImgFilename );
#else
		snprintf(buf,sizeof(buf), "%s", m_szImgFilename );
#endif
		iRet = m_pImage->Load( buf );
		*/
	iRet = m_pImage->Load(djDATAPATHc(m_szImgFilename));

	//dj2019-07 shrink/resize sprites .. quick n dirty test .. to 'make as if' DG1 use 8x8 blocks instead of 16x16, for testing unhardcoded BLOCKW etc. ..
	/*
	if (BLOCKW==8 && m_pImage->Width()==256)
	{
		for (int y = 0; y < m_pImage->Height() / 2; ++y)
		{
			for (int x = 0; x < m_pImage->Width() / 2; ++x)
			{
				// Do quick 'n dirty nearest neighbor downsampling
				int pixel = m_pImage->GetPixel(x * 2, y * 2);
				m_pImage->PutPixel(x, y, pixel);
			}
		}
	}
	//*/

	djCreateImageHWSurface( m_pImage );

#ifdef djSPRITE_AUTO_DROPSHADOWS
	// dj2018-01 SUPER-EXPERIMENTALY EXPERIMENT [see livestreams of 12/13 Jan 2018]
	// Basically what we're doing here is, we examine the sprite image, and create
	// a second corresponding sprite image that looks like a 'shadow' (black image
	// with alpha mask in shape of image), and then in specific contexts, we can
	// render that under the sprite at a 1x1 offset from the main sprite, to draw
	// a simple drop-shadow effect 'under the sprite', e.g. see DRAW_SPRITEA_SHADOW
	// helper and g_bSpriteDropShadows setting.
	// [low]In theory later we can make this 'nicer' e.g. slightly smooth the dropshadow image somewhat
	m_pImageAutoShadow = new djImage;//( m_pImage->Width(), m_pImage->Height(), m_pImage->BPP() );
	// Create image of exact same width/height/pitch and pixel format.
	// Note using the djImage constructor didn't seem to work (fixmeFuture some issue there), maybe pitch issues or bpp issues, not sure,
	// but we had to use CreateImage() rather here (LOW prio but should look into why at
	// some stage, not a prio unless we want to make this whole thing more generic 'engine') [dj2018-01-12]
	m_pImageAutoShadow->CreateImage(m_pImage->Width(), m_pImage->Height(), m_pImage->BPP(), m_pImage->Pitch());
	for (int y=0;y<m_pImage->Height();++y)
	{
		for (int x=0;x<m_pImage->Width();++x)
		{
			const djColor c = m_pImage->GetPixelColor(x,y);
			// PutPixel: 32-bit hex value containing alpha red green blue 0xAARRGGBB
			if (c.a!=0)
				m_pImageAutoShadow->PutPixel(x,y,0x90000000);// <- Highest two hex digits adjust intensity/darkness of dropshadow
			else
				m_pImageAutoShadow->PutPixel(x,y,0x00000000);
		}
	}
	djCreateImageHWSurface( m_pImageAutoShadow );
#endif

	return iRet;
}
/*--------------------------------------------------------------------------*/
