/*
djinput.cpp

Copyright (C) 1995-2018 David Joffe

Created: '95/07/20 (originally as a test keyboard interrupt hook)
*/

#include <stdlib.h>
#include <string.h>
#include "../djinput.h"
#include "../djgraph.h"
#include "../djlog.h"


int g_iKeys[DJKEY_MAX] = { 0 };
int g_iKeysLast[DJKEY_MAX] = { 0 };

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
	{ DJKEY_A,         SDLK_a },
	{ DJKEY_A,         SDLK_a },
	{ DJKEY_B,         SDLK_b },
	{ DJKEY_B,         SDLK_b },
	{ DJKEY_C,         SDLK_c },
	{ DJKEY_C,         SDLK_c },
	{ DJKEY_D,         SDLK_d },
	{ DJKEY_D,         SDLK_d },
	{ DJKEY_E,         SDLK_e },
	{ DJKEY_E,         SDLK_e },
	{ DJKEY_F,         SDLK_f },
	{ DJKEY_F,         SDLK_f },
	{ DJKEY_G,         SDLK_g },
	{ DJKEY_G,         SDLK_g },
	{ DJKEY_H,         SDLK_h },
	{ DJKEY_H,         SDLK_h },
	{ DJKEY_I,         SDLK_i },
	{ DJKEY_I,         SDLK_i },
	{ DJKEY_J,         SDLK_j },
	{ DJKEY_J,         SDLK_j },
	{ DJKEY_K,         SDLK_k },
	{ DJKEY_K,         SDLK_k },
	{ DJKEY_L,         SDLK_l },
	{ DJKEY_L,         SDLK_l },
	{ DJKEY_M,         SDLK_m },
	{ DJKEY_M,         SDLK_m },
	{ DJKEY_N,         SDLK_n },
	{ DJKEY_N,         SDLK_n },
	{ DJKEY_O,         SDLK_o },
	{ DJKEY_O,         SDLK_o },
	{ DJKEY_P,         SDLK_p },
	{ DJKEY_P,         SDLK_p },
	{ DJKEY_Q,         SDLK_q },
	{ DJKEY_Q,         SDLK_q },
	{ DJKEY_R,         SDLK_r },
	{ DJKEY_R,         SDLK_r },
	{ DJKEY_S,         SDLK_s },
	{ DJKEY_S,         SDLK_s },
	{ DJKEY_T,         SDLK_t },
	{ DJKEY_T,         SDLK_t },
	{ DJKEY_U,         SDLK_u },
	{ DJKEY_U,         SDLK_u },
	{ DJKEY_V,         SDLK_v },
	{ DJKEY_V,         SDLK_v },
	{ DJKEY_W,         SDLK_w },
	{ DJKEY_W,         SDLK_w },
	{ DJKEY_X,         SDLK_x },
	{ DJKEY_X,         SDLK_x },
	{ DJKEY_Y,         SDLK_y },
	{ DJKEY_Y,         SDLK_y },
	{ DJKEY_Z,         SDLK_z },
	{ DJKEY_Z,         SDLK_z },
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
	case SDLK_QUOTEDBL:		return "QUOTEDBL";
	case SDLK_HASH:			return "#";
	case SDLK_DOLLAR:		return "$";
	case SDLK_AMPERSAND:	return "&";
	case SDLK_QUOTE:		return "QUOTE";
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
	case SDLK_BACKQUOTE:	return "BACKQUOTE";
	case SDLK_a:			return "A";
	case SDLK_b:			return "B";
	case SDLK_c:			return "C";
	case SDLK_d:			return "D";
	case SDLK_e:			return "E";
	case SDLK_f:			return "F";
	case SDLK_g:			return "G";
	case SDLK_h:			return "H";
	case SDLK_i:			return "I";
	case SDLK_j:			return "J";
	case SDLK_k:			return "K";
	case SDLK_l:			return "L";
	case SDLK_m:			return "M";
	case SDLK_n:			return "N";
	case SDLK_o:			return "O";
	case SDLK_p:			return "P";
	case SDLK_q:			return "Q";
	case SDLK_r:			return "R";
	case SDLK_s:			return "S";
	case SDLK_t:			return "T";
	case SDLK_u:			return "U";
	case SDLK_v:			return "V";
	case SDLK_w:			return "W";
	case SDLK_x:			return "X";
	case SDLK_y:			return "Y";
	case SDLK_z:			return "Z";
	case SDLK_DELETE:		return "DEL";
	case SDLK_WORLD_0:		return "WORLD0";
	case SDLK_WORLD_1:		return "WORLD1";
	case SDLK_WORLD_2:		return "WORLD2";
	case SDLK_WORLD_3:		return "WORLD3";
	case SDLK_WORLD_4:		return "WORLD4";
	case SDLK_WORLD_5:		return "WORLD5";
	case SDLK_WORLD_6:		return "WORLD6";
	case SDLK_WORLD_7:		return "WORLD7";
	case SDLK_WORLD_8:		return "WORLD8";
	case SDLK_WORLD_9:		return "WORLD9";
	case SDLK_WORLD_10:		return "WORLD10";
	case SDLK_WORLD_11:		return "WORLD11";
	case SDLK_WORLD_12:		return "WORLD12";
	case SDLK_WORLD_13:		return "WORLD13";
	case SDLK_WORLD_14:		return "WORLD14";
	case SDLK_WORLD_15:		return "WORLD15";
	case SDLK_WORLD_16:		return "WORLD16";
	case SDLK_WORLD_17:		return "WORLD17";
	case SDLK_WORLD_18:		return "WORLD18";
	case SDLK_WORLD_19:		return "WORLD19";
	case SDLK_WORLD_20:		return "WORLD20";
	case SDLK_WORLD_21:		return "WORLD21";
	case SDLK_WORLD_22:		return "WORLD22";
	case SDLK_WORLD_23:		return "WORLD23";
	case SDLK_WORLD_24:		return "WORLD24";
	case SDLK_WORLD_25:		return "WORLD25";
	case SDLK_WORLD_26:		return "WORLD26";
	case SDLK_WORLD_27:		return "WORLD27";
	case SDLK_WORLD_28:		return "WORLD28";
	case SDLK_WORLD_29:		return "WORLD29";
	case SDLK_WORLD_30:		return "WORLD30";
	case SDLK_WORLD_31:		return "WORLD31";
	case SDLK_WORLD_32:		return "WORLD32";
	case SDLK_WORLD_33:		return "WORLD33";
	case SDLK_WORLD_34:		return "WORLD34";
	case SDLK_WORLD_35:		return "WORLD35";
	case SDLK_WORLD_36:		return "WORLD36";
	case SDLK_WORLD_37:		return "WORLD37";
	case SDLK_WORLD_38:		return "WORLD38";
	case SDLK_WORLD_39:		return "WORLD39";
	case SDLK_WORLD_40:		return "WORLD40";
	case SDLK_WORLD_41:		return "WORLD41";
	case SDLK_WORLD_42:		return "WORLD42";
	case SDLK_WORLD_43:		return "WORLD43";
	case SDLK_WORLD_44:		return "WORLD44";
	case SDLK_WORLD_45:		return "WORLD45";
	case SDLK_WORLD_46:		return "WORLD46";
	case SDLK_WORLD_47:		return "WORLD47";
	case SDLK_WORLD_48:		return "WORLD48";
	case SDLK_WORLD_49:		return "WORLD49";
	case SDLK_WORLD_50:		return "WORLD50";
	case SDLK_WORLD_51:		return "WORLD51";
	case SDLK_WORLD_52:		return "WORLD52";
	case SDLK_WORLD_53:		return "WORLD53";
	case SDLK_WORLD_54:		return "WORLD54";
	case SDLK_WORLD_55:		return "WORLD55";
	case SDLK_WORLD_56:		return "WORLD56";
	case SDLK_WORLD_57:		return "WORLD57";
	case SDLK_WORLD_58:		return "WORLD58";
	case SDLK_WORLD_59:		return "WORLD59";
	case SDLK_WORLD_60:		return "WORLD60";
	case SDLK_WORLD_61:		return "WORLD61";
	case SDLK_WORLD_62:		return "WORLD62";
	case SDLK_WORLD_63:		return "WORLD63";
	case SDLK_WORLD_64:		return "WORLD64";
	case SDLK_WORLD_65:		return "WORLD65";
	case SDLK_WORLD_66:		return "WORLD66";
	case SDLK_WORLD_67:		return "WORLD67";
	case SDLK_WORLD_68:		return "WORLD68";
	case SDLK_WORLD_69:		return "WORLD69";
	case SDLK_WORLD_70:		return "WORLD70";
	case SDLK_WORLD_71:		return "WORLD71";
	case SDLK_WORLD_72:		return "WORLD72";
	case SDLK_WORLD_73:		return "WORLD73";
	case SDLK_WORLD_74:		return "WORLD74";
	case SDLK_WORLD_75:		return "WORLD75";
	case SDLK_WORLD_76:		return "WORLD76";
	case SDLK_WORLD_77:		return "WORLD77";
	case SDLK_WORLD_78:		return "WORLD78";
	case SDLK_WORLD_79:		return "WORLD79";
	case SDLK_WORLD_80:		return "WORLD80";
	case SDLK_WORLD_81:		return "WORLD81";
	case SDLK_WORLD_82:		return "WORLD82";
	case SDLK_WORLD_83:		return "WORLD83";
	case SDLK_WORLD_84:		return "WORLD84";
	case SDLK_WORLD_85:		return "WORLD85";
	case SDLK_WORLD_86:		return "WORLD86";
	case SDLK_WORLD_87:		return "WORLD87";
	case SDLK_WORLD_88:		return "WORLD88";
	case SDLK_WORLD_89:		return "WORLD89";
	case SDLK_WORLD_90:		return "WORLD90";
	case SDLK_WORLD_91:		return "WORLD91";
	case SDLK_WORLD_92:		return "WORLD92";
	case SDLK_WORLD_93:		return "WORLD93";
	case SDLK_WORLD_94:		return "WORLD94";
	case SDLK_WORLD_95:		return "WORLD95";
	case SDLK_KP0:			return "KEYPAD0";
	case SDLK_KP1:			return "KEYPAD1";
	case SDLK_KP2:			return "KEYPAD2";
	case SDLK_KP3:			return "KEYPAD3";
	case SDLK_KP4:			return "KEYPAD4";
	case SDLK_KP5:			return "KEYPAD5";
	case SDLK_KP6:			return "KEYPAD6";
	case SDLK_KP7:			return "KEYPAD7";
	case SDLK_KP8:			return "KEYPAD8";
	case SDLK_KP9:			return "KEYPAD9";
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
	case SDLK_NUMLOCK:		return "NUMLOCK";
	case SDLK_CAPSLOCK:		return "CAPSLOCK";
	case SDLK_SCROLLOCK:	return "SCROLLOCK";
	case SDLK_RSHIFT:		return "RSHIFT";
	case SDLK_LSHIFT:		return "LSHIFT";
	case SDLK_RCTRL:		return "CTRL";
	case SDLK_LCTRL:		return "CTRL";
	case SDLK_RALT:			return "ALT";
	case SDLK_LALT:			return "ALT";
	case SDLK_RMETA:		return "RMETA";
	case SDLK_LMETA:		return "LMETA";
	case SDLK_LSUPER:		return "LSUPER";
	case SDLK_RSUPER:		return "RSUPER";
	case SDLK_MODE:			return "MODE";
	case SDLK_COMPOSE:		return "COMPOSE";
	case SDLK_HELP:			return "HELP";
	case SDLK_PRINT:		return "PRINT";
	case SDLK_SYSREQ:		return "SYSREQ";
	case SDLK_BREAK:		return "BREAK";
	case SDLK_MENU:			return "MENU";
	case SDLK_POWER:		return "POWER";
	case SDLK_EURO:			return "EURO";
	default:
		return "UNKNOWN";
	}
}

/*--------------------------------------------------------------------------*/
djVisual *g_pVisInput = NULL;
int           g_iFlags = 0;
int           mouse_x = 0;
int           mouse_y = 0;
int           mouse_b = 0;
/*--------------------------------------------------------------------------*/

bool djiPollEvents(SDL_Event &Event)
{
	if (SDL_PollEvent(&Event))
	{
		// [dj2016-10] Prevent distinguishing between left and right control/alt, i.e. treat both left-ctrl and right-ctrl as just 'ctrl'
		// and likewise for alt .. I think this makes for slightly more user-friendlier experience with default keys etc.
		if (Event.type==SDL_KEYDOWN||Event.type==SDL_KEYUP)
		{
			if (Event.key.keysym.sym==SDLK_LCTRL)
				Event.key.keysym.sym = SDLK_RCTRL;
			else if (Event.key.keysym.sym==SDLK_LALT)
				Event.key.keysym.sym = SDLK_RALT;
		}
		int i;
		// Handle some basic events
		switch (Event.type)
		{
		case SDL_KEYDOWN:
			for ( i=0; key_pairs[i].m_iScanCode!=-1; i++ )
			{
				if (Event.key.keysym.sym==(int)key_pairs[i].m_iPlatformCode)
					g_iKeys[key_pairs[i].m_iScanCode] = 1;
			}
			break;
		case SDL_KEYUP:
			for ( i=0; key_pairs[i].m_iScanCode!=-1; i++ )
			{
				if (Event.key.keysym.sym==(int)key_pairs[i].m_iPlatformCode)
					g_iKeys[key_pairs[i].m_iScanCode] = 0;
			}
			break;
		case SDL_MOUSEMOTION:
			mouse_x = Event.motion.x;
			mouse_y = Event.motion.y;
			break;
		case SDL_MOUSEBUTTONDOWN:
			if (Event.button.button==1) mouse_b |= 1;
			if (Event.button.button==3) mouse_b |= 2;
			mouse_x = Event.button.x;
			mouse_y = Event.button.y;
			break;
		case SDL_MOUSEBUTTONUP:
			if (Event.button.button==1) mouse_b &= ~1;
			if (Event.button.button==3) mouse_b &= ~2;
			mouse_x = Event.button.x;
			mouse_y = Event.button.y;
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
		case SDL_KEYDOWN:
			for ( i=0; key_pairs[i].m_iScanCode!=-1; i++ )
			{
				if (Event.key.keysym.sym==(int)key_pairs[i].m_iPlatformCode)
					g_iKeys[key_pairs[i].m_iScanCode] = 1;
			}
			break;
		case SDL_KEYUP:
			for ( i=0; key_pairs[i].m_iScanCode!=-1; i++ )
			{
				if (Event.key.keysym.sym==(int)key_pairs[i].m_iPlatformCode)
					g_iKeys[key_pairs[i].m_iScanCode] = 0;
			}
			break;
		case SDL_QUIT:
			break;
		case SDL_MOUSEMOTION:
			mouse_x = Event.motion.x;
			mouse_y = Event.motion.y;
			break;
		case SDL_MOUSEBUTTONDOWN:
			if (Event.button.button==1) mouse_b |= 1;
			if (Event.button.button==3) mouse_b |= 2;
			mouse_x = Event.button.x;
			mouse_y = Event.button.y;
			break;
		case SDL_MOUSEBUTTONUP:
			if (Event.button.button==1) mouse_b &= ~1;
			if (Event.button.button==3) mouse_b &= ~2;
			mouse_x = Event.button.x;
			mouse_y = Event.button.y;
			break;
		}
	}

	djiPollEnd();
}
/*--------------------------------------------------------------------------*/

bool djiInit( djVisual *pVis, int iFlags )
{
	g_iFlags = iFlags;

	g_pVisInput = pVis;

	// Initialize mouse variables
	mouse_b = 0;  // button
	mouse_x = -1; // x
	mouse_y = -1; // y

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

bool djiAnyKeyDown()
{
	int i,r=0;
	for (i = 0;i<128;i++)
	{
		r = r | g_iKeys[i];
	}
	return(r!=0);
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
