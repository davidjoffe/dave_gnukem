/*
keys.cpp

Copyright (C) 2001-2022 David Joffe

Created: 09/2001
*/

#include "keys.h"
#include "settings.h"
#ifdef __OS2__
#include <SDL/SDL.h>
#else
#include "SDL.h"
#endif
#include <vector>
#include <stdio.h>

// dj2022-11 [low prio] we might want to give porters more control over these default keys (some consolde platforms have needed to hardcode oddly specific things like 'j' etc. here, can't recall which right now but I saw it in a Dave Gnukem fork recently) ..
// Also to think about is that a slightly rude user reported to me that these keys can interfere with default keys for some window managers like MATE etc. apparently for things like desktop switching? Or something like that
// However, the reason these are the default keys is that these are the default keys on Duke Nukem 1, and though this is neither a clone nor a remake (more like a parody) of DN1 the point is to give some sense of retro familiarity and 'similar look and feel' of gameplay to original - so someone who played the original should feel immediately at home with the same keys.
// So I feel strongly the normal default keys should be as per below the same as DN1 except in specific 'edge cases' where it should or could maybe differ, i.e.:
//   Up: Action
//   Move left: Left
//   Move right: Right
//   Jump: Ctrl
//   Shoot: Alt
// And on major platforms like Windows these are anyway fine and shouldn't interfere with anything. But perhaps we could add some ability to add more 'default key profiles' (possibly also in e.g. small little config files porters could extend/add),
// and perhaps do detection if you are on a platform where the default keys may cause issues and make it easier for the user to select different default key profiles.
// But that's all low priority, I don't have time for that, but is good to think about as we refactor in future.

// Default keys
int g_anKeys[KEY_NUM_MAIN_REDEFINABLE_KEYS] =
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
	for ( unsigned int i=0; i<KEY_NUM_MAIN_REDEFINABLE_KEYS; ++i )
	{
		if ( g_anKeys[i]==nKeyCode )
			return true;
	}
	return false;
}

// Key descriptions
const char *g_aszKeys[KEY_NUM_MAIN_REDEFINABLE_KEYS] =
{
	"Action",
	"Left",
	"Right",
	"Jump",
	"Shoot"
};

// dj2022-11 low priority should probably use set or even better unordered_set for things like this (BUT not important right now it's just for redefining keys so not a bottleneck) but we might get rid of this later (see comments at IsGameKey)
std::vector<int> g_anValidGameKeys;

// dj: IMPORTANT NOTE FOR ~2022-11 SDL1 TO SDL2 IMPLEMENTATION:
// Previously the settings file stored the defined keys with keyname "Key" in front; however with SDL2 support this
// introduces a small bug where if you have already played the SDL1-based version it loads SDL1 scancode settings from the defined keys configuration file
// and then several keys e.g. left/right don't work because the scancodes changes. Simple fix, for SDL2 we now use a different setting name prefix
// and then e.g. whether you run SDL1 or SDL2 version it should in theory just load different sets of defined (or default) gameplay keys.

void StoreGameKeys()
{
	for ( int i=0; i<KEY_NUM_MAIN_REDEFINABLE_KEYS; i++ )
	{
		char szKey[128]={0};
		snprintf(szKey, sizeof(szKey), "SDL2Key%s", g_aszKeys[i]);
		g_Settings.SetSettingInt(szKey, g_anKeys[i]);
	}
}

void InitialiseGameKeySystem()
{
	// Set default keys (if not defined in settings already, e.g. if there was no config file)
	char szKey[128]={0};
	for ( int i=0; i<KEY_NUM_MAIN_REDEFINABLE_KEYS; i++ )
	{
		snprintf(szKey, sizeof(szKey), "SDL2Key%s", g_aszKeys[i]);
		g_Settings.SetDefaultSettingInt(szKey, g_anKeys[i]);
	}

	// Read key settings from config
	for ( int i=0; i<KEY_NUM_MAIN_REDEFINABLE_KEYS; i++ )
	{
		snprintf(szKey, sizeof(szKey), "SDL2Key%s", g_aszKeys[i]);
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

