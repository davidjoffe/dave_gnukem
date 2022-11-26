/*
djimage.cpp

Copyright (C) 1998-2022 David Joffe

dj2022-11 Note that since we're now on SDL2 we could potentially use e.g. SDLimage lib to more easily load 'better' file formats like png etc. (though would just require a new dependency)
*/

#include "config.h"
#include "djimage.h"
#include "djstring.h"
#include "djtypes.h"
#include "djlog.h"
#include "djgraph.h"
#include "sys_error.h"
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#ifdef WIN32
#include <io.h>//open, read etc. (this should maybe change in future)
#else
#include <unistd.h>
#endif
/*--------------------------------------------------------------------------*/
#ifdef WIN32
//#define FILECREATE_FLAGS (O_CREAT | O_TRUNC | O_BINARY | O_RDWR)
//#define FILECREATE_PERM  (S_IWRITE | S_IREAD)
#define FILEOPEN_FLAGS   (O_RDONLY | O_BINARY)
#else
//#define FILECREATE_FLAGS (O_CREAT | O_TRUNC | O_RDWR)
//#define FILECREATE_PERM  (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
#define FILEOPEN_FLAGS   (O_RDONLY)
#endif
/*--------------------------------------------------------------------------*/
// TGA types
enum EfdTGAType
{
	TGA_TYPE_MAPPED     =  1,
	TGA_TYPE_COLOR      =  2,
	TGA_TYPE_GRAY       =  3,
	TGA_TYPE_MAPPED_RLE =  9,
	TGA_TYPE_COLOR_RLE  = 10,
	TGA_TYPE_GRAY_RLE   = 11
};
// TGA image descriptor.
// 0-3: attribute bpp
#define TGA_DESC_ABITS      0x0f
// 4:   left-to-right ordering
#define TGA_DESC_HORIZONTAL 0x10
// 5:   top-to-bottom ordering
#define TGA_DESC_VERTICAL   0x20
// TGA file header
struct SdjHeaderTGA
{
	unsigned char m_idLength = 0;
	unsigned char m_colorMapType = 0;
	unsigned char m_iType = 0;
	unsigned char m_colorMapIndexLo = 0;
	unsigned char m_colorMapIndexHi = 0;
	unsigned char m_colorMapLengthLo = 0;
	unsigned char m_colorMapLengthHi = 0;
	unsigned char m_colorMapSize = 0;
	unsigned char m_xOriginLo = 0;
	unsigned char m_xOriginHi = 0;
	unsigned char m_yOriginLo = 0;
	unsigned char m_yOriginHi = 0;
	// Lo and Hi must be seperate to avoid endianness problems
	unsigned char m_iWidthLo = 0;
	unsigned char m_iWidthHi = 0;
	unsigned char m_iHeightLo = 0;
	unsigned char m_iHeightHi = 0;
	unsigned char m_iBitsPerPixel = 0;
	unsigned char m_iDescriptor = 0;
};
/*--------------------------------------------------------------------------*/
djImage::djImage()
{
	m_pData     = NULL;
	m_iWidth    = 0;
	m_iHeight   = 0;
	m_ibpp      = 0;
	m_ipixwidth = 0;
	m_ipitch    = 0;
}

djImage::djImage( int iWidth, int iHeight, int ibpp )
{
	m_iWidth    = iWidth;
	m_iHeight   = iHeight;
	m_ibpp      = ibpp;
	m_ipixwidth = CalculatePixelWidth( ibpp );
	m_ipitch = m_ipixwidth * iWidth;

	m_pData = new unsigned char[iWidth*iHeight*m_ipixwidth];

	if (m_pData != NULL)
		memset( (void*)m_pData, 0, iWidth*iHeight*m_ipixwidth );
}

djImage::~djImage()
{
	SYS_Debug("djImage::~djImage()\n");
	djDELV(m_pData);
}

void djImage::CreateImage( int x, int y, int ibpp, int ipitch/*=-1*/ )
{
	djDELV(m_pData);

	m_iWidth = x;
	m_iHeight = y;
	m_ibpp = ibpp;
	m_ipixwidth = CalculatePixelWidth( ibpp );
	if (ipitch==-1) ipitch = m_ipixwidth * x;
	m_ipitch = ipitch;

	m_pData = new unsigned char[ipitch*y*m_ipixwidth];
	if (m_pData != NULL)
		memset( (void*)m_pData, 0, ipitch*y*m_ipixwidth );
}

int djImage::CalculatePixelWidth( int ibpp )
{
	switch (ibpp)
	{
	case  8: return 1;
	case 16: return 2;
	case 24: return 3;
	case 32: return 4;
	}
	return 0;
}

djColor djImage::GetPixelColor( int x, int y ) const
{
	const uint32_t iPixel = GetPixel( x, y );
	switch (m_ibpp)
	{
	case 24: return djColor( (iPixel&0xFF0000)>>16, (iPixel&0xFF00)>>8, (iPixel&0xFF) );
	case 32: return djColor( (iPixel&0xFF0000)>>16, (iPixel&0xFF00)>>8, (iPixel&0xFF),
		(iPixel&0xFF000000)>>24);
	}
	//dj2018-01 case 32 didn't seem to return the alpha, am adding it now, hopefully won't cause side effects anywhere
	
	return djColor(0,0,0);
}

uint32_t djImage::GetPixel( int x, int y ) const
{

	if ( !m_pData || x>=m_iWidth || y>=m_iHeight )
		return -1;

	const int iOffset = m_ipixwidth * (y*m_iWidth + x);

	uint32_t iPixel = 0;
	memcpy( &iPixel, m_pData+iOffset, m_ipixwidth );

	return iPixel;
}

void djImage::PutPixel( int x, int y, uint32_t pixel )
{
	if ( !m_pData || x>=m_iWidth || y>=m_iHeight )
		return;

	const int iOffset = m_ipixwidth * (y*m_iWidth + x);

	memcpy( m_pData+iOffset, &pixel, m_ipixwidth );
}

int djImage::Load( const char * szFilename )
{
	if ( szFilename == NULL )
		return -1;

	char * szTemp = NULL;
	char * szExt = NULL;
	int    ret = -1;

	szTemp = djStrDeepCopy( szFilename );
	djStrToLower( szTemp );

	// Attempt to determine file type from extension
	if ( strlen( szTemp ) >= 4 )
	{
		szExt = szTemp + strlen(szTemp) - 4;
		//if (0 == strncmp( szExt, ".spr", 4 ))      ret = LoadSPR( szFilename );
		if (0 == strncmp( szExt, ".tga", 4 )) ret = LoadTGA( szFilename );
		else
		{
			// Attempt to load image as dj sprite file
			ret = -1;// LoadSPR(szFilename);
		}
	}
	else
	{
		// Attempt to load image as dj sprite file
		//ret = LoadSPR( szFilename );
	}

	djDELV(szTemp);

	return ret;
}

int djImage::LoadTGA( const char * szFilename )
{
	if (szFilename==NULL) return -1; // NULL string
	if (szFilename[0]==0) return -1; // empty string

	int iRet = -1;

	SYS_Debug ( "djImage::LoadTGA(%s)\n", szFilename );

	int fin=0;

	// Open the file
	if (0 > (fin = open( szFilename, FILEOPEN_FLAGS )))
	{
		SYS_Error ( "djImage::LoadTGA(%s): Couldn't open file\n", szFilename );
		return -1;
	}

	SdjHeaderTGA Header;

	// Read the TGA header
	read( fin, &Header, sizeof(Header) );

	// Skip past the comment field
	if (Header.m_idLength!=0)
	{
		char szComments[256]={0};
		read( fin, szComments, Header.m_idLength);
	}

	unsigned int nWidth  = MAKEINT16(Header.m_iWidthLo,Header.m_iWidthHi);
	unsigned int nHeight = MAKEINT16(Header.m_iHeightLo,Header.m_iHeightHi);
	unsigned int nType   = Header.m_iType;
//	unsigned int nABPP   = Header.m_iDescriptor & TGA_DESC_ABITS;
	unsigned int nBitsPerPixel = Header.m_iBitsPerPixel;

	bool bFlipY = !(Header.m_iDescriptor & 0x10);

	switch (nType)
	{
	case TGA_TYPE_MAPPED:
		SYS_Error ( "TGA type unsupported\n" );
		break;
	case TGA_TYPE_COLOR:
		// Load color targa
		{
			if (nBitsPerPixel==16)
			{
				// Create a blank image
				CreateImage( nWidth, nHeight, 16 );
				unsigned char *pBuf = new unsigned char[nWidth*3];
				for ( unsigned int i=0; i<nHeight; ++i )
				{
					// Read a row of pixels and copy it into the image buffer
					read( fin, pBuf, nWidth*2 );
					int iOffset = (bFlipY?(nHeight-i-1):i)*Pitch();
					for ( unsigned int j=0; j<nWidth; ++j )
					{
						// reverse endianness from file
						m_pData[iOffset+j*2+0] = pBuf[j*2+1];
						m_pData[iOffset+j*2+1] = pBuf[j*2+0];
					}
				}
				djDELV(pBuf);
				iRet = 0; // success
			}
			else if (nBitsPerPixel==24)
			{
				// FIXME: Creating a 32-bit image!
				// Create a blank image
				CreateImage( nWidth, nHeight, 32 );
				unsigned char *pBuf = new unsigned char[nWidth*3];
				for ( unsigned int i=0; i<nHeight; i++ )
				{
					// Read a row of pixels and copy it into the image buffer
					read( fin, pBuf, nWidth*3 );
					int iOffset = (bFlipY?(nHeight-i-1):i)*Pitch();
					for ( unsigned int j=0; j<nWidth; j++ )
					{
						// A,B,G,R
						m_pData[iOffset + j*4+0] = pBuf[j*3+0];
						m_pData[iOffset + j*4+1] = pBuf[j*3+1];
						m_pData[iOffset + j*4+2] = pBuf[j*3+2];
						m_pData[iOffset + j*4+3] = 0xFF;
					}
				}
				djDELV(pBuf);
				iRet = 0; // success
			}
			else if (nBitsPerPixel==32)
			{
				// Create a blank image
				CreateImage( nWidth, nHeight, 32 );
				unsigned char *pBuf = new unsigned char[nWidth*4];
				for ( unsigned int i=0; i<nHeight; i++ )
				{
					// Read a row of pixels and copy it into the image buffer
					read( fin, pBuf, nWidth*4 );
					int iOffset = (bFlipY?(nHeight-i-1):i)*Pitch();
					for ( unsigned int j=0; j<nWidth; j++ )
					{
						// A,B,G,R in file
						// B,G,R,A in file?
						// R,G,B,A in GGI
						// Thus BGRA in GGI?????
						// Huh? Hmm ... I see the "problem"
						// FIXME: Interesting "problem" .. it seems the VNC server
						// inverts the ordering of some of this stuff! So while doing
						// the remote thing I had everything confused!
						m_pData[iOffset + j*4+0] = pBuf[j*4+0];
						m_pData[iOffset + j*4+1] = pBuf[j*4+1];
						m_pData[iOffset + j*4+2] = pBuf[j*4+2];
						m_pData[iOffset + j*4+3] = pBuf[j*4+3];
					}
//					memcpy(m_pData+(bFlipY?(nHeight-i-1):i)*Pitch(), pBuf, nWidth*4);
				}
				djDELV(pBuf);
				iRet = 0; // success
			}
			else if (nBitsPerPixel==8)
			{
			}
		}
		break;
	case TGA_TYPE_GRAY:
		SYS_Error ( "TGA type unsupported\n" );
		break;
	case TGA_TYPE_MAPPED_RLE:
		SYS_Error ( "TGA type unsupported\n" );
		break;
	case TGA_TYPE_COLOR_RLE:
		SYS_Error ( "TGA type unsupported\n" );
		break;
	case TGA_TYPE_GRAY_RLE:
		SYS_Error ( "TGA type unsupported\n" );
		break;
	default:
		SYS_Error ( "Invalid TGA type!\n" );
	}

	close( fin );
	return iRet;
}

/*
//dj2022-11 for now comment out SaveRAW as not using it for anything and it's causing warnings, but maybe might use it in future so leaving it in for now, but should maybe be refactored differently (probably shouldn't be *in* the image class, image loaders/savers should conceptually be a layer above and 'outside' the core image class)
int djImage::SaveRAW( const char * szFilename )
{
	if ( m_pData == NULL )
		return -2;

	// %$@*#&@^%#*@!!
	// I can't use "creat", because of a bug in Microsoft's libraries
	// (they just assume you want a text file, and translate all CR's
	// found in the output data for "write")
	// fin = creat( szFilename, FILECREATE_FLAGS );
	int fin = open( szFilename, FILECREATE_FLAGS, FILECREATE_PERM );
	if (fin < 0)
	{
		return -1;
	}

	write( fin, (void*)m_pData, m_iWidth * m_iHeight * m_ipixwidth );

	close( fin );

	return 0;
}

*/