/*
djsound.cpp

Copyright (C) 1999-2022 David Joffe and Kent Mein
*/

#include "../config.h"
#include "../djsound.h"
#include "../djstring.h"
#include "../djlog.h"
#ifdef __OS2__
#include <SDL/SDL_audio.h>
#include <SDL/SDL_error.h>
#else
#include <SDL_audio.h>
#include <SDL_error.h>
#endif
#ifndef NOSOUND
#ifdef __OS2__
#include <SDL/SDL_mixer.h>
#else
#include <SDL_mixer.h>
#endif
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
	//dj2022-11 Fix music files not playing with SDL2.
	// Note that currently (as of writing this comment) this game only needs basically .wav and .ogg but I'm passing all flags
	// here in case someone wants to create new levels etc. or add more audio in other formats - but the SDL mixer library dynamically checks/loads support by checking
	// for presence of dynamic libraries (at least on Windows) so in theory it should work then as long as whatever game you create, you make sure to include
	// the associated libraries along with. So e.g. if using vcpkg to build libraries do e.g.:
	// "vcpkg search sdl2-mixer" and you can see which to build to support required/desired formats.
	int nRet = Mix_Init(
		MIX_INIT_FLAC|
		MIX_INIT_MOD |
		MIX_INIT_MP3 |
		MIX_INIT_OGG |
		MIX_INIT_MID |
		MIX_INIT_OPUS
	);
	// [dj2022-11] for now Dave Gnukem only using .wav and .ogg (I think?) though we should want to support more formats .. we say 'ERROR' for OGG support because it's definitely required for Dave Gnukem but for now the others are warnings
	if ((nRet & MIX_INIT_OGG) == 0)  djMSG("ERROR[sound]: djSoundInit: OGG support failed to initialize, some sounds may not play");
	if ((nRet & MIX_INIT_MP3) == 0)  djMSG("WARNING[sound]: djSoundInit: MP3 support failed to initialize");
	if ((nRet & MIX_INIT_FLAC) == 0) djMSG("WARNING[sound]: djSoundInit: FLAC support failed to initialize");
	if ((nRet & MIX_INIT_OPUS) == 0) djMSG("WARNING[sound]: djSoundInit: OPUS support failed to initialize");
	if ((nRet & MIX_INIT_OPUS) == 0) djMSG("WARNING[sound]: djSoundInit: OPUS support failed to initialize");
	if ((nRet & MIX_INIT_MID) == 0)  djMSG("WARNING[sound]: djSoundInit: MID support failed to initialize");

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
