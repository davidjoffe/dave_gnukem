/*
Copyright (C) 1998-2004 David Joffe
License: GNU GPL Version 2
New level editor started: 1998/12/31
*/

#include "ed.h"
#include "ed_spred.h"
#include "ed_lvled.h"
#include "ed_common.h"
#include "ed_DrawBoxContents.h"
#include "sys_error.h"

#include <stdio.h>		// printf: DEBUG

#include "game.h"
//#include "level.h"

static switch_e		switch_to;


static void EditorMainLoop ();



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
void EditorMainLoop ()
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
				SPRED_Init ();
				SwitchMode ( SPRED_MainLoop () );
				SPRED_Kill ();
				break;
			}
			case SWITCH_LVLED:
			{
				LVLED_Init (GetCurrentLevel());

				SwitchMode ( LVLED_MainLoop () );
				LVLED_Kill ();
				PerLevelSetup();
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

