/*
djsprite.cpp

Copyright (C) 1998-2023 David Joffe
*/

#include "config.h"
#include "djsprite.h"
#include "djimage.h"
#include "djimageload.h"

//---------------------------------------------------------------------------

bool djSprite::LoadSpriteImage( const char * szFilename, int nSpriteW, int nSpriteH )
{
	m_pImage=nullptr;
	m_nSpritesX = 0;
	m_nSpritesY = 0;
	m_nSpriteW = 0;
	m_nSpriteH = 0;

	if (szFilename==nullptr||szFilename[0]==0)
		return false;

	m_pImage = djImageLoad::LoadImage( szFilename );
	if (m_pImage==nullptr)
		return false;

	m_nSpriteW = nSpriteW;
	m_nSpriteH = nSpriteH;
	m_nSpritesX = (m_pImage->Width() / m_nSpriteW);
	m_nSpritesY = (m_pImage->Height() / m_nSpriteH);

	return true;
}

//---------------------------------------------------------------------------
