
#ifndef __EDITOR_COMMON_STUFF_H__
#define __EDITOR_COMMON_STUFF_H__


// !!!!!!!!!!!
// If these two change, change them in ed_DrawBoxContents too!!!
#define POS_SPRITES_X 0
#define POS_SPRITES_Y 272
// !!!!!!!!!


//! Set or clear 'unsaved-changes' statue on the document (level being edited)
extern void SetDocumentDirty(bool bDirty=true);

class djColor;		// forward



void ED_CommonInit ();
void ED_CommonKill ();


int ED_GetCurrSprite ();
/*
====================
ED_IncCurrSprite

	increments current sprite by `amount', which may also
 be negative, then it decrements, of course. Returns a new value
 of current sprite.
====================
*/
int ED_IncCurrSprite ( int amount );
int ED_GetCurrSpriteSet ();
int ED_SetCurrSpriteSet ( int new_spriteset );
/*
====================
ED_IncCurrSpriteSet

	increments current spriteset by `amount', which may also
 be negative, then it decrements, of course. Returns a new value
 of current spriteset.
====================
*/
int ED_IncCurrSpriteSet ( int amount );


/*
=============================================================
		Common Stuff
=============================================================
*/
// actaul drawers (relate on djg*):
void ED_DrawSprite( int x, int y, int a, int b );
// void ED_DrawSprites();
void ED_ClearScreen();
void ED_DrawString( int x, int y, const char *szStr );
void ED_DrawStringClear( int x, int y, const char *szStr );

// 2nd level drawers/visual-related:
//void ED_DrawBoxContents();


void ED_FlipBuffers ();



// 3rd level drawers (depend on 2nd level drawers):
/*
====================
ED_SetSprite
====================
*/
//void ED_SetSprite( int ispritenew );
void ED_SetSprite( int ispritenew, int ox, int oy );
void ED_SetSpriteSet ( int ispritesetnew );




// "show"-functions:
void ED_SpriteShowType( bool bClear );	// DrawString, DrawBoxContents.
				// refers to sprite as an object,
				// not as a sprite editor. hence,
				// it is common function
void ED_SpriteShowExtra( int i );	// DrawString
				// refers to sprite as an object,
				// not as a sprite editor. hence,
				// it is common function



// accessors (needed for both SPRED and LVLED):
int ED_GetSpriteType( int spriteset, int sprite );
int ED_GetSpriteExtra( int spriteset, int sprite, int i );
void ED_SetSpriteExtra( int spriteset, int sprite, int i, int value );
void ED_SetSpriteType( int spriteset, int sprite, int value );
djColor& ED_GetSpriteColor( int a, int b );



// misc:
void ED_LevelFill( int ax, int ay );	// what does this do??
void ED_LevelSet( int x, int y, int a, int b, bool bforeground );		// // in ed.cpp

#endif		// #ifndef __EDITOR_COMMON_STUFF_H__

