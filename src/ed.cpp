/*
Copyright (C) 1998-2022 David Joffe
New level editor started: 1998/12/31
*/

#include "config.h"
#include "ed.h"
#include "ed_spred.h"
#include "ed_lvled.h"
#include "ed_common.h"
#include "ed_DrawBoxContents.h"
#include "sys_error.h"

#include <stdio.h>		// printf: DEBUG

#include "game.h"
//#include "level.h"

//dj2022-11 removing the 'static' here somehow seems to have fixed Visual Studio internal compiler error kept getting :/ probably a compiler bug
switch_e		switch_to = SWITCH_NONE;


void EditorMainLoop ();



void ED_Main ()
{
	printf ( "ED_Main() start\n" );		// DEBUG

	// init everything
	ED_CommonInit ();

	// jump to main looop
	EditorMainLoop ();

	// deinit everything
	ED_CommonKill ();

	printf ( "ED_Main() end\n" );		// DEBUG
}



/*
===============================
EditorMainLoop

Handles switches among different edit modes.
===============================
*/
void EditorMainLoop()
{
	while ( 1 )
	{
		switch ( switch_to )
		{
			case SWITCH_NONE:	// TODO: put some error message here
			{
				SYS_Error ( "EditorMainLoop: SWITCH_NONE hit!\n" );
				break;
			}
			case SWITCH_SPRED:
			{
				SPRED_Init();
				switch_e eNewMode = SPRED_MainLoop();
				SwitchMode( eNewMode );
				//SPRED_Kill();
				break;
			}
			case SWITCH_LVLED:
			{
				LVLED_Init(GetCurrentLevel());

				switch_e eNewMode = LVLED_MainLoop();
				SwitchMode(eNewMode);

				LVLED_Kill();

				//DJ2017-06-19 THERE'S SOME SORT OF SERIOUS BUG HERE .. this isn't right .. when we exit the level editor we immediately continue with that 'heartbeat'
				// processing, however, we've destroyed everything during PerLevelSetup(), so we're basically playing one frame out with corrupted/deleted
				// /dangling objects, or something along those lines. I'm not sure if that's directly the cause of the subsequent font corruption in
				// subsequent level editor invocations or if that is something separate/different. Need more testing.
				// We had a similar problem recently with the "Dying in the pungee sticks will often cause a crash" issue, which was fixed - I think
				// this is basically along the lines of the same sort of problem, so look at how we solved it there also.
				// dj2017-06-20 I think the above turned out to be a texture manager issue,
				// and should be fixed now.
				// However, I replaced this PerLevelSetup() with a call in the return to
				// RestartLevel(), which I think is probably more 'correct' anyway?
				//PerLevelSetup();
				break;
			}
			case SWITCH_EXIT:
			{
				return;
				break;		// for clarity :)
			}
		}
	}
}



void SwitchMode ( switch_e new_mode )
{
	switch_to = new_mode;
	DBC_SwitchMode ( new_mode );
}

