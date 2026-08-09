/*
djinput.cpp

Copyright (C) 1995-2024 David Joffe

Created: '95/07/20 (originally as a test keyboard interrupt hook)
*/

#include "../config.h"
//#include <stdlib.h>
#include <string.h>
#include "../djinput.h"
#include "../djlog.h"
#include <SDL3/SDL_timer.h>//dj2022-11 for SDL_Delay (which may change eg cf. emscripten issues) ..


int g_iKeys[DJKEY_MAX] = { 0 };
int g_iKeysLast[DJKEY_MAX] = { 0 };
/*--------------------------------------------------------------------------*/
//dj2022-11 slight refactoring to at least make these non-global, simple static class for now .. dj2022-11
int djMouse::x = 0;
int djMouse::y = 0;
int djMouse::b = 0;
/*--------------------------------------------------------------------------*/

/*
One might be wondering why there's an extra layer of 'dj' key codes that map to 'sdlk' (and the code sometimes uses one, other times uses directly SDL keycodes)?
One might ask why not just use SDL keycodes? Although my memory's getting fuzzy on the details, the main reason is the history
of the code: In '94/95 there was no LibSDL yet (it was released only later), and so the first versions of the code, these 'dj keycodes'
acted as a sort of wrapper between different implementations (e.g. DirectX on Windows, at some stage LibGGI on Linux etc.) and this
actually originated even earlier as an x86 assembler keyboard interrupt handler sometime around the mid-90s.
Long story short, when LibSDL came along it functioned *as* the cross-platform wrapper that this input key code stuff had effectively been
for this code, and so probably the 'DJKEYs' aren't really necessary anymore, and it's arguable could be replaced with direct SDLK codes in all or most places,
unless we ever plan to port to something else and want or need some other wrappers, it may be good to just try keep that in mind in the code design and try use
re-usability etc. and other good principles when doing input stuff.
(In the mid-90s there was no 'libSDL' yet, and even after the first SDL releases it was initially small and a while before it was clear it would 'take over' more widely eg https://discourse.libsdl.org/t/sdl-and-ggi/1258/2)
*/

// This structure maps SDL key codes to DJ key codes
SdjKeyMapping key_pairs[] =
{
	{ DJKEY_F1,        SDLK_F1 },
	{ DJKEY_F2,        SDLK_F2 },
	{ DJKEY_F3,        SDLK_F3 },
	{ DJKEY_F4,        SDLK_F4 },
	{ DJKEY_F5,        SDLK_F5 },
	{ DJKEY_F6,        SDLK_F6 },
	{ DJKEY_F7,        SDLK_F7 },
	{ DJKEY_F8,        SDLK_F8 },
	{ DJKEY_F9,        SDLK_F9 },
	{ DJKEY_F10,       SDLK_F10 },
	{ DJKEY_F11,       SDLK_F11 },
	{ DJKEY_F12,       SDLK_F12 },
	{ DJKEY_SPACE,     SDLK_SPACE },
	{ DJKEY_ENTER,     SDLK_RETURN },
	{ DJKEY_ESC,       SDLK_ESCAPE },
	{ DJKEY_LEFT,      SDLK_LEFT },
	{ DJKEY_RIGHT,     SDLK_RIGHT },
	{ DJKEY_UP,        SDLK_UP },
	{ DJKEY_PGUP,      SDLK_PAGEUP },
	{ DJKEY_PGDN,      SDLK_PAGEDOWN },
	{ DJKEY_DOWN,      SDLK_DOWN },
	{ DJKEY_HOME,      SDLK_HOME },
	{ DJKEY_END,       SDLK_END },
	{ DJKEY_ALT,       SDLK_RALT },
	{ DJKEY_ALT,       SDLK_LALT },
	{ DJKEY_CTRL,      SDLK_RCTRL },
	{ DJKEY_CTRL,      SDLK_LCTRL },
	{ DJKEY_A,         SDLK_A },
	{ DJKEY_A,         SDLK_A },
	{ DJKEY_B,         SDLK_B },
	{ DJKEY_B,         SDLK_B },
	{ DJKEY_C,         SDLK_C },
	{ DJKEY_C,         SDLK_C },
	{ DJKEY_D,         SDLK_D },
	{ DJKEY_D,         SDLK_D },
	{ DJKEY_E,         SDLK_E },
	{ DJKEY_E,         SDLK_E },
	{ DJKEY_F,         SDLK_F },
	{ DJKEY_F,         SDLK_F },
	{ DJKEY_G,         SDLK_G },
	{ DJKEY_G,         SDLK_G },
	{ DJKEY_H,         SDLK_H },
	{ DJKEY_H,         SDLK_H },
	{ DJKEY_I,         SDLK_I },
	{ DJKEY_I,         SDLK_I },
	{ DJKEY_J,         SDLK_J },
	{ DJKEY_J,         SDLK_J },
	{ DJKEY_K,         SDLK_K },
	{ DJKEY_K,         SDLK_K },
	{ DJKEY_L,         SDLK_L },
	{ DJKEY_L,         SDLK_L },
	{ DJKEY_M,         SDLK_M },
	{ DJKEY_M,         SDLK_M },
	{ DJKEY_N,         SDLK_N },
	{ DJKEY_N,         SDLK_N },
	{ DJKEY_O,         SDLK_O },
	{ DJKEY_O,         SDLK_O },
	{ DJKEY_P,         SDLK_P },
	{ DJKEY_P,         SDLK_P },
	{ DJKEY_Q,         SDLK_Q },
	{ DJKEY_Q,         SDLK_Q },
	{ DJKEY_R,         SDLK_R },
	{ DJKEY_R,         SDLK_R },
	{ DJKEY_S,         SDLK_S },
	{ DJKEY_S,         SDLK_S },
	{ DJKEY_T,         SDLK_T },
	{ DJKEY_T,         SDLK_T },
	{ DJKEY_U,         SDLK_U },
	{ DJKEY_U,         SDLK_U },
	{ DJKEY_V,         SDLK_V },
	{ DJKEY_V,         SDLK_V },
	{ DJKEY_W,         SDLK_W },
	{ DJKEY_W,         SDLK_W },
	{ DJKEY_X,         SDLK_X },
	{ DJKEY_X,         SDLK_X },
	{ DJKEY_Y,         SDLK_Y },
	{ DJKEY_Y,         SDLK_Y },
	{ DJKEY_Z,         SDLK_Z },
	{ DJKEY_Z,         SDLK_Z },
	{ DJKEY_0,         SDLK_0 },
	{ DJKEY_1,         SDLK_1 },
	{ DJKEY_2,         SDLK_2 },
	{ DJKEY_3,         SDLK_3 },
	{ DJKEY_4,         SDLK_4 },
	{ DJKEY_5,         SDLK_5 },
	{ DJKEY_6,         SDLK_6 },
	{ DJKEY_7,         SDLK_7 },
	{ DJKEY_8,         SDLK_8 },
	{ DJKEY_9,         SDLK_9 },
	{ DJKEY_BACKSPACE, SDLK_BACKSPACE },
	{ -1, 0 }
};

const char *GetKeyString(int nSDLKeyCode)
{
	switch (nSDLKeyCode)
	{
	case SDLK_BACKSPACE:	return "BACKSPACE";
	case SDLK_TAB:			return "TAB";
	case SDLK_CLEAR:		return "CLEAR";
	case SDLK_RETURN:		return "RETURN";
	case SDLK_PAUSE:		return "PAUSE";
	case SDLK_ESCAPE:		return "ESCAPE";
	case SDLK_SPACE:		return "SPACE";
	case SDLK_EXCLAIM:		return "EXCLAIM";
	case SDLK_DBLAPOSTROPHE:		return "QUOTEDBL";
	case SDLK_HASH:			return "#";
	case SDLK_DOLLAR:		return "$";
	case SDLK_AMPERSAND:	return "&";
	case SDLK_APOSTROPHE:		return "QUOTE";
	case SDLK_LEFTPAREN:	return "(";
	case SDLK_RIGHTPAREN:	return ")";
	case SDLK_ASTERISK:		return "*";
	case SDLK_PLUS:			return "+";
	case SDLK_COMMA:		return ",";
	case SDLK_MINUS:		return "-";
	case SDLK_PERIOD:		return ".";
	case SDLK_SLASH:		return "/";
	case SDLK_0:			return "0";
	case SDLK_1:			return "1";
	case SDLK_2:			return "2";
	case SDLK_3:			return "3";
	case SDLK_4:			return "4";
	case SDLK_5:			return "5";
	case SDLK_6:			return "6";
	case SDLK_7:			return "7";
	case SDLK_8:			return "8";
	case SDLK_9:			return "9";
	case SDLK_COLON:		return ":";
	case SDLK_SEMICOLON:	return ";";
	case SDLK_LESS:			return "<";
	case SDLK_EQUALS:		return "=";
	case SDLK_GREATER:		return ">";
	case SDLK_QUESTION:		return "?";
	case SDLK_AT:			return "@";
	case SDLK_LEFTBRACKET:	return "(";
	case SDLK_BACKSLASH:	return "\\";
	case SDLK_RIGHTBRACKET:	return ")";
	case SDLK_CARET:		return "^";
	case SDLK_UNDERSCORE:	return "_";
	case SDLK_GRAVE:	return "BACKQUOTE";
	case SDLK_A:			return "A";
	case SDLK_B:			return "B";
	case SDLK_C:			return "C";
	case SDLK_D:			return "D";
	case SDLK_E:			return "E";
	case SDLK_F:			return "F";
	case SDLK_G:			return "G";
	case SDLK_H:			return "H";
	case SDLK_I:			return "I";
	case SDLK_J:			return "J";
	case SDLK_K:			return "K";
	case SDLK_L:			return "L";
	case SDLK_M:			return "M";
	case SDLK_N:			return "N";
	case SDLK_O:			return "O";
	case SDLK_P:			return "P";
	case SDLK_Q:			return "Q";
	case SDLK_R:			return "R";
	case SDLK_S:			return "S";
	case SDLK_T:			return "T";
	case SDLK_U:			return "U";
	case SDLK_V:			return "V";
	case SDLK_W:			return "W";
	case SDLK_X:			return "X";
	case SDLK_Y:			return "Y";
	case SDLK_Z:			return "Z";
	case SDLK_DELETE:		return "DEL";
	case SDLK_KP_0:			return "KEYPAD0";
	case SDLK_KP_1:			return "KEYPAD1";
	case SDLK_KP_2:			return "KEYPAD2";
	case SDLK_KP_3:			return "KEYPAD3";
	case SDLK_KP_4:			return "KEYPAD4";
	case SDLK_KP_5:			return "KEYPAD5";
	case SDLK_KP_6:			return "KEYPAD6";
	case SDLK_KP_7:			return "KEYPAD7";
	case SDLK_KP_8:			return "KEYPAD8";
	case SDLK_KP_9:			return "KEYPAD9";
	case SDLK_KP_PERIOD:	return "KPPERIOD";
	case SDLK_KP_DIVIDE:	return "KPDIVIDE";
	case SDLK_KP_MULTIPLY:	return "KPMULTIPLY";
	case SDLK_KP_MINUS:		return "KPMINUS";
	case SDLK_KP_PLUS:		return "KPPLUS";
	case SDLK_KP_ENTER:		return "KPENTER";
	case SDLK_KP_EQUALS:	return "KPEQUALS";
	case SDLK_UP:			return "UP";
	case SDLK_DOWN:			return "DOWN";
	case SDLK_RIGHT:		return "RIGHT";
	case SDLK_LEFT:			return "LEFT";
	case SDLK_INSERT:		return "INS";
	case SDLK_HOME:			return "HOME";
	case SDLK_END:			return "END";
	case SDLK_PAGEUP:		return "PGUP";
	case SDLK_PAGEDOWN:		return "PGDN";
	case SDLK_F1:			return "F1";
	case SDLK_F2:			return "F2";
	case SDLK_F3:			return "F3";
	case SDLK_F4:			return "F4";
	case SDLK_F5:			return "F5";
	case SDLK_F6:			return "F6";
	case SDLK_F7:			return "F7";
	case SDLK_F8:			return "F8";
	case SDLK_F9:			return "F9";
	case SDLK_F10:			return "F10";
	case SDLK_F11:			return "F11";
	case SDLK_F12:			return "F12";
	case SDLK_F13:			return "F13";
	case SDLK_F14:			return "F14";
	case SDLK_F15:			return "F15";
	case SDLK_NUMLOCKCLEAR:		return "NUMLOCKCLEAR";
	case SDLK_CAPSLOCK:		return "CAPSLOCK";
	case SDLK_SCROLLLOCK:	return "SCROLLLOCK";
	case SDLK_RSHIFT:		return "RSHIFT";
	case SDLK_LSHIFT:		return "LSHIFT";
	case SDLK_RCTRL:		return "CTRL";
	case SDLK_LCTRL:		return "CTRL";
	case SDLK_RALT:			return "ALT";
	case SDLK_LALT:			return "ALT";
	case SDLK_RGUI:			return "RGUI";
	case SDLK_LGUI:			return "LGUI";
	case SDLK_MODE:			return "MODE";
	case SDLK_APPLICATION:		return "APPLICATION";
	case SDLK_HELP:			return "HELP";
	case SDLK_PRINTSCREEN:		return "PRINTSCREEN";
	case SDLK_SYSREQ:		return "SYSREQ";
	case SDLK_MENU:			return "MENU";
	case SDLK_POWER:		return "POWER";
	case SDLK_CURRENCYSUBUNIT:	return "CURRENCYSUBUNIT";
	default:
		return "UNKNOWN";
	}
}

/*--------------------------------------------------------------------------*/

bool djiPollEvents(SDL_Event &Event)
{
	if (SDL_PollEvent(&Event))
	{
		// [dj2016-10] Prevent distinguishing between left and right control/alt, i.e. treat both left-ctrl and right-ctrl as just 'ctrl'
		// and likewise for alt .. I think this makes for slightly more user-friendlier experience with default keys etc.
		if (Event.type==SDL_EVENT_KEY_DOWN||Event.type==SDL_EVENT_KEY_UP)
		{
			if (Event.key.key==SDLK_LCTRL)
				Event.key.key = SDLK_RCTRL;
			else if (Event.key.key==SDLK_LALT)
				Event.key.key = SDLK_RALT;
		}
		int i;
		// Handle some basic events
		switch (Event.type)
		{
		case SDL_EVENT_KEY_DOWN:
			for ( i=0; key_pairs[i].m_iScanCode!=-1; i++ )
			{
				if (Event.key.key==(int)key_pairs[i].m_iPlatformCode)
					g_iKeys[key_pairs[i].m_iScanCode] = 1;
			}
			break;
		case SDL_EVENT_KEY_UP:
			for ( i=0; key_pairs[i].m_iScanCode!=-1; i++ )
			{
			if (Event.key.key==(int)key_pairs[i].m_iPlatformCode)
					g_iKeys[key_pairs[i].m_iScanCode] = 0;
			}
			break;
		case SDL_EVENT_MOUSE_MOTION:
			djMouse::x = Event.motion.x;
			djMouse::y = Event.motion.y;
			break;
		case SDL_EVENT_MOUSE_BUTTON_DOWN:
			if (Event.button.button==1) djMouse::b |= 1;
			if (Event.button.button==3) djMouse::b |= 2;
			djMouse::x = Event.button.x;
			djMouse::y = Event.button.y;
			break;
		case SDL_EVENT_MOUSE_BUTTON_UP:
			if (Event.button.button==1) djMouse::b &= ~1;
			if (Event.button.button==3) djMouse::b &= ~2;
			djMouse::x = Event.button.x;
			djMouse::y = Event.button.y;
			break;
		}
		return true;
	}
	return false;
}

void djiPollBegin()
{
	// Remember the key state from last time round
	memcpy( g_iKeysLast, g_iKeys, sizeof(g_iKeys) );
}

void djiPollEnd()
{
}

void djiPoll()
{
	djiPollBegin();

	// Update input
	SDL_Event Event;
	//SDLMod ModState;
	//ModState = SDL_GetModState();
	while (SDL_PollEvent(&Event))
	{
		int i;
		switch (Event.type)
		{
		case SDL_EVENT_KEY_DOWN:
			for ( i=0; key_pairs[i].m_iScanCode!=-1; i++ )
			{
				if (Event.key.key==(int)key_pairs[i].m_iPlatformCode)
					g_iKeys[key_pairs[i].m_iScanCode] = 1;
			}
			break;
		case SDL_EVENT_KEY_UP:
			for ( i=0; key_pairs[i].m_iScanCode!=-1; i++ )
			{
				if (Event.key.key==(int)key_pairs[i].m_iPlatformCode)
					g_iKeys[key_pairs[i].m_iScanCode] = 0;
			}
			break;
		case SDL_EVENT_QUIT:
			break;
		case SDL_EVENT_MOUSE_MOTION:
			djMouse::x = Event.motion.x;
			djMouse::y = Event.motion.y;
			break;
		case SDL_EVENT_MOUSE_BUTTON_DOWN:
			if (Event.button.button==1) djMouse::b |= 1;
			if (Event.button.button==3) djMouse::b |= 2;
			djMouse::x = Event.button.x;
			djMouse::y = Event.button.y;
			break;
		case SDL_EVENT_MOUSE_BUTTON_UP:
			if (Event.button.button==1) djMouse::b &= ~1;
			if (Event.button.button==3) djMouse::b &= ~2;
			djMouse::x = Event.button.x;
			djMouse::y = Event.button.y;
			break;
		}
	}

	djiPollEnd();
}
/*--------------------------------------------------------------------------*/

bool djiInit()
{
	// Initialize mouse variables
	djMouse::b = 0;  // button
	djMouse::x = -1; // x
	djMouse::y = -1; // y

	return true;
}
/*--------------------------------------------------------------------------*/
void djiDone()
{
}

void djiWaitForKeyUp(unsigned char cKey)
{
	// FIXME: CPU KILLER
	do
	{
		djiPoll();
	} while (g_iKeys[cKey]);
}

bool djiKeyDown(int iScanCode)
{
	return g_iKeys[iScanCode]!=0;
}

bool djiKeyPressed(int iScanCode)
{
	return (g_iKeys[iScanCode]!=0 && g_iKeysLast[iScanCode]==0);
}

// by rtfb
void djiClearBuffer ()
{
	int	i;

	for ( i=0; i<DJKEY_MAX; i++ )
	{
		g_iKeys[i] = 0;
	}
}
