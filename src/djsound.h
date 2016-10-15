/*--------------------------------------------------------------------------*/
// djsound.h
//
// Sound interface
/*
Copyright (C) 1999-2002 David Joffe

License: GNU GPL Version 2 (*not* "later versions")
*/
/*--------------------------------------------------------------------------*/
#ifndef _DJSOUND_H_
#define _DJSOUND_H_
/*--------------------------------------------------------------------------*/

typedef unsigned int SOUND_HANDLE;
#define SOUNDHANDLE_INVALID ((unsigned int)~0)

extern bool bSoundEnabled;
// not all sound cards have a mixer
extern bool bHaveMixer;

extern int          djSoundInit();
extern void         djSoundDone();
extern void         djSoundEnable();
extern void         djSoundDisable();
extern bool         djSoundEnabled();
extern SOUND_HANDLE djSoundLoad( const char *szFilename );
extern bool         djSoundPlay( SOUND_HANDLE iHandle );


#endif
