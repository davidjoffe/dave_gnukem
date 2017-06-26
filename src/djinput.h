/*--------------------------------------------------------------------------*/
/* David Joffe '95/07/20 test key hook */
/* 1999/02 converting over from keys.h/cpp , trying to genericize */
/* 2001/07 converting to SDL */
/*
Copyright (C) 1995-2017 David Joffe

License: GNU GPL Version 2
*/
/*--------------------------------------------------------------------------*/
#ifndef _DJINPUT_H_
#define _DJINPUT_H_
/*--------------------------------------------------------------------------*/
#include "djgraph.h"

#define DJKEY_MAX 256
extern int g_iKeys[DJKEY_MAX];
extern int g_iKeysLast[DJKEY_MAX];

extern djVisual *g_pVisInput;

extern bool djiInit( djVisual *pVis, int iFlags );
extern void djiDone();
extern void djiPoll();
extern void djiPollBegin();
extern void djiPollEnd();
extern bool djiPollEvents(SDL_Event &Event);
extern void djiWaitForKeyUp(unsigned char cKey);
extern bool djiAnyKeyDown();
extern bool djiKeyDown(int iScanCode);
extern bool djiKeyPressed(int iScanCode);
void djiClearBuffer ();				// rtfb

// Use to indicate which events you want
// FIXME: DOES THIS MAKE SNESE? WE CAN ONLY CHECK KEY STATE! (Make event based?)
#define INPUT_MOUSE     1
#define INPUT_KEYDOWN   2
#define INPUT_KEYUP     4
#define INPUT_KEYREPEAT 8
#define INPUT_KEYBOARD (INPUT_KEYDOWN|INPUT_KEYUP|INPUT_KEYREPEAT)

//! Maps a platform-dependent key code to a DJ key scan code. This will quite likely
//! go now that SDL is in
struct SdjKeyMapping
{
	int          m_iScanCode;     // DJ scan code
	unsigned int m_iPlatformCode; // platform dependent scan code
};


extern int mouse_x;
extern int mouse_y;
extern int mouse_b;

extern const char *GetKeyString(int nSDLKeyCode);


// This struct is the "future" (on the way in)
// This struct is no longer the "future". Its on the way out.
enum EdjKeys
{
   DJKEY_ESC = 0,
   DJKEY_F1,
   DJKEY_F2,
   DJKEY_F3,
   DJKEY_F4,
   DJKEY_F5,
   DJKEY_F6,
   DJKEY_F7,
   DJKEY_F8,
   DJKEY_F9,
   DJKEY_F10,
   DJKEY_F11,
   DJKEY_F12,
   DJKEY_a,
   DJKEY_b,
   DJKEY_c,
   DJKEY_d,
   DJKEY_e,
   DJKEY_f,
   DJKEY_g,
   DJKEY_h,
   DJKEY_i,
   DJKEY_j,
   DJKEY_k,
   DJKEY_l,
   DJKEY_m,
   DJKEY_n,
   DJKEY_o,
   DJKEY_p,
   DJKEY_q,
   DJKEY_r,
   DJKEY_s,
   DJKEY_t,
   DJKEY_u,
   DJKEY_v,
   DJKEY_w,
   DJKEY_x,
   DJKEY_y,
   DJKEY_z,
   DJKEY_A,
   DJKEY_B,
   DJKEY_C,
   DJKEY_D,
   DJKEY_E,
   DJKEY_F,
   DJKEY_G,
   DJKEY_H,
   DJKEY_I,
   DJKEY_J,
   DJKEY_K,
   DJKEY_L,
   DJKEY_M,
   DJKEY_N,
   DJKEY_O,
   DJKEY_P,
   DJKEY_Q,
   DJKEY_R,
   DJKEY_S,
   DJKEY_T,
   DJKEY_U,
   DJKEY_V,
   DJKEY_W,
   DJKEY_X,
   DJKEY_Y,
   DJKEY_Z,
   DJKEY_ENTER,
   DJKEY_SPACE,
   DJKEY_UP,
   DJKEY_DOWN,
   DJKEY_LEFT,
   DJKEY_RIGHT,
   DJKEY_PGUP,
   DJKEY_PGDN,
   DJKEY_HOME,
   DJKEY_END,
   DJKEY_INS,
   DJKEY_DEL,
   DJKEY_GREYPLUS,
   DJKEY_MINUS,
   DJKEY_GREYMINUS,
   DJKEY_ALT,
   DJKEY_CTRL,
   DJKEY_0,
   DJKEY_1,
   DJKEY_2,
   DJKEY_3,
   DJKEY_4,
   DJKEY_5,
   DJKEY_6,
   DJKEY_7,
   DJKEY_8,
   DJKEY_9,
   DJKEY_BACKSPACE
};

#endif
