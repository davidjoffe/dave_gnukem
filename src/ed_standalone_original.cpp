/* ed_standalone_original.cpp
 * David Joffe
 * 1998/12/31
 * New level and graphics editor
 * 2002-09-21: Changed from ed.cpp to ed_standalone_original.cpp.
 */
/*--------------------------------------------------------------------------*/
#include "datadir.h"
#include "djgamelib.h"
#include <string.h>
#include <stdio.h>
#include "mission.h"
#include "djlog.h"
#include "block.h"
#include "level.h"
#include <string>
#include <vector>
using namespace std;

// Macro stuff
struct SMacro
{
	char *szName;
	vector<int> m_aiBlocks[4];
};
int g_iAssignedMacros[9]; // Keys 1 to 9 to place assigned macros
vector<SMacro*> g_apMacros;
bool LoadMacros();
bool DeleteMacros();
void PlaceMacro(int x, int y, int iMacroIndex);
void ShowMacros();
#define MACROS_X 270
#define MACROS_Y 320
/*--------------------------------------------------------------------------*/

#define STATE_SPRITEEDITOR 0
#define STATE_LEVELEDITOR  1

djImage *pFont=NULL;
void DrawString( int x, int y, const char *szStr );
void DrawStringClear( int x, int y, const char *szStr );

// Convenience macro to call the sprite draw function for 16x16 sprite b in sprite set a
#define DRAW_SPRITE16(vis,a,b,x,y) djgDrawImage( vis, g_pCurMission->GetSpriteData(a)->m_pImage, ((b)%16)*16,((b)/16)*16, (x),(y), 16,16 )
// Same as above but uses alpha map
#define DRAW_SPRITE16A(vis,a,b,x,y) djgDrawImageAlpha( vis, g_pCurMission->GetSpriteData(a)->m_pImage, ((b)%16)*16,((b)/16)*16, (x),(y), 16,16 )

/*--------------------------------------------------------------------------*/
#define POS_SPRITES_X 0
#define POS_SPRITES_Y 272

#define POS_COLORBOX_W 60
#define POS_COLORBOX_H 40
#define POS_INSTRUCTIONS_X 420
#define POS_INSTRUCTIONS_Y (((12*16)+8)+48)
#define POS_BLOCKTYPES_X ((16*16)+40)
#define POS_BLOCKTYPES_Y 0
#define POS_EXTRAS_X ((16*16)+40)
#define POS_EXTRAS_Y ((POS_BLOCKTYPES_Y + (TYPE_LASTONE+1) * 8) + 8)
#define POS_FLAGS ((POS_SPRITES_Y + 8 * 16) + 8)

#define POS_LEVELINSTRUCTIONS_X 420
#define POS_LEVELINSTRUCTIONS_Y 320
#define POS_LEVELSPRITES_X 0
#define POS_LEVELSPRITES_Y 336
#define LEVEL_VIEW_WIDTH 22
#define LEVEL_VIEW_HEIGHT 16
#define POS_LEVELVIEW_X (390-16*7)
#define POS_LEVELVIEW_Y 2

#define NUMFLAGS 5
char * szFlags[NUMFLAGS] =
{
	"solid",
	"animated",
	"falls",
	"inventory",
	"persistent"
};
/*--------------------------------------------------------------------------*/
djVisual *pVis;
int		imode;
int		state;
int		ispriteset = 0, isprite = 0;

int		sprite0a = 0;
int		sprite0b = 0;
int		sprite1a = 0;
int		sprite1b = 0;
int		levelview_x = 0;
int		levelview_y = 0;
bool	blevelfore = false;
bool	bShowBack = true;
int		g_nLevel = 0;

#define NUM_SPRITE_INSTRUCTIONS 12
char *sprite_instructions[NUM_SPRITE_INSTRUCTIONS] =
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
#define NUM_LEVEL_INSTRUCTIONS 15
char *level_instructions[NUM_LEVEL_INSTRUCTIONS] =
{
	"- Instructions: ----------",
	"1-9     Place macros 1-9",
	"Arrows  Move",
	"Alt+Clk MoveTo",
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
/*--------------------------------------------------------------------------*/
int     load_sprites();
void    set_sprite( int ispritenew );
void    set_spriteset( int ispritesetnew );
void    draw_sprites();
void    draw_grid( int x, int y, int w, int h, int nx, int ny, djColor clr );
void    draw_rectangle( int x, int y, int w, int h );
void    sprite_handlemouse(bool bCtrl);
int     sprites_save();
void    sprite_show_instructions();
void    sprite_show_block_types();
void    sprite_show_type( int c );
void    sprite_set_type( int itype );
void    sprite_show_extras( int c );
void    level_show_instructions();
void    clear_screen();
void    sprite_refreshdisplay();
void    level_refreshdisplay();
void    draw_level_grid();
djColor sprite_get_color( int a, int b );
void    level_handlemouse();
void    level_draw_spritesel();
void    level_set_spritesel( int a0, int b0, int a1, int b1 );
void    draw_sprite( int x, int y, int a, int b );
void    draw_spritea( int x, int y, int a, int b );
void    level_draw_view();
void    level_move_view( int ox, int oy, bool bredrawview = true );
void    level_set( int x, int y, int a, int b, bool bforeground );
void    level_setlevel( int i );
void    level_draw_levelname();
void    level_fill( int ax, int ay );
void    sprite_drawflags();
int     get_sprite_type( int spriteset, int sprite );
void    set_sprite_type( int spriteset, int sprite, int value );
int     get_sprite_extra( int spriteset, int sprite, int i );
void    set_sprite_extra( int spriteset, int sprite, int i, int value );

void    DrawBoxContents();

#define LEVEL_GRIDSIZE (2)

#define DRAW_LEVEL_PIXEL(x, y) \
{\
	djgSetColorFore( pVis, sprite_get_color( *(level_pointer( g_nLevel, (x), (y) ) + 2) , *(level_pointer( g_nLevel, (x), (y) ) + 3) ) );\
	djgDrawBox( pVis, (x)*LEVEL_GRIDSIZE, (y)*LEVEL_GRIDSIZE, LEVEL_GRIDSIZE, LEVEL_GRIDSIZE );\
}

struct SMessage
{
	string sMessage;
	int nFrameTimer;
};
vector<SMessage> g_aMessages;

/*--------------------------------------------------------------------------*/
int main( int argc, char** argv )
{
	bool brunning;
	int  i;
	
	state = STATE_SPRITEEDITOR;

	SDL_Init(SDL_INIT_VIDEO);

	bool bFullscreen = false;
	if (argc>1 && !strcmp(argv[1], "-f"))
		bFullscreen = true;

	if (NULL == (pVis = djgOpenVisual( bFullscreen?"fullscreen":NULL, 640, 480, 32, false )))
	//if (NULL == (pVis = djgOpenVisual( "fullscreen", 640, 480, 32, true )))
	{
		printf( "Stuffup2\n" );
		SDL_Quit();
		return -2;
	}


	if (!LoadMacros())
		printf( "Failed to load macros!\n" );

	
	pFont = new djImage;
	pFont->Load( "data/simplefont.tga" );

	djgSetColorFore( pVis, djColor(0,0,0) );
	djgClear( pVis );
	
	if (!djiInit( pVis, INPUT_MOUSE|INPUT_KEYBOARD ))
	{
		printf("fialed itnit input stuff\n");
	}
	

	

	// level initialization stuff
	// load level sets
	if ( 0 != LoadMissions( "data/missions.txt" ) )
		return -1;
	TRACE( "main(): %d game(s) found.\n", g_apMissions.size() );

	djgSetColorFore( pVis, djColor(255,255,255) );
	// let user choose level set to edit
	for ( i=0; i<(int)g_apMissions.size(); i++ )
	{ 
		DrawString( 16, i*8+16, g_apMissions[i]->GetName() );
	}
	{
		int  choice;
		bool bchoosing;

		DrawString( 16, 0, "Please choose levelset (esc or enter selects)" );
		
		choice = 0;
		DrawString( 8, choice*8+16, ">" );
		
		bchoosing = true;
		while (bchoosing)
		{
			djiPoll();
			
			if ( djiKeyPressed(DJKEY_UP) )
			{
				// FIXME: CLEAR
				DrawStringClear( 8, choice*8+16, " " );
				choice--;
				while ( choice < 0 )
					choice += g_apMissions.size();
				choice = choice % g_apMissions.size();
				DrawString( 8, choice*8+16, ">" );
			}
			if (djiKeyPressed(DJKEY_DOWN))
			{
				DrawStringClear( 8, choice*8+16, " " );
				choice++;
				choice = choice % g_apMissions.size();
				DrawString( 8, choice*8+16, ">" );
			}
			if ( g_iKeys[DJKEY_ENTER] | g_iKeys[DJKEY_ESC] )
			{
				g_pCurMission = g_apMissions[choice];
				bchoosing = false;
			}
			djgFlip(pVis, NULL);

		}
	}

	djgSetColorFore( pVis, djColor(0,0,0) );
	djgClear( pVis );
	

	
	// FIXME: Have some way of selecting g_pCurMission here
	
	// Load levels
	level_init();
	g_nLevel = 0;
	for ( i=0; i<g_pCurMission->NumLevels(); i++ )
	{
		char * szfilename;
		szfilename = g_pCurMission->GetLevel(i)->GetFilename();
		if ( NULL == ( level_load( i, szfilename ) ) )
		{
			printf( "main(): error loading level file %d=%s.\n", i, szfilename );
			return -2;
		}
	}
	
	// load sprites
	if ( load_sprites() != 0 )
	{
		printf( "main(): error loading sprites.\n" );
		goto leave;
	}
	
	// Check level for invalid blocks
	for ( i=0; i<g_pCurMission->NumLevels(); i++ )
	{
		int x, y, j, a, b;
		for ( y=0; y<100; y++ )
		{
			for ( x=0; x<128; x++ )
			{
				for ( j=0; j<2; j++ )
				{
					a = *(level_pointer( i, x, y ) + j * 2    );
					b = *(level_pointer( i, x, y ) + j * 2 + 1);
					if ( g_pCurMission->GetSpriteData(a) == NULL )
					{
						djMSG( "WARNING: INVALID SPRITESET ID FOUND (%d,%d) AT (%d,%d,%d)\n",
							a, b, x, y, j );
						*(level_pointer( i, x, y ) + j * 2    ) = '\0';
						*(level_pointer( i, x, y ) + j * 2 + 1) = '\0';
					}
				}
			}
		}
	}
	
	sprite_show_instructions();
	sprite_show_block_types();
	
	
	draw_sprites();
	
	// Mainloop
	brunning = true;
	while (brunning)
	{

		unsigned long delay = 50;
		
		// Try prevent CPU hogging a bit ..
		SDL_Delay(10);

		SDL_ShowCursor(0);
		//djgFlip(pVis, NULL);

		djiPoll();
		
		if (g_iKeys[DJKEY_ESC])
			brunning = false;
		
		switch (state)
		{
		case STATE_SPRITEEDITOR:
			
			sprite_handlemouse(g_iKeys[DJKEY_CTRL]!=0);
			
			if (g_iKeys[DJKEY_F5])
			{
				state = STATE_LEVELEDITOR;
				level_refreshdisplay();
			}
			if (g_iKeys[DJKEY_RIGHT])
			{
				set_sprite( isprite + 1 );
				SDL_Delay( delay );
			}
			if (g_iKeys[DJKEY_LEFT])
			{
				set_sprite( isprite - 1 );
				SDL_Delay( delay );
			}
			if (g_iKeys[DJKEY_UP])
			{
				set_sprite( isprite - 16 );
				SDL_Delay( delay );
			}
			if (g_iKeys[DJKEY_DOWN])
			{
				set_sprite( isprite + 16 );
				SDL_Delay( delay );
			}
			if (djiKeyPressed(DJKEY_F1))
			{
				if (sprites_save() < 0)
				{
					SMessage Msg;
					Msg.sMessage = "There was an error saving the sprites. Read-only?";
					Msg.nFrameTimer = 180;
					g_aMessages.push_back(Msg);
				}
				else
				{
					SMessage Msg;
					Msg.sMessage = "Sprites saved OK.";
					Msg.nFrameTimer = 80;
					g_aMessages.push_back(Msg);
				}
			}
			if (djiKeyPressed(DJKEY_N))  set_spriteset( ispriteset - 1 );
			if (djiKeyPressed(DJKEY_M))  set_spriteset( ispriteset + 1 );
			
			break;
			
		case 1: // level editor
			level_handlemouse();
			static bool bflagleft = false;
			static bool bflagright = false;
			static bool bflagup = false;
			static bool bflagdown = false;
			
			if (g_iKeys[DJKEY_LEFT])
			{
				bflagleft = true;
				level_move_view( levelview_x - 1, levelview_y, false );
				SDL_Delay( 10 );
			}
			if ( (g_iKeys[DJKEY_LEFT] == 0) && (bflagleft == true) )
			{
				bflagleft = false;
				level_move_view( levelview_x - 1, levelview_y );
			}
			
			if (g_iKeys[DJKEY_RIGHT])
			{
				bflagright = true;
				level_move_view( levelview_x + 1, levelview_y, false );
				SDL_Delay( 10 );
			}
			if ( (g_iKeys[DJKEY_RIGHT] == 0) && (bflagright == true) )
			{
				bflagright = false;
				level_move_view( levelview_x + 1, levelview_y );
			}
			
			if (g_iKeys[DJKEY_UP])
			{
				bflagup = true;
				level_move_view( levelview_x, levelview_y - 1, false );
				SDL_Delay( 10 );
			}
			if ( (g_iKeys[DJKEY_UP] == 0) && (bflagup == true) )
			{
				bflagup = false;
				level_move_view( levelview_x, levelview_y - 1 );
			}
			
			if (g_iKeys[DJKEY_DOWN])
			{
				bflagdown = true;
				level_move_view( levelview_x, levelview_y + 1, false );
				SDL_Delay( 10 );
			}
			if ( (g_iKeys[DJKEY_DOWN] == 0) && (bflagdown == true) )
			{
				bflagdown = false;
				level_move_view( levelview_x, levelview_y + 1);
			}
			
			
			if ( (mouse_x >= 0) && (mouse_y >= 0) )
			{
				if (g_iKeys[DJKEY_F])
				{
					// if mouse in main level view
					if (INBOUNDS( mouse_x, mouse_y, 0, 0, 128*LEVEL_GRIDSIZE-1, 100*LEVEL_GRIDSIZE-1 ))
					{
						level_fill( mouse_x / LEVEL_GRIDSIZE, mouse_y / LEVEL_GRIDSIZE );
					}
					// if mouse in zoom level view
					else if (
						INBOUNDS( mouse_x, mouse_y,
						POS_LEVELVIEW_X, POS_LEVELVIEW_Y,
						POS_LEVELVIEW_X + LEVEL_VIEW_WIDTH * 16 - 1,
						POS_LEVELVIEW_Y + LEVEL_VIEW_HEIGHT * 16 - 1 ))
					{
						level_fill(
							levelview_x + (mouse_x-POS_LEVELVIEW_X) / 16,
							levelview_y + (mouse_y-POS_LEVELVIEW_Y) / 16 );
					}
				}
				
			}
			// If the mouse is inside the main level view, do some extra checking
			if (INBOUNDS( mouse_x, mouse_y,
				POS_LEVELVIEW_X, POS_LEVELVIEW_Y,
				POS_LEVELVIEW_X + LEVEL_VIEW_WIDTH * 16 - 1,
				POS_LEVELVIEW_Y + LEVEL_VIEW_HEIGHT * 16 - 1 ))
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
				bool bErrors = false;
				for ( i=0; i<g_pCurMission->NumLevels(); i++ )
				{
					char * szfilename;
					szfilename = g_pCurMission->GetLevel(i)->GetFilename();
					if (level_save( i, szfilename ) < 0)
					{
						bErrors = true;
						SMessage Msg;
						Msg.sMessage = "There was an error saving ";
						Msg.sMessage += szfilename;
						Msg.nFrameTimer = 180;
						g_aMessages.push_back(Msg);
					}
				}
				if (bErrors)
				{
					SMessage Msg;
					Msg.sMessage = "Check that the file(s) are not read-only.";
					Msg.nFrameTimer = 180;
					g_aMessages.push_back(Msg);
				}
				else
				{
					SMessage Msg;
					Msg.sMessage = "Levels saved OK.";
					Msg.nFrameTimer = 80;
					g_aMessages.push_back(Msg);
				}
			}
			if (g_iKeys[DJKEY_F4])
			{
				state = STATE_SPRITEEDITOR;
				sprite_refreshdisplay();
			}
			if (djiKeyPressed(DJKEY_X))
			{
				blevelfore = !blevelfore;
				if (blevelfore)
				{
					DrawString( 48, 308, "[FOREGROUND]" );
				}
				else
				{
					DrawStringClear( 48, 308, "[FOREGROUND]" );
				}
				level_draw_view();
			}
			//-- Z - Toggle display of background blocks
			if (djiKeyPressed(DJKEY_Z))
			{
				bShowBack = !bShowBack;
				level_draw_view();
			}
			if (g_iKeys[DJKEY_CTRL])
			{
				if (djiKeyPressed(DJKEY_N))
				{
					level_setlevel( g_nLevel - 1 );
				}
				if (djiKeyPressed(DJKEY_M))
				{
					level_setlevel( g_nLevel + 1 );
				}
			}
			else
			{
				if (djiKeyPressed(DJKEY_N))
				{
					set_spriteset( ispriteset - 1 );
				}
				if (djiKeyPressed(DJKEY_M))
				{
					set_spriteset( ispriteset + 1 );
				}
			}
			
			break;
		}

		// This stuff is so that it redraws the display when messages timeout and disappear.
		static bool bRefresh = false;
		if (bRefresh)
		{
			if (state == STATE_LEVELEDITOR)
				level_refreshdisplay();
			else
				sprite_refreshdisplay();
			bRefresh = false;
		}

		// Display messages
		for ( i=0; i<g_aMessages.size(); i++ )
		{
			g_aMessages[i].nFrameTimer--;
			if (g_aMessages[i].nFrameTimer < 0)
			{
				g_aMessages.erase(g_aMessages.begin() + i);
				i--;
				bRefresh = true;
			}
			else
			{
				DrawStringClear( 10, i*8+16, g_aMessages[i].sMessage.c_str() );
				DrawString( 10, i*8+16, g_aMessages[i].sMessage.c_str() );
			}
		}
		
		SDL_ShowCursor(1);
		djgFlip(pVis, NULL);
	} // while (brunning)

leave:

	level_done();
	
	djiDone();
	
	SDL_Quit();
	
	return 0;
} // main
/*--------------------------------------------------------------------------*/

void DrawMinimapRectangle()
{
	djgSetColorFore( pVis, djColor(255,0,255) );
	djgDrawRectangle( pVis,
		levelview_x * LEVEL_GRIDSIZE,
		levelview_y * LEVEL_GRIDSIZE,
		(LEVEL_VIEW_WIDTH-1) * LEVEL_GRIDSIZE + LEVEL_GRIDSIZE,
		(LEVEL_VIEW_HEIGHT-1) * LEVEL_GRIDSIZE + LEVEL_GRIDSIZE );
}

int load_sprites()
{
	TRACE( "load_sprites()\n" );
	int i = g_pCurMission->LoadSprites();
	TRACE( "load_sprites(): finished\n" );
	return i;
}

void set_sprite( int ispritenew )
{
	int ox, oy;
	
	if (state == STATE_SPRITEEDITOR)
	{
		ox = POS_SPRITES_X;
		oy = POS_SPRITES_Y;
	}
	else if (state == STATE_LEVELEDITOR)
	{
		ox = POS_LEVELSPRITES_X;
		oy = POS_LEVELSPRITES_Y;
	}
	
	while (ispritenew < 0)
		ispritenew += 128;
	ispritenew = (ispritenew % 128);
	
	// clear out arrows
	DrawStringClear( (isprite%16)*16, oy - 8, "VV" );
	DrawStringClear( ox + 256, oy + (isprite/16)*16  , "<" );
	DrawStringClear( ox + 256, oy + (isprite/16)*16+8, "<" );
	
	if (state == STATE_SPRITEEDITOR)
	{
		sprite_show_type( 0 );
		sprite_show_extras( 0 );
	}
	
	isprite = ispritenew;
	// show sprite index
	char buf[64];
	sprintf( buf, "%3d", (int)isprite );
	DrawStringClear( 0, 472, buf );
	DrawString( 0, 472, buf );
	
	djgSetColor( pVis, djColor(255,255,0), djColor(0,0,0) );
	DrawString( (isprite%16)*16, oy - 8, "VV" );
	DrawString( ox + 256, oy + (isprite/16)*16  , "<" );
	DrawString( ox + 256, oy + (isprite/16)*16+8, "<" );
	
	if (state == STATE_SPRITEEDITOR)
	{
		sprite_show_type( 10 );
		sprite_show_extras( 11 );
		sprite_drawflags();
	}
	
}

void set_spriteset( int ispritesetnew )
{
	int direction;
	
	if (ispriteset == ispritesetnew)
		return;
	
	sprite_show_type( 0 );
	
	direction = (ispritesetnew - ispriteset);
	
	while (ispritesetnew < 0)
		ispritesetnew += 256;
	
	if (direction < 0)
	{
		while (g_pCurMission->GetSpriteData( ispritesetnew ) == NULL)
		{
			ispritesetnew--;
			if (ispritesetnew < 0)
				ispritesetnew = 255;
		}
	}
	else
	{
		while (g_pCurMission->GetSpriteData( ispritesetnew ) == NULL)
		{
			ispritesetnew++;
			if (ispritesetnew > 255)
				ispritesetnew = 0;
		}
	}
	
	
	ispriteset = ispritesetnew;
	
	draw_sprites();
}

void draw_sprites()
{
	int i;
	int ox, oy;
	int xoffset, yoffset;
	char buf[1024];
	
	djgSetColorFore( pVis, djColor(255,255,255) );
	sprintf( buf, "%d,%-15.15s", ispriteset, g_pCurMission->GetSpriteData(ispriteset)->m_szImgFilename );
	
	if (state == STATE_SPRITEEDITOR)
	{
		DrawString( 120, POS_SPRITES_Y - 8, buf );
		ox = POS_SPRITES_X;
		oy = POS_SPRITES_Y;
	}
	else if (state == STATE_LEVELEDITOR)
	{
		DrawString( 120, POS_LEVELSPRITES_Y - 8, buf );
		ox = POS_LEVELSPRITES_X;
		oy = POS_LEVELSPRITES_Y;
	}
	
	for ( i=0; i<128; i++ )
	{
		xoffset = (i%16)*16;
		yoffset = (i/16)*16;
		draw_sprite( ox + xoffset, oy + yoffset, ispriteset, i );
	}
	
	// make it redraw the pointers to the current sprite
	set_sprite( isprite );
}

void draw_grid( int x, int y, int w, int h, int nx, int ny, djColor clr )
{
	djgSetColorFore( pVis, clr );
	int i;
	for ( i=0; i<=ny; i++ )
	{
		djgDrawHLine( pVis, x, y+i*h, w*nx+1 );
	}
	for ( i=0; i<=nx; i++ )
	{
		djgDrawVLine( pVis, x+i*w, y, h*ny+1 );
	}
}

void draw_rectangle( int x, int y, int w, int h )
{
	djgDrawRectangle( pVis, x, y, w, h );
}

void sprite_handlemouse(bool bCtrl)
{
	static bool bLastL = false;
	static bool bLastR = false;


	int ax, ay;
	// inside the area for setting sprite type info?
	if (INBOUNDS( mouse_x, mouse_y,
		POS_BLOCKTYPES_X - 8,
		POS_BLOCKTYPES_Y,
		POS_BLOCKTYPES_X + 16*8,
		POS_BLOCKTYPES_Y + (TYPE_LASTONE+1) * 8 - 1))
	{
		if (mouse_b & 1)
		{
			ay = (mouse_y - POS_BLOCKTYPES_Y) / 8;
			sprite_set_type( ay );
		}
	}
	// inside the area for adjusting sprite extras info?
	else if (INBOUNDS( mouse_x, mouse_y,
		POS_EXTRAS_X,
		POS_EXTRAS_Y,
		POS_EXTRAS_X + 9 * 8 - 1,
		POS_EXTRAS_Y + 12 * 8 - 1 ))
	{
		ay = (mouse_y - POS_EXTRAS_Y) / 8;
		if ((bCtrl || !bLastL) && (mouse_b & 1))       // left button = decrement
		{
			set_sprite_extra( ispriteset, isprite, ay,
				get_sprite_extra( ispriteset, isprite, ay ) - 1 );
			//	 sprite_set_extra( ay, sprite_get_extra( ay ) - 1 );
			if ( ay == 4 )
				sprite_drawflags();
			else
				SDL_Delay( 10 );
		}
		else if ((bCtrl || !bLastR) && (mouse_b & 2))  // right button = increment
		{
			set_sprite_extra( ispriteset, isprite, ay,
				get_sprite_extra( ispriteset, isprite, ay ) + 1 );
			//	 sprite_set_extra( ay, sprite_get_extra( ay ) + 1 );
			if ( ay == 4 )
				sprite_drawflags();
			else
				SDL_Delay( 10 );
		}
	}
	// inside the area for editting flags?
	else if (INBOUNDS( mouse_x, mouse_y, 0, POS_FLAGS, 10*8-1, POS_FLAGS+NUMFLAGS*8-1 ))
	{
		ay = (mouse_y - POS_FLAGS) / 8;
		if ( mouse_b & 1 ) // set flag
		{
			set_sprite_extra( ispriteset, isprite, 4,
				get_sprite_extra( ispriteset, isprite, 4 ) | (1 << ay) );
			//	 sprite_set_extra( 4, sprite_get_extra( 4 ) | (1 << ay) );
			sprite_drawflags();
		}
		else if ( mouse_b & 2 ) // clear flag
		{
			set_sprite_extra( ispriteset, isprite, 4,
				get_sprite_extra( ispriteset, isprite, 4 ) & (~(1 << ay)) );
			//	 sprite_set_extra( 4, sprite_get_extra( 4 ) & (~(1 << ay)) );
			sprite_drawflags();
		}
	}
	// inside actual sprites area
	else if (INBOUNDS( mouse_x, mouse_y,
		POS_SPRITES_X, POS_SPRITES_Y,
		POS_SPRITES_X + 16 * 16 - 1,
		POS_SPRITES_Y + 16 * 8 - 1 ))
	{
		ax = (mouse_x - POS_SPRITES_X) / 16;
		ay = (mouse_y - POS_SPRITES_Y) / 16;
		if ( (mouse_b & 1) || (mouse_b & 2) )
			set_sprite( ay * 16 + ax );
	}
	
	bLastL = ((mouse_b & 1)!=0);
	bLastR = ((mouse_b & 2)!=0);
	
}

int sprites_save()
{
	TRACE( "sprites_save()\n" );
	int i = g_pCurMission->SaveSprites();
	TRACE( "sprites_save(): finished\n" );
	return i;
}

void sprite_show_instructions()
{
	for ( int i=0; i<NUM_SPRITE_INSTRUCTIONS; i++ )
	{
		DrawString( POS_INSTRUCTIONS_X, POS_INSTRUCTIONS_Y + i*8, sprite_instructions[i] );
	}
}

void sprite_show_block_types()
{
	for ( int i=0; i<=(int)TYPE_LASTONE; i++ )
	{
		DrawString( POS_BLOCKTYPES_X, POS_BLOCKTYPES_Y + i*8, block_type_names[i] );
	}
}

void DrawBoxContents()
{
	int ox, oy;
	int a, b;
	if (state == STATE_SPRITEEDITOR)
	{
		ox = POS_SPRITES_X;
		oy = POS_SPRITES_Y;
		a = ispriteset;
		b = isprite;
	}
	else if (state == STATE_LEVELEDITOR)
	{
		ox = POS_LEVELSPRITES_X;
		oy = POS_LEVELSPRITES_Y;
		a = sprite0a;
		b = sprite0b;
	}
	int nType = get_sprite_type(a, b);
	if (nType==TYPE_BOX)
	{
		DrawString(ox + 16*16 - 48, oy+16*8, "Box:");
		int c = get_sprite_extra(a, b, 10);
		int d = get_sprite_extra(a, b, 11);
		draw_sprite(ox + 16*16 - 16, oy+16*8, c, d);
	}
	else
	{
		DrawStringClear(ox + 16*16 - 48, oy+16*8, "Box:  ");
		DrawStringClear(ox + 16*16 - 48, oy+16*8+8, "Box:  ");
	}
}

void sprite_show_type( int c )
{
	int nType = get_sprite_type(ispriteset, isprite);
	if (c==0)
		DrawStringClear(
		POS_BLOCKTYPES_X - 8,
		POS_BLOCKTYPES_Y + nType * 8, ">" );
	else
		DrawString(
		POS_BLOCKTYPES_X - 8,
		POS_BLOCKTYPES_Y + nType * 8,
		">" );
	
	DrawBoxContents();
}

void sprite_set_type( int itype )
{
	if (itype == get_sprite_type( ispriteset, isprite ))
		return;
	sprite_show_type( 0 );
	set_sprite_type( ispriteset, isprite, itype );
	sprite_show_type( 10 );
}

void sprite_show_extra( int i, int c )
{
	char buf[64];
	sprintf( buf, "%2d:[%4d]", i, get_sprite_extra( ispriteset, isprite, i ) );

	if (c==0)
		DrawStringClear(
		POS_EXTRAS_X,
		POS_EXTRAS_Y + i * 8,
		buf );
	else
		DrawStringClear(
		POS_EXTRAS_X,
		POS_EXTRAS_Y + i * 8,
		buf );
		DrawString(
		POS_EXTRAS_X,
		POS_EXTRAS_Y + i * 8,
		buf );
}

void sprite_show_extras( int c )
{
	for ( int i=0; i<12; i++ )
	{
		sprite_show_extra( i, c );
	}
}

void level_show_instructions()
{
	for ( int i=0; i<NUM_LEVEL_INSTRUCTIONS; i++ )
	{
		DrawString(
			POS_LEVELINSTRUCTIONS_X,
			POS_LEVELINSTRUCTIONS_Y + i * 8,
			level_instructions[i] );
	}
}

void clear_screen()
{
	djgSetColorFore( pVis, djColor(0,0,0) );
	djgClear( pVis );
}

void sprite_refreshdisplay()
{
	clear_screen();
	
	sprite_show_instructions();
	sprite_show_instructions();
	sprite_show_block_types();
	draw_sprites();
}

void level_refreshdisplay()
{
	clear_screen();
	level_show_instructions();
	if (blevelfore)
		DrawString( 48, 308, "[FOREGROUND]" );
	else
		DrawStringClear( 48, 308, "[FOREGROUND]" );
	draw_sprites();
	draw_grid( 0, 0, LEVEL_GRIDSIZE, LEVEL_GRIDSIZE, 128, 100, djColor(60,60,60) );
	draw_level_grid();
	level_draw_spritesel();
	level_draw_view();
	level_draw_levelname();
	ShowMacros();
}

void draw_level_grid()
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

djColor sprite_get_color( int a, int b )
{
	return g_pCurMission->GetSpriteData( a )->m_Color[b];
}

void level_handlemouse()
{
	int ax, ay;
	
	// sprites area
	if (INBOUNDS( mouse_x, mouse_y,
		POS_LEVELSPRITES_X, POS_LEVELSPRITES_Y,
		POS_LEVELSPRITES_X + 16 * 16 - 1,
		POS_LEVELSPRITES_Y + 16 * 8 - 1 ))
	{
		ax = (mouse_x - POS_LEVELSPRITES_X) / 16;
		ay = (mouse_y - POS_LEVELSPRITES_Y) / 16;
		if (mouse_b & 1)
			level_set_spritesel( ispriteset, ay * 16 + ax, sprite1a, sprite1b );
		else if (mouse_b & 2)
			level_set_spritesel( sprite0a, sprite0b, ispriteset, ay * 16 + ax );
	}
	// main editting area
	else if (INBOUNDS( mouse_x, mouse_y, 0, 0, 128 * LEVEL_GRIDSIZE - 1, 100 * LEVEL_GRIDSIZE - 1 ))
	{
		ax = mouse_x / LEVEL_GRIDSIZE;
		ay = mouse_y / LEVEL_GRIDSIZE;
		if (g_iKeys[DJKEY_ALT])
		{
			if (mouse_b & 1)
				level_move_view( ax - (LEVEL_VIEW_WIDTH / 2),
				ay - (LEVEL_VIEW_HEIGHT / 2) );
		}
		else if (mouse_b & 1)
		{
			level_set( ax, ay, sprite0a, sprite0b, blevelfore );
		}
		else if (mouse_b & 2)
		{
			level_set( ax, ay, sprite1a, sprite1b, blevelfore );
		}
	}
	// zoomed view area
	else if (INBOUNDS( mouse_x, mouse_y,
		POS_LEVELVIEW_X, POS_LEVELVIEW_Y,
		POS_LEVELVIEW_X + LEVEL_VIEW_WIDTH * 16 - 1,
		POS_LEVELVIEW_Y + LEVEL_VIEW_HEIGHT * 16 - 1 ))
	{
		ax = (mouse_x - POS_LEVELVIEW_X) / 16;
		ay = (mouse_y - POS_LEVELVIEW_Y) / 16;
		if (mouse_b & 1)
		{
			level_set( ax + levelview_x, ay + levelview_y,
				sprite0a, sprite0b, blevelfore );
		}
		else if (mouse_b & 2)
		{
			level_set( ax + levelview_x, ay + levelview_y,
				sprite1a, sprite1b, blevelfore );
		}
	}
	
}

void level_draw_spritesel()
{
	draw_sprite(  4, 308, sprite0a, sprite0b );
	draw_sprite( 24, 308, sprite1a, sprite1b );
	DrawBoxContents();
}

void level_set_spritesel( int a0, int b0, int a1, int b1 )
{
	sprite0a = a0;
	sprite0b = b0;
	sprite1a = a1;
	sprite1b = b1;
	
	level_draw_spritesel();
}

void draw_sprite( int x, int y, int a, int b )
{
	DRAW_SPRITE16(pVis,a,b,x,y);
	int nType = get_sprite_type(a, b);
	if (nType==TYPE_BOX)
	{
		int c = get_sprite_extra(a, b, 10);
		int d = get_sprite_extra(a, b, 11);
		if ((c|d)!=0)
		{
			//draw_spritea(x, y+1, c, d);
			djgDrawImageAlpha(pVis, g_pCurMission->GetSpriteData(c)->m_pImage, ((d)%16)*16,((d)/16)*16, x,y+1, 16, 15 );
		}
	}
}

void draw_spritea( int x, int y, int a, int b )
{
	DRAW_SPRITE16A(pVis,a,b,x,y);
}

void level_draw_view()
{
	int i, j;
	int a, b;

	// Draw the purple rectangle indicating where your zoomed view is
	DrawMinimapRectangle();

	// Draw the zoomed view
	for ( i=0; i<LEVEL_VIEW_HEIGHT; i++ )
	{
		for ( j=0; j<LEVEL_VIEW_WIDTH; j++ )
		{
			a = *(level_pointer( g_nLevel, levelview_x + j, levelview_y + i) + 2);
			b = *(level_pointer( g_nLevel, levelview_x + j, levelview_y + i) + 3);
			// Draw a black block if ShowBack is disabled
			if (!bShowBack)
			{
				djgSetColorFore( pVis, djColor(0,0,0) );
				djgDrawBox( pVis, POS_LEVELVIEW_X + j*16, POS_LEVELVIEW_Y + i*16, 16, 16 );
			}
			else
			{
				draw_sprite( POS_LEVELVIEW_X + j * 16,
					POS_LEVELVIEW_Y + i * 16,
					a, b );
			}
			if (blevelfore)
			{
				a = *(level_pointer( g_nLevel, levelview_x + j, levelview_y + i) + 0);
				b = *(level_pointer( g_nLevel, levelview_x + j, levelview_y + i) + 1);
				if ((a) || (b))
					draw_sprite( POS_LEVELVIEW_X + j * 16,
					POS_LEVELVIEW_Y + i * 16,
					a, b );
			}
		}
	}
}

void level_move_view( int ox, int oy, bool bredrawview )
{
	if ( ( ox == levelview_x ) && ( oy == levelview_y ) ) // Hasn't moved?
		return;

	// Erase the previous rectangle by redrawing the level blocks over the bits that the rectangle covers.
	int i;
	for ( i=0; i<LEVEL_VIEW_WIDTH; i++ )
	{
		DRAW_LEVEL_PIXEL(levelview_x+i, levelview_y);
		DRAW_LEVEL_PIXEL(levelview_x+i, levelview_y+LEVEL_VIEW_HEIGHT-1);
	}
	for ( i=0; i<LEVEL_VIEW_HEIGHT; i++ )
	{
		DRAW_LEVEL_PIXEL(levelview_x, levelview_y+i);
		DRAW_LEVEL_PIXEL(levelview_x+LEVEL_VIEW_WIDTH-1, levelview_y+i);
	}

	// Set new level view offset, checking bounds.
	levelview_x = djCLAMP(ox, 0, 128 - LEVEL_VIEW_WIDTH);
	levelview_y = djCLAMP(oy, 0, 100 - LEVEL_VIEW_HEIGHT);

	// For speed reasons, we don't redraw the zoomed view every time, only when keypress released.
	if (bredrawview)
		level_draw_view();
	else
		DrawMinimapRectangle(); // Redraw the purple rectangle
}

void level_set( int x, int y, int a, int b, bool bforeground )
{
	if (x<0 || y<0) return;
	if (x>=128 || y>=100) return;
	if (bforeground)
	{
		*(level_pointer(g_nLevel, x, y) + 0) = a;
		*(level_pointer(g_nLevel, x, y) + 1) = b;
	}
	else
	{
		*(level_pointer(g_nLevel, x, y) + 2) = a;
		*(level_pointer(g_nLevel, x, y) + 3) = b;

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
		&& ( x < levelview_x + LEVEL_VIEW_WIDTH )
		&& ( y < levelview_y + LEVEL_VIEW_HEIGHT ) )
		draw_sprite( POS_LEVELVIEW_X + (x - levelview_x) * 16,
		POS_LEVELVIEW_Y + (y - levelview_y) * 16,
		a, b );
}

void level_setlevel( int i )
{
	if (i < 0)
		i = 0;
	if (i > g_pCurMission->NumLevels() - 1)
		i = g_pCurMission->NumLevels() - 1;
	
	g_nLevel = i;
	
	level_draw_view();
	draw_level_grid();
	level_draw_levelname();
}

void level_draw_levelname()
{

	char * szfilename;
	szfilename = g_pCurMission->GetLevel(g_nLevel)->GetFilename();
	DrawStringClear( POS_LEVELVIEW_X, POS_LEVELVIEW_Y + 16 * LEVEL_VIEW_HEIGHT + 8,
		"                           " );
	DrawString( POS_LEVELVIEW_X, POS_LEVELVIEW_Y + 16 * LEVEL_VIEW_HEIGHT + 8,
		szfilename );
	
}

void level_fill( int ax, int ay )
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
		level_set( i, ay, sprite0a, sprite0b, false );
		i--;
	}
	i = ax + 1;
	while ( (a == *(level_pointer( g_nLevel, i, ay ) + 2)) &&
		(b == *(level_pointer( g_nLevel, i, ay ) + 3)) && (i <= 127) )
	{
		level_set( i, ay, sprite0a, sprite0b, false );
		i++;
	}
	
}

void sprite_drawflags()
{
	int  i;
	unsigned char buf[32], c;
	for ( i=0; i<NUMFLAGS; i++ )
	{
		if (get_sprite_extra( ispriteset, isprite, 4 ) & (1 << i))
			//      if (g_pSprites[ispriteset]->m_extras[isprite][4] & (1 << i))
			c = 'X';
		else
			c = 255; // solid black block
		sprintf( (char*)buf, "[ ] %s", szFlags[i] );
		buf[1] = c;
		
		DrawString( 0, POS_FLAGS + i * 8, (char*)buf );
	}
}

int get_sprite_type( int spriteset, int sprite )
{
	return g_pCurMission->GetSpriteData(spriteset)->m_type[sprite];
}

void set_sprite_type( int spriteset, int sprite, int value )
{
	g_pCurMission->GetSpriteData(spriteset)->m_type[sprite] = value;
}

int get_sprite_extra( int spriteset, int sprite, int i )
{
	return g_pCurMission->GetSpriteData(spriteset)->m_extras[sprite][i];
}

void set_sprite_extra( int spriteset, int sprite, int i, int value )
{
	if ( value < 0 )
		value = 0;
	
	g_pCurMission->GetSpriteData(spriteset)->m_extras[sprite][i] = value;
	
	sprite_show_extra( i, 11 );
}



void DrawString( int x, int y, const char *szStr )
{
	if (!pFont) return;
	for ( int i=0; i<(int)strlen(szStr); i++ )
	{
		int iChar = (int)((const unsigned char*)szStr)[i];
		int iX, iY;
		iX = (iChar%32)*8;
		iY = (iChar/32)*8;
		djgDrawImageAlpha( pVis, pFont, iX, iY, x+i*8, y, 8, 8 );
	}
}

void DrawStringClear( int x, int y, const char *szStr )
{
	if (!pFont) return;
	djgSetColorFore( pVis, djColor(0,0,0) );
	for ( int i=0; i<(int)strlen(szStr); i++ )
	{
		djgDrawBox( pVis, x+i*8, y, 8, 8 );
	}
}

// Macro stuff
bool LoadMacros()
{
	FILE *fin;
	char buf[1024];
	if (NULL == (fin = fopen( "data/editor/macros.txt", "r" )))
		return false;

	SMacro *pMacro;
	fgets( buf, sizeof(buf), fin );
	djStripCRLF(buf); // strip CR/LF characters
	while (strcmp(buf, "$") && !feof(fin))
	{
		pMacro = new SMacro;
		pMacro->szName = djStrDeepCopy( buf );

		fgets( buf, sizeof(buf), fin );
		djStripCRLF(buf); // strip CR/LF characters
		while (strcmp(buf, "~") && !feof(fin))
		{
			int ix, iy, a, b;
			sscanf( buf, "%d %d %d %d", &ix, &iy, &a, &b );
			pMacro->m_aiBlocks[0].push_back(ix);
			pMacro->m_aiBlocks[1].push_back(iy);
			pMacro->m_aiBlocks[2].push_back(a);
			pMacro->m_aiBlocks[3].push_back(b);

			fgets( buf, sizeof(buf), fin );
			djStripCRLF(buf); // strip CR/LF characters
		}

		g_apMacros.push_back(pMacro);
		
		fgets( buf, sizeof(buf), fin );
		djStripCRLF(buf); // strip CR/LF characters
	}

	int i;
	for ( i=0; i<9; i++ )
	{
		g_iAssignedMacros[i] = (i<(int)g_apMacros.size() ? i : -1);
	}

	for ( i=0; i<(int)g_apMacros.size(); i++ )
	{
		SMacro *pMac = g_apMacros[i];
		printf( "%s\n", pMac->szName );
		for ( int j=0; j<(int)pMac->m_aiBlocks[0].size(); j++ )
		{
			printf( "%d %d %d %d\n",
				pMac->m_aiBlocks[0][j],
				pMac->m_aiBlocks[1][j],
				pMac->m_aiBlocks[2][j],
				pMac->m_aiBlocks[3][j] );
		}
	}

	fclose(fin);
	return true;
}

bool DeleteMacros()
{
	return true;
}

void PlaceMacro(int x, int y, int iMacroIndex)
{
	if ( (x < 0) || (y < 0) || (x >= 128) || (y >= 100) )
		return;

	int iMacro = g_iAssignedMacros[iMacroIndex];
	if (iMacro<0) return;
	if (iMacro>=(int)g_apMacros.size()) return;

	SMacro *pMacro = g_apMacros[iMacro];
	if (pMacro==NULL)
		return;

	for ( int i=0; i<(int)pMacro->m_aiBlocks[0].size(); i++ )
	{
		level_set(
			x+pMacro->m_aiBlocks[0][i],
			y+pMacro->m_aiBlocks[1][i],
			pMacro->m_aiBlocks[2][i],
			pMacro->m_aiBlocks[3][i],
			blevelfore );
	}
}

void ShowMacros()
{
	for ( int i=0; i<9; i++ )
	{
		int iMacro = g_iAssignedMacros[i];
		char buf[1024];
		sprintf( buf, "%d.", i+1 );
		DrawStringClear( MACROS_X, MACROS_Y, "Macros:" );
		DrawString( MACROS_X, MACROS_Y, "Macros:" );
		
		DrawStringClear( MACROS_X, MACROS_Y+i*8+8, buf );
		DrawString( MACROS_X, MACROS_Y+i*8+8, buf );
		if (iMacro==-1)
		{
			DrawStringClear( MACROS_X+16, MACROS_Y+i*8+8, "none" );
			DrawString( MACROS_X+16, MACROS_Y+i*8+8, "none" );
		}
		else
		{
			SMacro *pMac = g_apMacros[iMacro];
			DrawStringClear( MACROS_X+16, MACROS_Y+i*8+8, pMac->szName );
			DrawString( MACROS_X+16, MACROS_Y+i*8+8, pMac->szName  );
		}
	}
}

