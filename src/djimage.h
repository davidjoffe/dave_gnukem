/*!
\file    djimage.h
\brief   Image manipulation class
\author  David Joffe

Copyright (C) 1998-2022 David Joffe
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
	
	int CalculatePixelWidth( int ibpp );
	//! Create a blank image of given size, bit depth and pitch (pitch is total actual in-memory width in bytes. Pitch is effective total actual in-memory width of a single row of image (including any hypothetical optional padding, if any)
	void CreateImage( int x, int y, int ibpp, int ipitch=-1 );

	djColor GetPixelColor( int x, int y ) const;
	uint32_t     GetPixel( int x, int y ) const;
	void    PutPixel( int x, int y, uint32_t pixel );
	//! Return a pointer to the image data
	unsigned char *Data() const { return m_pData; }
	
	int Height() const { return m_iHeight; } //!< height in pixels
	int Width()  const { return m_iWidth; }  //!< width in pixels
	int Pitch()  const { return m_ipitch; }  //!< total width in bytes [dj2022-11 to double-check is this bytes or was it something else e.g. pixels?]
	int BPP()    const { return m_ibpp; }    //!< bits per pixel
	
	//! Load an image from disk. Only TGA (24/32-bit) filetype is supported.
	int Load( const char * szFilename );
	
	int LoadTGA( const char * szFilename );
	
	//dj2022-11 for now comment out SaveRAW as not using it for anything, but maybe might use it in future so leaving it in for now, but should maybe be refactored differently (probably shouldn't be *in* the image class, image loaders/savers should conceptually be a layer above and 'outside' the core image class)
	//int SaveRAW( const char * szFilename );

protected:
	unsigned char*	m_pData = nullptr;
	int             m_iWidth = 0;
	int             m_iHeight = 0;
	int             m_ibpp = 0;
	int             m_ipixwidth = 0;
	int             m_ipitch = 0;		//!< pitch is effectively total actual in-memory width of a single row of image (including any hypothetical padding, if any)
};

/*
// loaders/savers - should be separate file? tho needs access to procected members - either a friend class, or e.g. maybe rather just expose protected members with set variables etc.
// should we bother or just transition to sdl_image etc.? 
// (dj2022 note: Our TGA image loading code literally harkens from the 90s and was some of the earliest game code, and also because we were on LibSDL1 for a long time we just stuck with it .. I felt for a long time maybe once we're on SDL2 we could start maybe using e.g. libsdl_image to load more modern formats like .png which offer mainly compression)
class djImageLoad
{
public:
};
*/

#endif
