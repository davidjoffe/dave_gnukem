/*
djtime.cpp

Copyright (C) 1999-2024 David Joffe
*/

#include "../config.h"
#include "../djtime.h"
#include <SDL3/SDL.h>

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

uint64_t djTimeGetTicks64()
{
	// Re "SDL_GetTicks":	*This function is not recommended as of SDL 2.0.18; use SDL_GetTicks64() instead, where the value doesn't wrap every ~49 days"
	return SDL_GetTicks();
}
