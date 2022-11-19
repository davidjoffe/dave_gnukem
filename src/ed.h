
#ifndef __EDITOR_STUFF_H__
#define __EDITOR_STUFF_H__


enum switch_e
{
	SWITCH_NONE = 0,		// error
	//! Sprite editor:
	SWITCH_SPRED,
	//! Level editor:
	SWITCH_LVLED,
	SWITCH_EXIT,			// exit edit mode

	NUM_SWITCHES
};


void SwitchMode ( switch_e new_mode );
void ED_Main ();


#endif		// #ifndef __EDITOR_STUFF_H__

