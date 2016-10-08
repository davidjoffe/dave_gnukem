/*
djsound.cpp

Copyright (C) 1999-2001 David Joffe and Kent Mein

License: GNU GPL Version 2 (*not* "later versions")
*/

#include "../djsound.h"
#include "../djstring.h"
#include "../djlog.h"
#include <SDL_audio.h>
#include <SDL_error.h>
#include <SDL_mixer.h>

#include <malloc.h>

//#define NOSOUND

bool bSoundEnabled = false;
bool bHaveMixer;
Mix_Chunk *sounds[255];
int numsounds = 0;
/*--------------------------------------------------------------------------*/
//
// Common
//
/*--------------------------------------------------------------------------*/
void djSoundEnable()
{
	bSoundEnabled = true;
}

void djSoundDisable()
{
	bSoundEnabled = false;
}

bool djSoundEnabled()
{
	return bSoundEnabled;
}

int djSoundInit()
{
#ifndef NOSOUND
	numsounds = 0;
	int audio_channels=2;
	if (Mix_OpenAudio(22050, AUDIO_S16, audio_channels, 1024) < 0) {
		fprintf(stderr,
			"Warning: Couldn't set 11025 Hz 8-bit audio\n- Reason : %s\n",
			SDL_GetError());
		djSoundDisable();
		return 0;
	}
	djSoundEnable();
#endif
	return 1;
}

void djSoundDone()
{
#ifndef NOSOUND
	int x, i;

	i=numsounds;
	for(x=0;x<i;x++) {
		Mix_FreeChunk(sounds[x]);
	}
	numsounds = 0;
#endif
}

SOUND_HANDLE djSoundLoad( char *szFilename )
{
#ifndef NOSOUND
	int i;
	// need to set i and alloc space for sounds[i]
	i = numsounds + 1;
	sounds[i] = (Mix_Chunk *)malloc(sizeof(Mix_Chunk));
	if (!(sounds[i] = Mix_LoadWAV(szFilename))) {
		djMSG("ERROR: SOUNDLOAD(%s): Unable to load sound", szFilename);
		return SOUNDHANDLE_INVALID;
	}
	numsounds ++;

	return i;
#else
	return -1;
#endif
}

bool djSoundPlay( SOUND_HANDLE i )
{
#ifndef NOSOUND
	if (djSoundEnabled()) {
		Mix_PlayChannel(0,sounds[i],0);
		// while (*Mix_Playing(0)) { SDL_Delay(100); }
	}
#endif
	return true;
}
