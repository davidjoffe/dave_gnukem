/*
level.cpp

Copyright (C) 1995-2018 David Joffe
*/
/*--------------------------------------------------------------------------*/
#include "level.h"
#include "datadir.h"
#include "djtypes.h"
#include "djlog.h"
#include "sys_log.h"
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#ifdef _DEBUG
#include <assert.h>
#endif
/*--------------------------------------------------------------------------*/
#ifdef WIN32
#include "io.h"
#else
#include <unistd.h>
#endif

#ifdef WIN32
#define FILECREATE_FLAGS (O_CREAT | O_TRUNC | O_BINARY | O_RDWR)
#define FILECREATE_PERM  (S_IWRITE | S_IREAD)
#define FILEOPEN_FLAGS   (O_RDONLY | O_BINARY)
#else
#define FILECREATE_FLAGS (O_CREAT | O_TRUNC | O_RDWR)
#define FILECREATE_PERM  (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
#define FILEOPEN_FLAGS   (O_RDONLY)
#endif
/*--------------------------------------------------------------------------*/
vector<unsigned char *> apLevels;
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
	unsigned char* pRet = NULL;
	unsigned char * buffer;

	printf( "level_load( %s ): loading at slot %d.\n", szfilename, i );

	// if a level is already loaded at position i, delete it
	level_delete( i );

	char filename[4096]={0};

	sprintf( filename, "%s%s", DATA_DIR, szfilename );

	// open level file
	int file_handle = -1;
	if ((file_handle = open( filename, FILEOPEN_FLAGS )) == -1)
	{
		printf( "level_load( %s ): failed to open file.\n", filename );
		//return NULL;
	}

	// allocate memory for level
	if (NULL == (buffer = new unsigned char[LEVEL_SIZE]))
	{
		printf( "level_load( %s ): failed to allocate level buffer.\n", szfilename );
		return NULL;
	}
	// Initialize level to blank block by default (in case it doesn't load e.g. bad filename passed or whatever, don't want to sit with random memory contents) [dj2017-07]
	memset(buffer, 0, LEVEL_SIZE);
	if (file_handle != -1)
	{
		// read level into buffer
		read( file_handle, buffer, LEVEL_SIZE );
		// close file
		close( file_handle );

		pRet = buffer;
	}

	// FIXME: add to tail end incorrect-ness
	apLevels.insert(apLevels.begin() + i, buffer);

	return pRet;
}

int level_save( int i, const char * szfilename )
{
	unsigned char * level=NULL;
	char filename[4096]={0};

	sprintf( filename, "%s%s", DATA_DIR, szfilename );

	level = apLevels[i];
	if ( level == NULL )
	{
		djMSG( "level_save( %d, %s ): NULL level!\n", i, filename );
		return -1;
	}

	printf(" level_save( %s ): saving.\n", filename );

	// make sure level i is not NULL
	if (level == NULL)
	{
		printf( "level_save( %s ): levels[%d] is NULL.\n", filename, i );
		return -1;
	}

	// open level file (FIXME: TEST THIS STILL WORKS, I'VE AHCNAGEAD FLAGS)
	int file_handle=-1;
	if ((file_handle = open( filename, FILECREATE_FLAGS, FILECREATE_PERM )) == -1)
	{
		printf( "level_save( %s ): failed to open file.\n", filename );
		return -2;
	}

	// write level to file
	write( file_handle, level, LEVEL_SIZE );

	close( file_handle );

	return 0;
}

unsigned char * level_pointer( int i, int x, int y )
{
#ifdef _DEBUG
	assert(i<apLevels.size() && i>=0);// Debug-mode only for speed reasons .. this 
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
