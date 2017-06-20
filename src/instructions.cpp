// David Joffe, 1999/12 GNU etc.
/*
instructions.cpp

Copyright (C) 1999-2017 David Joffe

License: GNU GPL Version 2
*/
#include "djtypes.h"
#include "instructions.h"
#include "menu.h"
#include "graph.h"

djImage *pImgInstructions=NULL;

struct SMenuItem instructionsMenuItems[] =
{
   /*{ false, "{~~~~~~}" },
   { true,  "|  OK  |" },
   { false, "[~~~~~~]" },*/
   { false, "        " },
   { true,  "   OK   " },
   { false, "        " },
   { false, NULL }
};
unsigned char instructionsMenuCursor[] = { 128, 129, 130, 131, 0 };
CMenu instructionsMenu ( "instructions.cpp:instructionsMenu" );

void ShowInstructions()
{
	instructionsMenu.setSize(0);
	instructionsMenu.setItems(instructionsMenuItems);
	instructionsMenu.setMenuCursor(instructionsMenuCursor);
	instructionsMenu.setClrBack(djColor(48,66,128));
	instructionsMenu.setXOffset(220);
	instructionsMenu.setYOffset(136);
	if (pImgInstructions==NULL)
	{
		if (NULL != (pImgInstructions = new djImage))
		{
			pImgInstructions->Load( FILE_IMG_INSTRUCTIONS );
			djCreateImageHWSurface( pImgInstructions );
		}
	}
	if (pImgInstructions)
	{
		djgDrawImage( pVisBack, pImgInstructions, 0, 0, pImgInstructions->Width(), pImgInstructions->Height() );
		GraphFlip(true);

		// Pop up credits menu
		do_menu( &instructionsMenu );

		// [dj2017-06-20] Clean up / delete instructions image
		djDestroyImageHWSurface( pImgInstructions );
		djDEL(pImgInstructions);
	}
}
