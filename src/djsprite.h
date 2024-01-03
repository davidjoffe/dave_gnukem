/*!
\file    djsprite.h
\brief   Helper 
\author  David Joffe

Copyright (C) 1998-2024 David Joffe
*/
/*--------------------------------------------------------------------------*/
#ifndef _DJSPRITE_H_
#define _DJSPRITE_H_

class djImage;

//----------------------------------------------------------------------------
// Should this be renamed spritegrid?
// This corresponds to a single physical image (for now, or maybe always .. depends how this evolves in future)
class djSprite
{
public:
	djSprite() { }
	virtual ~djSprite() { }

	virtual bool LoadSpriteImage( const char * szFilename, int nSpriteW, int nSpriteH );
	virtual djImage* GetImage() { return m_pImage; }

	virtual bool IsLoaded() const { return m_pImage!=nullptr; }

	int GetSpriteW() const { return m_nSpriteW; }
	int GetSpriteH() const { return m_nSpriteH; }

	int GetNumSpritesX() const { return m_nSpritesX; }
	int GetNumSpritesY() const { return m_nSpritesY; }

	djImage* m_pImage=nullptr;

	//! Number of actual sprites on horizontal axis, this is derived automatically when loading the image
	int m_nSpritesX=0;
	//! Number of actual sprites on vertical axis, this is derived automatically when loading the image
	int m_nSpritesY=0;
	int m_nSpriteW=0;
	int m_nSpriteH=0;
};
//----------------------------------------------------------------------------
#endif
