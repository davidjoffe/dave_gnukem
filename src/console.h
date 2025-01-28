//Copyright (C) 1995-2025 David Joffe / Dave Gnukem project
//
//dj2022-11 just refactoring crude simple console message (or ingame message) stuff into own .h/cpp
//[low prio thoughts]: Not quite sure if this is a 'console' or a 'game message' and maybe in future there might be two distint things (in-game messages vs an actual console where e.g. one might type commands say)

/*--------------------------------------------------------------------------*/
#ifndef _dj_CONSOLE_H_
#define _dj_CONSOLE_H_

#include "config.h"

//dj todo low - const char* rather? hmm debatable
#include <string>

//Very simple pseudo 'console message' [dj2016-10]
//dj2022-11 some small refactoring

class djConsoleMessage
{
public:
	static void SetConsoleMessage(const std::string& sMsg);
	// fDT = floating point deltatime [milliseconds]
	static void Update(float fDeltaTime);

	// Internal timer to help control how long the message will be displayed (in milliseconds)
	static float m_fTimer;
	static std::string m_sMsg;
};

#endif
