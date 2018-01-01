/*!
\file    djimage.h
\brief   Image manipulation class
\author  David Joffe

Copyright (C) 1998-2018 David Joffe

License: GNU GPL Version 2
*/
/*--------------------------------------------------------------------------*/
#ifndef _DJIMAGE_H_
#define _DJIMAGE_H_

#include "djtypes.h"
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
	//! Create a blank image of give size, bit depth and ipitch
	void CreateImage( int x, int y, int ibpp, int ipitch=-1 );

	djColor GetPixelColor( int x, int y );
	int     GetPixel( int x, int y );
	void    PutPixel( int x, int y, unsigned int pixel );
	//! Return a pointer to the image data
	unsigned char *Data() const { return m_pData; }
	
	int Height() const { return m_iHeight; } //!< height in pixels
	int Width()  const { return m_iWidth; }  //!< width in pixels
	int Pitch()  const { return m_ipitch; }  //!< total width in bytes
	int BPP()    const { return m_ibpp; }    //!< bits per pixel
	
	//! Load an image from disk. Only TGA (24/32-bit) filetype is supported.
	int Load( const char * szFilename );
	
	int LoadTGA( const char * szFilename );
	int LoadSPR( const char * szFilename );
	
	int SaveRAW( const char * szFilename );
protected:	
	unsigned char * m_pData;
	int             m_iWidth;
	int             m_iHeight;
	int             m_ibpp;
	int             m_ipixwidth;

	int             m_ipitch;
};

#endif
