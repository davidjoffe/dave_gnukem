/*
TODO:
	(-) Find out what's the buzz with `isprite'. If it is
	used only in SPRED, then it's ok, but if not, then the
	problem "what to do with common data" arises earlier than
	i hoped it will :))
*/
#include "config.h"
#include "ed_spred.h"
#include "ed_common.h"
#include "djinput.h"
#include "mission.h"		// g_pCurMission comes from here.
				// TODO: think of something to get
				// rid of that dependency on
				// g_pCurMission. Editors must
				// manage this by theirselves
#include "block.h"
#include "sys_error.h"

#include <stdio.h>		// sprintf (NB don't use sprintf anymore, use sprintfs etc.!)
#ifdef __OS2__
#include <SDL/SDL_timer.h>//dj2022-11 for SDL_Delay (which may change eg cf. emscripten issues) ..
#else
#include <SDL_timer.h>//dj2022-11 for SDL_Delay (which may change eg cf. emscripten issues) ..
#endif


#define POS_FLAGS ((POS_SPRITES_Y + 8 * 16) + 8)
#define POS_INSTRUCTIONS_X 420
#define POS_INSTRUCTIONS_Y (((12*16)+8)+48)
#define POS_BLOCKTYPES_X ((16*16)+40)
#define POS_BLOCKTYPES_Y 0

// also change in ed_common.cpp!!!!!!!!!!!!
#define POS_EXTRAS_X ((16*16)+40)
#define POS_EXTRAS_Y ((POS_BLOCKTYPES_Y + (TYPE_LASTONE+1) * 8) + 8)


#define NUMFLAGS 5
static const char *szFlags[NUMFLAGS] =
{
	"solid",
	"animated",
	"falls",
	"inventory",
	"persistent"
};




#define NUM_SPRITE_INSTRUCTIONS 12
static const char *sprite_instructions[NUM_SPRITE_INSTRUCTIONS] =
{
	"- Instructions: ----------",
	"Arrows  Select sprite",
	"L       Lift color",
	"X       Swap colors",
	"C       Copy",
	"P       Paste",
	"F1      Save sprites",
	"M       Next spriteset",
	"N       Previous spriteset",
	"F5      Level editor",
	"ESC     Quit",
	"--------------------------"
};





static void HandleMouse(bool bCtrl);
static void SpriteEd_RedrawView ();
static void SetSprite ( int new_sprite );
static void SpriteShowExtras();
static void SpriteDrawFlags();
static void ShowInstructions();
static void DrawSprites ();
static void ShowBlockTypes ();
static void SaveSprites ();
static void SpriteSetType( int itype );







void SPRED_Init ()
{
	ShowInstructions ();
	ShowBlockTypes ();
	DrawSprites ();
}



/*void SPRED_Kill ()
{
}
*/



switch_e SPRED_MainLoop ()
{
	bool	bRunning = true;

	while ( bRunning )
	{
		//dj2017-06 it's almost impossible to press left/right etc. only once .. going to try increase this delay
		unsigned long int delay = 100;
		// Try prevent CPU hogging a bit ..
//		SDL_Delay(10);

		SpriteEd_RedrawView ();

		djiPoll();

		if (g_iKeys[DJKEY_ESC])
			bRunning = false;

		HandleMouse(g_iKeys[DJKEY_CTRL]!=0);

		if (g_iKeys[DJKEY_F5])		// switch to LVLED
		{
			return SWITCH_LVLED;
//			state = STATE_LEVELEDITOR;
//			level_refreshdisplay();
		}
		if (g_iKeys[DJKEY_RIGHT])	// select next sprite
		{
			SetSprite( ED_GetCurrSprite() + 1 );
			SDL_Delay( delay );
		}
		if (g_iKeys[DJKEY_LEFT])	// select prev sprite
		{
			SetSprite( ED_GetCurrSprite() - 1 );
			SDL_Delay( delay );
		}
		if (g_iKeys[DJKEY_UP])		// select upper sprite
		{
			SetSprite( ED_GetCurrSprite() - 16 );
			SDL_Delay( delay );
		}
		if (g_iKeys[DJKEY_DOWN])	// select lower sprite
		{
			SetSprite( ED_GetCurrSprite() + 16 );
			SDL_Delay( delay );
		}
		if (djiKeyPressed(DJKEY_F1)) SaveSprites ();			// save changes
		if (djiKeyPressed(DJKEY_N))  ED_SetSpriteSet( ED_GetCurrSpriteSet() - 1 );	// select previous sprite set
		if (djiKeyPressed(DJKEY_M))  ED_SetSpriteSet( ED_GetCurrSpriteSet() + 1 );	// select next sprite set
	}
	ED_ClearScreen();
	return SWITCH_EXIT;
}



/*
===============
HandleMouse
===============
*/
void HandleMouse(bool bCtrl)
{
	static bool bLastL = false;
	static bool bLastR = false;


	int ax, ay;
	// inside the area for setting sprite type info?
	if (INBOUNDS( djMouse::x, djMouse::y,
		POS_BLOCKTYPES_X - 8,
		POS_BLOCKTYPES_Y,
		POS_BLOCKTYPES_X + 16*8,
		POS_BLOCKTYPES_Y + (TYPE_LASTONE+1) * 8 - 1))
	{
		if (djMouse::b & 1)
		{
			ay = (djMouse::y - POS_BLOCKTYPES_Y) / 8;
			SpriteSetType( ay );
		}
	}
	// inside the area for adjusting sprite extras info?
	else if (INBOUNDS( djMouse::x, djMouse::y,
		POS_EXTRAS_X,
		POS_EXTRAS_Y,
		POS_EXTRAS_X + 9 * 8 - 1,
		POS_EXTRAS_Y + 12 * 8 - 1 ))
	{
		ay = (djMouse::y - POS_EXTRAS_Y) / 8;
		if ((bCtrl || !bLastL) && (djMouse::b & 1))       // left button = decrement
		{
			ED_SetSpriteExtra( ED_GetCurrSpriteSet(), ED_GetCurrSprite(), ay,
				ED_GetSpriteExtra( ED_GetCurrSpriteSet(), ED_GetCurrSprite(), ay ) - 1 );
			//	 sprite_set_extra( ay, sprite_get_extra( ay ) - 1 );
			if ( ay == 4 )
				SpriteDrawFlags();
			else
				SDL_Delay( 10 );
		}
		else if ((bCtrl || !bLastR) && (djMouse::b & 2))  // right button = increment
		{
			ED_SetSpriteExtra( ED_GetCurrSpriteSet(), ED_GetCurrSprite(), ay,
				ED_GetSpriteExtra( ED_GetCurrSpriteSet(), ED_GetCurrSprite(), ay ) + 1 );
			//	 sprite_set_extra( ay, sprite_get_extra( ay ) + 1 );
			if ( ay == 4 )
				SpriteDrawFlags();
			else
				SDL_Delay( 10 );
		}
	}
	// inside the area for editting flags?
	else if (INBOUNDS( djMouse::x, djMouse::y, 0, POS_FLAGS, 10*8-1, POS_FLAGS+NUMFLAGS*8-1 ))
	{
		ay = (djMouse::y - POS_FLAGS) / 8;
		if ( djMouse::b & 1 ) // set flag
		{
			ED_SetSpriteExtra( ED_GetCurrSpriteSet(), ED_GetCurrSprite(), 4,
				ED_GetSpriteExtra( ED_GetCurrSpriteSet(), ED_GetCurrSprite(), 4 ) | (1 << ay) );
			//	 sprite_set_extra( 4, sprite_get_extra( 4 ) | (1 << ay) );
			SpriteDrawFlags();
		}
		else if ( djMouse::b & 2 ) // clear flag
		{
			ED_SetSpriteExtra( ED_GetCurrSpriteSet(), ED_GetCurrSprite(), 4,
				ED_GetSpriteExtra( ED_GetCurrSpriteSet(), ED_GetCurrSprite(), 4 ) & (~(1 << ay)) );
			//	 sprite_set_extra( 4, sprite_get_extra( 4 ) & (~(1 << ay)) );
			SpriteDrawFlags();
		}
	}
	// inside actual sprites area
	else if (INBOUNDS( djMouse::x, djMouse::y,
		POS_SPRITES_X, POS_SPRITES_Y,
		POS_SPRITES_X + 16 * 16 - 1,
		POS_SPRITES_Y + 16 * 8 - 1 ))
	{
		ax = (djMouse::x - POS_SPRITES_X) / 16;
		ay = (djMouse::y - POS_SPRITES_Y) / 16;
		if ( (djMouse::b & 1) || (djMouse::b & 2) )
			SetSprite( ay * 16 + ax );
	}

	bLastL = ((djMouse::b & 1)!=0);
	bLastR = ((djMouse::b & 2)!=0);
}


/*
===============
SpriteEd_RedrawView
===============
*/
void SpriteEd_RedrawView ()
{
	ED_ClearScreen ();
	ShowInstructions ();
	ShowBlockTypes ();
	DrawSprites ();
	ED_FlipBuffers ();
}




void SetSprite ( int new_sprite )
{
	int	ox, oy;

	ox = POS_SPRITES_X;
	oy = POS_SPRITES_Y;

	ED_SetSprite ( new_sprite, ox, oy );

	ED_SpriteShowType( false );// 10 ); (don't know or recall what "10" used to mean but doesn't seem to be used anymore, so cleaning up and simplifying - dj2022-11)
	SpriteShowExtras();// dj2022-11 don't know or recall right now what "11" used to mean here but doesn't seem to be used anymore so cleaning up and simplifying 11 );
	SpriteDrawFlags();
}



static void SpriteShowExtras()
{
	//dj2019-06 add these two help lines to try make it more self-documenting
	ED_DrawString(POS_EXTRAS_X,POS_EXTRAS_Y +12*8,"^ Sprite Metadata" );
	ED_DrawString(POS_EXTRAS_X,POS_EXTRAS_Y +13*8,"Left/right-click edits these values" );
	for ( int i=0; i<12; i++ )
	{
		ED_SpriteShowExtra ( i );
	}
}



/*
====================
SpriteDrawFlags

This one textouts a list of flags (attributes) that
belong to sprite.
====================
*/
void SpriteDrawFlags()
{
	int  i=0;
	unsigned char buf[128]={0}, c=0;
	for ( i=0; i<NUMFLAGS; i++ )
	{
		if (ED_GetSpriteExtra( ED_GetCurrSpriteSet(), ED_GetCurrSprite(), 4 ) & (1 << i))
			//      if (g_pSprites[ispriteset]->m_extras[isprite][4] & (1 << i))
			c = 'X';
		else
			c = 255; // solid black block
		snprintf( (char*)buf, sizeof(buf), "[ ] %s", szFlags[i] );
		buf[1] = c;

		ED_DrawString( 0, POS_FLAGS + i * 8, (char*)buf );
	}
}


/*
====================
ShowInstructions
====================
*/
void ShowInstructions()
{
	for ( int i=0; i<NUM_SPRITE_INSTRUCTIONS; i++ )
	{
		ED_DrawString( POS_INSTRUCTIONS_X, POS_INSTRUCTIONS_Y + i*8, sprite_instructions[i] );
	}
}






/*
====================
ShowBlocktypes
====================
*/
void ShowBlockTypes ()
{
	for ( int i=0; i<=(int)TYPE_LASTONE; i++ )
	{
		ED_DrawString( POS_BLOCKTYPES_X, POS_BLOCKTYPES_Y + i*8, block_type_names[i] );
	}
}




void SaveSprites ()
{
//	TRACE( "sprites_save()\n" );
	SYS_Debug ( "SaveSprites() called\n" );
	g_pCurMission->SaveSprites();
//	TRACE( "sprites_save(): finished\n" );
}



void SpriteSetType( int itype )
{
	if (itype == ED_GetSpriteType( ED_GetCurrSpriteSet(), ED_GetCurrSprite() ))
		return;
	ED_SpriteShowType( true );
	ED_SetSpriteType( ED_GetCurrSpriteSet(), ED_GetCurrSprite(), itype );
	ED_SpriteShowType( false );// 10 );
}



void DrawSprites ()
{
	int i=0;
	int ox=0, oy=0;
	int xoffset=0, yoffset=0;
	char buf[4096]={0};//possible long filenames?

//	djgSetColorFore( pVisMain, djColor(255,255,255) );
	snprintf( buf, sizeof(buf), "%d,%-15.15s", ED_GetCurrSpriteSet(), g_pCurMission->GetSpriteData(ED_GetCurrSpriteSet())->m_szImgFilename );

	ED_DrawString( 120, POS_SPRITES_Y - 8, buf );
	ox = POS_SPRITES_X;
	oy = POS_SPRITES_Y;

	for ( i=0; i<SPRITES_PER_SPRITESHEET; i++ )
	{
		xoffset = (i % SPRITESHEET_NUM_COLS) * BLOCKW;
		yoffset = (i / SPRITESHEET_NUM_COLS) * BLOCKH;
		ED_DrawSprite( ox + xoffset, oy + yoffset, ED_GetCurrSpriteSet(), i );
	}

	// make it redraw the pointers to the current sprite
	SetSprite ( ED_GetCurrSprite () );
}

