
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
#define DRAW_SPRITE16(vis,a,b,x,y) djgDrawImage( vis, g_pCurMission->GetSpriteData(a)->m_pImage, ((b)%16)*16,((b)/16)*16, (x),(y), 16,16 )
// Same as above but uses alpha map
#define DRAW_SPRITE16A(vis,a,b,x,y) djgDrawImageAlpha( vis, g_pCurMission->GetSpriteData(a)->m_pImage, ((b)%16)*16,((b)/16)*16, (x),(y), 16,16 )




#define POS_BLOCKTYPES_X ((16*16)+40)
#define POS_BLOCKTYPES_Y 0


// also change in ed_spred.cpp, if any changes!!!!!!!!!!!
#define POS_EXTRAS_X ((16*16)+40)
#define POS_EXTRAS_Y ((POS_BLOCKTYPES_Y + (TYPE_LASTONE+1) * 8) + 8)
// !!!!!!!!




int	g_iSpriteset = 0;		// same as isprite, but points to current spriteset
int	g_iSprite = 0;		// current sprite. looks like it's a good idea to embed it into 'common'



static djImage		*pFont = NULL;



void ED_CommonInit ()
{
	// rtfb:
	// i don't know how and why it worked before i set this,
	// but when i first tried to put my hands on Editor,
	// this was my main headache. In short, sprites just
	// *won't* display correctly if this is not set.
	pVisMain->stride = pVisMain->width * (pVisMain->bpp/8);
//	pVisMain->stride = 640*2;

	if (!LoadMacros())
		SYS_Warning ( "Failed to load macros!\n" );

	pFont = new djImage;
	pFont->Load( "data/simplefont.tga" );
	djCreateImageHWSurface(pFont);

	SDL_ShowCursor ( 1 );

	if (!djiInit( pVisMain, INPUT_MOUSE|INPUT_KEYBOARD ))
	{
		printf("failed init input stuff\n");
	}
}




void ED_CommonKill ()
{
	djiInit( pVisMain, INPUT_KEYDOWN|INPUT_KEYUP|INPUT_KEYREPEAT );
	SDL_ShowCursor(0);
	djDestroyImageHWSurface(pFont);
	delete pFont;
	pFont = NULL;
	DeleteMacros ();
	djiClearBuffer ();
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
int ED_IncCurrSprite ( int amount )
{
	return (g_iSprite += amount);
}




int ED_GetCurrSpriteSet ()
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
	ED_DrawStringClear( (g_iSprite%16)*16, oy - 8, "VV" );
	ED_DrawStringClear( ox + 256, oy + (g_iSprite/16)*16  , "<" );
	ED_DrawStringClear( ox + 256, oy + (g_iSprite/16)*16+8, "<" );

//	if (state == STATE_SPRITEEDITOR)
//	{
//		sprite_show_type( 0 );
//		sprite_show_extras( 0 );
//	}

	g_iSprite = ispritenew;
	// show sprite index
	char buf[64]={0};
	sprintf( buf, "%3d", (int)g_iSprite );
	ED_DrawStringClear( 0, 472, buf );
	ED_DrawString( 0, 472, buf );

	djgSetColor( pVisMain, djColor(255,255,0), djColor(0,0,0) );
	ED_DrawString( (g_iSprite%16)*16, oy - 8, "VV" );
	ED_DrawString( ox + 256, oy + (g_iSprite/16)*16  , "<" );
	ED_DrawString( ox + 256, oy + (g_iSprite/16)*16+8, "<" );

//	if (state == STATE_SPRITEEDITOR)
//	{
//		sprite_show_type( 10 );
//		sprite_show_extras( 11 );
//		sprite_drawflags();
//	}
}



void ED_DrawString( int x, int y, const char *szStr )
{
	if (!pFont) return;
	for ( int i=0; i<(int)strlen(szStr); i++ )
	{
		int iChar = (int)((unsigned char*)szStr)[i];
		int iX, iY;
		iX = (iChar%32)*8;
		iY = (iChar/32)*8;
		djgDrawImageAlpha( pVisMain, pFont, iX, iY, x+i*8, y, 8, 8 );
	}
}



void ED_DrawStringClear( int x, int y, const char *szStr )
{
	if (!pFont) return;
	djgSetColorFore( pVisMain, djColor(0,0,0) );
	for ( int i=0; i<(int)strlen(szStr); i++ )
	{
		djgDrawBox( pVisMain, x+i*8, y, 8, 8 );
	}
}




void ED_SpriteShowType( int c )
{
	int nType = ED_GetSpriteType(g_iSpriteset, g_iSprite);

	if (c==0)
		ED_DrawStringClear ( POS_BLOCKTYPES_X - 8,
				POS_BLOCKTYPES_Y + nType * 8,
				">" );
	else
		ED_DrawString ( POS_BLOCKTYPES_X - 8,
				POS_BLOCKTYPES_Y + nType * 8,
				">" );

	DBC_DrawBoxContents ();
}




void ED_SpriteShowExtra( int i, int c )
{
	char buf[128]={0};
	sprintf( buf, "%2d:[%4d]", i, ED_GetSpriteExtra( g_iSpriteset, g_iSprite, i ) );
	switch (i)
	{
	case 4: strcat(buf, "flags");break;
	case 10: strcat(buf, "box-contents-spriteset");break;
	case 11: strcat(buf, "box-contents-sprite");break;//or letter-ID
	}

	/*
	if (c==0)
		ED_DrawStringClear(
		POS_EXTRAS_X,
		POS_EXTRAS_Y + i * 8,
		buf );
	else
	*/
		ED_DrawStringClear(
		POS_EXTRAS_X,
		POS_EXTRAS_Y + i * 8,
		buf );
		ED_DrawString(
		POS_EXTRAS_X,
		POS_EXTRAS_Y + i * 8,
		buf );
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

	int nType = ED_GetSpriteType(a, b);
	if (nType==TYPE_BOX)
	{
		int c = ED_GetSpriteExtra(a, b, 10);
		int d = ED_GetSpriteExtra(a, b, 11);
		if ((c|d)!=0)
		{
			//draw_spritea(x, y+1, c, d);
			djgDrawImageAlpha(pVisMain, g_pCurMission->GetSpriteData(c)->m_pImage, ((d)%16)*16,((d)/16)*16, x,y+1, 16, 15 );
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

	ED_SpriteShowType( 0 );

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

	ED_SpriteShowExtra( i, 11 );
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

	increments current spriteset by `amount', which may also
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

