/*
keys.cpp

Copyright (C) 2001-2018 David Joffe

Created: 09/2001
*/

#include "keys.h"
#include "settings.h"
#include "SDL.h"
#include <vector>
using namespace std;

// Default keys
int g_anKeys[KEY_NUMKEYS] =
{
	SDLK_UP,
	SDLK_LEFT,
	SDLK_RIGHT,
	SDLK_RCTRL,//Note LCTRL and RCTRL are both 'mapped onto' just RCTRL currently [dj2016-10]
	SDLK_RALT//Note LALT and RALT are both 'mapped onto' just RALT currently [dj2016-10]
};

// Note here keycode is the SDL one, not the 'DJ' one [dj2016-10]
bool IsGameKeyAssigned(int nKeyCode)
{
	for ( unsigned int i=0; i<KEY_NUMKEYS; ++i )
	{
		if ( g_anKeys[i]==nKeyCode )
			return true;
	}
	return false;
}

// Key descriptions
const char *g_aszKeys[KEY_NUMKEYS] =
{
	"Action",
	"Left",
	"Right",
	"Jump",
	"Shoot"
};

vector<int> g_anValidGameKeys;

void StoreGameKeys()
{
	for ( int i=0; i<KEY_NUMKEYS; i++ )
	{
		char szKey[64]={0};
		sprintf(szKey, "Key%s", g_aszKeys[i]);
		g_Settings.SetSettingInt(szKey, g_anKeys[i]);
	}
}

void InitialiseGameKeySystem()
{
	// Set default keys (if not defined in settings already, e.g. if there was no config file)
	char szKey[64]={0};
	for ( int i=0; i<KEY_NUMKEYS; i++ )
	{
		sprintf(szKey, "Key%s", g_aszKeys[i]);
		g_Settings.SetDefaultSettingInt(szKey, g_anKeys[i]);
	}

	// Read key settings from config
	for ( int i=0; i<KEY_NUMKEYS; i++ )
	{
		sprintf(szKey, "Key%s", g_aszKeys[i]);
		g_anKeys[i] = g_Settings.FindSettingInt(szKey);
	}

	// FIXME: TODO: Trim this down, remove non-allowed keys
	g_anValidGameKeys.push_back(SDLK_BACKSPACE);
	g_anValidGameKeys.push_back(SDLK_TAB);
	g_anValidGameKeys.push_back(SDLK_CLEAR);
	g_anValidGameKeys.push_back(SDLK_RETURN);
	g_anValidGameKeys.push_back(SDLK_PAUSE);
	//g_anValidGameKeys.push_back(SDLK_ESCAPE);
	g_anValidGameKeys.push_back(SDLK_SPACE);
	g_anValidGameKeys.push_back(SDLK_EXCLAIM);
	g_anValidGameKeys.push_back(SDLK_QUOTEDBL);
	g_anValidGameKeys.push_back(SDLK_HASH);
	g_anValidGameKeys.push_back(SDLK_DOLLAR);
	g_anValidGameKeys.push_back(SDLK_AMPERSAND);
	g_anValidGameKeys.push_back(SDLK_QUOTE);
	g_anValidGameKeys.push_back(SDLK_LEFTPAREN);
	g_anValidGameKeys.push_back(SDLK_RIGHTPAREN);
	g_anValidGameKeys.push_back(SDLK_ASTERISK);
	g_anValidGameKeys.push_back(SDLK_PLUS);
	g_anValidGameKeys.push_back(SDLK_COMMA);
	g_anValidGameKeys.push_back(SDLK_MINUS);
	g_anValidGameKeys.push_back(SDLK_PERIOD);
	g_anValidGameKeys.push_back(SDLK_SLASH);
	g_anValidGameKeys.push_back(SDLK_0);
	g_anValidGameKeys.push_back(SDLK_1);
	g_anValidGameKeys.push_back(SDLK_2);
	g_anValidGameKeys.push_back(SDLK_3);
	g_anValidGameKeys.push_back(SDLK_4);
	g_anValidGameKeys.push_back(SDLK_5);
	g_anValidGameKeys.push_back(SDLK_6);
	g_anValidGameKeys.push_back(SDLK_7);
	g_anValidGameKeys.push_back(SDLK_8);
	g_anValidGameKeys.push_back(SDLK_9);
	g_anValidGameKeys.push_back(SDLK_COLON);
	g_anValidGameKeys.push_back(SDLK_SEMICOLON);
	g_anValidGameKeys.push_back(SDLK_LESS);
	g_anValidGameKeys.push_back(SDLK_EQUALS);
	g_anValidGameKeys.push_back(SDLK_GREATER);
	g_anValidGameKeys.push_back(SDLK_QUESTION);
	g_anValidGameKeys.push_back(SDLK_AT);
	g_anValidGameKeys.push_back(SDLK_LEFTBRACKET);
	g_anValidGameKeys.push_back(SDLK_BACKSLASH);
	g_anValidGameKeys.push_back(SDLK_RIGHTBRACKET);
	g_anValidGameKeys.push_back(SDLK_CARET);
	g_anValidGameKeys.push_back(SDLK_UNDERSCORE);
	g_anValidGameKeys.push_back(SDLK_BACKQUOTE);
	g_anValidGameKeys.push_back(SDLK_a);
	g_anValidGameKeys.push_back(SDLK_b);
	g_anValidGameKeys.push_back(SDLK_c);
	g_anValidGameKeys.push_back(SDLK_d);
	g_anValidGameKeys.push_back(SDLK_e);
	g_anValidGameKeys.push_back(SDLK_f);
	g_anValidGameKeys.push_back(SDLK_g);
	g_anValidGameKeys.push_back(SDLK_h);
	g_anValidGameKeys.push_back(SDLK_i);
	g_anValidGameKeys.push_back(SDLK_j);
	g_anValidGameKeys.push_back(SDLK_k);
	g_anValidGameKeys.push_back(SDLK_l);
	g_anValidGameKeys.push_back(SDLK_m);
	g_anValidGameKeys.push_back(SDLK_n);
	g_anValidGameKeys.push_back(SDLK_o);
	g_anValidGameKeys.push_back(SDLK_p);
	g_anValidGameKeys.push_back(SDLK_q);
	g_anValidGameKeys.push_back(SDLK_r);
	g_anValidGameKeys.push_back(SDLK_s);
	g_anValidGameKeys.push_back(SDLK_t);
	g_anValidGameKeys.push_back(SDLK_u);
	g_anValidGameKeys.push_back(SDLK_v);
	g_anValidGameKeys.push_back(SDLK_w);
	g_anValidGameKeys.push_back(SDLK_x);
	g_anValidGameKeys.push_back(SDLK_y);
	g_anValidGameKeys.push_back(SDLK_z);
	g_anValidGameKeys.push_back(SDLK_DELETE);
	g_anValidGameKeys.push_back(SDLK_KP_0);
	g_anValidGameKeys.push_back(SDLK_KP_1);
	g_anValidGameKeys.push_back(SDLK_KP_2);
	g_anValidGameKeys.push_back(SDLK_KP_3);
	g_anValidGameKeys.push_back(SDLK_KP_4);
	g_anValidGameKeys.push_back(SDLK_KP_5);
	g_anValidGameKeys.push_back(SDLK_KP_6);
	g_anValidGameKeys.push_back(SDLK_KP_7);
	g_anValidGameKeys.push_back(SDLK_KP_8);
	g_anValidGameKeys.push_back(SDLK_KP_9);
	g_anValidGameKeys.push_back(SDLK_KP_PERIOD);
	g_anValidGameKeys.push_back(SDLK_KP_DIVIDE);
	g_anValidGameKeys.push_back(SDLK_KP_MULTIPLY);
	g_anValidGameKeys.push_back(SDLK_KP_MINUS);
	g_anValidGameKeys.push_back(SDLK_KP_PLUS);
	g_anValidGameKeys.push_back(SDLK_KP_ENTER);
	g_anValidGameKeys.push_back(SDLK_KP_EQUALS);
	g_anValidGameKeys.push_back(SDLK_UP);
	g_anValidGameKeys.push_back(SDLK_DOWN);
	g_anValidGameKeys.push_back(SDLK_RIGHT);
	g_anValidGameKeys.push_back(SDLK_LEFT);
	g_anValidGameKeys.push_back(SDLK_INSERT);
	g_anValidGameKeys.push_back(SDLK_HOME);
	g_anValidGameKeys.push_back(SDLK_END);
	g_anValidGameKeys.push_back(SDLK_PAGEUP);
	g_anValidGameKeys.push_back(SDLK_PAGEDOWN);
	g_anValidGameKeys.push_back(SDLK_F1);
	g_anValidGameKeys.push_back(SDLK_F2);
	g_anValidGameKeys.push_back(SDLK_F3);
	g_anValidGameKeys.push_back(SDLK_F4);
	g_anValidGameKeys.push_back(SDLK_F5);
	g_anValidGameKeys.push_back(SDLK_F6);
	g_anValidGameKeys.push_back(SDLK_F7);
	g_anValidGameKeys.push_back(SDLK_F8);
	g_anValidGameKeys.push_back(SDLK_F9);
	g_anValidGameKeys.push_back(SDLK_F10);
	g_anValidGameKeys.push_back(SDLK_F11);
	g_anValidGameKeys.push_back(SDLK_F12);
	g_anValidGameKeys.push_back(SDLK_F13);
	g_anValidGameKeys.push_back(SDLK_F14);
	g_anValidGameKeys.push_back(SDLK_F15);
	//g_anValidGameKeys.push_back(SDLK_NUMLOCK);
	//g_anValidGameKeys.push_back(SDLK_CAPSLOCK);
	//g_anValidGameKeys.push_back(SDLK_SCROLLOCK);
	g_anValidGameKeys.push_back(SDLK_RSHIFT);
	g_anValidGameKeys.push_back(SDLK_LSHIFT);
	g_anValidGameKeys.push_back(SDLK_RCTRL);
	g_anValidGameKeys.push_back(SDLK_LCTRL);
	g_anValidGameKeys.push_back(SDLK_RALT);
	g_anValidGameKeys.push_back(SDLK_LALT);
	g_anValidGameKeys.push_back(SDLK_RGUI);
	g_anValidGameKeys.push_back(SDLK_LGUI);
	g_anValidGameKeys.push_back(SDLK_MODE);
	g_anValidGameKeys.push_back(SDLK_APPLICATION);
	g_anValidGameKeys.push_back(SDLK_HELP);
	g_anValidGameKeys.push_back(SDLK_PRINTSCREEN);
	//g_anValidGameKeys.push_back(SDLK_SYSREQ);
	//g_anValidGameKeys.push_back(SDLK_BREAK);
	g_anValidGameKeys.push_back(SDLK_MENU);
	//g_anValidGameKeys.push_back(SDLK_POWER);
	g_anValidGameKeys.push_back(SDLK_CURRENCYSUBUNIT);
}

bool IsGameKey(int nKeyCode)
{
	unsigned int i;
	for ( i=0; i<g_anValidGameKeys.size(); i++ )
	{
		if (nKeyCode==g_anValidGameKeys[i])
			return true;
	}
	return false;
}

