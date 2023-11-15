/*!
\file    djimage.h
\brief   Image manipulation class
\author  David Joffe

Copyright (C) 1998-2023 David Joffe
*/
/*--------------------------------------------------------------------------*/
#ifndef _DJIMAGE_H_
#define _DJIMAGE_H_

#include "djtypes.h"
#include <cstdint>//for uint32_t (because on 64-bit-int platforms we don't want e.g. putpixel to do weird things like overwrite 2 pixels)

/*!
\class djImage
\nosubgrouping

Generic image loading/manipulation class. Currently only loads TGA's.
*/
class djImage
{
public:
	//! Constructor
	djImage();
	//! Constructor
	djImage( int iWidth, int iHeight, int ibpp );
	//! Destructor
	~djImage();
	
	int CalculatePixelWidthBytesPerPixel( int nBitsPerPixel );
	//! Create a blank image of given size, bit depth and pitch (pitch is total actual in-memory width in bytes. Pitch is effective total actual in-memory width of a single row of image (including any hypothetical optional padding, if any)
	void CreateImage( int x, int y, int nBitsPerPixel, int ipitch=-1, void* pOptionalCopyDataFrom=nullptr
    //#if SDL_BYTEORDER == SDL_BIG_ENDIAN
//	, unsigned int Rmask=0x0000FF00
//	, unsigned int Gmask=0x00FF0000
//	, unsigned int Bmask=0xFF000000
//	, unsigned int Amask=0x000000FF
    //rmask = 0xff000000;
    //gmask = 0x00ff0000;
    //bmask = 0x0000ff00;
    //amask = 0x000000ff;
//    #else
	, unsigned int Rmask=0x00FF0000
	, unsigned int Gmask=0x0000FF00
	, unsigned int Bmask=0x000000FF
	, unsigned int Amask=0xFF000000
    //rmask = 0x000000ff;
    //gmask = 0x0000ff00;
    //bmask = 0x00ff0000;
    //amask = 0xff000000;
  //  #endif
/*
	// R,G,B,A masks (dj2018-03 specify different masks here on PPC etc. - see https://github.com/davidjoffe/dave_gnukem/issues/100 - thanks to @BeWorld2018 for report and patch suggestion)
	#if SDL_BYTEORDER==SDL_BIG_ENDIAN
	0x0000FF00, 0X00FF0000, 0xFF000000, 0x000000FF
	#else
	0xFF0000,
	0xFF00,
	0xFF,
	0xFF000000
	#endif
*/

	//unsigned int m_Rmask=0x00FF0000;
	//unsigned int m_Gmask=0x0000FF00;
	//unsigned int m_Bmask=0x000000FF;
	//unsigned int m_Amask=0xFF000000;
);

	djColor GetPixelColor( int x, int y ) const;
	uint32_t     GetPixel( int x, int y ) const;
	void    PutPixel( int x, int y, uint32_t pixel );
	//! Return a pointer to the image data
	unsigned char *Data() const { return m_pData; }
	
	int Height() const { return m_iHeight; } //!< height in pixels
	int Width()  const { return m_iWidth; }  //!< width in pixels
	int Pitch()  const { return m_ipitch; }  //!< total width in bytes (which may potentially be greater than 'width * bytes-per-pixel') [dj2022-11 to double-check is this bytes or was it something else e.g. pixels?]
	int BPP()    const { return m_ibpp; }    //!< bits per pixel
	
	//! Load an image from disk. Only TGA (24/32-bit) filetype is supported.
	int Load( const char * szFilename );
	
	// Todo [low prio], design-wise it's not really correct for the djImage class to have loading code in it, the core djImage should know nothing about loading but just expose methods for a code laying 'layer' above this class to do actual loading.
	int LoadTGA( const char * szFilename );
	
	//dj2022-11 for now comment out SaveRAW as not using it for anything, but maybe might use it in future so leaving it in for now, but should maybe be refactored differently (probably shouldn't be *in* the image class, image loaders/savers should conceptually be a layer above and 'outside' the core image class)
	//int SaveRAW( const char * szFilename );


	unsigned int m_Rmask=0x00FF0000;
	unsigned int m_Gmask=0x0000FF00;
	unsigned int m_Bmask=0x000000FF;
	unsigned int m_Amask=0xFF000000;

protected:
	unsigned char*	m_pData = nullptr;
	int             m_iWidth = 0;
	int             m_iHeight = 0;
	int             m_ibpp = 0;			//!< bits per pixel
	int             m_ipixwidth = 0;	//!< bytes per pixel
	int             m_ipitch = 0;		//!< pitch is effectively total actual in-memory width in of a single row of image (including any hypothetical padding, if any) (I think this should probably be in bytes but not 100% sure all old code in this codebase treats it that way or sets it up correctly)
};

//----------------------------------------------------------------------------
// This should be in its own file:
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
