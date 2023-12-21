/*--------------------------------------------------------------------------*/
// djsound.h
//
// Sound interface
//
// Copyright (C) 1999-2023 David Joffe
/*--------------------------------------------------------------------------*/
#ifndef _DJSOUND_H_
#define _DJSOUND_H_
/*--------------------------------------------------------------------------*/

typedef unsigned int SOUND_HANDLE;
#define SOUNDHANDLE_INVALID ((unsigned int)~0)

//gross globals ..

extern int          djSoundInit();
extern void         djSoundDone();
extern void         djSoundEnable();
extern void         djSoundDisable();
extern bool         djSoundEnabled();
extern SOUND_HANDLE djSoundLoad( const char *szFilename );
extern bool         djSoundPlay( SOUND_HANDLE iHandle/*, bool bLoop=false*/ );

//! Get current volume [0..128]
extern int djSoundGetVolume();
//! Set current volume setting [0..128]
extern void djSoundSetVolume(int nVolume,bool bApply);
//! Return true if volume change occurred (for purposes of e.g. on-screen message display or whatever)
extern bool djSoundAdjustVolume(int nDiff);

#endif
