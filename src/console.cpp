//Copyright (C) 1995-2022 David Joffe / Dave Gnukem project
//
//dj2022-11 just refactoring crude simple console message (or ingame message) stuff into own .h/cpp
//[low prio thoughts]: Not quite sure if this is a 'console' or a 'game message' and maybe in future there might be two distint things (in-game messages vs an actual console where e.g. one might type commands say)

/*--------------------------------------------------------------------------*/
#include "config.h"
#include "console.h"

//Very simple pseudo 'console message' [dj2016-10]
//dj2022-11 some small refactoring

std::string djConsoleMessage::m_sMsg;
float djConsoleMessage::m_fTimer = 0.f;

void djConsoleMessage::SetConsoleMessage(const std::string& sMsg)
{
	m_sMsg = sMsg;
	m_fTimer = 0.f;// Reset timer
}

void djConsoleMessage::Update(float fDeltaTime)
{
	if (m_sMsg.empty()) return;

	//Display message for about ~2s then it disappears
	m_fTimer += fDeltaTime;
	if (m_fTimer >= 2200.f)
	{
		m_sMsg.clear();
		m_fTimer = 0.f;
	}
}
