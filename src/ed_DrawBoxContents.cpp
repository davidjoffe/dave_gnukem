
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
#define DBC_POS_SPRITES_X 0
#define DBC_POS_SPRITES_Y 272
// !!!!!!!!

#define DBC_POS_LEVELSPRITES_X 0
#define DBC_POS_LEVELSPRITES_Y 336


static int dbc_ox = 0, dbc_oy = 0;
static int dbc_a = 0, dbc_b = 0;
static int dbc_sprite0a = 0, dbc_sprite0b = 0;




void DBC_DrawBoxContents ()
{
	dbc_a = ED_GetCurrSpriteSet ();		// FIXME: may be a bug in LVLED
	dbc_b = ED_GetCurrSprite ();

	int nType = ED_GetSpriteType ( dbc_a, dbc_b );
	if (nType==TYPE_BOX)
	{
		ED_DrawString(dbc_ox + 16*16 - 48, dbc_oy+16*8, "Box:");
		int c = ED_GetSpriteExtra(dbc_a, dbc_b, 10);
		int d = ED_GetSpriteExtra(dbc_a, dbc_b, 11);
		ED_DrawSprite(dbc_ox + 16*16 - 16, dbc_oy+16*8, c, d);
	}
	else
	{
		ED_DrawStringClear(dbc_ox + 16*16 - 48, dbc_oy+16*8, "Box:  ");
		ED_DrawStringClear(dbc_ox + 16*16 - 48, dbc_oy+16*8+8, "Box:  ");
	}
}



void DBC_SwitchMode ( switch_e mode )
{
printf ( "switching DBC mode...\n" );
	switch ( mode )
	{
		case SWITCH_SPRED:
		{
			dbc_ox = DBC_POS_SPRITES_X;
			dbc_oy = DBC_POS_SPRITES_Y;
			dbc_a = ED_GetCurrSpriteSet ();
			dbc_b = ED_GetCurrSprite ();
			break;
		}
		case SWITCH_LVLED:
		{
			dbc_ox = DBC_POS_LEVELSPRITES_X;
			dbc_oy = DBC_POS_LEVELSPRITES_Y;
			dbc_a = dbc_sprite0a;
			dbc_b = dbc_sprite0b;
			break;
		}
		case SWITCH_NONE:
		{
			break;
		}
	}
}


void DBC_Set0a0b ( int spr0a, int spr0b )
{
	dbc_sprite0a = spr0a;
	dbc_sprite0b = spr0b;
}

