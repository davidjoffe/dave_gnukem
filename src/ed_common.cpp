// Level editor

#include "config.h"
#include "datadir.h"
#include "ed_common.h"
#include "djinput.h"
#include "djimage.h"
#include "graph.h"
#include "block.h"		// TYPE_LASTONE is here
#include "mission.h"
#include "ed_DrawBoxContents.h"
#include "ed_macros.h"
#include "sys_error.h"


// Convenience macro to call the sprite draw function for 16x16 sprite b in sprite set a
#define DRAW_SPRITE16(vis,a,b,x,y) djgDrawImage( vis, g_pCurMission->GetSpriteData(a)->m_pImage, ((b)%SPRITESHEET_NUM_COLS)*BLOCKW,((b)/SPRITESHEET_NUM_COLS)*BLOCKH, (x),(y), BLOCKW,BLOCKH )
// Same as above but uses alpha map
#define DRAW_SPRITE16A(vis,a,b,x,y) djgDrawImageAlpha( vis, g_pCurMission->GetSpriteData(a)->m_pImage, ((b)%SPRITESHEET_NUM_COLS)*BLOCKW,((b)/SPRITESHEET_NUM_COLS)*BLOCKH, (x),(y), BLOCKW,BLOCKH )


#define POS_BLOCKTYPES_X ((16*16)+40)
#define POS_BLOCKTYPES_Y 0


// also change in ed_spred.cpp, if any changes!!!!!!!!!!!
#define POS_EXTRAS_X ((16*16)+40)
#define POS_EXTRAS_Y ((POS_BLOCKTYPES_Y + (TYPE_LASTONE+1) * 8) + 8)
// !!!!!!!!



int	g_iSpriteset = 0;		// same as isprite, but points to current spriteset
int	g_iSprite = 0;		// current sprite. looks like it's a good idea to embed it into 'common'


static djImage		*g_pEdFont = NULL;


void ED_CommonInit ()
{
	SDL_DestroySurface(pVisMain->pSurface);
	SDL_DestroyTexture(pVisMain->pTexture);
	SDL_SetRenderLogicalPresentation(pVisMain->pRenderer, pVisMain->width, pVisMain->height, SDL_LOGICAL_PRESENTATION_STRETCH);
	pVisMain->pSurface = SDL_CreateSurface(pVisMain->width, pVisMain->height, pVisMain->pSurface->format);
	pVisMain->pTexture = SDL_CreateTextureFromSurface(pVisMain->pRenderer, pVisMain->pSurface);

	// rtfb:
	// i don't know how and why it worked before i set this,
	// but when i first tried to put my hands on Editor,
	// this was my main headache. In short, sprites just
	// *won't* display correctly if this is not set.
	auto f = SDL_GetPixelFormatDetails(pVisMain->format);
	pVisMain->stride = pVisMain->width * (f->bytes_per_pixel);
//	pVisMain->stride = 640*2;

	if (!LoadMacros())
		SYS_Warning ( "Failed to load macros!\n" );

	g_pEdFont = new djImage;
	g_pEdFont->Load( djDATAPATHc( "simplefont.tga" ));
	djCreateImageHWSurface(g_pEdFont);

	SDL_ShowCursor ();

	djiInit();
}

void ED_CommonKill ()
{
	djiInit();
	SDL_HideCursor();
	djDestroyImageHWSurface(g_pEdFont);
	if (g_pEdFont)
	{
		delete g_pEdFont;
		g_pEdFont = NULL;
	}
	DeleteMacros ();
	djiClearBuffer ();

	SDL_DestroySurface(pVisMain->pSurface);
	SDL_DestroyTexture(pVisMain->pTexture);
	SDL_SetRenderLogicalPresentation(pVisMain->pRenderer, CFG_APPLICATION_RENDER_RES_W, CFG_APPLICATION_RENDER_RES_H, SDL_LOGICAL_PRESENTATION_STRETCH);
	pVisMain->pSurface = SDL_CreateSurface(CFG_APPLICATION_RENDER_RES_W, CFG_APPLICATION_RENDER_RES_H, pVisMain->format);
	pVisMain->pTexture = SDL_CreateTextureFromSurface(pVisMain->pRenderer, pVisMain->pSurface);
}

int ED_GetCurrSprite ()
{
	return g_iSprite;
}


/*
====================
ED_IncCurrSprite

	increments current sprite by `amount', which may also
 be negative, then it decrements, of course. Returns a new value
 of current sprite.
====================
*/
int ED_IncCurrSprite( int amount )
{
	return (g_iSprite += amount);
}

int ED_GetCurrSpriteSet()
{
	return g_iSpriteset;
}

/*
====================
ED_SetSprite
====================
*/
void ED_SetSprite( int ispritenew, int ox, int oy )
{
	while (ispritenew < 0)
		ispritenew += 128;
	ispritenew = (ispritenew % 128);

	// clear out arrows
	ED_DrawStringClear( (g_iSprite%SPRITESHEET_NUM_COLS)*BLOCKW, oy - 8, "VV" );
	ED_DrawStringClear( ox + (BLOCKW*SPRITESHEET_NUM_COLS), oy + (g_iSprite/SPRITESHEET_NUM_COLS)*BLOCKH  , "<" );
	ED_DrawStringClear( ox + (BLOCKW*SPRITESHEET_NUM_COLS), oy + (g_iSprite/SPRITESHEET_NUM_COLS)*BLOCKH+8, "<" );

//	if (state == STATE_SPRITEEDITOR)
//	{
//		sprite_show_type( 0 );
//		sprite_show_extras( 0 );
//	}

	g_iSprite = ispritenew;
	// show sprite index
	std::string buf = std::to_string((int)g_iSprite);
	buf = std::string(3 - buf.length(), ' ') + buf;//right pad
	ED_DrawStringClear( 0, 472, buf );
	ED_DrawString( 0, 472, buf );

	djgSetColor( pVisMain, djColor(255,255,0), djColor(0,0,0) );
	ED_DrawString( (g_iSprite%SPRITESHEET_NUM_COLS)*BLOCKW, oy - 8, "VV" );
	ED_DrawString( ox + (BLOCKW*SPRITESHEET_NUM_COLS), oy + (g_iSprite/SPRITESHEET_NUM_COLS)*BLOCKH  , "<" );
	ED_DrawString( ox + (BLOCKW*SPRITESHEET_NUM_COLS), oy + (g_iSprite/SPRITESHEET_NUM_COLS)*BLOCKH+8, "<" );

//	if (state == STATE_SPRITEEDITOR)
//	{
//		sprite_show_type( 10 );
//		sprite_show_extras( 11 );
//		sprite_drawflags();
//	}
}


void ED_DrawString( int x, int y, const std::string& sText )
{
	if (!g_pEdFont) return;
	const size_t uLen=sText.length();
	for ( size_t i=0; i<uLen; ++i )
	{
		const int iChar = (int)sText[i];
		int iX, iY;
		iX = (iChar%32)*8;
		iY = (iChar/32)*8;
		djgDrawImageAlpha( pVisMain, g_pEdFont, iX, iY, x+i*8, y, 8, 8 );
	}
}

void ED_DrawStringClear( int x, int y, const std::string& sText )
{
	if (!g_pEdFont) return;
	djgSetColorFore( pVisMain, djColor(0,0,0) );
	const size_t uLen=sText.length();
	for ( int i=0; i<(int)uLen; ++i )
	{
		djgDrawBox( pVisMain, x+i*8, y, 8, 8 );
	}
}


void ED_SpriteShowType( bool bClear )
{
	int nType = ED_GetSpriteType(g_iSpriteset, g_iSprite);

	if (bClear)
		ED_DrawStringClear ( POS_BLOCKTYPES_X - 8,
				POS_BLOCKTYPES_Y + nType * 8,
				">" );
	else
		ED_DrawString ( POS_BLOCKTYPES_X - 8,
				POS_BLOCKTYPES_Y + nType * 8,
				">" );

	DBC_DrawBoxContents ();
}


void ED_SpriteShowExtra( int i )
{
	std::string s = std::to_string(i) + ":[" + std::to_string(ED_GetSpriteExtra(g_iSpriteset, g_iSprite, i)) + "]";
	switch (i)
	{
	case 4: s += "flags"; break;
	case 10: s += "box-contents-spriteset"; break;
	case 11: s += "box-contents-sprite"; break;//or letter-ID
	}

	ED_DrawStringClear(
	POS_EXTRAS_X,
	POS_EXTRAS_Y + i * 8,
	s.c_str());
	ED_DrawString(
	POS_EXTRAS_X,
	POS_EXTRAS_Y + i * 8,
	s.c_str());
}


int ED_GetSpriteType( int spriteset, int sprite )
{
	return g_pCurMission->GetSpriteData(spriteset)->m_type[sprite];
}



int ED_GetSpriteExtra( int spriteset, int sprite, int i )
{
	return g_pCurMission->GetSpriteData(spriteset)->m_extras[sprite][i];
}


void ED_DrawSprite( int x, int y, int a, int b )
{
	DRAW_SPRITE16(pVisMain,a,b,x,y);

	// If a box, show box contents overlaid on sprite so level editor can see what's inside the box immediately
	int nType = ED_GetSpriteType(a, b);
	if (nType==TYPE_BOX)
	{
		int c = ED_GetSpriteExtra(a, b, 10);
		int d = ED_GetSpriteExtra(a, b, 11);
		if ((c|d)!=0)
		{
			djgDrawImageAlpha(pVisMain, g_pCurMission->GetSpriteData(c)->m_pImage, ((d)%SPRITESHEET_NUM_COLS)*BLOCKW,((d)/SPRITESHEET_NUM_COLS)*BLOCKH, x,y+1, BLOCKW, BLOCKH-1 );
		}
	}
}


void ED_ClearScreen()
{
	djgSetColorFore( pVisMain, djColor(0,0,0) );
	djgClear( pVisMain );
}

void ED_FlipBuffers ()
{
	djgFlip ( pVisMain, NULL, false );
}


void ED_SetSpriteSet ( int new_spriteset )
{
	int direction;

	if (g_iSpriteset == new_spriteset)
		return;

	ED_SpriteShowType( true );

	direction = (new_spriteset - g_iSpriteset);

	while (new_spriteset < 0)
		new_spriteset += 256;

	if (direction < 0)
	{
		while (g_pCurMission->GetSpriteData( new_spriteset ) == NULL)
		{
			new_spriteset--;
			if (new_spriteset < 0)
				new_spriteset = 255;
		}
	}
	else
	{
		while (g_pCurMission->GetSpriteData( new_spriteset ) == NULL)
		{
			new_spriteset++;
			if (new_spriteset > 255)
				new_spriteset = 0;
		}
	}


	g_iSpriteset = new_spriteset;
}


void ED_SetSpriteExtra( int spriteset, int sprite, int i, int value )
{
	if ( value < 0 )
		value = 0;

	g_pCurMission->GetSpriteData(spriteset)->m_extras[sprite][i] = value;

	ED_SpriteShowExtra( i );
}


void ED_SetSpriteType( int spriteset, int sprite, int value )
{
	g_pCurMission->GetSpriteData(spriteset)->m_type[sprite] = value;
}



int ED_SetCurrSpriteSet ( int new_spriteset )
{
	g_iSpriteset = new_spriteset;
	return g_iSpriteset;
}


/*
====================
ED_IncCurrSpriteSet

 Increments current spriteset by `amount', which may also
 be negative, then it decrements, of course. Returns a new value
 of current spriteset.
====================
*/
int ED_IncCurrSpriteSet ( int amount )
{
	return (g_iSpriteset += amount);
}

djColor& ED_GetSpriteColor( int a, int b )
{
	return g_pCurMission->GetSpriteData( a )->m_Color[b];
}
