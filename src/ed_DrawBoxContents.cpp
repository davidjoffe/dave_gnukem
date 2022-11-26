
/*
TODO:
	(-) Find out what the hell is going on here with
	editor-dependancy. I know that it should draw box
	contents in different places for LVLED and SPRED,
	but that difference in what does it draw looks silly
	for me.

// dj2022-11 not sure anymore who wrote above question? not me but yes someday clean this stuff up a bit
*/

#include "config.h"
#include "ed_DrawBoxContents.h"
#include "ed_common.h"
#include "block.h"

#include <stdio.h>		// printf: DEBUG


// !!!!!!!!!!!!!!!
// if these two change, change them in ed_spred.cpp too!!!
// sprite editor position
#define DBC_POS_SPRITES_X 0
#define DBC_POS_SPRITES_Y 272
// !!!!!!!!

// level editor positon
#define DBC_POS_LEVELSPRITES_X 0
#define DBC_POS_LEVELSPRITES_Y 336

// drawboxcontents(?) 'offset x' and 'offset y' (may be different depending on whether we're in level editor or sprite editor)
int g_dbc_drawposx = 0, g_dbc_drawposy = 0;

void DBC_DrawBoxContents ()
{
	// sprite a and b values (a is the selected spriteset index, b is the selected sprite index in that spriteset)
	const int dbc_a = ED_GetCurrSpriteSet ();		// FIXME: may be a bug in LVLED
	const int dbc_b = ED_GetCurrSprite ();

	const int nType = ED_GetSpriteType ( dbc_a, dbc_b );
	if (nType==TYPE_BOX)
	{
		ED_DrawString(g_dbc_drawposx + 16*16 - 48, g_dbc_drawposy+16*8, "Box:");
		// If it's e.g. a box try show what's in the box ('c' and 'd' values which are analogous to the 'a' and 'b values above to reference the sprite info for what's in the box):
		const int c = ED_GetSpriteExtra(dbc_a, dbc_b, 10);
		const int d = ED_GetSpriteExtra(dbc_a, dbc_b, 11);
		ED_DrawSprite(g_dbc_drawposx + 16*16 - 16, g_dbc_drawposy+16*8, c, d);
	}
	else
	{
		ED_DrawStringClear(g_dbc_drawposx + 16*16 - 48, g_dbc_drawposy+16*8, "Box:  ");
		ED_DrawStringClear(g_dbc_drawposx + 16*16 - 48, g_dbc_drawposy+16*8+8, "Box:  ");
	}
}



void DBC_OnSwitchMode ( switch_e mode )
{
printf ( "switching editor mode...\n" );
	switch ( mode )
	{
		case SWITCH_SPRED:
		{
			printf ( "sprite editor...\n" );
			// set draw position (different if in sprite editor vs level editor) for this
			g_dbc_drawposx = DBC_POS_SPRITES_X;
			g_dbc_drawposy = DBC_POS_SPRITES_Y;
			break;
		}
		case SWITCH_LVLED:
		{
			printf ( "level editor...\n" );
			// set draw position (different if in sprite editor vs level editor) for this
			g_dbc_drawposx = DBC_POS_LEVELSPRITES_X;
			g_dbc_drawposy = DBC_POS_LEVELSPRITES_Y;
			break;
		}
		default:
			break;
	}
}
