/*
level.cpp

Copyright (C) 1995-2023 David Joffe
*/
/*--------------------------------------------------------------------------*/
#include "level.h"
#include "djfile.h"
#include "datadir.h"
#include "djtypes.h"
#include "djlog.h"
#include "sys_log.h"
#include <stdio.h>
#include <string.h>
#ifdef _DEBUG
#include <assert.h>
#endif
/*--------------------------------------------------------------------------*/
std::vector<unsigned char *> apLevels;
/*--------------------------------------------------------------------------*/
void InitLevelSystem()
{
}

void KillLevelSystem()
{
	while ( apLevels.size() != 0 )
	{
		level_delete( 0 );
	}
}

// returns NULL on failure
unsigned char * level_load( int i, const char * szfilename )
{
	printf( "level_load( %s ): loading at slot %d.\n", szfilename, i );

	// if a level is already loaded at position i, delete it
	level_delete( i );

	std::string sFilename = djDATAPATHs(szfilename);

	// open level file
	FILE* pIn = NULL;
	pIn = djFile::dj_fopen(sFilename.c_str(), "rb");// NB! MUST BE BINARY MODE (on Windows anyway; Linux it does nothing) and for reading
	if (pIn == NULL)
	{
		printf( "level_load( %s ): failed to open file.\n", sFilename.c_str());
		//return NULL;
	}

	unsigned char* pRet = NULL;
	unsigned char* buffer = nullptr;

	// allocate memory for level
	unsigned int uMemSize = LEVEL_SIZE;
	if (NULL == (buffer = new unsigned char[uMemSize]))
	{
		printf( "level_load( %s ): failed to allocate level buffer.\n", sFilename.c_str());
		return NULL;
	}
	// Initialize level to blank block by default (in case it doesn't load e.g. bad filename passed or whatever, don't want to sit with random memory contents) [dj2017-07]
	memset(buffer, 0, uMemSize);
	if (pIn)
	{
		// read level into buffer
		size_t sizeRead = fread(buffer, 1, uMemSize, pIn);
		if (sizeRead < uMemSize)
		{
			printf("level_load( %s ): ERROR partial level read only. Read: %d bytes.\n", sFilename.c_str(), (int)sizeRead);
			//todo  add onscreen messages?
		}
		// close file
		fclose(pIn);
		pIn = NULL;

		pRet = buffer;
	}

	// FIXME: add to tail end incorrect-ness
	apLevels.insert(apLevels.begin() + i, buffer);

	return pRet;
}

int level_save( int i, const char * szfilename )
{
	std::string sFilename = djDATAPATHs(szfilename);

	unsigned char* level = apLevels[i];
	if ( level == NULL )
	{
		djMSG( "level_save( %d, %s ): NULL level!\n", i, sFilename.c_str());
		return -1;
	}

	printf(" level_save( %s ): saving.\n", sFilename.c_str());

	// make sure level i is not NULL
	if (level == NULL)
	{
		printf( "level_save( %s ): levels[%d] is NULL.\n", sFilename.c_str(), i );
		return -1;
	}

	// open level file (FIXME: TEST THIS STILL WORKS, I'VE AHCNAGEAD FLAGS)
	FILE* pFile = djFile::dj_fopen(sFilename.c_str(), "wb");// NB! MUST BE BINARY MODE (on Windows anyway; Linux it does nothing) and for reading
	if (pFile == NULL)
	{
		printf( "level_save( %s ): failed to open file.\n", sFilename.c_str());
		return -2;
	}

	// write level to file
	size_t sizeRet = fwrite(level, 1, LEVEL_SIZE, pFile);
	if (sizeRet < LEVEL_SIZE)
	{
		printf("level_save( %s ): ERROR partial level save: Only %d bytes.\n", sFilename.c_str(), (int)sizeRet);
	}

	fclose(pFile);

	return 0;
}

unsigned char * level_pointer( int i, int x, int y )
{
#ifdef _DEBUG
	assert(i<(int)apLevels.size() && i>=0);// Debug-mode only for speed reasons .. this 
#endif
	unsigned char * level = apLevels[i];
	if (level == NULL)
		return NULL;

	return level + LEVEL_BYTESPERBLOCK * (y * LEVEL_WIDTH + x);
}

SLevelBlock level_get_block( int i, int x, int y )
{
	SLevelBlock ret;
	memcpy( &ret, level_pointer( i, x, y ), 4 );
	return ret;
}

void level_delete( int i )
{
	if ( i >= (int)apLevels.size() )
		return;

	unsigned char * level = apLevels[i];
	djDELV(level);
	apLevels.erase(apLevels.begin() + i);
}
