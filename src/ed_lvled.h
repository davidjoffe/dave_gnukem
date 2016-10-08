
#ifndef __EDITOR_LEVEL_ED_H__
#define __EDITOR_LEVEL_ED_H__

#include "ed.h"



void LVLED_Init (int curr_level);
void LVLED_Kill ();


switch_e LVLED_MainLoop ();
bool LVLED_GetLevelFore ();


#endif		// #ifndef __EDITOR_LEVEL_ED_H__

