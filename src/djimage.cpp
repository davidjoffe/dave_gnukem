/*
djimage.cpp

Copyright (C) 1998-2024 David Joffe

dj2022-11 Note that since we're now on SDL2 we could potentially use e.g. SDLimage lib to more easily load 'better' file formats like png etc. (though would just require a new dependency)
*/

#include "config.h"
#include "djimage.h"
#include "djimageload.h"
#include "djfile.h"
#include "djstring.h"
#include "djtypes.h"
#include "djlog.h"
#include "djgraph.h"
#include "sys_error.h"
#include <string.h>
	#ifdef __OS2__
	#include <SDL/SDL_endian.h>
	#else
	#include <SDL_endian.h>
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

// hm these probably shouldn't be int's
djImage::djImage( int iWidth, int iHeight, int ibpp )
{
	m_iWidth    = iWidth;
	m_iHeight   = iHeight;
	m_ibpp      = ibpp;
	m_ipixwidth = CalculatePixelWidthBytesPerPixel( ibpp );
	m_ipitch = m_ipixwidth * iWidth;

	// If someone passes in parameters that make it overly huge to allocate then not sure what best to do but for
	// safety let's shrink image dimensions until it's not too big to allocate
	// Note on Win32 even though in theory we have 4GB in fact we have max 2GB for user-mode apps (and even then it's not all contiguous)
	// But still, that's a huge image size - it's not realistic really that for a game we are likely to want such a huge image,
	// it's more likely a mistake if it happens, but let's just shrink it down to something more reasonable
	// Let's make it maybe 1GB or hmm ~512MB max (which is still huge, but at least it's not 2GB)
	// If this happens it's either someone doing something dodgy or a developer error (e.g. trying to load excessively huge image)
	size_t uMemSize = m_iWidth * m_iHeight * m_ipixwidth;
	bool bWarnResized = false;
	//while (uMemSize > (size_t)(1L*1024*1024*1024))
	while (uMemSize > (size_t)(512*1024*1024))
	{
		m_iWidth /= 2;
		if (m_iWidth<1) m_iWidth=1;
		m_iHeight /= 2;
		if (m_iHeight<1) m_iHeight=1;
		m_ipitch = m_ipixwidth * m_iWidth;

		uMemSize = m_iWidth * m_iHeight * m_ipixwidth;
		bWarnResized = true;
	}
	if (bWarnResized)
	{
		std::string sError = "Error: djImage::Create: image size too large, resizing to " + std::to_string(m_iWidth) + "x" + std::to_string(m_iHeight) + "\n";
		SYS_Error("%s", sError.c_str());
		printf("%s", sError.c_str());
	}

	m_pData = new unsigned char[uMemSize];
	if (m_pData != NULL)
		memset( (void*)m_pData, 0, uMemSize );
	else
	{
		printf("Error: djImage::djImage: failed to allocate image memory\n");
		// Todo should we add try/catch in alloc ... also should we auto-shrink?
	}
}

djImage::~djImage()
{
	SYS_Debug("djImage::~djImage()\n");
	djDELV(m_pData);
}

void djImage::CreateImage( int x, int y, int nBitsPerPixel, int pitch/*=-1*/, void* pOptionalCopyDataFrom, unsigned int Rmask
	, unsigned int Gmask
	, unsigned int Bmask
	, unsigned int Amask
 )
{
	djDELV(m_pData);

	//dj2023-02 new fields adding to help with possible PNG loading different formats so we can be mroe explicit
	m_Rmask = Rmask;
	m_Gmask = Gmask;
	m_Bmask = Bmask;
	m_Amask = Amask;

	m_iWidth = x;
	m_iHeight = y;
	m_ibpp = nBitsPerPixel;
	m_ipixwidth = CalculatePixelWidthBytesPerPixel( nBitsPerPixel );
	
	// if pitch negative auto-calculate it by multiplying horizontal pixels by bytes-per-pixel
	if (pitch<=0) pitch = m_ipixwidth * x;
	// error-check: logically pitch must be a minimum of 'm_ipixwidth * x' (but it may be higher, that's the point of pitch/stride)
	if (pitch<m_ipixwidth * x) pitch = m_ipixwidth * x;

	m_ipitch = pitch;

	// dj2023-02 this code that was here for ages looks wrong but then it's always been wrong how can nobody have noticed until now?
	// "pitch*y*m_ipixwidth" seems to alloc too much ..? the pitch is already the 'bytes per row' so we should only have to multiply 'bytes per row * rows' I think?
	//m_pData = new unsigned char[pitch*y*m_ipixwidth];
	// if thing suddenly start crashing here though, this is probably the culprit .. dj2023-02 .. as I'm changing what appears to be an over-alloc of memory in this image data
	//const size_t uMemSize = pitch*y;

	// If someone passes in parameters that make it overly huge to allocate then not sure what best to do but for
	// safety let's shrink image dimensions until it's not too big to allocate
	// Note on Win32 even though in theory we have 4GB in fact we have max 2GB for user-mode apps (and even then it's not all contiguous)
	// But still, that's a huge image size - it's not realistic really that for a game we are likely to want such a huge image,
	// it's more likely a mistake if it happens, but let's just shrink it down to something more reasonable
	// Let's make it maybe 1GB or hmm ~512MB max (which is still huge, but at least it's not 2GB)
	// If this happens it's either someone doing something dodgy or a developer error (e.g. trying to load excessively huge image)
	size_t uMemSize = m_iWidth * m_iHeight * m_ipixwidth;
	bool bWarnResized = false;
	//while (uMemSize > (size_t)(1L*1024*1024*1024))
	while (uMemSize > (size_t)(512*1024*1024))
	{
		m_iWidth /= 2;
		if (m_iWidth<1) m_iWidth=1;
		m_iHeight /= 2;
		if (m_iHeight<1) m_iHeight=1;
		m_ipitch = m_ipixwidth * m_iWidth;

		uMemSize = m_iWidth * m_iHeight * m_ipixwidth;
		bWarnResized = true;
	}
	if (bWarnResized)
	{
		std::string sError = "Error: djImage::Create: image size too large, resizing to " + std::to_string(m_iWidth) + "x" + std::to_string(m_iHeight) + "\n";
		SYS_Error("%s", sError.c_str());
		printf("%s", sError.c_str());
	}


	m_pData = new unsigned char[uMemSize];
	if (m_pData != NULL)
	{
		if (pOptionalCopyDataFrom)
			memcpy((void*)m_pData, pOptionalCopyDataFrom, uMemSize);
		else
			memset( (void*)m_pData, 0, uMemSize);
	}
}

int djImage::CalculatePixelWidthBytesPerPixel( int nBitsPerPixel )
{
	switch (nBitsPerPixel)
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
	if (szFilename == NULL) return -1; // NULL string
	if (szFilename[0] == 0) return -1; // empty string

    std::string filename = szFilename;
    std::string extension = filename.substr(filename.find_last_of(".") + 1);
	extern void djStrToLowerTmpHelper( std::string& s );
    djStrToLowerTmpHelper(extension);

    // For TGA files pass to our own old TGA loader
	if (extension == "tga")
	{
		// fixme why are we bothering with all this? we only load TGA
		int ret = LoadTGA(szFilename);
		if (ret < 0)
		{
			// dj2022-11 hm this is maybe slightly gross must rethink where all the various logs "should" go etc. and clean up logging system
			printf("Warning: Image load failed: %s\n", szFilename);
			// fixme add some sort of 'debugassert' stuff here to help with testign?
		}
		return ret;
	}
	return djImageLoad::LoadImage(this, szFilename);
}

int djImage::LoadTGA( const char * szFilename )
{
	if (szFilename==NULL) return -1; // NULL string
	if (szFilename[0]==0) return -1; // empty string

	int iRet = -1;

	SYS_Debug ( "djImage::LoadTGA(%s)\n", szFilename );

	// Open the file
	FILE* pFile = djFile::dj_fopen(szFilename, "rb");// NB! MUST BE BINARY MODE (on Windows anyway; Linux it does nothing) and for reading
	if (pFile==nullptr)
	{
		SYS_Error ( "djImage::LoadTGA(%s): Couldn't open file\n", szFilename );
		return -1;
	}

	SdjHeaderTGA Header;

	size_t uRead = 0;
	#define djREADBYTES(pBuf, uSize) uRead = fread(pBuf, 1, uSize, pFile); if (uRead<(size_t)(uSize)) { SYS_Error ( "djImage::LoadTGA(%s): ERROR/WARNING reading TGA\n", szFilename ); }
	// Read the TGA header
	djREADBYTES(&Header, sizeof(Header));

	// Skip past the comment field
	if (Header.m_idLength!=0)
	{
		char szComments[512]={0};
		djREADBYTES(szComments, Header.m_idLength);
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
				// todo - handle bitmasks for 16-bit? or rather: should we even really support this? not sure... I think all our existing TGAs are 24 or 32-bit, and, in future any new images will be .png anyway ... so maybe phase out? [dj2023-11])
				unsigned char *pBuf = new unsigned char[nWidth*3];
				for ( unsigned int i=0; i<nHeight; ++i )
				{
					// Read a row of pixels and copy it into the image buffer
					djREADBYTES(pBuf, nWidth * 2);
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
				// R,G,B,A masks (dj2018-03 specify different masks here on PPC etc. - see https://github.com/davidjoffe/dave_gnukem/issues/100 - thanks to @BeWorld2018 for report and patch suggestion)
				//dj2023 moving this code and check from djgraph.h 'createHWsurface' to the LoadTGA code ->
				// This is due to PNGs having different byte order from TGAs, but createhwsurface should probably do things uniformly using these mask bits ..
				// so try move this here (likewise .png loader should set up 'equivalent but different')... hope that's correct solution and hope doesn't break any ports [dj2023-11]
#if SDL_BYTEORDER==SDL_BIG_ENDIAN
				// ARGB?
				m_Rmask=0x0000FF00;
				m_Gmask=0X00FF0000;
				m_Bmask=0xFF000000;
				m_Amask=0x000000FF;
#else
				// BGRA?
				m_Rmask=0x00FF0000;
				m_Gmask=0x0000FF00;
				m_Bmask=0x000000FF;
				m_Amask=0xFF000000;

#endif

				unsigned char *pBuf = new unsigned char[nWidth*3];
				for ( unsigned int i=0; i<nHeight; i++ )
				{
					// Read a row of pixels and copy it into the image buffer
					djREADBYTES(pBuf, nWidth * 3);
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

				// R,G,B,A masks (dj2018-03 specify different masks here on PPC etc. - see https://github.com/davidjoffe/dave_gnukem/issues/100 - thanks to @BeWorld2018 for report and patch suggestion)
				//dj2023 moving this code and check from djgraph.h 'createHWsurface' to the LoadTGA code ->
				// This is due to PNGs having different byte order from TGAs, but createhwsurface should probably do things uniformly using these mask bits ..
				// so try move this here (likewise .png loader should set up 'equivalent but different')... hope that's correct solution and hope doesn't break any ports [dj2023-11]
#if SDL_BYTEORDER==SDL_BIG_ENDIAN
				// ARGB?
				m_Rmask=0x0000FF00;
				m_Gmask=0X00FF0000;
				m_Bmask=0xFF000000;
				m_Amask=0x000000FF;
#else
				// BGRA?
				m_Rmask=0x00FF0000;
				m_Gmask=0x0000FF00;
				m_Bmask=0x000000FF;
				m_Amask=0xFF000000;
#endif
				unsigned char *pBuf = new unsigned char[nWidth*4];
				for ( unsigned int i=0; i<nHeight; i++ )
				{
					// Read a row of pixels and copy it into the image buffer
					djREADBYTES(pBuf, nWidth * 4);
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

	fclose( pFile );
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

//---------------------------------------------------------------------------
