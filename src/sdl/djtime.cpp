/*
djtime.cpp

Copyright (C) 1999-2017 David Joffe

License: GNU GPL Version 2
*/

#include "../djtime.h"
#include "SDL.h"
#ifdef WIN32
#include <windows.h>
#include <mmsystem.h>
#endif
/*--------------------------------------------------------------------------*/
//
// SDL version
//
/*--------------------------------------------------------------------------*/
// Initialize the time system
void djTimeInit()
{
#ifdef WIN32
	// Windows NT / 2000 have a default timer resolution of 5 ms
	//timeBeginPeriod(1);
#endif
}

// Shut down the time system
void djTimeDone()
{
#ifdef WIN32
	//timeEndPeriod(1);
#endif
}

// return time of day in seconds
float djTimeGetTime()
{
	return (float)SDL_GetTicks() / 1000;
}

