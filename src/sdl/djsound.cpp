/*
djsound.cpp

Copyright (C) 1999-2018 David Joffe and Kent Mein
*/

#include "../djsound.h"
#include "../djstring.h"
#include "../djlog.h"
#include <SDL_audio.h>
#include <SDL_error.h>
#ifndef NOSOUND
#include <SDL_mixer.h>
#endif

#if defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__)
#include <stdlib.h>//Fixing malloc.h 'not found' error compiling on Mac [dj2016-10]
#else
#include <malloc.h>
#endif

//#define NOSOUND

//For now [dj2016-10] just make background music some relative percentage of the main volume (so background is a bit softer so as not to get too annoying) - later could add a separate setting for background music volume
const float fBACKGROUNDMUSIC_RELATIVE_VOLUME = 0.36f;

bool g_bSoundInit = false;
bool g_bSoundEnabled = true;
#ifndef NOSOUND
Mix_Chunk *sounds[255]={NULL};
#endif
int numsounds = 0;
int g_nVolume = 85;//[0..128] Default volume (don't have default volume at max, I think? dj2016-10) Note LibSDL Mixer MIX_MAX_VOLUME is 128.
/*--------------------------------------------------------------------------*/
//
// Common
//
/*--------------------------------------------------------------------------*/
void SetVolumeHelper()
{
#ifndef NOSOUND
	if (g_bSoundInit)
	{
	Mix_Volume(-1,g_bSoundEnabled ? g_nVolume : 0);
	Mix_VolumeMusic(g_bSoundEnabled ? (int)((float)g_nVolume * fBACKGROUNDMUSIC_RELATIVE_VOLUME) : 0);//For now [dj2016-10] just make background music some relative percentage of the main volume (so background is a bit softer so as not to get too annoying) - later could add a separate setting for background music volume
	}
#endif
}
/*--------------------------------------------------------------------------*/
void djSoundEnable()
{
	if (!g_bSoundEnabled)
	{
#ifndef NOSOUND
		Mix_ResumeMusic();
#endif
		g_bSoundEnabled = true;
		SetVolumeHelper();
	}
}

void djSoundDisable()
{
	if (g_bSoundEnabled)
	{
#ifndef NOSOUND
		Mix_PauseMusic();
#endif
		g_bSoundEnabled = false;
		SetVolumeHelper();
	}
}

bool djSoundEnabled()
{
	return g_bSoundEnabled;
}

int djSoundInit()
{
#ifndef NOSOUND
	//if (!g_bSoundInit)
	//{
	numsounds = 0;
	int audio_channels=2;
	if (Mix_OpenAudio(22050, AUDIO_S16, audio_channels, 1024) < 0) {
		fprintf(stderr,
			"Warning: Couldn't set 22050 Hz 16-bit audio\n- Reason : %s\n",
			SDL_GetError());
		djSoundDisable();
		return 0;
	}
	// This must be set before calling SetVolumeHelper() or volume will start at 100%
	g_bSoundInit = true;
	//dj2016-10 Adding ability to change volume
	SetVolumeHelper();
	//djSoundEnable();
	//}
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
	g_bSoundInit = false;
}

SOUND_HANDLE djSoundLoad( const char *szFilename )
{
#ifndef NOSOUND
	int i;
	// need to set i and alloc space for sounds[i]
	i = numsounds + 1;
	sounds[i] = (Mix_Chunk *)malloc(sizeof(Mix_Chunk));
	if (!(sounds[i] = Mix_LoadWAV(szFilename))) {
		djMSG("ERROR: SOUNDLOAD(%s): Unable to load sound", szFilename);
		//printf("ERROR: SOUNDLOAD(%s): Unable to load sound\n", szFilename);
		return SOUNDHANDLE_INVALID;
	}
	numsounds ++;

	return i;
#else
	return -1;
#endif
}

bool djSoundPlay( SOUND_HANDLE i )//, bool bLoop )
{
#ifndef NOSOUND
	if (i==SOUNDHANDLE_INVALID)return false;
	if (djSoundEnabled()) {
		// [dj2016-10] Changing this channel here from 0 to -1 (to auto-select 'next available channel') .. I think this should fix
		// the 'explosion sounds don't always play' issue, but this needs more testing etc. ... I think the issue with explosion
		// sounds not playing is this was always trying to play all sounds on channel 0 so if channel busy then sound doesn't play.
		Mix_PlayChannel(-1,sounds[i],0);
		//Mix_PlayChannel(bLoop?1:-1,sounds[i],bLoop ? -1 : 0);
		// while (*Mix_Playing(0)) { SDL_Delay(100); }
	}
#endif
	return true;
}

int djSoundGetVolume()
{
	return g_nVolume;
}

void djSoundSetVolume(int nVolume,bool bApply)
{
	if (g_nVolume!=nVolume)
	{
		g_nVolume = nVolume;
		if (g_nVolume<0)g_nVolume=0;
		if (g_nVolume>128)g_nVolume=128;

#ifndef NOSOUND
		if (bApply)
		{
			SetVolumeHelper();
		}
#endif
	}
}

bool djSoundAdjustVolume(int nDiff)
{
#ifndef NOSOUND
	int nVolumePrev = g_nVolume;
	g_nVolume += nDiff;
	if (g_nVolume<0)g_nVolume=0;
	if (g_nVolume>128)g_nVolume=128;//MIX_MAX_VOLUME
	if (nVolumePrev != g_nVolume)
	{
		SetVolumeHelper();
		return true;
	}
#endif
	return false;
}
