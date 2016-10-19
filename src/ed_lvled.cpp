
#include "ed_lvled.h"
#include "ed_common.h"
#include "ed_macros.h"
#include "djinput.h"
#include "graph.h"
#include "mission.h"		// g_pCurMission comes from here.
				// TODO: think of something to get
				// rid of that dependency on
				// g_pCurMission. Editors must
				// manage this by theirselves
#include "level.h"	// level_pointer() comes from here
#include "ed_DrawBoxContents.h"




// !!!!!!!!!!!
// If these change, change them in ed_DrawBocContents too!!!
#define POS_LEVELSPRITES_X 0
#define POS_LEVELSPRITES_Y 336
// !!!!!!!!!

#define POS_LEVELVIEW_X (390-16*7)
#define POS_LEVELVIEW_Y 2
#define LEVEL_VIEW_WIDTH 22
#define LEVEL_VIEW_HEIGHT 16



static int	levelview_x = 0;	// these two are for representing
static int	levelview_y = 0;	// a minimap.
static int	levelview_w = LEVEL_VIEW_WIDTH;	// these two are for representing
static int	levelview_h = LEVEL_VIEW_HEIGHT;	// a minimap.
static bool	bShowBack = true;	// toggles display of background blocks (the ones that could be trated as 'world', not the entities)
static bool	bLevelFore = true;//false;	// toggles drawing of foreground layer
static int	g_nLevel = 0;		// points to current level.
static int	sprite0a = 0;
static int	sprite0b = 0;
static int	sprite1a = 0;
static int	sprite1b = 0;


#define NUM_LEVEL_INSTRUCTIONS 16
static const char *level_instructions[NUM_LEVEL_INSTRUCTIONS] =
{
	"- Instructions: ----------",
	"1-9     Place macros 1-9",
	"Arrows  Move",
	"Alt+Clk MoveTo",
	"Ctl+Alt+Clk Start-Level-At",
	"F4      Sprite editor",
	"X       Toggle foreground",
	"Z       Toggle background",
	"F1      Save level",
	"F       Horizontal fill",
	"M       Next spriteset",
	"N       Previous spriteset",
	"^C+M    Next level",
	"^C+N    Previous level",
	"ESC     Quit",
	"--------------------------"
};


bool HandleMouse();
//static void SetSprite ( int new_sprite );
static void MoveMinimap( int ox, int oy );
static void DrawMinimapRectangle();
static void RedrawView ();
static void DrawSprites ();
static void DrawGrid( int x, int y, int w, int h, int nx, int ny, djColor clr );
static void DrawLevelGrid();
static void DrawLevelname();
static void DrawMinimap();
static void ShowInstructions();
static void DrawSpritesel ();
static void SetSpriteSelection( int a0, int b0, int a1, int b1 );
static void SelectLevel ( int i );
static void LevelFill( int ax, int ay );

void SetLevel( int x, int y, int a, int b, bool bforeground );


// <Unrefined>
//void level_fill( int ax, int ay );	// what does this do??
//void level_set( int x, int y, int a, int b, bool bforeground );		// // in ed.cpp
//void level_draw_spritesel();
//void level_set_spritesel( int a0, int b0, int a1, int b1 );
//void level_show_instructions();
// </Unrefined>

#define LEVEL_GRIDSIZE (2)

#define DRAW_LEVEL_PIXEL(x, y) \
{\
	djgSetColorFore( pVisMain, ED_GetSpriteColor( *(level_pointer( 0, (x), (y) ) + 2) , *(level_pointer( 0, (x), (y) ) + 3) ) );\
	djgDrawBox( pVisMain, (x)*LEVEL_GRIDSIZE, (y)*LEVEL_GRIDSIZE, LEVEL_GRIDSIZE, LEVEL_GRIDSIZE );\
}



void LVLED_Init (int curr_level)
{
	SelectLevel(curr_level);

	//load the level from disk to get a clean copy.
	const char * szfilename = g_pCurMission->GetLevel( g_nLevel )->GetFilename( );
	level_load( 0, szfilename );

	// [dj2016-10] Calculate how much screen area we can devote to minimap, so we can take advantage of
	// modern larger screens for a much friendlier level editing experience.
	levelview_w = (pVisMain->width - POS_LEVELVIEW_X) / 16;
	levelview_h = (pVisMain->height - (NUM_LEVEL_INSTRUCTIONS*8) - POS_LEVELVIEW_Y) / 16;
	// [dj2016-10] If we have a really high-res screen, theoretically this box could be larger than the whole level -
	// if that happens to happen, clip to the max level W/H (don't want this box being larger than the map):
	levelview_w = djMIN(levelview_w, LEVEL_WIDTH);
	levelview_h = djMIN(levelview_h, LEVEL_HEIGHT);
}



void LVLED_Kill ()
{
}



switch_e LVLED_MainLoop ()
{
	//fixme hogs cpu ... but looking at below, maybe not trivial to fix without affecting usability? [dj2016-10]
	//int	i;
	bool	bRunning = true;
	while ( bRunning )
	{
		unsigned long delay = 10;

		RedrawView ();
		djiPoll();

		if (g_iKeys[DJKEY_ESC])
			bRunning = false;

		if (!HandleMouse ())
			bRunning = false;
		static bool bflagleft = false;
		static bool bflagright = false;
		static bool bflagup = false;
		static bool bflagdown = false;
		if (g_iKeys[DJKEY_LEFT])
		{
			bflagleft = true;
			MoveMinimap( levelview_x - 1, levelview_y );
			SDL_Delay( delay );
		}
		if ( (g_iKeys[DJKEY_LEFT] == 0) && (bflagleft == true) )
		{
			bflagleft = false;
			MoveMinimap( levelview_x - 1, levelview_y );
		}

		if (g_iKeys[DJKEY_RIGHT])
		{
			bflagright = true;
			MoveMinimap( levelview_x + 1, levelview_y );
			SDL_Delay( delay );
		}
		if ( (g_iKeys[DJKEY_RIGHT] == 0) && (bflagright == true) )
		{
			bflagright = false;
			MoveMinimap( levelview_x + 1, levelview_y );
		}

		if (g_iKeys[DJKEY_UP])
		{
			bflagup = true;
			MoveMinimap( levelview_x, levelview_y - 1 );
			SDL_Delay( delay );
		}
		if ( (g_iKeys[DJKEY_UP] == 0) && (bflagup == true) )
		{
			bflagup = false;
			MoveMinimap( levelview_x, levelview_y - 1 );
		}

		if (g_iKeys[DJKEY_DOWN])
		{
			bflagdown = true;
			MoveMinimap( levelview_x, levelview_y + 1 );
			SDL_Delay( delay );
		}
		if ( (g_iKeys[DJKEY_DOWN] == 0) && (bflagdown == true) )
		{
			bflagdown = false;
			MoveMinimap( levelview_x, levelview_y + 1);
		}


		if ( (mouse_x >= 0) && (mouse_y >= 0) )
		{
// TODO: no level filling at this point. Put it back here whenever possible.
			if (g_iKeys[DJKEY_F])
			{
				// if mouse in main level view
				if (INBOUNDS( mouse_x, mouse_y, 0, 0, 128*LEVEL_GRIDSIZE-1, 100*LEVEL_GRIDSIZE-1 ))
				{
					LevelFill( mouse_x / LEVEL_GRIDSIZE, mouse_y / LEVEL_GRIDSIZE );
				}
				// if mouse in zoom level view
				else if (
						INBOUNDS( mouse_x, mouse_y,
					POS_LEVELVIEW_X, POS_LEVELVIEW_Y,
					POS_LEVELVIEW_X + levelview_w * 16 - 1,
					POS_LEVELVIEW_Y + levelview_h * 16 - 1 ))
				{
					LevelFill(
						levelview_x + (mouse_x-POS_LEVELVIEW_X) / 16,
						levelview_y + (mouse_y-POS_LEVELVIEW_Y) / 16 );
				}
			}
/**/
		}

		// If the mouse is inside the main level view, do some extra checking
		if (INBOUNDS( mouse_x, mouse_y,
			POS_LEVELVIEW_X, POS_LEVELVIEW_Y,
			POS_LEVELVIEW_X + levelview_w * 16 - 1,
			POS_LEVELVIEW_Y + levelview_h * 16 - 1 ))
		{
			int iLevelX = levelview_x + (mouse_x-POS_LEVELVIEW_X) / 16;
			int iLevelY = levelview_y + (mouse_y-POS_LEVELVIEW_Y) / 16;
			if (g_iKeys[DJKEY_1]) PlaceMacro( iLevelX, iLevelY, 0 );
			if (g_iKeys[DJKEY_2]) PlaceMacro( iLevelX, iLevelY, 1 );
			if (g_iKeys[DJKEY_3]) PlaceMacro( iLevelX, iLevelY, 2 );
			if (g_iKeys[DJKEY_4]) PlaceMacro( iLevelX, iLevelY, 3 );
			if (g_iKeys[DJKEY_5]) PlaceMacro( iLevelX, iLevelY, 4 );
			if (g_iKeys[DJKEY_6]) PlaceMacro( iLevelX, iLevelY, 5 );
			if (g_iKeys[DJKEY_7]) PlaceMacro( iLevelX, iLevelY, 6 );
			if (g_iKeys[DJKEY_8]) PlaceMacro( iLevelX, iLevelY, 7 );
			if (g_iKeys[DJKEY_9]) PlaceMacro( iLevelX, iLevelY, 8 );
		} // if (mouse inside main view)

		if (djiKeyPressed(DJKEY_F1))
		{
// TODO: no saving at this point. Reenable it as soon as possible.
			//for ( i=0; i<g_pCurMission->NumLevels(); i++ )
			//{
				const char * szfilename;
				szfilename = g_pCurMission->GetLevel(g_nLevel)->GetFilename();
				level_save( 0, szfilename );
			//}
		}
		if (g_iKeys[DJKEY_F4])		// switch off the LVLED
		{
			return SWITCH_SPRED;
		}
		if (djiKeyPressed(DJKEY_X))
		{
			bLevelFore = !bLevelFore;
			if (bLevelFore)
			{
				ED_DrawString( 48, 308, "[FOREGROUND]" );
			}
			else
			{
				ED_DrawStringClear( 48, 308, "[FOREGROUND]" );
			}
			DrawMinimap ();
		}
		//-- Z - Toggle display of background blocks
		if (djiKeyPressed(DJKEY_Z))
		{
			bShowBack = !bShowBack;
			DrawMinimap ();
		}
		if (g_iKeys[DJKEY_CTRL])
		{
// TODO: no level selection at this point
/*			if (djiKeyPressed(DJKEY_N))
			{
				SelectLevel( g_nLevel - 1 );
			}
			if (djiKeyPressed(DJKEY_M))
			{
				SelectLevel( g_nLevel + 1 );
			}
*/
		}
		else
		{
			if (djiKeyPressed(DJKEY_N))
			{
				ED_SetCurrSpriteSet( ED_GetCurrSpriteSet() - 1 );
				if (ED_GetCurrSpriteSet() == -1)
					ED_SetCurrSpriteSet(255);
				while (g_pCurMission->GetSpriteData(ED_GetCurrSpriteSet()) == NULL)
				{
					if (ED_GetCurrSpriteSet() == -1)
						ED_SetCurrSpriteSet(255);
					else
						ED_SetCurrSpriteSet( ED_GetCurrSpriteSet() - 1 );
				}
			}
			if (djiKeyPressed(DJKEY_M))
			{
				ED_SetCurrSpriteSet( ED_GetCurrSpriteSet() + 1 );
				while (g_pCurMission->GetSpriteData(ED_GetCurrSpriteSet()) == NULL)
				{
					if (ED_GetCurrSpriteSet() == 255)
						ED_SetCurrSpriteSet(0);
					else
						ED_SetCurrSpriteSet( ED_GetCurrSpriteSet() + 1 );
				}
			}
		}
	} // while (bRunning)

	ED_ClearScreen();

	return SWITCH_EXIT;
}



void SetSprite ( int new_sprite )
{
	int	ox, oy;

	ox = POS_LEVELSPRITES_X;
	oy = POS_LEVELSPRITES_Y;

	ED_SetSprite ( new_sprite, ox, oy );
}


/*
=================
HandleMouse

TODO: write the bastard
=================
*/
bool HandleMouse ()
{
	int ax=0, ay=0;

	// sprites area
	if (INBOUNDS( mouse_x, mouse_y,
		POS_LEVELSPRITES_X, POS_LEVELSPRITES_Y,
		POS_LEVELSPRITES_X + 16 * 16 - 1,
		POS_LEVELSPRITES_Y + 16 * 8 - 1 ))
	{
		ax = (mouse_x - POS_LEVELSPRITES_X) / 16;
		ay = (mouse_y - POS_LEVELSPRITES_Y) / 16;
		if (mouse_b & 1)
			SetSpriteSelection( ED_GetCurrSpriteSet(), ay * 16 + ax, sprite1a, sprite1b );
		else if (mouse_b & 2)
			SetSpriteSelection( sprite0a, sprite0b, ED_GetCurrSpriteSet(), ay * 16 + ax );
	}
	// main editting area
	else if (INBOUNDS( mouse_x, mouse_y, 0, 0, 128 * LEVEL_GRIDSIZE - 1, 100 * LEVEL_GRIDSIZE - 1 ))
	{
		ax = mouse_x / LEVEL_GRIDSIZE;
		ay = mouse_y / LEVEL_GRIDSIZE;
		if (g_iKeys[DJKEY_ALT])
		{
			if (mouse_b & 1)
				MoveMinimap( ax - (levelview_w / 2),
//				level_move_view( ax - (levelview_w / 2),
				ay - (levelview_h / 2) );
		}
		else if (mouse_b & 1)
		{
			SetLevel( ax, ay, sprite0a, sprite0b, bLevelFore );
		}
		else if (mouse_b & 2)
		{
			SetLevel( ax, ay, sprite1a, sprite1b, bLevelFore );
		}
	}
	// zoomed view area
	else if (INBOUNDS( mouse_x, mouse_y,
		POS_LEVELVIEW_X, POS_LEVELVIEW_Y,
		POS_LEVELVIEW_X + levelview_w * 16 - 1,
		POS_LEVELVIEW_Y + levelview_h * 16 - 1 ))
	{
		ax = (mouse_x - POS_LEVELVIEW_X) / 16;
		ay = (mouse_y - POS_LEVELVIEW_Y) / 16;
		// dj2016-10: Hold in Ctrl+Alt and click with the mouse to automatically start level with hero 'dropped in' to the clicked position as starting position (to help with level editing / testing)
		if ((mouse_b & 1)!=0 && g_iKeys[DJKEY_CTRL]!=0 && g_iKeys[DJKEY_ALT]!=0)
		{
			extern int g_nOverrideStartX;
			extern int g_nOverrideStartY;

			g_nOverrideStartX = levelview_x+ax;
			g_nOverrideStartY = levelview_y+ay;
			return false;
		}
		else if (mouse_b & 1)
		{
			SetLevel( ax + levelview_x, ay + levelview_y,
				sprite0a, sprite0b, bLevelFore );
		}
		else if (mouse_b & 2)
		{
			SetLevel( ax + levelview_x, ay + levelview_y,
				sprite1a, sprite1b, bLevelFore );
		}
	}
	return true;
}



/*
=================
MoveMinimap
=================
*/
static void MoveMinimap( int ox, int oy )
{
	if ( ( ox == levelview_x ) && ( oy == levelview_y ) ) // Hasn't moved?
		return;

	// Set new level view offset, checking bounds.
	levelview_x = djCLAMP(ox, 0, 128 - levelview_w);
	levelview_y = djCLAMP(oy, 0, 100 - levelview_h);

	// Redraw the purple rectangle
	DrawMinimapRectangle();
}





void RedrawView ()
{
	ED_ClearScreen();
	ShowInstructions();
	if (bLevelFore)
		ED_DrawString( 48, 308, "[FOREGROUND]" );
	else
		ED_DrawStringClear( 48, 308, "[FOREGROUND]" );
	DrawSprites();
	DrawGrid( 0, 0, LEVEL_GRIDSIZE, LEVEL_GRIDSIZE, 128, 100, djColor(60,60,60) );
	DrawLevelGrid();
	DrawSpritesel();
	DrawMinimap();
	DrawLevelname();
	ShowMacros();
	ED_FlipBuffers ();
}



void DrawSprites ()
{
	int i;
	int ox, oy;
	int xoffset, yoffset;
	char buf[1024];

//	djgSetColorFore( pVisMain, djColor(255,255,255) );

	sprintf( buf, "%d,%-15.15s", ED_GetCurrSpriteSet(), g_pCurMission->GetSpriteData(ED_GetCurrSpriteSet())->m_szImgFilename );

	ED_DrawString( 120, POS_LEVELSPRITES_Y - 8, buf );
	ox = POS_LEVELSPRITES_X;
	oy = POS_LEVELSPRITES_Y;

	for ( i=0; i<128; i++ )
	{
		xoffset = (i%16)*16;
		yoffset = (i/16)*16;
		ED_DrawSprite( ox + xoffset, oy + yoffset, ED_GetCurrSpriteSet(), i );
	}

	// make it redraw the pointers to the current sprite
	SetSprite ( ED_GetCurrSprite () );
}


void DrawGrid( int x, int y, int w, int h, int nx, int ny, djColor clr )
{
	djgSetColorFore( pVisMain, clr );
	int i;
	for ( i=0; i<=ny; i++ )
	{
		djgDrawHLine( pVisMain, x, y+i*h, w*nx+1 );
	}
	for ( i=0; i<=nx; i++ )
	{
		djgDrawVLine( pVisMain, x+i*w, y, h*ny+1 );
	}
}



void DrawLevelGrid()
{
	int y, x;
	for ( y=0; y<100; y++ )
	{
		for ( x=0; x<128; x++ )
		{
			DRAW_LEVEL_PIXEL(x, y);
		}
	}
}


void DrawLevelname()
{
	const char * szfilename = g_pCurMission->GetLevel(g_nLevel)->GetFilename();
	ED_DrawStringClear( 0, POS_LEVELVIEW_Y + 16 * levelview_h + 8,
		"                           " );
	ED_DrawString( 0, POS_LEVELVIEW_Y + 16 * levelview_h + 8,
		szfilename );
}

void DrawMinimapRectangle()
{
	djgSetColorFore( pVisMain, djColor(255,0,255) );
	djgDrawRectangle( pVisMain,
		levelview_x * LEVEL_GRIDSIZE,
		levelview_y * LEVEL_GRIDSIZE,
		(levelview_w-1) * LEVEL_GRIDSIZE + LEVEL_GRIDSIZE,
		(levelview_h-1) * LEVEL_GRIDSIZE + LEVEL_GRIDSIZE );
}

void DrawMinimap()
{
	int i, j;
	int a, b;

	// Draw the purple rectangle indicating where your zoomed view is
	DrawMinimapRectangle();
	// Draw the zoomed view
	for ( i=0; i<levelview_h; i++ )
	{
		for ( j=0; j<levelview_w; j++ )
		{
			a = *(level_pointer( 0, levelview_x + j, levelview_y + i) + 2);
			b = *(level_pointer( 0, levelview_x + j, levelview_y + i) + 3);
			// Draw a black block if ShowBack is disabled
			if (!bShowBack)
			{
				djgSetColorFore( pVisMain, djColor(0,0,0) );
				djgDrawBox( pVisMain, POS_LEVELVIEW_X + j*16, POS_LEVELVIEW_Y + i*16, 16, 16 );
			}
			else
			{
				ED_DrawSprite( POS_LEVELVIEW_X + j * 16,
					POS_LEVELVIEW_Y + i * 16,
					a, b );
			}
			if (bLevelFore)
			{
				a = *(level_pointer( 0, levelview_x + j, levelview_y + i) + 0);
				b = *(level_pointer( 0, levelview_x + j, levelview_y + i) + 1);
				if ((a) || (b))
					ED_DrawSprite( POS_LEVELVIEW_X + j * 16,
					POS_LEVELVIEW_Y + i * 16,
					a, b );
			}
		}
	}

	// If hold in Ctrl+Alt, draw hero overlay at mouse cursor position to indicate the new hero 'drop-in-level-here' functionality [dj2016-10]
	if (INBOUNDS( mouse_x, mouse_y,
		POS_LEVELVIEW_X, POS_LEVELVIEW_Y,
		POS_LEVELVIEW_X + levelview_w * 16 - 1,
		POS_LEVELVIEW_Y + levelview_h * 16 - 1 ))
	{
		int ax = (mouse_x - POS_LEVELVIEW_X) / 16;
		int ay = (mouse_y - POS_LEVELVIEW_Y) / 16;
		if (g_iKeys[DJKEY_CTRL]!=0 && g_iKeys[DJKEY_ALT]!=0)
		{
			// These sprite offsets etc. are horribly hardcoded [show hero sprite]
			if (ay>0)
			{
				ED_DrawSprite( POS_LEVELVIEW_X + (ax-1) * 16+8, POS_LEVELVIEW_Y + (ay-1) * 16, 4, 16 );
				ED_DrawSprite( POS_LEVELVIEW_X + (ax  ) * 16+8, POS_LEVELVIEW_Y + (ay-1) * 16, 4, 17 );
			}
			ED_DrawSprite( POS_LEVELVIEW_X + (ax-1) * 16+8, POS_LEVELVIEW_Y + ay * 16, 4, 18 );
			ED_DrawSprite( POS_LEVELVIEW_X + (ax  ) * 16+8, POS_LEVELVIEW_Y + ay * 16, 4, 19 );
		}
	}
}



void ShowInstructions()
{
	//[dj20126-10] fixme[low/future]: Font width/height should be gotten from the font object, not hardcoded all over the place
	const unsigned int FONT_HEIGHT = 8;
	const unsigned int FONT_WIDTH = 8;
	
	// [dj2016-10] Instructions in bottom right of screen
	unsigned int uMaxStrLen = 0;
	for ( int i=0; i<NUM_LEVEL_INSTRUCTIONS; ++i )//Get longest string width, to calculate this things width
	{
		uMaxStrLen = djMAX(uMaxStrLen,strlen(level_instructions[i]));
	}
	unsigned int uOffsetX = pVisMain->width - uMaxStrLen * FONT_WIDTH;
	unsigned int uOffsetY = pVisMain->height - NUM_LEVEL_INSTRUCTIONS * FONT_HEIGHT;
	djgSetColorFore( pVisMain, djColor(42,57,112) );
	djgDrawBox( pVisMain, uOffsetX, uOffsetY, uMaxStrLen*FONT_WIDTH, NUM_LEVEL_INSTRUCTIONS*FONT_HEIGHT );
	for ( int i=0; i<NUM_LEVEL_INSTRUCTIONS; i++ )
	{
		ED_DrawString(
			uOffsetX,
			uOffsetY + i * FONT_HEIGHT,
			level_instructions[i] );
	}
}



void DrawSpritesel ()
{
	ED_DrawSprite(  4, 308, sprite0a, sprite0b );
	ED_DrawSprite( 24, 308, sprite1a, sprite1b );
	DBC_DrawBoxContents();
}



bool LVLED_GetLevelFore ()
{
	return bLevelFore;
}



void SetSpriteSelection( int a0, int b0, int a1, int b1 )
{
	sprite0a = a0;
	sprite0b = b0;
	sprite1a = a1;
	sprite1b = b1;

//	level_draw_spritesel();
	ED_DrawSprite(  4, 308, sprite0a, sprite0b );
	ED_DrawSprite( 24, 308, sprite1a, sprite1b );
	DBC_DrawBoxContents();
}



void SetLevel( int x, int y, int a, int b, bool bforeground )
{
	if (x<0 || y<0) return;
	if (x>=128 || y>=100) return;
	if (bforeground)
	{
		*(level_pointer(0, x, y) + 0) = a;
		*(level_pointer(0, x, y) + 1) = b;
	}
	else
	{
		*(level_pointer(0, x, y) + 2) = a;
		*(level_pointer(0, x, y) + 3) = b;

		DRAW_LEVEL_PIXEL(x, y);
		/*
		djgSetColorFore( pVis, sprite_get_color(a,b) );
		djgDrawBox( pVis,
			x*LEVEL_GRIDSIZE+1, y*LEVEL_GRIDSIZE+1,
			LEVEL_GRIDSIZE-1, LEVEL_GRIDSIZE-1 );
			*/
	}

	if ( ( x >= levelview_x )
		&& ( y >= levelview_y )
		&& ( x < levelview_x + levelview_w )
		&& ( y < levelview_y + levelview_h ) )
		ED_DrawSprite( POS_LEVELVIEW_X + (x - levelview_x) * 16,
		POS_LEVELVIEW_Y + (y - levelview_y) * 16,
		a, b );
}


void SelectLevel ( int i )
{
	if (i < 0)
		i = 0;
	if (i > g_pCurMission->NumLevels() - 1)
		i = g_pCurMission->NumLevels() - 1;

	g_nLevel = i;

//	level_draw_view();
//	draw_level_grid();
//	level_draw_levelname();
}




void LevelFill( int ax, int ay )
{
	int a, b; // block to replace
	int i;

	if ( (ax < 0) || (ay < 0) || (ax >= 128) || (ay >= 100) )
		return;

	// background block
	a = *(level_pointer( g_nLevel, ax, ay ) + 2);
	b = *(level_pointer( g_nLevel, ax, ay ) + 3);

	i = ax;
	while ( (a == *(level_pointer( g_nLevel, i, ay ) + 2)) &&
		(b == *(level_pointer( g_nLevel, i, ay ) + 3)) && (i >= 0) )
	{
		SetLevel( i, ay, sprite0a, sprite0b, false );
		i--;
	}
	i = ax + 1;
	while ( (a == *(level_pointer( g_nLevel, i, ay ) + 2)) &&
		(b == *(level_pointer( g_nLevel, i, ay ) + 3)) && (i <= 127) )
	{
		SetLevel( i, ay, sprite0a, sprite0b, false );
		i++;
	}
}

